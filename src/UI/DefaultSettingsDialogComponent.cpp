#include "DefaultSettingsDialogComponent.h"
#include "StyleSheet.h"

DefaultSettingsDialogComponent::DefaultSettingsDialogComponent(const juce::String& defaultAudioFolder,
                                                             const juce::String& defaultExportFolder,
                                                             const juce::String& defaultProjectFolder,
                                                             ApplyFn applyFn,
                                                             SaveDefaultsFn saveDefaultsFn)
    : onApply(std::move(applyFn)),
      onSaveCurrentSettingsAsDefaults(std::move(saveDefaultsFn))
{
    setSize(700, 276);

    setupRow(audioRow, "Default audio folder", defaultAudioFolder, FolderKind::audio);
    setupRow(exportRow, "Default export folder", defaultExportFolder, FolderKind::exportFolder);
    setupRow(projectRow, "Default project folder", defaultProjectFolder, FolderKind::project);

    addAndMakeVisible(saveCurrentSettingsButton);
    addAndMakeVisible(saveButton);
    addAndMakeVisible(cancelButton);

    saveCurrentSettingsButton.onClick = [this]
    {
        if (this->onSaveCurrentSettingsAsDefaults)
            this->onSaveCurrentSettingsAsDefaults();
    };

    saveButton.onClick = [this]
    {
        applyChanges();
    };

    cancelButton.onClick = [this]
    {
        closeDialog();
    };
}

void DefaultSettingsDialogComponent::setupRow(FolderRow& row, const juce::String& labelText, const juce::String& initialValue,
                                              const FolderKind kind)
{
    row.label.setText(labelText, juce::dontSendNotification);
    row.label.setFont(StyleSheet::getControlFont());
    row.label.setJustificationType(juce::Justification::centredLeft);

    row.editor.setText(initialValue);
    row.editor.setScrollbarsShown(true);
    row.editor.setMultiLine(false);
    row.editor.setReturnKeyStartsNewLine(false);
    row.editor.setPopupMenuEnabled(true);
    row.editor.setIndents(StyleSheet::controlTextInsetX, StyleSheet::controlTextInsetY);

    row.browseButton.onClick = [this, kind]
    {
        browseForFolder(kind);
    };

    addAndMakeVisible(row.label);
    addAndMakeVisible(row.editor);
    addAndMakeVisible(row.browseButton);
}

void DefaultSettingsDialogComponent::resized()
{
    auto area = getLocalBounds().reduced(StyleSheet::panelMargins);
    auto layoutRow = [](FolderRow& row, juce::Rectangle<int>& rowArea)
    {
        row.label.setBounds(rowArea.removeFromLeft(rowLabelWidth));
        rowArea.removeFromLeft(StyleSheet::controlGap);
        row.browseButton.setBounds(rowArea.removeFromRight(browseButtonWidth));
        row.editor.setBounds(rowArea);
    };

    auto audioArea = area.removeFromTop(rowHeight);
    layoutRow(audioRow, audioArea);

    area.removeFromTop(StyleSheet::controlGap);

    auto exportArea = area.removeFromTop(rowHeight);
    layoutRow(exportRow, exportArea);

    area.removeFromTop(StyleSheet::controlGap);

    auto projectArea = area.removeFromTop(rowHeight);
    layoutRow(projectRow, projectArea);

    area.removeFromTop(StyleSheet::controlGap);
    saveCurrentSettingsButton.setBounds(area.removeFromTop(footerHeight));

    area.removeFromTop(StyleSheet::controlGap);
    auto footerArea = area.removeFromTop(footerHeight);
    const auto footerButtonWidth = (footerArea.getWidth() - StyleSheet::controlGap) / 2;
    saveButton.setBounds(footerArea.removeFromLeft(footerButtonWidth));
    footerArea.removeFromLeft(StyleSheet::controlGap);
    cancelButton.setBounds(footerArea);
}

void DefaultSettingsDialogComponent::browseForFolder(const FolderKind kind)
{
    const juce::TextEditor* editor = &projectRow.editor;
    switch (kind)
    {
        case FolderKind::audio:
            editor = &audioRow.editor;
            break;
        case FolderKind::exportFolder:
            editor = &exportRow.editor;
            break;
        case FolderKind::project:
            break;
    }
    const juce::Component::SafePointer<DefaultSettingsDialogComponent> safeThis(this);

    folderChooser = std::make_unique<juce::FileChooser>("Select folder", getBrowseStartFolder(*editor));
    constexpr auto browserFlags = juce::FileBrowserComponent::openMode
                                  | juce::FileBrowserComponent::canSelectDirectories;

    folderChooser->launchAsync(browserFlags, [safeThis, kind](const juce::FileChooser& chooser)
    {
        if (safeThis == nullptr)
            return;

        const auto result = chooser.getResult();
        if (result.exists())
        {
            const auto folder = result.isDirectory() ? result : result.getParentDirectory();
            if (folder.exists())
                safeThis->setFolderPath(kind, folder.getFullPathName());
        }
    });
}

void DefaultSettingsDialogComponent::applyChanges() const
{
    if (onApply)
        onApply(normaliseFolderPath(audioRow.editor.getText()),
                normaliseFolderPath(exportRow.editor.getText()),
                normaliseFolderPath(projectRow.editor.getText()));

    closeDialog();
}

void DefaultSettingsDialogComponent::closeDialog() const
{
    if (auto* dialog = findParentComponentOfClass<juce::DialogWindow>())
        dialog->exitModalState(0);
}

juce::File DefaultSettingsDialogComponent::getBrowseStartFolder(const juce::TextEditor& editor)
{
    auto folder = juce::File(editor.getText().trim());
    if (!folder.isDirectory())
        folder = folder.getParentDirectory();

    if (!folder.isDirectory())
        folder = juce::File::getSpecialLocation(juce::File::userHomeDirectory);

    return folder;
}

juce::String DefaultSettingsDialogComponent::normaliseFolderPath(const juce::String& path)
{
    const auto trimmed = path.trim();
    if (trimmed.isEmpty())
        return juce::File::getSpecialLocation(juce::File::userHomeDirectory).getFullPathName();

    return juce::File(trimmed).getFullPathName();
}

void DefaultSettingsDialogComponent::setFolderPath(const FolderKind kind, const juce::String& path)
{
    if (kind == FolderKind::audio)
    {
        audioRow.editor.setText(path);
        return;
    }

    if (kind == FolderKind::exportFolder)
    {
        exportRow.editor.setText(path);
        return;
    }

    projectRow.editor.setText(path);
}
