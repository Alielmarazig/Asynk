#pragma once
#include <QString>
#include <QStringList>
#include "../core/sync_engine.h"

namespace asynk {

enum class ExportFormat {
    FCPXML,         // Final Cut Pro X
    PremiereXML,    // Premiere Pro
    EDL,            // Vegas/Edius/Resolve
    All             // Export all formats
};

struct ExportResult {
    QStringList exportedFiles;
    bool success = false;
    QString error;
};

ExportResult exportSession(const SyncSession& session,
                           const QString& outputDir,
                           ExportFormat format = ExportFormat::All,
                           const QString& projectName = "asynk_Synced_Timeline",
                           double fps = 24.0);

} // namespace asynk
