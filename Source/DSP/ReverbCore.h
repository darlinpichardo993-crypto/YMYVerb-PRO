#pragma once
// YMYVerb DSP core: pure C++ (no JUCE) so it can be unit-tested anywhere.
// Late reverb: 8-line FDN, Householder feedback matrix, per-line two-band RT60 control,
// per-line allpass (density), modulated delays, input allpass diffusion, early reflections,
// predelay, tone, stereo, ducking. No allocation in process().
#include <vector>
#include <array>
#include <cmath>
#include <algorithm>
#include <cstdint>

namespace orymy {

constexpr float kPi = 3.14159265358979f;
inline float flush (float x) { return std::fabs (x) < 1.0e-20f ? 0.0f : x; }
inline float dbToGain (float db) { return std::pow (10.0f, db * 0.05f); }
inline int nextPow2 (int v) { int p = 1; while (p < v) p <<= 1; return p; }
inline bool isPrime (int n) { if (n < 2) return false; for (int i = 2; i * i <= n; ++i) if (n % i == 0) return false; return true; }
inline int nearestPrime (int n) { while (! isPrime (n)) ++n; return n; }

struct OnePole
{
    float a = 0, z = 0;
    void setCutoff (float fc, float sr) { fc = std::min (fc, sr * 0.45f); a = 1.0f - std::exp (-2.0f * kPi * fc / sr); }
    float lp (float x) { z += a * (x - z); z = flush (z); return z; }
    void reset() { z = 0; }
};

struct Biquad
{
    float b0 = 1, b1 = 0, b2 = 0, a1 = 0, a2 = 0, z1 = 0, z2 = 0;
    void setLP (float fc, float sr) { coeffs (fc, sr, false); }
    void setHP (float fc, float sr) { coeffs (fc, sr, true); }
    void coeffs (float fc, float sr, bool hp)
    {
        fc = std::min (std::max (fc, 10.0f), sr * 0.45f);
        const float w = 2 * kPi * fc / sr, c = std::cos (w), s = std::sin (w), q = 0.70710678f;
        const float al = s / (2 * q), a0 = 1 + al;
        if (hp) { b0 = (1 + c) / 2 / a0; b1 = -(1 + c) / a0; b2 = b0; }
        else    { b0 = (1 - c) / 2 / a0; b1 = (1 - c) / a0;  b2 = b0; }
        a1 = -2 * c / a0; a2 = (1 - al) / a0;
    }
    float process (float x)
    {
        const float y = b0 * x + z1;
        z1 = flush (b1 * x - a1 * y + z2);
        z2 = flush (b2 * x - a2 * y);
        return y;
    }
    void reset() { z1 = z2 = 0; }
};

struct DelayLine
{
    std::vector<float> buf; int mask = 0, w = 0;
    void init (int maxSamples) { buf.assign ((size_t) nextPow2 (maxSamples + 4), 0.0f); mask = (int) buf.size() - 1; w = 0; }
    void clear() { std::fill (buf.begin(), buf.end(), 0.0f); w = 0; }
    inline void push (float x) { buf[(size_t) w] = x; w = (w + 1) & mask; }
    inline float read (float d) const   // fractional delay, d >= 1
    {
        const int i = (int) d; const float f = d - (float) i;
        const float a = buf[(size_t) ((w - 1 - i) & mask)], b = buf[(size_t) ((w - 2 - i) & mask)];
        return a + f * (b - a);
    }
    inline float readInt (int d) const { return buf[(size_t) ((w - 1 - d) & mask)]; }
};

struct Allpass
{
    DelayLine d; int len = 1; float g = 0.5f;
    void init (int maxLen) { d.init (maxLen); }
    void set (int l, float gain) { len = std::max (1, l); g = gain; }
    inline float process (float x)
    {
        const float del = d.readInt (len - 1);
        const float v = x - g * del;
        d.push (v);
        return flush (del + g * v);
    }
    void clear() { d.clear(); }
};

struct Params
{
    int   algo = 0;                 // 0 Plate 1 Room 2 Chamber 3 Hall 4 Vintage 5 Hybrid
    float roomSize = 0.45f, decay = 2.8f, predelayMs = 32.0f, early = 0.6f, diffusion = 0.75f,
          density = 0.7f, modulation = 0.25f, warmth = 0.55f, damping = 0.45f,
          lowCutHz = 120.0f, highCutHz = 8000.0f, lowDecay = 0.4f, highDecay = 0.5f, tone = 0.6f,
          width = 0.85f, balance = 0.0f, erWidth = 0.7f, spread = 0.8f,
          dry = 1.0f, wet = 0.25f, inDb = 0.0f, outDb = 0.0f, retDb = 0.0f;
    bool  duckOn = false; float duckThrDb = -30.0f, duckDepthDb = 12.0f, duckAttMs = 10.0f, duckRelMs = 250.0f;
    bool  autoMakeup = true;
};

struct AlgoShape { float delayScale, diffBias, modScale, erScale, erLevel, dampBias, brightness; };
static const AlgoShape kAlgo[6] = {
    // delayScale diffBias modScale erScale erLevel dampBias brightness
    { 0.85f,  0.10f, 1.00f, 0.55f, 0.35f, 0.00f, 1.00f },   // Plate: dense, smooth
    { 0.45f,  0.00f, 0.60f, 1.00f, 1.00f, 0.05f, 0.95f },   // Room: short, early heavy
    { 0.70f,  0.05f, 0.80f, 0.90f, 0.80f, 0.10f, 0.90f },   // Chamber
    { 1.40f,  0.05f, 1.10f, 1.30f, 0.60f, 0.00f, 1.00f },   // Hall: big
    { 0.95f, -0.05f, 1.60f, 0.70f, 0.45f, 0.15f, 0.80f },   // Vintage Digital: darker, more motion
    { 1.00f,  0.10f, 1.20f, 0.80f, 0.55f, 0.08f, 0.92f } }; // Hybrid Bachata: warm, dense

class ReverbEngine
{
public:
    static constexpr int N = 8;

    void prepare (double sampleRate)
    {
        sr = (float) sampleRate;
        const int maxLine = (int) (sr * 0.45f);
        for (int i = 0; i < N; ++i) { line[i].init (maxLine + 64); apd[i].init ((int) (sr * 0.02f) + 8); }
        for (int c = 0; c < 2; ++c)
        {
            pre[c].init ((int) (sr * 0.30f) + 64); er[c].init ((int) (sr * 0.25f) + 64);
            for (int k = 0; k < 4; ++k) inAp[c][k].init ((int) (sr * 0.03f) + 8);
        }
        smoothCoef = 1.0f - std::exp (-(float) kSub / (sr * 0.05f));
        reset();
        haveInit = false;
    }

    void reset()
    {
        for (int i = 0; i < N; ++i) { line[i].clear(); apd[i].clear(); lowLP[i].reset(); damp[i].reset(); }
        for (int c = 0; c < 2; ++c) { pre[c].clear(); er[c].clear(); for (auto& a : inAp[c]) a.clear(); hp[c].reset(); lp[c].reset(); tiltLP[c].reset(); }
        duckEnv = 0; duckGainDb = 0;
        for (int i = 0; i < N; ++i) lfoPhase[(size_t) i] = (float) i / N;
    }

    void setParams (const Params& p)
    {
        target = p;
        if (! haveInit) { cur = p; haveInit = true; }
    }

    // L/R processed in place. Safe for any block size.
    void process (float* L, float* R, int n)
    {
        int pos = 0;
        while (pos < n)
        {
            const int len = std::min (kSub, n - pos);
            smoothParams();
            updateCoefficients();
            processSub (L + pos, R + pos, len);
            pos += len;
        }
    }

private:
    static constexpr int kSub = 32;
    float sr = 48000;
    Params cur, target; bool haveInit = false; float smoothCoef = 0.01f;

    std::array<DelayLine, N> line; std::array<Allpass, N> apd;
    std::array<OnePole, N> lowLP, damp;
    std::array<float, N> dly {}, gLo {}, gHi {}, lfoPhase {}, lfoInc {};
    float lfoDepth = 0, apGain = 0;
    DelayLine pre[2], er[2]; Allpass inAp[2][4];
    Biquad hp[2], lp[2]; OnePole tiltLP[2];
    float wetGain = 0, retGain = 1, inGain = 1, outGain = 1, dryGain = 1, makeup = 1;
    float erTapsL[8] {}, erTapsR[8] {}, erGainL[8] {}, erGainR[8] {};
    float tiltLo = 1, tiltHi = 1, balL = 1, balR = 1, erLevel = 0, sideGain = 1, spreadAmt = 1;
    float duckEnv = 0, duckGainDb = 0;

    static void sm (float& c, float t, float k) { c += k * (t - c); }

    void smoothParams()
    {
        const float k = smoothCoef;
        cur.algo = target.algo;
        sm (cur.roomSize, target.roomSize, k); sm (cur.decay, target.decay, k);
        sm (cur.predelayMs, target.predelayMs, k * 0.5f);
        sm (cur.early, target.early, k); sm (cur.diffusion, target.diffusion, k); sm (cur.density, target.density, k);
        sm (cur.modulation, target.modulation, k); sm (cur.warmth, target.warmth, k); sm (cur.damping, target.damping, k);
        sm (cur.lowCutHz, target.lowCutHz, k); sm (cur.highCutHz, target.highCutHz, k);
        sm (cur.lowDecay, target.lowDecay, k); sm (cur.highDecay, target.highDecay, k); sm (cur.tone, target.tone, k);
        sm (cur.width, target.width, k); sm (cur.balance, target.balance, k); sm (cur.erWidth, target.erWidth, k); sm (cur.spread, target.spread, k);
        sm (cur.dry, target.dry, k); sm (cur.wet, target.wet, k); sm (cur.inDb, target.inDb, k); sm (cur.outDb, target.outDb, k); sm (cur.retDb, target.retDb, k);
        cur.duckOn = target.duckOn; cur.duckThrDb = target.duckThrDb; cur.duckDepthDb = target.duckDepthDb;
        cur.duckAttMs = target.duckAttMs; cur.duckRelMs = target.duckRelMs; cur.autoMakeup = target.autoMakeup;
    }

    void updateCoefficients()
    {
        const AlgoShape& a = kAlgo[std::min (std::max (cur.algo, 0), 5)];
        const float rt60 = std::min (std::max (cur.decay, 0.1f), 20.0f);
        static const float baseMs[N] = { 29.7f, 37.1f, 41.1f, 43.7f, 53.3f, 59.9f, 67.3f, 71.3f };
        const float scale = a.delayScale * (0.4f + 1.6f * cur.roomSize);
        const float lowMult  = std::pow (2.0f, (cur.lowDecay  - 0.5f) * 2.0f);
        const float highMult = std::pow (2.0f, (cur.highDecay - 0.5f) * 2.0f);
        const float dampFc = 18000.0f * std::pow (2000.0f / 18000.0f, std::min (1.0f, cur.damping + a.dampBias))
                             * std::pow (0.65f, cur.warmth) * a.brightness;
        apGain = 0.55f * cur.density;
        const float maxLen = (float) line[0].buf.size() - 8.0f;
        for (int i = 0; i < N; ++i)
        {
            const float apMs = 3.1f + 1.7f * (float) i;
            const int apSamp = nearestPrime ((int) (apMs * 0.001f * sr));
            apd[(size_t) i].set (apSamp, apGain);
            const float d = std::min (baseMs[i] * scale * 0.001f * sr, maxLen - 64.0f);
            dly[(size_t) i] = std::max (d, 16.0f);
            const float total = dly[(size_t) i] + (float) apSamp;
            gLo[(size_t) i] = std::min (0.9995f, std::pow (10.0f, -3.0f * total / (rt60 * lowMult  * sr)));
            gHi[(size_t) i] = std::min (0.9995f, std::pow (10.0f, -3.0f * total / (rt60 * highMult * sr)));
            lowLP[(size_t) i].setCutoff (300.0f, sr);
            damp[(size_t) i].setCutoff (dampFc, sr);
            lfoInc[(size_t) i] = (0.23f + 0.11f * (float) i) / sr;
        }
        lfoDepth = std::min (cur.modulation * a.modScale * 0.0018f * sr, 40.0f);

        const float apg = std::min (0.7f, 0.25f + 0.45f * std::min (1.0f, cur.diffusion + a.diffBias));
        static const int apBase[4] = { 142, 107, 379, 277 };
        for (int c = 0; c < 2; ++c)
            for (int k = 0; k < 4; ++k)
                inAp[c][k].set (nearestPrime ((int) ((float) apBase[k] * (c ? 1.07f : 1.0f) * sr / 44100.0f * 0.5f)), apg);

        static const float erMs[8] = { 5.3f, 9.1f, 13.7f, 19.3f, 26.9f, 33.1f, 41.9f, 52.7f };
        static const float erOffR[8] = { 1.17f, 0.93f, 1.21f, 0.89f, 1.13f, 0.97f, 1.19f, 0.91f };
        for (int t = 0; t < 8; ++t)
        {
            erTapsL[t] = std::max (1.0f, erMs[t] * a.erScale * (0.6f + 0.8f * cur.roomSize) * 0.001f * sr);
            erTapsR[t] = std::max (1.0f, erTapsL[t] * erOffR[t]);
            const float g = std::pow (0.82f, (float) t) * ((t & 1) ? -1.0f : 1.0f);
            erGainL[t] = g; erGainR[t] = -g * ((t & 2) ? -1.0f : 1.0f);
        }
        erLevel = cur.early * a.erLevel * 0.45f;

        hp[0].setHP (cur.lowCutHz, sr);
        hp[1].b0 = hp[0].b0; hp[1].b1 = hp[0].b1; hp[1].b2 = hp[0].b2; hp[1].a1 = hp[0].a1; hp[1].a2 = hp[0].a2;
        lp[0].setLP (std::min (cur.highCutHz, sr * 0.45f), sr);
        lp[1].b0 = lp[0].b0; lp[1].b1 = lp[0].b1; lp[1].b2 = lp[0].b2; lp[1].a1 = lp[0].a1; lp[1].a2 = lp[0].a2;
        tiltLP[0].setCutoff (1000.0f, sr); tiltLP[1].setCutoff (1000.0f, sr);
        const float tl = std::pow (2.0f, (0.5f - cur.tone) * 1.2f) * (1.0f + 0.3f * (cur.warmth - 0.5f));
        tiltLo = tl; tiltHi = 1.0f / tl;

        sideGain = std::min (1.6f, cur.width * 1.4f);
        spreadAmt = std::min (1.0f, std::max (0.0f, cur.spread));
        balL = std::min (1.0f, 1.0f - cur.balance); balR = std::min (1.0f, 1.0f + cur.balance);

        inGain = dbToGain (cur.inDb); outGain = dbToGain (cur.outDb); retGain = dbToGain (cur.retDb);
        dryGain = cur.dry; wetGain = cur.wet;
        makeup = cur.autoMakeup ? std::min (1.5f, std::max (0.5f, 1.0f / std::sqrt (rt60 / 2.5f))) : 1.0f;
    }

    void processSub (float* L, float* R, int n)
    {
        static const float sgnL[N] = { 1, -1, 1, -1, 1, -1, 1, -1 };
        static const float sgnR[N] = { 1, 1, -1, -1, 1, 1, -1, -1 };
        const float attC = std::exp (-1.0f / (std::max (0.1f, cur.duckAttMs) * 0.001f * sr));
        const float relC = std::exp (-1.0f / (std::max (1.0f, cur.duckRelMs) * 0.001f * sr));
        const float detRel = std::exp (-1.0f / (0.05f * sr));
        for (int s = 0; s < n; ++s)
        {
            float xl = L[s] * inGain, xr = R[s] * inGain;
            if (! (std::fabs (xl) < 1.0e4f)) xl = 0.0f;     // NaN/inf guard
            if (! (std::fabs (xr) < 1.0e4f)) xr = 0.0f;

            float duckLin = 1.0f;
            if (cur.duckOn)
            {
                const float lvl = std::max (std::fabs (xl), std::fabs (xr));
                duckEnv = lvl > duckEnv ? lvl : lvl + detRel * (duckEnv - lvl);
                duckEnv = flush (duckEnv);
                const float envDb = 20.0f * std::log10 (duckEnv + 1.0e-9f);
                const float amt = std::min (1.0f, std::max (0.0f, (envDb - cur.duckThrDb) / 6.0f));
                const float tgt = -cur.duckDepthDb * amt;
                const float c = tgt < duckGainDb ? attC : relC;
                duckGainDb = tgt + c * (duckGainDb - tgt);
                duckLin = dbToGain (duckGainDb);
            }
            else if (duckGainDb < -0.001f) { duckGainDb *= relC; duckLin = dbToGain (duckGainDb); }

            pre[0].push (xl); pre[1].push (xr);
            const float pd = std::max (1.0f, cur.predelayMs * 0.001f * sr);
            const float pl = pre[0].read (pd), pr = pre[1].read (pd);

            er[0].push ((pl + pr) * 0.5f); er[1].push ((pl - pr) * 0.5f + (pl + pr) * 0.25f);
            float eL = 0, eR = 0;
            for (int t = 0; t < 8; ++t) { eL += er[0].read (erTapsL[t]) * erGainL[t]; eR += er[1].read (erTapsR[t]) * erGainR[t]; }
            const float eM = (eL + eR) * 0.5f, eS = (eL - eR) * 0.5f * cur.erWidth * 1.4f;
            eL = (eM + eS) * erLevel; eR = (eM - eS) * erLevel;

            float dl = pl, dr = pr;
            for (int k = 0; k < 4; ++k) { dl = inAp[0][k].process (dl); dr = inAp[1][k].process (dr); }

            float f[N], sum = 0;
            for (int i = 0; i < N; ++i)
            {
                lfoPhase[(size_t) i] += lfoInc[(size_t) i]; if (lfoPhase[(size_t) i] >= 1.0f) lfoPhase[(size_t) i] -= 1.0f;
                const float m = std::sin (2.0f * kPi * lfoPhase[(size_t) i]) * lfoDepth;
                float x = line[(size_t) i].read (std::max (4.0f, dly[(size_t) i] + m));
                x = apd[(size_t) i].process (x);
                const float lo = lowLP[(size_t) i].lp (x);
                const float hi = damp[(size_t) i].lp (x - lo);
                f[i] = flush (lo * gLo[(size_t) i] + hi * gHi[(size_t) i]);
                sum += f[i];
            }
            const float hh = sum * (2.0f / N);
            float lateL = 0, lateR = 0;
            for (int i = 0; i < N; ++i)
            {
                const float in = (i & 1) ? dr : dl;
                float v = f[i] - hh + in * 0.35f;
                if (! (std::fabs (v) < 8.0f)) v = std::isnan (v) ? 0.0f : std::max (-8.0f, std::min (8.0f, v));
                line[(size_t) i].push (v);
                lateL += f[i] * sgnL[i]; lateR += f[i] * sgnR[i];
            }
            lateL *= 0.35f; lateR *= 0.35f;

            const float lm = (lateL + lateR) * 0.5f;
            float wl = eL + lm + (lateL - lm) * spreadAmt, wr = eR + lm + (lateR - lm) * spreadAmt;
            const float mid = (wl + wr) * 0.5f, side = (wl - wr) * 0.5f * sideGain;
            wl = (mid + side) * balL; wr = (mid - side) * balR;

            wl = lp[0].process (hp[0].process (wl)); wr = lp[1].process (hp[1].process (wr));
            float lo = tiltLP[0].lp (wl); wl = lo * tiltLo + (wl - lo) * tiltHi;
            lo = tiltLP[1].lp (wr);       wr = lo * tiltLo + (wr - lo) * tiltHi;

            const float g = wetGain * retGain * makeup * duckLin;
            L[s] = (xl * dryGain + wl * g) * outGain;
            R[s] = (xr * dryGain + wr * g) * outGain;
        }
    }
};

} // namespace orymy
