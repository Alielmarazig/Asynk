/**
 * main.cpp
 *
 * asynk — Multi-track audio/video synchronization tool.
 * C++ / Qt6 native desktop application.
 */

#include <QApplication>
#include <QFont>
#include "ui/main_window.h"
#include "ui/theme.h"

int main(int argc, char* argv[]) {
    // Dark title bar on Windows 11
#ifdef Q_OS_WIN
    qputenv("QT_QPA_PLATFORM", "windows:darkmode=2");
#endif

    QApplication app(argc, argv);
    app.setApplicationName("asynk");
    app.setApplicationVersion("0.3.0");
    app.setOrganizationName("asynk");

    // Apply global theme
    app.setStyleSheet(asynk::theme::globalStylesheet());

    QFont defaultFont = asynk::theme::bodyFont();
    app.setFont(defaultFont);

    asynk::MainWindow window;
    window.show();

    return app.exec();
}
