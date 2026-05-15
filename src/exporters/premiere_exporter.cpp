/**
 * premiere_exporter.cpp
 *
 * Generates Premiere Pro compatible XML sequence.
 * Compatible with Premiere CS6 through CC 2024.
 */

#include "premiere_exporter.h"
#include "../core/media_handler.h"
#include <QXmlStreamWriter>
#include <QFile>
#include <QFileInfo>
#include <cmath>

namespace asynk {

static int secondsToFrames(double seconds, double fps) {
    return static_cast<int>(std::round(seconds * fps));
}

bool exportPremiereXML(const SyncSession& session,
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

    int timebase = static_cast<int>(std::round(fps));
    bool ntsc = (std::abs(fps - 23.976) < 0.01 ||
                 std::abs(fps - 29.97) < 0.01 ||
                 std::abs(fps - 59.94) < 0.01);

    int totalFrames = secondsToFrames(session.totalDuration, fps);

    // <xmeml version="5">
    xml.writeStartElement("xmeml");
    xml.writeAttribute("version", "5");

    xml.writeStartElement("sequence");
    xml.writeTextElement("name", projectName);
    xml.writeTextElement("duration", QString::number(totalFrames));

    // Rate
    xml.writeStartElement("rate");
    xml.writeTextElement("timebase", QString::number(timebase));
    xml.writeTextElement("ntsc", ntsc ? "TRUE" : "FALSE");
    xml.writeEndElement(); // rate

    // Media > video > track
    xml.writeStartElement("media");
    xml.writeStartElement("video");

    // One track per clip
    int trackIdx = 0;
    for (const auto& result : session.results) {
        auto info = probeFile(result.clipPath);
        double dur = info ? info->duration : 60.0;
        int startFrame = secondsToFrames(result.offsetSeconds, fps);
        int durFrames = secondsToFrames(dur, fps);

        xml.writeStartElement("track");

        xml.writeStartElement("clipitem");
        xml.writeTextElement("name", QFileInfo(result.clipPath).baseName());
        xml.writeTextElement("duration", QString::number(durFrames));

        xml.writeStartElement("rate");
        xml.writeTextElement("timebase", QString::number(timebase));
        xml.writeTextElement("ntsc", ntsc ? "TRUE" : "FALSE");
        xml.writeEndElement(); // rate

        xml.writeTextElement("start", QString::number(startFrame));
        xml.writeTextElement("end", QString::number(startFrame + durFrames));
        xml.writeTextElement("in", "0");
        xml.writeTextElement("out", QString::number(durFrames));

        // File reference
        xml.writeStartElement("file");
        xml.writeAttribute("id", QString("file-%1").arg(trackIdx));
        xml.writeTextElement("name", QFileInfo(result.clipPath).fileName());
        xml.writeTextElement("pathurl", "file://localhost/" + result.clipPath);
        xml.writeEndElement(); // file

        xml.writeEndElement(); // clipitem
        xml.writeEndElement(); // track

        trackIdx++;
    }

    xml.writeEndElement(); // video

    // Audio tracks
    xml.writeStartElement("audio");

    trackIdx = 0;
    for (const auto& result : session.results) {
        auto info = probeFile(result.clipPath);
        if (!info || !info->hasAudio) { trackIdx++; continue; }

        double dur = info->duration;
        int startFrame = secondsToFrames(result.offsetSeconds, fps);
        int durFrames = secondsToFrames(dur, fps);

        xml.writeStartElement("track");

        xml.writeStartElement("clipitem");
        xml.writeTextElement("name", QFileInfo(result.clipPath).baseName() + " (audio)");
        xml.writeTextElement("duration", QString::number(durFrames));

        xml.writeStartElement("rate");
        xml.writeTextElement("timebase", QString::number(timebase));
        xml.writeTextElement("ntsc", ntsc ? "TRUE" : "FALSE");
        xml.writeEndElement(); // rate

        xml.writeTextElement("start", QString::number(startFrame));
        xml.writeTextElement("end", QString::number(startFrame + durFrames));
        xml.writeTextElement("in", "0");
        xml.writeTextElement("out", QString::number(durFrames));

        xml.writeStartElement("file");
        xml.writeAttribute("id", QString("file-%1").arg(trackIdx));
        xml.writeEndElement(); // file (reference by id)

        xml.writeEndElement(); // clipitem
        xml.writeEndElement(); // track

        trackIdx++;
    }

    xml.writeEndElement(); // audio
    xml.writeEndElement(); // media
    xml.writeEndElement(); // sequence
    xml.writeEndElement(); // xmeml

    xml.writeEndDocument();
    file.close();

    return true;
}

} // namespace asynk
