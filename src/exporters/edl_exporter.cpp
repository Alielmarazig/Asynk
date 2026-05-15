/**
 * edl_exporter.cpp
 *
 * CMX 3600 EDL format. Compatible with Sony Vegas, Magix Vegas, EDIUS,
 * DaVinci Resolve, and most NLEs that accept EDL.
 */

#include "edl_exporter.h"
#include "../core/media_handler.h"
#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <cmath>

namespace asynk {

static QString framesToTimecode(int totalFrames, double fps) {
    int fpsInt = static_cast<int>(std::round(fps));
    int frames = totalFrames % fpsInt;
    int totalSec = totalFrames / fpsInt;
    int sec = totalSec % 60;
    int totalMin = totalSec / 60;
    int min = totalMin % 60;
    int hr = totalMin / 60;

    // Use semicolon for drop-frame
    bool dropFrame = (std::abs(fps - 29.97) < 0.01 ||
                      std::abs(fps - 59.94) < 0.01);
    QChar sep = dropFrame ? ';' : ':';

    return QString("%1:%2:%3%4%5")
        .arg(hr, 2, 10, QChar('0'))
        .arg(min, 2, 10, QChar('0'))
        .arg(sec, 2, 10, QChar('0'))
        .arg(sep)
        .arg(frames, 2, 10, QChar('0'));
}

bool exportEDL(const SyncSession& session,
               const QString& outputPath,
               const QString& title,
               double fps)
{
    QFile file(outputPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    QTextStream out(&file);

    // EDL header
    out << "TITLE: " << title << "\n";
    out << "FCM: "
        << ((std::abs(fps - 29.97) < 0.01 || std::abs(fps - 59.94) < 0.01)
            ? "DROP FRAME" : "NON-DROP FRAME")
        << "\n\n";

    int editNum = 1;
    for (const auto& result : session.results) {
        auto info = probeFile(result.clipPath);
        double dur = info ? info->duration : 60.0;

        int srcInFrames = 0;
        int srcOutFrames = static_cast<int>(std::round(dur * fps));
        int recInFrames = static_cast<int>(std::round(result.offsetSeconds * fps));
        int recOutFrames = recInFrames + srcOutFrames;

        // Clamp negative
        if (recInFrames < 0) recInFrames = 0;
        if (recOutFrames < 0) recOutFrames = srcOutFrames;

        QString reelName = QFileInfo(result.clipPath).baseName()
                           .left(8).toUpper();
        if (reelName.isEmpty()) reelName = "AX";

        // Edit decision line
        out << QString("%1").arg(editNum, 3, 10, QChar('0'))
            << "  " << reelName
            << "  AA/V  C        "
            << framesToTimecode(srcInFrames, fps)
            << " " << framesToTimecode(srcOutFrames, fps)
            << " " << framesToTimecode(recInFrames, fps)
            << " " << framesToTimecode(recOutFrames, fps)
            << "\n";

        // Source file comment
        out << "* FROM CLIP NAME: "
            << QFileInfo(result.clipPath).fileName() << "\n";

        if (result.success) {
            out << "* SYNC OFFSET: "
                << QString::number(result.offsetSeconds, 'f', 4) << "s"
                << "  CONFIDENCE: "
                << QString::number(result.confidence * 100.0, 'f', 1)
                << "%\n";
        }

        out << "\n";
        editNum++;
    }

    file.close();
    return true;
}

} // namespace asynk
