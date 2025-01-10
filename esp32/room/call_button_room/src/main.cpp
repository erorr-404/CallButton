#include <DFRobotDFPlayerMini.h>
#include <esp_now.h>
#include "Arduino.h"
#include <WiFi.h>
#include "esp_task_wdt.h"

// DFPlayer serial
#define FPSerial Serial1
DFRobotDFPlayerMini player;

// LED
const int ledPin = 2;
const int activeLedPin = 4;

// Buttons
const int acceptBtnPin = 22;
const int waitBtnPin = 19;
const int rejectBtnPin = 21;

// Button states
bool acceptBtnPressed = false;
bool waitBtnPressed = false;
bool rejectBtnPressed = false;

// ESP-NOW
uint8_t peerAddress[] = {0x3C, 0x8A, 0x1F, 0x9B, 0xCD, 0x00};

void onReceive(const uint8_t *macAddr, const uint8_t *data, int len);
void sendResponse(const char *message);
void playComeHere();
void playDinnerReady();
const char *waitForButton();
void printDetail(uint8_t type, int value);

void setup() {
  // Initialize serial for debugging and DFPlayer Mini communication
  Serial.begin(115200);
  FPSerial.begin(9600, SERIAL_8N1, /*rx =*/16, /*tx =*/17);

  // Initialize LED
  pinMode(ledPin, OUTPUT);
  pinMode(activeLedPin, OUTPUT);

  // Initialize DFPlayer Mini
  Serial.println(F("Initializing DFPlayer ... (May take 3~5 seconds)"));
  if (!player.begin(FPSerial, /*isACK = */false, /*doReset = */false)) {  //Use serial to communicate with mp3.
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    while(true){
      digitalWrite(ledPin, HIGH);
      delay(100);
      digitalWrite(ledPin, LOW);
      delay(100);
    }
  }

  // Wait while DFPlayer is available
  while (!player.available()) {
    delay(1000);
    Serial.println("DFPlayer is not available.");
  }
  
  Serial.println(F("DFPlayer Mini online."));
  player.volume(10);

  // Initialize buttons
  pinMode(acceptBtnPin, INPUT_PULLUP);
  pinMode(waitBtnPin, INPUT_PULLUP);
  pinMode(rejectBtnPin, INPUT_PULLUP);

  // Initialize Wi-Fi in station mode
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

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

  // Allow operations to block ESP for 16 seconds
  esp_task_wdt_init(16, true);

  // Wait 2 seconds to avoid bugs and problems
  sleep(2000);

  // TODO: imitate onReceive to avoid bug
  // onReceive(peerAddress, (uint8_t *)"test", strlen("test"));

  // Activate LED to make user understand device is ready to use
  digitalWrite(activeLedPin, HIGH);
}

void loop() {
  // Poll buttons
  acceptBtnPressed = digitalRead(acceptBtnPin) == LOW;
  waitBtnPressed = digitalRead(waitBtnPin) == LOW;
  rejectBtnPressed = digitalRead(rejectBtnPin) == LOW;
}

// void onReceive(const uint8_t *macAddr, const uint8_t *data, int len);
void onReceive(const uint8_t *macAddr, const uint8_t *data, int len) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
         macAddr[0], macAddr[1], macAddr[2],
         macAddr[3], macAddr[4], macAddr[5]);

  Serial.print("Received message from: ");
  Serial.println(macStr);

  String message = "";
  for (int i = 0; i < len; i++) {
    message += (char)data[i];
  }

  Serial.print("Message: ");
  Serial.println(message);

  digitalWrite(ledPin, HIGH);

  if (message == "dinner") {
    playDinnerReady();
    const char *response = waitForButton();
    sendResponse(response);
  } else if (message == "come") {
    playComeHere();
    const char *response = waitForButton();
    sendResponse(response);
  } else if (message == "test") {
    playComeHere();
    playDinnerReady();
    const char *response = waitForButton();
    sendResponse(response);
  } else if (message == "end") {
    Serial.println("End command received.");
  }

  digitalWrite(ledPin, LOW);
}


void sendResponse(const char *message) {
  esp_now_send(peerAddress, (uint8_t *)message, strlen(message));
}


const char *waitForButton() {
  Serial.println("Waiting for button press...");
  unsigned long startTime = millis();

  while (millis() - startTime < 15000) { // 15 seconds timeout
    if (digitalRead(acceptBtnPin) == LOW) {
      Serial.println("User accepted request");
      return "yes";
    }
    if (digitalRead(waitBtnPin) == LOW) {
      Serial.println("User delayed request");
      return "wait";
    }
    if (digitalRead(rejectBtnPin) == LOW) {
      Serial.println("User rejected request");
      return "no";
    }

    yield(); // For deblocking
  }

  Serial.println("Timeout waiting for button");
  return "none";
}

void playComeHere() {
  player.play(1);
  delay(1000);
}

void playDinnerReady() {
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
      // Inform user that there is an error
      digitalWrite(activeLedPin, LOW);
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
