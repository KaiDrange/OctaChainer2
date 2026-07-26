#include "VstXmlDragData.h"

bool VstXmlDragData::looksLikeAudioRegionXml(const juce::String& text)
{
    const auto lowerText = text.toLowerCase();

    return lowerText.contains("<")
        && (lowerText.contains("vst-xml") || lowerText.contains("<region"))
        && (lowerText.contains("filename") || lowerText.contains("filepath") || lowerText.contains("<file"));
}

std::vector<VstXmlDragData::AudioRegion> VstXmlDragData::parseAudioRegions(const juce::String& text)
{
    std::vector<AudioRegion> regions;

    const auto xml = parseXmlPayload(text);
    if (xml == nullptr)
        return regions;

    std::vector<const juce::XmlElement*> candidateElements;
    collectElementsWithTag(*xml, { "region" }, candidateElements);

    if (candidateElements.empty())
        collectElementsWithFilePath(*xml, candidateElements);

    for (const auto* element : candidateElements)
    {
        if (element == nullptr)
            continue;

        auto region = audioRegionFromElement(*element);
        if (region.sourceFile.getFullPathName().isNotEmpty())
            regions.push_back(std::move(region));
    }

    return regions;
}

juce::String VstXmlDragData::normaliseXmlName(juce::String name)
{
    name = name.fromLastOccurrenceOf(":", false, false);
    return name.retainCharacters("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789").toLowerCase();
}

bool VstXmlDragData::nameMatches(const juce::String& name, const NameList candidates)
{
    const auto normalisedName = normaliseXmlName(name);

    for (const auto* candidate : candidates)
        if (normalisedName == normaliseXmlName(candidate))
            return true;

    return false;
}

bool VstXmlDragData::tagMatches(const juce::XmlElement& element, const NameList candidates)
{
    return nameMatches(element.getTagName(), candidates);
}

juce::String VstXmlDragData::getAttributeIgnoringCase(const juce::XmlElement& element, const NameList names)
{
    for (int i = 0; i < element.getNumAttributes(); ++i)
        if (nameMatches(element.getAttributeName(i), names))
            return element.getAttributeValue(i).trim().unquoted();

    return {};
}

juce::String VstXmlDragData::getDirectChildText(const juce::XmlElement& element, const NameList childNames)
{
    for (const auto* child : element.getChildIterator())
        if (child != nullptr && tagMatches(*child, childNames))
            return child->getAllSubText().trim().unquoted();

    return {};
}

juce::String VstXmlDragData::getDescendantText(const juce::XmlElement& element, const NameList childNames)
{
    if (auto direct = getDirectChildText(element, childNames); direct.isNotEmpty())
        return direct;

    for (const auto* child : element.getChildIterator())
    {
        if (child == nullptr)
            continue;

        if (auto nested = getDescendantText(*child, childNames); nested.isNotEmpty())
            return nested;
    }

    return {};
}

juce::String VstXmlDragData::getXmlValue(const juce::XmlElement& element, const NameList names)
{
    if (auto attribute = getAttributeIgnoringCase(element, names); attribute.isNotEmpty())
        return attribute;

    return getDescendantText(element, names);
}

bool VstXmlDragData::tryGetInt64Value(const juce::XmlElement& element, const NameList names, juce::int64& value)
{
    const auto text = getXmlValue(element, names);
    if (text.isEmpty() || ! text.containsAnyOf("0123456789"))
        return false;

    value = text.getLargeIntValue();
    return true;
}

juce::File VstXmlDragData::fileFromVstXmlPath(juce::String path)
{
    path = path.trim().unquoted();

    if (path.startsWithIgnoreCase("file:"))
    {
        const juce::URL url(path);
        if (url.isLocalFile())
            return url.getLocalFile();
    }

    path = juce::URL::removeEscapeChars(path);

#if JUCE_WINDOWS
    if (path.length() > 2 && path[0] == '/' && path[2] == ':')
        path = path.substring(1);
#endif

    return juce::File(path);
}

void VstXmlDragData::collectElementsWithTag(const juce::XmlElement& element, const NameList tagNames,
    std::vector<const juce::XmlElement*>& matches)
{
    if (tagMatches(element, tagNames))
        matches.push_back(&element);

    for (const auto* child : element.getChildIterator())
        if (child != nullptr)
            collectElementsWithTag(*child, tagNames, matches);
}

void VstXmlDragData::collectElementsWithFilePath(const juce::XmlElement& element,
    std::vector<const juce::XmlElement*>& matches)
{
    static constexpr NameList pathNames{ "filename", "file", "filepath", "path", "source", "url", "href" };

    if (getXmlValue(element, pathNames).isNotEmpty())
        matches.push_back(&element);

    for (const auto* child : element.getChildIterator())
        if (child != nullptr)
            collectElementsWithFilePath(*child, matches);
}

std::unique_ptr<juce::XmlElement> VstXmlDragData::parseXmlPayload(juce::String text)
{
    text = text.trim();

    if (auto xml = juce::XmlDocument::parse(text))
        return xml;

    const auto xmlStart = text.indexOfChar('<');
    const auto xmlEnd = text.lastIndexOfChar('>');

    if (xmlStart >= 0 && xmlEnd > xmlStart)
        return juce::XmlDocument::parse(text.substring(xmlStart, xmlEnd + 1));

    return {};
}

VstXmlDragData::AudioRegion VstXmlDragData::audioRegionFromElement(const juce::XmlElement& element)
{
    static constexpr NameList pathNames{ "filename", "file", "filepath", "path", "source", "url", "href" };
    static constexpr NameList startNames{ "start", "samplestart", "startsample", "startsamples", "offset" };
    static constexpr NameList endNames{ "end", "sampleend", "endsample", "endsamples" };
    static constexpr NameList lengthNames{ "length", "duration", "samplecount", "numsamples", "samples" };
    static constexpr NameList nameNames{ "name", "description", "title" };

    VstXmlDragData::AudioRegion region;
    region.sourceFile = fileFromVstXmlPath(getXmlValue(element, pathNames));
    region.name = getXmlValue(element, nameNames);

    juce::int64 start = 0;
    juce::int64 end = 0;
    const auto hasStart = tryGetInt64Value(element, startNames, start);
    const auto hasEnd = tryGetInt64Value(element, endNames, end);

    if (hasStart && hasEnd && end > start)
    {
        region.sampleRange = { juce::jmax<juce::int64>(0, start), juce::jmax<juce::int64>(0, end) };
        return region;
    }

    juce::int64 length = 0;
    if (hasStart && tryGetInt64Value(element, lengthNames, length) && length > 0)
    {
        const auto clampedStart = juce::jmax<juce::int64>(0, start);
        region.sampleRange = { clampedStart, clampedStart + length };
    }

    return region;
}
