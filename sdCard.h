#ifndef SD_HANDLER_H
#define SD_HANDLER_H

#include <Arduino.h>
#include <FS.h> // Filesystem support for SD card operations

void listDir(fs::FS &fs, String dirname, uint8_t levels);
void createDir(fs::FS &fs, String path);
void removeDir(fs::FS &fs, String path);
void readFile(fs::FS &fs, String path);
void writeFile(fs::FS &fs, String path, String message);
void appendFile(fs::FS &fs, String path, String message);
void renameFile(fs::FS &fs, String path1, String path2);
void deleteFile(fs::FS &fs, String path);
void createFile(fs::FS &fs, String path);
void testFileIO(fs::FS &fs, String path);

#endif
