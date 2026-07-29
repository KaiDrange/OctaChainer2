#pragma once

#include <JuceHeader.h>

class DefaultSettingsDialogComponent final : public juce::Component
{
public:
    static constexpr int rowLabelWidth = 170;
    static constexpr int browseButtonWidth = 84;
    static constexpr int rowHeight = 28;
    static constexpr int footerHeight = 28;

    using ApplyFn = std::function<void(const juce::String&, const juce::String&, const juce::String&)>;
    using SaveDefaultsFn = std::function<void()>;

    DefaultSettingsDialogComponent(const juce::String& defaultAudioFolder,
                                  const juce::String& defaultExportFolder,
                                  const juce::String& defaultProjectFolder,
                                  ApplyFn applyFn,
                                  SaveDefaultsFn saveDefaultsFn);

    void resized() override;

private:
    enum class FolderKind
    {
        audio,
        exportFolder,
        project
    };

    struct FolderRow
    {
        juce::Label label;
        juce::TextEditor editor;
        juce::TextButton browseButton{"Browse"};
    };

    void setupRow(FolderRow& row, const juce::String& labelText, const juce::String& initialValue, FolderKind kind);
    void browseForFolder(FolderKind kind);
    void applyChanges() const;
    void closeDialog() const;
    static juce::File getBrowseStartFolder(const juce::TextEditor& editor);
    static juce::String normaliseFolderPath(const juce::String& path);
    void setFolderPath(FolderKind kind, const juce::String& path);

    FolderRow audioRow;
    FolderRow exportRow;
    FolderRow projectRow;
    juce::TextButton saveCurrentSettingsButton{"Use current settings as defaults"};
    juce::TextButton saveButton{"Save"};
    juce::TextButton cancelButton{"Cancel"};
    ApplyFn onApply;
    SaveDefaultsFn onSaveCurrentSettingsAsDefaults;
    std::unique_ptr<juce::FileChooser> folderChooser;
};
