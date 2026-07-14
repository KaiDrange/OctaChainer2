#pragma once

#include <JuceHeader.h>

// Root identifiers
const juce::Identifier stateTypeId{"octaChainer2"};
const juce::Identifier versionId{"version"};
const juce::Identifier settingsId{"settings"};

// Settings identifiers
const juce::Identifier bitrateId = "bitrate";
const juce::Identifier channelsId = "channels";
const juce::Identifier samplerateId = "samplerate";
const juce::Identifier timestretchId = "timeStretch";
const juce::Identifier loopModeId = "loopMode";
const juce::Identifier triqQuantId = "trigQuant";
const juce::Identifier gainId = "gain";
const juce::Identifier bpmId = "bpm";
const juce::Identifier normalizationId = "normalizationMode";
const juce::Identifier fadeinId = "fade-in";
const juce::Identifier fadeoutId = "fade-out";
const juce::Identifier otFileId = "otFile";
const juce::Identifier evenGridId = "evenGrid";
const juce::Identifier embedMarkersId = "embedMarkers";
const juce::Identifier megabreakFileCountId = "megabreakFileCount";
const juce::Identifier masterVolumeId = "masterVolume";

// Data identifiers
const juce::Identifier dataId = "data";
const juce::Identifier sliceId = "slice";
const juce::Identifier selectedSliceId = "selectedSlice";
const juce::Identifier sliceNameId = "sliceName";
const juce::Identifier sliceSourcePathId = "sliceSourcePath";
const juce::Identifier sliceChannelsId = "sliceChannels";
const juce::Identifier sliceSamplerateId = "sliceSampleRate";
const juce::Identifier sliceBitrateId = "sliceBitrate";
const juce::Identifier sliceNumSamplesId = "sliceNumSamples";
const juce::Identifier sliceAudioDataId = "sliceAudioData";
const juce::Identifier sliceStartSampleId = "sliceStartSample";
const juce::Identifier sliceEndSampleId = "sliceEndSample";
const juce::Identifier sliceLoopStartSampleId = "sliceLoopStartSample";
const juce::Identifier sliceLoopEndSampleId = "sliceLoopEndSample";
