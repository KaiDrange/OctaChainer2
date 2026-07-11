#include "TransportButtonComponent.h"

#include "StyleSheet.h"

TransportButtonComponent::TransportButtonComponent(const juce::String& labelText)
    : button("Play " + labelText, juce::DrawableButton::ImageFitted)
{
    addAndMakeVisible(label);
    addAndMakeVisible(button);

    label.setText(labelText, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    label.setInterceptsMouseClicks(false, false);

    button.setClickingTogglesState(true);
    button.setToggleState(false, juce::dontSendNotification);
    button.setButtonStyle(juce::DrawableButton::ImageOnButtonBackground);
    button.setColour(juce::TextButton::buttonColourId, juce::Colour(StyleSheet::controlBackgroundColour));
    button.setColour(juce::TextButton::buttonOnColourId, juce::Colour(StyleSheet::controlBackgroundColour));
    button.setImages(createPlayIcon(juce::Colour(0xFF2EAF58)).get(),
                     nullptr,
                     nullptr,
                     nullptr,
                     createStopIcon(juce::Colour(0xFFD94B4B)).get(),
                     nullptr,
                     nullptr,
                     nullptr);
    button.setEdgeIndent(8);
}

juce::DrawableButton& TransportButtonComponent::getButton() noexcept
{
    return button;
}

const juce::DrawableButton& TransportButtonComponent::getButton() const noexcept
{
    return button;
}

void TransportButtonComponent::resized()
{
    auto area = getLocalBounds();

    label.setVisible(showLabel);
    if (showLabel)
    {
        constexpr int labelHeight = static_cast<int>(StyleSheet::fontDefaultSize);
        label.setBounds(area.removeFromTop(labelHeight));
    }

    area.removeFromTop(StyleSheet::controlGap);
    const auto controlSize = juce::jmax(0, juce::jmin(area.getWidth(), area.getHeight()));
    button.setBounds(area.withSizeKeepingCentre(controlSize, controlSize));
}

std::unique_ptr<juce::DrawablePath> TransportButtonComponent::createPlayIcon(const juce::Colour colour)
{
    auto drawable = std::make_unique<juce::DrawablePath>();
    juce::Path path;
    path.addTriangle(28.0f, 20.0f, 28.0f, 80.0f, 74.0f, 50.0f);
    drawable->setPath(path);
    drawable->setFill(juce::FillType(colour));
    return drawable;
}

std::unique_ptr<juce::DrawablePath> TransportButtonComponent::createStopIcon(const juce::Colour colour)
{
    auto drawable = std::make_unique<juce::DrawablePath>();
    juce::Path path;
    path.addRectangle(30.0f, 30.0f, 40.0f, 40.0f);
    drawable->setPath(path);
    drawable->setFill(juce::FillType(colour));
    return drawable;
}
