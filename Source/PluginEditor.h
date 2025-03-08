/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class SmallEQAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    SmallEQAudioProcessorEditor (SmallEQAudioProcessor&);
    ~SmallEQAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    SmallEQAudioProcessor& audioProcessor;

    // Sliders
    juce::Slider lpCutoffSlider;
    juce::Slider hpCutoffSlider;

    // Labels
    juce::Label lpLabel;
    juce::Label hpLabel;

    // Attachments to sync GUI with parameters
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lpCutoffAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> hpCutoffAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SmallEQAudioProcessorEditor)
};
