#include "views/account_list_item.hpp"

using namespace brls::i18n::literals;

AccountListItem::AccountListItem(const SwitchProfile& profile, bool initialValue)
    : ListItem(profile.name, "", (profile.is_linked ? "translations/account_list_item/linked"_i18n : "translations/account_list_item/not_linked"_i18n))
    , toggleState(initialValue)
    , accountProfile(profile)
{
    this->setChecked(this->toggleState);
    this->setThumbnail(this->accountProfile.icon.first, this->accountProfile.icon.second);
}

bool AccountListItem::onClick()
{
    this->toggleState = !this->toggleState;
    this->setChecked(this->toggleState);

    ListItem::onClick();
    return true;
}

bool AccountListItem::getToggleState()
{
    return this->toggleState;
}

void AccountListItem::forceState(bool state) {
    this->toggleState = state;
    this->setChecked(this->toggleState);
    ListItem::onClick();
}

const SwitchProfile& AccountListItem::getAccountProfile() {
    return this->accountProfile;
}