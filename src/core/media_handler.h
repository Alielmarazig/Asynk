#pragma once
#include <QString>
#include <QStringList>
#include <QFileInfo>
#include <vector>
#include <optional>

namespace asynk {

/// Supported media file extensions
inline const QStringList SUPPORTED_EXTENSIONS = {
    // Video
    ".mp4", ".mov", ".avi", ".mkv", ".mxf", ".wmv", ".flv", ".webm",
    ".m4v", ".mpg", ".mpeg", ".ts", ".m2ts", ".3gp",
    // Pro camera
    ".r3d", ".braw", ".ari", ".arx", ".crm", ".nef",
    // Audio
    ".wav", ".mp3", ".aac", ".flac", ".ogg", ".m4a", ".aif", ".aiff", ".wma",
};

/// Metadata for a media file
struct MediaInfo {
    QString filePath;
    QString fileName;
    QString codec;
    QString format;
    double  duration    = 0.0;   // seconds
    int     sampleRate  = 0;
    int     channels    = 0;
    int     width       = 0;
    int     height      = 0;
    double  fps         = 0.0;
    bool    hasVideo    = false;
    bool    hasAudio    = false;
    qint64  fileSize    = 0;
};

/// Check if FFmpeg is available on PATH
bool ffmpegAvailable();

/// Probe a single file with ffprobe, return metadata
std::optional<MediaInfo> probeFile(const QString& path);

/// Check if file extension is supported
bool isSupported(const QString& path);

/// Extract audio from a media file as raw PCM float32 mono at given sample rate.
/// Writes to a temp file and returns its path, or empty on failure.
QString extractAudioPCM(const QString& inputPath, int sampleRate = 8000);

/// Load raw PCM float32 mono from file into a vector
std::vector<float> loadPCM(const QString& pcmPath);

/// Scan a directory for supported media files
QStringList scanDirectory(const QString& dirPath, bool recursive = true);

} // namespace asynk
