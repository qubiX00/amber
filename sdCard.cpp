// functions for managing sd card (stolen from https://github.com/m5stack/M5Cardputer/blob/master/examples/Basic/sdcard/sdcard.ino lol)

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

#include <FS.h>
#include <FSImpl.h>
#include <vfs_api.h>

#include <SPI.h>

#include <SD.h>
#include <sd_defines.h>
#include <sd_diskio.h>

#include <M5Cardputer.h>
#include "sdCard.h"

void listDir(fs::FS & fs, String dirname, uint8_t levels) {
  File root = fs.open(dirname);
  if (!root) {
    M5Cardputer.Display.println("Error: Failed to open directory");
    return;
  }

  int fileLine = 0;
  bool morePrinted = false;

  File file = root.openNextFile();
  while (file) {
    M5Cardputer.update();

    if (fileLine < 6) {
      String line;
      if (file.isDirectory()) {
        line = "(dir) ";
      } else {
        line = "";
      }
      line += file.name();
      M5Cardputer.Display.println(line);

      if (file.isDirectory() && levels) {
        listDir(fs, file.path(), levels - 1);
      }
      fileLine++;
      file = root.openNextFile();
    } else {
      if (!morePrinted) {
        M5Cardputer.Display.println("More...");
        morePrinted = true;
      }
      if (M5Cardputer.Keyboard.isChange()) {
        if (M5Cardputer.Keyboard.isKeyPressed(KEY_ENTER)) {
          fileLine = 0;
          morePrinted = false;
          continue;
        }
      }
    }
  }
}



void createDir(fs::FS & fs, String path) {
  if (fs.mkdir(path)) {
    M5Cardputer.Display.println("");
  } else {
    M5Cardputer.Display.println("Error: Failed to create directory");
  }
}

void removeDir(fs::FS & fs, String path) {
  if (fs.rmdir(path)) {
    M5Cardputer.Display.println("");
  } else {
    M5Cardputer.Display.println("Error: Failed to remove directory");
  }
}

void readFile(fs::FS & fs, String path) {
  File file = fs.open(path);
  if (!file) {
    M5Cardputer.Display.println("Error: Failed to read from file");
    return;
  }

  M5Cardputer.Display.println("Read from file: ");
  while (file.available()) {
    M5Cardputer.Display.println(file.read());
  }
  file.close();
}

void writeFile(fs::FS & fs, String path, String message) {
  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    M5Cardputer.Display.println("Error: Failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    M5Cardputer.Display.println("");
  } else {
    M5Cardputer.Display.println("Error: Write failed");
  }
  file.close();
}

void appendFile(fs::FS & fs, String path, String message) {
  File file = fs.open(path, FILE_APPEND);
  if (!file) {
    M5Cardputer.Display.println("Error: Failed to open file for appending");
    return;
  }
  if (file.print(message)) {
    M5Cardputer.Display.println("");
  } else {
    M5Cardputer.Display.println("Error: Append failed");
  }
  file.close();
}

void renameFile(fs::FS & fs, String path1, String path2) {
  if (fs.rename(path1, path2)) {
    M5Cardputer.Display.println("");
  } else {
    M5Cardputer.Display.println("Error: Rename failed");
  }
}

void deleteFile(fs::FS & fs, String path) {
  if (fs.remove(path)) {
    M5Cardputer.Display.println("");
  } else {
    M5Cardputer.Display.println("Error: Delete failed");
  }
}

void testFileIO(fs::FS & fs, String path) {
  File file = fs.open(path);
  static uint8_t buf[512];
  size_t len     = 0;
  uint32_t start = millis();
  uint32_t end   = start;
  if (file) {
    len         = file.size();
    size_t flen = len;
    start       = millis();
    while (len) {
      size_t toRead = len;
      if (toRead > 512) {
        toRead = 512;
      }
      file.read(buf, toRead);
      len -= toRead;
    }
    end = millis() - start;
    String line;
    line = String(flen) + " bytes read for " + String(end) + " ms";
    M5Cardputer.Display.println(line);
    file.close();
  } else {
    M5Cardputer.Display.println("Error: Failed to open file for reading");
    return;
  }

  file = fs.open(path, FILE_WRITE);
  if (!file) {
    M5Cardputer.Display.println("Error: Failed to open file for writing");
    return;
  }

  size_t i;
  start = millis();
  for (i = 0; i < 2048; i++) {
    file.write(buf, 512);
  }
  end = millis() - start;
  String line;
  line = String(2048 * 512) + " bytes written for " + String(end) + " ms";
  M5Cardputer.Display.println(line);
  file.close();
}

void createFile(fs::FS &fs, String path) {
  File file = fs.open(path, FILE_WRITE);

  if (!file) {
    M5Cardputer.Display.println("Error: Could not create file");
    return;
  }

  file.close();
}
