#include "MainMenuBarModel.h"

MainMenuBarModel::MainMenuBarModel(Action onQuitAction, Action onProjectSaveAction, Action onProjectLoadAction, Action onOtImportAction,
                                   Action onAudioSettingsAction, Action onDefaultSettingsAction, Action onAboutAction,
                                   Action onOnlineManualAction, Action onOurMusicAction)
    : onQuit(std::move(onQuitAction)),
      onAudioSettings(std::move(onAudioSettingsAction)),
      onDefaultSettings(std::move(onDefaultSettingsAction)),
      onAbout(std::move(onAboutAction)),
      onOnlineManual(std::move(onOnlineManualAction)),
      onOurMusic(std::move(onOurMusicAction)),
      onProjectSave(std::move(onProjectSaveAction)),
      onProjectLoad(std::move(onProjectLoadAction)),
      onOtImport(std::move(onOtImportAction))
{
}

juce::StringArray MainMenuBarModel::getMenuBarNames()
{
    return { "File", "Options", "Help" };
}

juce::PopupMenu MainMenuBarModel::getMenuForIndex(int topLevelMenuIndex, const juce::String& menuName)
{
    juce::ignoreUnused(topLevelMenuIndex);
    juce::PopupMenu menu;

    if (menuName == "File")
    {
        menu.addItem(2, "Save project");
        menu.addItem(3, "Load project");
        menu.addItem(4, "Import OT file");
        menu.addItem(1, "Quit");
    }
    else if(menuName == "Options")
    {
        menu.addItem(20, "Audio settings");
        menu.addSeparator();
        menu.addItem(21, "Set application defaults");
    }
    else if (menuName == "Help")
    {
        menu.addItem(30, "About OctaChainer");
        menu.addSeparator();
        menu.addItem(31, "Online manual");
        menu.addItem(32, "Our music as Tic Tic");
    }

    return menu;
}

void MainMenuBarModel::menuItemSelected(const int menuItemID, int topLevelMenuIndex)
{
    juce::ignoreUnused(topLevelMenuIndex);

    if (menuItemID == 1 && onQuit)
        onQuit();
    else if (menuItemID == 2 && onProjectSave)
        onProjectSave();
    else if (menuItemID == 3 && onProjectLoad)
        onProjectLoad();
    else if (menuItemID == 4 && onOtImport)
        onOtImport();
    else if (menuItemID == 20 && onAudioSettings)
        onAudioSettings();
    else if (menuItemID == 21 && onDefaultSettings)
        onDefaultSettings();
    else if (menuItemID == 30 && onAbout)
        onAbout();
    else if (menuItemID == 31 && onOnlineManual)
        onOnlineManual();
    else if (menuItemID == 32 && onOurMusic)
        onOurMusic();
}
