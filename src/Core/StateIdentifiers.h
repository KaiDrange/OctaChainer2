#pragma once

#include <JuceHeader.h>

// Root identifiers
static inline const juce::Identifier stateTypeId{"octaChainer2"};
static inline const juce::Identifier versionId{"version"};
static inline const juce::Identifier settingsId{"settings"};

// Settings identifiers
static inline const juce::Identifier bitDepthId = "bitDepth";
static inline const juce::Identifier channelsId = "channels";
static inline const juce::Identifier samplerateId = "samplerate";
static inline const juce::Identifier timestretchId = "timeStretch";
static inline const juce::Identifier loopModeId = "loopMode";
static inline const juce::Identifier triqQuantId = "trigQuant";
static inline const juce::Identifier gainId = "gain";
static inline const juce::Identifier bpmId = "bpm";
static inline const juce::Identifier normalizationId = "normalizationMode";
static inline const juce::Identifier fadeinId = "fade-in";
static inline const juce::Identifier fadeoutId = "fade-out";
static inline const juce::Identifier otFileId = "otFile";
static inline const juce::Identifier evenGridId = "evenGrid";
static inline const juce::Identifier embedMarkersId = "embedMarkers";
static inline const juce::Identifier megabreakFileCountId = "megabreakFileCount";
static inline const juce::Identifier chainMaxLengthId = "chainMaxLength";
static inline const juce::Identifier masterVolumeId = "masterVolume";
static inline const juce::Identifier defaultAudioFolderId = "defaultAudioFolder";
static inline const juce::Identifier defaultExportFolderId = "defaultExportFolder";
static inline const juce::Identifier defaultProjectFolderId = "defaultProjectFolder";

// Data identifiers
static inline const juce::Identifier dataId = "data";
static inline const juce::Identifier sliceId = "slice";
static inline const juce::Identifier selectedSliceId = "selectedSlice";
static inline const juce::Identifier sliceNameId = "sliceName";
static inline const juce::Identifier sliceSourcePathId = "sliceSourcePath";
static inline const juce::Identifier sliceChannelsId = "sliceChannels";
static inline const juce::Identifier sliceSamplerateId = "sliceSampleRate";
static inline const juce::Identifier sliceBitrateId = "sliceBitrate";
static inline const juce::Identifier sliceNumSamplesId = "sliceNumSamples";
static inline const juce::Identifier sliceAudioDataId = "sliceAudioData";
static inline const juce::Identifier sliceStartSampleId = "sliceStartSample";
static inline const juce::Identifier sliceEndSampleId = "sliceEndSample";
static inline const juce::Identifier sliceLoopStartSampleId = "sliceLoopStartSample";
static inline const juce::Identifier sliceLoopEndSampleId = "sliceLoopEndSample";
