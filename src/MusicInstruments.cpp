#include "MusicInstruments.h"
#include "midiFile.h"

MusicInstruments::MusicInstruments()
{
    for (int i=0;i<8;i++)
    {
        pianos.push_back({0});
        pianos[i].pianoProgram = i;
    }
}

void MusicInstruments::updateInstruments(midiEvent &event, midiFile& file)
{
    for (int i=0;i<8;i++)
    {
        pianos[i].updateState(event, file);
    }
}

void MusicInstruments::renderInstruments()
{
    for (int i=0;i<8;i++)
    {
        pianos[i].renderSelf({0,0,(float)i});
    }
}
