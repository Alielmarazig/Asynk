#pragma once
#include <QWidget>
#include <QTimer>
#include <vector>
#include "../core/sync_engine.h"

namespace asynk {

struct WaveformTrack {
    QString name;
    double  offsetSeconds = 0.0;
    double  duration      = 0.0;
    bool    isReference   = false;
    bool    isAudioOnly   = false;
    double  confidence    = 0.0;
    std::vector<float> envelope;  // Pre-computed waveform envelope
};

class WaveformWidget : public QWidget {
    Q_OBJECT
public:
    explicit WaveformWidget(QWidget* parent = nullptr);

    void setTracks(const std::vector<WaveformTrack>& tracks);
    void clear();
    void startPlayheadAnimation();
    void stopPlayheadAnimation();

    QSize sizeHint() const override { return QSize(600, 180); }
    QSize minimumSizeHint() const override { return QSize(300, 100); }

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;

private:
    void generateEnvelope(WaveformTrack& track);
    double timeToX(double seconds) const;
    double xToTime(double x) const;

    std::vector<WaveformTrack> m_tracks;
    double m_timelineStart  = 0.0;
    double m_timelineEnd    = 60.0;
    double m_playheadPos    = 0.0;
    bool   m_animating      = false;
    QTimer m_animTimer;
};

} // namespace asynk
