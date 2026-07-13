#pragma once

#include <JuceHeader.h>
#include <vector>

struct ComboBoxOption
{
    int itemId;
    std::type_index valueType{typeid(void)};
    juce::var value;
    juce::String name;
};

const std::vector<ComboBoxOption> trigQuantOptions = {
    {1, typeid(juce::String), "direct", "Trig quant direct"},
    {2, typeid(juce::String), "pattern", "Trig quant pattern"},
    {3, typeid(int), 1, "Trig quant 1"},
    {4, typeid(int), 2, "Trig quant 2"},
    {5, typeid(int), 3, "Trig quant 3"},
    {6, typeid(int), 4, "Trig quant 4"},
    {7, typeid(int), 6, "Trig quant 6"},
    {8, typeid(int), 8, "Trig quant 8"},
    {9, typeid(int), 12, "Trig quant 12"},
    {10, typeid(int), 16, "Trig quant 16"},
    {11, typeid(int), 24, "Trig quant 24"},
    {12, typeid(int), 32, "Trig quant 32"},
    {13, typeid(int), 48, "Trig quant 48"},
    {14, typeid(int), 64, "Trig quant 64"},
    {15, typeid(int), 96, "Trig quant 96"},
    {16, typeid(int), 128, "Trig quant 128"},
    {17, typeid(int), 192, "Trig quant 192"},
    {18, typeid(int), 256, "Trig quant 256"}
};

const std::vector<ComboBoxOption> timestretchOptions = {
    {1, typeid(juce::String), "off", "Timestretch off"},
    {2, typeid(juce::String), "normal", "Timestretch normal"},
    {3, typeid(juce::String), "beat", "Timestretch beat"}
};

const std::vector<ComboBoxOption> loopOptions = {
    {1, typeid(juce::String), "off", "Loop off"},
    {2, typeid(juce::String), "on", "Loop on"},
    {3, typeid(juce::String), "pingpong", "Loop pingpong"}
};

const std::vector<ComboBoxOption> normalizationOptions = {
    {1, typeid(juce::String), "none", "No normalization"},
    {2, typeid(juce::String), "slices", "Normalize slices"},
    {3, typeid(juce::String), "chain", "Normalize chain"}
};

const std::vector<ComboBoxOption> fadeinOptions = {
    {1, typeid(juce::String), "none", "No fade-in"},
    {2, typeid(int), 1, "1ms fade-in"},
    {3, typeid(int), 2, "2ms fade-in"},
    {4, typeid(int), 3, "3ms fade-in"},
    {5, typeid(int), 5, "5ms fade-in"},
    {6, typeid(int), 10, "10ms fade-in"}
};

const std::vector<ComboBoxOption> fadeoutOptions = {
    {1, typeid(juce::String), "none", "No fade-out"},
    {2, typeid(int), 1, "1ms fade-out"},
    {3, typeid(int), 2, "2ms fade-out"},
    {4, typeid(int), 3, "3ms fade-out"},
    {5, typeid(int), 5, "5ms fade-out"},
    {6, typeid(int), 10, "10ms fade-out"}
};

const std::vector<ComboBoxOption> megabreakFileCountOptions = {
    {1, typeid(int), 1, "File count: 1"},
    {2, typeid(int), 2, "File count: 2"},
    {3, typeid(int), 3, "File count: 3"},
    {4, typeid(int), 4, "File count: 4"},
    {5, typeid(int), 6, "File count: 6"},
    {6, typeid(int), 8, "File count: 8"},
    {7, typeid(int), 12, "File count: 12"},
    {8, typeid(int), 16, "File count: 16"},
    {9, typeid(int), 24, "File count: 24"},
    {10, typeid(int), 32, "File count: 32"},
    {11, typeid(int), 48, "File count: 48"},
    {12, typeid(int), 64, "File count: 64"}
};
