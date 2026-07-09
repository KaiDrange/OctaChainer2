#pragma once

#include <JuceHeader.h>
#include "SampleListComponent.h"
#include "SettingsPanelComponent.h"
#include "StyleSheet.h"
#include "WaveformComponent.h"

class MainComponent : public juce::Component
{
public:
    static constexpr int defaultWidth = 1536;
    static constexpr int defaultHeight = 864;
    static constexpr int maxWidth = 2560;
    static constexpr int maxHeight = 1440;
    static constexpr int minWidth = 800;
    static constexpr int minHeight = 600;

    static constexpr int sampleListWidthPercentage = 70;
    static constexpr int sampleListHeightPercentage = 75;

    static constexpr int settingsPanelHeightPercentage = sampleListHeightPercentage;
    static constexpr int settingsPanelWidthPercentage = 100 - sampleListWidthPercentage;

    static constexpr int sliceWaveformHeightPercentage = (100 - sampleListHeightPercentage) / 2;
    static constexpr int chainWaveformWidthPercentage = sliceWaveformHeightPercentage;

    MainComponent();
    ~MainComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)

    StyleSheet style;
    SampleListComponent sampleListComponent;
    SettingsPanelComponent settingsPanelComponent;
    WaveformComponent sliceWaveformComponent;
    WaveformComponent chainWaveformComponent;
};
