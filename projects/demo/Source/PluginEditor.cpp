/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
DemoAudioProcessorEditor::DemoAudioProcessorEditor (DemoAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (200, 200);
    
    midiVolume.setSliderStyle(juce::Slider::LinearBarVertical);
    midiVolume.setRange(0.0, 1, 0.01);
    midiVolume.setTextBoxStyle(juce::Slider::NoTextBox, false, 90, 0);
    midiVolume.setPopupDisplayEnabled(true, false, this);
    midiVolume.setTextValueSuffix(" Volume ");
    midiVolume.setValue(0.5);
    midiVolume.addListener(this);
    
    addAndMakeVisible(&midiVolume);
}

DemoAudioProcessorEditor::~DemoAudioProcessorEditor()
{
}

void DemoAudioProcessorEditor::sliderValueChanged(juce::Slider* slider)
{
    audioProcessor.noteOnVel = midiVolume.getValue();
}

//==============================================================================
void DemoAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll(juce::Colours::white);
    g.setColour(juce::Colours::black);
    g.setFont(15.0f);
    g.drawFittedText("Midi Volume", 0, 0, getWidth(), 30, juce::Justification::centred, 1);
    
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void DemoAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    midiVolume.setBounds(40, 30, 20, getHeight() - 60);
}
