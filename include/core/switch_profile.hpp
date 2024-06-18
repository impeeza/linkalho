#pragma once

#include <borealis.hpp>
#include <string>
#include <switch.h>

class SwitchProfile {
public:
    const AccountUid id;
    const std::string uid_str;
    const std::string name;
    const std::pair<unsigned char*, size_t> icon;
    const bool is_linked;
    uint64_t account_id;
    uint64_t nas_id;
    bool selected;
};