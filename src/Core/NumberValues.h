# pragma once

struct NumberValue
{
    double min;
    double max;
    double defaultValue;
    double stepSize;
};

static const inline NumberValue gainValue = { -24, 24, 0, 0.5f };
static const inline NumberValue bpmValue = { 30, 300, 120, 0.25f };
static const inline NumberValue chainMaxLengthValue = { 1, 120, 64, 1 };
