/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/
class SmallEQAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    SmallEQAudioProcessor();
    ~SmallEQAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    void updateCoefficients(double sampleRate);
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    //==============================================================================
    juce::AudioProcessorValueTreeState parameters;

private:
    // Filters
    using StereoEffect = juce::dsp::ProcessorDuplicator<juce::dsp::FIR::Filter<float>, juce::dsp::FIR::Coefficients<float>>;
    juce::dsp::ProcessorChain<StereoEffect, StereoEffect> filter;

    // Stored values for knowing when to update coefficients
    float lastLpCutoff = -1.0f;  // Store last used LPF cutoff
    float lastHpCutoff = -1.0f;  // Store last used HPF cutoff

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SmallEQAudioProcessor);
};
