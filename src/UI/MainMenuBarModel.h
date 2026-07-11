#pragma once

#include <JuceHeader.h>

class MainMenuBarModel : public juce::MenuBarModel
{
public:
    MainMenuBarModel() = default;
    juce::StringArray getMenuBarNames() override;
    juce::PopupMenu getMenuForIndex(int topLevelMenuIndex, const juce::String& menuName) override;
    void menuItemSelected(int menuItemID, int topLevelMenuIndex) override;
};
