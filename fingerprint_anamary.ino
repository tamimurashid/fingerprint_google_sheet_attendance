#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_Fingerprint.h>
#include <HardwareSerial.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <WiFi.h>


// Fingerprint sensor on Serial2
#define RXD2 17
#define TXD2 16
#define BUZZER 5

HardwareSerial mySerial(2);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);


// Google Apps Script Info
const char* host = "script.google.com";
const int httpsPort = 443;
String server_id = "AKfycbyTtO-9QeHjDd68ibW0uV8hM5ZFBer2nC8G6h83oEQGifMnkFK-6We_KOkss0c8JMQ-";

// WiFi Credentials
const char* ssid = "Reindeer";
const char* password = "200120022003";

WiFiClientSecure client;


// LCD on SDA=21, SCL=22
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  // WiFi.begin(ssid, password);
  Wire.begin(21, 22);         // I2C pins for ESP32
  lcd.begin(16, 2);           // Initialize 20x4 LCD
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("System init..");

  Serial.begin(115200);
  mySerial.begin(57600, SERIAL_8N1, RXD2, TXD2);

  finger.begin(57600);
  if (finger.verifyPassword()) {
    Serial.println("Fingerprint sensor connected.");
    lcd.setCursor(0, 1);
    lcd.print("Sensor Found");
  } else {
    Serial.println("Fingerprint sensor NOT found.");
    lcd.setCursor(0, 1);
    lcd.print("Sensor Not Found");
    while (true);  // Stop everything
  }

  delay(2000); // Wait and then clear
  WiFi.setSleep(false);
  WiFi.begin(ssid, password);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WiFi  Status");
  lcd.setCursor(0, 1);
  lcd.print("Connecting.........");
  Serial.println("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WiFi  Connected!");
  lcd.setCursor(0, 1);
  lcd.print(WiFi.localIP().toString());
  Serial.println("Connected!");

  client.setInsecure(); // Disable SSL cert validation
}

void loop() {

  // Obfuscate the ID before sending
    char hexID[10];
    sprintf(hexID, "%04X", finger.fingerID); // Uppercase hex format
    String id = "ID_" + String(hexID); // Final obfuscated string

  int result = getFingerprintID();
  if (result == FINGERPRINT_OK) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Access Granted!");
    lcd.setCursor(0, 1);
    lcd.print("User ID: ");
    lcd.print(id);


    successFeedback();
    sendToGoogle(id);

  } else if (result == FINGERPRINT_NOTFOUND) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Access Denied!");
    lcd.setCursor(0, 1);
    lcd.print("Try again...");
    sendToGoogle("E_000");
    failFeedback();
  }

  delay(3000);  // Show result for 3 sec

  // Reset back to prompt
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Scan fingerprint");
  lcd.setCursor(0, 1);
  lcd.print("to access system");
  delay(1000);  // slight pause before next scan
}

int getFingerprintID() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK) return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK) return -1;

  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK) {
    Serial.print("Found ID: "); Serial.println(finger.fingerID);
    Serial.print("Confidence: "); Serial.println(finger.confidence);
    return FINGERPRINT_OK;
  } else {
    Serial.println("No match found.");
    return FINGERPRINT_NOTFOUND;
  }
}

void sendToGoogle(String uid) {
  if (!client.connect(host, httpsPort)) {
    Serial.println(" HTTPS Connection failed");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Error !");
    lcd.setCursor(0, 1);
    lcd.print("Https conn fail");
    return;
  }

  String url = "/macros/s/" + server_id  + "/exec";
  String postData = "user_id=" + uid;  //  Fix here

  client.println("POST " + url + " HTTP/1.1");
  client.println("Host: " + String(host));
  client.println("Content-Type: application/x-www-form-urlencoded");
  client.print("Content-Length: ");
  client.println(postData.length());
  client.println("Connection: close");
  client.println();
  client.println(postData);

  delay(100);
  client.stop();
}

void successFeedback() {
  tone(BUZZER, 1000, 150); delay(150);
  tone(BUZZER, 1500, 150); delay(150);
  noTone(BUZZER);
}

void failFeedback() {
  tone(BUZZER, 400, 300); delay(300);
  noTone(BUZZER);
}


