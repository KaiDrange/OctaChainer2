#pragma once

#include <JuceHeader.h>

class MainMenuBarModel : public juce::MenuBarModel
{
public:
    using Action = std::function<void()>;

    MainMenuBarModel(Action onQuitAction, Action onProjectSaveAction, Action onProjectLoadAction, Action onOtImportAction,
                     Action onAudioSettingsAction, Action onDefaultSettingsAction, Action onAboutAction = {},
                     Action onOnlineManualAction = {}, Action onOurMusicAction = {});
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
