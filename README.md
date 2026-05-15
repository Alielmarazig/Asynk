# asynk

Multi-track audio/video synchronization tool. Import clips from multiple cameras and audio recorders, sync them by audio waveform cross-correlation, and export aligned timelines for your NLE.

**Native C++/Qt6 application** — no runtime dependencies, fast startup, small binary.

## How it works

asynk extracts audio from every imported clip via FFmpeg, downsamples to 8kHz mono, then runs FFT-based cross-correlation against a reference clip. The correlation peak gives the time offset; peak height gives a confidence score.

## Requirements

- **Qt6** (6.5+) with Widgets, Core, Gui, Xml modules
- **CMake** 3.20+
- **FFmpeg** on PATH (for audio extraction)
- C++20 compiler (MSVC 2022, GCC 12+, Clang 14+)

## Build

```bash
# Clone
git clone https://github.com/Alielmarazig/asynk.git
cd asynk

# Configure & build
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release

# Run
./build/asynk          # Linux/macOS
build\Release\asynk.exe   # Windows
```

### Windows with Visual Studio

```powershell
cmake -B build -G "Visual Studio 17 2022"
cmake --build build --config Release
```

### Deploy on Windows

```powershell
mkdir deploy
copy build\Release\asynk.exe deploy\
windeployqt deploy\asynk.exe
```

## Usage

1. **Import clips** — drag and drop files onto the window, or use File > Import Clips / Import Folder
2. **Set reference** — select the master clip and use Edit > Set as Reference (Ctrl+R)
3. **Sync** — hit the Sync button. Progress shows in the status bar. Each clip gets an offset and confidence score
4. **Export** — choose format (FCP X, Premiere, EDL, or All) and fps, then export

## Timeline Export Formats

| Format | Extension | Compatible With |
|--------|-----------|----------------|
| FCPXML v1.9 | `.fcpxml` | Final Cut Pro X 10.2.3+ |
| Premiere XML | `.xml` | Premiere Pro CS6 – CC 2024 |
| CMX 3600 EDL | `.edl` | Vegas Pro, EDIUS, DaVinci Resolve |

## Project Structure

```
src/
├── main.cpp                 # Entry point
├── core/
│   ├── fft.h                # Radix-2 FFT (header-only)
│   ├── media_handler.h/cpp  # FFmpeg probe & audio extraction
│   └── sync_engine.h/cpp    # Cross-correlation sync engine
├── exporters/
│   ├── fcpxml_exporter.h/cpp
│   ├── premiere_exporter.h/cpp
│   ├── edl_exporter.h/cpp
│   └── export_manager.h/cpp
└── ui/
    ├── theme.h/cpp           # Dark design system
    ├── stat_cards.h/cpp      # Metric cards
    ├── waveform_widget.h/cpp # Multi-track waveform panel
    └── main_window.h/cpp     # Main application window
```

## GitHub Actions

Push to `main` triggers CI builds for Windows, Linux, and macOS. Push a version tag (`v0.3.0`) to create a release with downloadable binaries.

## License

MIT
