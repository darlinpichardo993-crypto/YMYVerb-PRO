// Standalone DSP tests (no JUCE). Build: g++ -O2 -std=c++17 Tests/test_dsp.cpp -o test_dsp
#include "../Source/DSP/ReverbCore.h"
#include "../Source/DSP/TempoSync.h"
#include <cstdio>
#include <random>
using namespace orymy;
static int fails = 0;
#define CHECK(c, msg) do { if (!(c)) { std::printf("  FAIL: %s\n", msg); ++fails; } else std::printf("  ok:   %s\n", msg); } while (0)

static bool finite (const std::vector<float>& v, float lim, float& peak)
{ peak = 0; for (float x : v) { if (!(std::fabs (x) < lim)) return false; peak = std::max (peak, std::fabs (x)); } return true; }

static void run (ReverbEngine& e, std::vector<float>& L, std::vector<float>& R, int block)
{ for (size_t p = 0; p < L.size(); p += (size_t) block) e.process (&L[p], &R[p], (int) std::min<size_t> ((size_t) block, L.size() - p)); }

static double measureRT60 (double sr, float decay, int algo)
{
    ReverbEngine e; e.prepare (sr);
    Params p; p.algo = algo; p.decay = decay; p.dry = 0; p.wet = 1; p.early = 0; p.predelayMs = 0; p.modulation = 0; p.autoMakeup = false;
    p.lowDecay = 0.5f; p.highDecay = 0.5f; p.damping = 0.0f; p.warmth = 0.0f; p.lowCutHz = 20; p.highCutHz = 20000; p.tone = 0.5f;
    e.setParams (p);
    const int n = (int) (sr * (decay * 2.5 + 2));
    std::vector<float> L ((size_t) n, 0), R ((size_t) n, 0); L[0] = R[0] = 1;
    run (e, L, R, 512);
    std::vector<double> edc ((size_t) n); double acc = 0;
    for (int i = n - 1; i >= 0; --i) { acc += (double) L[(size_t) i] * L[(size_t) i] + (double) R[(size_t) i] * R[(size_t) i]; edc[(size_t) i] = acc; }
    auto db = [&] (int i) { return 10 * std::log10 (edc[(size_t) i] / edc[0] + 1e-30); };
    int i5 = 0, i25 = 0; for (int i = 0; i < n; ++i) { if (!i5 && db (i) < -5) i5 = i; if (!i25 && db (i) < -25) { i25 = i; break; } }
    return 3.0 * (i25 - i5) / sr;   // T20 extrapolated to 60 dB
}

int main()
{
    std::mt19937 rng (1); std::uniform_real_distribution<float> uni (-1, 1);
    std::printf("Silence\n");
    { ReverbEngine e; e.prepare (48000); e.setParams (Params{}); std::vector<float> L (48000, 0), R (48000, 0); run (e, L, R, 256);
      float pk; bool ok = finite (L, 1e-6f, pk) && finite (R, 1e-6f, pk); CHECK (ok, "silence in -> silence out"); }

    std::printf("Impulse / decay time (RT60 measured, T20 extrapolated)\n");
    for (int algo : { 0, 3, 5 })
        for (float d : { 1.0f, 2.8f, 6.0f })
        {
            const double m = measureRT60 (48000, d, algo);
            std::printf("    algo %d  target %.1fs  measured %.2fs\n", algo, d, m);
            char b[96]; std::snprintf (b, sizeof b, "algo %d RT60 within 35%% of %.1fs", algo, d);
            CHECK (std::fabs (m - d) < 0.35 * d, b);
        }

    std::printf("Stability: extreme settings, loud noise, 20 s\n");
    for (int algo = 0; algo < 6; ++algo)
    {
        ReverbEngine e; e.prepare (48000); Params p; p.algo = algo; p.decay = 20; p.roomSize = 1; p.density = 1; p.diffusion = 1; p.modulation = 1;
        p.lowDecay = 1; p.highDecay = 1; p.damping = 0; p.warmth = 0; p.wet = 1; p.retDb = 12; p.width = 1; p.erWidth = 1; p.early = 1; p.highCutHz = 20000; p.lowCutHz = 20; p.autoMakeup = false;
        e.setParams (p); std::vector<float> L (48000 * 20), R (48000 * 20);
        for (size_t i = 0; i < L.size(); ++i) { const bool on = (i / 48000) < 2; L[i] = on ? uni (rng) * 0.9f : 0; R[i] = on ? uni (rng) * 0.9f : 0; }
        run (e, L, R, 480); float pk, pk2; bool ok = finite (L, 50.0f, pk) && finite (R, 50.0f, pk2);
        // tail after input stops must decay, not grow
        double early = 0, late = 0; for (int i = 96000 + 48000; i < 96000 + 96000; ++i) early += L[(size_t) i] * L[(size_t) i];
        for (int i = 48000 * 18; i < 48000 * 20; ++i) late += L[(size_t) i] * L[(size_t) i];
        char b[96]; std::snprintf (b, sizeof b, "algo %d max settings: finite, peak %.2f, tail not growing", algo, std::max (pk, pk2));
        CHECK (ok && late < early, b);
    }

    std::printf("Sample rates & block sizes\n");
    for (double sr : { 44100.0, 48000.0, 88200.0, 96000.0, 192000.0 })
        for (int blk : { 1, 64, 128, 256, 512, 1024, 1000 })
        {
            ReverbEngine e; e.prepare (sr); Params p; p.wet = 1; p.decay = 4; p.algo = 3; e.setParams (p);
            std::vector<float> L ((size_t) sr), R ((size_t) sr); for (size_t i = 0; i < L.size(); ++i) { L[i] = uni (rng) * 0.5f; R[i] = uni (rng) * 0.5f; }
            run (e, L, R, blk); float pk; bool ok = finite (L, 10, pk) && finite (R, 10, pk);
            if (!ok) { std::printf ("  FAIL sr %.0f block %d\n", sr, blk); ++fails; }
        }
    std::printf("  ok:   5 sample rates x 7 block sizes finite and bounded\n");

    std::printf("Predelay\n");
    { ReverbEngine e; e.prepare (48000); Params p; p.dry = 0; p.wet = 1; p.predelayMs = 100; p.early = 1; e.setParams (p);
      std::vector<float> L (48000, 0), R (48000, 0); L[0] = R[0] = 1; run (e, L, R, 256);
      int first = -1; for (int i = 0; i < 48000; ++i) if (std::fabs (L[(size_t) i]) > 1e-4f) { first = i; break; }
      const double ms = first * 1000.0 / 48000; std::printf ("    first wet sample at %.2f ms (set 100 ms)\n", ms);
      CHECK (ms >= 98.0 && ms <= 125.0, "wet signal starts after predelay (no early leak)"); }

    std::printf("Ducking\n");
    { auto wetLevel = [&] (bool duck) {
          ReverbEngine e; e.prepare (48000); Params p; p.dry = 0; p.wet = 1; p.decay = 3; p.duckOn = duck; p.duckThrDb = -30; p.duckDepthDb = 18; p.duckAttMs = 5; p.duckRelMs = 200; e.setParams (p);
          std::vector<float> L (48000 * 3), R (48000 * 3);
          for (size_t i = 0; i < L.size(); ++i) { const bool on = (i < 48000); L[i] = on ? uni (rng) * 0.3f : 0; R[i] = L[i]; }
          run (e, L, R, 256); double a = 0, b = 0;
          for (int i = 24000; i < 48000; ++i) a += L[(size_t) i] * L[(size_t) i];          // while "singing"
          for (int i = 48000 + 24000; i < 48000 + 48000; ++i) b += L[(size_t) i] * L[(size_t) i]; // after pause
          return std::pair<double, double> (a, b); };
      auto off = wetLevel (false), on = wetLevel (true);
      const double redDb = 10 * std::log10 (on.first / off.first);
      std::printf ("    wet energy while voice active: %.1f dB with ducking; after pause tail kept: %.1f dB\n", redDb, 10 * std::log10 (on.second / off.second));
      CHECK (redDb < -10.0, "ducking lowers wet while input is active");
      CHECK (10 * std::log10 (on.second / off.second) > -3.0, "ducking releases: tail returns during pauses"); }

    std::printf("Tempo sync\n");
    CHECK (std::fabs (syncedMs (120, 2, false, false) - 125.0) < 1e-6, "120 BPM 1/16 = 125 ms");
    CHECK (std::fabs (syncedMs (120, 2, true, false) - 83.3333) < 1e-3, "120 BPM 1/16T = 83.33 ms");
    CHECK (std::fabs (syncedMs (120, 3, false, true) - 187.5) < 1e-6, "120 BPM dotted 1/8 = 187.5 ms");
    CHECK (std::fabs (syncedMs (120, 4, false, false) - 250.0) < 1e-6, "120 BPM 1/4 = 500 ms folds to 250 ms");
    CHECK (syncedMs (0, 2, false, false) == 125.0, "invalid BPM falls back safely");

    std::printf("\n%s (%d failures)\n", fails ? "FAILED" : "ALL DSP TESTS PASSED", fails);
    return fails ? 1 : 0;
}
