cmake_minimum_required(VERSION 3.10.0)
project(midiVisualizer2 VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
find_package(raylib REQUIRED)
find_package(FluidSynth REQUIRED)

file(GLOB_RECURSE srcfiles CONFIGURE_DEPENDS "src/*.cpp" "src/*.h")

add_executable(midiVisualizer2 ${srcfiles})
target_link_libraries(midiVisualizer2 PRIVATE FluidSynth::libfluidsynth raylib)