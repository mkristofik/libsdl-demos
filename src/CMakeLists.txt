cmake_minimum_required(VERSION 2.4)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11 -Werror -D_GNU_SOURCE=1 -Dmain=SDL_main")

set(EXENAME hello)
#file(GLOB SRC *.cpp)

include_directories("c:/MyLibs/SDL-1.2.15/include/SDL"
    "c:/MyLibs/SDL_image-1.2.12/include"
    "c:/MyLibs/SDL_ttf-2.0.11/include")

# Must appear before the add_executable line.
link_directories("c:/MyLibs/SDL-1.2.15/lib"
    "c:/MyLibs/SDL_image-1.2.12/lib/x86"
    "c:/MyLibs/SDL_ttf-2.0.11/lib/x86")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mwindows")

add_executable(${EXENAME} test.cpp)

# Must appear after add_executable line.
target_link_libraries(${EXENAME} mingw32 SDLmain SDL SDL_image SDL_ttf)

set(EXE2 random)
add_executable(${EXE2} random.cpp)
target_link_libraries(${EXE2} mingw32 SDLmain SDL SDL_image)
