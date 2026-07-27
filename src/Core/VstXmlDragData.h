#pragma once

#include <JuceHeader.h>

using NameList = std::initializer_list<const char*>;

class VstXmlDragData
{
public:
    struct AudioRegion
    {
        juce::File sourceFile;
        juce::Range<juce::int64> sampleRange;
        juce::String name;

        [[nodiscard]] bool hasSampleRange() const noexcept { return ! sampleRange.isEmpty(); }
    };

    static bool looksLikeAudioRegionXml(const juce::String& text);
    static std::vector<AudioRegion> parseAudioRegions(const juce::String& text);

private:
    static juce::String normaliseXmlName(juce::String name);
    static bool nameMatches(const juce::String& name, NameList candidates);
    static bool tagMatches(const juce::XmlElement& element, NameList candidates);
    static juce::String getAttributeIgnoringCase(const juce::XmlElement& element, NameList names);
    static juce::String getDirectChildText(const juce::XmlElement& element, NameList childNames);
    static juce::String getDescendantText(const juce::XmlElement& element, NameList childNames);
    static juce::String getXmlValue(const juce::XmlElement& element, NameList names);
    static bool tryGetInt64Value(const juce::XmlElement& element, NameList names, juce::int64& value);
    static juce::File fileFromVstXmlPath(juce::String path);
    static void collectElementsWithTag(const juce::XmlElement& element, NameList tagNames,
                            std::vector<const juce::XmlElement*>& matches);
    static void collectElementsWithFilePath(const juce::XmlElement& element, std::vector<const juce::XmlElement*>& matches);
    static std::unique_ptr<juce::XmlElement> parseXmlPayload(juce::String text);
    static VstXmlDragData::AudioRegion audioRegionFromElement(const juce::XmlElement& element);
};
