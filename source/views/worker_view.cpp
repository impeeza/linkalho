#include "views/worker_view.hpp"
#include "utils/progress_event.hpp"
#include <math.h>
#include <functional>

WorkerView::WorkerView(brls::StagedAppletFrame* frame, const std::string& warning, worker_func_t workerFunc): frame(frame), workerFunc(workerFunc)
{

    this->progressDisp = new brls::ProgressDisplay();
    this->progressDisp->setParent(this);

    this->label = new brls::Label(brls::LabelStyle::DIALOG, warning, true);
    this->label->setHorizontalAlign(NVG_ALIGN_CENTER);
    this->label->setParent(this);

    this->button = new brls::Button(brls::ButtonStyle::BORDERLESS);  // avoid back button bug
    this->button->setParent(this);
}

void WorkerView::draw(NVGcontext* vg, int x, int y, unsigned width, unsigned height, brls::Style* style, brls::FrameContext* ctx)
{
    if (!this->workStarted)
    {
        this->workStarted = true;
        workerThread = new std::thread(&WorkerView::doWork, this);
    }
    else if (ProgressEvent::instance().finished())
    {
        frame->nextStage();
    }
    else
    {
        this->progressDisp->setProgress(ProgressEvent::instance().increment(), ProgressEvent::instance().getMax());
        this->progressDisp->frame(ctx);

        this->label->frame(ctx);
    }
}

void WorkerView::doWork()
{
    if (this->workerFunc)
        this->workerFunc();
    //frame->nextStage();
}

brls::View* WorkerView::getDefaultFocus()
{
    return this->button;
}

void WorkerView::layout(NVGcontext* vg, brls::Style* style, brls::FontStash* stash)
{
    this->label->setWidth(roundf((float)this->width * style->CrashFrame.labelWidth));
    //this->label->invalidate(true);

    this->label->setBoundaries(
        this->x + this->width / 2 - this->label->getWidth() / 2,
        this->y + (this->height - style->AppletFrame.footerHeight) / 2,
        this->label->getWidth(),
        this->label->getHeight());

    this->progressDisp->setBoundaries(
        this->x + this->width / 2 - style->CrashFrame.buttonWidth,
        this->y + this->height / 2,
        style->CrashFrame.buttonWidth * 2,
        style->CrashFrame.buttonHeight);

    // this->button->setBoundaries(
    //     this->x,
    //     this->y,
    //     0,
    //     0);
}

void WorkerView::willAppear(bool resetState)
{
    this->progressDisp->willAppear(resetState);
}

void WorkerView::willDisappear(bool resetState)
{
    this->progressDisp->willDisappear(resetState);
}

WorkerView::~WorkerView()
{
    if (this->workStarted && workerThread->joinable())
    {
        this->workerThread->join();
        if (this->workerThread)
            delete this->workerThread;
    }

    delete this->progressDisp;
    delete this->label;
    delete this->button;
}
