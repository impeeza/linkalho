#include "views/create_backup_view.hpp"
#include "views/confirm_view.hpp"
#include "views/worker_view.hpp"
#include "core/file_operations.hpp"

using namespace std;
using namespace brls::i18n::literals;

CreateBackupView::CreateBackupView(bool canUseLed) : ListItem("translations/create_backup_view/title"_i18n)
{
    this->getClickEvent()->subscribe([canUseLed](brls::View* view) {
        brls::StagedAppletFrame* stagedFrame = new brls::StagedAppletFrame();
        stagedFrame->setTitle("Create manual backup");

        stagedFrame->addStage(
            new ConfirmView(stagedFrame, "translations/create_backup_view/warning"_i18n, false, canUseLed)
        );
        stagedFrame->addStage(
            new WorkerView(stagedFrame, "translations/create_backup_view/working"_i18n, [](){
                manualBackup();
            })
        );
        stagedFrame->addStage(
            new ConfirmView(stagedFrame, "translations/create_backup_view/complete"_i18n, true, canUseLed)
        );

        brls::Application::pushView(stagedFrame);
        stagedFrame->registerAction("", brls::Key::PLUS, []{return true;}, true);
        stagedFrame->updateActionHint(brls::Key::PLUS, ""); // make the change visible
        stagedFrame->registerAction("", brls::Key::B, []{return true;}, true);
        stagedFrame->updateActionHint(brls::Key::B, ""); // make the change visible
    });
}