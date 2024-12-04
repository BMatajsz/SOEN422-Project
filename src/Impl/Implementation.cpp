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

struct ResponseData {
  String action;
  String name;
  String studentID;
};

const unsigned char GREEN_LED = 12;
const unsigned char RED_LED = 17;
const unsigned char BUZZER_PIN = 13;
const String CARD_UID = " 25 3c da 3f";
unsigned char FLAG = 0;

const char *SSID = "Imm.LeMontfort"; //"SOEN422";
// const char* password = "m2%a$S88"; //"nibbieking";

/* IPAddress local_IP(172, 30, 140, 172); // static

// Gateway IP address
IPAddress gateway(172, 30, 140, 129);

// Subnet Mask
IPAddress subnet(255, 255, 255, 128); */

const char *URL = "https://qa74pu7ut9.execute-api.us-east-1.amazonaws.com/"
                  "PostStage/SmartAttendance";
const String checkInRes = "check-in";
const String checkOutRes = "check-out";

// Initializes WiFi connection.
void initWiFi() {

  /* if (!WiFi.config(local_IP, gateway, subnet)) {
    Serial.println("Failed to configure Static IP");
  } */

  // Connect to Wi-Fi network
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(SSID /* , password */);

  // Wait for the Wi-Fi connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);

    Serial.print(WiFi.status());
  }

  Serial.println("\nWi-Fi connected successfully!");
  Serial.print("Assigned IP: ");
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

ResponseData sendPostRequest(const char *url, const String &jsonPayload) {
  ResponseData response = {"", "", ""}; // Initialize with empty values

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

      String jsonResponse = http.getString();
      Serial.println("Full Response:");
      Serial.println(jsonResponse);

      // Parse JSON response and populate the ResponseData struct
      response = parseJSON(jsonResponse);
    } else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
      displayHttpFail();
    }

    http.end(); // Free resources
  } else {
    Serial.println("Wi-Fi not connected");
  }

  return response;
}

ResponseData parseJSON(const String &jsonString) {
  StaticJsonDocument<500> doc;
  ResponseData response = {"", "", ""}; // Initialize with empty values

  // Deserialize the top-level JSON
  DeserializationError error = deserializeJson(doc, jsonString);

  if (error) {
    Serial.print("Top-level JSON parse failed: ");
    Serial.println(error.c_str());
    return response;
  }

  // Extract the body field as a nested JSON
  const char *body = doc["body"];

  if (body) {
    StaticJsonDocument<300> nestedDoc;

    // Parse the nested JSON in the body field
    DeserializationError nestedError = deserializeJson(nestedDoc, body);

    if (nestedError) {
      Serial.print("Nested JSON parse failed: ");
      Serial.println(nestedError.c_str());
      return response;
    }

    // Extract fields from the nested JSON
    response.action = nestedDoc["action"] | "";       // Extract "action"
    response.name = nestedDoc["name"] | "";           // Extract "name"
    response.studentID = nestedDoc["studentID"] | ""; // Extract "studentID"
  } else {
    Serial.println("No 'body' field found in JSON.");
  }

  return response;
}

void displayHttpFail() {
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);

  display.println("Request failed.");
  display.println("Please try agian.");

  display.display();
  delay(2000);
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
  display.println("Demo");
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

void displayValid(String name, String studentID, unsigned char flag) {
  display.clearDisplay();

  if (!flag) {
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(16, 10);
    display.println("Check-in");
    display.setTextSize(1);
    display.setCursor(24, 30);
    display.println(name);
    display.setTextSize(1);
    display.setCursor(40, 44);
    display.println(studentID);
  } else {
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(10, 10);
    display.println("Check-out");
    display.setTextSize(1);
    display.setCursor(24, 30);
    display.println(name);
    display.setTextSize(1);
    display.setCursor(40, 44);
    display.println(studentID);
  }

  display.display();
}

void displayInvalid(String action) {
  display.clearDisplay();

  // Set text size, color, and start position
  display.setTextSize(2); // Text size (1 is the smallest, increase as needed)
  display.setTextColor(SSD1306_WHITE); // Text color
  display.setCursor(24, 24);           // Starting position

  // Display text
  display.println("Invalid");

  // Send buffer to the display
  display.display();
  delay(1000);
  display.clearDisplay();

  // Set text size, color, and start position
  display.setTextSize(1); // Text size (1 is the smallest, increase as needed)
  display.setTextColor(SSD1306_WHITE); // Text color
  display.setCursor(0, 0);             // Starting position

  // Display text
  display.println(action);

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
  ResponseData res = sendPostRequest(URL, payload);

  Serial.println("RES:");
  Serial.println(res.action);
  Serial.println(res.name);
  Serial.println(res.studentID);

  if (res.action.equals(checkInRes)) {
    displayValid(res.name, res.studentID, 0);
    checkInOutput(1);
    Serial.println("VALID");
  } else if (res.action.equals(checkOutRes)) {
    displayValid(res.name, res.studentID, 1);
    checkOutOutput(1);
    Serial.println("VALID");
  } else {
    displayInvalid(res.action);
    checkInOutput(0);
    Serial.println("INVALID");
  }
  delay(1500);
}