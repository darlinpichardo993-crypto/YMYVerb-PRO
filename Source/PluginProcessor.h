#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "DSP/ReverbCore.h"
#include "DSP/TempoSync.h"

struct Preset { juce::String name, category, description; juce::var params; bool user = false; };

class YmyProcessor : public juce::AudioProcessor, private juce::ValueTree::Listener
{
public:
    YmyProcessor();
    ~YmyProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout&) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "YMYVerb"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 20.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}
    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();

    juce::UndoManager undo;
    juce::AudioProcessorValueTreeState apvts;

    // ---- shared with the GUI (lock-free)
    std::atomic<float> inLevel { 0.f }, outLevel { 0.f }, hostBpm { 0.f }, effectivePredelay { 0.f };

    // ---- presets
    const std::vector<Preset>& getPresets() const { return presets; }
    int  getCurrentPreset() const { return currentPreset; }
    bool isModified() const { return modified.load(); }
    void loadPreset (int index);
    bool saveUserPreset (const juce::String& name);
    void refreshPresets();
    bool isFavorite (int i) const;
    void toggleFavorite (int i);
    void toggleAB();
    int  getABSlot() const { return abSlot; }
    juce::File getUserPresetDir() const;

private:
    void valueTreePropertyChanged (juce::ValueTree&, const juce::Identifier&) override;
    void loadFactory();
    void loadUserPresets();
    juce::File favoritesFile() const;

    orymy::ReverbEngine engine;
    std::vector<Preset> presets; int currentPreset = 0; std::atomic<bool> modified { false }; bool loadingPreset = false;
    juce::StringArray favorites;
    juce::ValueTree abState[2]; int abSlot = 0;
    std::vector<float> monoScratch;
    double sampleRateHz = 48000.0;

    std::atomic<float> *p_algo, *p_room, *p_decay, *p_predelay, *p_early, *p_diffusion, *p_density, *p_mod, *p_warmth, *p_damping,
        *p_lowcut, *p_highcut, *p_lowdecay, *p_highdecay, *p_tone, *p_width, *p_balance, *p_erwidth, *p_spread, *p_dry, *p_wet,
        *p_in, *p_out, *p_ret, *p_duckon, *p_duckthr, *p_duckdepth, *p_duckatt, *p_duckrel, *p_makeup, *p_sync, *p_bpm, *p_div,
        *p_triplet, *p_dotted;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (YmyProcessor)
};
