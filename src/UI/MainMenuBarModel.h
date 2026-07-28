#pragma once

#include <JuceHeader.h>

class MainMenuBarModel : public juce::MenuBarModel
{
public:
    using Action = std::function<void()>;

    MainMenuBarModel(Action onQuit, Action onProjectSave, Action onProjectLoad, Action onOtImport,
                     Action onAudioSettings, Action onDefaultSettings, Action onAbout = {},
                     Action onOnlineManual = {}, Action onOurMusic = {});
    juce::StringArray getMenuBarNames() override;
    juce::PopupMenu getMenuForIndex(int topLevelMenuIndex, const juce::String& menuName) override;
    void menuItemSelected(int menuItemID, int topLevelMenuIndex) override;

private:
    Action onQuit;
    Action onAudioSettings;
    Action onDefaultSettings;
    Action onAbout;
    Action onOnlineManual;
    Action onOurMusic;
    Action onProjectSave;
    Action onProjectLoad;
    Action onOtImport;
};
