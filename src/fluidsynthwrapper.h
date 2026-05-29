#pragma once
#include <string>
#include <fluidsynth.h>
class FluidSynthObj
{
    
public:
    FluidSynthObj();

    fluid_player_t* player;
    fluid_settings_t* settings;
    fluid_audio_driver_t* driver;
    fluid_synth_t* synth;
};