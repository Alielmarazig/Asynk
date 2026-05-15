/**
 * waveform_widget.cpp
 *
 * Custom QPainter widget for multi-track waveform alignment visualization.
 * Purple = reference, gray = video tracks, coral = audio-only tracks.
 * Playhead with click/drag scrubbing and auto-animation after sync.
 */

#include "waveform_widget.h"
#include "theme.h"
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <cmath>
#include <functional>
#include <random>

namespace asynk {

WaveformWidget::WaveformWidget(QWidget* parent) : QWidget(parent) {
    setMinimumHeight(100);
    setStyleSheet("background: transparent;");
    setMouseTracking(true);

    connect(&m_animTimer, &QTimer::timeout, this, [this]() {
        m_playheadPos += 0.05;
        if (m_playheadPos > m_timelineEnd) {
            stopPlayheadAnimation();
            return;
        }
        update();
    });
}

void WaveformWidget::setTracks(const std::vector<WaveformTrack>& tracks) {
    m_tracks = tracks;

    // Compute timeline bounds
    m_timelineStart = 0.0;
    m_timelineEnd = 1.0;
    for (auto& t : m_tracks) {
        double start = t.offsetSeconds;
        double end   = t.offsetSeconds + t.duration;
        if (start < m_timelineStart) m_timelineStart = start;
        if (end > m_timelineEnd) m_timelineEnd = end;
    }

    // Add padding
    double pad = (m_timelineEnd - m_timelineStart) * 0.05;
    m_timelineStart -= pad;
    m_timelineEnd   += pad;

    // Generate envelopes
    for (auto& t : m_tracks) {
        generateEnvelope(t);
    }

    m_playheadPos = m_timelineStart;
    update();
}

void WaveformWidget::clear() {
    m_tracks.clear();
    m_playheadPos = 0.0;
    stopPlayheadAnimation();
    update();
}

void WaveformWidget::startPlayheadAnimation() {
    m_playheadPos = m_timelineStart;
    m_animating = true;
    m_animTimer.start(30);  // ~33fps
}

void WaveformWidget::stopPlayheadAnimation() {
    m_animating = false;
    m_animTimer.stop();
    update();
}

void WaveformWidget::generateEnvelope(WaveformTrack& track) {
    // Seeded pseudo-random envelope from name hash
    std::hash<std::string> hasher;
    size_t seed = hasher(track.name.toStdString());
    std::mt19937 rng(static_cast<unsigned>(seed));
    std::uniform_real_distribution<float> dist(0.2f, 1.0f);

    const int points = 200;
    track.envelope.resize(points);

    // Generate base noise
    for (int i = 0; i < points; ++i) {
        track.envelope[i] = dist(rng);
    }

    // Smooth with running average
    std::vector<float> smoothed(points);
    for (int i = 0; i < points; ++i) {
        float sum = 0.0f;
        int count = 0;
        for (int j = std::max(0, i - 3); j <= std::min(points - 1, i + 3); ++j) {
            sum += track.envelope[j];
            count++;
        }
        smoothed[i] = sum / count;
    }
    track.envelope = smoothed;
}

double WaveformWidget::timeToX(double seconds) const {
    double range = m_timelineEnd - m_timelineStart;
    if (range <= 0) return 0;
    return ((seconds - m_timelineStart) / range) * width();
}

double WaveformWidget::xToTime(double x) const {
    double range = m_timelineEnd - m_timelineStart;
    return m_timelineStart + (x / width()) * range;
}

void WaveformWidget::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    int w = width();
    int h = height();

    // Background
    p.fillRect(rect(), theme::SURFACE);

    // Border
    p.setPen(QPen(theme::BORDER, 1));
    p.drawRoundedRect(rect().adjusted(0, 0, -1, -1), 8, 8);

    if (m_tracks.empty()) {
        p.setPen(theme::TEXT_DIM);
        p.setFont(theme::bodyFont());
        p.drawText(rect(), Qt::AlignCenter, "Waveforms appear after import");
        return;
    }

    // Draw tracks
    int trackCount = static_cast<int>(m_tracks.size());
    int trackH = std::max(20, (h - 20) / std::max(trackCount, 1));
    int y = 10;

    for (const auto& track : m_tracks) {
        // Track color
        QColor color;
        if (track.isReference)
            color = theme::PURPLE;
        else if (track.isAudioOnly)
            color = theme::CORAL;
        else
            color = QColor("#666680");

        // Track area
        double xStart = timeToX(track.offsetSeconds);
        double xEnd   = timeToX(track.offsetSeconds + track.duration);
        double trackW = xEnd - xStart;

        // Background bar
        QColor bgColor = color;
        bgColor.setAlpha(30);
        p.fillRect(QRectF(xStart, y, trackW, trackH - 4), bgColor);

        // Waveform envelope
        if (!track.envelope.empty()) {
            QPainterPath wavePath;
            int envSize = static_cast<int>(track.envelope.size());
            double halfH = (trackH - 4) / 2.0;
            double midY = y + halfH;

            // Top half
            wavePath.moveTo(xStart, midY);
            for (int i = 0; i < envSize; ++i) {
                double ex = xStart + (static_cast<double>(i) / envSize) * trackW;
                double ey = midY - track.envelope[i] * halfH * 0.8;
                wavePath.lineTo(ex, ey);
            }

            // Bottom half (mirror)
            for (int i = envSize - 1; i >= 0; --i) {
                double ex = xStart + (static_cast<double>(i) / envSize) * trackW;
                double ey = midY + track.envelope[i] * halfH * 0.8;
                wavePath.lineTo(ex, ey);
            }
            wavePath.closeSubpath();

            QColor fillColor = color;
            fillColor.setAlpha(100);
            p.fillPath(wavePath, fillColor);

            // Outline
            p.setPen(QPen(color, 1));
            p.drawPath(wavePath);
        }

        // Track label
        p.setPen(theme::TEXT);
        QFont lf = theme::bodyFont();
        lf.setPointSize(8);
        p.setFont(lf);
        p.drawText(QRectF(xStart + 4, y + 2, trackW - 8, 14),
                   Qt::AlignLeft | Qt::AlignTop,
                   track.name);

        y += trackH;
    }

    // Playhead
    double phX = timeToX(m_playheadPos);
    if (phX >= 0 && phX <= w) {
        p.setPen(QPen(QColor("#ffffff"), 1.5));
        p.drawLine(QPointF(phX, 0), QPointF(phX, h));

        // Head triangle
        QPainterPath tri;
        tri.moveTo(phX - 5, 0);
        tri.lineTo(phX + 5, 0);
        tri.lineTo(phX, 7);
        tri.closeSubpath();
        p.fillPath(tri, QColor("#ffffff"));
    }
}

void WaveformWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        m_playheadPos = xToTime(event->position().x());
        update();
    }
}

void WaveformWidget::mouseMoveEvent(QMouseEvent* event) {
    if (event->buttons() & Qt::LeftButton) {
        m_playheadPos = xToTime(event->position().x());
        update();
    }
}

} // namespace asynk
