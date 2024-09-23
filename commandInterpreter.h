#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

#include <Arduino.h>
#include <vector>

void runCommand(String input);
void handleAbout(std::vector<String> args);
void handleListDir(std::vector<String> args);
void handleCurrentDir(std::vector<String> args);
void handleCharInfo(std::vector<String> args);
void handleLedColor(std::vector<String> args);
void handleHelp(std::vector<String> args);
void handlePrint(std::vector<String> args);
void handleSound(std::vector<String> args);
void handleCalculate(std::vector<String> args);
void handleSysCfg(std::vector<String> args);
void handleSysCfgPrint(std::vector<String> args);
void handleWifiInit(std::vector<String> args);
void handleWifiStatus(std::vector<String> args);
void handleWifiDisconnect(std::vector<String> args);
void handleClear(std::vector<String> args);
void handleCat(std::vector<String> args);
void handleChangeDir(std::vector<String> args);
void handleMkDir(std::vector<String> args);
void handleRmDir(std::vector<String> args);
void handleNewFile(std::vector<String> args);
void handleRemoveFile(std::vector<String> args);
void handleRoll(std::vector<String> args);
void handleMatrix(std::vector<String> args);

void displayFileContent(String path);
void updateConfig(String key, String newValue);
void readConfig();
void createDefaultConfig();

void WifiInit(String ssid, String pass);


#endif
