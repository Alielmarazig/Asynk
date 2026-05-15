/**
 * sync_engine.cpp
 *
 * FFT-based cross-correlation sync engine.
 * Extracts audio from each clip via FFmpeg, downsamples to 8kHz mono,
 * cross-correlates against the reference, finds peak offset.
 */

#include "sync_engine.h"
#include "fft.h"
#include <QFile>
#include <QDebug>
#include <cmath>
#include <algorithm>
#include <numeric>

namespace asynk {

SyncEngine::SyncEngine(QObject* parent) : QObject(parent) {}

void SyncEngine::setReference(const QString& path) {
    m_referencePath = path;
}

void SyncEngine::setClips(const QStringList& paths) {
    m_clipPaths = paths;
}

void SyncEngine::setSampleRate(int rate) {
    m_sampleRate = rate;
}

void SyncEngine::runSync() {
    m_session = SyncSession{};
    m_session.referencePath = m_referencePath;

    // Extract reference audio
    emit progressChanged(0, m_clipPaths.size(), "Extracting reference audio...");

    QString refPCM = extractAudioPCM(m_referencePath, m_sampleRate);
    if (refPCM.isEmpty()) {
        emit syncError("Failed to extract audio from reference clip.");
        return;
    }

    std::vector<float> refAudio = loadPCM(refPCM);
    QFile::remove(refPCM);

    if (refAudio.empty()) {
        emit syncError("Reference clip has no audio data.");
        return;
    }

    // Normalize reference
    float refMax = *std::max_element(refAudio.begin(), refAudio.end(),
        [](float a, float b) { return std::abs(a) < std::abs(b); });
    if (refMax > 0.0f) {
        for (auto& s : refAudio) s /= refMax;
    }

    // Process each clip
    int total = m_clipPaths.size();
    for (int i = 0; i < total; ++i) {
        const QString& clipPath = m_clipPaths[i];
        QString clipName = QFileInfo(clipPath).fileName();

        emit progressChanged(i, total, clipName);

        SyncResult result;
        if (clipPath == m_referencePath) {
            // Reference syncs to itself at offset 0
            result.clipPath = clipPath;
            result.offsetSamples = 0;
            result.offsetSeconds = 0.0;
            result.confidence = 1.0;
            result.sampleRate = m_sampleRate;
            result.success = true;
        } else {
            result = syncOneClip(clipPath, refAudio);
        }

        m_session.results.push_back(result);
        emit clipSynced(i, result);
    }

    // Calculate total duration
    for (const auto& r : m_session.results) {
        auto info = probeFile(r.clipPath);
        if (info) {
            double endTime = r.offsetSeconds + info->duration;
            if (endTime > m_session.totalDuration)
                m_session.totalDuration = endTime;
        }
    }

    emit progressChanged(total, total, "Done");
    emit syncFinished(m_session);
}

SyncResult SyncEngine::syncOneClip(
    const QString& clipPath,
    const std::vector<float>& refAudio)
{
    SyncResult result;
    result.clipPath = clipPath;
    result.sampleRate = m_sampleRate;

    // Extract audio
    QString pcmPath = extractAudioPCM(clipPath, m_sampleRate);
    if (pcmPath.isEmpty()) {
        result.error = "Failed to extract audio";
        return result;
    }

    std::vector<float> clipAudio = loadPCM(pcmPath);
    QFile::remove(pcmPath);

    if (clipAudio.empty()) {
        result.error = "No audio data in clip";
        return result;
    }

    // Normalize
    float clipMax = *std::max_element(clipAudio.begin(), clipAudio.end(),
        [](float a, float b) { return std::abs(a) < std::abs(b); });
    if (clipMax > 0.0f) {
        for (auto& s : clipAudio) s /= clipMax;
    }

    // Limit to 5 minutes for performance
    size_t maxSamples = static_cast<size_t>(300.0 * m_sampleRate);
    if (refAudio.size() > maxSamples) {
        // We work with truncated copy
        std::vector<float> refTrunc(refAudio.begin(),
                                    refAudio.begin() + maxSamples);
        if (clipAudio.size() > maxSamples)
            clipAudio.resize(maxSamples);

        auto corr = crossCorrelate(refTrunc, clipAudio);

        // Find peak
        auto peakIt = std::max_element(corr.begin(), corr.end());
        size_t peakIdx = std::distance(corr.begin(), peakIt);
        double peakVal = *peakIt;

        // Offset: peak index relative to center of correlation
        int offset = static_cast<int>(peakIdx) -
                     static_cast<int>(refTrunc.size()) + 1;

        // Confidence: peak normalized by energy
        double refEnergy = std::inner_product(
            refTrunc.begin(), refTrunc.end(), refTrunc.begin(), 0.0);
        double clipEnergy = std::inner_product(
            clipAudio.begin(), clipAudio.end(), clipAudio.begin(), 0.0);
        double normFactor = std::sqrt(refEnergy * clipEnergy);
        double confidence = (normFactor > 0.0) ? peakVal / normFactor : 0.0;

        result.offsetSamples = offset;
        result.offsetSeconds = static_cast<double>(offset) / m_sampleRate;
        result.confidence = std::clamp(confidence, 0.0, 1.0);
        result.success = true;
    } else {
        if (clipAudio.size() > maxSamples)
            clipAudio.resize(maxSamples);

        auto corr = crossCorrelate(refAudio, clipAudio);

        auto peakIt = std::max_element(corr.begin(), corr.end());
        size_t peakIdx = std::distance(corr.begin(), peakIt);
        double peakVal = *peakIt;

        int offset = static_cast<int>(peakIdx) -
                     static_cast<int>(refAudio.size()) + 1;

        double refEnergy = std::inner_product(
            refAudio.begin(), refAudio.end(), refAudio.begin(), 0.0);
        double clipEnergy = std::inner_product(
            clipAudio.begin(), clipAudio.end(), clipAudio.begin(), 0.0);
        double normFactor = std::sqrt(refEnergy * clipEnergy);
        double confidence = (normFactor > 0.0) ? peakVal / normFactor : 0.0;

        result.offsetSamples = offset;
        result.offsetSeconds = static_cast<double>(offset) / m_sampleRate;
        result.confidence = std::clamp(confidence, 0.0, 1.0);
        result.success = true;
    }

    return result;
}

} // namespace asynk
