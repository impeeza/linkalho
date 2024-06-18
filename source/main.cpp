#include <switch.h>
#include <borealis.hpp>
#include "styles/visual_overrides.hpp"
#include "views/country_select_view.hpp"
#include "views/restore_backup_view.hpp"
#include "views/create_backup_view.hpp"
#include "views/link_accounts_view.hpp"
#include "views/unlink_accounts_view.hpp"
#include "views/account_select_view.hpp"
#include "core/file_operations.hpp"

using namespace std;
using namespace brls::i18n::literals;

int main(int argc, char* argv[])
{
    pmshellInitialize();
    bool canUseLed = false;
    if (R_SUCCEEDED(hidsysInitialize())) {
        canUseLed = true;
    }
    initDirs();

    brls::i18n::loadTranslations();

    auto linkalhoStyle = VisualOverrides::LinkalhoStyle();
    auto linkalhoTheme = VisualOverrides::LinkalhoTheme();

    if (!brls::Application::init(APP_TITLE_LOWER, linkalhoStyle, linkalhoTheme))
    {
        brls::Logger::error(string("Unable to init ") + APP_TITLE);
        return EXIT_FAILURE;
    }

    brls::AppletFrame* rootFrame = new brls::AppletFrame(false, false);
    rootFrame->setTitle(string(APP_TITLE) + " v" + string(APP_VERSION));
    if (R_SUCCEEDED(accountInitialize(AccountServiceType_System))) {
        s32 acc_count = 0;
        accountGetUserCount(&acc_count);
        accountExit();
    }

    auto linkAccountsView = new LinkAccountsView(canUseLed);
    auto unlinkAccountsView = new UnlinkAccountsView(canUseLed);
    auto restoreBackupView = new RestoreBackupView(canUseLed);
    auto createBackupView = new CreateBackupView(canUseLed);
    auto countrySelectView = new CountrySelectView();
    auto accountSelectView = new AccountSelectView();

    auto operationList = new brls::List();
    {
        operationList->addView(linkAccountsView);
        operationList->addView(unlinkAccountsView);
        operationList->addView(restoreBackupView);
        operationList->addView(createBackupView);
        operationList->addView(countrySelectView);
        operationList->addView(accountSelectView);
    }
    rootFrame->setContentView(operationList);

    brls::Application::pushView(rootFrame);
    rootFrame->registerAction("", brls::Key::MINUS, []{return true;}, true);
    rootFrame->updateActionHint(brls::Key::MINUS, ""); // make the change visible

    // Run the app
    while (brls::Application::mainLoop())
        ;

    pmshellExit();
    if (canUseLed)
        hidsysExit();

    // Exit
    return EXIT_SUCCESS;
}
