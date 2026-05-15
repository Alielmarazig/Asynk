/**
 * main_window.cpp
 *
 * Full main window with:
 * - Drop zone (shown until clips imported)
 * - Stat cards row
 * - Waveform alignment panel
 * - Clip table with reference highlighting and confidence colors
 * - Sync/Export controls
 * - Worker thread for sync engine
 */

#include "main_window.h"
#include "theme.h"
#include "../exporters/export_manager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <QMessageBox>
#include <QStatusBar>
#include <QHeaderView>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>
#include <QFileInfo>
#include <cmath>

namespace asynk {

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("asynk — Multi-track Sync");
    setMinimumSize(1000, 700);
    resize(1200, 800);
    setAcceptDrops(true);

    setupMenuBar();
    setupUI();

    // Status bar
    m_statusLeft = new QLabel("Ready — drag files or use File > Import");
    m_statusRight = new QLabel("v0.3.0");
    statusBar()->addWidget(m_statusLeft, 1);
    statusBar()->addPermanentWidget(m_statusRight);
}

MainWindow::~MainWindow() {
    if (m_syncThread) {
        m_syncThread->quit();
        m_syncThread->wait();
    }
}

// ── Menu Bar ──

void MainWindow::setupMenuBar() {
    auto* fileMenu = menuBar()->addMenu("&File");

    auto* importClips = fileMenu->addAction("Import &Clips...");
    importClips->setShortcut(QKeySequence("Ctrl+O"));
    connect(importClips, &QAction::triggered, this, &MainWindow::onImportClips);

    auto* importFolder = fileMenu->addAction("Import &Folder...");
    importFolder->setShortcut(QKeySequence("Ctrl+Shift+O"));
    connect(importFolder, &QAction::triggered, this, &MainWindow::onImportFolder);

    fileMenu->addSeparator();

    auto* exportAct = fileMenu->addAction("&Export Timeline...");
    exportAct->setShortcut(QKeySequence("Ctrl+E"));
    connect(exportAct, &QAction::triggered, this, &MainWindow::onExport);

    fileMenu->addSeparator();

    auto* quit = fileMenu->addAction("&Quit");
    quit->setShortcut(QKeySequence("Ctrl+Q"));
    connect(quit, &QAction::triggered, this, &QMainWindow::close);

    auto* editMenu = menuBar()->addMenu("&Edit");

    auto* setRef = editMenu->addAction("Set as &Reference");
    setRef->setShortcut(QKeySequence("Ctrl+R"));
    connect(setRef, &QAction::triggered, this, &MainWindow::onSetReference);

    auto* clearAll = editMenu->addAction("&Clear All");
    connect(clearAll, &QAction::triggered, this, [this]() {
        m_clipPaths.clear();
        m_clipInfos.clear();
        m_referenceIndex = -1;
        m_table->setRowCount(0);
        m_statCards->reset();
        m_waveform->clear();
        m_stack->setCurrentIndex(0);
        m_statusLeft->setText("Cleared. Drag files or use File > Import.");
    });
}

// ── UI Setup ──

void MainWindow::setupUI() {
    auto* central = new QWidget(this);
    setCentralWidget(central);
    auto* mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(16, 12, 16, 8);
    mainLayout->setSpacing(12);

    // Stacked widget: drop zone vs main panel
    m_stack = new QStackedWidget();

    // ── Drop Zone ──
    m_dropZone = new QWidget();
    auto* dropLayout = new QVBoxLayout(m_dropZone);
    dropLayout->setAlignment(Qt::AlignCenter);

    auto* dropIcon = new QLabel("🎬", m_dropZone);
    dropIcon->setFont(QFont("Segoe UI Emoji", 48));
    dropIcon->setAlignment(Qt::AlignCenter);
    dropIcon->setStyleSheet("background: transparent;");

    auto* dropTitle = new QLabel("Drop media files here", m_dropZone);
    dropTitle->setFont(theme::headerFont());
    dropTitle->setAlignment(Qt::AlignCenter);
    dropTitle->setStyleSheet("background: transparent; color: #e8e8f0;");

    auto* dropSub = new QLabel("or use File > Import Clips / Import Folder", m_dropZone);
    dropSub->setObjectName("dimLabel");
    dropSub->setAlignment(Qt::AlignCenter);
    dropSub->setStyleSheet("background: transparent;");

    auto* dropBtn = new QPushButton("Import Clips", m_dropZone);
    dropBtn->setFixedWidth(160);
    connect(dropBtn, &QPushButton::clicked, this, &MainWindow::onImportClips);

    dropLayout->addStretch();
    dropLayout->addWidget(dropIcon);
    dropLayout->addSpacing(8);
    dropLayout->addWidget(dropTitle);
    dropLayout->addWidget(dropSub);
    dropLayout->addSpacing(16);
    dropLayout->addWidget(dropBtn, 0, Qt::AlignCenter);
    dropLayout->addStretch();

    m_dropZone->setStyleSheet(
        "background: #1a1a24;"
        "border: 2px dashed #2d2d3d;"
        "border-radius: 12px;");

    // ── Main Panel ──
    m_mainPanel = new QWidget();
    auto* panelLayout = new QVBoxLayout(m_mainPanel);
    panelLayout->setContentsMargins(0, 0, 0, 0);
    panelLayout->setSpacing(12);

    // Stat cards
    m_statCards = new StatCardRow();
    panelLayout->addWidget(m_statCards);

    // Waveform
    m_waveform = new WaveformWidget();
    panelLayout->addWidget(m_waveform);

    // Table
    m_table = new QTableWidget(0, 7);
    m_table->setHorizontalHeaderLabels({
        "Name", "Type", "Duration", "Resolution", "Size", "Offset", "Confidence"
    });
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(5, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(6, QHeaderView::ResizeToContents);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->verticalHeader()->setVisible(false);
    m_table->setAlternatingRowColors(false);
    panelLayout->addWidget(m_table, 1);

    // Controls row
    auto* controlsRow = new QHBoxLayout();
    controlsRow->setSpacing(10);

    auto* importBtn = new QPushButton("+ Import");
    connect(importBtn, &QPushButton::clicked, this, &MainWindow::onImportClips);

    auto* refBtn = new QPushButton("Set Reference");
    connect(refBtn, &QPushButton::clicked, this, &MainWindow::onSetReference);

    m_syncButton = new QPushButton("Sync");
    m_syncButton->setObjectName("syncButton");
    m_syncButton->setEnabled(false);
    connect(m_syncButton, &QPushButton::clicked, this, &MainWindow::onSync);

    auto* separator = new QFrame();
    separator->setFrameShape(QFrame::VLine);
    separator->setStyleSheet("color: #2d2d3d;");

    auto* fpsLabel = new QLabel("FPS:");
    fpsLabel->setObjectName("dimLabel");
    m_fpsCombo = new QComboBox();
    m_fpsCombo->addItems({"23.976", "24", "25", "29.97", "30", "50", "59.94", "60"});
    m_fpsCombo->setCurrentIndex(1);

    auto* fmtLabel = new QLabel("Export:");
    fmtLabel->setObjectName("dimLabel");
    m_formatCombo = new QComboBox();
    m_formatCombo->addItem("All Formats",    static_cast<int>(ExportFormat::All));
    m_formatCombo->addItem("FCP X (.fcpxml)", static_cast<int>(ExportFormat::FCPXML));
    m_formatCombo->addItem("Premiere (.xml)", static_cast<int>(ExportFormat::PremiereXML));
    m_formatCombo->addItem("EDL (.edl)",      static_cast<int>(ExportFormat::EDL));

    m_exportButton = new QPushButton("Export");
    m_exportButton->setEnabled(false);
    connect(m_exportButton, &QPushButton::clicked, this, &MainWindow::onExport);

    controlsRow->addWidget(importBtn);
    controlsRow->addWidget(refBtn);
    controlsRow->addWidget(m_syncButton);
    controlsRow->addStretch();
    controlsRow->addWidget(separator);
    controlsRow->addWidget(fpsLabel);
    controlsRow->addWidget(m_fpsCombo);
    controlsRow->addWidget(fmtLabel);
    controlsRow->addWidget(m_formatCombo);
    controlsRow->addWidget(m_exportButton);

    panelLayout->addLayout(controlsRow);

    // Progress bar
    m_progressBar = new QProgressBar();
    m_progressBar->setVisible(false);
    m_progressBar->setTextVisible(true);
    panelLayout->addWidget(m_progressBar);

    // Stack
    m_stack->addWidget(m_dropZone);
    m_stack->addWidget(m_mainPanel);
    m_stack->setCurrentIndex(0);

    mainLayout->addWidget(m_stack);
}

// ── Import ──

void MainWindow::onImportClips() {
    QStringList filters;
    for (const auto& ext : SUPPORTED_EXTENSIONS)
        filters << "*" + ext;
    QString filter = "Media files (" + filters.join(" ") + ");;All files (*)";

    QStringList files = QFileDialog::getOpenFileNames(
        this, "Import Clips", QString(), filter);

    if (!files.isEmpty())
        addClipsToTable(files);
}

void MainWindow::onImportFolder() {
    QString dir = QFileDialog::getExistingDirectory(this, "Import Folder");
    if (dir.isEmpty()) return;

    QStringList files = scanDirectory(dir, true);
    if (files.isEmpty()) {
        QMessageBox::information(this, "No media found",
            "No supported media files found in this folder.");
        return;
    }

    addClipsToTable(files);
}

void MainWindow::addClipsToTable(const QStringList& paths) {
    for (const auto& path : paths) {
        if (m_clipPaths.contains(path)) continue;  // Skip duplicates

        auto info = probeFile(path);
        if (!info) continue;

        m_clipPaths.append(path);
        m_clipInfos.push_back(*info);

        int row = m_table->rowCount();
        m_table->insertRow(row);

        // Name
        auto* nameItem = new QTableWidgetItem(info->fileName);
        nameItem->setToolTip(path);
        m_table->setItem(row, 0, nameItem);

        // Type
        QString type = info->hasVideo ? "Video" : "Audio";
        m_table->setItem(row, 1, new QTableWidgetItem(type));

        // Duration
        m_table->setItem(row, 2, new QTableWidgetItem(formatDuration(info->duration)));

        // Resolution
        QString res = info->hasVideo
            ? QString("%1×%2").arg(info->width).arg(info->height)
            : "—";
        m_table->setItem(row, 3, new QTableWidgetItem(res));

        // Size
        m_table->setItem(row, 4, new QTableWidgetItem(formatFileSize(info->fileSize)));

        // Offset (filled after sync)
        m_table->setItem(row, 5, new QTableWidgetItem("—"));

        // Confidence (filled after sync)
        m_table->setItem(row, 6, new QTableWidgetItem("—"));
    }

    // Auto-set first clip as reference if none set
    if (m_referenceIndex < 0 && !m_clipPaths.isEmpty()) {
        m_referenceIndex = 0;
        updateReferenceHighlight();
    }

    m_syncButton->setEnabled(m_clipPaths.size() >= 2);
    m_stack->setCurrentIndex(1);

    // Update stats
    double totalDur = 0;
    for (const auto& info : m_clipInfos) totalDur += info.duration;
    m_statCards->updateStats(m_clipPaths.size(), 0, 0.0, totalDur);

    updateWaveformTracks();

    m_statusLeft->setText(
        QString("Loaded %1 clips. Select reference and hit Sync.")
            .arg(m_clipPaths.size()));
}

void MainWindow::updateReferenceHighlight() {
    for (int row = 0; row < m_table->rowCount(); ++row) {
        QColor bg = (row == m_referenceIndex)
            ? QColor(108, 92, 231, 30)  // purple tint
            : QColor(0, 0, 0, 0);       // transparent

        for (int col = 0; col < m_table->columnCount(); ++col) {
            if (auto* item = m_table->item(row, col))
                item->setBackground(bg);
        }

        // Add (REF) suffix to name
        if (auto* item = m_table->item(row, 0)) {
            QString name = m_clipInfos[row].fileName;
            if (row == m_referenceIndex)
                name += "  ★ REF";
            item->setText(name);
        }
    }
}

void MainWindow::onSetReference() {
    int row = m_table->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "No selection",
            "Select a clip in the table first.");
        return;
    }

    m_referenceIndex = row;
    updateReferenceHighlight();
    m_statusLeft->setText(
        QString("Reference: %1").arg(m_clipInfos[row].fileName));
}

// ── Sync ──

void MainWindow::onSync() {
    if (m_clipPaths.size() < 2) return;
    if (m_referenceIndex < 0) {
        QMessageBox::warning(this, "No reference",
            "Select a reference clip first (Edit > Set as Reference).");
        return;
    }

    m_syncButton->setEnabled(false);
    m_exportButton->setEnabled(false);
    m_progressBar->setVisible(true);
    m_progressBar->setRange(0, m_clipPaths.size());
    m_progressBar->setValue(0);

    // Create worker thread
    if (m_syncThread) {
        m_syncThread->quit();
        m_syncThread->wait();
        delete m_syncThread;
    }

    m_syncThread = new QThread(this);
    m_syncEngine = new SyncEngine();
    m_syncEngine->moveToThread(m_syncThread);

    m_syncEngine->setReference(m_clipPaths[m_referenceIndex]);
    m_syncEngine->setClips(m_clipPaths);

    connect(m_syncThread, &QThread::started, m_syncEngine, &SyncEngine::runSync);
    connect(m_syncEngine, &SyncEngine::progressChanged,
            this, &MainWindow::onProgressChanged);
    connect(m_syncEngine, &SyncEngine::clipSynced,
            this, &MainWindow::onClipSynced);
    connect(m_syncEngine, &SyncEngine::syncFinished,
            this, &MainWindow::onSyncFinished);
    connect(m_syncEngine, &SyncEngine::syncError,
            this, &MainWindow::onSyncError);

    // Cleanup
    connect(m_syncEngine, &SyncEngine::syncFinished, m_syncThread, &QThread::quit);
    connect(m_syncEngine, &SyncEngine::syncError, m_syncThread, &QThread::quit);
    connect(m_syncThread, &QThread::finished, m_syncEngine, &QObject::deleteLater);

    m_syncThread->start();
}

void MainWindow::onProgressChanged(int current, int total, const QString& clipName) {
    m_progressBar->setValue(current);
    m_progressBar->setFormat(
        QString("Syncing %1/%2 — %3").arg(current).arg(total).arg(clipName));
}

void MainWindow::onClipSynced(int index, const SyncResult& result) {
    if (index < 0 || index >= m_table->rowCount()) return;

    // Offset column
    if (result.success) {
        m_table->item(index, 5)->setText(
            QString("%1%2s")
                .arg(result.offsetSeconds >= 0 ? "+" : "")
                .arg(result.offsetSeconds, 0, 'f', 4));
    } else {
        m_table->item(index, 5)->setText("ERR");
    }

    // Confidence column with color
    if (result.success) {
        double pct = result.confidence * 100.0;
        auto* confItem = m_table->item(index, 6);
        confItem->setText(QString("%1%").arg(pct, 0, 'f', 1));

        QColor confColor;
        if (pct >= 70)      confColor = theme::GREEN;
        else if (pct >= 40) confColor = theme::YELLOW;
        else                confColor = theme::RED;
        confItem->setForeground(confColor);
    } else {
        m_table->item(index, 6)->setText("—");
        m_table->item(index, 6)->setForeground(theme::RED);
    }
}

void MainWindow::onSyncFinished(const SyncSession& session) {
    m_session = session;

    m_progressBar->setValue(m_progressBar->maximum());
    m_progressBar->setFormat("Sync complete!");

    m_syncButton->setEnabled(true);
    m_exportButton->setEnabled(true);

    // Calculate stats
    int synced = 0;
    double totalConf = 0;
    for (const auto& r : session.results) {
        if (r.success) {
            synced++;
            totalConf += r.confidence;
        }
    }
    double avgConf = synced > 0 ? (totalConf / synced) * 100.0 : 0.0;

    double totalDur = 0;
    for (const auto& info : m_clipInfos) totalDur += info.duration;

    m_statCards->updateStats(m_clipPaths.size(), synced, avgConf, totalDur);
    updateWaveformTracks();
    m_waveform->startPlayheadAnimation();

    m_statusLeft->setText(
        QString("Synced %1/%2 clips — avg confidence %3%")
            .arg(synced).arg(m_clipPaths.size())
            .arg(avgConf, 0, 'f', 1));
}

void MainWindow::onSyncError(const QString& message) {
    m_progressBar->setVisible(false);
    m_syncButton->setEnabled(true);
    QMessageBox::critical(this, "Sync Error", message);
}

// ── Export ──

void MainWindow::onExport() {
    if (m_session.results.empty()) {
        QMessageBox::information(this, "Nothing to export",
            "Run Sync first before exporting.");
        return;
    }

    QString dir = QFileDialog::getExistingDirectory(this, "Choose Export Folder");
    if (dir.isEmpty()) return;

    auto fmt = static_cast<ExportFormat>(m_formatCombo->currentData().toInt());
    double fps = m_fpsCombo->currentText().toDouble();

    auto result = exportSession(m_session, dir, fmt, "asynk_Synced_Timeline", fps);

    if (result.success) {
        QStringList names;
        for (const auto& f : result.exportedFiles)
            names << "  " + QFileInfo(f).fileName();

        QMessageBox::information(this, "Export complete",
            QString("Exported %1 file(s) to:\n%2\n\n%3")
                .arg(result.exportedFiles.size())
                .arg(dir)
                .arg(names.join("\n")));

        m_statusLeft->setText(
            QString("Exported %1 timeline(s) to %2")
                .arg(result.exportedFiles.size()).arg(dir));
    } else {
        QMessageBox::critical(this, "Export error",
            "Export failed: " + result.error);
    }
}

// ── Drag & Drop ──

void MainWindow::dragEnterEvent(QDragEnterEvent* event) {
    if (event->mimeData()->hasUrls())
        event->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent* event) {
    QStringList paths;
    for (const auto& url : event->mimeData()->urls()) {
        QString path = url.toLocalFile();
        if (QFileInfo(path).isDir()) {
            paths.append(scanDirectory(path, true));
        } else if (isSupported(path)) {
            paths.append(path);
        }
    }
    if (!paths.isEmpty())
        addClipsToTable(paths);
}

// ── Waveform ──

void MainWindow::updateWaveformTracks() {
    std::vector<WaveformTrack> tracks;

    for (int i = 0; i < m_clipPaths.size(); ++i) {
        WaveformTrack t;
        t.name = m_clipInfos[i].fileName;
        t.duration = m_clipInfos[i].duration;
        t.isReference = (i == m_referenceIndex);
        t.isAudioOnly = !m_clipInfos[i].hasVideo;

        // Apply sync offsets if available
        if (i < static_cast<int>(m_session.results.size())) {
            t.offsetSeconds = m_session.results[i].offsetSeconds;
            t.confidence = m_session.results[i].confidence;
        }

        tracks.push_back(t);
    }

    m_waveform->setTracks(tracks);
}

// ── Helpers ──

QString MainWindow::formatDuration(double seconds) {
    int totalSec = static_cast<int>(seconds);
    int h = totalSec / 3600;
    int m = (totalSec % 3600) / 60;
    int s = totalSec % 60;
    int ms = static_cast<int>((seconds - totalSec) * 1000);

    if (h > 0)
        return QString("%1:%2:%3.%4")
            .arg(h).arg(m, 2, 10, QChar('0'))
            .arg(s, 2, 10, QChar('0')).arg(ms, 3, 10, QChar('0'));
    else
        return QString("%1:%2.%3")
            .arg(m, 2, 10, QChar('0'))
            .arg(s, 2, 10, QChar('0')).arg(ms, 3, 10, QChar('0'));
}

QString MainWindow::formatFileSize(qint64 bytes) {
    if (bytes < 1024)
        return QString("%1 B").arg(bytes);
    if (bytes < 1024 * 1024)
        return QString("%1 KB").arg(bytes / 1024.0, 0, 'f', 1);
    if (bytes < 1024 * 1024 * 1024)
        return QString("%1 MB").arg(bytes / (1024.0 * 1024.0), 0, 'f', 1);
    return QString("%1 GB").arg(bytes / (1024.0 * 1024.0 * 1024.0), 0, 'f', 2);
}

} // namespace asynk
