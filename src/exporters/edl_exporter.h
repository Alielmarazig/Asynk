#pragma once
#include <QString>
#include "../core/sync_engine.h"

namespace asynk {

bool exportEDL(const SyncSession& session,
               const QString& outputPath,
               const QString& title = "asynk Synced Timeline",
               double fps = 24.0);

} // namespace asynk
