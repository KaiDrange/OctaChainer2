#include "MainMenuBarModel.h"

MainMenuBarModel::MainMenuBarModel(Action onQuit, Action onProjectSave, Action onProjectLoad, Action onOtImport,
                                   Action onAudioSettings, Action onDefaultFolders, Action onAbout)
    : onQuit(std::move(onQuit)),
      onAudioSettings(std::move(onAudioSettings)),
      onDefaultSettings(std::move(onDefaultFolders)),
      onAbout(std::move(onAbout)),
      onProjectSave(std::move(onProjectSave)),
      onProjectLoad(std::move(onProjectLoad)),
      onOtImport(std::move(onOtImport))
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
    }

    return menu;
}

void MainMenuBarModel::menuItemSelected(const int menuItemID, int topLevelMenuIndex)
{
    juce::ignoreUnused(topLevelMenuIndex);

    if (menuItemID == 1 && onQuit) // Quit
    {
        onQuit();
    }
    else if (menuItemID == 2 && onProjectSave) // Save project
    {
        onProjectSave();
    }
    else if (menuItemID == 3 && onProjectLoad) // Load project
    {
        onProjectLoad();
    }
    else if (menuItemID == 4 && onOtImport)
    {
        onOtImport();
    }
    else if (menuItemID == 20 && onAudioSettings)
    {
        onAudioSettings();
    }
    else if (menuItemID == 21 && onDefaultSettings)
    {
        onDefaultSettings();
    }
    else if (menuItemID == 30 && onAbout)
    {
        onAbout();
    }
}
