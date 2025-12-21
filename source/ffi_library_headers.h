#pragma once

#include "String.h"
#include "Map.h"
#include <sys/stat.h>

// Maps library names to their potential header file locations
// This allows us to find signatures via header parsing instead of hardcoding them

inline Map<String, List<String>*>& get_library_header_map() {
    static Map<String, List<String>*> library_headers;
    static bool initialized = false;

    if (!initialized) {
        initialized = true;

        // Standard C library - multiple headers for different functions
        auto c_headers = new List<String>();
        c_headers->add("/usr/include/string.h");      // strlen, strcmp, etc.
        c_headers->add("/usr/include/stdlib.h");      // atoi, atol, malloc, etc.
        c_headers->add("/usr/include/stdio.h");       // printf, fopen, etc.
        c_headers->add("/usr/local/include/string.h");
        c_headers->add("/usr/local/include/stdlib.h");
        library_headers.add("c", c_headers);

        // Math library
        auto m_headers = new List<String>();
        m_headers->add("/usr/include/math.h");
        m_headers->add("/usr/local/include/math.h");
        m_headers->add("/opt/homebrew/include/math.h");
        m_headers->add("/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/include/math.h");
        library_headers.add("m", m_headers);

        // SDL2
        auto sdl_headers = new List<String>();
        sdl_headers->add("/opt/homebrew/include/SDL2/SDL.h");
        sdl_headers->add("/opt/homebrew/include/SDL2/SDL_events.h");
        sdl_headers->add("/opt/homebrew/include/SDL2/SDL_render.h");
        sdl_headers->add("/opt/homebrew/include/SDL2/SDL_timer.h");
        sdl_headers->add("/usr/local/include/SDL2/SDL.h");
        sdl_headers->add("/usr/include/SDL2/SDL.h");
        library_headers.add("SDL2", sdl_headers);

        // raylib
        auto raylib_headers = new List<String>();
        raylib_headers->add("/opt/homebrew/include/raylib.h");
        raylib_headers->add("/usr/local/include/raylib.h");
        raylib_headers->add("/usr/include/raylib.h");
        library_headers.add("raylib", raylib_headers);
    }

    return library_headers;
}

// Get all potential c header files for a library
inline List<String>* get_library_header_files(const String& library_name) {
    auto& map = get_library_header_map();
    if (map.has(library_name)) return map[library_name];
    return 0;
}

// Find first existing header file for a library
inline String find_existing_header(const String& library_name) {
    List<String>* headers = get_library_header_files(library_name);
    if (!headers) return String();

    for (int i = 0; i < headers->size(); i++) {
        String header = (*headers)[i];
        struct stat buffer;
        if (stat(header.data, &buffer) == 0) {
            return header;  // Found existing header
        }
    }

    return String();  // No header found
}
