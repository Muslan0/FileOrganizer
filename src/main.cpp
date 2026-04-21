/**
 * @file main.cpp
 * @brief Punkt wejścia aplikacji FileOrganizer AI.
 *
 * Inicjalizuje aplikację Qt, ustawia metadane i wyświetla okno główne.
 */

#include <QApplication>
#include <QIcon>
#include "MainWindow.h"

/**
 * @brief Funkcja main – punkt wejścia programu.
 * @param argc Liczba argumentów wiersza poleceń.
 * @param argv Argumenty wiersza poleceń.
 * @return Kod wyjścia aplikacji.
 */
int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("FileOrganizer AI");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("JPO-2026");

    MainWindow window;
    window.show();

    return app.exec();
}