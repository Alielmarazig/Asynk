#pragma once
#include <QMainWindow>
#include <QTableWidget>
#include <QProgressBar>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QStackedWidget>
#include <QThread>
#include "../core/sync_engine.h"
#include "../core/media_handler.h"
#include "stat_cards.h"
#include "waveform_widget.h"

namespace asynk {

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private slots:
    void onImportClips();
    void onImportFolder();
    void onSetReference();
    void onSync();
    void onExport();
    void onClipSynced(int index, const SyncResult& result);
    void onSyncFinished(const SyncSession& session);
    void onSyncError(const QString& message);
    void onProgressChanged(int current, int total, const QString& clipName);

private:
    void setupMenuBar();
    void setupUI();
    void addClipsToTable(const QStringList& paths);
    void updateReferenceHighlight();
    void updateWaveformTracks();
    QString formatDuration(double seconds);
    QString formatFileSize(qint64 bytes);

    // Widgets
    QStackedWidget*  m_stack;
    QWidget*         m_dropZone;
    QWidget*         m_mainPanel;
    StatCardRow*     m_statCards;
    WaveformWidget*  m_waveform;
    QTableWidget*    m_table;
    QPushButton*     m_syncButton;
    QPushButton*     m_exportButton;
    QComboBox*       m_formatCombo;
    QComboBox*       m_fpsCombo;
    QProgressBar*    m_progressBar;
    QLabel*          m_statusLeft;
    QLabel*          m_statusRight;

    // Data
    QStringList      m_clipPaths;
    std::vector<MediaInfo> m_clipInfos;
    int              m_referenceIndex = -1;
    SyncSession      m_session;

    // Sync worker thread
    QThread*         m_syncThread = nullptr;
    SyncEngine*      m_syncEngine = nullptr;
};

} // namespace asynk
