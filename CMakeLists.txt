cmake_minimum_required(VERSION 3.28)
project(untitled1 C)

set(CMAKE_C_STANDARD 11)

include(FetchContent)
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
FetchContent_Declare(
        SDL2
        URL https://github.com/libsdl-org/SDL/archive/refs/tags/release-2.32.10.tar.gz
)
FetchContent_MakeAvailable(SDL2)

add_executable(untilted1 main.c)
target_link_libraries(untilted1 SDL2::SDL2main SDL2::SDL2-static)
target_compile_options(untilted1 PRIVATE -Wall -Wextra)