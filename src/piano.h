#pragma once
#include <array>
#include <stdint.h>
#include "midiFile.h"
#include <raylib.h>
class pianoKey {
    public:
        bool down;
        uint32_t cooldown;
        float yTarget = 0;
        float currY = 0;
};

void drawPianoKeyboard(std::array<pianoKey, 88>& keyState, Vector3 offset);
int getWhiteKeyIndex(uint32_t noteIndex);


class PianoObj 
{
public:
    uint8_t pianoProgram;
    std::array<pianoKey, 88> keyState;
    void renderSelf(Vector3 offset);
    void updateState(midiEvent& event, midiFile& file);
};