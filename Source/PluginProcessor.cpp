/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SmallEQAudioProcessor::SmallEQAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    ),
#endif
    parameters(*this, nullptr, "PARAMETERS", createParameterLayout())
{
}

SmallEQAudioProcessor::~SmallEQAudioProcessor()
{
}

//==============================================================================
const juce::String SmallEQAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SmallEQAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SmallEQAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SmallEQAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SmallEQAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SmallEQAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SmallEQAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SmallEQAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SmallEQAudioProcessor::getProgramName (int index)
{
    return {};
}

void SmallEQAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void SmallEQAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getMainBusNumOutputChannels();

    filter.prepare(spec);
    filter.reset();

    updateCoefficients(sampleRate);
}

void SmallEQAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SmallEQAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void SmallEQAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    juce::dsp::AudioBlock<float> block(buffer);

    updateCoefficients(getSampleRate());
    filter.process(juce::dsp::ProcessContextReplacing<float>(block));
}

void SmallEQAudioProcessor::updateCoefficients(double sampleRate) {
    float lpCutoff = parameters.getRawParameterValue("lpCutoff")->load();
    float hpCutoff = parameters.getRawParameterValue("hpCutoff")->load();

    if (lpCutoff == lastLpCutoff && hpCutoff == lastHpCutoff) return;
    
    lastLpCutoff = lpCutoff;
    lastHpCutoff = hpCutoff;

    int M = 21; // filter order

    double wcLP = 2.0 * juce::MathConstants<double>::pi * lpCutoff / sampleRate;
    double wcHP = 2.0 * juce::MathConstants<double>::pi * hpCutoff / sampleRate;

    std::vector<float> hLP(M, 0.f);
    std::vector<float> hHP(M, 0.f);

    // Generate the ideal impulse response using the sinc function
    for (int n = 0; n < M; ++n)
    {
        if (n == (M - 1) / 2) { // Handle the center element (avoid division by zero)
            hLP.at(n) = wcLP / juce::MathConstants<double>::pi;
        }
        else {
            hLP.at(n) = std::sin(wcLP * (n - (M - 1) / 2.f)) / (juce::MathConstants<double>::pi * (n - (M - 1) / 2.f));
        }
    }

    for (int n = 0; n < M; ++n)
    {
        if (n == (M - 1) / 2) { // Handle the center element (avoid division by zero)
            hHP.at(n) = 1.0 - (wcHP / juce::MathConstants<double>::pi);
        }
        else {
            hHP.at(n) = - (std::sin(wcHP * (n - (M - 1) / 2.f)) / (juce::MathConstants<double>::pi * (n - (M - 1) / 2.f)));
        }
    }

    // Apply a Hamming window to smooth the response
    for (int n = 0; n < M; ++n)
    {
        double window = 0.54 - 0.46 * std::cos(2 * juce::MathConstants<double>::pi * n / (M - 1));  // Hamming window
        hLP.at(n) *= window;
        hHP.at(n) *= window;
    }
    
    auto lpCoeffs = juce::dsp::FIR::Coefficients<float>::Coefficients(&hLP[0], hLP.size());
    auto hpCoeffs = juce::dsp::FIR::Coefficients<float>::Coefficients(&hHP[0], hHP.size());

    //*filter.get<0>().state = *juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, lpCutoff);
    //*filter.get<1>().state = *juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, hpCutoff);

    *filter.get<0>().state = lpCoeffs;
    *filter.get<1>().state = hpCoeffs;
}

juce::AudioProcessorValueTreeState::ParameterLayout SmallEQAudioProcessor::createParameterLayout() {
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> parameters;
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>("lpCutoff", "Low Pass Cutoff Frequency", juce::NormalisableRange<float>(10.f, 20000.f, 1.f, 0.5f, false), 20000.f, juce::AudioParameterFloatAttributes()));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>("hpCutoff", "High Pass Cutoff Frequency", juce::NormalisableRange<float>(10.f, 20000.f, 1.f, 0.5f, false), 10.f, juce::AudioParameterFloatAttributes()));

    return { parameters.begin(), parameters.end() };
}

//==============================================================================
bool SmallEQAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* SmallEQAudioProcessor::createEditor()
{
    return new SmallEQAudioProcessorEditor (*this);
}

//==============================================================================
void SmallEQAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.

    /*juce::MemoryOutputStream mos(destData, true);
    parameters.state.writeToStream(mos);*/
}

void SmallEQAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    
    /*auto tree = juce::ValueTree::readFromData(data, sizeInBytes);
    if (tree.isValid())
    {
        parameters.replaceState(tree);
        updateCoefficients();
    }*/
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SmallEQAudioProcessor();
}
