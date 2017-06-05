#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <time.h>
#include <ESP8266WiFi.h>
#include "wifisettings.h"

LiquidCrystal_I2C lcd(0x27, 16, 2);

const char* host = "support.flemingfulton.org.uk";
const char* url = "/node/14?tca=4ezZx6G1a00khifZ87eQzQfqN9uAew47G-ybgmL8aUI";

// Variables for timings
const int jobsInterval = 180000;  // Update interval for jobs - 3 minutes
const int timeInterval = 30000;   // Update interval for time - 30 seconds

unsigned long currentMillis = 0;
unsigned long previousJobMillis = -jobsInterval;  // Set previous time as negative to allow imediate update
unsigned long previousTimeMillis = -timeInterval; // Set previous time as negative to allow imediate update

int find_text(String needle, String haystack, int from) {
  int foundpos = -1;
  if (haystack.length() < needle.length())
    return foundpos;
  for (int i = from; (i < haystack.length() - needle.length()); i++) {
    if (haystack.substring(i, needle.length() + i) == needle) {
      foundpos = i;
      return foundpos;
    }
  }
  return foundpos;
}

void setup() {
  Serial.begin(115200);
  delay(100);

  lcd.init();   // initializing the LCD
  lcd.backlight(); // Enable or Turn On the backlight
  delay(200);

  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  lcd.setCursor(0, 0);
  lcd.print("Connecting");

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  int i = 4;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (i < 4) {
      lcd.setCursor(10 + i, 0);
      lcd.print (".");
      i++;
    } else {
      lcd.setCursor(10, 0);
      lcd.print ("    ");
      i = 0;
    }
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  configTime(1 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  lcd.clear();
}

void loop() {
  currentMillis = millis();
  updateJobs();
  updateTime();
}

void updateJobs() {
  if (currentMillis - previousJobMillis >= jobsInterval) {
    // save the last time you blinked the LED
    Serial.print("connecting to ");
    Serial.println(host);

    // Use WiFiClient class to create TCP connections
    WiFiClient client;
    const int httpPort = 80;
    if (!client.connect(host, httpPort)) {
      Serial.println("connection failed");
      lcd.clear();
      lcd.print("Connection");
      lcd.setCursor(0, 1);
      lcd.print("Failed");
    }

    // We now create a URI for the request

    Serial.print("Requesting URL: ");
    Serial.println(url);
    // This will send the request to the server
    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" +
                 "Connection: close\r\n\r\n");
    delay(500);

    // Read all the lines of the reply from server and print them to Serial
    while (client.available()) {
      String line = client.readStringUntil('\r');

      //      int start_loc = find_text("<html>", line, 0);
      //      int end_loc = find_text("</html>", line, 0);
      int start_loc = find_text("<total>", line, 0);
      int end_loc = find_text("</total>", line, 0);

      lcd.setCursor(0, 0);
      //Serial.println(line);
      if (start_loc > 0 && end_loc > 0)
      {
        Serial.println("Jobs: ");
        lcd.print("Jobs: ");
        for (int i = start_loc + 7; i < end_loc; i++)
        {
          Serial.print(line[i]);
          lcd.print(line[i]);
        }
              lcd.print("   ");

        //        lcd.setCursor(0, 1);
        //        for (int i = 0; i < 16; i++) {
        //          lcd.print(" ");
        //        }

        int start_loc2 = find_text("<urgent>", line, end_loc + 1);
        int end_loc2 = find_text("</urgent>", line, end_loc + 1);

        lcd.setCursor(9, 0);
        if (start_loc2 > 0 && end_loc2 > 0)
        {
          Serial.println("Urgent:");
          lcd.print("Urg: ");
          for (int i = start_loc2 + 8; i < end_loc2; i++)
          {
            Serial.print(line[i]);
            lcd.print(line[i]);
          }
        }
      }

      //       Serial.println(line);
    }

    Serial.println();
    Serial.println("closing connection");

    previousJobMillis = currentMillis;
  }
}

void updateTime() {
  if (currentMillis - previousTimeMillis >= timeInterval) {
    lcd.setCursor(0, 1);
    time_t t = time(NULL);
    lcd.print(ctime(&t));

    int hour = localtime(&t)->tm_hour;
    if (hour < 8 or hour > 17) {
      lcd.noBacklight();
    } else {
      lcd.backlight();
    }
//    Serial.print("Hour: ");
//    Serial.println(hour);
    previousTimeMillis = currentMillis;
  }
}


void flashLED() {
  lcd.noBacklight();
  delay(1000);
  lcd.backlight();
  delay(1000);
  lcd.noBacklight();
  delay(1000);
  lcd.backlight(); // Enable or Turn On the backlight
  delay(800);
  lcd.noBacklight();
  delay(500);
  lcd.backlight(); // Enable or Turn On the backlight
  delay(400);
}

