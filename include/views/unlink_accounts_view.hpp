#pragma once

#include "core/switch_profile.hpp"
#include <borealis.hpp>
#include <vector>

class UnlinkAccountsView : public brls::ListItem
{
public:
    UnlinkAccountsView(bool canUseLed);
};