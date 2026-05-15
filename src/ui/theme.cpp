/**
 * theme.cpp
 *
 * Full Qt stylesheet for dark cinematic UI.
 */

#include "theme.h"

namespace asynk {
namespace theme {

QString globalStylesheet() {
    return QStringLiteral(R"(
        /* ── Global ── */
        QWidget {
            background-color: #0f0f14;
            color: #e8e8f0;
            font-family: "Segoe UI", "SF Pro", "Helvetica Neue", sans-serif;
            font-size: 10pt;
        }

        /* ── Main Window ── */
        QMainWindow {
            background-color: #0f0f14;
        }

        QMainWindow::separator {
            background: #2d2d3d;
            width: 1px;
            height: 1px;
        }

        /* ── Menu Bar ── */
        QMenuBar {
            background-color: #0f0f14;
            border-bottom: 1px solid #2d2d3d;
            padding: 2px;
        }

        QMenuBar::item {
            background: transparent;
            padding: 4px 10px;
            border-radius: 4px;
        }

        QMenuBar::item:selected {
            background: #22222e;
        }

        QMenu {
            background-color: #1a1a24;
            border: 1px solid #2d2d3d;
            border-radius: 6px;
            padding: 4px;
        }

        QMenu::item {
            padding: 6px 24px;
            border-radius: 4px;
        }

        QMenu::item:selected {
            background: #6c5ce7;
        }

        /* ── Buttons ── */
        QPushButton {
            background-color: #22222e;
            border: 1px solid #2d2d3d;
            border-radius: 6px;
            padding: 8px 16px;
            color: #e8e8f0;
            font-weight: 500;
        }

        QPushButton:hover {
            background-color: #2d2d3d;
            border-color: #6c5ce7;
        }

        QPushButton:pressed {
            background-color: #1a1a24;
        }

        QPushButton:disabled {
            background-color: #1a1a24;
            color: #555570;
            border-color: #22222e;
        }

        QPushButton#syncButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #6c5ce7, stop:1 #8b7cf0);
            border: none;
            color: white;
            font-size: 11pt;
            font-weight: 600;
            padding: 10px 28px;
            border-radius: 8px;
        }

        QPushButton#syncButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #7b6bf0, stop:1 #9b8ff8);
        }

        QPushButton#syncButton:disabled {
            background: #333348;
            color: #666680;
        }

        /* ── Table ── */
        QTableWidget {
            background-color: #1a1a24;
            border: 1px solid #2d2d3d;
            border-radius: 8px;
            gridline-color: #22222e;
            selection-background-color: #2d2d3d;
        }

        QTableWidget::item {
            padding: 6px 8px;
            border: none;
        }

        QTableWidget::item:selected {
            background: #2d2d3d;
        }

        QHeaderView::section {
            background-color: #22222e;
            color: #8888a0;
            border: none;
            border-bottom: 1px solid #2d2d3d;
            border-right: 1px solid #2d2d3d;
            padding: 6px 8px;
            font-weight: 600;
            font-size: 9pt;
            text-transform: uppercase;
        }

        /* ── Scrollbars ── */
        QScrollBar:vertical {
            background: transparent;
            width: 8px;
            margin: 0;
        }

        QScrollBar::handle:vertical {
            background: #2d2d3d;
            border-radius: 4px;
            min-height: 30px;
        }

        QScrollBar::handle:vertical:hover {
            background: #3d3d50;
        }

        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0;
        }

        QScrollBar:horizontal {
            background: transparent;
            height: 8px;
        }

        QScrollBar::handle:horizontal {
            background: #2d2d3d;
            border-radius: 4px;
            min-width: 30px;
        }

        /* ── Progress Bar ── */
        QProgressBar {
            background-color: #1a1a24;
            border: 1px solid #2d2d3d;
            border-radius: 6px;
            text-align: center;
            color: #e8e8f0;
            font-size: 9pt;
            height: 20px;
        }

        QProgressBar::chunk {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #6c5ce7, stop:1 #28c840);
            border-radius: 5px;
        }

        /* ── Combo Box ── */
        QComboBox {
            background-color: #22222e;
            border: 1px solid #2d2d3d;
            border-radius: 6px;
            padding: 6px 10px;
            color: #e8e8f0;
            min-width: 80px;
        }

        QComboBox:hover {
            border-color: #6c5ce7;
        }

        QComboBox::drop-down {
            border: none;
            width: 24px;
        }

        QComboBox QAbstractItemView {
            background: #1a1a24;
            border: 1px solid #2d2d3d;
            selection-background-color: #6c5ce7;
        }

        /* ── Status Bar ── */
        QStatusBar {
            background-color: #0f0f14;
            border-top: 1px solid #2d2d3d;
            color: #8888a0;
            font-size: 9pt;
        }

        /* ── Labels ── */
        QLabel {
            background: transparent;
            color: #e8e8f0;
        }

        QLabel#dimLabel {
            color: #8888a0;
        }

        /* ── Frames ── */
        QFrame#card {
            background-color: #1a1a24;
            border: 1px solid #2d2d3d;
            border-radius: 8px;
        }

        /* ── Spin Box ── */
        QSpinBox {
            background-color: #22222e;
            border: 1px solid #2d2d3d;
            border-radius: 6px;
            padding: 4px 8px;
            color: #e8e8f0;
        }

        QSpinBox:hover {
            border-color: #6c5ce7;
        }

        /* ── Tooltips ── */
        QToolTip {
            background-color: #22222e;
            color: #e8e8f0;
            border: 1px solid #2d2d3d;
            border-radius: 4px;
            padding: 4px 8px;
        }
    )");
}

} // namespace theme
} // namespace asynk
