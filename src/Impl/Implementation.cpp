#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <MFRC522.h>
#include <SPI.h>
#include <WiFi.h>

// OLED pins
#define OLED_SDA 4
#define OLED_SCL 15
#define OLED_RST 16
#define SCREEN_WIDTH 128 // Width
#define SCREEN_HEIGHT 64 // Height

#define SS_PIN 5
#define RST_PIN 14
MFRC522 myRFID(SS_PIN, RST_PIN); // Create MFRC522 instance.
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);

const unsigned char GREEN_LED = 12;
const unsigned char RED_LED = 17;
const unsigned char BUZZER_PIN = 13;
const String CARD_UID = " 25 3c da 3f";
unsigned char FLAG = 0;

const char *SSID = "Imm.LeMontfort";
// const char* password = "nibbieking";

const char *URL = "https://qa74pu7ut9.execute-api.us-east-1.amazonaws.com/"
                  "PostStage/SmartAttendance";
const String checkInRes = "\"check-in\"";
const String checkOutRes = "\"check-out\"";
const String insuffTime = "\"Insufficient time\"";
const String notRegistered = "\"Not registered\"";

// Initializes WiFi connection.
void initWiFi() {

  /* if (!WiFi.config(local_IP, gateway, subnet)) {
    Serial.println("Failed to configure static IP");
  } */

  WiFi.begin(SSID);
  Serial.println("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
}

String createJSONPayload(const String &UID) {
  // Create a JSON document
  StaticJsonDocument<200> doc;
  doc["UID"] = UID;

  // Serialize the JSON document to a string
  String jsonString;
  serializeJson(doc, jsonString);

  return jsonString;
}

String sendPostRequest(const char *url, const String &jsonPayload) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    Serial.print("Connecting to: ");
    Serial.println(url);

    http.begin(url);
    http.addHeader("Content-Type",
                   "application/json"); // Set the content type to JSON

    int httpResponseCode = http.POST(jsonPayload);

    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);

      String response = http.getString();
      Serial.println("Full Response:");
      Serial.println(response);

      // Parse JSON response and return the body
      return parseJSON(response);
    } else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
      return "Error: HTTP request failed";
    }

    http.end(); // Free resources
  } else {
    Serial.println("Wi-Fi not connected");
    return "Error: Wi-Fi not connected";
  }
}

String parseJSON(const String &jsonString) {
  StaticJsonDocument<300> doc;

  // Deserialize the JSON string
  DeserializationError error = deserializeJson(doc, jsonString);

  if (error) {
    Serial.print("JSON parse failed: ");
    Serial.println(error.c_str());
    return "Error: JSON parse failed";
  }

  // Extract the body field
  const char *body = doc["body"];
  if (body) {
    return String(body);
  } else {
    return "Error: No 'body' field found in JSON";
  }
}

void setup() {
  Serial.begin(9600); // Initiate a serial communication
  SPI.begin();        // Initiate  SPI bus
  myRFID.PCD_Init();  // Initiate MFRC522
  Serial.println("Please scan your RFID card...");
  Serial.println();
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(OLED_RST, OUTPUT);
  digitalWrite(OLED_RST, LOW);
  delay(20);
  digitalWrite(OLED_RST, HIGH);

  Wire.begin(OLED_SDA, OLED_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3c, false,
                     false)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ; // Don't proceed, loop forever
  }

  display.clearDisplay();

  // Set text size, color, and start position
  display.setTextSize(1); // Text size (1 is the smallest, increase as needed)
  display.setTextColor(SSD1306_WHITE); // Text color
  display.setCursor(0, 0);             // Starting position

  // Display text
  display.println("Connecting to WiFi...");

  // Send buffer to the display
  display.display();

  initWiFi();
}

void defaultDisplay() {
  display.clearDisplay();

  // Set text size, color, and start position
  display.setTextSize(2); // Text size (1 is the smallest, increase as needed)
  display.setTextColor(SSD1306_WHITE); // Text color
  display.setCursor(16, 0);            // Starting position

  // Display text
  display.println("SOEN 422");
  display.setTextSize(1);
  display.setCursor(16, 22);
  display.println("Embedded Systems");
  display.setCursor(46, 35);
  display.println("Lab 9");
  display.setCursor(46, 52);
  display.println("H-968");

  // Send buffer to the display
  display.display();
}

void checkInOutput(unsigned char valid) {
  if (valid) {
    digitalWrite(GREEN_LED, HIGH);
    tone(BUZZER_PIN, 500, 500);
    delay(400);
    digitalWrite(GREEN_LED, LOW);
    noTone(BUZZER_PIN);
  } else {
    for (unsigned char i = 0; i < 2; i++) {
      digitalWrite(RED_LED, HIGH);
      tone(BUZZER_PIN, 275, 200);
      delay(200);
      digitalWrite(RED_LED, LOW);
      noTone(BUZZER_PIN);
      delay(200);
    }
  }
}

void checkOutOutput(unsigned char valid) {
  if (valid) {
    for (unsigned char i = 0; i < 2; i++) {
      digitalWrite(GREEN_LED, HIGH);
      tone(BUZZER_PIN, 600, 100);
      delay(100);
      digitalWrite(GREEN_LED, LOW);
      noTone(BUZZER_PIN);
      delay(100);
    }
  } else {
    for (unsigned char i = 0; i < 2; i++) {
      digitalWrite(RED_LED, HIGH);
      tone(BUZZER_PIN, 275, 200);
      delay(200);
      digitalWrite(RED_LED, LOW);
      noTone(BUZZER_PIN);
      delay(200);
    }
  }
}

void displayValid() {
  display.clearDisplay();

  if (!FLAG) {
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(16, 10);
    display.println("Check-in");
    display.setTextSize(1);
    display.setCursor(24, 30);
    display.println("Bence Matajsz");
    display.setTextSize(1);
    display.setCursor(40, 44);
    display.println("40322797");
  } else {
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(10, 10);
    display.println("Check-out");
    display.setTextSize(1);
    display.setCursor(24, 30);
    display.println("Bence Matajsz");
    display.setTextSize(1);
    display.setCursor(40, 44);
    display.println("40322797");
  }

  display.display();
}

void displayInvalid() {
  display.clearDisplay();

  // Set text size, color, and start position
  display.setTextSize(2); // Text size (1 is the smallest, increase as needed)
  display.setTextColor(SSD1306_WHITE); // Text color
  display.setCursor(24, 24);           // Starting position

  // Display text
  display.println("Invalid");

  // Send buffer to the display
  display.display();
}

String printHex(byte *buffer, byte bufferSize) {
  String content = "";
  for (byte i = 0; i < bufferSize; i++) {
    content.concat(String(buffer[i] < 0x10 ? " 0" : " "));
    content.concat(String(buffer[i], HEX));
  }
  return content;
}

void loop() {
  defaultDisplay();
  // Wait for RFID cards to be scanned
  if (!myRFID.PICC_IsNewCardPresent()) {

    return;
  }
  // an RFID card has been scanned but no UID
  if (!myRFID.PICC_ReadCardSerial()) {

    return;
  }
  // Show UID on serial monitor
  // Serial.print("USER ID tag :");
  String content = printHex(myRFID.uid.uidByte, myRFID.uid.size);
  String payload = createJSONPayload(content);
  String res = sendPostRequest(URL, payload);

  Serial.println("RES and CHECKINRES:");
  Serial.println(res);
  Serial.println(checkInRes);

  if (res.equals(checkInRes)) {
    displayValid();
    checkInOutput(1);
    Serial.println("VALID");
  } else if (res.equals(checkOutRes)) {
    displayValid();
    checkOutOutput(1);
    Serial.println("VALID");
  } else {
    displayInvalid();
    checkInOutput(0);
    Serial.println("INVALID");
  }
  delay(1500);
}