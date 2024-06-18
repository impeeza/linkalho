#include "views/account_select_view.hpp"
#include "utils/utils.hpp"
#include "core/shared_settings.hpp"
#include "constants.hpp"
#include <switch.h>
#include <string>
#include <sstream>
#include <iomanip>

using namespace std;
using namespace brls::i18n::literals;

void AccountSelectView::computeValue() {
    this->setValue(fmt::format("translations/account_select_view/selected_count"_i18n, SharedSettings::instance().getSelectedCount(), SharedSettings::instance().getProfileCount()), false, false);
    HardwareType hwType = getHardwareType();
    auto reboot_payload_text = "translations/account_select_view/reboot_payload_disabled"_i18n;
    if (hwType == Erista) {
        if (getPayload().empty()) {
            reboot_payload_text = fmt::format("translations/account_select_view/reboot_payload_inactive"_i18n, CUSTOM_PAYLOAD_FILE);
        } else {
            reboot_payload_text = "translations/account_select_view/reboot_payload_active"_i18n;
        }
    }
    this->setDescription(fmt::format("translations/account_select_view/extra_info"_i18n, SharedSettings::instance().getProfileCount(), (SharedSettings::instance().getProfileCount() == 1 ? "" : "s"), reboot_payload_text));
}

AccountSelectView::AccountSelectView() : ListItem("translations/account_select_view/title"_i18n)
{
    if (R_SUCCEEDED(accountInitialize(AccountServiceType_Administrator))) {
        AccountUid* uids = new AccountUid[ACC_USER_LIST_SIZE];
        s32 userCount = 0;

        if (R_SUCCEEDED(accountListAllUsers(uids, ACC_USER_LIST_SIZE, &userCount))) {

            for (int i = 0; i < userCount; i++) {
                // Icon data
                u8* iconBuffer;
                u32 imageSize, realSize;
                AccountUid uid = uids[i];
                // Lookup and cache the users details
                AccountProfile profile;
                AccountProfileBase profileBase = {};

                if (R_SUCCEEDED(accountGetProfile(&profile, uid)) && R_SUCCEEDED(accountProfileGet(&profile, nullptr, &profileBase)) && R_SUCCEEDED(accountProfileGetImageSize(&profile, &imageSize)) && (iconBuffer = (u8*)malloc(imageSize)) != NULL && R_SUCCEEDED(accountProfileLoadImage(&profile, iconBuffer, imageSize, &realSize))) {
                    stringstream uid_str;
                    uid_str << setfill('0') << setw(8) << hex << (uid.uid[0] & 0xffffffff) << "-";
                    uid_str << setfill('0') << setw(4) << hex << ((uid.uid[0] >> 32) & 0xffff) << "-";
                    uid_str << setfill('0') << setw(4) << hex << ((uid.uid[0] >> 48) & 0xffff) << "-";
                    uid_str << setfill('0') << setw(2) << hex << (uid.uid[1] & 0xff);
                    uid_str << setfill('0') << setw(2) << hex << ((uid.uid[1] >> 8) & 0xff) << "-";
                    uid_str << setfill('0') << setw(8) << hex << ((uid.uid[1] >> 32) & 0xffffffff);
                    uid_str << setfill('0') << setw(4) << ((uid.uid[1] >> 16) & 0xffff);

                    accountProfileClose(&profile);

                    // extract current AccountId and NasId
                    Service baas;
                    const auto rc = getBaasAccountAdministrator(uid, &baas);
                    uint64_t account_id = 0;
                    uint64_t nas_id = 0;
                    bool linked = false;

                    if(R_SUCCEEDED(rc)) {
                        if (R_FAILED(baasAdministrator_isLinkedWithNAS(&baas, &linked))) {
                            cout << "Failed to extract link status" << endl;
                        }
                        if (R_FAILED(baasAdministrator_getAccountId(&baas, &account_id))) {
                            cout << "Failed to extract account_id" << endl;
                        }
                        if (R_FAILED(baasAdministrator_getNasId(&baas, &nas_id))) {
                            cout << "Failed to extract nas_id" << endl;
                        }
                        serviceClose(&baas);
                    }

                    auto item = SwitchProfile {
                        .id   = uid,
                        .uid_str = uid_str.str(),
                        .name = string(profileBase.nickname, 0x20),
                        .icon = make_pair(iconBuffer, imageSize),
                        .is_linked = linked,
                        .account_id = account_id,
                        .nas_id = nas_id,
                        .selected = true
                    };
                    SharedSettings::instance().addProfile(item);
                }
            }
        }

        delete[] uids;
        accountExit();
    }

    this->getClickEvent()->subscribe([this](brls::View* view) {
        auto accountSelectFrame = new brls::AppletFrame(true, true);
        accountSelectFrame->setTitle("translations/account_select_view/title"_i18n);

        this->accountListItems.clear();
        this->accountListItems.reserve( SharedSettings::instance().getProfileCount());

        auto accountList = new brls::List();
        for (auto& p : SharedSettings::instance().getSwitchProfiles()) {
            auto item = new AccountListItem(p, p.selected);
            item->getClickEvent()->subscribe([this](View* v){
                auto itemPtr = reinterpret_cast<AccountListItem*>(v);
                SharedSettings::instance().setProfileSelected(itemPtr->getAccountProfile().id, itemPtr->getToggleState());
            });
            this->accountListItems.emplace_back(item);
            accountList->addView(item);
        }
        accountSelectFrame->setContentView(accountList);

        brls::Application::pushView(accountSelectFrame);

        accountSelectFrame->registerAction("translations/account_select_view/unselect_all"_i18n, brls::Key::L, [this] {
            for (auto item: this->accountListItems) {
                item->forceState(false);
            }
            return true;
        });
        accountSelectFrame->registerAction("translations/account_select_view/select_all"_i18n, brls::Key::R, [this] {
            for (auto item: this->accountListItems) {
                item->forceState(true);
            }
            return true;
        });
        accountSelectFrame->registerAction("translations/confirm_view/back"_i18n, brls::Key::B, [this, &accountList] {
            this->computeValue();
            brls::Application::popView();
            return true;
        });
        accountSelectFrame->registerAction("", brls::Key::PLUS, [this]{return true;}, true);
        accountSelectFrame->updateActionHint(brls::Key::PLUS, ""); // make the change visible
    });
    this->computeValue();
}
