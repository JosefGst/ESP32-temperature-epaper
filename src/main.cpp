#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "DHT.h"

#define ENABLE_GxEPD2_GFX 0
#include <GxEPD2_BW.h>
#include <GxEPD2_3C.h>
#include <GxEPD2_7C.h>
#include <Fonts/FreeMonoBold9pt7b.h>

#define MAX_DISPLAY_BUFFER_SIZE 800 // 
#define MAX_HEIGHT(EPD) (EPD::HEIGHT <= MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8) ? EPD::HEIGHT : MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8))
//BUSY -> 4, RST -> 16, DC -> 17, CS -> SS(5), CLK -> SCK(18), DIN -> MOSI(23), GND -> GND, 3.3V -> 3.3V
GxEPD2_BW<GxEPD2_270, GxEPD2_270::HEIGHT> display(GxEPD2_270(/*CS=5*/ SS, /*DC=*/ 17, /*RST=*/ 16, /*BUSY=*/ 4));

#define DHTPIN 13     // Digital pin connected to the DHT sensor
// Feather HUZZAH ESP8266 note: use pins 2, 3, 4, 5, 12, 13 or 14 --
//! pin 2 had problems with the connection while flashing

// Uncomment whatever type you're using!
#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

const char* ssid = "myHotspot";
const char* password = "wifijosef";
const String endpoint = "http://api.openweathermap.org/data/2.5/weather?q=";
const String location = "Hong Kong";
const String key = "92d5346233bb17d38c01bb14827fd07a"; //API key

DHT dht(DHTPIN, DHTTYPE);

float temp;
float hindex;
int hum;
const char* description;

void setup() {
  Serial.begin(115200);
  delay(100);
  display.init(115200);
  display.setFullWindow();
  display.setRotation(1);
  display.setFont(&FreeMonoBold9pt7b);
  display.setTextColor(GxEPD_BLACK);
  dht.begin();
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  } 
  Serial.println("Connected to the WiFi network");
}

void loop() {
  if ((WiFi.status() == WL_CONNECTED)) { //Check the current connection status
    HTTPClient http;
    http.begin(endpoint + location + "&appid=" + key);
    int httpCode = http.GET();
    if (httpCode > 0) { //Check for the returning code
      StaticJsonDocument<1100> doc;
      String payload = http.getString();
      DeserializationError err = deserializeJson(doc, payload);
      if (err){
        Serial.print("ERROR: ");
        Serial.println(err.c_str());
        return;
      }
      //Serial.println(httpCode);
      //Serial.println(payload);
          
          
      description = doc["weather"][0]["description"];
      temp = doc["main"]["temp"];
      temp = temp - 273.15;
      hindex = doc["main"]["feels_like"];
      hum = doc["main"]["humidity"];

      Serial.print(F("Weather in "));
      Serial.print(location);
      Serial.println(":");
      Serial.println(F("OPEN WEATHER: "));
      Serial.print(F("The weather is "));
      Serial.println(description);
      Serial.print(F("Humidity: "));
      Serial.print(hum);
      Serial.print(F(".00%  Temperature: "));
      Serial.print(temp); //convert to celsius
      Serial.print(F("°C  Heat index: "));
      Serial.print(hindex- 273.15);
      Serial.println(F("°C"));
    } 
    else {
        Serial.println("Error on HTTP request");
    }
    http.end(); //Free the resources
  }
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
  

  // Compute heat index in Fahrenheit (the default)
  //float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);
  Serial.println(F("DHT: ")); 
  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.print(F("°C "));
  //Serial.print(f);
  Serial.print(F(" Heat index: "));
  Serial.print(hic);
  Serial.println(F("°C "));
  //Serial.print(hif);
  //Serial.println(F("°F"));

  // e-paper display
  display.firstPage();
  do
  {
    int16_t tbx, tby; uint16_t tbw, tbh;
    display.getTextBounds("DHT: ", 0, 0, &tbx, &tby, &tbw, &tbh);
    // center bounding box by transposition of origin:
    uint16_t x = 0;
    uint16_t y = tbh;
    display.setCursor(x,y);
    display.fillScreen(GxEPD_WHITE);
    display.print(F("Weather in "));
    display.print(location);
    display.println(":");
    display.println(F("Outside Temp from web: "));
    //display.print(F("The weather is "));
    //display.println(description);
    display.print(F("Humidity: "));
    display.print(hum); display.println("%");
    display.print(F("Temperature: "));
    display.print(temp); display.println("C");
    display.print(F("Feels like: "));
    display.print(hindex- 273.15);
    display.println(F("C"));
    display.println();
    

    display.println(F("Indoor Temp Sensor: ")); 
    display.print(F("Humidity: "));
    display.print(h);
    display.println(F("%"));
    display.print(F("Temperature: "));
    display.print(t);
    display.println(F("C"));
    display.print(F("Feels like: "));
    display.print(hic);
    display.println(F("C "));


  }
  while (display.nextPage());

  delay(30000);
}