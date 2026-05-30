#include "midiFile.h"
#include <fstream>
#include <byteswap.h>
#include <algorithm>
#include <bit>
#include <exception>
#include <array>
#include <cstdint>
#include <iostream>

void createMidiEvent(std::vector<midiEvent>& currTrack, uint32_t timeDelta, uint32_t absoluteTick,
                     midiFile::midiEventName eventType = midiFile::midiEventName::noteOff, 
                     bool isMidiEvent = true, uint8_t channel = 0, 
                     uint8_t noteIndex = 0, 
                     uint8_t velocity = 0, 
                     uint8_t program = 0, 
                     uint8_t pressure = 0, 
                     uint8_t controller = 0, 
                     uint8_t controllerValue = 0,
                     uint8_t metaEventType = 0,
                     uint8_t timeSignatureNumerator = 0,
                     uint8_t timeSignatureDenominator = 0,
                     uint8_t clocksPerTick = 0,
                     uint8_t _32per24Clocks = 0,
                     uint32_t Tempo = 0,
                     uint16_t pitchValue = 0
                    )
{
    currTrack.push_back({0});
    currTrack.back().deltaTime = timeDelta;
    currTrack.back().eventType = eventType;
    currTrack.back().isMidiEvent = isMidiEvent;
    currTrack.back().channel = channel;


    currTrack.back().noteIndex = noteIndex;
    currTrack.back().velocity = velocity;
    currTrack.back().program = program;
    currTrack.back().pressure = pressure;
    currTrack.back().controller = controller;
    currTrack.back().controllerValue = controllerValue;
    currTrack.back().metaEventType = metaEventType;


    currTrack.back().timeSignatureNumerator = timeSignatureNumerator;
    currTrack.back().timeSignatureDenominator = timeSignatureDenominator;
    currTrack.back().clocksPerTick = clocksPerTick;
    currTrack.back()._32per24Clocks = _32per24Clocks;
    currTrack.back().Tempo = Tempo;

    currTrack.back().absoluteTick = absoluteTick;
    currTrack.back().pitchValue = pitchValue;
}

uint32_t midiFile::readVariableAmount(std::ifstream &inputMidi)
{
    uint32_t returnVal = 0;
    uint8_t currentByte = 0;

    do {
        inputMidi.read((char*)&currentByte, 1);
        returnVal = (returnVal<<7) | (currentByte&0x7F);
    } while (currentByte&0x80 && !inputMidi.eof());
    return returnVal;
}

std::string midiFile::readString(std::ifstream &inputMidi, uint32_t length)
{
    std::string result;
    for (int i=0;i<length;i++)
    {
        if (inputMidi.eof())
        {
            return result;
        }
        result+=inputMidi.get();
    }
    return result;
}


midiFile::midiFile(std::string filePath)
{
    std::ifstream inputMidi(filePath, std::ios::binary);

    if (!inputMidi.is_open())
    {
        throw std::runtime_error("Failed to open midi file");
    }
    timeSignatureNumerator = 4;
    timeSignatureDenominator = 4;
    clocksPerTick = 24;
    _32per24Clocks = 8;
    Tempo = 500000;
    division = 0;

    uint32_t midiMagicNumber = 0;
    uint32_t headerLength = 0;
    uint16_t format = 0;
    uint16_t trackCount = 0;

    inputMidi.read((char *)&midiMagicNumber, 4);
    inputMidi.read((char *)&headerLength, 4);
    inputMidi.read((char *)&format, 2);
    inputMidi.read((char *)&trackCount, 2);
    inputMidi.read((char *)&division, 2);


    headerLength = std::byteswap(headerLength);
    format = std::byteswap(format);
    trackCount = std::byteswap(trackCount);
    division = std::byteswap(division);

    if ((division & 0x8000)) {
        throw std::runtime_error("Invalid timing proto"); // invalid timing proto
    }

    if (midiMagicNumber == 0x6468544d)
    {
        std::cout<<"Midi header detected\n";
    } else {
        throw std::invalid_argument("Provided file is not MIDI");
    }
    std::cout<<"Format: "<<format<<" Track Count: "<<trackCount<<" Division: "<<division<<"\n";

    

    for (int currTrack=0;currTrack<trackCount;currTrack++)
    {
        tracksList.emplace_back();
        std::string trackMagicNumber;
        uint32_t trackLength = 0;

        trackMagicNumber=readString(inputMidi, 4);
        std::cout<<"------"<<trackMagicNumber<<"------"<<"\n";

        inputMidi.read((char *)&trackLength, 4);
        trackLength=std::byteswap(trackLength);
        bool endTrack = false;
        uint8_t previousStatus = 0;
        uint32_t absoluteTick = 0;

        while(!endTrack && !inputMidi.eof())
        {
            uint32_t timeDelta = readVariableAmount(inputMidi);
            uint8_t status = inputMidi.get();
            absoluteTick+=timeDelta;
            //std::cout<<std::hex<<(int)status<<std::dec<<"\n";
            //running status
            if (status<0x80)
            {
                status = previousStatus;
                inputMidi.seekg(-1, std::ios_base::cur);
                if (previousStatus<0x80)
                {
                    std::cout<<"INVALID PREVIOUS STATUS\n";
                }
            } else if (status < 0xF0) {
                previousStatus = status;
            } else {
                previousStatus = 0;
                uint8_t systemMessage = status&0x0F;
                if (systemMessage == 0x0 || systemMessage == 0x7) // sys exclusive;
                {
                    uint32_t length = readVariableAmount(inputMidi);
                    for (int sysI=0;sysI<length;sysI++)
                    {
                        inputMidi.get();
                    }
                    createMidiEvent(tracksList[currTrack], timeDelta, absoluteTick, midiEventName::systemExclusive, false);

                } else if (systemMessage == 0xF) { //Meta Event
                    uint8_t metaType = inputMidi.get();
                    uint32_t length = readVariableAmount(inputMidi);
                    uint32_t Tempo = 0;
                    uint8_t channel = 0;
                    switch (metaType)
                    {
                        case MetaEventName::EndOfTrack:
                            endTrack = true;
                            createMidiEvent(tracksList[currTrack], timeDelta, absoluteTick, midiEventName::systemExclusive, false, 0, 0, 0, 0, 0, 0, 0, metaType);
                            break;
                        case MetaEventName::TimeSignature:
                            timeSignatureNumerator = inputMidi.get();
                            timeSignatureDenominator = (1 << inputMidi.get());
                            clocksPerTick = inputMidi.get();
                            _32per24Clocks = inputMidi.get();
                            createMidiEvent(tracksList[currTrack], timeDelta, absoluteTick, midiEventName::systemExclusive, false, 0, 0, 0, 0, 0, 0, 0, metaType, timeSignatureNumerator, timeSignatureDenominator, clocksPerTick, _32per24Clocks);
                            break;
                        case MetaEventName::SetTempo:
                            Tempo = 0;
                            (Tempo |= (inputMidi.get() << 16));
                            (Tempo |= (inputMidi.get() << 8));
                            (Tempo |= (inputMidi.get() << 0));
                            createMidiEvent(tracksList[currTrack], timeDelta, absoluteTick, midiEventName::systemExclusive, false, 0, 0, 0, 0, 0, 0, 0, metaType, 0, 0, 0, 0, Tempo);
                            break;
                        case MetaEventName::Sequence:
                            createMidiEvent(tracksList[currTrack], timeDelta, absoluteTick, midiEventName::systemExclusive, false, 0, 0, 0, 0, 0, 0, 0, metaType);
                            break;
                        case MetaEventName::Copyright:
                            std::cout<<"Text: "<<readString(inputMidi, length)<<"\n";
                            createMidiEvent(tracksList[currTrack], timeDelta, absoluteTick, midiEventName::systemExclusive, false, 0, 0, 0, 0, 0, 0, 0, metaType);
                            break;
                        case MetaEventName::TrackName:
                            std::cout<<"Text: "<<readString(inputMidi, length)<<"\n";
                            createMidiEvent(tracksList[currTrack], timeDelta, absoluteTick, midiEventName::systemExclusive, false, 0, 0, 0, 0, 0, 0, 0, metaType);
                            break;
                        case MetaEventName::InstrumentName:
                            std::cout<<"Text: "<<readString(inputMidi, length)<<"\n";
                            createMidiEvent(tracksList[currTrack], timeDelta, absoluteTick, midiEventName::systemExclusive, false, 0, 0, 0, 0, 0, 0, 0, metaType);
                            break;
                        case MetaEventName::Lyrics:
                            std::cout<<"Text: "<<readString(inputMidi, length)<<"\n";
                            createMidiEvent(tracksList[currTrack], timeDelta, absoluteTick, midiEventName::systemExclusive, false, 0, 0, 0, 0, 0, 0, 0, metaType);
                            break;
                        case MetaEventName::Marker:
                            std::cout<<"Text: "<<readString(inputMidi, length)<<"\n";
                            createMidiEvent(tracksList[currTrack], timeDelta, absoluteTick, midiEventName::systemExclusive, false, 0, 0, 0, 0, 0, 0, 0, metaType);
                            break;
                        case MetaEventName::CuePoint:
                            std::cout<<"Text: "<<readString(inputMidi, length)<<"\n";
                            createMidiEvent(tracksList[currTrack], timeDelta, absoluteTick, midiEventName::systemExclusive, false, 0, 0, 0, 0, 0, 0, 0, metaType);
                            break;
                        case MetaEventName::ChannelPrefix:
                            channel = inputMidi.get();
                            createMidiEvent(tracksList[currTrack], timeDelta, absoluteTick, midiEventName::systemExclusive, false, channel, 0, 0, 0, 0, 0, 0, metaType);
                            break;
                        case MetaEventName::KeySignature:
                            inputMidi.get();
                            inputMidi.get();
                            createMidiEvent(tracksList[currTrack], timeDelta, absoluteTick, midiEventName::systemExclusive, false, 0, 0, 0, 0, 0, 0, 0, metaType);
                            break;
                        default:
                            for (int i=0;i<length;i++)
                            {
                                inputMidi.get();
                            }
                            createMidiEvent(tracksList[currTrack], timeDelta, absoluteTick, midiEventName::systemExclusive, false, 0, 0, 0, 0, 0, 0, 0, metaType);
                            std::cout<<"UNHANDLED META 0x"<<std::hex<<(int)metaType<<std::dec<<" "<<length<<"\n";
                    }
                    //createMidiEvent(tracksList[currTrack], timeDelta, midiEventName::systemExclusive, false, 0, 0, 0, 0, 0, 0, 0, metaType);
                } else {
                    std::cout<<"INVALID SYSTEM MESSAGE\n";
                }
                continue;
            }
             
            
            uint8_t eventType = status&0xF0;
            uint8_t channel = status&0x0F;

            uint8_t noteIndex = 0;
            uint8_t noteVelocity = 0;
            uint8_t notePressure = 0;
            uint8_t controller = 0;
            uint8_t controllerValue = 0;
            uint8_t programID = 0;
            uint8_t pressure = 0;
            uint8_t lsb = 0;
            uint8_t msb = 0;
            uint16_t pitchValue = 0;
                    
            channelUtilization[channel].isUsed = true;
            switch (eventType)
            {
                case midiEventName::noteOff: 
                    noteIndex = inputMidi.get();
                    noteVelocity = inputMidi.get();
                    //midiTracks[trackI].midiEvents.push_back({midiEventName::noteOff, noteIndex, noteVelocity, timeDelta, channel, tickSum, Tempo, trackI});
                    //if (noteIndex>127) {std::cout<<"high "<<eventCount<<" "<<(int)noteIndex<<"track: "<<trackI<<"\n";}
                    createMidiEvent(tracksList[currTrack], timeDelta, absoluteTick, midiEventName::noteOff, true, channel, noteIndex, noteVelocity);
                    break;
                case midiEventName::noteOn:
                    noteIndex = inputMidi.get();
                    noteVelocity = inputMidi.get();
                    if (noteVelocity > 0)
                    {
                    //midiTracks[trackI].midiEvents.push_back({midiEventName::noteOn, noteIndex, noteVelocity, timeDelta, channel, tickSum, Tempo, trackI});
                        createMidiEvent(tracksList[currTrack], timeDelta, absoluteTick, midiEventName::noteOn, true, channel, noteIndex, noteVelocity);
                    } else {
                    //midiTracks[trackI].midiEvents.push_back({midiEventName::noteOff, noteIndex, noteVelocity, timeDelta, channel, tickSum, Tempo, trackI});
                        createMidiEvent(tracksList[currTrack], timeDelta, absoluteTick, midiEventName::noteOff, true, channel, noteIndex, noteVelocity);
                    }
                    if (noteIndex>127) {std::cout<<"high "<<(int)noteIndex<<"track: "<<currTrack<<"\n";}
                    break;
                case midiEventName::afterTouch:
                    noteIndex = inputMidi.get();
                    notePressure = inputMidi.get();
                    //midiTracks[trackI].midiEvents.emplace_back(midiEventName::afterTouch, noteIndex, noteVelocity, timeDelta, channel, tickSum, Tempo, trackI);
                    createMidiEvent(tracksList[currTrack], timeDelta, absoluteTick, midiEventName::afterTouch, true, channel, noteIndex, 0, 0, notePressure);
                    break;
                case midiEventName::controlChange:
                    controller = inputMidi.get();
                    controllerValue = inputMidi.get();
                    //midiTracks[trackI].midiEvents.emplace_back(midiEventName::controlChange, noteIndex, noteVelocity, timeDelta, channel, tickSum, Tempo, trackI);
                    createMidiEvent(tracksList[currTrack], timeDelta, absoluteTick, midiEventName::controlChange, true, channel, 0, 0, 0, 0, controller, controllerValue);
                    break;
                case midiEventName::programChange:
                    programID = inputMidi.get();
                    //midiTracks[trackI].midiEvents.emplace_back(midiEventName::programChange, programID, 0, timeDelta, channel, tickSum, Tempo, trackI);
                    createMidiEvent(tracksList[currTrack], timeDelta, absoluteTick, midiEventName::programChange, true, channel, 0, 0, programID);
                    break;
                case midiEventName::channelPressure:
                    pressure = inputMidi.get();
                    createMidiEvent(tracksList[currTrack], timeDelta, absoluteTick, midiEventName::channelPressure, true, channel, 0, 0, 0, pressure);
                    break;
                case midiEventName::pitchBend:
                    lsb = inputMidi.get();
                    msb = inputMidi.get();

                    pitchValue = (msb << 7) | lsb;
                    createMidiEvent(tracksList[currTrack], timeDelta, absoluteTick, midiEventName::pitchBend, true, channel, 0, 0, 0, 0, 0, 0, 0,0 , 0, 0, 0, 0, pitchValue);
                    break;
                default:
                    std::cout<<"INVALID STATUS" << std::hex<< (int)status <<std::dec << "\n";
            }
        }
    }

    for (int i=0;i<tracksList.size();i++)
    {
        for (int j=0;j<tracksList[i].size();j++)
        {
            unifiedEvents.push_back(tracksList[i][j]);
        }
    }

    std::sort(unifiedEvents.begin(), unifiedEvents.end(), [](const midiEvent& a, const midiEvent& b) {
    if (a.absoluteTick != b.absoluteTick)
        return a.absoluteTick < b.absoluteTick;


    uint32_t priorityA = 0;
    if (!a.isMidiEvent) {priorityA=0;}
    else if (a.eventType==midiEventName::noteOff) {priorityA=1;}
    else if (a.eventType==midiEventName::controlChange) {priorityA=2;}
    else if (a.eventType==midiEventName::programChange) {priorityA=2;}
    else if (a.eventType==midiEventName::channelPressure) {priorityA=2;}
    else if (a.eventType==midiEventName::noteOn) {priorityA=3;}
    else {priorityA=4;}

    uint32_t priorityB = 0;
    if (!b.isMidiEvent) {priorityB=0;}
    else if (b.eventType==midiEventName::noteOff) {priorityB=1;}
    else if (b.eventType==midiEventName::controlChange) {priorityB=2;}
    else if (b.eventType==midiEventName::programChange) {priorityB=2;}
    else if (b.eventType==midiEventName::channelPressure) {priorityB=2;}
    else if (b.eventType==midiEventName::noteOn) {priorityB=3;}
    else {priorityB=4;}

    if (priorityA != priorityB) return priorityA < priorityB;

    return a.channel < b.channel;
    });

    double currentTime = 0;
    Tempo = 500000;
    uint32_t previousTick = 0;
    for (int i=0;i<unifiedEvents.size();i++)
    {
        uint32_t deltaTick = unifiedEvents[i].absoluteTick - previousTick;
        double deltaSecond = (deltaTick * Tempo) / (division * 1000000.0);

        currentTime+=deltaSecond;
        unifiedEvents[i].absoluteTime = currentTime;

        if (unifiedEvents[i].eventType == midiEventName::systemExclusive &&
            unifiedEvents[i].metaEventType == midiFile::MetaEventName::SetTempo) {
            Tempo = unifiedEvents[i].Tempo;
        }

        previousTick = unifiedEvents[i].absoluteTick;
        if (unifiedEvents[i].isMidiEvent && unifiedEvents[i].eventType == midiFile::midiEventName::programChange)
        {
            if (unifiedEvents[i].channel==9)
            {
                std::cout<<"Percussion Channel PC Change "<<(int)unifiedEvents[i].program<<"\n";
                continue;
            }
            if (std::find(channelUtilization[unifiedEvents[i].channel].usedProgram.begin(), channelUtilization[unifiedEvents[i].channel].usedProgram.end(), unifiedEvents[i].program) == channelUtilization[unifiedEvents[i].channel].usedProgram.end()) {
                std::cout<<"channel: "<<(int)unifiedEvents[i].channel<<" "<<(int)unifiedEvents[i].program<<"\n";
                channelUtilization[unifiedEvents[i].channel].usedProgram.push_back(unifiedEvents[i].program);
            }
            if (std::find(programUsed.begin(), programUsed.end(), unifiedEvents[i].program) == programUsed.end()) {
                programUsed.push_back(unifiedEvents[i].program);
            }
        }
    }

    inputMidi.close();    
    if (programUsed.size()==0)
    {
        programUsed.push_back(0);
    }

    for (int i=0;i<programUsed.size();i++)
    {
        std::cout<<getInstrumentName(programUsed[i])<<"\n";
    }

    for (int i=0;i<16;i++)
    {
        channelUtilization[i].currentProgram = 0;
        if (channelUtilization[i].usedProgram.size()==0 && i!=9)
        {
            channelUtilization[i].usedProgram.push_back(0);
        }
        if (channelUtilization[i].isUsed)
        {
            std::cout<<"channel "<<i<<": "<<(channelUtilization[i].isUsed ? "true" : "false") <<" "<<channelUtilization[i].usedProgram.size()<<"\n";
        }
    }
}

midiChannelMeta::midiChannelMeta()
{
    isUsed = false;
}


std::string midiFile::getInstrumentName(int programID) {
    programID++;
    switch (programID) {
        case 1: return "Acoustic Grand Piano";
        case 2: return "Bright Acoustic Piano";
        case 3: return "Electric Grand Piano";
        case 4: return "Honky-tonk Piano";
        case 5: return "Electric Piano 1";
        case 6: return "Electric Piano 2";
        case 7: return "Harpsichord";
        case 8: return "Clavinet";

        case 9: return "Celesta";
        case 10: return "Glockenspiel";
        case 11: return "Music Box";
        case 12: return "Vibraphone";
        case 13: return "Marimba";
        case 14: return "Xylophone";
        case 15: return "Tubular Bells";
        case 16: return "Dulcimer";

        case 17: return "Drawbar Organ";
        case 18: return "Percussive Organ";
        case 19: return "Rock Organ";
        case 20: return "Church Organ";
        case 21: return "Reed Organ";
        case 22: return "Accordion";
        case 23: return "Harmonica";
        case 24: return "Bandoneon";

        case 25: return "Acoustic Guitar (nylon)";
        case 26: return "Acoustic Guitar (steel)";
        case 27: return "Electric Guitar (jazz)";
        case 28: return "Electric Guitar (clean)";
        case 29: return "Electric Guitar (muted)";
        case 30: return "Electric Guitar (overdrive)";
        case 31: return "Electric Guitar (distortion)";
        case 32: return "Electric Guitar (harmonics)";

        case 33: return "Acoustic Bass";
        case 34: return "Electric Bass (finger)";
        case 35: return "Electric Bass (picked)";
        case 36: return "Fretless Bass";
        case 37: return "Slap Bass 1";
        case 38: return "Slap Bass 2";
        case 39: return "Synth Bass 1";
        case 40: return "Synth Bass 2";

        case 41: return "Violin";
        case 42: return "Viola";
        case 43: return "Cello";
        case 44: return "Contrabass";
        case 45: return "Tremolo Strings";
        case 46: return "Pizzicato Strings";
        case 47: return "Orchestral Harp";
        case 48: return "Timpani";

        case 49: return "String Ensemble 1";
        case 50: return "String Ensemble 2";
        case 51: return "Synth Strings 1";
        case 52: return "Synth Strings 2";
        case 53: return "Choir Aahs";
        case 54: return "Voice Oohs";
        case 55: return "Synth Voice";
        case 56: return "Orchestra Hit";

        case 57: return "Trumpet";
        case 58: return "Trombone";
        case 59: return "Tuba";
        case 60: return "Muted Trumpet";
        case 61: return "French Horn";
        case 62: return "Brass Section";
        case 63: return "Synth Brass 1";
        case 64: return "Synth Brass 2";

        case 65: return "Soprano Sax";
        case 66: return "Alto Sax";
        case 67: return "Tenor Sax";
        case 68: return "Baritone Sax";
        case 69: return "Oboe";
        case 70: return "English Horn";
        case 71: return "Bassoon";
        case 72: return "Clarinet";

        case 73: return "Piccolo";
        case 74: return "Flute";
        case 75: return "Recorder";
        case 76: return "Pan Flute";
        case 77: return "Blown Bottle";
        case 78: return "Shakuhachi";
        case 79: return "Whistle";
        case 80: return "Ocarina";

        case 81: return "Lead 1 (square)";
        case 82: return "Lead 2 (sawtooth)";
        case 83: return "Lead 3 (calliope)";
        case 84: return "Lead 4 (chiff)";
        case 85: return "Lead 5 (charang)";
        case 86: return "Lead 6 (voice)";
        case 87: return "Lead 7 (fifths)";
        case 88: return "Lead 8 (bass + lead)";

        case 89: return "Pad 1 (new age)";
        case 90: return "Pad 2 (warm)";
        case 91: return "Pad 3 (polysynth)";
        case 92: return "Pad 4 (choir)";
        case 93: return "Pad 5 (bowed)";
        case 94: return "Pad 6 (metallic)";
        case 95: return "Pad 7 (halo)";
        case 96: return "Pad 8 (sweep)";

        case 97: return "FX 1 (rain)";
        case 98: return "FX 2 (soundtrack)";
        case 99: return "FX 3 (crystal)";
        case 100: return "FX 4 (atmosphere)";
        case 101: return "FX 5 (brightness)";
        case 102: return "FX 6 (goblins)";
        case 103: return "FX 7 (echoes)";
        case 104: return "FX 8 (sci-fi)";

        case 105: return "Sitar";
        case 106: return "Banjo";
        case 107: return "Shamisen";
        case 108: return "Koto";
        case 109: return "Kalimba";
        case 110: return "Bag Pipe";
        case 111: return "Fiddle";
        case 112: return "Shanai";

        case 113: return "Tinkle Bell";
        case 114: return "Agogo";
        case 115: return "Steel Drums";
        case 116: return "Woodblock";
        case 117: return "Taiko Drum";
        case 118: return "Melodic Tom";
        case 119: return "Synth Drum";
        case 120: return "Reverse Cymbal";

        case 121: return "Guitar Fret Noise";
        case 122: return "Breath Noise";
        case 123: return "Seashore";
        case 124: return "Bird Tweet";
        case 125: return "Telephone Ring";
        case 126: return "Helicopter";
        case 127: return "Applause";
        case 128: return "Gunshot";

        default: return "Unknown Instrument";
    }
}
