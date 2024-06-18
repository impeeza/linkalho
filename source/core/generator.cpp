#include <random>
#include <chrono>
#include <cstring>
#include <algorithm>
#include <sstream>
#include <fstream>
#include "constants.hpp"
#include "utils/utils.hpp"
#include "core/shared_settings.hpp"
#include "core/generator.hpp"

#include <iostream>

using namespace std;

// static auto BAAS_HEADER1    = 0xA5D192EA40AD1304;
static auto BAAS_HEADER2    = 0x0000006E00000001;
static auto BAAS_HEADER3    = 0x0000000100000001;
static auto PROFILE = R"({"id":"#NAS_ID#","language":"#LOCALE#","timezone":"#TIMEZONE#","country":"#COUNTRY_CODE#","analyticsOptedIn":false,"gender":"male","emailOptedIn":false,"birthday":"1980-01-01","isChild":false,"email":"•","screenName":"•","region":"","loginId":"•","nickname":"•","isNnLinked":false,"isTwitterLinked":false,"isFacebookLinked":false,"isGoogleLinked":false})";

static mt19937_64 engine(chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count());

const string generateRandomAlphanumericString(size_t len)
{
    static constexpr auto chars =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

    uniform_int_distribution<size_t> dist(0, strlen(chars) - 1);
    auto result = string(len, '\0');
    generate_n(begin(result), len, [&]() { return chars[dist(engine)]; });
    return result;
}

unsigned long generateBytes()
{
    uniform_int_distribution<unsigned long> byteGenerator(0x0UL, 0xFFFFFFFFFFFFFFFF);
    return byteGenerator(engine);
}

unsigned long generateNasId()
{
    return generateBytes();
}

unsigned long generateBaasId()
{
    return generateBytes();
}

const string generateBaasUserPassword()
{
    return generateRandomAlphanumericString(40);
}

const string generateProfileDat()
{
    return generateRandomAlphanumericString(128);
}

string stringReplace(const string& str, const string& from, const string& to) {
    string strCopy = str;
    size_t startPos = strCopy.find(from);
    if(startPos == string::npos)
        return str;
    strCopy.replace(startPos, from.length(), to);
    return strCopy;
}

Generator::Generator()
{
    _baasUserId = generateBaasId();
    _nasId = generateNasId();
    stringstream ss;
    ss << std::hex << _nasId;
    _nasIdStr = ss.str();
}

const string& Generator::nasIdStr() {
    return _nasIdStr;
}


void Generator::writeBaas(const string& fullpath)
{
    ofstream ofs(fullpath, ios::binary);
    auto account_id = generateBaasId();
    ofs.write(reinterpret_cast<char*>(&account_id), sizeof(account_id));
    ofs.write(reinterpret_cast<char*>(&BAAS_HEADER2), sizeof(BAAS_HEADER2));
    ofs.write(reinterpret_cast<char*>(&_nasId), sizeof(_nasId));
    ofs.write(reinterpret_cast<char*>(&BAAS_HEADER3), sizeof(BAAS_HEADER3));
    ofs.write(reinterpret_cast<char*>(&_baasUserId), sizeof(_baasUserId));
    ofs << generateBaasUserPassword();
}

void Generator::writeProfileDat(const string& fullpath)
{
    ofstream ofs(fullpath, ios::binary);
    ofs << generateProfileDat();
}

void Generator::writeProfileJson(const string& fullpath)
{
    ofstream ofs(fullpath, ios::binary);
    string locale = getLocale();
    string timezone = getTimezone();
    string country_code = SharedSettings::instance().getCountryCode();
    string generated_profile = stringReplace(
        stringReplace(
            stringReplace(
                stringReplace(
                    PROFILE,
                    "#NAS_ID#",
                    _nasIdStr
                ),
                "#LOCALE#",
                locale
            ),
            "#TIMEZONE#",
            timezone
        ),
        "#COUNTRY_CODE#",
        country_code
    );
    cout << "generated profile: " << generated_profile << endl;
    ofs << generated_profile;
}
