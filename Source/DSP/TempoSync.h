#pragma once
#include <cmath>
namespace orymy {
// Divisions: 0=1/64 1=1/32 2=1/16 3=1/8 4=1/4 5=1/2 6=1/1
inline double divisionInBeats (int div)
{
    static const double beats[] = { 4.0/64, 4.0/32, 4.0/16, 4.0/8, 4.0/4, 4.0/2, 4.0 };
    if (div < 0) div = 0;
    if (div > 6) div = 6;
    return beats[div];
}
// Predelay in ms from BPM. If the result exceeds maxMs it is folded down by octaves (halved)
// so the predelay always stays musical and inside the allowed range.
inline double syncedMs (double bpm, int div, bool triplet, bool dotted, double maxMs = 250.0)
{
    if (! (bpm > 20.0 && bpm < 400.0)) bpm = 120.0;
    double ms = 60000.0 / bpm * divisionInBeats (div);
    if (triplet) ms *= 2.0 / 3.0;
    else if (dotted) ms *= 1.5;
    while (ms > maxMs) ms *= 0.5;
    return ms;
}
}
