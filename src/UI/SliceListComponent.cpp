#include "SliceListComponent.h"

SliceListComponent::SliceListComponent(const PanelComponent::Dimension& height, const PanelComponent::Dimension& width,
                                         StateHandler& stateHandlerToUse,
                                         const juce::String& title)
    : PanelComponent(height, width, title),
      stateHandler(stateHandlerToUse)
{
    configureTable();

    addAndMakeVisible(table);
    addAndMakeVisible(btnAdd);
    addAndMakeVisible(btnAddSilence);
    addAndMakeVisible(btnRemove);
    addAndMakeVisible(btnRemoveAll);
    addAndMakeVisible(chainMaxLength);

    btnAdd.onClick = [this] { showAddFileChooser(); };
    stateHandler.addListener(this);
}

SliceListComponent::~SliceListComponent()
{
    stateHandler.removeListener(this);
}

void SliceListComponent::resized()
{
    PanelComponent::resized();

    auto contentArea = innerBounds;
    auto buttonArea = contentArea.removeFromBottom(StyleSheet::defaultButtonHeight);
    table.setBounds(contentArea);

    btnAdd.setBounds(buttonArea.removeFromLeft(buttonArea.getWidth() / 5).reduced(StyleSheet::buttonMargins, StyleSheet::buttonMargins));
    btnAddSilence.setBounds(buttonArea.removeFromLeft(buttonArea.getWidth() / 4).reduced(StyleSheet::buttonMargins, StyleSheet::buttonMargins));
    btnRemove.setBounds(buttonArea.removeFromLeft(buttonArea.getWidth() / 3).reduced(StyleSheet::buttonMargins, StyleSheet::buttonMargins));
    btnRemoveAll.setBounds(buttonArea.removeFromLeft(buttonArea.getWidth() / 2).reduced(StyleSheet::buttonMargins, StyleSheet::buttonMargins));
    chainMaxLength.setBounds(buttonArea.reduced(StyleSheet::buttonMargins, StyleSheet::buttonMargins));
}

int SliceListComponent::getNumRows()
{
    return stateHandler.getNumSlices();
}

void SliceListComponent::paintRowBackground(juce::Graphics& g, const int rowNumber, int, int, const bool rowIsSelected)
{
    const auto alternateColour = getLookAndFeel().findColour (juce::ListBox::backgroundColourId).interpolatedWith (getLookAndFeel().findColour (juce::ListBox::textColourId), 0.03f);
    if (rowIsSelected)
        g.fillAll (juce::Colours::lightblue);
    else if (rowNumber % 2)
        g.fillAll (alternateColour);
}

void SliceListComponent::paintCell(juce::Graphics& g, const int rowNumber, const int columnId, const int width, const int height,
    const bool rowIsSelected)
{
    juce::ignoreUnused(rowIsSelected);

    const auto sliceTree = stateHandler.getSliceTree(rowNumber);
    if (! sliceTree.isValid())
        return;

    juce::String text;

    if (columnId == 1)
        text = sliceTree.getProperty(stateHandler.sliceNameId).toString();
    else if (columnId == 2)
        text = formatAudioformat(stateHandler, sliceTree);
    else if (columnId == 3)
        text = formatDuration(stateHandler, sliceTree);
    else if (columnId == 4)
        text = sliceTree.getProperty(stateHandler.sliceSourcePathId).toString();

    g.setColour(getLookAndFeel().findColour(juce::ListBox::textColourId));
    g.drawText(text, 4, 0, width - 8, height, juce::Justification::centredLeft, true);

    g.setColour(getLookAndFeel().findColour(juce::ListBox::backgroundColourId).contrasting(0.2f));
    g.drawVerticalLine(width - 1, 0.0f, static_cast<float>(height));
}

void SliceListComponent::selectedRowsChanged(const int lastRowSelected)
{
    stateHandler.selectSlice(lastRowSelected);
}

void SliceListComponent::stateChanged()
{
    table.updateContent();
    table.repaint();
}

void SliceListComponent::configureTable()
{
    table.setMultipleSelectionEnabled(false);
    for (const auto& column : columns)
        table.getHeader().addColumn(column.name, column.id, column.width, column.minWidth, column.maxWidth);

    table.getHeader().setStretchToFitActive(true);
}

void SliceListComponent::showAddFileChooser()
{
    fileChooser = std::make_unique<juce::FileChooser>("Add audio file",
                                                       juce::File(),
                                                       audioFileLoader.getSupportedFilePatterns());

    constexpr auto browserFlags = juce::FileBrowserComponent::openMode
                                  | juce::FileBrowserComponent::canSelectFiles
                                  | juce::FileBrowserComponent::canSelectMultipleItems;

    const juce::Component::SafePointer<SliceListComponent> safeThis(this);
    fileChooser->launchAsync(browserFlags, [safeThis](const juce::FileChooser& chooser)
    {
        if (safeThis != nullptr)
            safeThis->loadFiles(chooser.getResults());
    });
}

void SliceListComponent::loadFiles(const juce::Array<juce::File>& files)
{
    for (const auto& file : files)
    {
        juce::String errorMessage;
        const auto slice = audioFileLoader.loadFile(file, &errorMessage);

        if (slice == nullptr)
        {
            showLoadError(errorMessage);
            continue;
        }

        const auto row = stateHandler.addSlice(*slice);
        table.selectRow(row);
    }
}

void SliceListComponent::showLoadError(const juce::String& message)
{
    juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
                                           "Could not load audio file",
                                           message.isNotEmpty() ? message : "The selected file could not be loaded.");
}

juce::String SliceListComponent::formatDuration(const StateHandler& stateHandler, const juce::ValueTree& sliceTree)
{
    const auto sampleRate = static_cast<double>(sliceTree.getProperty(stateHandler.sliceSamplerateId, 0.0));
    const auto lengthInSamples = static_cast<juce::int64>(sliceTree.getProperty(stateHandler.sliceNumSamplesId, 0));

    if (sampleRate <= 0.0 || lengthInSamples <= 0)
        return {};

    return juce::String(static_cast<double>(lengthInSamples) / sampleRate, 2) + "s";
}

juce::String SliceListComponent::formatAudioformat(const StateHandler& stateHandler, const juce::ValueTree& sliceTree)
{
    const auto numChannels = static_cast<int>(sliceTree.getProperty(stateHandler.sliceChannelsId, 0));
    const auto bitrate = static_cast<int>(sliceTree.getProperty(stateHandler.sliceBitrateId, 0));
    const auto samplerate = static_cast<int>(sliceTree.getProperty(stateHandler.sliceSamplerateId, 0));
    if (bitrate <= 0 || samplerate <= 0  || numChannels <= 0)
        return {};

    juce::String channelsString;
    if (numChannels > 2)
        channelsString = juce::String(numChannels) + " ch";
    else
        channelsString = numChannels == 1 ? "Mono" : "Stereo";


    return channelsString + ", " + juce::String(samplerate/1000) + "kHz/" + juce::String(bitrate) + "bit";
}
