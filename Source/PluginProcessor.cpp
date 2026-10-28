#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <BinaryData.h>

using namespace juce;

AudioProcessorValueTreeState::ParameterLayout YmyProcessor::createLayout()
{
    std::vector<std::unique_ptr<RangedAudioParameter>> v;
    auto pct = [&] (const char* id, const char* name, float def)
    {
        v.push_back (std::make_unique<AudioParameterFloat> (ParameterID { id, 1 }, name, NormalisableRange<float> (0.f, 1.f, 0.001f), def,
            AudioParameterFloatAttributes().withStringFromValueFunction ([] (float x, int) { return String (roundToInt (x * 100.f)) + "%"; })
                                           .withValueFromStringFunction ([] (const String& s) { return s.getFloatValue() / 100.f; })));
    };
    auto flt = [&] (const char* id, const char* name, NormalisableRange<float> r, float def, String suffix, int dec)
    {
        v.push_back (std::make_unique<AudioParameterFloat> (ParameterID { id, 1 }, name, r, def,
            AudioParameterFloatAttributes().withStringFromValueFunction ([suffix, dec] (float x, int) { return String (x, dec) + suffix; })));
    };
    auto boo = [&] (const char* id, const char* name, bool def) { v.push_back (std::make_unique<AudioParameterBool> (ParameterID { id, 1 }, name, def)); };
    auto cho = [&] (const char* id, const char* name, StringArray c, int def) { v.push_back (std::make_unique<AudioParameterChoice> (ParameterID { id, 1 }, name, c, def)); };

    cho ("algo", "Algorithm", { "Plate", "Room", "Chamber", "Hall", "Vintage Digital", "Hybrid Bachata" }, 0);
    pct ("room", "Room Size", 0.45f);
    flt ("decay", "Decay", NormalisableRange<float> (0.2f, 20.f, 0.01f, 0.4f), 2.8f, "s", 1);
    flt ("predelay", "Predelay", NormalisableRange<float> (0.f, 250.f, 0.1f), 32.f, "ms", 0);
    pct ("early", "Early Reflections", 0.55f);
    pct ("diffusion", "Diffusion", 0.75f);
    pct ("density", "Density", 0.7f);
    pct ("mod", "Modulation", 0.25f);
    pct ("warmth", "Warmth", 0.55f);
    pct ("damping", "Damping", 0.45f);
    flt ("lowcut", "Low Cut", NormalisableRange<float> (20.f, 600.f, 1.f, 0.45f), 120.f, "Hz", 0);
    v.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "highcut", 1 }, "High Cut", NormalisableRange<float> (2000.f, 20000.f, 10.f, 0.5f), 8000.f,
        AudioParameterFloatAttributes().withStringFromValueFunction ([] (float x, int) { return String (x / 1000.f, 1) + "kHz"; })));
    pct ("lowdecay", "Low Decay", 0.4f);
    pct ("highdecay", "High Decay", 0.5f);
    pct ("tone", "Reverb Tone", 0.6f);
    pct ("width", "Width", 0.85f);
    v.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "balance", 1 }, "Balance", NormalisableRange<float> (-1.f, 1.f, 0.01f), 0.f,
        AudioParameterFloatAttributes().withStringFromValueFunction ([] (float x, int) { return String (roundToInt (x * 100.f)) + "%"; })));
    pct ("erwidth", "Early Ref. Width", 0.7f);
    pct ("spread", "Reverb Spread", 0.8f);
    pct ("dry", "Dry", 1.0f);
    pct ("wet", "Wet", 0.28f);
    flt ("in", "Input", NormalisableRange<float> (-24.f, 12.f, 0.1f), 0.f, "dB", 1);
    flt ("out", "Output", NormalisableRange<float> (-24.f, 12.f, 0.1f), 0.f, "dB", 1);
    flt ("ret", "Reverb Return", NormalisableRange<float> (-24.f, 12.f, 0.1f), 0.f, "dB", 1);
    boo ("duckon", "Ducking", false);
    flt ("duckthr", "Threshold", NormalisableRange<float> (-60.f, 0.f, 0.1f), -30.f, "dB", 0);
    flt ("duckdepth", "Depth", NormalisableRange<float> (0.f, 30.f, 0.1f), 10.f, "dB", 0);
    flt ("duckatt", "Attack", NormalisableRange<float> (1.f, 200.f, 0.1f, 0.5f), 10.f, "ms", 0);
    flt ("duckrel", "Release", NormalisableRange<float> (20.f, 1500.f, 1.f, 0.5f), 300.f, "ms", 0);
    boo ("makeup", "Auto Makeup", true);
    boo ("sync", "Sync", true);
    flt ("bpm", "Manual BPM", NormalisableRange<float> (40.f, 240.f, 0.1f), 130.f, "", 1);
    cho ("div", "Division", { "1/64", "1/32", "1/16", "1/8", "1/4", "1/2", "1/1" }, 1);
    boo ("triplet", "Triplet", false);
    boo ("dotted", "Dotted", false);
    cho ("mode", "Mode", { "Insert", "Aux Send" }, 0);
    boo ("led", "LED", true);
    return { v.begin(), v.end() };
}

YmyProcessor::YmyProcessor()
    : AudioProcessor (BusesProperties().withInput ("Input", AudioChannelSet::stereo(), true).withOutput ("Output", AudioChannelSet::stereo(), true)),
      apvts (*this, &undo, "YMYVERB", createLayout())
{
    #define CACHE(x) p_##x = apvts.getRawParameterValue (#x);
    CACHE(algo) CACHE(room) CACHE(decay) CACHE(predelay) CACHE(early) CACHE(diffusion) CACHE(density) CACHE(mod) CACHE(warmth) CACHE(damping)
    CACHE(lowcut) CACHE(highcut) CACHE(lowdecay) CACHE(highdecay) CACHE(tone) CACHE(width) CACHE(balance) CACHE(erwidth) CACHE(spread)
    CACHE(dry) CACHE(wet) CACHE(in) CACHE(out) CACHE(ret) CACHE(duckon) CACHE(duckthr) CACHE(duckdepth) CACHE(duckatt) CACHE(duckrel)
    CACHE(makeup) CACHE(sync) CACHE(bpm) CACHE(div) CACHE(triplet) CACHE(dotted)
    #undef CACHE
    monoScratch.assign (8192, 0.f);
    loadFactory();
    loadUserPresets();
    {
        auto f = favoritesFile();
        if (f.existsAsFile()) favorites.addLines (f.loadFileAsString());
    }
    apvts.state.addListener (this);
}

YmyProcessor::~YmyProcessor() { apvts.state.removeListener (this); }

bool YmyProcessor::isBusesLayoutSupported (const BusesLayout& l) const
{
    const auto o = l.getMainOutputChannelSet();
    return (o == AudioChannelSet::mono() || o == AudioChannelSet::stereo()) && o == l.getMainInputChannelSet();
}

void YmyProcessor::prepareToPlay (double sr, int)
{
    sampleRateHz = sr;
    engine.prepare (sr);
}
void YmyProcessor::releaseResources() { engine.reset(); }

void YmyProcessor::processBlock (AudioBuffer<float>& buf, MidiBuffer&)
{
    ScopedNoDenormals noDenormals;
    const int n = buf.getNumSamples(), nch = buf.getNumChannels();
    if (nch < 1) return;

    double bpm = 0.0;
    if (auto* ph = getPlayHead())
        if (auto pos = ph->getPosition())
            if (auto b = pos->getBpm()) bpm = *b;
    hostBpm.store ((float) bpm);

    orymy::Params p;
    p.algo = (int) p_algo->load(); p.roomSize = p_room->load(); p.decay = p_decay->load();
    const bool sync = p_sync->load() > 0.5f;
    const double useBpm = (bpm > 20.0 && bpm < 400.0) ? bpm : (double) p_bpm->load();
    p.predelayMs = sync ? (float) orymy::syncedMs (useBpm, (int) p_div->load(), p_triplet->load() > 0.5f, p_dotted->load() > 0.5f)
                        : p_predelay->load();
    effectivePredelay.store (p.predelayMs);
    p.early = p_early->load(); p.diffusion = p_diffusion->load(); p.density = p_density->load(); p.modulation = p_mod->load();
    p.warmth = p_warmth->load(); p.damping = p_damping->load(); p.lowCutHz = p_lowcut->load(); p.highCutHz = p_highcut->load();
    p.lowDecay = p_lowdecay->load(); p.highDecay = p_highdecay->load(); p.tone = p_tone->load(); p.width = p_width->load();
    p.balance = p_balance->load(); p.erWidth = p_erwidth->load(); p.spread = p_spread->load(); p.dry = p_dry->load(); p.wet = p_wet->load();
    p.inDb = p_in->load(); p.outDb = p_out->load(); p.retDb = p_ret->load();
    p.duckOn = p_duckon->load() > 0.5f; p.duckThrDb = p_duckthr->load(); p.duckDepthDb = p_duckdepth->load();
    p.duckAttMs = p_duckatt->load(); p.duckRelMs = p_duckrel->load(); p.autoMakeup = p_makeup->load() > 0.5f;
    engine.setParams (p);

    float peakIn = 0.f;
    for (int c = 0; c < nch; ++c) peakIn = jmax (peakIn, buf.getMagnitude (c, 0, n));
    inLevel.store (jmax (peakIn, inLevel.load() * 0.92f));

    float* L = buf.getWritePointer (0);
    float* R = nch > 1 ? buf.getWritePointer (1) : nullptr;
    const int chunk = (int) monoScratch.size();
    for (int pos = 0; pos < n; pos += chunk)
    {
        const int len = jmin (chunk, n - pos);
        if (R != nullptr) engine.process (L + pos, R + pos, len);
        else
        {
            std::copy (L + pos, L + pos + len, monoScratch.begin());
            engine.process (L + pos, monoScratch.data(), len);
            for (int i = 0; i < len; ++i) L[pos + i] = 0.5f * (L[pos + i] + monoScratch[(size_t) i]);
        }
    }
    float peakOut = 0.f;
    for (int c = 0; c < nch; ++c) peakOut = jmax (peakOut, buf.getMagnitude (c, 0, n));
    outLevel.store (jmax (peakOut, outLevel.load() * 0.92f));
}

// ---------------------------------------------------------------- presets
void YmyProcessor::loadFactory()
{
    auto parsed = JSON::parse (String::fromUTF8 (BinaryData::factory_presets_json, BinaryData::factory_presets_jsonSize));
    if (auto* arr = parsed.getArray())
        for (auto& e : *arr)
            presets.push_back ({ e["name"].toString(), e["category"].toString(), e["description"].toString(), e["params"], false });
}

File YmyProcessor::getUserPresetDir() const
{
    return File::getSpecialLocation (File::userDocumentsDirectory).getChildFile ("YMY Records").getChildFile ("YMYVerb").getChildFile ("Presets");
}
File YmyProcessor::favoritesFile() const { return getUserPresetDir().getParentDirectory().getChildFile ("favorites.txt"); }

void YmyProcessor::loadUserPresets()
{
    auto dir = getUserPresetDir();
    if (! dir.isDirectory()) return;
    for (auto& f : dir.findChildFiles (File::findFiles, false, "*.json"))
    {
        auto v = JSON::parse (f);
        if (v.isObject() && v["params"].isObject())
            presets.push_back ({ v["name"].toString(), "Mis presets", v["description"].toString(), v["params"], true });
    }
}

void YmyProcessor::refreshPresets()
{
    presets.erase (std::remove_if (presets.begin(), presets.end(), [] (const Preset& p) { return p.user; }), presets.end());
    loadUserPresets();
}

void YmyProcessor::loadPreset (int index)
{
    if (index < 0 || index >= (int) presets.size()) return;
    undo.beginNewTransaction();
    loadingPreset = true;
    if (auto* obj = presets[(size_t) index].params.getDynamicObject())
        for (auto& nv : obj->getProperties())
            if (auto* prm = apvts.getParameter (nv.name.toString()))
                prm->setValueNotifyingHost (prm->convertTo0to1 (nv.value.isBool() ? ((bool) nv.value ? 1.f : 0.f) : (float) (double) nv.value));
    loadingPreset = false;
    engine.reset();                         // clean tail on preset change
    currentPreset = index;
    modified = false;
}

bool YmyProcessor::saveUserPreset (const String& name)
{
    auto clean = File::createLegalFileName (name.trim());
    if (clean.isEmpty()) return false;
    auto dir = getUserPresetDir();
    if (! dir.createDirectory().wasOk()) return false;
    auto* obj = new DynamicObject();
    for (auto* prm : getParameters())
        if (auto* r = dynamic_cast<RangedAudioParameter*> (prm))
        {
            const auto id = r->paramID;
            if (id == "led") continue;
            obj->setProperty (id, r->convertFrom0to1 (r->getValue()));
        }
    auto* root = new DynamicObject();
    root->setProperty ("name", name.trim()); root->setProperty ("category", "Mis presets"); root->setProperty ("schema", 1);
    root->setProperty ("description", "Preset de usuario"); root->setProperty ("params", var (obj));
    if (! dir.getChildFile (clean + ".json").replaceWithText (JSON::toString (var (root)), false, false, "\n")) return false;
    refreshPresets();
    for (int i = 0; i < (int) presets.size(); ++i) if (presets[(size_t) i].user && presets[(size_t) i].name == name.trim()) currentPreset = i;
    modified = false;
    return true;
}

bool YmyProcessor::isFavorite (int i) const { return i >= 0 && i < (int) presets.size() && favorites.contains (presets[(size_t) i].name); }
void YmyProcessor::toggleFavorite (int i)
{
    if (i < 0 || i >= (int) presets.size()) return;
    const auto n = presets[(size_t) i].name;
    if (favorites.contains (n)) favorites.removeString (n); else favorites.add (n);
    auto f = favoritesFile(); f.getParentDirectory().createDirectory();
    f.replaceWithText (favorites.joinIntoString ("\n"));
}

void YmyProcessor::toggleAB()
{
    abState[abSlot] = apvts.copyState();
    abSlot ^= 1;
    if (abState[abSlot].isValid()) apvts.replaceState (abState[abSlot].createCopy());
}

void YmyProcessor::valueTreePropertyChanged (ValueTree& t, const Identifier&)
{
    if (! loadingPreset && t.hasType ("PARAM")) modified = true;
}

void YmyProcessor::getStateInformation (MemoryBlock& dest)
{
    auto s = apvts.copyState();
    s.setProperty ("presetIndex", currentPreset, nullptr);
    if (auto xml = s.createXml()) copyXmlToBinary (*xml, dest);
}
void YmyProcessor::setStateInformation (const void* data, int size)
{
    if (auto xml = getXmlFromBinary (data, size))
        if (xml->hasTagName (apvts.state.getType()))
        {
            auto s = ValueTree::fromXml (*xml);
            currentPreset = jlimit (0, jmax (0, (int) presets.size() - 1), (int) s.getProperty ("presetIndex", 0));
            loadingPreset = true; apvts.replaceState (s); loadingPreset = false;
        }
}

AudioProcessorEditor* YmyProcessor::createEditor() { return new YmyEditor (*this); }
AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new YmyProcessor(); }
