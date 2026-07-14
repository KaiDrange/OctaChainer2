#pragma once

#include <JuceHeader.h>

#include "../Core/StateHandler.h"
#include "AudioPanelComponent.h"
#include "SliceListComponent.h"
#include "SettingsPanelComponent.h"
#include "StyleSheet.h"
#include "WaveformComponent.h"

class MainComponent : public juce::Component,
                      private StateHandler::Listener
{
public:
    static constexpr int defaultWidth = 1536;
    static constexpr int defaultHeight = 880;
    static constexpr int maxWidth = 2560;
    static constexpr int maxHeight = 1440;
    static constexpr int minWidth = 900;
    static constexpr int minHeight = 600;

    static constexpr int sampleListWidthPercentage = 70;
    static constexpr int sampleListHeightPercentage = 75;
    static constexpr int sampleListMinHeight = minHeight;

    static constexpr int audioSectionFixedHeight = 100;

    static constexpr int panelMarginPixels = StyleSheet::panelMargins;
    static constexpr int settingsPanelFixedWidth = 300;
    static constexpr int settingsPanelFixedHeight = 534;

    static constexpr int sliceWaveformHeightPercentage = (100 - sampleListHeightPercentage) / 2;
    static constexpr int chainWaveformWidthPercentage = sliceWaveformHeightPercentage;

    MainComponent(StateHandler& stateHandlerToUse);
    ~MainComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void stateChanged() override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)

    void updateSliceWaveform();
    bool loadSelectedSliceAudio(juce::AudioBuffer<float>& destination, double& sampleRate) const;

    StyleSheet style;
    StateHandler& stateHandler;
    SliceListComponent sampleListComponent;
    SettingsPanelComponent settingsPanelComponent;
    WaveformComponent sliceWaveformComponent;
    WaveformComponent chainWaveformComponent;
    AudioPanelComponent audioPanelComponent;
};
