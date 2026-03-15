#pragma once

#include <JuceHeader.h>

/**
 * Custom LookAndFeel for 3D glossy button effects and pill-shaped sliders
 */
class GlossyButtonLookAndFeel : public juce::LookAndFeel_V4
{
public:
    /**
     * drawButtonBackground - Renders 3D glossy button background with gradient effects
     * @param g Graphics context for drawing
     * @param button Button component to draw
     * @param backgroundColour Base color of the button
     * @param shouldDrawButtonAsHighlighted True if button is being hovered
     * @param shouldDrawButtonAsDown True if button is being pressed
     */
    void drawButtonBackground(juce::Graphics& g, juce::Button& button, 
                            const juce::Colour& backgroundColour,
                            bool shouldDrawButtonAsHighlighted, 
                            bool shouldDrawButtonAsDown) override;
    
    /**
     * drawButtonText - Renders button text or icon (e.g., play triangle)
     * @param g Graphics context for drawing
     * @param button Text button component to draw
     * @param shouldDrawButtonAsHighlighted True if button is being hovered
     * @param shouldDrawButtonAsDown True if button is being pressed
     */
    void drawButtonText(juce::Graphics& g, juce::TextButton& button,
                       bool shouldDrawButtonAsHighlighted,
                       bool shouldDrawButtonAsDown) override;
    
    /**
     * drawLinearSlider - Renders vertical pill-shaped slider with rectangular thumb
     * @param g Graphics context for drawing
     * @param x X position of slider
     * @param y Y position of slider
     * @param width Width of slider
     * @param height Height of slider
     * @param sliderPos Current position of slider thumb
     * @param minSliderPos Minimum slider position
     * @param maxSliderPos Maximum slider position
     * @param style Slider style (linear vertical/horizontal/rotary)
     * @param slider Slider component reference
     */
    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                         float sliderPos, float minSliderPos, float maxSliderPos,
                         juce::Slider::SliderStyle style, juce::Slider& slider) override;
    
    /**
     * drawRotarySlider - Renders custom knob for EQ controls
     * @param g Graphics context for drawing
     * @param x X position of knob
     * @param y Y position of knob
     * @param width Width of knob area
     * @param height Height of knob area
     * @param sliderPosProportional Position as proportion (0.0 to 1.0)
     * @param rotaryStartAngle Start angle in radians
     * @param rotaryEndAngle End angle in radians
     * @param slider Slider component reference
     */
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                         float sliderPosProportional, float rotaryStartAngle,
                         float rotaryEndAngle, juce::Slider& slider) override;
};
