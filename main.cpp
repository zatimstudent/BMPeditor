#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[]) {
    // Vytvoření instance QApplication
    QApplication app(argc, argv);

    // Vytvoření instance MainWindow
    MainWindow window;

    // Zobrazení okna
    window.show();

    // Spuštění hlavní smyčky aplikace
    return app.exec();
}
