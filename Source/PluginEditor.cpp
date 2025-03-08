/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SmallEQAudioProcessorEditor::SmallEQAudioProcessorEditor (SmallEQAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Low-Pass Filter Slider
    lpCutoffSlider.setSliderStyle(juce::Slider::Rotary);
    lpCutoffSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    addAndMakeVisible(lpCutoffSlider);

    lpLabel.setText("Low-Pass Cutoff", juce::dontSendNotification);
    lpLabel.attachToComponent(&lpCutoffSlider, false);
    addAndMakeVisible(lpLabel);

    // High-Pass Filter Slider
    hpCutoffSlider.setSliderStyle(juce::Slider::Rotary);
    hpCutoffSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    addAndMakeVisible(hpCutoffSlider);

    hpLabel.setText("High-Pass Cutoff", juce::dontSendNotification);
    hpLabel.attachToComponent(&hpCutoffSlider, false);
    addAndMakeVisible(hpLabel);

    // Attach sliders to parameters
    lpCutoffAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "lpCutoff", lpCutoffSlider);

    hpCutoffAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "hpCutoff", hpCutoffSlider);

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);
}

SmallEQAudioProcessorEditor::~SmallEQAudioProcessorEditor()
{
}

//==============================================================================
void SmallEQAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (15.0f));
    g.drawFittedText ("Small EQ", getLocalBounds(), juce::Justification::centredTop, 1);
}

void SmallEQAudioProcessorEditor::resized()
{
    // Layout positions
    int sliderWidth = 150;
    int sliderHeight = 150;

    lpCutoffSlider.setBounds(50, 50, sliderWidth, sliderHeight);
    hpCutoffSlider.setBounds(200, 50, sliderWidth, sliderHeight);
}
