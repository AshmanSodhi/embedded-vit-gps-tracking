#include <WiFi.h>
#include <HTTPClient.h>
#include <TinyGPS++.h>
#include <Wire.h>
#include <U8g2lib.h>

// WiFi
const char* ssid = "Ash Redmi Note 12 Pro+ 5G";
const char* password = "ashsod99";

#define SWITCH_PIN 18
#define LED_PIN 19

bool isOnline = false;
int button_state;
int last_button_state;

// API (Render)
const char* API_URL = "https://embedded-vit-gps-tracking.onrender.com/update";

// Google Sheets
String scriptURL = "https://script.google.com/macros/s/AKfycbzSzTxr8dzgx79bpR8iMTTs0UuDAXfot4JG2ZUiRlHyIIejVr7dsb7mQDEWDoaV9UpmnA/exec";

// GPS
TinyGPSPlus gps;
HardwareSerial gpsSerial(2);

// OLED
U8G2_SH1106_128X64_NONAME_F_HW_I2C display(U8G2_R0, U8X8_PIN_NONE);

// Timer
unsigned long lastSend = 0;
const int interval = 5000;

void setup() {
  Serial.begin(115200);
  pinMode(SWITCH_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  button_state = digitalRead(SWITCH_PIN);

  // GPS init
  gpsSerial.begin(9600, SERIAL_8N1, 16, 17);

  // OLED init
  Wire.begin(21, 22);
  display.begin();
  display.setFont(u8g2_font_ncenB08_tr);

  display.clearBuffer();
  display.drawStr(0, 30, "Connecting WiFi...");
  display.sendBuffer();

  // WiFi connect
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected");

  display.clearBuffer();
  display.drawStr(0, 30, "GPS Initializing...");
  display.sendBuffer();
}

void loop() {
  int current_button_state = digitalRead(SWITCH_PIN);

  // Detect state change: from HIGH to LOW (press)
  if (last_button_state == HIGH && current_button_state == LOW) { // Simple debounce// Confirm it's still pressed
      isOnline = !isOnline; // Toggle state
      digitalWrite(LED_PIN, isOnline ? HIGH : LOW); // Toggle LED
      
      Serial.print("Status changed to: ");
      Serial.println(isOnline ? "ONLINE" : "OFFLINE");
  }
  last_button_state = current_button_state;
  // Read GPS continuously
  while (gpsSerial.available()) {
    gps.encode(gpsSerial.read());
  }

  // Display always (even if not sending)
  updateOLED();

  // Send every 5 sec
  if (millis() - lastSend > interval) {
    lastSend = millis();

    if (gps.location.isValid()) {
      float lat = gps.location.lat();
      float lon = gps.location.lng();
      float speed = gps.speed.kmph();

      Serial.println("Sending Data...");
      Serial.println(lat, 6);
      Serial.println(lon, 6);

      sendToAPI(lat, lon, speed);
      sendToGoogleSheets(lat, lon);

    } else {
      Serial.println("No GPS Fix");
    }
  }
}

/////////////////////////////////////////////////////
// SEND TO YOUR API (Render)
/////////////////////////////////////////////////////
void sendToAPI(float lat, float lon, float speed) {

  if (WiFi.status() == WL_CONNECTED) {

    HTTPClient http;
    http.begin(API_URL);
    http.addHeader("Content-Type", "application/json");

    String status = isOnline ? "ONLINE" : "OFFLINE";

    String json = "{";
    json += "\"bus_id\":\"BUS_01\",";
    json += "\"lat\":" + String(lat, 6) + ",";
    json += "\"lon\":" + String(lon, 6) + ",";
    json += "\"speed\":" + String(speed, 2) + ",";
    json += "\"status\":\"" + status + "\"";
    json += "}";

    int code = http.POST(json);

    Serial.print("API Response: ");
    Serial.println(code);

    http.end();
  }
}

/////////////////////////////////////////////////////
// SEND TO GOOGLE SHEETS
/////////////////////////////////////////////////////
void sendToGoogleSheets(float lat, float lon) {

  if (WiFi.status() == WL_CONNECTED) {

    HTTPClient http;

    String url = scriptURL + "?lat=" + String(lat,6) + "&lng=" + String(lon,6);

    http.begin(url);
    int code = http.GET();

    Serial.print("Sheets Response: ");
    Serial.println(code);

    http.end();
  }
}

/////////////////////////////////////////////////////
// OLED DISPLAY
/////////////////////////////////////////////////////
void updateOLED() {

  display.clearBuffer();

  if (gps.location.isValid()) {

    String lat = "Lat: " + String(gps.location.lat(), 6);
    String lon = "Lng: " + String(gps.location.lng(), 6);
    String status = isOnline ? "ONLINE" : "OFFLINE";

    display.drawStr(0, 15, status.c_str());
    display.drawStr(0, 30, lat.c_str());
    display.drawStr(0, 50, lon.c_str());

  } else {
    display.drawStr(0, 30, "Waiting GPS...");
  }

  display.sendBuffer();
}

void updateStatus() {
  
}
