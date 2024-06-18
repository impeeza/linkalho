#include "views/link_accounts_view.hpp"
#include "views/confirm_view.hpp"
#include "views/worker_view.hpp"
#include "core/file_operations.hpp"
#include "core/shared_settings.hpp"

using namespace std;
using namespace brls::i18n::literals;

LinkAccountsView::LinkAccountsView(bool canUseLed) : ListItem("translations/link_accounts_view/title"_i18n)
{
    this->getClickEvent()->subscribe([canUseLed](brls::View* view) {

        if (SharedSettings::instance().getSelectedCount() == 0) {
            brls::Application::notify("translations/errors/no_accounts_selected"_i18n);
            return;
        }

        brls::StagedAppletFrame* stagedFrame = new brls::StagedAppletFrame();
        stagedFrame->setTitle("translations/link_accounts_view/title"_i18n);
#ifndef LINKALHO_DEBUG
        stagedFrame->addStage(
            new ConfirmView(stagedFrame, "translations/link_accounts_view/warning"_i18n, false, canUseLed)
        );
#endif
        stagedFrame->addStage(
            new WorkerView(stagedFrame, "translations/link_accounts_view/working"_i18n, [](){
                linkAccounts();
            })
        );
        stagedFrame->addStage(
            new ConfirmView(stagedFrame, "translations/link_accounts_view/complete"_i18n, true, canUseLed)
        );

        brls::Application::pushView(stagedFrame);
        stagedFrame->registerAction("", brls::Key::PLUS, []{return true;}, true);
        stagedFrame->updateActionHint(brls::Key::PLUS, ""); // make the change visible
        stagedFrame->registerAction("", brls::Key::B, []{return true;}, true);
        stagedFrame->updateActionHint(brls::Key::B, ""); // make the change visible
    });
}