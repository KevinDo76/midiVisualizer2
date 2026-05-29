#pragma once
#include "piano.h"
#include "midiFile.h"
#include <array>
#include <vector>
class PianoObj;
class midiFile;
class midiEvent;

class MusicInstruments
{
public:
    std::vector<PianoObj> pianos;

    MusicInstruments();

    void updateInstruments(midiEvent& event, midiFile& file);
    void renderInstruments();
};