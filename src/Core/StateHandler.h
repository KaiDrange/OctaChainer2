#pragma once

#include <juceheader.h>


class StateHandler
{
public:
    enum NormalizationMode { None = 0, Slice = 1, Chain = 2 };

private:
    juce::ValueTree valueTree;
};
