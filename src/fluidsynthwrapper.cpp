#include "fluidsynth.h"
#include "fluidsynthwrapper.h"
#include <iostream>
#define SF2_FILE_PATH "/usr/share/soundfonts/FluidR3_GM.sf2"

void quiet_log_handler(int level, const char* message, void* data) {}

FluidSynthObj::FluidSynthObj()
{
    fluid_set_log_function(FLUID_PANIC, quiet_log_handler, nullptr);
    //fluid_set_log_function(FLUID_ERR, quiet_log_handler, nullptr);
    //fluid_set_log_function(FLUID_WARN, quiet_log_handler, nullptr);
    //fluid_set_log_function(FLUID_INFO, quiet_log_handler, nullptr);
    fluid_set_log_function(FLUID_DBG, quiet_log_handler, nullptr);
    settings = new_fluid_settings();
    fluid_settings_setstr(settings, "audio.driver", "pulseaudio");
    fluid_settings_setnum(settings, "synth.sample-rate", 48000.0); 
    fluid_settings_setnum(settings, "synth.gain", 2);

    synth = new_fluid_synth(settings);
    
    std::string soundFont = SF2_FILE_PATH;
    if (fluid_synth_sfload(synth, soundFont.c_str(), 1) == FLUID_FAILED) {
        std::cerr << "Failed to load SoundFont: " << soundFont << "\n";
        throw std::invalid_argument("sf2 file not found");
    }
    driver = new_fluid_audio_driver(settings, synth);
    player = new_fluid_player(synth);
    //if (fluid_player_add(player, path.c_str()) != FLUID_OK) {
    //    std::cerr << "Failed to load MIDI file\n";
    //    throw std::invalid_argument("midi file not found");
    //}
}
