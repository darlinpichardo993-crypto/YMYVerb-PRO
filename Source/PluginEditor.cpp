#include "PluginEditor.h"
using namespace juce;
using namespace ymy;

// ------------------------------------------------------------------ look and feel
void LnF::drawRotarySlider (Graphics& g, int x, int y, int w, int h, float pos, float a0, float a1, Slider&)
{
    auto b = Rectangle<float> ((float) x, (float) y, (float) w, (float) h).reduced (2.f);
    const float d = jmin (b.getWidth(), b.getHeight()), r = d * 0.5f; const auto c = b.getCentre();
    g.setColour (Colours::black.withAlpha (0.20f)); g.fillEllipse (Rectangle<float> (d, d).withCentre (c + Point<float> (0.f, 2.f)));
    g.setGradientFill (ColourGradient (gold1, c.x - r, c.y - r, gold2, c.x + r, c.y + r, false)); g.fillEllipse (Rectangle<float> (d, d).withCentre (c));
    const float rc = r * 0.82f;
    g.setGradientFill (ColourGradient (Colours::white, c.x - rc * 0.5f, c.y - rc * 0.7f, cream2, c.x + rc * 0.6f, c.y + rc * 0.8f, true));
    g.fillEllipse (Rectangle<float> (rc * 2, rc * 2).withCentre (c));
    g.setColour (gold2.withAlpha (0.5f)); g.drawEllipse (Rectangle<float> (rc * 2, rc * 2).withCentre (c), 1.f);
    Path p; p.addRoundedRectangle (-1.8f, -rc * 0.88f, 3.6f, rc * 0.42f, 1.6f);
    g.setColour (wine); g.fillPath (p, AffineTransform::rotation (a0 + pos * (a1 - a0)).translated (c));
}

void LnF::drawToggleButton (Graphics& g, ToggleButton& b, bool, bool)
{
    auto r = b.getLocalBounds().toFloat();
    const bool on = b.getToggleState();
    auto pill = Rectangle<float> (jmin (34.f, r.getWidth()), 16.f).withCentre ({ r.getCentreX(), r.getY() + 12.f });
    if (b.getButtonText().isEmpty()) pill.setCentre (r.getCentre());
    g.setColour (on ? Colour (0xff2f8f83) : Colour (0xffb8b0a4)); g.fillRoundedRectangle (pill, 8.f);
    g.setColour (Colours::white); g.fillEllipse (Rectangle<float> (12.f, 12.f).withCentre ({ on ? pill.getRight() - 9.f : pill.getX() + 9.f, pill.getCentreY() }));
    g.setColour (ink); g.setFont (F (11.f));
    g.drawText (b.getButtonText(), r.withTop (pill.getBottom() + 1.f).toNearestInt(), Justification::centred);
}

void LnF::drawButtonBackground (Graphics& g, Button& b, const Colour&, bool over, bool down)
{
    auto r = b.getLocalBounds().toFloat().reduced (0.5f);
    const bool on = b.getToggleState();
    g.setColour (on ? wine : (down ? Colour (0xffd7cdbb) : (over ? Colour (0xfff8f3ea) : Colour (0xffeee5d5))));
    g.fillRoundedRectangle (r, 5.f); g.setColour (gold2.withAlpha (0.6f)); g.drawRoundedRectangle (r, 5.f, 1.f);
}

void LnF::drawComboBox (Graphics& g, int w, int h, bool, int, int, int, int, ComboBox& cb)
{
    auto r = Rectangle<float> (0.f, 0.f, (float) w, (float) h).reduced (0.5f);
    g.setColour (Colour (0xfffbf8f2)); g.fillRoundedRectangle (r, 5.f);
    g.setColour (gold2.withAlpha (0.7f)); g.drawRoundedRectangle (r, 5.f, 1.f);
    Path t; t.addTriangle ((float) w - 18.f, h * 0.4f, (float) w - 8.f, h * 0.4f, (float) w - 13.f, h * 0.62f);
    g.setColour (ink); g.fillPath (t); (void) cb;
}

void SimpleList::paintListBoxItem (int row, Graphics& g, int w, int h, bool sel)
{
    if (sel) { g.setColour (Colour (0xff2c5da8)); g.fillRect (0, 0, w, h); }
    g.setColour (sel ? Colours::white : Colour (0xffd9deea)); g.setFont (F (12.f));
    g.drawText (text (row), 8, 0, w - 24, h, Justification::centredLeft);
    if (starred && starred (row)) { g.setColour (Colour (0xfff2c14e)); g.drawText (juce::String::fromUTF8 ("\xe2\x98\x85"), w - 20, 0, 16, h, Justification::centred); }
}

// ------------------------------------------------------------------ knob / meter / mic
Knob::Knob (AudioProcessorValueTreeState& a, const String& id, const String& label)
{
    s.setSliderStyle (Slider::RotaryHorizontalVerticalDrag); s.setTextBoxStyle (Slider::NoTextBox, true, 0, 0);
    s.setRotaryParameters (MathConstants<float>::pi * 1.2f, MathConstants<float>::pi * 2.8f, true);
    addAndMakeVisible (s); addAndMakeVisible (name); addAndMakeVisible (value);
    for (auto* l : { &name, &value }) { l->setJustificationType (Justification::centred); l->setColour (Label::textColourId, ink); l->setInterceptsMouseClicks (false, false); }
    name.setText (label, dontSendNotification); name.setFont (F (12.f));
    value.setFont (F (12.f)); value.setColour (Label::textColourId, ink.withAlpha (0.8f));
    att = std::make_unique<AudioProcessorValueTreeState::SliderAttachment> (a, id, s);
    s.onValueChange = [this] { value.setText (s.getTextFromValue (s.getValue()), dontSendNotification); };
    s.onValueChange();
}
void Knob::resized()
{
    auto b = getLocalBounds(); const int lh = jlimit (11, 18, getHeight() / 6);
    value.setBounds (b.removeFromBottom (lh)); name.setBounds (b.removeFromBottom (lh)); s.setBounds (b);
    name.setFont (F ((float) lh * 0.78f)); value.setFont (F ((float) lh * 0.78f));
}

void Meter::paint (Graphics& g)
{
    auto b = getLocalBounds().toFloat();
    g.setColour (Colour (0xff1b1614)); g.fillRoundedRectangle (b, 3.f);
    const int segs = 28; const float sh = (b.getHeight() - 4.f) / segs;
    const float db = Decibels::gainToDecibels (level, -60.f);
    for (int i = 0; i < segs; ++i)
    {
        const float segDb = -42.f + 42.f * (float) i / (segs - 1);
        const bool lit = db >= segDb;
        Colour c = segDb > -3.f ? Colour (0xffe0442c) : segDb > -12.f ? Colour (0xfff2a33a) : Colour (0xffe9c46a);
        g.setColour (lit ? c : c.withAlpha (0.14f));
        g.fillRect (b.getX() + 3.f, b.getBottom() - 2.f - (float) (i + 1) * sh + 1.f, b.getWidth() - 6.f, sh - 1.5f);
    }
}

void MicComponent::paint (Graphics& g)
{
    auto b = getLocalBounds().toFloat(); const float W = b.getWidth(), H = b.getHeight(); const float cx = b.getCentreX();
    const float act = ledOn ? glow : 0.f;
    // pearl capsule
    auto cap = Rectangle<float> (W * 0.12f, 0.f, W * 0.76f, H);
    g.setGradientFill (ColourGradient (Colours::white, cap.getX(), 0.f, Colour (0xffe9e1d4), cap.getRight(), 0.f, false));
    g.fillRoundedRectangle (cap, W * 0.38f);
    if (act > 0.01f)   // warm halo driven by the real input level
    {
        g.setGradientFill (ColourGradient (Colour (0xffff9a3c).withAlpha (0.55f * act), cx, H * 0.14f, Colour (0x00ff9a3c), cx - W * 0.7f, H * 0.14f, true));
        g.fillEllipse (cx - W * 0.7f, H * 0.14f - W * 0.7f, W * 1.4f, W * 1.4f);
        g.setGradientFill (ColourGradient (Colour (0xffff9a3c).withAlpha (0.35f * act), cx, H * 0.7f, Colour (0x00ff9a3c), cx - W * 0.6f, H * 0.7f, true));
        g.fillEllipse (cx - W * 0.6f, H * 0.7f - W * 0.6f, W * 1.2f, W * 1.2f);
    }
    // grille head
    auto head = Rectangle<float> (W * 0.2f, H * 0.03f, W * 0.6f, H * 0.2f);
    const Colour hc1 = Colour (0xffd23a3a).interpolatedWith (Colour (0xffff7a52), act), hc2 = Colour (0xff6a0c18).interpolatedWith (Colour (0xffc22a2a), act);
    g.setGradientFill (ColourGradient (hc1, head.getX(), head.getY(), hc2, head.getRight(), head.getBottom(), false));
    g.fillRoundedRectangle (head, W * 0.28f);
    for (int j = 0; j < 9; ++j) for (int i = 0; i < 7; ++i)
    {
        const float px = head.getX() + head.getWidth() * (0.12f + 0.76f * (float) i / 6.f + ((j & 1) ? 0.05f : 0.f)), py = head.getY() + head.getHeight() * (0.08f + 0.84f * (float) j / 8.f);
        g.setColour (Colours::white.withAlpha (0.18f + 0.5f * act * (float) ((i * 7 + j * 3) % 5) / 4.f)); g.fillEllipse (px - 1.4f, py - 1.4f, 2.8f, 2.8f);
    }
    // gold ring with crystals
    auto ring = Rectangle<float> (W * 0.16f, H * 0.225f, W * 0.68f, H * 0.045f);
    g.setGradientFill (ColourGradient (gold1, ring.getX(), 0.f, gold2, ring.getRight(), 0.f, false)); g.fillRoundedRectangle (ring, 4.f);
    for (int i = 0; i < 14; ++i) { g.setColour (Colours::white.withAlpha (0.55f + 0.4f * act)); g.fillEllipse (ring.getX() + 5.f + (ring.getWidth() - 12.f) * (float) i / 13.f, ring.getCentreY() - 2.f, 4.f, 4.f); }
    // body
    auto body = Rectangle<float> (W * 0.2f, H * 0.27f, W * 0.6f, H * 0.5f);
    g.setGradientFill (ColourGradient (Colour (0xffb11d33), body.getX(), 0.f, Colour (0xff560a16), body.getRight(), 0.f, false));
    g.fillRoundedRectangle (body, 6.f);
    Path vine; for (int i = 0; i <= 40; ++i) { const float t = (float) i / 40.f; const float x = body.getCentreX() + std::sin (t * 9.f) * body.getWidth() * 0.22f, y = body.getY() + 8.f + t * (body.getHeight() - 16.f); if (i == 0) vine.startNewSubPath (x, y); else vine.lineTo (x, y); }
    g.setColour (gold1.withAlpha (0.75f)); g.strokePath (vine, PathStrokeType (1.4f));
    for (int i = 4; i <= 36; i += 4) { const float t = (float) i / 40.f; g.fillEllipse (body.getCentreX() + std::sin (t * 9.f) * body.getWidth() * 0.22f - 3.f, body.getY() + 8.f + t * (body.getHeight() - 16.f) - 3.f, 6.f, 6.f); }
    auto plate = Rectangle<float> (W * 0.34f, H * 0.34f, W * 0.32f, H * 0.075f);
    g.setColour (Colour (0xff1a1210)); g.fillRoundedRectangle (plate, 3.f); g.setColour (gold1); g.drawRoundedRectangle (plate, 3.f, 1.2f);
    g.setFont (Serif (plate.getHeight() * 0.62f)); g.drawText ("YMY", plate.toNearestInt(), Justification::centred);
    // handle
    auto hnd = Rectangle<float> (W * 0.27f, H * 0.78f, W * 0.46f, H * 0.17f);
    g.setGradientFill (ColourGradient (Colour (0xff6e5322), hnd.getX(), 0.f, Colour (0xffe4c46c), hnd.getCentreX(), 0.f, true)); g.fillRoundedRectangle (hnd, 6.f);
    for (int j = 0; j < 7; ++j) for (int i = 0; i < 5; ++i) { g.setColour (Colours::white.withAlpha (0.25f + 0.45f * act)); g.fillEllipse (hnd.getX() + hnd.getWidth() * (0.14f + 0.72f * (float) i / 4.f) - 1.5f, hnd.getY() + hnd.getHeight() * (0.1f + 0.8f * (float) j / 6.f) - 1.5f, 3.f, 3.f); }
}

// ------------------------------------------------------------------ editor
Knob* YmyEditor::mk (const String& id, const String& label)
{
    knobs.push_back (std::make_unique<Knob> (proc.apvts, id, label)); auto* k = knobs.back().get(); addAndMakeVisible (k); return k;
}

YmyEditor::YmyEditor (YmyProcessor& p) : AudioProcessorEditor (&p), proc (p), catList ("cats", &catModel), presetList ("presets", &listModel)
{
    setLookAndFeel (&lnf);
    kRoom = mk ("room", "Room Size"); kDecay = mk ("decay", "Decay"); kPre = mk ("predelay", "Predelay"); kEarly = mk ("early", "Early Reflections");
    kDiff = mk ("diffusion", "Diffusion"); kDens = mk ("density", "Density"); kMod = mk ("mod", "Modulation");
    kWarm = mk ("warmth", "Warmth"); kDamp = mk ("damping", "Damping"); kLow = mk ("lowcut", "Low Cut"); kHigh = mk ("highcut", "High Cut");
    kLowD = mk ("lowdecay", "Low Decay"); kHighD = mk ("highdecay", "High Decay"); kTone = mk ("tone", "Reverb Tone");
    kWidth = mk ("width", "Width"); kBal = mk ("balance", "Balance"); kErW = mk ("erwidth", "Early Ref. Width"); kSpread = mk ("spread", "Reverb Spread");
    kDry = mk ("dry", "Dry"); kWet = mk ("wet", "Wet"); kIn = mk ("in", "Input"); kOut = mk ("out", "Output"); kRet = mk ("ret", "Reverb Return");
    kThr = mk ("duckthr", "Threshold"); kDepth = mk ("duckdepth", "Depth"); kAtt = mk ("duckatt", "Attack"); kRel = mk ("duckrel", "Release");

    algo.addItemList ({ "Plate", "Room", "Chamber", "Hall", "Vintage Digital", "Hybrid Bachata" }, 1);
    division.addItemList ({ "1/64", "1/32", "1/16", "1/8", "1/4", "1/2", "1/1" }, 1);
    modeBox.addItemList ({ "Insert", "Aux Send" }, 1);
    for (auto* c : { &algo, &division, &modeBox, &presetBox }) addAndMakeVisible (c);
    comboAtt.push_back (std::make_unique<AudioProcessorValueTreeState::ComboBoxAttachment> (proc.apvts, "algo", algo));
    comboAtt.push_back (std::make_unique<AudioProcessorValueTreeState::ComboBoxAttachment> (proc.apvts, "div", division));
    modeBox.setSelectedId ((int) proc.apvts.getRawParameterValue ("mode")->load() + 1, dontSendNotification);
    modeBox.onChange = [this]   // only fires on user choice (the timer syncs it from the parameter without notifying)
    {
        auto set = [this] (const char* id, float v) { if (auto* pr = proc.apvts.getParameter (id)) pr->setValueNotifyingHost (pr->convertTo0to1 (v)); };
        set ("mode", (float) (modeBox.getSelectedId() - 1));
        if (modeBox.getSelectedId() == 2) { set ("dry", 0.f); set ("wet", 1.f); } else { set ("dry", 1.f); set ("wet", 0.28f); }
    };

    struct B { ToggleButton* b; const char* id; };
    for (auto& t : { B { &sync, "sync" }, B { &triplet, "triplet" }, B { &dotted, "dotted" }, B { &duck, "duckon" }, B { &makeup, "makeup" }, B { &led, "led" } })
    { addAndMakeVisible (t.b); btnAtt.push_back (std::make_unique<AudioProcessorValueTreeState::ButtonAttachment> (proc.apvts, t.id, *t.b)); }
    triplet.onClick = [this] { if (triplet.getToggleState()) dotted.setToggleState (false, sendNotificationSync); };
    dotted.onClick  = [this] { if (dotted.getToggleState()) triplet.setToggleState (false, sendNotificationSync); };

    manualBpm.setSliderStyle (Slider::LinearBar); manualBpm.setColour (Slider::trackColourId, gold1.withAlpha (0.35f)); manualBpm.setColour (Slider::textBoxTextColourId, ink);
    addAndMakeVisible (manualBpm); bpmAtt = std::make_unique<AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, "bpm", manualBpm);
    for (auto* l : { &hostBpmLabel, &preLabel, &descLabel, &titleLabel, &warnLabel, &bpmCap, &hostCap }) { addAndMakeVisible (l); l->setColour (Label::textColourId, ink); }
    hostBpmLabel.setJustificationType (Justification::centred); preLabel.setJustificationType (Justification::centred);
    bpmCap.setText ("Manual BPM", dontSendNotification); hostCap.setText ("Host BPM", dontSendNotification); bpmCap.setFont (F (11.f)); hostCap.setFont (F (11.f));
    titleLabel.setText ("YMYVerb", dontSendNotification); titleLabel.setColour (Label::textColourId, gold2);
    warnLabel.setColour (Label::textColourId, Colour (0xffb3261e)); warnLabel.setJustificationType (Justification::centred);
    descLabel.setColour (Label::textColourId, Colour (0xffe8ecf5)); descLabel.setColour (Label::backgroundColourId, Colour (0xff0e1730)); descLabel.setBorderSize (BorderSize<int> (8)); descLabel.setJustificationType (Justification::topLeft);

    for (auto* b : { &prev, &next, &saveB, &saveAsB, &abB, &undoB, &redoB, &favB }) addAndMakeVisible (b);
    addAndMakeVisible (catList); addAndMakeVisible (presetList);
    for (auto* l : { &catList, &presetList }) { l->setColour (ListBox::backgroundColourId, Colour (0xff0e1730)); l->setRowHeight (20); }

    catModel.count = [this] { return cats.size(); };
    catModel.text = [this] (int i) { return cats[i]; };
    catModel.select = [this] (int i) { if (updatingUI) return; catSel = i; refreshPresetUI (true); };
    listModel.count = [this] { return (int) shownIdx.size(); };
    listModel.text = [this] (int i) { return proc.getPresets()[(size_t) shownIdx[(size_t) i]].name; };
    listModel.starred = [this] (int i) { return proc.isFavorite (shownIdx[(size_t) i]); };
    listModel.select = [this] (int i) { if (updatingUI) return; proc.loadPreset (shownIdx[(size_t) i]); refreshPresetUI (false); };

    presetBox.onChange = [this] { const int id = presetBox.getSelectedId(); if (id > 0 && id - 1 != proc.getCurrentPreset()) { proc.loadPreset (id - 1); refreshPresetUI (false); } };
    auto step = [this] (int d) { const int n = (int) proc.getPresets().size(); proc.loadPreset ((proc.getCurrentPreset() + d + n) % n); refreshPresetUI (false); };
    prev.onClick = [step] { step (-1); }; next.onClick = [step] { step (1); };
    favB.onClick = [this] { proc.toggleFavorite (proc.getCurrentPreset()); refreshPresetUI (true); };
    saveB.onClick = [this] { promptSave (false); }; saveAsB.onClick = [this] { promptSave (true); };
    abB.onClick = [this] { proc.toggleAB(); abB.setButtonText (proc.getABSlot() == 0 ? "A / B  (A)" : "A / B  (B)"); };
    undoB.onClick = [this] { proc.undo.undo(); }; redoB.onClick = [this] { proc.undo.redo(); };

    addAndMakeVisible (meterIn); addAndMakeVisible (meterOut); addAndMakeVisible (mic);
    refreshPresetUI (true);
    constrainer.setFixedAspectRatio (1.5); constrainer.setSizeLimits (900, 600, 1800, 1200);
    setConstrainer (&constrainer); setResizable (true, true); setSize (1200, 800);
    startTimerHz (30);
}
YmyEditor::~YmyEditor() { stopTimer(); setLookAndFeel (nullptr); }

void YmyEditor::refreshPresetUI (bool rebuild)
{
    const auto& ps = proc.getPresets();
    const juce::ScopedValueSetter<bool> guard (updatingUI, true);
    if (rebuild)
    {
        cats.clear(); cats.add ("All Presets"); cats.add (String::fromUTF8 ("\xe2\x98\x85 Favoritos"));
        for (auto& p : ps) cats.addIfNotAlreadyThere (p.category);
        catSel = jlimit (0, cats.size() - 1, catSel);
        shownIdx.clear();
        for (int i = 0; i < (int) ps.size(); ++i)
            if (catSel == 0 || (catSel == 1 && proc.isFavorite (i)) || (catSel > 1 && ps[(size_t) i].category == cats[catSel])) shownIdx.push_back (i);
        presetBox.clear (dontSendNotification);
        String lastCat; for (int i = 0; i < (int) ps.size(); ++i) { if (ps[(size_t) i].category != lastCat) { lastCat = ps[(size_t) i].category; presetBox.addSectionHeading (lastCat); } presetBox.addItem (ps[(size_t) i].name, i + 1); }
        catList.updateContent(); presetList.updateContent(); catList.selectRow (catSel, false, false);
    }
    const int cur = proc.getCurrentPreset();
    presetBox.setSelectedId (cur + 1, dontSendNotification);
    if (cur >= 0 && cur < (int) ps.size())
    {
        descLabel.setText (ps[(size_t) cur].name + "\n\n" + ps[(size_t) cur].description + "\n\nPuntos de partida inspirados en cada estilo; no son los ajustes reales de ningún artista.", dontSendNotification);
        for (int r = 0; r < (int) shownIdx.size(); ++r) if (shownIdx[(size_t) r] == cur) presetList.selectRow (r, false, false);
    }
    presetList.repaint(); repaint();
}

void YmyEditor::promptSave (bool saveAs)
{
    const auto& ps = proc.getPresets(); const int cur = proc.getCurrentPreset();
    String nm = (cur >= 0 && cur < (int) ps.size()) ? ps[(size_t) cur].name : String ("Mi preset");
    if (! saveAs && cur >= 0 && cur < (int) ps.size() && ps[(size_t) cur].user) { proc.saveUserPreset (nm); refreshPresetUI (true); return; }
    if (! saveAs || nm.isEmpty()) nm += " (user)";
    auto* w = new AlertWindow ("Guardar preset", "Nombre del preset:", MessageBoxIconType::NoIcon);
    w->addTextEditor ("n", nm); w->addButton ("Guardar", 1, KeyPress (KeyPress::returnKey)); w->addButton ("Cancelar", 0, KeyPress (KeyPress::escapeKey));
    Component::SafePointer<YmyEditor> self (this);
    w->enterModalState (true, ModalCallbackFunction::create ([self, w] (int r) {
        if (r == 1 && self != nullptr) { self->proc.saveUserPreset (w->getTextEditorContents ("n")); self->refreshPresetUI (true); }
    }), true);
}

void YmyEditor::timerCallback()
{
    const float in = proc.inLevel.load(), out = proc.outLevel.load();
    meterIn.level = in; meterOut.level = out; meterIn.repaint(); meterOut.repaint();
    // mic activity: threshold -50 dB, attack ~40 ms, release ~350 ms (30 Hz timer)
    const float db = Decibels::gainToDecibels (in, -80.f);
    const float target = jlimit (0.f, 1.f, (db + 50.f) / 45.f);
    glowState += (target - glowState) * (target > glowState ? 0.55f : 0.09f);
    mic.ledOn = led.getToggleState();
    if (std::abs (mic.glow - glowState) > 0.004f) { mic.glow = glowState; mic.repaint(); }
    const float hb = proc.hostBpm.load();
    hostBpmLabel.setText (hb > 20.f ? String (hb, 1) : String ("--"), dontSendNotification);
    preLabel.setText ("Predelay  " + String (proc.effectivePredelay.load(), 1) + " ms" + (sync.getToggleState() ? "  (sync)" : "  (free)"), dontSendNotification);
    kPre->s.setEnabled (! sync.getToggleState()); kPre->setAlpha (sync.getToggleState() ? 0.45f : 1.f);
    const int modeNow = (int) proc.apvts.getRawParameterValue ("mode")->load();
    if (modeBox.getSelectedId() != modeNow + 1) modeBox.setSelectedId (modeNow + 1, dontSendNotification);
    const bool aux = modeNow == 1 && proc.apvts.getRawParameterValue ("dry")->load() > 0.01f;
    warnLabel.setText (aux ? "Modo Aux Send: baja Dry a 0 % para no duplicar la señal seca" : "", dontSendNotification);
    const auto& ps = proc.getPresets();
    const String nm = proc.getCurrentPreset() < (int) ps.size() ? ps[(size_t) proc.getCurrentPreset()].name : String();
    if (presetBox.getText() != nm) presetBox.setText (nm, dontSendNotification);
    saveB.setButtonText (proc.isModified() ? "Save *" : "Save");
}

Rectangle<int> YmyEditor::R (float x, float y, float w, float h) const
{ const float s = (float) getWidth() / 1200.f; return { roundToInt (x * s), roundToInt (y * s), roundToInt (w * s), roundToInt (h * s) }; }

void YmyEditor::layoutRow (std::vector<Knob*> ks, Rectangle<int> a)
{ const int w = a.getWidth() / (int) ks.size(); for (size_t i = 0; i < ks.size(); ++i) ks[i]->setBounds (a.getX() + (int) i * w, a.getY(), w, a.getHeight()); }

void YmyEditor::paint (Graphics& g)
{
    g.fillAll (Colour (0xff201a1a));
    auto body = getLocalBounds().toFloat().reduced (6.f).withTrimmedTop (R (0, 0, 0, 44).getHeight() - 6.f);
    g.setGradientFill (ColourGradient (cream1, 0.f, body.getY(), cream2, 0.f, body.getBottom(), false)); g.fillRoundedRectangle (body, 10.f);
    g.setColour (Colour (0xff12100f)); g.fillRect (0, 0, getWidth(), R (0, 0, 0, 40).getHeight());
    auto section = [&] (float x, float y, float w, float h, const char* title)
    {
        auto r = R (x, y, w, h).toFloat(); g.setColour (panel); g.fillRoundedRectangle (r, 10.f);
        g.setColour (gold2.withAlpha (0.35f)); g.drawRoundedRectangle (r, 10.f, 1.2f);
        g.setColour (gold2); g.setFont (Serif (R (0, 0, 0, 17).getHeight() * 1.0f)); g.drawText (String (title).toUpperCase(), R (x + 18, y + 8, 200, 24), Justification::centredLeft);
    };
    section (20, 150, 430, 262, "Space"); section (20, 420, 430, 172, "Stereo"); section (20, 600, 395, 190, "Tempo");
    section (770, 150, 410, 255, "Tone"); section (690, 415, 240, 180, "Mix"); section (940, 415, 240, 180, "Ducking");
    section (425, 600, 515, 190, "Presets"); section (950, 600, 230, 190, "Mode");
    g.setColour (gold2); g.setFont (Serif (R (0, 0, 0, 44).getHeight() * 0.92f)); g.drawText ("YMYVerb", R (50, 52, 330, 70), Justification::centredLeft);
    g.setColour (ink.withAlpha (0.75f)); g.setFont (F (R (0, 0, 0, 15).getHeight() * 0.9f)); g.drawText ("V O C A L   R E V E R B", R (54, 118, 330, 22), Justification::centredLeft);
    g.setColour (gold2); g.setFont (Serif (R (0, 0, 0, 30).getHeight() * 0.9f, false).italicised()); g.drawText ("By Made ORYMY", R (790, 62, 380, 40), Justification::centredRight);
    g.setColour (ink.withAlpha (0.7f)); g.setFont (F (static_cast<float> (R (0, 0, 0, 12).getHeight()))); g.drawText ("THE SWEET SPACE OF BACHATA", R (790, 100, 380, 20), Justification::centredRight);
    g.setColour (Colour (0xffe6c777)); g.setFont (F (static_cast<float> (R (0, 0, 0, 14).getHeight()), true)); g.drawText ("YMYVerb", R (14, 8, 120, 24), Justification::centredLeft);
    g.setColour (ink); g.setFont (F (static_cast<float> (R (0, 0, 0, 12).getHeight()))); g.drawText ("IN", R (462, 150, 40, 14), Justification::centred); g.drawText ("OUT", R (698, 150, 40, 14), Justification::centred);
    g.setColour (gold2); g.setFont (Serif (static_cast<float> (R (0, 0, 0, 20).getHeight()))); g.drawText ("YMY", R (540, 566, 120, 28), Justification::centred);
}

void YmyEditor::resized()
{
    titleLabel.setBounds (0, 0, 0, 0);
    algo.setBounds (R (30, 188, 200, 28));
    layoutRow ({ kRoom, kDecay, kPre }, R (30, 222, 410, 92));
    layoutRow ({ kEarly, kDiff, kDens, kMod }, R (30, 316, 410, 92));
    layoutRow ({ kWidth, kBal, kErW, kSpread }, R (30, 452, 410, 130));
    layoutRow ({ kWarm, kDamp, kLow, kHigh }, R (780, 186, 390, 100));
    layoutRow ({ kLowD, kHighD, kTone }, R (780, 292, 300, 105));
    layoutRow ({ kDry, kWet, kIn, kOut }, R (698, 442, 224, 86)); kRet->setBounds (R (698, 528, 70, 62));
    duck.setBounds (R (775, 532, 70, 50)); makeup.setBounds (R (850, 532, 76, 50));
    layoutRow ({ kThr, kDepth }, R (948, 445, 224, 70)); layoutRow ({ kAtt, kRel }, R (948, 518, 224, 70));
    // tempo
    hostCap.setBounds (R (35, 636, 90, 16)); hostBpmLabel.setBounds (R (35, 654, 100, 30)); hostBpmLabel.setFont (F (static_cast<float> (R (0, 0, 0, 20).getHeight()), true));
    bpmCap.setBounds (R (160, 636, 90, 16)); manualBpm.setBounds (R (160, 654, 100, 30));
    sync.setBounds (R (290, 640, 100, 44));
    division.setBounds (R (35, 706, 120, 28)); triplet.setBounds (R (170, 696, 70, 46)); dotted.setBounds (R (245, 696, 70, 46));
    preLabel.setBounds (R (35, 750, 360, 28)); preLabel.setFont (F (static_cast<float> (R (0, 0, 0, 14).getHeight()), true));
    // presets
    catList.setBounds (R (437, 640, 105, 140)); presetList.setBounds (R (546, 640, 190, 140)); descLabel.setBounds (R (742, 640, 190, 140));
    descLabel.setFont (F (static_cast<float> (R (0, 0, 0, 12).getHeight())));
    // header
    prev.setBounds (R (300, 6, 28, 28)); presetBox.setBounds (R (332, 6, 300, 28)); next.setBounds (R (636, 6, 28, 28));
    saveB.setBounds (R (672, 6, 70, 28)); saveAsB.setBounds (R (746, 6, 76, 28)); favB.setBounds (R (826, 6, 46, 28));
    abB.setBounds (R (876, 6, 96, 28)); undoB.setBounds (R (976, 6, 56, 28)); redoB.setBounds (R (1036, 6, 56, 28)); modeBox.setBounds (R (1100, 6, 90, 28));
    // mode
    led.setBounds (R (1070, 680, 80, 50)); warnLabel.setBounds (R (958, 735, 214, 48)); warnLabel.setFont (F (static_cast<float> (R (0, 0, 0, 11).getHeight())));
    meterIn.setBounds (R (468, 170, 26, 240)); meterOut.setBounds (R (706, 170, 26, 240));
    mic.setBounds (R (500, 44, 200, 546));
}
