#pragma once
#include <borealis.hpp>
#include "core/switch_profile.hpp"

class AccountListItem : public brls::ListItem
{
protected:
    SwitchProfile accountProfile;
    bool toggleState;

public:
    AccountListItem(const SwitchProfile& profile, bool initialValue = true);

    virtual bool onClick() override;

    bool getToggleState();
    void forceState(bool state);
    const SwitchProfile& getAccountProfile();
};