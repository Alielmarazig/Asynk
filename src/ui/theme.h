#pragma once
#include <QString>
#include <QColor>
#include <QFont>

namespace asynk {
namespace theme {

// ── Palette ──
inline const QColor BASE       = QColor("#0f0f14");
inline const QColor SURFACE    = QColor("#1a1a24");
inline const QColor SURFACE2   = QColor("#22222e");
inline const QColor BORDER     = QColor("#2d2d3d");
inline const QColor TEXT       = QColor("#e8e8f0");
inline const QColor TEXT_DIM   = QColor("#8888a0");
inline const QColor PURPLE     = QColor("#6c5ce7");
inline const QColor PURPLE_DK  = QColor("#5a4bd6");
inline const QColor CORAL      = QColor("#f0997b");
inline const QColor GREEN      = QColor("#28c840");
inline const QColor YELLOW     = QColor("#f0c030");
inline const QColor RED        = QColor("#e04040");

// ── Fonts ──
inline QFont bodyFont() {
    QFont f("Segoe UI", 10);
    f.setStyleHint(QFont::SansSerif);
    return f;
}

inline QFont headerFont() {
    QFont f("Segoe UI", 12, QFont::DemiBold);
    f.setStyleHint(QFont::SansSerif);
    return f;
}

inline QFont monoFont() {
    QFont f("Consolas", 9);
    f.setStyleHint(QFont::Monospace);
    return f;
}

// ── Global stylesheet ──
QString globalStylesheet();

} // namespace theme
} // namespace asynk
