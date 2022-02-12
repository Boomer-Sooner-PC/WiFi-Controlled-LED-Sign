// configure these vaiables
const char *ssid = "Wifi Name";
const char *password = "Password";
#define DATA_PIN 4
#define LED_PIN 5
#define MATRIX_WIDTH 32
#define MATRIX_HEIGHT 8
#define HOSTNAME "LED-Sign"
#define PORT 80 // suggested to leave at 80 so you dont have to add a ":port" to the end of your url

// import libraries
#include "WiFi.h"
#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

LiquidCrystal_I2C lcd(0x27, 20, 4); // sets up LCD

// set up led matrix
#define arr_len(x) (sizeof(x) / sizeof(*x))
Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(MATRIX_WIDTH, MATRIX_HEIGHT, DATA_PIN, NEO_MATRIX_TOP + NEO_MATRIX_LEFT + NEO_MATRIX_COLUMNS + NEO_MATRIX_PROGRESSIVE + NEO_MATRIX_ZIGZAG, NEO_GRB + NEO_KHZ800);

WiFiServer server(PORT); // initialize wifi server

// Variables for later
String header;
unsigned long currentTime = millis();
unsigned long previousTime = 0;
const long timeoutTime = 2000;
String message = "";
int speedVar = 100;
int red = 255;
int green = 255;
int blue = 255;
int brightness = 50;
int x = matrix.width();

void setup()
{
  // turns status LED off
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  Serial.begin(115200);
  lcd.begin();
  lcd.backlight();
  lcd.clear();

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  // status message
  lcd.setCursor(0, 0);
  lcd.print("Connecting to: ");
  lcd.setCursor(0, 1);
  lcd.print(ssid);
  WiFi.setHostname(HOSTNAME);
  lcd.setCursor(0, 3);
  WiFi.begin(ssid, password); // connects to wifi
  while (WiFi.status() != WL_CONNECTED)
  {
    // loading animation
    delay(500);
    Serial.print(".");
    lcd.print(".");
  }
  // Print local IP address and start web server
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WiFi Connected");
  lcd.setCursor(0, 1);
  lcd.print("IP address: ");
  lcd.setCursor(0, 3);
  lcd.print(WiFi.localIP());

  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  server.begin(); // turns on server

  // sets up matrix
  matrix.begin();
  matrix.show();
  matrix.setTextWrap(false);
  matrix.setBrightness(brightness);
  matrix.setTextColor(matrix.Color(0, 0, 0));

  // turns on status LED
  digitalWrite(LED_PIN, HIGH);
}

void loop()
{
  header = "";
  WiFiClient client = server.available();

  if (client)
  { // if there is a request
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New request");
    String currentLine = "";
    while (client.connected() && currentTime - previousTime <= timeoutTime)
    { // loop while the client's connected
      currentTime = millis();
      if (client.available())
      {                         // if there's bytes to read from the client,
        char c = client.read(); // read a byte, then
        header += c;            // add it to the header
        if (c == '\n')
        { // if the next byte is a line break then the request header is complete
          String path = header;
          path.remove(0, 5); // trims off some extra data from the header
          int len = path.length();
          path.remove(len - 11, 11);
          Serial.println(path);

          if (path.indexOf("speed/") >= 0)
          {
            path.remove(0, 6); // removes speed/ from header
            path = urldecode(path);
            speedVar = path.toInt();

            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Set speed to:");
            lcd.setCursor(0, 1);
            lcd.print(path);
            lcd.setCursor(0, 3);
            lcd.print(WiFi.localIP());
          }
          else if (path.indexOf("brightness/") >= 0)
          {
            path.remove(0, 11); // removes speed/ from header
            path = urldecode(path);
            brightness = path.toInt();

            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Set brightness to:");
            lcd.setCursor(0, 1);
            lcd.print(path);
            lcd.setCursor(0, 3);
            lcd.print(WiFi.localIP());
          }
          else if (path.indexOf("text/") >= 0)
          {
            path.remove(0, 5); // removes text/ from header
            if (path.indexOf("favicon.ico") == -1)
            { // if the request is not favicon.ico
              path = urldecode(path);
              message = path;

              lcd.clear();
              lcd.setCursor(0, 0);
              lcd.print("Set message to:");
              lcd.setCursor(0, 1);
              lcd.print(path);
              lcd.setCursor(0, 3);
              lcd.print(WiFi.localIP());
            }
          }
          else if (path.indexOf("color/") >= 0)
          {
            path.remove(0, 6); // removes the color/
            path = urldecode(path);
            long number = (long)strtol(&path[0], NULL, 16);
            red = number >> 16;
            green = number >> 8 & 0xFF;
            blue = number & 0xFF;

            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Set color to:");
            lcd.setCursor(0, 1);
            lcd.print(path);
            lcd.setCursor(0, 3);
            lcd.print(WiFi.localIP());
          }

          // write out the webpage
          client.println("HTTP/1.1 200 OK");
          client.println("Content-type:text/html");
          client.println("Connection: close");
          client.println();

          // sends the HTML
          client.println("<!DOCTYPE html><html lang='en'> <head> <meta charset='UTF-8' /> <meta http-equiv='X-UA-Compatible' content='IE=edge' /> <meta name='viewport' content='width=device-width, initial-scale=1.0' /> <title>LED Sign</title> <script> function textSubmit() { text = document.getElementById('text').value; window.location = window.location.origin + '/text/' + text; } function speedSubmit() { text = document.getElementById('speed').value; window.location = window.location.origin + '/speed/' + text; } function colorSubmit() { text = document.getElementById('color').value.replace('#', ''); window.location = window.location.origin + '/color/' + text; } function brightnessSubmit() { text = document.getElementById('brightness').value; window.location = window.location.origin + '/brightness/' + text; } </script> </head> <body style='align-content: center'> <h1 style='text-align: center; font-family: monospace'> LED Sign Control </h1> <form style='text-align: center' onsubmit='return false'> <input type='text' id='text' style='width: 30vw; margin-left: 0vw' /><br /> <label style='font-family: monospace' >Enter the text you want displayed here</label ><br /> <button onclick='textSubmit()'>Enter</button> </form> <form style='text-align: center; margin-top: 5vh' onsubmit='return false' > <input type='text' id='speed' style='width: 30vw; margin-left: 0vw' /><br /> <label style='font-family: monospace' >Enter the delay between updates</label ><br /> <button onclick='speedSubmit()'>Enter</button> </form> <form style='text-align: center; margin-top: 5vh' onsubmit='return false' > <input type='color' value='#ffffff' id='color' style='width: 30vw; margin-left: 0vw' /><br /> <label style='font-family: monospace'>Select color</label><br /> <button onclick='colorSubmit()'>Enter</button> </form> <form style='text-align: center; margin-top: 5vh' onsubmit='return false' > <input type='range' id='brightness' style='width: 30vw; margin-left: 0vw' min='1' max='100' value='50' step='1' /><br /> <label style='font-family: monospace' >Set the brightness 1-100</label ><br /> <button onclick='brightnessSubmit()'>Enter</button> </form> </body></html>");
          client.println();
          client.stop();
        }
        else if (c != '\r')
        {                   // if you got anything else but a carriage return character,
          currentLine += c; // add it to the end of the currentLine
        }
      }
    }
  }

  matrix.fillScreen(0);   // clear display
  matrix.setCursor(x, 0); // moves to the location where to print it
  matrix.print(message);

  int i = message.length() * -6; // the width of the message

  matrix.setTextColor(matrix.Color(red, green, blue)); // set the color
  matrix.setBrightness(brightness);                    // set the brightness

  if (--x < i)
  {                     // if decreasing x is less than i
    x = matrix.width(); // reset x
  }

  matrix.show();
  delay(speedVar);
}

String urldecode(String str) // converts the url style string back to a normal string
{

  String encodedString = "";
  char c;
  char code0;
  char code1;
  for (int i = 0; i < str.length(); i++)
  {
    c = str.charAt(i);
    if (c == '+')
    {
      encodedString += ' ';
    }
    else if (c == '%')
    {
      i++;
      code0 = str.charAt(i);
      i++;
      code1 = str.charAt(i);
      c = (h2int(code0) << 4) | h2int(code1);
      encodedString += c;
    }
    else
    {

      encodedString += c;
    }

    yield();
  }

  return encodedString;
}

unsigned char h2int(char c)
{
  if (c >= '0' && c <= '9')
  {
    return ((unsigned char)c - '0');
  }
  if (c >= 'a' && c <= 'f')
  {
    return ((unsigned char)c - 'a' + 10);
  }
  if (c >= 'A' && c <= 'F')
  {
    return ((unsigned char)c - 'A' + 10);
  }
  return (0);
}
