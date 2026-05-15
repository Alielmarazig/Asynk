/**
 * fcpxml_exporter.cpp
 *
 * Generates FCPXML v1.9 timeline with synced clips on separate tracks.
 */

#include "fcpxml_exporter.h"
#include "../core/media_handler.h"
#include <QXmlStreamWriter>
#include <QFile>
#include <QFileInfo>
#include <cmath>

namespace asynk {

static QString rationalTime(double seconds, double fps) {
    int frames = static_cast<int>(std::round(std::abs(seconds) * fps));
    int fpsInt = static_cast<int>(std::round(fps));

    if (std::abs(fps - 23.976) < 0.01)
        return QString("%1/24000s").arg(frames * 1001);
    else if (std::abs(fps - 29.97) < 0.01)
        return QString("%1/30000s").arg(frames * 1001);
    else if (std::abs(fps - 59.94) < 0.01)
        return QString("%1/60000s").arg(frames * 1001);
    else
        return QString("%1/%2s").arg(frames).arg(fpsInt);
}

bool exportFCPXML(const SyncSession& session,
                  const QString& outputPath,
                  const QString& projectName,
                  double fps)
{
    QFile file(outputPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    QXmlStreamWriter xml(&file);
    xml.setAutoFormatting(true);
    xml.writeStartDocument();
    xml.writeDTD("<!DOCTYPE fcpxml>");

    // <fcpxml version="1.9">
    xml.writeStartElement("fcpxml");
    xml.writeAttribute("version", "1.9");

    // Resources
    xml.writeStartElement("resources");

    // Format resource
    QString frameDur = rationalTime(1.0 / fps, fps);
    xml.writeStartElement("format");
    xml.writeAttribute("id", "r1");
    xml.writeAttribute("frameDuration", frameDur);
    xml.writeAttribute("width", "1920");
    xml.writeAttribute("height", "1080");
    xml.writeEndElement(); // format

    // Asset resources for each clip
    int assetIdx = 1;
    for (const auto& result : session.results) {
        auto info = probeFile(result.clipPath);
        xml.writeStartElement("asset");
        xml.writeAttribute("id", QString("a%1").arg(assetIdx));
        xml.writeAttribute("name", QFileInfo(result.clipPath).baseName());
        xml.writeAttribute("src", "file://" + result.clipPath);
        if (info) {
            xml.writeAttribute("duration",
                rationalTime(info->duration, fps));
        }
        xml.writeAttribute("format", "r1");
        xml.writeEndElement(); // asset
        assetIdx++;
    }

    xml.writeEndElement(); // resources

    // Library > Event > Project > Sequence
    xml.writeStartElement("library");
    xml.writeStartElement("event");
    xml.writeAttribute("name", "asynk Export");

    xml.writeStartElement("project");
    xml.writeAttribute("name", projectName);

    xml.writeStartElement("sequence");
    xml.writeAttribute("format", "r1");
    xml.writeAttribute("duration",
        rationalTime(session.totalDuration, fps));

    xml.writeStartElement("spine");

    // Write clips
    assetIdx = 1;
    for (const auto& result : session.results) {
        auto info = probeFile(result.clipPath);
        double dur = info ? info->duration : 60.0;

        xml.writeStartElement("clip");
        xml.writeAttribute("name", QFileInfo(result.clipPath).baseName());
        xml.writeAttribute("offset", rationalTime(result.offsetSeconds, fps));
        xml.writeAttribute("duration", rationalTime(dur, fps));

        xml.writeStartElement("video");
        xml.writeAttribute("ref", QString("a%1").arg(assetIdx));
        xml.writeEndElement(); // video

        xml.writeEndElement(); // clip
        assetIdx++;
    }

    xml.writeEndElement(); // spine
    xml.writeEndElement(); // sequence
    xml.writeEndElement(); // project
    xml.writeEndElement(); // event
    xml.writeEndElement(); // library
    xml.writeEndElement(); // fcpxml

    xml.writeEndDocument();
    file.close();

    return true;
}

} // namespace asynk
