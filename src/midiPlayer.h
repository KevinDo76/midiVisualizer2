#pragma once
#include <chrono>
#include "fluidsynthwrapper.h"
#include "MusicInstruments.h"
#include "midiFile.h"

class MidiPlayer {
public:
    MidiPlayer(midiFile& midiF);
    midiFile& midiF;
    uint32_t messagePointer;
    std::chrono::_V2::system_clock::time_point startTime;
    bool playing;
    void play();

    void updatePlayback(FluidSynthObj& fluid, MusicInstruments& instruments);
};