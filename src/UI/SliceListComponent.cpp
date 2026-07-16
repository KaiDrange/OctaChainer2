#include "SliceListComponent.h"

SliceListComponent::SliceListComponent(const PanelComponent::Dimension& height, const PanelComponent::Dimension& width,
                                         StateHandler& stateHandlerToUse,
                                         const juce::String& title)
    : PanelComponent(height, width, title),
      stateHandler(stateHandlerToUse)
{
    backgroundColour = StyleSheet::getSliceListBackgroundColour();
    borderColour = StyleSheet::getSliceListBorderColour();

    configureTable();

    addAndMakeVisible(table);
    addAndMakeVisible(btnAdd);
    addAndMakeVisible(btnAddSilence);
    addAndMakeVisible(btnRemove);
    addAndMakeVisible(btnRemoveAll);
    addAndMakeVisible(chainMaxLength);
    chainMaxLength.setLabelColour(StyleSheet::getSliceListTextColour());

    btnAdd.onClick = [this] { showAddFileChooser(); };
    btnRemove.onClick = [this] { stateHandler.removeSelectedSlice(); };
    btnRemoveAll.onClick = [this] { stateHandler.removeAllSlices(); };
    btnAddSilence.onClick = [this] { stateHandler.addBlankSlice(22050); };
    stateHandler.addListener(this);
    chainMaxLength.addListener(this);

    btnRemove.setEnabled(stateHandler.getNumSlices() > 0);
    btnRemoveAll.setEnabled(stateHandler.getNumSlices() > 0);
}

SliceListComponent::~SliceListComponent()
{
    chainMaxLength.removeListener(this);
    stateHandler.removeListener(this);
}

void SliceListComponent::resized()
{
    PanelComponent::resized();

    auto contentArea = innerBounds;
    auto buttonArea = contentArea.removeFromBottom(StyleSheet::defaultButtonHeight);
    table.setBounds(contentArea);

    table.getHeader().setColumnVisible(5, contentArea.getWidth() > 700);

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

int SliceListComponent::getChainGroupSize() const
{
    const auto value = chainMaxLength.getValue();
    const auto groupSize = value.isVoid() ? static_cast<int>(StateHandler::chainMaxLengthValue.defaultValue)
                                          : juce::roundToInt(static_cast<double>(value));

    return juce::jmax(1, groupSize);
}

juce::Colour SliceListComponent::getRowBackgroundColour(const int rowNumber, const bool rowIsSelected) const
{
    const auto groupSize = getChainGroupSize();
    const auto groupIndex = juce::jmax(0, rowNumber) / groupSize;
    const auto baseColour = StyleSheet::getSliceListRowColour((groupIndex % 2) != 0);

    if (! rowIsSelected)
        return baseColour;

    return StyleSheet::getSliceListSelectedRowColour().interpolatedWith(baseColour, 0.18f);
}

void SliceListComponent::paintRowBackground(juce::Graphics& g, const int rowNumber, int, int, const bool rowIsSelected)
{
    const auto clipBounds = g.getClipBounds();
    const auto rowBackgroundColour = getRowBackgroundColour(rowNumber, rowIsSelected);
    g.setColour(rowBackgroundColour);
    g.fillRect(clipBounds);

    const auto separatorColour = StyleSheet::getSliceListDividerColour().withAlpha(rowIsSelected ? 0.82f : 0.56f);
    const auto groupSize = getChainGroupSize();
    const auto isGroupStart = (rowNumber % groupSize) == 0;
    const auto isGroupEnd = ((rowNumber + 1) % groupSize) == 0 || rowNumber == getNumRows() - 1;

    if (isGroupStart)
    {
        g.setColour(separatorColour);
        g.fillRect(0, 0, clipBounds.getWidth(), 1);
    }

    if (isGroupEnd)
    {
        g.setColour(separatorColour);
        g.fillRect(0, juce::jmax(0, clipBounds.getHeight() - 1), clipBounds.getWidth(), 1);
    }

    if (dragInsertIndex == rowNumber)
    {
        g.setColour(StyleSheet::getSliceListDragIndicatorColour().withAlpha(0.70f));
        g.fillRect(0, 0, g.getClipBounds().getWidth(), 2);
    }

    if (dragInsertIndex == getNumRows() && rowNumber == getNumRows() - 1)
    {
        g.setColour(StyleSheet::getSliceListDragIndicatorColour().withAlpha(0.70f));
        g.fillRect(0, juce::jmax(0, g.getClipBounds().getHeight() - 2), g.getClipBounds().getWidth(), 2);
    }
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
        text = juce::String(rowNumber + 1);
    else if (columnId == 2)
        text = sliceTree.getProperty(stateHandler.sliceNameId).toString();
    else if (columnId == 3)
        text = formatAudioFormat(stateHandler, sliceTree);
    else if (columnId == 4)
        text = formatDuration(stateHandler, sliceTree);
    else if (columnId == 5)
        text = sliceTree.getProperty(stateHandler.sliceSourcePathId).toString();

    g.setColour(StyleSheet::getSliceListTextColour());
    g.setFont(StyleSheet::getControlFont());
    g.drawText(text, 6, 0, width - 12, height, columnId == 1 ? juce::Justification::centredRight : juce::Justification::centredLeft, true);

    g.setColour(StyleSheet::getSliceListDividerColour().withAlpha(0.55f));
    g.drawVerticalLine(width - 1, 0.0f, static_cast<float>(height));
}

void SliceListComponent::selectedRowsChanged(const int lastRowSelected)
{
    stateHandler.selectSlice(lastRowSelected);
}

juce::var SliceListComponent::getDragSourceDescription(const juce::SparseSet<int>& currentlySelectedRows)
{
    if (currentlySelectedRows.size() != 1)
        return {};

    const auto row = currentlySelectedRows[0];
    if (! juce::isPositiveAndBelow(row, stateHandler.getNumSlices()))
        return {};

    return "slice-row:" + juce::String(row);
}

bool SliceListComponent::isRowDragFromThisTable(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails) const
{
    if (dragSourceDetails.sourceComponent.get() != &table)
        return false;

    const auto description = dragSourceDetails.description.toString();
    return description.startsWith("slice-row:");
}

int SliceListComponent::getDragInsertionIndex(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails) const
{
    if (! table.getBounds().contains(dragSourceDetails.localPosition))
        return -1;

    const auto localPositionInTable = dragSourceDetails.localPosition - table.getPosition();
    return table.getInsertionIndexForPosition(localPositionInTable.x, localPositionInTable.y);
}

bool SliceListComponent::isInterestedInDragSource(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails)
{
    return isRowDragFromThisTable(dragSourceDetails);
}

void SliceListComponent::itemDragEnter(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails)
{
    itemDragMove(dragSourceDetails);
}

void SliceListComponent::itemDragMove(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails)
{
    const auto newInsertIndex = getDragInsertionIndex(dragSourceDetails);
    if (dragInsertIndex != newInsertIndex)
    {
        dragInsertIndex = newInsertIndex;
        table.repaint();
    }
}

void SliceListComponent::itemDragExit(const juce::DragAndDropTarget::SourceDetails&)
{
    clearDragIndicator();
}

void SliceListComponent::itemDropped(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails)
{
    const auto sourceRow = dragSourceDetails.description.toString()
                           .fromFirstOccurrenceOf("slice-row:", false, false)
                           .getIntValue();
    const auto insertionIndex = dragInsertIndex >= 0 ? dragInsertIndex : getDragInsertionIndex(dragSourceDetails);

    clearDragIndicator();

    if (sourceRow < 0 || insertionIndex < 0)
        return;

    if (stateHandler.moveSlice(sourceRow, insertionIndex))
    {
        const auto selectedRow = stateHandler.getSelectedSliceIndex();
        if (selectedRow >= 0)
            table.selectRow(selectedRow, true, true);
        else
            table.deselectAllRows();
    }
}

void SliceListComponent::clearDragIndicator()
{
    if (dragInsertIndex >= 0)
        table.repaint();

    dragInsertIndex = -1;
}

void SliceListComponent::stateChanged()
{
    table.updateContent();
    const auto selectedRow = stateHandler.getSelectedSliceIndex();
    if (selectedRow >= 0)
        table.selectRow(selectedRow, true, true);
    else
        table.deselectAllRows();

    btnRemove.setEnabled(stateHandler.getNumSlices() > 0);
    btnRemoveAll.setEnabled(stateHandler.getNumSlices() > 0);

    table.repaint();
}

void SliceListComponent::numberInputChanged(NumberInputComponent* numberInput)
{
    if (numberInput == &chainMaxLength)
        table.repaint();
}

void SliceListComponent::configureTable()
{
    table.setMultipleSelectionEnabled(false);
    table.setRowHeight(25);
    table.setHeaderHeight(26);
    table.setOutlineThickness(1);
    table.setColour(juce::ListBox::backgroundColourId, backgroundColour.darker(0.03f));
    table.setColour(juce::ListBox::outlineColourId, borderColour);
    table.setColour(juce::ListBox::textColourId, StyleSheet::getSliceListTextColour());

    auto& header = table.getHeader();
    header.setColour(juce::TableHeaderComponent::backgroundColourId, StyleSheet::getSliceListHeaderBackgroundColour());
    header.setColour(juce::TableHeaderComponent::textColourId, StyleSheet::getSliceListTextColour());
    header.setColour(juce::TableHeaderComponent::outlineColourId, borderColour);
    header.setColour(juce::TableHeaderComponent::highlightColourId,
                     StyleSheet::getSliceListDragIndicatorColour().withAlpha(0.22f));

    for (const auto& column : columns)
        header.addColumn(column.name, column.id, column.width, column.minWidth, column.maxWidth);

    header.setStretchToFitActive(true);
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

juce::String SliceListComponent::formatAudioFormat(const StateHandler& stateHandler, const juce::ValueTree& sliceTree)
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
