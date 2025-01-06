#include <Arduino.h>
#include <DFRobotDFPlayerMini.h>
#include <esp_now.h>
#include <WiFi.h>
#include "esp_task_wdt.h"

// DFPlayer serial
#define FPSerial Serial1
DFRobotDFPlayerMini player;

// LED
const int ledPin = 2;

// Buttons
const int dinnerBtnPin = 4;
const int comeBtnPin = 15;

// Button states
bool dinnerBtnPressed = false;
bool comeBtnPressed = false;

bool dinnerBtnPrev = false;
bool comeBtnPrev = false;

// ESP-NOW
uint8_t peerAddress[] = {0x3c, 0x8a, 0x1f, 0x9d, 0x1e, 0x1c};
bool waitingResponse = false;

// Function declarations
void sendRequest(const char *message);
void onReceive(const uint8_t *macAddr, const uint8_t *data, int len);
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
void playYes();
void playWait();
void playNo();
void printDetail(uint8_t type, int value);

void setup() {
  // Initialize serial for debugging and DFPlayer Mini communication
  Serial.begin(115200);
  FPSerial.begin(9600, SERIAL_8N1, /*rx =*/16, /*tx =*/17);

  // Initialize DFPlayer Mini
  Serial.println(F("Initializing DFPlayer ... (May take 3~5 seconds)"));
  if (!player.begin(FPSerial, /*isACK = */true, /*doReset = */false)) {  //Use serial to communicate with mp3.
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    while(true){
      delay(0); // Code to compatible with ESP8266 watch dog.
    }
  }
  Serial.println(F("DFPlayer Mini online."));
  player.volume(30);

  // Initialize LED
  pinMode(ledPin, OUTPUT);
  Serial.println("Led initialized.");

  // Initialize buttons
  pinMode(dinnerBtnPin, INPUT_PULLUP);
  pinMode(comeBtnPin, INPUT_PULLUP);
  Serial.println("Buttons initialized.");

  // Initialize Wi-Fi in station mode
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  Serial.println("ESP-NOW initialized.");

  // Register peer
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, peerAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
  Serial.println("Peer successfully added");

  // Register callback for receiving data
  esp_now_register_recv_cb(onReceive);
  esp_now_register_send_cb(OnDataSent);

  // Allow operations to block ESP for 21 seconds
  esp_task_wdt_init(21, true);
  Serial.println("ESP Task WDT initialized.");
}

void loop() {
  // Poll buttons
  comeBtnPressed = digitalRead(comeBtnPin) == LOW;
  dinnerBtnPressed = digitalRead(dinnerBtnPin) == LOW;

  // Come button action
  if (comeBtnPressed && !comeBtnPrev) {
    unsigned long int pressTime = millis();
    Serial.println("Come button pressed.");
    sendRequest("come");
    digitalWrite(ledPin, HIGH);
    
    // Wait 20 seconds for request
    while (millis() - pressTime < 20000) {
      if (waitingResponse){
        delay(0);
      } else {
        break;
      }
    }
    
    Serial.println("20-seconds have been passed.");
    waitingResponse = false;
    digitalWrite(ledPin, LOW);
  }

  // Dinner button action
  if (dinnerBtnPressed && !dinnerBtnPrev) {
    unsigned long int pressTime = millis();
    Serial.println("Dinner button pressed.");
    sendRequest("dinner");
    digitalWrite(ledPin, HIGH);
   
    // Wait 20 seconds for request
    while (millis() - pressTime < 20000) {
      if (waitingResponse){
        delay(0);
      } else {
        break;
      }
    }

    Serial.println("20-seconds have been passed.");
    waitingResponse = false;
    digitalWrite(ledPin, LOW);
  }

  // Save button states
  comeBtnPrev = comeBtnPressed;
  dinnerBtnPrev = dinnerBtnPressed;

  // Print DFPlayer status
  if (player.available()) {
    printDetail(player.readType(), player.read()); //Print the detail message from DFPlayer to handle different errors and states.
  }
}

// Send message to second device
void sendRequest(const char *message) {
  Serial.println("Sending ESP-NOW request...");
  waitingResponse = true;
  esp_now_send(peerAddress, (uint8_t *)message, strlen(message));
}

void onReceive(const uint8_t *macAddr, const uint8_t *data, int len) {
  // Get MAC-address
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
         macAddr[0], macAddr[1], macAddr[2],
         macAddr[3], macAddr[4], macAddr[5]);

  Serial.print("Received message from: ");
  Serial.println(macStr);

  // Get received message
  String message = "";
  for (int i = 0; i < len; i++) {
    message += (char)data[i];
  }

  Serial.print("Message: ");
  Serial.println(message);

  if (message == "yes") {
    Serial.println("Playing yes...");
    playYes();
  } else if (message == "wait") {
    Serial.println("Playing wait...");
    playWait();
  } else if (message == "no") {
    Serial.println("Playing no...");
    playNo();
  } else {
    Serial.println("Wrong messaage.");
  }

  waitingResponse = false;
  digitalWrite(ledPin, LOW);
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void playYes() {
  player.play(1);
  delay(1000);
}

void playWait() {
  player.play(3);
  delay(1000);
}

void playNo() {
  player.play(2);
  delay(1000);
}

void printDetail(uint8_t type, int value){
  switch (type) {
    case TimeOut:
      Serial.println(F("Time Out!"));
      break;
    case WrongStack:
      Serial.println(F("Stack Wrong!"));
      break;
    case DFPlayerCardInserted:
      Serial.println(F("Card Inserted!"));
      break;
    case DFPlayerCardRemoved:
      Serial.println(F("Card Removed!"));
      break;
    case DFPlayerCardOnline:
      Serial.println(F("Card Online!"));
      break;
    case DFPlayerUSBInserted:
      Serial.println("USB Inserted!");
      break;
    case DFPlayerUSBRemoved:
      Serial.println("USB Removed!");
      break;
    case DFPlayerPlayFinished:
      Serial.print(F("Number:"));
      Serial.print(value);
      Serial.println(F(" Play Finished!"));
      break;
    case DFPlayerError:
      Serial.print(F("DFPlayerError:"));
      switch (value) {
        case Busy:
          Serial.println(F("Card not found"));
          break;
        case Sleeping:
          Serial.println(F("Sleeping"));
          break;
        case SerialWrongStack:
          Serial.println(F("Get Wrong Stack"));
          break;
        case CheckSumNotMatch:
          Serial.println(F("Check Sum Not Match"));
          break;
        case FileIndexOut:
          Serial.println(F("File Index Out of Bound"));
          break;
        case FileMismatch:
          Serial.println(F("Cannot Find File"));
          break;
        case Advertise:
          Serial.println(F("In Advertise"));
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
  
}