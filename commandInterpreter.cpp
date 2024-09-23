// functions for commands to work

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

#include "sdCard.h"
#include "commandInterpreter.h"

extern bool initWifiOnBoot;
extern bool passwordEnabled;
extern bool clearInitLogs;
extern bool syncTime;
extern String wifiSSID;
extern String wifiPass;
extern String fgColor;
extern String password;

extern Adafruit_NeoPixel pixels;
extern int charWidth, charHeight;
extern String currentDir;
extern bool sdAvailable;
extern long randNumber;

extern unsigned long lastKeyPressMillis;
extern const unsigned long debounceDelay;

std::vector<String> split(String str, char delimiter) {
  std::vector<String> result;
  String token = "";
  for (int i = 0; i < str.length(); i++) {
    if (str[i] == delimiter) {
      if (token != "") {
        result.push_back(token);
        token = "";
      }
    } else {
      token += str[i];
    }
  }
  if (token != "") {
    result.push_back(token);
  }
  return result;
}

void runCommand(String input) {
  input.trim();
  std::vector<String> parts = split(input, ' ');

  if (parts.size() == 0) {
    return;
  }

  String command = parts[0];
  std::vector<String> args(parts.begin() + 1, parts.end());

  if (command == "charinfo") {
    handleCharInfo(args);
  } else if (command == "ledcolor") {
    handleLedColor(args);
  } else if (command == "help") {
    handleHelp(args);
  } else if (command == "print") {
    handlePrint(args);
  } else if (command == "sound") {
    handleSound(args);
  } else if (command == "calculate") {
    handleCalculate(args);
  } else if (command == "ls") {
    handleListDir(args);
  } else if (command == "pwd") {
    handleCurrentDir(args);
  } else if (command == "syscfg") {
    handleSysCfg(args);
  } else if (command == "wifiinit") {
    handleWifiInit(args);
  } else if (command == "wifidisconnect") {
    handleWifiDisconnect(args);
  } else if (command == "about") {
    handleAbout(args);
  } else if (command == "clear") {
    handleClear(args);
  } else if (command == "syscfgprint") {
    handleSysCfgPrint(args);
  } else if (command == "cat") {
    handleCat(args);
  } else if (command == "cd") {
    handleChangeDir(args);
  } else if (command == "mkdir") {
    handleMkDir(args);
  } else if (command == "rmdir") {
    handleRmDir(args);
  } else if (command == "touch") {
    handleNewFile(args);
  } else if (command == "rm") {
    handleRemoveFile(args);
  } else if (command == "wifistatus") {
    handleWifiStatus(args);
  } else if (command == "roll") {
    handleRoll(args);
  } else if (command == "matrix") {
    handleMatrix(args);
  } else {
    M5Cardputer.Display.println("");
    M5Cardputer.Display.println("ambersh: command not found: " + command);
  }
}

void handleCharInfo(std::vector<String> args) {
  // charinfo doesn't need parameters, just show character info
  M5Cardputer.Display.println("");
  M5Cardputer.Display.println("Char Width: " + String(charWidth));
  M5Cardputer.Display.println("Char Height: " + String(charHeight));
  M5Cardputer.Display.println("Cursor X: " + String(M5Cardputer.Display.getCursorX()));
  M5Cardputer.Display.println("Cursor Y: " + String(M5Cardputer.Display.getCursorY()));
}

void handleLedColor(std::vector<String> args) {
  if (args.size() != 3) {
    M5Cardputer.Display.println("");
    M5Cardputer.Display.println("Usage: ledcolor <r> <g> <b>");
    return;
  }

  int r = args[0].toInt();
  int g = args[1].toInt();
  int b = args[2].toInt();

  pixels.setPixelColor(0, pixels.Color(r, g, b));
  pixels.show();
  M5Cardputer.Display.println("");

}

void handleRoll(std::vector<String> args) {
  if (args.size() != 2) {
    randNumber = random(1, 7);

    M5Cardputer.Display.println("");
    M5Cardputer.Display.println(randNumber);
    return;
  }

  int nummin = args[0].toInt();
  int nummax = args[1].toInt();

  randNumber = random(nummin, nummax);

  M5Cardputer.Display.println("");
  M5Cardputer.Display.println(randNumber);

}

void handleMatrix(std::vector<String> args) {
  M5Cardputer.Display.clear();
  M5Cardputer.Display.setCursor(0, 0);

  while (true) {
    M5Cardputer.update();

    randNumber = random(1, 10);

    M5Cardputer.Display.print(randNumber);

    if (M5Cardputer.BtnA.wasPressed()) {
      M5Cardputer.Display.clear();
      M5Cardputer.Display.setCursor(0, 0);
      break;
    }
  }
}


void handleAbout(std::vector<String> args) {
  M5Cardputer.Display.println("");
  M5Cardputer.Display.println("   ___         __          ");
  M5Cardputer.Display.println("  / _ | __ _  / /  ___ ____");
  M5Cardputer.Display.println(" / __ |/  ' \/ _ \/ -_) __/");
  M5Cardputer.Display.println("/_/ |_/_/_/_/_.__/\__/_/   ");
  M5Cardputer.Display.println("Command Line Interface with unix-like commands for the M5Stack Cardputer");
  M5Cardputer.Display.println("Maintained and developed by https://github.com/qubiX00");
  M5Cardputer.Display.println("v.beta1909");
}

void handleListDir(std::vector<String> args) {
  if (!sdAvailable) {
    M5Cardputer.Display.println("");
    M5Cardputer.Display.println("Error: SD Card is not available");
    return;
  }

  M5Cardputer.Display.println("");
  listDir(SD, currentDir, 0);
}

void handleMkDir(std::vector<String> args) {
  if (!sdAvailable) {
    M5Cardputer.Display.println("");
    M5Cardputer.Display.println("Error: SD Card is not available");
    return;
  }

  if (args.empty()) {
    M5Cardputer.Display.println("");
    M5Cardputer.Display.println("Usage: 'mkdir <path>'");
    return;
  }

  String input = args[0];
  createDir(SD, input);
}

void handleRemoveFile(std::vector<String> args) {
  if (!sdAvailable) {
    M5Cardputer.Display.println("");
    M5Cardputer.Display.println("Error: SD Card is not available");
    return;
  }

  if (args.empty()) {
    M5Cardputer.Display.println("");
    M5Cardputer.Display.println("Usage: 'rf <path>'");
    return;
  }

  String input = args[0];
  deleteFile(SD, input);
}

void handleRmDir(std::vector<String> args) {
  if (!sdAvailable) {
    M5Cardputer.Display.println("");
    M5Cardputer.Display.println("Error: SD Card is not available");
    return;
  }

  if (args.empty()) {
    M5Cardputer.Display.println("");
    M5Cardputer.Display.println("Usage: 'rmdir <path>'");
    return;
  }

  String input = args[0];
  M5Cardputer.Display.println("");
  removeDir(SD, input);
}

void handleNewFile(std::vector<String> args) {
  if (!sdAvailable) {
    M5Cardputer.Display.println("");
    M5Cardputer.Display.println("Error: SD Card is not available");
    return;
  }

  if (args.empty()) {
    M5Cardputer.Display.println("");
    M5Cardputer.Display.println("Usage: 'nf <path>'");
    return;
  }

  String input = args[0];
  M5Cardputer.Display.println("");
  createFile(SD, input);
}

void handleChangeDir(std::vector<String> args) {
  if (!sdAvailable) {
    M5Cardputer.Display.println("");
    M5Cardputer.Display.println("Error: SD Card is not available");
    return;
  }

  if (args.empty()) {
    M5Cardputer.Display.println("");
    M5Cardputer.Display.println("Set directory to /");
    currentDir = "/";
    return;
  }

  String input = args[0];

  File dir = SD.open(input);

  if (dir && dir.isDirectory()) {
    currentDir = input;
    M5Cardputer.Display.println("");
    M5Cardputer.Display.print("Set directory to ");
    M5Cardputer.Display.println(currentDir);
    dir.close();
  } else {
    M5Cardputer.Display.println("");
    M5Cardputer.Display.println("Error: Directory does not exist");
    if (dir) dir.close();
  }
}

void handleCurrentDir(std::vector<String> args) {
  if (!sdAvailable) {
    M5Cardputer.Display.println("");
    M5Cardputer.Display.println("Error: SD Card is not available");
    return;
  }
  M5Cardputer.Display.println("");
  M5Cardputer.Display.println(currentDir);
}


void handleCalculate(std::vector<String> args) {
  if (args.size() != 3) {
    M5Cardputer.Display.println("");
    M5Cardputer.Display.println("Usage: 'calculate <1st num> <op (+, -, *, /)> <2nd num>'");
    return;
  }

  int first = args[0].toInt();
  char op = args[1][0];
  int second = args[2].toInt();
  String output;

  switch (op) {
    case '+':
      output = String(first + second);
      M5Cardputer.Display.println(" = " + output);
      break;

    case '-':
      output = String(first - second);
      M5Cardputer.Display.println(" = " + output);
      break;

    case '*':
      output = String(first * second);
      M5Cardputer.Display.println(" = " + output);
      break;

    case '/':
      if (second == 0) {
        M5Cardputer.Display.println("Error: Division by 0");
      } else {
        output = String((float)first / second);
        M5Cardputer.Display.println(" = " + output);
      }
      break;

    default:
      M5Cardputer.Display.println("");
      M5Cardputer.Display.println("Error: Unsupported operator (please use these: +, -, *, /)");
      break;
  }
}


void handleSound(std::vector<String> args) {
  if (args.size() != 2) {
    M5Cardputer.Display.println("");
    M5Cardputer.Display.println("Usage: sound <freq> <length>");
    return;
  }

  int freq = args[0].toInt();
  int soundLength = args[1].toInt();

  M5Cardputer.Speaker.tone(freq, soundLength);
  M5Cardputer.Display.println("");
  M5Cardputer.Display.println("Beep!");
  delay(soundLength);
}

void handleClear(std::vector<String> args) {
  M5Cardputer.Display.clear();
  M5Cardputer.Display.setCursor(0, 0);
}

void handlePrint(std::vector<String> args) {
  if (args.size() == 0) {
    M5Cardputer.Display.println("");
    M5Cardputer.Display.println("Usage: 'print <text>'");
    return;
  }

  String input = args[0];
  M5Cardputer.Display.println("");
  M5Cardputer.Display.println(input);
}

void handleSysCfg(std::vector<String> args) {
  if (!sdAvailable) {
    M5Cardputer.Display.println("");
    M5Cardputer.Display.println("Error: SD Card is not available");
    return;
  } else {
    if (args.size() == 0) {
      M5Cardputer.Display.println("");
      M5Cardputer.Display.println("Options:");
      M5Cardputer.Display.println("'initWifiOnBoot' (by default set to false)");
      M5Cardputer.Display.println("'WifiSSID' (by default set to defaultSSID)");
      M5Cardputer.Display.println("'WifiPass' (by default set to defaultPass)");
      M5Cardputer.Display.println("'clearInitLogs' (by default set to false)");
      M5Cardputer.Display.println("'fgColor' (by default set to WHITE)");
      //M5Cardputer.Display.println("'syncTime' (only works when initWifiOnBoot is true, by default set to true)");
      M5Cardputer.Display.println("'passwordEnabled' (by default set to false)");
      M5Cardputer.Display.println("'password' (by default set to nothing)");
      M5Cardputer.Display.println("'Usage: 'syscfg <option> <value>'");
      return;
    }


    String option = args[0];
    String value = args[1];

    if (option == "fgColor") {
      if (value == "WHITE" || value == "white") {
        M5Cardputer.Display.setTextColor(WHITE);
        updateConfig(option, value);
      } else if (value == "RED" || value == "red") {
        M5Cardputer.Display.setTextColor(RED);
        updateConfig(option, value);
      } else if (value == "GREEN" || value == "green") {
        M5Cardputer.Display.setTextColor(GREEN);
        updateConfig(option, value);
      } else if (value == "BLUE" || value == "blue") {
        M5Cardputer.Display.setTextColor(BLUE);
        updateConfig(option, value);
      } else if (value == "YELLOW" || value == "yellow") {
        M5Cardputer.Display.setTextColor(YELLOW);
        updateConfig(option, value);
      } else if (value == "CYAN" || value == "cyan") {
        M5Cardputer.Display.setTextColor(CYAN);
        updateConfig(option, value);
      } else if (value == "PURPLE" || value == "purple") {
        M5Cardputer.Display.setTextColor(PURPLE);
        updateConfig(option, value);
      } else if (value == "ORANGE" || value == "orange") {
        M5Cardputer.Display.setTextColor(ORANGE);
        updateConfig(option, value);
      } else if (value == "pink" || value == "pink") {
        M5Cardputer.Display.setTextColor(PINK);
        updateConfig(option, value);
      } else {
        M5Cardputer.Display.println("Error: Invalid color, available colors: white, red, green, blue, yellow, cyan, purple, orange, pink");
      }
    } else {
      updateConfig(option, value);
    }
  }
}
void handleSysCfgPrint(std::vector<String> args) {
  if (!sdAvailable) {
    M5Cardputer.Display.println("");
    M5Cardputer.Display.println("Error: SD Card is not available");
    return;
  } else {
    M5Cardputer.Display.println("");
    M5Cardputer.Display.print("initWifiOnBoot: ");
    if (initWifiOnBoot) {
      M5Cardputer.Display.println("true");
    } else if (!initWifiOnBoot) {
      M5Cardputer.Display.println("false");
    }
    M5Cardputer.Display.print("WifiSSID: ");
    M5Cardputer.Display.println(wifiSSID);
    M5Cardputer.Display.print("WifiPass: ");
    M5Cardputer.Display.println(wifiPass);
    M5Cardputer.Display.print("ClearInitLogs: ");
    if (clearInitLogs) {
      M5Cardputer.Display.println("true");
    } else if (!clearInitLogs) {
      M5Cardputer.Display.println("false");
    }
    M5Cardputer.Display.print("fgColor: ");
    M5Cardputer.Display.println(fgColor);
    ///M5Cardputer.Display.print("syncTime: ");
    //if (syncTime) {
    //  M5Cardputer.Display.println("true");
    //} else if (!syncTime) {
    //  M5Cardputer.Display.println("false");
    //}
    M5Cardputer.Display.print("passwordEnabled: ");
    if (passwordEnabled) {
      M5Cardputer.Display.println("true");
    } else if (!passwordEnabled) {
      M5Cardputer.Display.println("false");
    }
    M5Cardputer.Display.print("password: ");
    M5Cardputer.Display.println("-");
  }
}


void handleCat(std::vector<String> args) {
  if (!sdAvailable) {
    M5Cardputer.Display.println("");
    M5Cardputer.Display.println("Error: SD Card is not available");
    return;
  } else {
    if (args.size() == 0) {
      M5Cardputer.Display.println("");
      M5Cardputer.Display.println("Usage: cat <file>");
      return;
    }

    String input = args[0];
    displayFileContent(input);
  }
}

void handleHelp(std::vector<String> args) {
  if (args.size() == 0) {
    M5Cardputer.Display.println("");
    M5Cardputer.Display.println("Commands available:");
    M5Cardputer.Display.println("help, charinfo, ledcolor, print, sound, calculate, ls, pwd, syscfg, syscfgprint, about, clear, cat, cd, mkdir, rmdir, touch, rm, roll, matrix");
    return;
  }

  String cmd = args[0];
  if (cmd == "help") {
    M5Cardputer.Display.println("");
    M5Cardputer.Display.println("Displays all commands and detailed information about certain command");
    M5Cardputer.Display.println("Usage: 'help' to display all commands, 'help <cmd>' to display detailed information about certain command");
  } else if (cmd == "charinfo") {
    M5Cardputer.Display.println("");
    M5Cardputer.Display.println("Displays information about characters for debugging");
  } else if (cmd == "ledcolor") {
    M5Cardputer.Display.println("");
    M5Cardputer.Display.println("Changes the color of the LED light");
    M5Cardputer.Display.println("Usage: 'ledcolor <r> <g> <b>' to control the color of the LED light");
  } else if (cmd == "print") {
    M5Cardputer.Display.println("");
    M5Cardputer.Display.println("Prints text to the screen");
    M5Cardputer.Display.println("Usage: 'print <text>'");
  } else if (cmd == "sound") {
    M5Cardputer.Display.println("");
    M5Cardputer.Display.println("Plays a sound");
    M5Cardputer.Display.println("Usage: 'sound <freq> <length>'");
  } else if (cmd == "calculate") {
    M5Cardputer.Display.println("");
    M5Cardputer.Display.println("Solves a basic math problem");
    M5Cardputer.Display.println("Usage: 'calculate <1st num> <op (+, -, *, /)> <2nd num>'");
  } else if (cmd == "ls") {
    M5Cardputer.Display.println("");
    M5Cardputer.Display.println("Lists all files in current directory");
    M5Cardputer.Display.println("Usage: 'ls <path>'");
  } else if (cmd == "pwd") {
    M5Cardputer.Display.println("");
    M5Cardputer.Display.println("Prints current directory");
    M5Cardputer.Display.println("Usage: 'pwd'");
  } else if (cmd == "syscfg") {
    M5Cardputer.Display.println("");
    M5Cardputer.Display.println("Command for managing amber's configuration file");
    M5Cardputer.Display.println("Usage: 'syscfg' to print all the available options, 'syscfg <option> <value>' to modify an option");
  } else if (cmd == "clear") {
    M5Cardputer.Display.println("");
    M5Cardputer.Display.println("Clears the screen");
  } else if (cmd == "mkdir") {
    M5Cardputer.Display.println("");
    M5Cardputer.Display.println("Creates a new directory");
    M5Cardputer.Display.println("Usage: 'mkdir <path>'");
  } else if (cmd == "rmdir") {
    M5Cardputer.Display.println("");
    M5Cardputer.Display.println("Removes an existing directory");
    M5Cardputer.Display.println("Usage: 'rmdir <path>'");
  } else if (cmd == "touch") {
    M5Cardputer.Display.println("");
    M5Cardputer.Display.println("Creates a new file");
    M5Cardputer.Display.println("Usage: 'touch <path>'");
  } else if (cmd == "cat") {
    M5Cardputer.Display.println("");
    M5Cardputer.Display.println("Prints contents of a file");
    M5Cardputer.Display.println("Usage: 'cat <path>'");
  } else if (cmd == "cd") {
    M5Cardputer.Display.println("");
    M5Cardputer.Display.println("Changes current directory (current directory can be checked with 'pwd')");
    M5Cardputer.Display.println("Usage: 'cd <path>'");
  } else if (cmd == "rm") {
    M5Cardputer.Display.println("");
    M5Cardputer.Display.println("Removes an existing file");
    M5Cardputer.Display.println("Usage: 'rm <path>'");
  } else if (cmd == "roll") {
    M5Cardputer.Display.println("");
    M5Cardputer.Display.println("Generates a random number");
    M5Cardputer.Display.println("Usage: roll <min number> <max number>");
  } else if (cmd == "matrix") {
    M5Cardputer.Display.println("");
    M5Cardputer.Display.println("Cool little animation i guess");
    M5Cardputer.Display.println("Usage: matrix");
  } else {
    M5Cardputer.Display.println("");
    M5Cardputer.Display.println("Unknown command: " + cmd);
  }
}

// not related to command interpretation but idc lol
void displayFileContent(String path) {
  File file = SD.open(path);

  if (!file) {
    M5Cardputer.Display.println("");
    M5Cardputer.Display.println("Error: Failed to open file");
    return;
  }

  M5Cardputer.Display.println("");

  while (file.available()) {
    char c = file.read();
    M5Cardputer.Display.print(c);
  }

  file.close();
}

void readConfig() {
  File configFile = SD.open("/amber.cfg");

  if (configFile) {
    M5Cardputer.Display.println("[cfg] Reading amber.cfg...");
    while (configFile.available()) {
      String line = configFile.readStringUntil('\n');
      line.trim();

      int separatorIndex = line.indexOf(':');
      if (separatorIndex > 0) {
        String key = line.substring(0, separatorIndex);
        String value = line.substring(separatorIndex + 1);
        key.trim();
        value.trim();

        if (key.equals("initWifiOnBoot")) {
          if (value.equals("true")) {
            initWifiOnBoot = true;
          } else if (value.equals("false")) {
            initWifiOnBoot = false;
          }
        } else if (key.equals("wifiSSID")) {
          wifiSSID = value;
        } else if (key.equals("wifiPass")) {
          wifiPass = value;
        } else if (key.equals("clearInitLogs")) {
          if (value.equals("true")) {
            clearInitLogs = true;
          } else if (value.equals("false")) {
            clearInitLogs = false;
          }
        } else if (key.equals("fgColor")) {
          fgColor = value;
          //} else if (key.equals("syncTime")) {
          //  if (value.equals("true")) {
          //    syncTime = true;
          //  } else if (value.equals("false")) {
          //    syncTime = false;
          //  }
        } else if (key.equals("passwordEnabled")) {
          if (value.equals("true")) {
            passwordEnabled = true;
          } else if (value.equals("false")) {
            passwordEnabled = false;
          }
        } else if (key.equals("password")) {
          password = value;
        }
      }
    }
    configFile.close();
  } else {
    M5Cardputer.Display.println("[cfg] Looks like amber.cfg doesn't exist... Creating default configuration file...");
    createDefaultConfig();
  }
}

void updateConfig(String key, String newValue) {
  File configFile = SD.open("/amber.cfg");
  String tempConfig = "";
  bool keyFound = false;

  if (configFile) {
    while (configFile.available()) {
      String line = configFile.readStringUntil('\n');
      line.trim();

      int separatorIndex = line.indexOf(':');
      if (separatorIndex > 0) {
        String currentKey = line.substring(0, separatorIndex);
        currentKey.trim();

        if (currentKey == key) {
          tempConfig += key + ": " + newValue + "\n";
          keyFound = true;
        } else {
          tempConfig += line + "\n";
        }
      }
    }
    configFile.close();

    if (!keyFound) {
      tempConfig += key + ": " + newValue + "\n";
    }

    File newConfigFile = SD.open("/amber.cfg", FILE_WRITE);
    if (newConfigFile) {
      newConfigFile.print(tempConfig);
      newConfigFile.close();
      M5Cardputer.Display.println("");
      M5Cardputer.Display.println("Config file updated!");
    } else {
      M5Cardputer.Display.println("");
      M5Cardputer.Display.println("Error: Couldn't open config file for writing");
    }
  } else {
    M5Cardputer.Display.println("");
    M5Cardputer.Display.println("Error: Couldn't open config file for reading");
  }
}

void createDefaultConfig() {
  File configFile = SD.open("/amber.cfg", FILE_WRITE);

  if (configFile) {
    configFile.println("initWifiOnBoot: false");
    configFile.println("wifiSSID: defaultSSID");
    configFile.println("wifiPass: defaultPass");
    configFile.println("clearInitLogs: false");
    configFile.println("fgColor: WHITE");
    //configFile.println("syncTime: true");
    configFile.println("passwordEnabled: false");
    configFile.println("password: ");
    configFile.close();
    M5Cardputer.Display.println("[cfg] Config file created, you can use 'syscfg' command to modify it");
  } else {
    M5Cardputer.Display.println("[err] Couldn't create config file! skipping...");
  }
}

void WifiInit(String ssid, String pass) {
  const char* wifiSSID = ssid.c_str();
  const char* wifiPass = pass.c_str();

  WiFi.begin(wifiSSID, wifiPass);

  M5Cardputer.Display.print("[wifi] Connecting to: ");
  M5Cardputer.Display.print(ssid);

  int max_attempts = 10;
  int attempt = 0;

  while (WiFi.status() != WL_CONNECTED && attempt < max_attempts) {
    delay(1000);
    M5Cardputer.Display.print(".");
    attempt++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    M5Cardputer.Display.println("");
    M5Cardputer.Display.println("[wifi] Connected");
    M5Cardputer.Display.print("IP: ");
    M5Cardputer.Display.println(WiFi.localIP());
    M5Cardputer.Display.print("Strength: ");
    M5Cardputer.Display.println(WiFi.RSSI());
  } else {
    M5Cardputer.Display.println("");
    M5Cardputer.Display.println("[err] Failed to connect to Wifi , skipping...");
  }
}

void handleWifiInit(std::vector<String> args) {
  if (args.size() != 2) {
    M5Cardputer.Display.println("");
    M5Cardputer.Display.println("Usage: wifiinit <ssid> <pass>");
    return;
  }

  String ssid = args[0];
  String pass = args[1];

  const char* cssid = ssid.c_str();
  const char* cpass = pass.c_str();

  int max_attempts = 15;
  int attempt = 0;

  WiFi.begin(cssid, cpass);

  M5Cardputer.Display.println("");
  M5Cardputer.Display.print("Connecting to: ");
  M5Cardputer.Display.print(ssid);

  while (WiFi.status() != WL_CONNECTED && attempt < max_attempts) {
    delay(500);
    M5Cardputer.Display.print(".");
    attempt++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    M5Cardputer.Display.println("");
    M5Cardputer.Display.println("Connected to Wifi!");
    M5Cardputer.Display.print("IP: ");
    M5Cardputer.Display.println(WiFi.localIP());
    M5Cardputer.Display.print("Strength: ");
    M5Cardputer.Display.println(WiFi.RSSI());
    return;
  } else {
    M5Cardputer.Display.println("");
    M5Cardputer.Display.println("Error: Couldn't connect to Wifi");
    return;
  }

}

void handleWifiStatus(std::vector<String> args) {
  if (WiFi.status() == WL_CONNECTED) {
    M5Cardputer.Display.println("");
    M5Cardputer.Display.print("IP: ");
    M5Cardputer.Display.println(WiFi.localIP());
    M5Cardputer.Display.print("Strength: ");
    M5Cardputer.Display.println(WiFi.RSSI());
    return;
  } else {
    M5Cardputer.Display.println("");
    M5Cardputer.Display.println("Not connected!");
    return;
  }
}

void handleWifiDisconnect(std::vector<String> args) {
  if (WiFi.status() == WL_CONNECTED) {
    WiFi.disconnect();
    M5Cardputer.Display.println("");
    M5Cardputer.Display.println("Disconnected!");
    return;
  } else {
    M5Cardputer.Display.println("");
    M5Cardputer.Display.println("Not Connected!");
  }
}
