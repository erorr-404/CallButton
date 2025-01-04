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

// ESP-NOW
uint8_t peerAddress[] = {0x8a, 0x1f, 0x9d, 0x1e, 0x1c};

// Function declarations
void sendRequest(const char *message);
void onReceive(const uint8_t *macAddr, const uint8_t *data, int len);
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
void playYes();
void playWait();
void playNo();

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
  player.volume(10);

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

  // Allow operations to block ESP for 21 seconds
  esp_task_wdt_init(21, true);
  Serial.println("ESP Task WDT initialized.");
}

void loop() {
  // Poll buttons
  comeBtnPressed = digitalRead(comeBtnPin) == LOW;
  dinnerBtnPressed = digitalRead(dinnerBtnPin) == LOW;
}

void sendRequest(const char *message) {
  Serial.println("Sending ESP-NOW request...");
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
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void playYes() {
  player.play(3);
  delay(1000);
}

void playWait() {
  player.play(2);
  delay(1000);
}

void playNo() {
  player.play(1);
  delay(1000);
}