#include <borealis.hpp>
#include <switch.h>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <filesystem>
#include <sstream>
#include <vector>
#include <fstream>
#include "zipper.h"
#include "unzipper.h"
#include "constants.hpp"
#include "core/generator.hpp"
#include "core/shared_settings.hpp"
#include "core/switch_profile.hpp"
#include "utils/progress_event.hpp"

using namespace std;
using namespace zipper;
using namespace brls::i18n::literals;

static auto avatorsDir = string(ACCOUNT_PATH) + "/avators";
static auto baasDir = string(ACCOUNT_PATH) + "/baas";
static auto nasDir = string(ACCOUNT_PATH) + "/nas";

void shutdownAccount()
{
    cout << "Attempting to shut down BCAT, account and olsc... ";
    bool success_bcat = R_SUCCEEDED(pmshellTerminateProgram(0x010000000000000C));  // BCAT
    bool success_account = R_SUCCEEDED(pmshellTerminateProgram(0x010000000000001E));  // ACCOUNT
    if (success_account && success_bcat) {
        cout << "Success!" << endl;
    } else {
        cout << "BCAT: " << (success_bcat ? "success!" : "Failed!") << endl;
        cout << "ACCOUNT: " << (success_account ? "success!" : "Failed!") << endl;
    }
    //helps a bit
    pmshellTerminateProgram(0x010000000000003E);  // OLSC
}

vector<filesystem::path> getDirContents(const string& path, const string& extension, bool onlydirs=false)
{
    vector<filesystem::path> contents;
    bool filtered = extension.length() > 0;
    for (auto& entry: filesystem::recursive_directory_iterator(path)) {
        if (!entry.is_regular_file()) {
            if (onlydirs || !filtered) {
                contents.push_back(entry.path());
            }
            continue;
        }
        if (onlydirs || entry.path().extension().string() == "")
            continue;
        if (!filtered || entry.path().extension().string() == extension) {
            contents.push_back(entry.path());
        }
    }
    return contents;
}

void listFilesDebug() {
    cout << endl;
    cout << "Listing " << avatorsDir << endl;
    for (auto& entry: getDirContents(avatorsDir, ".jpg")) {
        cout << "[" <<  entry.string() << "]" << endl;
    }
    cout << "Listing " << baasDir << endl;
    for (auto& entry: getDirContents(baasDir, ".dat")) {
        cout << "[" <<  entry.string() << "]" << endl;
    }
    cout << "Listing " << nasDir << endl;
    for (auto& entry: getDirContents(nasDir, ".dat")) {
        cout << "[" <<  entry.string() << "]" << endl;
    }
    for (auto& entry: getDirContents(nasDir, ".json")) {
        cout << "[" <<  entry.string() << "]" << endl;
    }
    cout << endl;
}

void cleanupRogueFiles() {
    set<string> expected_uids;
    set<string> expected_nas_ids;

    for (const auto& p: SharedSettings::instance().getSwitchProfiles()) {
        expected_uids.insert(p.uid_str);
        stringstream nasId_ss;
        nasId_ss << setfill('0') << setw(16) << hex << p.nas_id;
        expected_nas_ids.insert(nasId_ss.str());
    }

    std::error_code ec;
    for (auto& entry: getDirContents(avatorsDir, ".jpg")) {
        if (expected_uids.contains(entry.stem().string())) {
            continue;
        }
        if(!filesystem::remove(entry.string(), ec)) {
            cout << "Error cleaning " << entry << ": " << ec << endl;
        } else {
            cout << "Cleaned: " << entry << endl;
        }
    }
    for (auto& entry: getDirContents(baasDir, ".dat")) {
        if (expected_uids.contains(entry.stem().string())) {
            continue;
        }
        if(!filesystem::remove(entry.string(), ec)) {
            cout << "Error cleaning " << entry << ": " << ec << endl;
        } else {
            cout << "Cleaned: " << entry << endl;
        }
    }
    for (auto& entry: getDirContents(nasDir, ".dat")) {
        if (expected_nas_ids.contains(entry.stem().string())) {
            continue;
        }
        if(!filesystem::remove(entry.string(), ec)) {
            cout << "Error cleaning " << entry << ": " << ec << endl;
        } else {
            cout << "Cleaned: " << entry << endl;
        }
    }
    for (auto& entry: getDirContents(nasDir, ".json")) {
        auto stem = entry.stem().string();
        stem.erase(stem.size() - 5); // remove _user
        if (expected_nas_ids.contains(stem)) {
            continue;
        }
        if(!filesystem::remove(entry.string(), ec)) {
            cout << "Error cleaning " << entry << ": " << ec << endl;
        } else {
            cout << "Cleaned: " << entry << endl;
        }
    }

}

void cleanupMacFiles(const string& path)
{
    for (auto& entry: filesystem::recursive_directory_iterator(path)) {
        if (!entry.is_regular_file()) {
            continue;
        }
        std::error_code ec;
        if (entry.path().filename().string() == ".DS_Store") {
            auto remove_result = filesystem::remove_all(entry.path().string(), ec);
            if(remove_result == static_cast<std::uintmax_t>(-1)) {
                cout << "Error cleaning Mac files: " << ec << endl;
            } else {
                cout << "Cleaned " << remove_result << " files" << endl;
            }
        }
    }
}

bool initDirs()
{
    try
    {
        filesystem::create_directories(BACKUP_PATH);
        filesystem::create_directories(RESTORE_FILE_DIR);
        return true;
    }
    catch (exception& e) // Not using filesystem_error since bad_alloc can throw too.
    {
        cout << e.what() << endl;
        brls::Application::crash(string("Failed! ") + e.what());
    }
    return false;
}

FsFileSystem mountSaveData()
{
    FsFileSystem acc;
#ifndef LINKALHO_DEBUG
    shutdownAccount();
    cout << "Attempting to mount savedata... ";
    if(R_SUCCEEDED(fsOpen_SystemSaveData(&acc, FsSaveDataSpaceId_System, 0x8000000000000010, (AccountUid) {0}))) {
        cout << "Succeess!" << endl;
        fsdevMountDevice("account", acc);
    } else {
        cout << "Failed!" << endl;
    }
#endif
    return acc;
}

void unmountSaveData(FsFileSystem& acc, bool commit=false)
{
#ifndef LINKALHO_DEBUG
    if (commit) {
        if (R_SUCCEEDED(fsdevCommitDevice("account"))) {
            cout << "fsdevCommitDevice successful!" << endl;
        } else {
            cout << "fsdevCommitDevice failed!" << endl;
        }
    }
    if (R_SUCCEEDED(fsdevUnmountDevice("account"))) {
        cout << "fsdevUnmountDevice successful!" << endl;
    } else {
        cout << "fsdevUnmountDevice failed!" << endl;
    }
    fsFsClose(&acc);
#endif
}

void executeBackup(const string& reason)
{
    cleanupMacFiles(ACCOUNT_PATH);

    time_t t = time(nullptr);
    tm tm = *localtime(&t);
    stringstream backupFile;
    backupFile << BACKUP_PATH << "backup_profiles_" << put_time(&tm, "%Y%m%d_%H%M%S") << "[" << reason << "].zip";
    cout << "Creating backup..." << endl << endl;
    listFilesDebug();
    Zipper zipper(backupFile.str());
    //auto flags = (Zipper::zipFlags::Better | Zipper::zipFlags::SaveHierarchy);
    zipper.add(string(ACCOUNT_PATH)+"/registry.dat");
    zipper.add(avatorsDir);
    zipper.add(baasDir);
    zipper.add(nasDir);
    zipper.close();
    cout << "Backup created in " << BACKUP_PATH << endl;
}

void restoreBackup(const string& backupFullpath)
{
    try {
        ProgressEvent::instance().reset();
        ProgressEvent::instance().setTotalSteps(7);

        FsFileSystem acc = mountSaveData();
        ProgressEvent::instance().setStep(1);

        executeBackup("restore");
        ProgressEvent::instance().setStep(2);
        cout << endl << endl  << "Restoring backup... ";

        std::error_code ec;
        auto remove_result = filesystem::remove_all(baasDir, ec);
        if(remove_result == static_cast<std::uintmax_t>(-1)) {
            cout << "Error cleaning " << baasDir << ": " << ec << endl;
        } else {
            cout << "Cleaned " << remove_result << " files from " << baasDir << endl;
        }
        ProgressEvent::instance().setStep(3);

        remove_result = filesystem::remove_all(nasDir, ec);
        if(remove_result == static_cast<std::uintmax_t>(-1)) {
            cout << "Error cleaning " << nasDir << ": " << ec << endl;
        } else {
            cout << "Cleaned " << remove_result << " files from " << nasDir << endl;
        }
        ProgressEvent::instance().setStep(4);

        for (auto& jpgFile: getDirContents(avatorsDir, ".jpg")) {
            remove_result = filesystem::remove(jpgFile.string(), ec);
            if(remove_result == static_cast<std::uintmax_t>(-1)) {
                cout << "Error cleaning " << jpgFile << ": " << ec << endl;
            } else {
                cout << "Cleaned " << remove_result << ": " << jpgFile << endl;
            }
        }
        ProgressEvent::instance().setStep(5);

        Unzipper unzipper(backupFullpath);
        unzipper.extract(ACCOUNT_PATH);
        unzipper.close();
        ProgressEvent::instance().setStep(6);
        cout << "Success!" << endl;
        listFilesDebug();
        unmountSaveData(acc, true);
        ProgressEvent::instance().setStep(7);
    }
    catch (exception& e) // Not using filesystem_error since bad_alloc can throw too.
    {
        cout << "Failed! " << e.what() << endl;
        brls::Application::crash(string("Failed! ") + e.what());
    }
}

void unlinkSingleAccount(const SwitchProfile& profile) {

    if (!profile.is_linked) {
        cout << profile.uid_str << " profile not linked." << endl;
        return;
    }

    std::error_code ec;
    string baas_file = string(ACCOUNT_PATH) + "/baas/" + profile.uid_str + ".dat";
    if (filesystem::exists(baas_file)) {
        if (!filesystem::remove(baas_file, ec)) {
            cout << "Error removing baas_file: " << baas_file << " error: " << ec << endl;
        }
    } else {
        cout << "Baas file not found: " << baas_file << endl;
    }

    stringstream nasId_ss;
    nasId_ss << setfill('0') << setw(16) << hex << profile.nas_id;

    auto nasDatFile = string(ACCOUNT_PATH) + "/nas/" + nasId_ss.str() + ".dat";
    if (filesystem::exists(nasDatFile)) {
        if (!filesystem::remove(nasDatFile, ec)) {
            cout << "Error removing nasDatFile: " << nasDatFile << " error: " << ec << endl;
        }
    } else {
        cout << "nasDat file not found: " << nasDatFile << endl;
    }

    auto nasJsonFile = string(ACCOUNT_PATH) + "/nas/" + nasId_ss.str() + "_user.json";
    if (filesystem::exists(nasJsonFile)) {
        if (!filesystem::remove(nasJsonFile, ec)) {
            cout << "Error removing nasJsonFile: " << nasJsonFile << " error: " << ec << endl;
        }
    } else {
        cout << "nasJson file not found: " << nasJsonFile << endl;
    }
}

void linkAccounts()
{
    auto selectedCount = SharedSettings::instance().getSelectedCount();
    if (selectedCount == 0) {
        brls::Application::notify("translations/errors/no_accounts_selected"_i18n);
        brls::Application::popView();
        return;
    }
    try {
        ProgressEvent::instance().reset();
        ProgressEvent::instance().setTotalSteps(5+selectedCount);

        FsFileSystem acc = mountSaveData();
        ProgressEvent::instance().setStep(1);

        executeBackup("link");
        cout << endl << endl  << "Linking accounts... ";

        cout << "create=[" << baasDir << "]" << endl;
        filesystem::create_directories(baasDir);

        cout << "create=[" << nasDir << "]" << endl;
        filesystem::create_directories(nasDir);

        ProgressEvent::instance().setStep(2);

        cleanupRogueFiles();
        ProgressEvent::instance().setStep(3);

        int c = 4;
        for (const auto& p: SharedSettings::instance().getSwitchProfiles()) {
            if (!p.selected) {
                continue;
            }
            unlinkSingleAccount(p);

            Generator gen;
            cout << "Generating data for " << p.name << ": " << p.uid_str << std::endl;
            auto linkerFile = baasDir+"/"+p.uid_str+".dat";
            gen.writeBaas(linkerFile);

            auto profileDatFilename = nasDir + "/" + gen.nasIdStr() + ".dat";
            gen.writeProfileDat(profileDatFilename);

            auto profileJsonFilename = nasDir + "/" + gen.nasIdStr() + "_user.json";
            gen.writeProfileJson(profileJsonFilename);

            ProgressEvent::instance().setStep(c++);
        }

        ProgressEvent::instance().setStep(c++);
        cout << "Success!" << endl;

        listFilesDebug();

        unmountSaveData(acc, true);
        ProgressEvent::instance().setStep(c++);
    }
    catch (exception& e) // Not using filesystem_error since bad_alloc can throw too.
    {
        cout << "Failed! " << e.what() << endl;
        brls::Application::crash(string("Failed! ") + e.what());
    }
}

void unlinkAccounts()
{
    auto selectedCount = SharedSettings::instance().getSelectedCount();
    if (selectedCount == 0) {
        brls::Application::notify("translations/errors/no_accounts_selected"_i18n);
        brls::Application::popView();
        return;
    }
    try {
        ProgressEvent::instance().reset();
        ProgressEvent::instance().setTotalSteps(4+selectedCount);

        FsFileSystem acc = mountSaveData();
        ProgressEvent::instance().setStep(1);

        executeBackup("unlink");
        ProgressEvent::instance().setStep(2);
        cout << endl << endl << "Unlinking accounts... ";

        cleanupRogueFiles();
        ProgressEvent::instance().setStep(3);

        int c = 4;
        for (const auto& p: SharedSettings::instance().getSwitchProfiles()) {
            if (!p.selected) {
                continue;
            }
            unlinkSingleAccount(p);
            ProgressEvent::instance().setStep(c++);
        }

        cout << "Success!" << endl;

        listFilesDebug();

        unmountSaveData(acc, true);
        ProgressEvent::instance().setStep(c++);
    }
    catch (exception& e) // Not using filesystem_error since bad_alloc can throw too.
    {
        cout << "Failed! " << e.what() << endl;
        brls::Application::crash(string("Failed! ") + e.what());
    }
}

void manualBackup()
{
    try {
        ProgressEvent::instance().reset();
        ProgressEvent::instance().setTotalSteps(3);

        FsFileSystem acc = mountSaveData();
        ProgressEvent::instance().setStep(1);

        executeBackup("manual");
        cout << "Success!" << endl;

        listFilesDebug();
        unmountSaveData(acc, true);
        ProgressEvent::instance().setStep(3);
    }
    catch (exception& e) // Not using filesystem_error since bad_alloc can throw too.
    {
        cout << "Failed! " << e.what() << endl;
        brls::Application::crash(string("Failed! ") + e.what());
    }
}