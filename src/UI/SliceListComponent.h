#pragma once

#include <JuceHeader.h>

#include "../Core/AudioFileLoader.h"
#include "../Core/StateHandler.h"
#include "NumberInputComponent.h"
#include "PanelComponent.h"

class SliceListComponent : public PanelComponent,
                            public juce::TableListBoxModel,
                            private StateHandler::Listener
{
public:
    SliceListComponent(const PanelComponent::Dimension& height, const PanelComponent::Dimension& width,
                        StateHandler& stateHandlerToUse,
                        const juce::String& title = "");
    ~SliceListComponent() override;
    void resized() override;
    void stateChanged() override;

    int getNumRows() override;
    void paintRowBackground(juce::Graphics& g, int rowNumber, int /*width*/, int /*height*/,
                            bool rowIsSelected) override;
    void paintCell(juce::Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override;

private:
    struct Column
    {
        int id;
        juce::String name;
        int width;
        int minWidth;
        int maxWidth;
    };

    const std::vector<Column> columns = {
        {1, "Slice", 200, 120, 400},
        {2, "Format", 90, 60, 160},
        {3, "Length", 90, 0, 160},
        {4, "Source path", 280, 0, 800}
    };

    void configureTable();
    void showAddFileChooser();
    void loadFiles(const juce::Array<juce::File>& files);
    static void showLoadError(const juce::String& message);
    static juce::String formatDuration(const StateHandler& stateHandler, const juce::ValueTree& sliceTree);
    static juce::String formatAudioformat(const StateHandler& stateHandler, const juce::ValueTree& sliceTree);

    StateHandler& stateHandler;
    AudioFileLoader audioFileLoader;
    std::unique_ptr<juce::FileChooser> fileChooser;

    juce::TableListBox table{"Sample List", this};
    juce::TextButton btnAdd{"Add file"};
    juce::TextButton btnAddSilence{"Add blank"};
    juce::TextButton btnRemove{"Remove"};
    juce::TextButton btnRemoveAll{"Clear"};
    NumberInputComponent chainMaxLength{
        "Max slices:", StateHandler::chainMaxLengthValue.min, StateHandler::chainMaxLengthValue.max,
        StateHandler::chainMaxLengthValue.defaultValue, StateHandler::chainMaxLengthValue.stepSize, false
    };
};
