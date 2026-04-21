#pragma once

#include <QMainWindow>
#include <QLineEdit>
#include <QComboBox>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QProgressBar>
#include <QGroupBox>
#include <QThread>

#include "OllamaClient.h"
#include "ScriptRunner.h"
#include "PromptBuilder.h"

/**
 * @file MainWindow.h
 * @brief Główne okno aplikacji FileOrganizer.
 *
 * Zawiera formularz do wprowadzania parametrów organizacji plików,
 * pole podglądu wygenerowanego skryptu Python oraz konsolę wyników.
 */

/**
 * @class MainWindow
 * @brief Główne okno GUI aplikacji.
 *
 * Odpowiada za:
 * - prezentację formularza parametrów,
 * - komunikację z OllamaClient (w osobnym wątku),
 * - wyświetlanie wygenerowanego kodu Python,
 * - uruchamianie kodu przez ScriptRunner,
 * - wyświetlanie wyników i błędów.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief Konstruktor.
     * @param parent Rodzic Qt (opcjonalny).
     */
    explicit MainWindow(QWidget* parent = nullptr);

    /** @brief Destruktor – kończy wątek roboczy. */
    ~MainWindow();

private slots:
    /** @brief Slot uruchamiający generację skryptu przez model. */
    void onGenerateClicked();

    /** @brief Slot uruchamiający wygenerowany skrypt Python. */
    void onRunScriptClicked();

    /** @brief Slot otwierający dialog wyboru katalogu źródłowego. */
    void onBrowseSource();

    /** @brief Slot otwierający dialog wyboru katalogu docelowego. */
    void onBrowseTarget();

    /** @brief Obsługuje odpowiedź modelu Ollama. */
    void onOllamaResponse(const QString& response);

    /** @brief Obsługuje błąd komunikacji z Ollama. */
    void onOllamaError(const QString& error);

    /** @brief Obsługuje zakończenie skryptu Python. */
    void onScriptFinished(const QString& output);

    /** @brief Obsługuje błąd skryptu Python. */
    void onScriptError(const QString& error);

    /** @brief Aktualizuje konsolę o nową linię stdout. */
    void onOutputLine(const QString& line);

    /** @brief Aktualizuje widoczność spinnera ładowania. */
    void onLoadingChanged(bool loading);

private:
    // ── Sekcja: parametry ────────────────────────────────────────────────────
    QLineEdit*  m_sourceEdit;        ///< Katalog źródłowy
    QLineEdit*  m_targetEdit;        ///< Katalog docelowy
    QComboBox*  m_sortByCombo;       ///< Kryterium sortowania
    QComboBox*  m_actionCombo;       ///< Akcja (move/copy/rename/archive)
    QLineEdit*  m_filterEdit;        ///< Filtr rozszerzeń
    QLineEdit*  m_renamePatternEdit; ///< Wzorzec zmiany nazw
    QLineEdit*  m_modelEdit;         ///< Nazwa modelu Ollama


    // ── Sekcja: skrypt ───────────────────────────────────────────────────────
    QTextEdit*  m_scriptEdit;        ///< Podgląd wygenerowanego kodu

    // ── Sekcja: konsola ──────────────────────────────────────────────────────
    QTextEdit*  m_consoleEdit;       ///< Wyjście skryptu (stdout/stderr)

    // ── Przyciski i status ───────────────────────────────────────────────────
    QPushButton*  m_generateBtn;     ///< Przycisk "Generuj skrypt"
    QPushButton*  m_runBtn;          ///< Przycisk "Uruchom skrypt"
    QPushButton*  m_stopBtn;         ///< Przycisk "Zatrzymaj"
    QLabel*       m_statusLabel;     ///< Pasek statusu
    QProgressBar* m_progressBar;     ///< Pasek postępu (nieskończony)

    // ── Logika ──────────────────────────────────────────────────────────────
    OllamaClient* m_ollamaClient;    ///< Klient Ollama
    ScriptRunner* m_scriptRunner;    ///< Uruchamiacz skryptów
    QThread*      m_workerThread;    ///< Wątek dla OllamaClient

    /** @brief Buduje i układa widżety w oknie. */
    void setupUi();

    /** @brief Podłącza sygnały i sloty. */
    void connectSignals();

    /** @brief Zbiera parametry z formularza do struktury OrganizeParams. */
    OrganizeParams collectParams() const;

    /** @brief Wyświetla komunikat w konsoli z kolorem. */
    void appendConsole(const QString& text, const QString& color = "#e0e0e0");

    /** @brief Ustawia stan aktywności przycisków. */
    void setButtonsEnabled(bool generateEnabled, bool runEnabled, bool stopEnabled);
};