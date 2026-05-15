/**
 * media_handler.cpp
 *
 * Uses ffprobe/ffmpeg via QProcess for media analysis and audio extraction.
 * All audio is extracted as raw PCM float32 mono at 8kHz for sync correlation.
 */

#include "media_handler.h"
#include <QProcess>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTemporaryFile>
#include <QDir>
#include <QStandardPaths>
#include <QFile>
#include <QDebug>
#include <cstring>

namespace asynk {

bool ffmpegAvailable() {
    QProcess proc;
    proc.start("ffmpeg", {"-version"});
    proc.waitForFinished(3000);
    return proc.exitCode() == 0;
}

std::optional<MediaInfo> probeFile(const QString& path) {
    QProcess proc;
    proc.start("ffprobe", {
        "-v", "quiet",
        "-print_format", "json",
        "-show_format",
        "-show_streams",
        path
    });
    proc.waitForFinished(10000);

    if (proc.exitCode() != 0) {
        qWarning() << "ffprobe failed for" << path << proc.readAllStandardError();
        return std::nullopt;
    }

    QJsonDocument doc = QJsonDocument::fromJson(proc.readAllStandardOutput());
    if (doc.isNull()) return std::nullopt;

    QJsonObject root = doc.object();
    QJsonObject fmt  = root["format"].toObject();
    QJsonArray  streams = root["streams"].toArray();

    MediaInfo info;
    info.filePath = path;
    info.fileName = QFileInfo(path).fileName();
    info.duration = fmt["duration"].toString().toDouble();
    info.format   = fmt["format_name"].toString();
    info.fileSize = QFileInfo(path).size();

    for (const auto& s : streams) {
        QJsonObject st = s.toObject();
        QString type = st["codec_type"].toString();

        if (type == "video" && !info.hasVideo) {
            info.hasVideo = true;
            info.width    = st["width"].toInt();
            info.height   = st["height"].toInt();
            info.codec    = st["codec_name"].toString();

            // Parse fps from r_frame_rate "30/1" or "30000/1001"
            QString rfr = st["r_frame_rate"].toString();
            if (rfr.contains('/')) {
                auto parts = rfr.split('/');
                double num = parts[0].toDouble();
                double den = parts[1].toDouble();
                if (den > 0) info.fps = num / den;
            }
        }
        else if (type == "audio" && !info.hasAudio) {
            info.hasAudio    = true;
            info.sampleRate  = st["sample_rate"].toString().toInt();
            info.channels    = st["channels"].toInt();
            if (info.codec.isEmpty())
                info.codec = st["codec_name"].toString();
        }
    }

    return info;
}

bool isSupported(const QString& path) {
    QString ext = QFileInfo(path).suffix().toLower();
    for (const auto& e : SUPPORTED_EXTENSIONS) {
        if (e.mid(1) == ext) return true;  // Compare without leading dot
    }
    return false;
}

QString extractAudioPCM(const QString& inputPath, int sampleRate) {
    // Create temp file for raw PCM output
    QString tmpDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    QString tmpPath = tmpDir + "/asynk_" +
                      QFileInfo(inputPath).baseName() + "_" +
                      QString::number(sampleRate) + ".pcm";

    QProcess proc;
    proc.start("ffmpeg", {
        "-y",                          // Overwrite
        "-i", inputPath,               // Input
        "-vn",                         // No video
        "-ac", "1",                    // Mono
        "-ar", QString::number(sampleRate),  // Resample
        "-f", "f32le",                 // Raw float32 little-endian
        "-acodec", "pcm_f32le",
        tmpPath
    });
    proc.waitForFinished(60000);  // 60s timeout

    if (proc.exitCode() != 0) {
        qWarning() << "Audio extraction failed:" << proc.readAllStandardError();
        return {};
    }

    return tmpPath;
}

std::vector<float> loadPCM(const QString& pcmPath) {
    QFile file(pcmPath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Cannot open PCM file:" << pcmPath;
        return {};
    }

    QByteArray data = file.readAll();
    file.close();

    size_t count = data.size() / sizeof(float);
    std::vector<float> samples(count);
    std::memcpy(samples.data(), data.constData(), count * sizeof(float));

    return samples;
}

QStringList scanDirectory(const QString& dirPath, bool recursive) {
    QStringList result;
    QDir dir(dirPath);

    auto flags = recursive
        ? QDir::Files | QDir::AllDirs | QDir::NoDotAndDotDot
        : QDir::Files;

    QStringList entries = dir.entryList(flags, QDir::Name);

    for (const auto& entry : entries) {
        QString fullPath = dir.absoluteFilePath(entry);
        QFileInfo fi(fullPath);

        if (fi.isDir() && recursive) {
            result.append(scanDirectory(fullPath, true));
        } else if (fi.isFile() && isSupported(fullPath)) {
            result.append(fullPath);
        }
    }

    return result;
}

} // namespace asynk
