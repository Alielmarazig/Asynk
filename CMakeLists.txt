cmake_minimum_required(VERSION 3.20)
project(asynk VERSION 0.3.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# ── Qt6 ──
find_package(Qt6 REQUIRED COMPONENTS Widgets Core Gui Xml)

# ── Sources ──
set(SOURCES
    src/main.cpp
    src/core/sync_engine.cpp
    src/core/media_handler.cpp
    src/core/fft.cpp
    src/exporters/fcpxml_exporter.cpp
    src/exporters/premiere_exporter.cpp
    src/exporters/edl_exporter.cpp
    src/exporters/export_manager.cpp
    src/ui/theme.cpp
    src/ui/main_window.cpp
    src/ui/waveform_widget.cpp
    src/ui/stat_cards.cpp
)

set(HEADERS
    src/core/sync_engine.h
    src/core/media_handler.h
    src/core/fft.h
    src/exporters/fcpxml_exporter.h
    src/exporters/premiere_exporter.h
    src/exporters/edl_exporter.h
    src/exporters/export_manager.h
    src/ui/theme.h
    src/ui/main_window.h
    src/ui/waveform_widget.h
    src/ui/stat_cards.h
)

# ── Executable ──
if(WIN32)
    add_executable(asynk WIN32 ${SOURCES} ${HEADERS})
else()
    add_executable(asynk ${SOURCES} ${HEADERS})
endif()

target_include_directories(asynk PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# MSVC needs _USE_MATH_DEFINES for M_PI
if(MSVC)
    target_compile_definitions(asynk PRIVATE _USE_MATH_DEFINES)
endif()

target_link_libraries(asynk PRIVATE
    Qt6::Widgets
    Qt6::Core
    Qt6::Gui
    Qt6::Xml
)

# Platform-specific math library
if(UNIX)
    target_link_libraries(asynk PRIVATE m)
endif()

# ── Install ──
install(TARGETS asynk DESTINATION bin)
