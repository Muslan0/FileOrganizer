#pragma once

#include <QObject>
#include <QProcess>
#include <QString>

/**
 * @file ScriptRunner.h
 * @brief Uruchamianie wygenerowanych skryptów Python w osobnym wątku procesu.
 *
 * Klasa zapisuje skrypt do tymczasowego pliku .py, a następnie uruchamia
 * interpreter Pythona i zbiera stdout/stderr asynchronicznie.
 */

/**
 * @class ScriptRunner
 * @brief Uruchamia skrypty Python i raportuje wyniki przez sygnały Qt.
 *
 * Używa QProcess do uruchomienia skryptu bez blokowania UI.
 * Wyniki (stdout, stderr, kod wyjścia) są przekazywane przez sygnały.
 */
class ScriptRunner : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Konstruktor.
     * @param parent Rodzic Qt (opcjonalny).
     */
    explicit ScriptRunner(QObject* parent = nullptr);

    /**
     * @brief Destruktor – zatrzymuje proces jeśli działa.
     */
    ~ScriptRunner();

    /**
     * @brief Zapisuje kod do pliku tymczasowego i uruchamia Python.
     * @param pythonCode Kod Python do wykonania.
     */
    void runScript(const QString& pythonCode);

    /**
     * @brief Sprawdza czy interpreter Pythona jest dostępny w PATH.
     * @return true jeśli python/python3 jest osiągalny.
     */
    static bool isPythonAvailable();

    /**
     * @brief Zatrzymuje uruchomiony proces (jeśli trwa).
     */
    void stopScript();

    /**
     * @brief Zwraca ścieżkę ostatnio zapisanego pliku tymczasowego.
     * @return Ścieżka do pliku .py lub pusty string.
     */
    QString lastScriptPath() const;

signals:
    /** @brief Emitowany gdy skrypt zakończy działanie z sukcesem. */
    void scriptFinished(const QString& output);

    /** @brief Emitowany gdy skrypt zakończy z błędem (stderr lub exit code != 0). */
    void scriptError(const QString& errorOutput);

    /** @brief Emitowany gdy skrypt startuje. */
    void scriptStarted();

    /** @brief Emitowany na bieżąco gdy pojawiają się nowe dane na stdout. */
    void outputLine(const QString& line);

private slots:
    void onProcessFinished(int exitCode, QProcess::ExitStatus status);
    void onReadyReadStandardOutput();
    void onReadyReadStandardError();

private:
    QProcess*  m_process;      ///< Proces interpretera Python
    QString    m_scriptPath;   ///< Ścieżka tymczasowego pliku skryptu
    QString    m_stdoutBuffer; ///< Bufor stdout
    QString    m_stderrBuffer; ///< Bufor stderr

    /** @brief Tworzy unikalną ścieżkę tymczasowego pliku .py */
    static QString tempScriptPath();
};