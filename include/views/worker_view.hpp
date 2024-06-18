#pragma once

#include <borealis.hpp>
#include <thread>
#include <functional>

typedef std::function<void()> worker_func_t;

class WorkerView : public brls::View
{
  private:
    brls::Label* label;
    brls::ProgressDisplay* progressDisp = nullptr;
    brls::StagedAppletFrame* frame;
    brls::Button* button;
    int progressValue = 0;
    bool workStarted = false;
    std::thread* workerThread;
    worker_func_t workerFunc;

  public:
    WorkerView(brls::StagedAppletFrame* frame, const std::string& warning, worker_func_t workerFunc);
    ~WorkerView();

    void draw(NVGcontext* vg, int x, int y, unsigned width, unsigned height, brls::Style* style, brls::FrameContext* ctx) override;
    void layout(NVGcontext* vg, brls::Style* style, brls::FontStash* stash) override;
    brls::View* getDefaultFocus() override;
    void doWork();

    void willAppear(bool resetState = false) override;
    void willDisappear(bool resetState = false) override;
};
