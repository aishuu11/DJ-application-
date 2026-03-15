#include "GlossyButtonLookAndFeel.h"

//==============================================================================
// Custom LookAndFeel Implementation - 3D glossy buttons
void GlossyButtonLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button,
                                                   const juce::Colour& backgroundColour,
                                                   bool shouldDrawButtonAsHighlighted,
                                                   bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat().reduced(2.0f);
    auto cornerSize = 6.0f;
    
    // Shadow effect (drawn below button)
    if (!shouldDrawButtonAsDown)
    {
        g.setColour(juce::Colours::black.withAlpha(0.5f));
        g.fillRoundedRectangle(bounds.translated(0, 3), cornerSize);
    }
    
    auto flatPink = juce::Colour(0xffff1493); // Standard pink for deck buttons
    auto specialPink = juce::Colour(0xffff69b4); // Special marker for main control buttons
    bool isMainControlButton = backgroundColour == specialPink; // PLAY, STOP, LOOP IN/OUT, PITCH +/-
    bool isDeckButton = backgroundColour == flatPink; // Other deck buttons (hot cues, etc)
    
    if (isMainControlButton)
    {
        // Main control buttons: REVERSED behavior
        // Normal state = flat pink, Hover = gradient pink
        if (shouldDrawButtonAsHighlighted || shouldDrawButtonAsDown)
        {
            // Hovering or pressed = gradient pink
            auto pinkColor = juce::Colour(0xffff1493);   // Hot pink
            auto purpleColor = juce::Colour(0xff9d00ff); // Vibrant purple
            
            juce::ColourGradient gradient(
                pinkColor, bounds.getX(), bounds.getCentreY(),
                purpleColor, bounds.getRight(), bounds.getCentreY(),
                false);
            g.setGradientFill(gradient);
            g.fillRoundedRectangle(bounds, cornerSize);
        }
        else
        {
            // Normal state = flat pink
            g.setColour(flatPink);
            g.fillRoundedRectangle(bounds, cornerSize);
        }
    }
    else if (isDeckButton)
    {
        // Other deck buttons: normal behavior (gradient -> flat on hover)
        if (shouldDrawButtonAsHighlighted || shouldDrawButtonAsDown)
        {
            // Flat pink on hover/press
            auto hoverColor = shouldDrawButtonAsDown ? flatPink.darker(0.3f) : flatPink.brighter(0.2f);
            g.setColour(hoverColor);
            g.fillRoundedRectangle(bounds, cornerSize);
        }
        else
        {
            // Gradient pink in normal state
            auto pinkColor = juce::Colour(0xffff1493);   // Hot pink
            auto purpleColor = juce::Colour(0xff9d00ff); // Vibrant purple
            
            juce::ColourGradient gradient(
                pinkColor, bounds.getX(), bounds.getCentreY(),
                purpleColor, bounds.getRight(), bounds.getCentreY(),
                false);
            g.setGradientFill(gradient);
            g.fillRoundedRectangle(bounds, cornerSize);
        }
    }
    else
    {
        // Playlist buttons: white normally, pink on hover
        if (shouldDrawButtonAsHighlighted || shouldDrawButtonAsDown)
        {
            auto hoverColor = shouldDrawButtonAsDown ? flatPink.darker(0.3f) : flatPink.brighter(0.2f);
            g.setColour(hoverColor);
            g.fillRoundedRectangle(bounds, cornerSize);
        }
        else
        {
            g.setColour(backgroundColour);
            g.fillRoundedRectangle(bounds, cornerSize);
        }
    }
    
    // Glossy highlight on top half
    if (!shouldDrawButtonAsDown)
    {
        auto highlightArea = bounds.removeFromTop(bounds.getHeight() * 0.5f);
        juce::ColourGradient highlight(
            juce::Colours::white.withAlpha(0.3f),
            highlightArea.getX(), highlightArea.getY(),
            juce::Colours::white.withAlpha(0.0f),
            highlightArea.getX(), highlightArea.getBottom(),
            false);
        g.setGradientFill(highlight);
        g.fillRoundedRectangle(highlightArea, cornerSize);
    }
    
    // Border - pink when hovering, subtle gray when normal
    auto borderColour = shouldDrawButtonAsHighlighted 
        ? juce::Colour(0xffff1493).brighter(0.5f)
        : juce::Colours::lightgrey.withAlpha(0.5f);
    g.setColour(borderColour);
    g.drawRoundedRectangle(bounds, cornerSize, shouldDrawButtonAsHighlighted ? 2.5f : 1.5f);
}

void GlossyButtonLookAndFeel::drawButtonText(juce::Graphics& g, juce::TextButton& button,
                                            bool shouldDrawButtonAsHighlighted,
                                            bool shouldDrawButtonAsDown)
{
    // Special handling for PLAY button - draw triangle icon
    if (button.getButtonText() == "PLAY")
    {
        auto bounds = button.getLocalBounds().toFloat();
        auto centre = bounds.getCentre();
        float size = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.35f;
        
        // Create triangle path (play icon)
        juce::Path triangle;
        triangle.addTriangle(
            centre.x - size * 0.4f, centre.y - size * 0.5f,  // Top left
            centre.x - size * 0.4f, centre.y + size * 0.5f,  // Bottom left
            centre.x + size * 0.6f, centre.y                 // Right point
        );
        
        // Set color
        g.setColour(button.findColour(button.getToggleState() ? juce::TextButton::textColourOnId
                                                                : juce::TextButton::textColourOffId)
                        .withMultipliedAlpha(button.isEnabled() ? 1.0f : 0.5f));
        
        // Fill triangle
        g.fillPath(triangle);
        return;
    }
    
    // Default text drawing for other buttons
    juce::Font font(15.0f, juce::Font::bold); // Bold and slightly bigger
    g.setFont(font);
    g.setColour(button.findColour(button.getToggleState() ? juce::TextButton::textColourOnId
                                                            : juce::TextButton::textColourOffId)
                    .withMultipliedAlpha(button.isEnabled() ? 1.0f : 0.5f));

    const int yIndent = juce::jmin(4, button.proportionOfHeight(0.3f));
    const int cornerSize = juce::jmin(button.getHeight(), button.getWidth()) / 2;

    const int fontHeight = juce::roundToInt(font.getHeight() * 0.6f);
    const int leftIndent = juce::jmin(fontHeight, 2 + cornerSize / (button.isConnectedOnLeft() ? 4 : 2));
    const int rightIndent = juce::jmin(fontHeight, 2 + cornerSize / (button.isConnectedOnRight() ? 4 : 2));
    const int textWidth = button.getWidth() - leftIndent - rightIndent;

    if (textWidth > 0)
        g.drawFittedText(button.getButtonText(),
                        leftIndent, yIndent, textWidth, button.getHeight() - yIndent * 2,
                        juce::Justification::centred, 2);
}

//==============================================================================
// Custom pill-shaped slider for volume, speed, and tone
void GlossyButtonLookAndFeel::drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                                               float sliderPos, float minSliderPos, float maxSliderPos,
                                               juce::Slider::SliderStyle style, juce::Slider& slider)
{
    // Only customize vertical linear sliders (volume, speed, tone)
    if (style != juce::Slider::LinearVertical)
    {
        LookAndFeel_V4::drawLinearSlider(g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider);
        return;
    }
    
    auto trackBounds = juce::Rectangle<float>(x, y, width, height).reduced(width * 0.2f, 0);
    auto cornerSize = trackBounds.getWidth() / 2.0f; // Perfect pill shape (half of width)
    
    // Dark background track (full pill)
    g.setColour(juce::Colour(0xff1a1f35));
    g.fillRoundedRectangle(trackBounds, cornerSize);
    
    // Filled track (from bottom to slider position)
    auto filledHeight = juce::jmap(sliderPos, static_cast<float>(height - y), 0.0f, 0.0f, static_cast<float>(height));
    auto filledTrack = trackBounds.withTop(sliderPos).withHeight(filledHeight);
    
    // Get the track color from the slider
    auto trackColour = slider.findColour(juce::Slider::trackColourId);
    
    // Create gradient for filled portion
    juce::ColourGradient fillGradient(
        trackColour.brighter(0.3f), filledTrack.getCentreX(), filledTrack.getY(),
        trackColour, filledTrack.getCentreX(), filledTrack.getBottom(), false);
    
    g.setGradientFill(fillGradient);
    g.fillRoundedRectangle(filledTrack, cornerSize);
    
    // Glossy highlight on filled track
    auto highlightArea = filledTrack.withWidth(filledTrack.getWidth() * 0.4f);
    juce::ColourGradient highlight(
        juce::Colours::white.withAlpha(0.3f), highlightArea.getX(), highlightArea.getY(),
        juce::Colours::white.withAlpha(0.0f), highlightArea.getRight(), highlightArea.getY(), false);
    g.setGradientFill(highlight);
    g.fillRoundedRectangle(highlightArea, cornerSize);
    
    // Border
    g.setColour(trackColour.brighter(0.5f));
    g.drawRoundedRectangle(trackBounds, cornerSize, 1.5f);
    
    // Custom thumb (circular with glow)
    auto thumbSize = trackBounds.getWidth() + 4.0f;
    auto thumbX = trackBounds.getCentreX() - thumbSize / 2.0f;
    auto thumbY = sliderPos - thumbSize / 2.0f;
    auto thumbBounds = juce::Rectangle<float>(thumbX, thumbY, thumbSize, thumbSize);
    
    // Thumb glow
    g.setColour(trackColour.withAlpha(0.3f));
    g.fillEllipse(thumbBounds.expanded(2.0f));
    
    // Thumb gradient
    juce::ColourGradient thumbGradient(
        trackColour.brighter(0.8f), thumbBounds.getCentreX(), thumbBounds.getY(),
        trackColour.darker(0.3f), thumbBounds.getCentreX(), thumbBounds.getBottom(), false);
    g.setGradientFill(thumbGradient);
    g.fillEllipse(thumbBounds);
    
    // Thumb border
    g.setColour(juce::Colours::white.withAlpha(0.6f));
    g.drawEllipse(thumbBounds, 1.5f);
    
    // Thumb highlight
    auto thumbHighlight = thumbBounds.withHeight(thumbBounds.getHeight() * 0.4f);
    g.setColour(juce::Colours::white.withAlpha(0.4f));
    g.fillEllipse(thumbHighlight);
}

//==============================================================================
// Custom rotary slider (knob) for EQ
void GlossyButtonLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                               float sliderPosProportional, float rotaryStartAngle,
                                               float rotaryEndAngle, juce::Slider& slider)
{
    auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(10.0f);
    auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
    auto centre = bounds.getCentre();
    auto knobRadius = radius * 0.85f;
    
    // Shadow
    g.setColour(juce::Colours::black.withAlpha(0.4f));
    g.fillEllipse(centre.x - knobRadius, centre.y - knobRadius + 2.0f, knobRadius * 2.0f, knobRadius * 2.0f);
    
    // Outer ring with gradient
    auto ringOuter = knobRadius;
    auto ringInner = knobRadius * 0.85f;
    
    g.setColour(juce::Colour(0xff2a2f45));
    g.fillEllipse(centre.x - ringOuter, centre.y - ringOuter, ringOuter * 2.0f, ringOuter * 2.0f);
    
    // Knob body with metallic gradient
    juce::ColourGradient knobGradient(
        juce::Colour(0xff4a4f65), centre.x, centre.y - ringInner,
        juce::Colour(0xff2a2f45), centre.x, centre.y + ringInner, false);
    g.setGradientFill(knobGradient);
    g.fillEllipse(centre.x - ringInner, centre.y - ringInner, ringInner * 2.0f, ringInner * 2.0f);
    
    // Glossy highlight
    auto highlightRadius = ringInner * 0.6f;
    juce::ColourGradient highlight(
        juce::Colours::white.withAlpha(0.4f), centre.x, centre.y - ringInner * 0.5f,
        juce::Colours::white.withAlpha(0.0f), centre.x, centre.y, false);
    g.setGradientFill(highlight);
    g.fillEllipse(centre.x - highlightRadius, centre.y - highlightRadius * 1.2f, 
                  highlightRadius * 2.0f, highlightRadius * 2.0f);
    
    // Value arc
    auto angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
    auto arcRadius = knobRadius * 1.1f;
    
    juce::Path valueArc;
    valueArc.addCentredArc(centre.x, centre.y, arcRadius, arcRadius,
                          0.0f, rotaryStartAngle, angle, true);
    
    // Get color based on slider
    auto trackColour = slider.findColour(juce::Slider::rotarySliderFillColourId);
    if (trackColour == juce::Colour(0x00000000)) // If not set, use thumb color
        trackColour = slider.findColour(juce::Slider::thumbColourId);
    
    g.setColour(trackColour);
    g.strokePath(valueArc, juce::PathStrokeType(3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    
    // Pointer line
    juce::Path pointer;
    auto pointerLength = ringInner * 0.6f;
    auto pointerThickness = 3.0f;
    pointer.addRectangle(-pointerThickness * 0.5f, -ringInner + 5.0f, pointerThickness, pointerLength);
    pointer.applyTransform(juce::AffineTransform::rotation(angle).translated(centre.x, centre.y));
    
    g.setColour(juce::Colours::white);
    g.fillPath(pointer);
    
    // Center dot
    auto dotRadius = knobRadius * 0.12f;
    g.setColour(juce::Colour(0xff1a1f35));
    g.fillEllipse(centre.x - dotRadius, centre.y - dotRadius, dotRadius * 2.0f, dotRadius * 2.0f);
}
