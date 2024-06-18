#include <iostream>
#include <cstring>
#include <string>
#include "constants.hpp"
#include "utils/utils.hpp"
#include <sys/stat.h>
#include <filesystem>

void attemptForceReboot()
{
#ifndef LINKALHO_DEBUG
    Result rc = spsmInitialize();
    if (R_FAILED(rc))
        printf("spsmInit: %08X\n", rc);
    else
    {
        spsmShutdown(true);
        spsmExit();
    }
#else
    std::cout << "Reboot would happen here" << std::endl;
#endif
}

HidsysNotificationLedPattern getBreathePattern()
{
    HidsysNotificationLedPattern pattern;
    memset(&pattern, 0, sizeof(pattern));

    pattern.baseMiniCycleDuration = 0x8;    // 100ms.
    pattern.totalMiniCycles = 0x2;          // 2 mini cycles. Last one 12.5ms.
    pattern.totalFullCycles = 0x0;          // Repeat forever.
    pattern.startIntensity = 0x0;           // 0%

    pattern.miniCycles[0].ledIntensity = 0xF;        // 100%.
    pattern.miniCycles[0].transitionSteps = 0xF;     // 15 steps. Transition time 1.5s.
    pattern.miniCycles[0].finalStepDuration = 0x0;   // Forced 12.5ms.
    pattern.miniCycles[1].ledIntensity = 0x0;        // 0%.
    pattern.miniCycles[1].transitionSteps = 0xF;     // 15 steps. Transition time 1.5s.
    pattern.miniCycles[1].finalStepDuration = 0x0;   // Forced 12.5ms.

    return pattern;
}

HidsysNotificationLedPattern getConfirmPattern()
{
    HidsysNotificationLedPattern pattern;
    memset(&pattern, 0, sizeof(pattern));

    pattern.baseMiniCycleDuration = 0x1;             // 12.5ms.
    pattern.totalMiniCycles = 0x2;                   // 3 mini cycles. Last one 12.5ms.
    pattern.totalFullCycles = 0x3;                   // Repeat 3 times.
    pattern.startIntensity = 0x0;                    // 0%.

    pattern.miniCycles[0].ledIntensity = 0xF;        // 100%.
    pattern.miniCycles[0].transitionSteps = 0xF;     // 15 steps. Transition time 187.5ms.
    pattern.miniCycles[0].finalStepDuration = 0x0;   // Forced 12.5ms.
    pattern.miniCycles[1].ledIntensity = 0x0;        // 0%.
    pattern.miniCycles[1].transitionSteps = 0x2;     // 2 steps. Transition time 25ms.
    pattern.miniCycles[1].finalStepDuration = 0x0;   // Forced 12.5ms.

    return pattern;
}

HidsysNotificationLedPattern getClearPattern()
{
    HidsysNotificationLedPattern pattern;
    memset(&pattern, 0, sizeof(pattern));
    return pattern;
}

void sendLedPattern(HidsysNotificationLedPattern pattern)
{
    s32 totalEntries = 0;
    HidsysUniquePadId uniquePadIds[2] = {0};
    memset(uniquePadIds, 0, sizeof(uniquePadIds));
    Result res = hidsysGetUniquePadsFromNpad(HidNpadIdType_Handheld, uniquePadIds, 2, &totalEntries);
    if (R_SUCCEEDED(res)) {
        for (auto i = 0; i < totalEntries; ++i) {
            hidsysSetNotificationLedPattern(&pattern, uniquePadIds[i]);
        }
    }
}

const std::string getPayload()
{
    // if an override payload exists, boot from it
    return std::filesystem::exists(CUSTOM_PAYLOAD_FILE_PATH) ? CUSTOM_PAYLOAD_FILE_PATH : "";
}

HardwareType getHardwareType() {
    /**
     * Icosa = 0,   // Erista retail
     * Copper = 1,  // Erista prototype
     * Hoag = 2,    // Mariko Lite Retail for 8.0.0+, Invalid for 1.0.0-7.0.1
     * Iowa = 3,    // Mariko retail for 4.0.0+
     * Calcio = 4,  // Mariko prototype for 8.0.0+
     * Five = 5,    // Aula for 10.0.0+
    */

    if (splInitialize() != 0) return UnknownHardware;
    u64 hwType = 15;  // invalid
    Result rc = splGetConfig(SplConfigItem_HardwareType, &hwType);
    splExit();

    if (R_FAILED(rc)) return UnknownHardware;

    switch (hwType) {
        case 0:
        case 1:
            return Erista;
        case 2:
        case 3:
        case 4:
            return Mariko;
        case 5:
            return Aula;
    }
    return UnknownHardware;
}

bool isErista() {
    return getHardwareType() == Erista;
}

const std::string getLocale() {
    u64 languageCode=0;
    if (R_SUCCEEDED(setGetSystemLanguage(&languageCode)) && languageCode) {
        return std::string(reinterpret_cast<char*>(&languageCode));
    }
    return std::string(DEFAULT_LOCALE);
}

const std::string getTimezone() {
    TimeLocationName tl;
    if (R_SUCCEEDED(setsysGetDeviceTimeZoneLocationName(&tl))) {
        return tl.name;
    }
    return DEFAULT_TIMEZONE;
}

Result baasAdministrator_getAccountId(Service *admin_srv, u64 *out_id) {
    return serviceDispatchOut(admin_srv, 1, *out_id);
}

Result baasAdministrator_getNasId(Service *admin_srv, u64 *out_id) {
    return serviceDispatchOut(admin_srv, 120, *out_id);
}

Result getBaasAccountAdministrator(const AccountUid user_id, Service *out_admin_srv) {
    return serviceDispatchIn(accountGetServiceSession(), 250, user_id,
        .out_num_objects = 1,
        .out_objects = out_admin_srv,
    );
}

Result baasAdministrator_isLinkedWithNAS(Service *admin_srv, bool *out_linked) {
    return serviceDispatchOut(admin_srv, 250, *out_linked);
}

Result baasAdministrator_deleteRegistrationInfoLocally(Service *admin_srv) {
    return serviceDispatch(admin_srv, 203);
}

Result unlinkLocally(const AccountUid user_id) {
    Service baas;
    auto rc = getBaasAccountAdministrator(user_id, &baas);
    if(R_SUCCEEDED(rc)) {
        bool linked = false;
        rc = baasAdministrator_isLinkedWithNAS(&baas, &linked);
        std::cout << "is linked:" << linked << std::endl;
        if(R_SUCCEEDED(rc) && linked) {
            rc = baasAdministrator_deleteRegistrationInfoLocally(&baas);
            std::cout << (R_SUCCEEDED(rc) ? "Unlink successful" : "Unlink failed") << std::endl;
        } else {
            std::cout << (R_SUCCEEDED(rc) ? "Not linked" : "Could not query isLinkedWithNintendoAccount") << std::endl;
        }
        serviceClose(&baas);
    } else {std::cout << "could not elevate to baas administrator" << std::endl;}
    return rc;
}