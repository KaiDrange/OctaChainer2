#include "MainMenuBarModel.h"

juce::StringArray MainMenuBarModel::getMenuBarNames()
{
    return { "File", "Options", "About" };
}

juce::PopupMenu MainMenuBarModel::getMenuForIndex(int topLevelMenuIndex, const juce::String& menuName)
{
    juce::PopupMenu menu;

    if (menuName == "File")
    {
        menu.addItem(1, "Quit");
    }
    else if(menuName == "Options")
    {
        menu.addItem(2, "Audio settings");
    }
    else if (menuName == "About")
    {
        menu.addItem(3, "About OctaChainer");
    }

    return menu;
}

void MainMenuBarModel::menuItemSelected(int menuItemID, int topLevelMenuIndex)
{
    if (menuItemID == 1) // Quit
    {
        juce::JUCEApplication::getInstance()->systemRequestedQuit();
    }
}
