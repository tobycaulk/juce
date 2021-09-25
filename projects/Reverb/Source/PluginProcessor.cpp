/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ReverbAudioProcessor::ReverbAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    delayMs = 100;
    decayFactor = 0.1;
    mixPercent = 1;
}

ReverbAudioProcessor::~ReverbAudioProcessor()
{
}

//==============================================================================
const juce::String ReverbAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ReverbAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool ReverbAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool ReverbAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double ReverbAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ReverbAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int ReverbAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ReverbAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String ReverbAudioProcessor::getProgramName (int index)
{
    return {};
}

void ReverbAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void ReverbAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void ReverbAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ReverbAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
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

float* combFilter (float* samples, int sampleLength, float delayMs, float decayFactor, float sampleRate)
{
    int delaySamples = (int) ((float) delayMs * (sampleRate / 1000));
    
    auto* combFilterSamples = new float[sampleLength];
    memcpy(combFilterSamples, samples, sizeof(*samples));
    
    for (int i = 0; i < sampleLength - delaySamples; i++)
    {
        combFilterSamples[i + delaySamples] += ((float) combFilterSamples[i] * decayFactor);
    }
    
    return combFilterSamples;
}

float* allPassFilter (float samples[], int sampleLength, float sampleRate)
{
    int delaySamples = (int) ((float) 89.27f * (sampleRate / 1000));
    float* allPassFilterSamples = new float[sampleLength];
    float decayFactor = 0.131f;
    
    for (int i = 0; i < sampleLength; i++)
    {
        allPassFilterSamples[i] = samples[i];
        
        if (i - delaySamples >= 0)
        {
            allPassFilterSamples[i] += -decayFactor * allPassFilterSamples[i - delaySamples];
        }
        
        if (i - delaySamples >= 1)
        {
            allPassFilterSamples[i] += decayFactor * allPassFilterSamples[i + 20 - delaySamples];
        }
    }
    
    float value = allPassFilterSamples[0];
    float max = 0.0f;
    
    for (int i = 0; i < sampleLength; i++)
    {
        if (abs(allPassFilterSamples[i]) > max)
        {
            max = abs(allPassFilterSamples[i]);
        }
    }
    
    for (int i = 0; i < sizeof(allPassFilterSamples); i++)
    {
        float currentValue = allPassFilterSamples[i];
        value = ((value + (currentValue - value)) / max);
        
        allPassFilterSamples[i] = value;
    }
    
    return allPassFilterSamples;
}

void ReverbAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    auto totalNumSamples        = buffer.getNumSamples();
    auto sampleRate             = (float) getSampleRate();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
    {
        buffer.clear (i, 0, buffer.getNumSamples());
    }

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* samples = new float[totalNumSamples];
        for (int sample = 0; sample < totalNumSamples; ++sample)
        {
            samples[sample] = *buffer.getReadPointer(channel, sample);
            //auto* channelData = buffer.getWritePointer (channel, sample);
            //float* combFilter1 = combFilter(channel)
            
            
            // ..do something to the data...
        }
        
        auto* combFilter1 = combFilter(samples, totalNumSamples, delayMs, decayFactor, sampleRate);
        auto* combFilter2 = combFilter(samples, totalNumSamples, (delayMs - 11.73f), (decayFactor - 0.1313f), sampleRate);
        auto* combFilter3 = combFilter(samples, totalNumSamples, (delayMs - 19.13f), (decayFactor - 0.2743f), sampleRate);
        auto* combFilter4 = combFilter(samples, totalNumSamples, (delayMs - 7.97f), (decayFactor - 0.31f), sampleRate);
        
        auto* combOutput = new float[totalNumSamples];
        for (int i = 0; i < totalNumSamples; i++)
        {
            combOutput[i] = combFilter1[i] + combFilter2[i] + combFilter3[i] + combFilter4[i];
        }
        
        auto* mixed = new float[totalNumSamples];
        for (int i = 0; i < totalNumSamples; i++)
        {
            mixed[i] = ((100 - mixPercent) * samples[i]) + (mixPercent * combOutput[i]);
        }
        
        auto* allPassFilter1 = allPassFilter(samples, totalNumSamples, sampleRate);
        auto* allPassFilter2 = allPassFilter(allPassFilter1, totalNumSamples, sampleRate);
        
        for(int sample = 0; sample < totalNumSamples; ++sample)
        {
            *buffer.getWritePointer(channel, sample) = mixed[sample];
        }
    }
}

//==============================================================================
bool ReverbAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* ReverbAudioProcessor::createEditor()
{
    return new ReverbAudioProcessorEditor (*this);
}

//==============================================================================
void ReverbAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void ReverbAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ReverbAudioProcessor();
}
