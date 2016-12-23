# Audio Spectrum Visualization using Vulkan
Experimental project.
- Compile using CMake.
- Execute using `./avis <path-to-audio-file>`, use `<space>` to pause, `q` or `<esc>` to quit.

Interesting features:
- FFT: Unit-roots, shuffle-indices for the input, and the window-coefficients are computed at compile-time.

## Dependencies
| Name                     | Link                                                           |
|--------------------------|----------------------------------------------------------------|
| GLFW                     | http://www.glfw.org/                                           |
| glslang/glslangValidator | https://www.khronos.org/opengles/sdk/tools/Reference-Compiler/ |
| ffmpeg                   | https://ffmpeg.org/                                            |
| PortAudio                | http://www.portaudio.com/                                      |
| boost                    | http://www.boost.org/                                          |
