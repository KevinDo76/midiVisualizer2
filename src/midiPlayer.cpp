#include "midiPlayer.h"
#include <iostream>
MidiPlayer::MidiPlayer(midiFile& _midiF)
    : midiF(_midiF)
{
    playing = false;
    messagePointer = 0;
}


void MidiPlayer::play()
{
    messagePointer = 0;
    startTime = std::chrono::high_resolution_clock::now();
    playing = true;
}

void MidiPlayer::updatePlayback(FluidSynthObj& fluid, MusicInstruments& instruments)
{
    if (!playing)
    {
        return;
    }


    double timeElapsed = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - startTime).count();
    while (messagePointer<midiF.unifiedEvents.size() && timeElapsed>=midiF.unifiedEvents[messagePointer].absoluteTime && true)
    {
        std::cout<<midiF.unifiedEvents[messagePointer].deltaTime<<" tick\n";
        if (midiF.unifiedEvents[messagePointer].isMidiEvent && midiF.unifiedEvents[messagePointer].eventType == midiFile::midiEventName::noteOn)
        {
            //if (channelUtilization[unifiedEvents[messagePointer].channel].currentProgram>=8)
            //{
            //    messagePointer+=1;
            //    continue;
            //}
            //std::cout<<(int)channelUtilization[unifiedEvents[messagePointer].channel].currentProgram<<"\n";
            //std::cout<<(int)unifiedEvents[messagePointer].noteIndex<<" "<<timeElapsed<<"s\n";
            fluid_synth_noteon(fluid.synth, midiF.unifiedEvents[messagePointer].channel, midiF.unifiedEvents[messagePointer].noteIndex, midiF.unifiedEvents[messagePointer].velocity);
        } else if (midiF.unifiedEvents[messagePointer].isMidiEvent && midiF.unifiedEvents[messagePointer].eventType == midiFile::midiEventName::noteOff)
        {
            //if (channelUtilization[unifiedEvents[messagePointer].channel].currentProgram>=8)
            //{
            //    messagePointer+=1;
            //    continue;
            //}
            fluid_synth_noteoff(fluid.synth, midiF.unifiedEvents[messagePointer].channel, midiF.unifiedEvents[messagePointer].noteIndex);
        } else if (midiF.unifiedEvents[messagePointer].isMidiEvent && midiF.unifiedEvents[messagePointer].eventType == midiFile::midiEventName::channelPressure)
        {
            //std::cout<<"pressure\n";
            fluid_synth_channel_pressure(fluid.synth, midiF.unifiedEvents[messagePointer].channel, midiF.unifiedEvents[messagePointer].pressure);
        } else if (midiF.unifiedEvents[messagePointer].isMidiEvent && midiF.unifiedEvents[messagePointer].eventType == midiFile::midiEventName::programChange)
        {
            //std::cout<<"program\n";
            fluid_synth_program_change(fluid.synth, midiF.unifiedEvents[messagePointer].channel, midiF.unifiedEvents[messagePointer].program);
            midiF.channelUtilization[midiF.unifiedEvents[messagePointer].channel].currentProgram = midiF.unifiedEvents[messagePointer].program;
        } else if (midiF.unifiedEvents[messagePointer].isMidiEvent && midiF.unifiedEvents[messagePointer].eventType == midiFile::midiEventName::pitchBend) {
            //std::cout<<"pitch\n";
            fluid_synth_pitch_bend(fluid.synth, midiF.unifiedEvents[messagePointer].channel, midiF.unifiedEvents[messagePointer].pitchValue);
        }  else if (midiF.unifiedEvents[messagePointer].isMidiEvent && midiF.unifiedEvents[messagePointer].eventType == midiFile::midiEventName::controlChange)
        {
            //std::cout<<"controller\n";
            fluid_synth_cc(fluid.synth, midiF.unifiedEvents[messagePointer].channel, midiF.unifiedEvents[messagePointer].controller, midiF.unifiedEvents[messagePointer].controllerValue);
        }
        instruments.updateInstruments(midiF.unifiedEvents[messagePointer], midiF);
        messagePointer+=1;
    }
    if (messagePointer==midiF.unifiedEvents.size())
    {
        std::cout<<"end\n";
        messagePointer+=1;
        playing = false;
    }
}
