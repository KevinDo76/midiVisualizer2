#include <raylib.h>
#include <array>
#include <stdint.h>
#include "piano.h"
#include <array>
#include <iostream>

void drawPianoBlackKey(Vector3 position, Color main, Color bound)
{
    float sizeConstant = 1.0f;
    float offsetsize = (((2.35f-1.4f)*sizeConstant)/2.0f)+((1.4f*sizeConstant)/2.0f);
    Vector3 dimension = {1.4f*sizeConstant, 2.0f*sizeConstant, 10.0f*sizeConstant};
    DrawCube({position.x*2.35f*sizeConstant+offsetsize, position.y*2.0f*sizeConstant+0.7f*sizeConstant, position.z*16.5f*sizeConstant-(6.5f*sizeConstant/2.0f)}, dimension.x, dimension.y, dimension.z, main);
    DrawCubeWires({position.x*2.35f*sizeConstant+offsetsize, position.y*2.0f*sizeConstant+0.7f*sizeConstant, position.z*16.5f*sizeConstant-(6.5f*sizeConstant/2.0f)}, dimension.x, dimension.y, dimension.z, bound);
}

void drawPianoWhiteKey(Vector3 position, Color main, Color bound)
{
    float sizeConstant = 1.0f;
    Vector3 dimension = {2.35f*sizeConstant, 2.0f*sizeConstant, 16.5f*sizeConstant};
    DrawCube({position.x*2.35f*sizeConstant, position.y*2.0f*sizeConstant, position.z*16.5f*sizeConstant}, dimension.x, dimension.y, dimension.z, main);
    DrawCubeWires({position.x*2.35f*sizeConstant, position.y*2.0f*sizeConstant, position.z*16.5f*sizeConstant}, dimension.x, dimension.y, dimension.z, bound);
}

int getBlackKeyIndex(uint32_t noteIndex)
{
    static const int blackMap[12] = {
        -1, 0,  -1, 1,  -1,
        -1, 2,  -1, 3,  -1,
        4,  -1
    };

    int n = noteIndex - 21; // shift so A0 = 0
    if (n < 0) return -1;

    int noteInOctave = noteIndex % 12;
    int blackOffset = blackMap[noteInOctave];

    if (blackOffset == -1)
        return -1; // not a black key

    int octaveOffset = (noteIndex / 12) * 5;

    // base = black index of A#0 (MIDI 22)
    int base = (21 / 12) * 5 + blackMap[21 % 12];

    return octaveOffset + blackOffset - base;
}

int getWhiteKeyIndex(uint32_t noteIndex)
{
    static const int whiteMap[12] = {
        0,  -1, 1,  -1, 2,
        3,  -1, 4,  -1, 5,
        -1, 6
    };

    int noteInOctave = noteIndex % 12;
    int whiteOffset = whiteMap[noteInOctave];

    if (whiteOffset == -1)
        return -1; 

    int octave = noteIndex / 12;


    int baseOffset = - (21 / 12) * 7 - whiteMap[21 % 12];

    return octave * 7 + whiteOffset + baseOffset;
}

bool isBlackKey(int key)
{
    int note = (key + 9) % 12;

    switch (note)
    {
        case 1:  // C#
        case 3:  // D#
        case 6:  // F#
        case 8:  // G#
        case 10: // A#
            return true;

        default:
            return false;
    }
}

void drawPianoKeyboard(std::array<pianoKey, 88>& keyState, Vector3 offsetInp)
{
    int whiteIndex = 0;
    int blackOffset = 1;
    for (int i=0;i<88;i++)
    {
        keyState[i].currY+=(keyState[i].yTarget-keyState[i].currY)*0.1;
        float offset = keyState[i].currY;
        bool isBlack = isBlackKey(i);
        if (isBlack)
        {
            Color mainKey = BLACK;
            if (keyState[i].down && keyState[i].cooldown==0)
            {
                mainKey = BLUE;
                offset = -0.2f;
            } else if (keyState[i].cooldown>0) {
                keyState[i].cooldown--;
            }
            drawPianoBlackKey({(float)i-blackOffset + offsetInp.x, offset + offsetInp.y, offsetInp.z}, mainKey, WHITE);
            blackOffset++;
        } else {
            Color mainKey = WHITE;
            if (keyState[i].down && keyState[i].cooldown==0)
            {
                mainKey = BLUE;
                offset = -0.2f;
            } else if (keyState[i].cooldown>0) {
                keyState[i].cooldown--;
            }
            drawPianoWhiteKey({(float)whiteIndex + offsetInp.x, offset + offsetInp.y, offsetInp.z}, mainKey, BLACK);
            whiteIndex++;
        }
    }
}

void PianoObj::renderSelf(Vector3 offset)
{
    drawPianoKeyboard(keyState, offset);
}

void PianoObj::updateState(midiEvent &event, midiFile& file)
{
    if (file.channelUtilization[event.channel].currentProgram!=pianoProgram || event.channel == 9)
    {
        //std::cout<<"rejected event "<<(int)file.channelUtilization[event.channel].currentProgram<<" "<<(int)pianoProgram<<"\n";
        return;
    }
    if (!event.isMidiEvent || event.noteIndex<21 || event.noteIndex>108)
    {
        return;
    }
    
    if (event.eventType == midiFile::midiEventName::noteOn)
    {
        keyState[event.noteIndex-21].down = true;
        keyState[event.noteIndex-21].yTarget = -0.25f;
    } 
    if (event.eventType == midiFile::midiEventName::noteOff)
    {
        keyState[event.noteIndex-21].down = false;
        keyState[event.noteIndex-21].yTarget = 0.0f;
        keyState[event.noteIndex-21].cooldown = 5;
    }
}
