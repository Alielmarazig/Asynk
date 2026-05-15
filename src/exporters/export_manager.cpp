/**
 * export_manager.cpp
 *
 * Routes export requests to the appropriate exporter(s).
 */

#include "export_manager.h"
#include "fcpxml_exporter.h"
#include "premiere_exporter.h"
#include "edl_exporter.h"
#include <QDir>

namespace asynk {

ExportResult exportSession(const SyncSession& session,
                           const QString& outputDir,
                           ExportFormat format,
                           const QString& projectName,
                           double fps)
{
    ExportResult result;
    QDir dir(outputDir);
    if (!dir.exists()) dir.mkpath(".");

    auto doExport = [&](ExportFormat fmt) {
        QString path;
        bool ok = false;

        switch (fmt) {
        case ExportFormat::FCPXML:
            path = dir.filePath(projectName + ".fcpxml");
            ok = exportFCPXML(session, path, projectName, fps);
            break;
        case ExportFormat::PremiereXML:
            path = dir.filePath(projectName + ".xml");
            ok = exportPremiereXML(session, path, projectName, fps);
            break;
        case ExportFormat::EDL:
            path = dir.filePath(projectName + ".edl");
            ok = exportEDL(session, path, projectName, fps);
            break;
        default:
            break;
        }

        if (ok) result.exportedFiles.append(path);
    };

    if (format == ExportFormat::All) {
        doExport(ExportFormat::FCPXML);
        doExport(ExportFormat::PremiereXML);
        doExport(ExportFormat::EDL);
    } else {
        doExport(format);
    }

    result.success = !result.exportedFiles.isEmpty();
    if (!result.success)
        result.error = "No files were exported. Check write permissions.";

    return result;
}

} // namespace asynk
