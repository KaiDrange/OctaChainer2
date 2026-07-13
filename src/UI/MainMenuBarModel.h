#pragma once

#include <JuceHeader.h>
#include <functional>

class MainMenuBarModel : public juce::MenuBarModel
{
public:
    using Action = std::function<void()>;

    MainMenuBarModel(Action onQuit, Action onProjectSave, Action onProjectLoad, Action onAudioSettings, Action onHelp = {});
    juce::StringArray getMenuBarNames() override;
    juce::PopupMenu getMenuForIndex(int topLevelMenuIndex, const juce::String& menuName) override;
    void menuItemSelected(int menuItemID, int topLevelMenuIndex) override;

private:
    Action onQuit;
    Action onAudioSettings;
    Action onHelp;
    Action onProjectSave;
    Action onProjectLoad;
};
