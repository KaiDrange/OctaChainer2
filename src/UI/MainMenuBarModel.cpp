#include "MainMenuBarModel.h"

#include <utility>

MainMenuBarModel::MainMenuBarModel(Action onQuit, Action onProjectSave, Action onProjectLoad, Action onAudioSettings, Action onHelp)
    : onQuit(std::move(onQuit)),
      onProjectSave(std::move(onProjectSave)),
      onProjectLoad(std::move(onProjectLoad)),
      onAudioSettings(std::move(onAudioSettings)),
      onHelp(std::move(onHelp))
{
}

juce::StringArray MainMenuBarModel::getMenuBarNames()
{
    return { "File", "Options", "Help" };
}

juce::PopupMenu MainMenuBarModel::getMenuForIndex(int topLevelMenuIndex, const juce::String& menuName)
{
    juce::PopupMenu menu;

    if (menuName == "File")
    {
        menu.addItem(2, "Save project");
        menu.addItem(3, "Load project");
        menu.addItem(1, "Quit");
    }
    else if(menuName == "Options")
    {
        menu.addItem(20, "Audio settings");
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
    else if (menuItemID == 20 && onAudioSettings)
    {
        onAudioSettings();
    }
    else if (menuItemID == 30 && onHelp)
    {
        onHelp();
    }
}
