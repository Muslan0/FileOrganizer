#include "ScriptRunner.h"
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QDateTime>
#include <QStandardPaths>
#include <QCoreApplication>

/**
 * @file ScriptRunner.cpp
 * @brief Implementacja uruchamiania skryptów Python przez QProcess.
 */

ScriptRunner::ScriptRunner(QObject* parent)
    : QObject(parent)
    , m_process(new QProcess(this))
{
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &ScriptRunner::onProcessFinished);
    connect(m_process, &QProcess::readyReadStandardOutput,
            this, &ScriptRunner::onReadyReadStandardOutput);
    connect(m_process, &QProcess::readyReadStandardError,
            this, &ScriptRunner::onReadyReadStandardError);
}

ScriptRunner::~ScriptRunner()
{
    stopScript();
}

void ScriptRunner::runScript(const QString& pythonCode)
{
    if (m_process->state() != QProcess::NotRunning) {
        emit scriptError("Skrypt jest już uruchomiony. Poczekaj na zakończenie.");
        return;
    }

    m_stdoutBuffer.clear();
    m_stderrBuffer.clear();

    // Zapisz skrypt do pliku tymczasowego
    m_scriptPath = tempScriptPath();
    QFile file(m_scriptPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        emit scriptError(QString("Nie można zapisać skryptu do: %1").arg(m_scriptPath));
        return;
    }
    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);
    // Nagłówek kodowania dla Windows
    stream << "# -*- coding: utf-8 -*-\n";
    stream << pythonCode;
    file.close();

    emit scriptStarted();

    // Ustaw zmienne środowiskowe dla poprawnego kodowania na Windows
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("PYTHONIOENCODING", "utf-8");
    env.insert("PYTHONUNBUFFERED", "1");
    m_process->setProcessEnvironment(env);

    // Uruchom Python (Windows: python, Linux/Mac: python3)
#ifdef Q_OS_WIN
    m_process->start("python", QStringList() << m_scriptPath);
#else
    m_process->start("python3", QStringList() << m_scriptPath);
#endif

    if (!m_process->waitForStarted(5000)) {
        emit scriptError("Nie można uruchomić interpretera Python.\n"
                         "Upewnij się że Python jest zainstalowany i dostępny w PATH.");
    }
}

bool ScriptRunner::isPythonAvailable()
{
    QProcess checker;
#ifdef Q_OS_WIN
    checker.start("python", QStringList() << "--version");
#else
    checker.start("python3", QStringList() << "--version");
#endif
    checker.waitForFinished(3000);
    return checker.exitCode() == 0;
}

void ScriptRunner::stopScript()
{
    if (m_process->state() != QProcess::NotRunning) {
        m_process->kill();
        m_process->waitForFinished(3000);
    }
}

QString ScriptRunner::lastScriptPath() const
{
    return m_scriptPath;
}

void ScriptRunner::onProcessFinished(int exitCode, QProcess::ExitStatus /*status*/)
{
    // Odczytaj pozostałe dane z buforów
    onReadyReadStandardOutput();
    onReadyReadStandardError();

    if (exitCode == 0) {
        emit scriptFinished(m_stdoutBuffer);
    } else {
        QString combined = m_stdoutBuffer;
        if (!m_stderrBuffer.isEmpty()) {
            combined += "\n--- BŁĘDY ---\n" + m_stderrBuffer;
        }
        emit scriptError(combined);
    }
}

void ScriptRunner::onReadyReadStandardOutput()
{
    m_process->setReadChannel(QProcess::StandardOutput);
    while (m_process->canReadLine()) {
        QString line = QString::fromUtf8(m_process->readLine()).trimmed();
        m_stdoutBuffer += line + "\n";
        emit outputLine(line);
    }
}

void ScriptRunner::onReadyReadStandardError()
{
    m_process->setReadChannel(QProcess::StandardError);
    QByteArray data = m_process->readAllStandardError();
    if (!data.isEmpty()) {
        QString text = QString::fromUtf8(data);
        m_stderrBuffer += text;
    }
}

QString ScriptRunner::tempScriptPath()
{
    QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss_zzz");
    return QDir(tempDir).filePath(QString("fo_script_%1.py").arg(timestamp));
}