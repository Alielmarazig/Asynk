#pragma once
#include <QString>
#include <QObject>
#include <QThread>
#include <vector>
#include <optional>
#include "media_handler.h"

namespace asynk {

/// Result of syncing one clip against the reference
struct SyncResult {
    QString clipPath;
    int     offsetSamples  = 0;
    double  offsetSeconds  = 0.0;
    double  confidence     = 0.0;   // 0.0–1.0
    int     sampleRate     = 8000;
    bool    success        = false;
    QString error;
};

/// A complete sync session
struct SyncSession {
    QString referencePath;
    std::vector<SyncResult> results;
    double  totalDuration = 0.0;
};

/// The sync engine runs in a worker thread, emits progress signals.
class SyncEngine : public QObject {
    Q_OBJECT
public:
    explicit SyncEngine(QObject* parent = nullptr);

    void setReference(const QString& path);
    void setClips(const QStringList& paths);
    void setSampleRate(int rate);

    const SyncSession& session() const { return m_session; }

signals:
    void progressChanged(int current, int total, const QString& clipName);
    void clipSynced(int index, const SyncResult& result);
    void syncFinished(const SyncSession& session);
    void syncError(const QString& message);

public slots:
    void runSync();

private:
    SyncResult syncOneClip(const QString& clipPath,
                           const std::vector<float>& refAudio);

    QString     m_referencePath;
    QStringList m_clipPaths;
    int         m_sampleRate = 8000;
    SyncSession m_session;
};

} // namespace asynk
