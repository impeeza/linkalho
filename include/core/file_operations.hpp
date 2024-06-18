#pragma once

#include <filesystem>
#include <vector>

bool initDirs();
void linkAccounts();
void unlinkAccounts();
void restoreBackup(const std::string& backupFullpath);
void manualBackup();
