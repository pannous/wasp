#pragma once

#include "String.h"
#include "Map.h"
#include <sys/stat.h>

// Maps library names to their potential header file locations
// This allows us to find signatures via header parsing instead of hardcoding them

inline Map<String, List<String> *> &get_library_header_map() {
    // Add headers from standard locations not covered by generic pattern
    // String generic_path = "/opt/homebrew/include/"s + library_name + "/*" ;
    static Map<String, List<String> *> library_headers;
    static bool initialized = false;
    if (!initialized) {
        initialized = true;
        // Standard C library - multiple headers for different functions
        auto c_headers = new List<String>();
        c_headers->add("/usr/include/string.h"); // strlen, strcmp, etc.
        c_headers->add("/usr/include/stdlib.h"); // atoi, atol, malloc, etc.
        c_headers->add("/usr/include/stdio.h"); // printf, fopen, etc.
        c_headers->add("/usr/local/include/string.h");
        c_headers->add("/usr/local/include/stdlib.h");
        c_headers->add("/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/include/_string.h");
        c_headers->add("/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/include/_stdlib.h");
        library_headers.add("c", c_headers);

        // Math library
        auto m_headers = new List<String>();
        m_headers->add("/usr/include/math.h");
        m_headers->add("/usr/local/include/math.h");
        m_headers->add("/opt/homebrew/include/math.h");
        m_headers->add("/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/include/math.h");
        library_headers.add("m", m_headers);

        // SDL2
        auto sdl2_headers = new List<String>();
        sdl2_headers->add("/opt/homebrew/Cellar/sdl2/current/include/SDL2/SDL.h");
        // Homebrew SDL2 path includes all these:
        sdl2_headers->add("/opt/homebrew/include/SDL2/SDL.h");
        sdl2_headers->add("/opt/homebrew/include/SDL2/SDL_events.h");
        sdl2_headers->add("/opt/homebrew/include/SDL2/SDL_render.h");
        sdl2_headers->add("/opt/homebrew/include/SDL2/SDL_timer.h");
        sdl2_headers->add("/usr/local/include/SDL2/SDL.h");
        sdl2_headers->add("/usr/include/SDL2/SDL.h");
        library_headers.add("SDL2", sdl2_headers);
        // library_headers.add("SDL", sdl2_headers);

        auto sdl3_headers = new List<String>();
        sdl3_headers->add("/opt/homebrew/Cellar/sdl3/current/include/SDL3/SDL.h");
        // Homebrew SDL3 path includes all these:
        for (String file: ls("/opt/homebrew/Cellar/sdl3/current/include/SDL3/"))
            sdl3_headers->add("/opt/homebrew/Cellar/sdl3/current/include/SDL3/"s + file);

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
inline List<String> *get_library_header_files(const String &library_name) {
    auto &map = get_library_header_map();
    List<String> generic_include_paths = {
        "/opt/homebrew/include/"s,
        "/usr/local/include/"s,
        "/usr/include/"s,
        "/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/include/"s
    };
    if (!map.has(library_name))map.add(library_name, new List<String>());
    auto paths = map[library_name];
    for (String path: generic_include_paths) {
        String generic_path = path + library_name;
        if(isDir(generic_path))
        for (String file: ls(generic_path)) {
            auto filename = generic_path + "/"s + file;
            if (not paths->has(filename) and fileExists(filename)) {
                paths->add(filename);
            print("Added generic header file for library %s: %s\n"s % library_name % filename);
            }
        }
    }
    return paths;
}

// Find first existing header file for a library
inline String find_existing_header(const String &library_name) {
    List<String> *headers = get_library_header_files(library_name);
    if (!headers) return String();

    for (int i = 0; i < headers->size(); i++) {
        String header = (*headers)[i];
        struct stat buffer;
        if (stat(header.data, &buffer) == 0) {
            return header; // Found existing header
        }
    }

    return String(); // No header found
}
