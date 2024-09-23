#include <HTTPClient.h>

#include <WiFi.h>
#include <WiFiAP.h>
#include <WiFiClient.h>
#include <WiFiGeneric.h>
#include <WiFiMulti.h>
#include <WiFiScan.h>
#include <WiFiServer.h>
#include <WiFiSTA.h>
#include <WiFiType.h>
#include <WiFiUdp.h>

#include <SPI.h>

#include <SD.h>
#include <sd_defines.h>
#include <sd_diskio.h>

#include <Adafruit_NeoPixel.h>
#include <M5Cardputer.h>

#include "commandInterpreter.h"
#include "sdCard.h"

#define SD_SPI_SCK_PIN  40
#define SD_SPI_MISO_PIN 39
#define SD_SPI_MOSI_PIN 14
#define SD_SPI_CS_PIN   12

// config file things
bool initWifiOnBoot = false;
bool clearInitLogs = false;
//bool syncTime = false;
bool passwordEnabled = false;
String wifiSSID = "";
String wifiPass = "";
String fgColor = "";
String password = "";

bool sdAvailable;
String currentDir = "";
String enteredPassword = "";
String prompt = "";
int cursorY = 0;
long randNumber;
unsigned long lastKeyPressMillis = 0;
const unsigned long debounceDelay = 200;
const unsigned long timeUpdateInterval = 1000;
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(1, 21, NEO_GRB + NEO_KHZ800);

int charWidth;
int charHeight;

void setup() {
  auto cfg = M5.config();
  M5Cardputer.begin(cfg, true);
  M5Cardputer.Display.setRotation(1);
  M5Cardputer.Display.setTextSize(1.25);
  M5Cardputer.Display.fillScreen(BLACK);
  M5Cardputer.Display.setTextColor(WHITE);
  M5Cardputer.Display.setTextScroll(true);

  M5Cardputer.Speaker.tone(1000, 300);
  delay(300);

  M5Cardputer.Display.println("[sys] Starting Amber...");

  randomSeed(analogRead(0));

  SPI.begin(SD_SPI_SCK_PIN, SD_SPI_MISO_PIN, SD_SPI_MOSI_PIN, SD_SPI_CS_PIN);

  if (!SD.begin(SD_SPI_CS_PIN, SPI, 25000000)) {
    M5Cardputer.Display.println("[sd] SD card not found!");
    sdAvailable = false;
    M5Cardputer.Display.println("[sys] Welcome to Amber!");
    delay(500);
    M5Cardputer.Display.println("");
    M5Cardputer.Display.print("M5> ");
    return;
  }

  uint8_t cardType = SD.cardType();
  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  M5Cardputer.Display.println("[sd] SD initialized!");
  sdAvailable = true;
  currentDir = "/";

  readConfig();

  if (fgColor == "WHITE" || fgColor == "white") {
    M5Cardputer.Display.setTextColor(WHITE);
  } else if (fgColor == "RED" || fgColor == "red") {
    M5Cardputer.Display.setTextColor(RED);
  } else if (fgColor == "GREEN" || fgColor == "green") {
    M5Cardputer.Display.setTextColor(GREEN);
  } else if (fgColor == "BLUE" || fgColor == "blue") {
    M5Cardputer.Display.setTextColor(BLUE);
  } else if (fgColor == "YELLOW" || fgColor == "yellow") {
    M5Cardputer.Display.setTextColor(YELLOW);
  } else if (fgColor == "CYAN" || fgColor == "cyan") {
    M5Cardputer.Display.setTextColor(CYAN);
  } else if (fgColor == "PURPLE" || fgColor == "purple") {
    M5Cardputer.Display.setTextColor(PURPLE);
  } else if (fgColor == "ORANGE" || fgColor == "orange") {
    M5Cardputer.Display.setTextColor(ORANGE);
  } else if (fgColor == "pink" || fgColor == "pink") {
    M5Cardputer.Display.setTextColor(PINK);
  } else {
    M5Cardputer.Display.println("[err] Invalid color, reverting to white");
    M5Cardputer.Display.setTextColor(WHITE);
  }

  if (initWifiOnBoot) {
    WifiInit(wifiSSID, wifiPass);
  }

  if (passwordEnabled) {
    M5Cardputer.Display.print("[lock] Enter password: ");
    while (true) {
      M5Cardputer.update();

      if (M5Cardputer.Keyboard.isChange() && M5Cardputer.Keyboard.isPressed()) {
        unsigned long currentMillis = millis();
        if (currentMillis - lastKeyPressMillis >= debounceDelay) {
          lastKeyPressMillis               = currentMillis;
          Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();

          for (auto i : status.word) {
            enteredPassword += i;
            M5Cardputer.Display.print("*");
          }

          if (status.enter) {
            if (enteredPassword == password) {
              M5Cardputer.Display.println("");
              break;
            } else {
              delay(1000);
              M5Cardputer.Display.println("");
              M5Cardputer.Display.println("[lock] Incorrect password, try again!");
              M5Cardputer.Display.print(": ");
              enteredPassword = "";
            }
          }

          if (status.del && enteredPassword.length() > 0) {
            enteredPassword.remove(enteredPassword.length() - 1);
            M5Cardputer.Display.setCursor(M5Cardputer.Display.getCursorX() - 6, M5Cardputer.Display.getCursorY());
            M5Cardputer.Display.fillRect(M5Cardputer.Display.getCursorX(), M5Cardputer.Display.getCursorY(), 7, 10, BLACK);
            M5Cardputer.Display.setCursor(M5Cardputer.Display.getCursorX() - 6, M5Cardputer.Display.getCursorY());
          }
        }
      }
    }
  }

  M5Cardputer.Display.println("Welcome to Amber!");

  delay(500);

  cursorY = M5Cardputer.Display.getCursorY();
  if (!clearInitLogs) {
    M5Cardputer.Display.println("");
    M5Cardputer.Display.print("M5> ");
  } else {
    M5Cardputer.Display.clear();
    M5Cardputer.Display.setCursor(0, 0);
    M5Cardputer.Display.println("");
    M5Cardputer.Display.print("M5> ");
  }

  charWidth = M5Cardputer.Display.textWidth("O");
  charHeight = M5Cardputer.Display.fontHeight();

  // Initialize the NeoPixel
  pixels.begin();
  pixels.show();  // Initialize all pixels to 'off'
}

void loop() {
  M5Cardputer.update();

  if (M5Cardputer.Keyboard.isChange() && M5Cardputer.Keyboard.isPressed()) {
    unsigned long currentMillis = millis();
    if (currentMillis - lastKeyPressMillis >= debounceDelay) {
      lastKeyPressMillis               = currentMillis;
      Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();

      for (auto i : status.word) {
        prompt += i;
        M5Cardputer.Display.print(i);
        cursorY = M5Cardputer.Display.getCursorY();
      }

      if (status.enter) {
        runCommand(prompt);
        prompt = "";
        M5Cardputer.Display.println("");
        M5Cardputer.Display.print("M5> ");
      }

      if (status.del && prompt.length() > 0) {
        prompt.remove(prompt.length() - 1);
        M5Cardputer.Display.setCursor(M5Cardputer.Display.getCursorX() - 3, M5Cardputer.Display.getCursorY());
        M5Cardputer.Display.fillRect(M5Cardputer.Display.getCursorX(), M5Cardputer.Display.getCursorY(), 7, 10, BLACK);
        M5Cardputer.Display.setCursor(M5Cardputer.Display.getCursorX() - 3, M5Cardputer.Display.getCursorY());
      }
    }
  }
}
