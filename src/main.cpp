#include <iostream>
#include "midiFile.h"
#include <raylib.h>
#include <fluidsynth.h>
#include "fluidsynthwrapper.h"
#include "MusicInstruments.h"
#include "midiPlayer.h"
#include "piano.h"
#include <chrono>
#include <thread>

int main(int, char**){
    const int screenWidth = 800;
    const int screenHeight = 450;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, "MIDI Visualizer 2");
    SetTargetFPS(120);

    Camera3D camera = { 0 };
    camera.position = (Vector3){ 10.0f, 10.0f, 10.0f }; // Camera position
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };      // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 70.0f;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;             // Camera projection type
    
    bool mouseCaptured = false;
    midiFile file("assets/lullaby.mid");
    MidiPlayer player(file);
    FluidSynthObj fluid;
    uint32_t currentPointer = 0;

    MusicInstruments instruments;

    player.play();


    auto start = std::chrono::high_resolution_clock::now();
    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_Z)) {DisableCursor(); mouseCaptured=true;}
        if (IsKeyPressed(KEY_X)) {EnableCursor(); mouseCaptured=false;}
        std::this_thread::sleep_for(std::chrono::microseconds(200));

        player.updatePlayback(fluid, instruments);
        
        BeginDrawing();
        ClearBackground(GRAY);


        if (mouseCaptured) UpdateCamera(&camera, CAMERA_FREE);

        BeginMode3D(camera);
        instruments.renderInstruments();

        DrawGrid(10, 1.0f);
        EndMode3D();


        EndDrawing();
    }

    CloseWindow(); 

}

