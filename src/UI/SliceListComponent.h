#pragma once

#include <JuceHeader.h>
#include "../Core/AudioFileLoader.h"
#include "../Core/StateHandler.h"
#include "../Core/VstXmlDragData.h"
#include "NumberInputComponent.h"
#include "PanelComponent.h"

class SliceListComponent : public PanelComponent,
                           public juce::TableListBoxModel,
                           public juce::DragAndDropTarget,
                           public juce::FileDragAndDropTarget,
                           public juce::TextDragAndDropTarget,
                           StateHandler::Listener,
                           NumberInputComponent::Listener
{
public:
    SliceListComponent(const PanelComponent::Dimension& height, const PanelComponent::Dimension& width,
                        StateHandler& stateHandlerToUse,
                        const juce::String& title = "");
    ~SliceListComponent() override;
    void resized() override;
    void stateChanged(const StateHandler::StateChange& change) override;
    void numberInputChanged(NumberInputComponent* numberInput) override;

    int getNumRows() override;
    void paintRowBackground(juce::Graphics& g, int rowNumber, int /*width*/, int /*height*/,
                            bool rowIsSelected) override;
    void paintCell(juce::Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override;
    void selectedRowsChanged(int lastRowSelected) override;
    juce::var getDragSourceDescription(const juce::SparseSet<int>& currentlySelectedRows) override;

    bool isInterestedInDragSource(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails) override;
    void itemDragEnter(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails) override;
    void itemDragMove(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails) override;
    void itemDragExit(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails) override;
    void itemDropped(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails) override;

    bool isInterestedInFileDrag(const juce::StringArray& files) override;
    void fileDragEnter(const juce::StringArray& files, int x, int y) override;
    void fileDragMove(const juce::StringArray& files, int x, int y) override;
    void fileDragExit(const juce::StringArray& files) override;
    void filesDropped(const juce::StringArray& files, int x, int y) override;
    bool isInterestedInTextDrag(const juce::String& text) override;
    void textDragEnter(const juce::String& text, int x, int y) override;
    void textDragMove(const juce::String& text, int x, int y) override;
    void textDragExit(const juce::String& text) override;
    void textDropped(const juce::String& text, int x, int y) override;

private:
    static constexpr int maxSliceCount = 1000;

    struct Column
    {
        int id;
        juce::String name;
        int width;
        int minWidth;
        int maxWidth;
    };

    const std::vector<Column> columns = {
        {1, "#", 40, 40, 40},
        {2, "Slice", 200, 120, 400},
        {3, "Format", 90, 60, 160},
        {4, "Length", 90, 0, 160},
        {5, "Source path", 280, 0, 800}
    };

    void configureTable();
    int getChainGroupSize() const;
    int getDragInsertionIndex(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails) const;
    bool isRowDragFromThisTable(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails) const;
    bool hasSupportedAudioFiles(const juce::StringArray& files) const;
    bool isSupportedAudioFile(const juce::File& file) const;
    void collectDroppedFiles(const juce::File& file, juce::Array<juce::File>& droppedFiles, int& remainingSlots) const;
    juce::Colour getRowBackgroundColour(int rowNumber, bool rowIsSelected) const;
    void clearDragIndicator();
    void showAddFileChooser();
    void loadFiles(const juce::Array<juce::File>& files);
    void loadAudioRegions(const std::vector<VstXmlDragData::AudioRegion>& regions);
    static void showLoadError(const juce::String& message);
    static juce::String formatDuration(const juce::ValueTree& sliceTree);
    static juce::String formatAudioFormat(const juce::ValueTree& sliceTree);
    static juce::File getDefaultAudioFolder(const StateHandler& stateHandler);

    StateHandler& stateHandler;
    AudioFileLoader audioFileLoader;
    std::unique_ptr<juce::FileChooser> fileChooser;

    juce::TableListBox table{"Sample List", this};
    int dragInsertIndex = -1;
    juce::TextButton btnAdd{"Load file(s)"};
    juce::TextButton btnAddSilence{"Add blank"};
    juce::TextButton btnRemove{"Remove"};
    juce::TextButton btnRemoveAll{"Clear"};
    NumberInputComponent chainMaxLength{
        "Slices per chain:", StateHandler::chainMaxLengthValue.min, StateHandler::chainMaxLengthValue.max,
        StateHandler::chainMaxLengthValue.defaultValue, StateHandler::chainMaxLengthValue.stepSize, false
    };
};
