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
    SliceListComponent::stateChanged({ StateHandler::StateChange::fullReload });
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
    return juce::jmax(1, stateHandler.getStateValue<int>(StateHandler::chainMaxLengthId,
                                                         static_cast<int>(StateHandler::chainMaxLengthValue.defaultValue)));
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
        text = sliceTree.getProperty(StateHandler::sliceNameId).toString();
    else if (columnId == 3)
        text = formatAudioFormat(sliceTree);
    else if (columnId == 4)
        text = formatDuration(sliceTree);
    else if (columnId == 5)
        text = sliceTree.getProperty(StateHandler::sliceSourcePathId).toString();

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

bool SliceListComponent::hasSupportedAudioFiles(const juce::StringArray& files) const
{
    for (const auto& filePath : files)
    {
        const juce::File file(filePath);
        if (file.existsAsFile() && isSupportedAudioFile(file))
            return true;

        if (file.isDirectory())
            return true;
    }

    return false;
}

bool SliceListComponent::isInterestedInFileDrag(const juce::StringArray& files)
{
    return hasSupportedAudioFiles(files);
}

void SliceListComponent::fileDragEnter(const juce::StringArray& files, int, int)
{
    juce::ignoreUnused(files);
}

void SliceListComponent::fileDragMove(const juce::StringArray& files, int, int)
{
    juce::ignoreUnused(files);
}

void SliceListComponent::fileDragExit(const juce::StringArray& files)
{
    juce::ignoreUnused(files);
}

void SliceListComponent::filesDropped(const juce::StringArray& files, int, int)
{
    juce::Array<juce::File> droppedFiles;
    auto remainingSlots = maxSliceCount - stateHandler.getNumSlices();

    if (remainingSlots <= 0)
    {
        showLoadError("The slice list already contains the maximum of 1000 slices.");
        return;
    }

    for (const auto& filePath : files)
    {
        collectDroppedFiles(juce::File(filePath), droppedFiles, remainingSlots);

        if (remainingSlots <= 0)
            break;
    }

    if (! droppedFiles.isEmpty())
        loadFiles(droppedFiles);
}

bool SliceListComponent::isInterestedInTextDrag(const juce::String& text)
{
    return VstXmlDragData::looksLikeAudioRegionXml(text);
}

void SliceListComponent::textDragEnter(const juce::String& text, int, int)
{
    juce::ignoreUnused(text);
}

void SliceListComponent::textDragMove(const juce::String& text, int, int)
{
    juce::ignoreUnused(text);
}

void SliceListComponent::textDragExit(const juce::String& text)
{
    juce::ignoreUnused(text);
}

void SliceListComponent::textDropped(const juce::String& text, int, int)
{
    const auto regions = VstXmlDragData::parseAudioRegions(text);

    if (regions.empty())
    {
        showLoadError("The dropped VST XML data did not contain any audio regions.");
        return;
    }

    loadAudioRegions(regions);
}

bool SliceListComponent::isSupportedAudioFile(const juce::File& file) const
{
    const auto supportedPatterns = audioFileLoader.getSupportedFilePatterns();
    juce::StringArray wildcardPatterns;
    wildcardPatterns.addTokens(supportedPatterns, ";", "");
    wildcardPatterns.trim();
    wildcardPatterns.removeEmptyStrings();

    const auto fileName = file.getFileName();
    for (const auto& pattern : wildcardPatterns)
    {
        if (fileName.matchesWildcard(pattern, true))
            return true;
    }

    return false;
}

void SliceListComponent::collectDroppedFiles(const juce::File& file, juce::Array<juce::File>& droppedFiles,
                                             int& remainingSlots) const
{
    if (remainingSlots <= 0 || ! file.exists())
        return;

    if (file.isDirectory())
    {
        const auto supportedPatterns = audioFileLoader.getSupportedFilePatterns();
        for (const auto& entry : juce::RangedDirectoryIterator(file, true, supportedPatterns, juce::File::findFiles))
        {
            if (remainingSlots <= 0)
                break;

            droppedFiles.addIfNotAlreadyThere(entry.getFile());
            --remainingSlots;
        }

        return;
    }

    if (isSupportedAudioFile(file))
    {
        droppedFiles.addIfNotAlreadyThere(file);
        --remainingSlots;
    }
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

void SliceListComponent::stateChanged(const StateHandler::StateChange& change)
{
    const bool refreshContent = change.has(StateHandler::StateChange::sliceList)
                                || change.has(StateHandler::StateChange::fullReload);
    const bool refreshSelection = change.has(StateHandler::StateChange::sliceList)
                                  || change.has(StateHandler::StateChange::selectedSlice)
                                  || change.has(StateHandler::StateChange::fullReload);
    const bool refreshChainLength = change.has(StateHandler::StateChange::settings)
                                    || change.has(StateHandler::StateChange::fullReload);

    if (! refreshContent && ! refreshSelection && ! refreshChainLength)
        return;

    if (refreshChainLength)
    {
        chainMaxLength.setValue(juce::var(stateHandler.getStateValue<int>(StateHandler::chainMaxLengthId,
                                                                          static_cast<int>(StateHandler::chainMaxLengthValue.defaultValue))));
    }

    if (refreshContent)
        table.updateContent();

    if (refreshSelection)
    {
        const auto selectedRow = stateHandler.getSelectedSliceIndex();
        if (selectedRow >= 0)
            table.selectRow(selectedRow, true, true);
        else
            table.deselectAllRows();
    }

    if (refreshContent || change.has(StateHandler::StateChange::fullReload))
    {
        const auto hasSlices = stateHandler.getNumSlices() > 0;
        btnRemove.setEnabled(hasSlices);
        btnRemoveAll.setEnabled(hasSlices);
    }

    table.repaint();
}

void SliceListComponent::numberInputChanged(NumberInputComponent* numberInput)
{
    if (numberInput == &chainMaxLength)
    {
        stateHandler.setStateValue(StateHandler::chainMaxLengthId, numberInput->getValue());
        table.repaint();
    }
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
    const auto initialFolder = getDefaultAudioFolder(stateHandler);
    fileChooser = std::make_unique<juce::FileChooser>("Add audio file",
                                                       initialFolder,
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
    auto remainingSlots = maxSliceCount - stateHandler.getNumSlices();
    int failedFiles = 0;
    juce::String firstErrorMessage;
    int lastLoadedRow = -1;

    for (const auto& file : files)
    {
        if (remainingSlots <= 0)
            break;

        juce::String errorMessage;
        const auto slices = audioFileLoader.loadFileSlices(file, &errorMessage);

        if (slices.empty())
        {
            ++failedFiles;
            if (firstErrorMessage.isEmpty())
                firstErrorMessage = errorMessage;
            continue;
        }

        for (const auto& slice : slices)
        {
            if (remainingSlots <= 0)
                break;

            lastLoadedRow = stateHandler.addSlice(*slice, nullptr, false);
            --remainingSlots;
        }
    }

    if (lastLoadedRow >= 0)
        stateHandler.selectSlice(lastLoadedRow);

    juce::String summaryMessage;
    if (failedFiles > 0)
    {
        if (failedFiles == 1)
            summaryMessage = firstErrorMessage;
        else
            summaryMessage = juce::String(failedFiles) + " files could not be loaded."
                             + (firstErrorMessage.isNotEmpty() ? " First error: " + firstErrorMessage : juce::String());
    }

    if (remainingSlots <= 0 && files.size() > 0)
    {
        if (summaryMessage.isNotEmpty())
            summaryMessage += " ";

        summaryMessage += "The maximum slice count of 1000 has been reached.";
    }

    if (summaryMessage.isNotEmpty())
        showLoadError(summaryMessage);
}

void SliceListComponent::loadAudioRegions(const std::vector<VstXmlDragData::AudioRegion>& regions)
{
    auto remainingSlots = maxSliceCount - stateHandler.getNumSlices();
    int failedRegions = 0;
    juce::String firstErrorMessage;
    int lastLoadedRow = -1;

    if (remainingSlots <= 0)
    {
        showLoadError("The slice list already contains the maximum of 1000 slices.");
        return;
    }

    for (const auto& region : regions)
    {
        if (remainingSlots <= 0)
            break;

        juce::String errorMessage;
        const auto slice = audioFileLoader.loadFileRegion(region.sourceFile, region.sampleRange, &errorMessage);

        if (slice == nullptr)
        {
            ++failedRegions;
            if (firstErrorMessage.isEmpty())
                firstErrorMessage = errorMessage;
            continue;
        }

        if (region.name.isNotEmpty())
            slice->name = region.name;
        else if (region.hasSampleRange())
            slice->name = region.sourceFile.getFileNameWithoutExtension()
                          + " [" + juce::String(region.sampleRange.getStart())
                          + "-" + juce::String(region.sampleRange.getEnd()) + "]";

        lastLoadedRow = stateHandler.addSlice(*slice, nullptr, false);
        --remainingSlots;
    }

    if (lastLoadedRow >= 0)
        stateHandler.selectSlice(lastLoadedRow);

    juce::String summaryMessage;
    if (failedRegions > 0)
    {
        if (failedRegions == 1)
            summaryMessage = firstErrorMessage;
        else
            summaryMessage = juce::String(failedRegions) + " dropped regions could not be loaded."
                             + (firstErrorMessage.isNotEmpty() ? " First error: " + firstErrorMessage : juce::String());
    }

    if (remainingSlots <= 0 && ! regions.empty())
    {
        if (summaryMessage.isNotEmpty())
            summaryMessage += " ";

        summaryMessage += "The maximum slice count of 1000 has been reached.";
    }

    if (summaryMessage.isNotEmpty())
        showLoadError(summaryMessage);
}

void SliceListComponent::showLoadError(const juce::String& message)
{
    juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
                                           "Could not load audio file",
                                           message.isNotEmpty() ? message : "The selected file could not be loaded.");
}

juce::String SliceListComponent::formatDuration(const juce::ValueTree& sliceTree)
{
    const auto sampleRate = static_cast<double>(sliceTree.getProperty(StateHandler::sliceSamplerateId, 0.0));
    const auto lengthInSamples = static_cast<juce::int64>(sliceTree.getProperty(StateHandler::sliceNumSamplesId, 0));

    if (sampleRate <= 0.0 || lengthInSamples <= 0)
        return {};

    return juce::String(static_cast<double>(lengthInSamples) / sampleRate, 2) + "s";
}

juce::String SliceListComponent::formatAudioFormat(const juce::ValueTree& sliceTree)
{
    const auto numChannels = static_cast<int>(sliceTree.getProperty(StateHandler::sliceChannelsId, 0));
    const auto bitrate = static_cast<int>(sliceTree.getProperty(StateHandler::sliceBitrateId, 0));
    const auto samplerate = static_cast<int>(sliceTree.getProperty(StateHandler::sliceSamplerateId, 0));
    if (bitrate <= 0 || samplerate <= 0  || numChannels <= 0)
        return {};

    juce::String channelsString;
    if (numChannels > 2)
        channelsString = juce::String(numChannels) + " ch";
    else
        channelsString = numChannels == 1 ? "Mono" : "Stereo";


    return channelsString + ", " + juce::String(samplerate/1000) + "kHz/" + juce::String(bitrate) + "bit";
}

juce::File SliceListComponent::getDefaultAudioFolder(const StateHandler& stateHandler)
{
    auto folder = juce::File(stateHandler.getStateValue(
        StateHandler::defaultAudioFolderId,
        juce::File::getSpecialLocation(juce::File::userHomeDirectory).getFullPathName()));

    if (! folder.isDirectory())
        folder = juce::File::getSpecialLocation(juce::File::userHomeDirectory);

    return folder;
}
