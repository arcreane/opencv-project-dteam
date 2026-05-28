#include <QApplication>
#include "MainWindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("MyEditor");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("ISEP");
    app.setStyle("Fusion");
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window,          QColor(45,45,45));
    darkPalette.setColor(QPalette::WindowText,       Qt::white);
    darkPalette.setColor(QPalette::Base,             QColor(30,30,30));
    darkPalette.setColor(QPalette::AlternateBase,    QColor(45,45,45));
    darkPalette.setColor(QPalette::Text,             Qt::white);
    darkPalette.setColor(QPalette::Button,           QColor(55,55,55));
    darkPalette.setColor(QPalette::ButtonText,       Qt::white);
    darkPalette.setColor(QPalette::Highlight,        QColor(42,130,218));
    darkPalette.setColor(QPalette::HighlightedText,  Qt::black);
    app.setPalette(darkPalette);
    MainWindow window;
    window.show();
    return app.exec();
}
