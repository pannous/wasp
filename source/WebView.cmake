# This hasn't been tested on all OSs

include(FetchContent)
FetchContent_Declare(
        webview
        GIT_REPOSITORY https://github.com/webview/webview.git
)
# <<<< check out regularly for update!
# - The C++ interface in this repository is still changing often
# still does not work for some platforms
# Currently supported Ubuntu, macOS, Windows
# Support for Android or iOS is still unclear, but might be possible and is highly desirable.

FetchContent_GetProperties(webview)
if (NOT webview_POPULATED)
    # Library does not have a CMake build script
    # We have to do it ourselves
    FetchContent_Populate(webview)
    add_library(webview INTERFACE)
    target_sources(webview INTERFACE ${webview_SOURCE_DIR}/webview.h)
    target_include_directories(webview INTERFACE ${webview_SOURCE_DIR})

    # Set compile options
    # See: https://github.com/webview/webview/blob/master/script/build.sh
    if (WIN32)
        target_compile_definitions(webview INTERFACE WEBVIEW_EDGE)
        # See: https://github.com/webview/webview/blob/master/script/build.bat
        target_link_libraries(webview INTERFACE "-mwindows -L./dll/x64 -lwebview -lWebView2Loader")
        # target_compile_options(...) ?
    elseif (APPLE)
        target_compile_definitions(webview INTERFACE WEBVIEW_COCOA)
        target_compile_definitions(webview INTERFACE "GUI_SOURCE_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}\"")
        target_compile_options(webview INTERFACE -Wno-all -Wno-extra -Wno-pedantic -Wno-delete-non-abstract-non-virtual-dtor)
        target_link_libraries(webview INTERFACE "-framework WebKit")
    elseif (UNIX)
        target_compile_definitions(webview INTERFACE WEBVIEW_GTK)
        target_compile_options(webview INTERFACE -Wall -Wextra -Wpedantic)
        target_link_libraries(webview INTERFACE "$(pkg-config --cflags --libs gtk+-3.0 webkit2gtk-4.0)")
    endif ()
endif ()

# Create example
#add_executable(webview_hello webview_hello.cpp)
#target_link_libraries(webview_hello PUBLIC webview)