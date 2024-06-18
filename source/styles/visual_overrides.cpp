#include "styles/visual_overrides.hpp"
#include <stdio.h>

brls::Style* VisualOverrides::LinkalhoStyle()
{
    brls::Style* style = new brls::HorizonStyle();
    style->AppletFrame.titleSize = 30;
    style->AppletFrame.titleStart = 35;
    style->List.marginLeftRight = 40;
    style->List.marginTopBottom = 38;
    style->Dialog.height = 380;
    style->Dialog.paddingLeftRight = 56;
    return style;
}

brls::LibraryViewsThemeVariantsWrapper* VisualOverrides::LinkalhoTheme()
{
    brls::Theme* custom_light = new brls::HorizonLightTheme();
    custom_light->buttonPrimaryDisabledBackgroundColor = nvgRGB(136, 90, 75);
    custom_light->buttonPrimaryDisabledTextColor = nvgRGB(167, 129, 118);
    custom_light->buttonPrimaryEnabledBackgroundColor = nvgRGB(255, 59, 0);
    custom_light->buttonPrimaryEnabledTextColor = nvgRGB(255, 247, 255);

    brls::Theme* custom_dark = new brls::HorizonDarkTheme();
    custom_dark->buttonPrimaryDisabledBackgroundColor = nvgRGB(136, 90, 75);
    custom_dark->buttonPrimaryDisabledTextColor = nvgRGB(167, 129, 118);
    custom_dark->buttonPrimaryEnabledBackgroundColor = nvgRGB(255, 59, 0);
    custom_dark->buttonPrimaryEnabledTextColor = nvgRGB(255, 247, 255);

    return new brls::LibraryViewsThemeVariantsWrapper(custom_light, custom_dark);
}
