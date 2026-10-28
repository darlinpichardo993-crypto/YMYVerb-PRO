#pragma once
#include "PluginProcessor.h"

namespace ymy
{
    inline juce::Font F (float h, bool bold = false) { return juce::Font (juce::FontOptions (h, bold ? juce::Font::bold : juce::Font::plain)); }
    inline juce::Font Serif (float h, bool bold = true) { return juce::Font (juce::FontOptions ("Georgia", h, bold ? juce::Font::bold : juce::Font::plain)); }
    const juce::Colour cream1 (0xfff6f1ea), cream2 (0xffe2dacb), panel (0xffefe8dc), gold1 (0xffe6c777), gold2 (0xff9c7a2b), wine (0xff8a1426), ink (0xff3a2f28);

    struct LnF : juce::LookAndFeel_V4
    {
        void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h, float pos, float a0, float a1, juce::Slider&) override;
        void drawToggleButton (juce::Graphics&, juce::ToggleButton&, bool, bool) override;
        void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour&, bool, bool) override;
        void drawComboBox (juce::Graphics&, int w, int h, bool, int, int, int, int, juce::ComboBox&) override;
        juce::Font getComboBoxFont (juce::ComboBox&) override { return F (13.f); }
    };

    struct Knob : juce::Component
    {
        Knob (juce::AudioProcessorValueTreeState&, const juce::String& id, const juce::String& label);
        void resized() override;
        juce::Slider s; juce::Label name, value;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> att;
    };

    struct Meter : juce::Component { float level = 0.f; void paint (juce::Graphics&) override; };

    struct MicComponent : juce::Component
    {
        float glow = 0.f; bool ledOn = true;
        void paint (juce::Graphics&) override;
    };

    struct SimpleList : juce::ListBoxModel
    {
        std::function<int()> count; std::function<juce::String (int)> text; std::function<void (int)> select; std::function<bool (int)> starred;
        int getNumRows() override { return count(); }
        void paintListBoxItem (int row, juce::Graphics&, int w, int h, bool sel) override;
        void selectedRowsChanged (int row) override { if (row >= 0 && select) select (row); }
    };
}

class YmyEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit YmyEditor (YmyProcessor&);
    ~YmyEditor() override;
    void paint (juce::Graphics&) override;
    void resized() override;
private:
    void timerCallback() override;
    void refreshPresetUI (bool rebuildLists);
    void promptSave (bool saveAs);
    juce::Rectangle<int> R (float x, float y, float w, float h) const;
    void layoutRow (std::vector<ymy::Knob*> ks, juce::Rectangle<int> area);

    YmyProcessor& proc;
    ymy::LnF lnf;
    std::vector<std::unique_ptr<ymy::Knob>> knobs;
    ymy::Knob *kRoom, *kDecay, *kPre, *kEarly, *kDiff, *kDens, *kMod, *kWarm, *kDamp, *kLow, *kHigh, *kLowD, *kHighD, *kTone,
              *kWidth, *kBal, *kErW, *kSpread, *kDry, *kWet, *kIn, *kOut, *kRet, *kThr, *kDepth, *kAtt, *kRel;
    ymy::Knob* mk (const juce::String& id, const juce::String& label);

    juce::ComboBox algo, division, modeBox, presetBox;
    juce::ToggleButton sync { "Sync" }, triplet { "Triplet" }, dotted { "Dotted" }, duck { "Ducking" }, makeup { "Auto Makeup" }, led { "LED" };
    juce::Slider manualBpm;
    juce::Label hostBpmLabel, preLabel, descLabel, titleLabel, warnLabel, bpmCap, hostCap;
    juce::TextButton prev { "<" }, next { ">" }, saveB { "Save" }, saveAsB { "Save As" }, abB { "A / B" }, undoB { "Undo" }, redoB { "Redo" }, favB { "Fav" };
    ymy::SimpleList catModel, listModel; juce::ListBox catList, presetList;
    std::vector<int> shownIdx; juce::StringArray cats; int catSel = 0;
    ymy::Meter meterIn, meterOut; ymy::MicComponent mic;
    float glowState = 0.f; bool updatingUI = false;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>> comboAtt;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>> btnAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> bpmAtt;
    juce::ComponentBoundsConstrainer constrainer;
};
