#pragma once
#include <QString>
#include "../core/sync_engine.h"

namespace asynk {

bool exportPremiereXML(const SyncSession& session,
                       const QString& outputPath,
                       const QString& projectName = "asynk Synced Timeline",
                       double fps = 24.0);

} // namespace asynk
