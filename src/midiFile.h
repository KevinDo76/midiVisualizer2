#pragma once
#include <string>
#include <stdint.h>
#include <vector>
#include <array>
#include <chrono>
#include "fluidsynthwrapper.h"
//#include "MusicInstruments.h"

class MusicInstruments;

class midiEvent;

class midiChannelMeta
{
public:
    midiChannelMeta();
    bool isUsed;
    uint8_t currentProgram;
    std::vector<uint32_t> usedProgram;
};

class midiFile
{
public:
    midiFile(std::string filePath);
    uint16_t division;
    uint32_t timeSignatureNumerator;
    uint32_t timeSignatureDenominator;
    uint32_t clocksPerTick;
    uint32_t _32per24Clocks;
    uint32_t Tempo;

    static uint32_t readVariableAmount(std::ifstream &inputMidi);
    static std::string readString(std::ifstream &inputMidi, uint32_t length);
    static std::string getInstrumentName(int programID);

    std::vector<std::vector<midiEvent>> tracksList;
    std::vector<midiEvent> unifiedEvents;
    std::array<midiChannelMeta, 16> channelUtilization;
    std::vector<uint8_t> programUsed;

    uint32_t messagePointer;
    std::chrono::_V2::system_clock::time_point startTime;
    bool playing;
    void play();

    void updatePlayback(FluidSynthObj& fluid, MusicInstruments& instruments);

    enum midiEventName : uint8_t
    {
        noteOff = 0x80,
        noteOn = 0x90,
        afterTouch = 0xA0,
        controlChange = 0xB0,
        programChange = 0xC0,
        channelPressure = 0xD0,
        pitchBend = 0xE0,
        systemExclusive = 0xF0,
    };

    enum MetaEventName : uint8_t
	{
		Sequence = 0x00,
		Text = 0x01,
		Copyright = 0x02,
		TrackName = 0x03,
		InstrumentName = 0x04,
		Lyrics = 0x05,
		Marker = 0x06,
		CuePoint = 0x07,
		ChannelPrefix = 0x20,
		EndOfTrack = 0x2F,
		SetTempo = 0x51,
		SMPTEOffset = 0x54,
		TimeSignature = 0x58,
		KeySignature = 0x59,
		SequencerSpecific = 0x7F,
	};
};

class midiEvent {
public:
    uint32_t deltaTime;
    uint32_t absoluteTick;
    double absoluteTime;
    uint8_t eventType;
    uint8_t channel;

    uint8_t noteIndex;
    uint8_t velocity;
    uint8_t program;
    uint8_t pressure;
    uint8_t controller;
    uint8_t controllerValue;
    uint8_t metaEventType;

    uint32_t timeSignatureNumerator;
    uint32_t timeSignatureDenominator;
    uint32_t clocksPerTick;
    uint32_t _32per24Clocks;
    uint32_t Tempo;

    uint16_t pitchValue;


    bool isMidiEvent;
};

