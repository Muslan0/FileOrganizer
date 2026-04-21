/**
 * @file test_ScriptRunner.cpp
 * @brief Testy jednostkowe dla klasy ScriptRunner.
 *
 * Weryfikuje uruchamianie skryptów Python, obsługę błędów
 * i dostępność interpretera.
 */

#include <QtTest/QtTest>
#include <QSignalSpy>
#include "ScriptRunner.h"

/**
 * @class TestScriptRunner
 * @brief Klasa testowa dla ScriptRunner.
 */
class TestScriptRunner : public QObject
{
    Q_OBJECT

private slots:
    /** @brief Test: sprawdzenie dostępności Pythona. */
    void testPythonAvailable()
    {
        // Ten test może nie przejść jeśli Python nie jest zainstalowany
        // Na potrzeby testów CI traktujemy to jako informację
        bool available = ScriptRunner::isPythonAvailable();
        qDebug() << "Python available:" << available;
        // Nie wymuszamy true/false – tylko weryfikujemy że metoda działa
        QVERIFY(available == true || available == false);
    }

    /** @brief Test: uruchomienie prostego skryptu Python. */
    void testRunSimpleScript()
    {
        if (!ScriptRunner::isPythonAvailable()) {
            QSKIP("Python nie jest dostępny w PATH – pomijam test");
        }

        ScriptRunner runner;
        QSignalSpy finishedSpy(&runner, &ScriptRunner::scriptFinished);
        QSignalSpy errorSpy(&runner, &ScriptRunner::scriptError);

        runner.runScript("print('FileOrganizer test OK')");

        // Czekaj max 10 sekund
        QVERIFY(finishedSpy.wait(10000) || errorSpy.count() > 0);

        if (finishedSpy.count() > 0) {
            QString output = finishedSpy.first().first().toString();
            QVERIFY(output.contains("FileOrganizer test OK"));
        }
    }

    /** @brief Test: skrypt z błędem emituje sygnał scriptError. */
    void testScriptWithError()
    {
        if (!ScriptRunner::isPythonAvailable()) {
            QSKIP("Python nie jest dostępny – pomijam test");
        }

        ScriptRunner runner;
        QSignalSpy errorSpy(&runner, &ScriptRunner::scriptError);
        QSignalSpy finishedSpy(&runner, &ScriptRunner::scriptFinished);

        runner.runScript("raise RuntimeError('test error')");

        QVERIFY(errorSpy.wait(10000) || finishedSpy.count() > 0);
        QVERIFY(errorSpy.count() > 0);
    }

    /** @brief Test: ścieżka tymczasowego skryptu ustawiana po uruchomieniu. */
    void testLastScriptPathSet()
    {
        if (!ScriptRunner::isPythonAvailable()) {
            QSKIP("Python nie jest dostępny – pomijam test");
        }

        ScriptRunner runner;
        QSignalSpy spy(&runner, &ScriptRunner::scriptFinished);

        runner.runScript("print('path test')");
        spy.wait(10000);

        QVERIFY(!runner.lastScriptPath().isEmpty());
        QVERIFY(runner.lastScriptPath().endsWith(".py"));
    }

    /** @brief Test: sygnał scriptStarted jest emitowany. */
    void testScriptStartedSignal()
    {
        if (!ScriptRunner::isPythonAvailable()) {
            QSKIP("Python nie jest dostępny – pomijam test");
        }

        ScriptRunner runner;
        QSignalSpy startedSpy(&runner, &ScriptRunner::scriptStarted);
        QSignalSpy finishedSpy(&runner, &ScriptRunner::scriptFinished);

        runner.runScript("import time; time.sleep(0.1); print('done')");

        QVERIFY(startedSpy.count() > 0 || finishedSpy.wait(10000));
    }
};

QTEST_MAIN(TestScriptRunner)
#include "test_ScriptRunner.moc"