#pragma once

#include "core/switch_profile.hpp"
#include "constants.hpp"
#include <string>
#include <vector>
#include <iostream>
#include <set>
#include <switch.h>

using namespace std;

class SharedSettings{
private:
    SharedSettings() {}

    string countryCode = DEFAULT_COUNTRY_CODE;
    vector<SwitchProfile> switchProfiles;

public:

    void printSelection() {
        for(auto& p: this->switchProfiles) {
            cout << p.uid_str << " - " << (p.selected ? "selected" : "not selected") << endl;
        }
    }

    SharedSettings(const SharedSettings&) = delete;
    SharedSettings& operator=(const SharedSettings &) = delete;
    SharedSettings(SharedSettings &&) = delete;
    SharedSettings & operator=(SharedSettings &&) = delete;

    static auto& instance(){
        static SharedSettings storage;
        return storage;
    }

    inline const string& getCountryCode() {
        return countryCode;
    }

    inline void setCountryCode(const string& code) {
        countryCode = code;
        cout << "new country code: " << code << endl;
    }

    void setProfileSelected(const AccountUid id, bool selected) {
        for(auto& p: this->switchProfiles) {
            if (p.id.uid[0] == id.uid[0] && p.id.uid[1] == id.uid[1]) {
                p.selected = selected;
            }
        }
        printSelection();
    }

    size_t getSelectedCount() {
        return count_if(this->switchProfiles.begin(), this->switchProfiles.end(), [](const SwitchProfile& p) { return p.selected; });
    }

    size_t getProfileCount() {
        return this->switchProfiles.size();
    }

    void addProfile(const SwitchProfile& p) {
        this->switchProfiles.emplace_back(p);
    }

    vector<SwitchProfile>& getSwitchProfiles() {
        return this->switchProfiles;
    }
};
