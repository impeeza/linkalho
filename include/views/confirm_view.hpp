#pragma once

#include <borealis.hpp>
#include <chrono>

class ConfirmView : public brls::View
{
  private:
    brls::Button* button = nullptr;
    brls::Label* label = nullptr;
    std::chrono::system_clock::time_point start = std::chrono::high_resolution_clock::now();
    bool reboot = false;
    bool isLastStage = false;
    bool canUseLed = false;
    brls::StagedAppletFrame* frame = nullptr;

  public:
    ConfirmView(brls::StagedAppletFrame* frame, const std::string& text, bool reboot, bool canUseLed);
    ~ConfirmView();

    void draw(NVGcontext* vg, int x, int y, unsigned width, unsigned height, brls::Style* style, brls::FrameContext* ctx) override;
    void layout(NVGcontext* vg, brls::Style* style, brls::FontStash* stash) override;
    brls::View* getDefaultFocus() override;
    void willDisappear(bool resetState = false) override;
};
