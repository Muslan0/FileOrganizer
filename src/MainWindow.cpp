#include "MainWindow.h"
#include "PromptBuilder.h"

#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QSplitter>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QThread>
#include <QFont>
#include <QSizePolicy>
#include <QScrollBar>

/**
 * @file MainWindow.cpp
 * @brief Implementacja głównego okna GUI aplikacji FileOrganizer.
 */

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_workerThread(new QThread(this))
    , m_ollamaClient(new OllamaClient)
    , m_scriptRunner(new ScriptRunner(this))
{
    setWindowTitle("FileOrganizer AI  |  powered by Ollama");
    setMinimumSize(1000, 780);
    resize(1200, 860);

    // Przenieś OllamaClient na osobny wątek (wielowątkowość)
    m_ollamaClient->moveToThread(m_workerThread);
    m_workerThread->start();

    setupUi();
    connectSignals();

    // Sprawdź dostępność Ollama przy starcie
    QMetaObject::invokeMethod(this, [this]() {
        if (!m_ollamaClient->isServiceAvailable()) {
            appendConsole("⚠  Ollama nie jest uruchomiona lub niedostępna na localhost:11434.", "#f4c542");
            appendConsole("   Uruchom: ollama serve", "#f4c542");
        } else {
            appendConsole("✓  Ollama dostępna na localhost:11434", "#4caf50");
        }
        if (!ScriptRunner::isPythonAvailable()) {
            appendConsole("⚠  Python nie znaleziony w PATH. Dodaj Python do zmiennej PATH.", "#f4c542");
        } else {
            appendConsole("✓  Python dostępny", "#4caf50");
        }
    }, Qt::QueuedConnection);
}

MainWindow::~MainWindow()
{
    m_workerThread->quit();
    m_workerThread->wait(3000);
    delete m_ollamaClient;
}

// ─────────────────────────────────────────────────────────────────────────────
//  setupUi
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::setupUi()
{
    // Ciemny motyw – styl przemysłowy / utility
    qApp->setStyle("Fusion");

    QPalette dark;
    dark.setColor(QPalette::Window,          QColor(0x1a1d23));
    dark.setColor(QPalette::WindowText,      QColor(0xe0e4ec));
    dark.setColor(QPalette::Base,            QColor(0x12141a));
    dark.setColor(QPalette::AlternateBase,   QColor(0x1f2330));
    dark.setColor(QPalette::ToolTipBase,     QColor(0x252a38));
    dark.setColor(QPalette::ToolTipText,     QColor(0xe0e4ec));
    dark.setColor(QPalette::Text,            QColor(0xe0e4ec));
    dark.setColor(QPalette::Button,          QColor(0x252a38));
    dark.setColor(QPalette::ButtonText,      QColor(0xe0e4ec));
    dark.setColor(QPalette::BrightText,      Qt::red);
    dark.setColor(QPalette::Link,            QColor(0x3d9cf0));
    dark.setColor(QPalette::Highlight,       QColor(0x3d6da8));
    dark.setColor(QPalette::HighlightedText, Qt::white);
    qApp->setPalette(dark);

    qApp->setStyleSheet(R"(
        QGroupBox {
            border: 1px solid #2e3446;
            border-radius: 6px;
            margin-top: 10px;
            font-weight: bold;
            color: #8ab4f8;
            font-size: 11px;
            letter-spacing: 1px;
            text-transform: uppercase;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top left;
            padding: 0 6px;
        }
        QLineEdit, QComboBox, QTextEdit {
            background: #12141a;
            border: 1px solid #2e3446;
            border-radius: 4px;
            padding: 4px 8px;
            color: #e0e4ec;
            selection-background-color: #3d6da8;
        }
        QLineEdit:focus, QComboBox:focus, QTextEdit:focus {
            border: 1px solid #3d9cf0;
        }
        QPushButton {
            background: #252a38;
            border: 1px solid #3a4260;
            border-radius: 5px;
            padding: 6px 18px;
            color: #c9d4f0;
            font-weight: bold;
        }
        QPushButton:hover { background: #2f3650; border-color: #5570b0; }
        QPushButton:pressed { background: #1e2236; }
        QPushButton:disabled { color: #4a5070; border-color: #252a38; }
        QPushButton#generateBtn {
            background: #1b3a6b;
            border-color: #3d7ae8;
            color: #9fc8ff;
            font-size: 13px;
        }
        QPushButton#generateBtn:hover { background: #2249888; }
        QPushButton#runBtn {
            background: #1b4a2e;
            border-color: #3da862;
            color: #82e0a0;
        }
        QPushButton#runBtn:hover { background: #20603a; }
        QPushButton#stopBtn {
            background: #4a1a1a;
            border-color: #a83d3d;
            color: #f08080;
        }
        QProgressBar {
            border: 1px solid #2e3446;
            border-radius: 4px;
            text-align: center;
            background: #12141a;
        }
        QProgressBar::chunk { background: #3d6da8; }
        QScrollBar:vertical { background: #12141a; width: 8px; }
        QScrollBar::handle:vertical { background: #2e3446; border-radius: 4px; min-height: 20px; }
        QSplitter::handle { background: #2e3446; }
    )");

    QWidget* central = new QWidget(this);
    setCentralWidget(central);
    QVBoxLayout* mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(12, 10, 12, 10);
    mainLayout->setSpacing(10);

    // ── Nagłówek ─────────────────────────────────────────────────────────────
    QLabel* titleLabel = new QLabel("⚙  FILE ORGANIZER AI", this);
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; "
                              "color: #8ab4f8; letter-spacing: 3px; "
                              "padding: 4px 0;");
    QLabel* subtitleLabel = new QLabel("Generuj i uruchamiaj skrypty Python do organizacji plików za pomocą lokalnego LLM", this);
    subtitleLabel->setStyleSheet("color: #5a6888; font-size: 11px;");
    mainLayout->addWidget(titleLabel);
    mainLayout->addWidget(subtitleLabel);

    // ── Splitter: formularz + skrypt + konsola ────────────────────────────────
    QSplitter* mainSplitter = new QSplitter(Qt::Vertical, this);
    mainSplitter->setChildrenCollapsible(false);
    mainLayout->addWidget(mainSplitter, 1);

    // --- Górna sekcja: formularz + model ---
    QWidget* topWidget = new QWidget;
    QHBoxLayout* topLayout = new QHBoxLayout(topWidget);
    topLayout->setContentsMargins(0, 0, 0, 0);
    topLayout->setSpacing(10);

    // Formularz parametrów
    QGroupBox* paramsGroup = new QGroupBox("Parametry organizacji");
    QGridLayout* paramsGrid = new QGridLayout(paramsGroup);
    paramsGrid->setSpacing(8);

    int row = 0;

    // Katalog źródłowy
    paramsGrid->addWidget(new QLabel("Katalog źródłowy:"), row, 0);
    m_sourceEdit = new QLineEdit;
    m_sourceEdit->setPlaceholderText("C:\\Users\\Jan\\Dokumenty");
    QPushButton* browseSourceBtn = new QPushButton("...");
    browseSourceBtn->setMaximumWidth(32);
    QHBoxLayout* srcLayout = new QHBoxLayout;
    srcLayout->addWidget(m_sourceEdit);
    srcLayout->addWidget(browseSourceBtn);
    paramsGrid->addLayout(srcLayout, row++, 1);

    // Katalog docelowy
    paramsGrid->addWidget(new QLabel("Katalog docelowy:"), row, 0);
    m_targetEdit = new QLineEdit;
    m_targetEdit->setPlaceholderText("(puste = taki sam jak źródłowy)");
    QPushButton* browseTargetBtn = new QPushButton("...");
    browseTargetBtn->setMaximumWidth(32);
    QHBoxLayout* dstLayout = new QHBoxLayout;
    dstLayout->addWidget(m_targetEdit);
    dstLayout->addWidget(browseTargetBtn);
    paramsGrid->addLayout(dstLayout, row++, 1);

    // Akcja
    paramsGrid->addWidget(new QLabel("Akcja:"), row, 0);
    m_actionCombo = new QComboBox;
    m_actionCombo->addItems({"move – przenieś pliki",
                             "copy – kopiuj pliki",
                             "rename – zmień nazwy",
                             "archive – spakuj do ZIP"});
    paramsGrid->addWidget(m_actionCombo, row++, 1);

    // Sortowanie/grupowanie
    paramsGrid->addWidget(new QLabel("Grupuj według:"), row, 0);
    m_sortByCombo = new QComboBox;
    m_sortByCombo->addItems({"extension – rozszerzenie pliku",
                             "date – data modyfikacji (rok/miesiąc)",
                             "size – rozmiar (małe/średnie/duże)",
                             "name – pierwsza litera nazwy"});
    paramsGrid->addWidget(m_sortByCombo, row++, 1);

    // Filtr rozszerzeń
    paramsGrid->addWidget(new QLabel("Filtr rozszerzeń:"), row, 0);
    m_filterEdit = new QLineEdit;
    m_filterEdit->setPlaceholderText("np. .jpg .png .pdf   (puste = wszystkie)");
    paramsGrid->addWidget(m_filterEdit, row++, 1);

    // Wzorzec zmiany nazw
    paramsGrid->addWidget(new QLabel("Wzorzec nazwy:"), row, 0);
    m_renamePatternEdit = new QLineEdit;
    m_renamePatternEdit->setPlaceholderText("{date}_{index}_{name}  (tylko dla akcji 'zmień nazwy')");
    paramsGrid->addWidget(m_renamePatternEdit, row++, 1);


    topLayout->addWidget(paramsGroup, 3);

    // Panel konfiguracji modelu + przyciski
    QGroupBox* modelGroup = new QGroupBox("Model i akcje");
    QVBoxLayout* modelLayout = new QVBoxLayout(modelGroup);

    modelLayout->addWidget(new QLabel("Model Ollama:"));
    m_modelEdit = new QLineEdit("llama3");
    m_modelEdit->setPlaceholderText("np. llama3, bielik, mistral");
    modelLayout->addWidget(m_modelEdit);

    modelLayout->addSpacing(8);

    m_generateBtn = new QPushButton("▶  Generuj skrypt");
    m_generateBtn->setObjectName("generateBtn");
    m_generateBtn->setMinimumHeight(42);
    m_generateBtn->setToolTip("Wyślij parametry do modelu LLM i wygeneruj skrypt Python");
    modelLayout->addWidget(m_generateBtn);

    m_runBtn = new QPushButton("⚡  Uruchom skrypt");
    m_runBtn->setObjectName("runBtn");
    m_runBtn->setMinimumHeight(36);
    m_runBtn->setEnabled(false);
    m_runBtn->setToolTip("Wykonaj wygenerowany skrypt Python");
    modelLayout->addWidget(m_runBtn);

    m_stopBtn = new QPushButton("■  Zatrzymaj");
    m_stopBtn->setObjectName("stopBtn");
    m_stopBtn->setEnabled(false);
    m_stopBtn->setToolTip("Przerwij działający skrypt");
    modelLayout->addWidget(m_stopBtn);

    modelLayout->addStretch();

    m_progressBar = new QProgressBar;
    m_progressBar->setRange(0, 0); // nieskończony
    m_progressBar->setVisible(false);
    m_progressBar->setMaximumHeight(6);
    modelLayout->addWidget(m_progressBar);

    m_statusLabel = new QLabel("Gotowy");
    m_statusLabel->setStyleSheet("color: #5a6888; font-size: 10px;");
    m_statusLabel->setAlignment(Qt::AlignCenter);
    modelLayout->addWidget(m_statusLabel);

    topLayout->addWidget(modelGroup, 1);

    mainSplitter->addWidget(topWidget);

    // --- Środkowa sekcja: podgląd skryptu ---
    QGroupBox* scriptGroup = new QGroupBox("Wygenerowany skrypt Python (edytowalny przed uruchomieniem)");
    QVBoxLayout* scriptLayout = new QVBoxLayout(scriptGroup);
    m_scriptEdit = new QTextEdit;
    m_scriptEdit->setFont(QFont("Consolas", 10));
    m_scriptEdit->setPlaceholderText("Tutaj pojawi się wygenerowany skrypt Python...");
    m_scriptEdit->setStyleSheet("background: #0d0f14; color: #a8d8a8; "
                                "border: 1px solid #1e2436;");
    scriptLayout->addWidget(m_scriptEdit);
    mainSplitter->addWidget(scriptGroup);

    // --- Dolna sekcja: konsola ---
    QGroupBox* consoleGroup = new QGroupBox("Konsola wyjściowa");
    QVBoxLayout* consoleLayout = new QVBoxLayout(consoleGroup);
    m_consoleEdit = new QTextEdit;
    m_consoleEdit->setReadOnly(true);
    m_consoleEdit->setFont(QFont("Consolas", 10));
    m_consoleEdit->setStyleSheet("background: #0a0c10; color: #e0e4ec; "
                                 "border: 1px solid #1e2436;");
    consoleLayout->addWidget(m_consoleEdit);
    mainSplitter->addWidget(consoleGroup);

    mainSplitter->setSizes({340, 240, 180});

    // Połącz browse-buttony
    connect(browseSourceBtn, &QPushButton::clicked, this, &MainWindow::onBrowseSource);
    connect(browseTargetBtn, &QPushButton::clicked, this, &MainWindow::onBrowseTarget);
}

// ─────────────────────────────────────────────────────────────────────────────
//  connectSignals
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::connectSignals()
{
    connect(m_generateBtn, &QPushButton::clicked, this, &MainWindow::onGenerateClicked);
    connect(m_runBtn,      &QPushButton::clicked, this, &MainWindow::onRunScriptClicked);
    connect(m_stopBtn, &QPushButton::clicked, this, [this]() {
        // Zatrzymaj generację Ollama (jeśli trwa)
        QMetaObject::invokeMethod(m_ollamaClient, [this]() {
            m_ollamaClient->cancelRequest();
        }, Qt::QueuedConnection);
        // Zatrzymaj skrypt Python (jeśli trwa)
        m_scriptRunner->stopScript();
        setButtonsEnabled(true, !m_scriptEdit->toPlainText().isEmpty(), false);
        m_statusLabel->setText("Anulowano");
    });

    connect(m_ollamaClient, &OllamaClient::responseReady,  this, &MainWindow::onOllamaResponse);
    connect(m_ollamaClient, &OllamaClient::errorOccurred,  this, &MainWindow::onOllamaError);
    connect(m_ollamaClient, &OllamaClient::loadingChanged, this, &MainWindow::onLoadingChanged);

    connect(m_scriptRunner, &ScriptRunner::scriptFinished, this, &MainWindow::onScriptFinished);
    connect(m_scriptRunner, &ScriptRunner::scriptError,    this, &MainWindow::onScriptError);
    connect(m_scriptRunner, &ScriptRunner::outputLine,     this, &MainWindow::onOutputLine);
    connect(m_scriptRunner, &ScriptRunner::scriptStarted,  this, [this]() {
        appendConsole("\n▶  Skrypt uruchomiony...", "#3d9cf0");
        setButtonsEnabled(false, false, true);
        m_statusLabel->setText("Skrypt działa...");
    });
}

// ─────────────────────────────────────────────────────────────────────────────
//  Sloty
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::onGenerateClicked()
{
    OrganizeParams params = collectParams();

    QString error;
    if (!PromptBuilder::validate(params, error)) {
        QMessageBox::warning(this, "Brak danych", error);
        return;
    }

    m_consoleEdit->clear();
    appendConsole("⏳  Wysyłam zapytanie do modelu " + m_modelEdit->text().trimmed() + "...", "#8ab4f8");

    QString systemPrompt = PromptBuilder::buildSystemPrompt();
    QString userPrompt   = PromptBuilder::buildUserPrompt(params);

    setButtonsEnabled(false, false, false);
    m_scriptEdit->clear();

    // Wywołanie przez Qt::QueuedConnection przez granicę wątku
    QMetaObject::invokeMethod(m_ollamaClient, [this, params, systemPrompt, userPrompt]() {
        m_ollamaClient->sendRequest(m_modelEdit->text().trimmed(), systemPrompt, userPrompt);
    }, Qt::QueuedConnection);
}

void MainWindow::onRunScriptClicked()
{
    QString code = m_scriptEdit->toPlainText().trimmed();
    if (code.isEmpty()) {
        QMessageBox::warning(this, "Brak skryptu",
                             "Najpierw wygeneruj skrypt klikając 'Generuj skrypt'.");
        return;
    }

    QMessageBox confirm(this);
    confirm.setWindowTitle("Potwierdzenie");
    confirm.setText("<b>Czy na pewno uruchomić skrypt?</b>");
    confirm.setInformativeText("Skrypt wykona operacje na plikach. "
                               "Upewnij się, że ścieżki są poprawne.");
    confirm.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    confirm.setDefaultButton(QMessageBox::No);
    confirm.setIcon(QMessageBox::Question);
    if (confirm.exec() != QMessageBox::Yes) return;

    m_consoleEdit->clear();
    m_scriptRunner->runScript(code);
}

void MainWindow::onBrowseSource()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Wybierz katalog źródłowy",
                                                    m_sourceEdit->text());
    if (!dir.isEmpty()) m_sourceEdit->setText(QDir::toNativeSeparators(dir));
}

void MainWindow::onBrowseTarget()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Wybierz katalog docelowy",
                                                    m_targetEdit->text());
    if (!dir.isEmpty()) m_targetEdit->setText(QDir::toNativeSeparators(dir));
}

void MainWindow::onOllamaResponse(const QString& response)
{
    appendConsole("✓  Odpowiedź otrzymana. Wyodrębnianie kodu...", "#4caf50");
    QString code = PromptBuilder::extractPythonCode(response);

    if (code.isEmpty()) {
        appendConsole("⚠  Nie udało się wyodrębnić kodu Python z odpowiedzi.", "#f4c542");
        appendConsole("Surowa odpowiedź:\n" + response, "#888");
        setButtonsEnabled(true, false, false);
        return;
    }

    m_scriptEdit->setPlainText(code);
    appendConsole("✓  Skrypt gotowy do uruchomienia.", "#4caf50");
    setButtonsEnabled(true, true, false);
    m_statusLabel->setText("Skrypt wygenerowany");
}

void MainWindow::onOllamaError(const QString& error)
{
    appendConsole("✗  Błąd: " + error, "#f08080");
    setButtonsEnabled(true, !m_scriptEdit->toPlainText().isEmpty(), false);
    m_statusLabel->setText("Błąd połączenia");
}

void MainWindow::onScriptFinished(const QString& output)
{
    if (!output.trimmed().isEmpty()) {
        appendConsole("\n── WYNIK ──────────────────────────────", "#3d9cf0");
        appendConsole(output.trimmed());
    }
    appendConsole("\n✓  Skrypt zakończony pomyślnie.", "#4caf50");
    setButtonsEnabled(true, true, false);
    m_statusLabel->setText("Skrypt zakończony");
}

void MainWindow::onScriptError(const QString& error)
{
    appendConsole("\n✗  Skrypt zakończył się z błędem:", "#f08080");
    appendConsole(error, "#f08080");
    setButtonsEnabled(true, true, false);
    m_statusLabel->setText("Błąd skryptu");
}

void MainWindow::onOutputLine(const QString& line)
{
    appendConsole(line);
}

void MainWindow::onLoadingChanged(bool loading)
{
    m_progressBar->setVisible(loading);
    if (loading) {
        m_statusLabel->setText("Oczekiwanie na model...");
        m_stopBtn->setEnabled(true);   // można anulować podczas generacji
    } else {
        m_stopBtn->setEnabled(false);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  Helpers
// ─────────────────────────────────────────────────────────────────────────────
OrganizeParams MainWindow::collectParams() const
{
    OrganizeParams p;
    p.sourceDir   = m_sourceEdit->text().trimmed();
    p.targetDir   = m_targetEdit->text().trimmed();
    p.extraInstructions = "";
    p.filterExtensions  = m_filterEdit->text().trimmed();
    p.renamePattern     = m_renamePatternEdit->text().trimmed();

    // Wyodrębnij sam klucz (przed " – ")
    QString actionText = m_actionCombo->currentText();
    p.action = actionText.split(" – ").first().trimmed();

    QString sortText = m_sortByCombo->currentText();
    p.sortBy = sortText.split(" – ").first().trimmed();

    return p;
}

void MainWindow::appendConsole(const QString& text, const QString& color)
{
    QString html = QString("<span style='color:%1;'>%2</span><br/>")
    .arg(color.isEmpty() ? "#e0e4ec" : color,
         text.toHtmlEscaped().replace("  ", "&nbsp;&nbsp;"));
    m_consoleEdit->insertHtml(html);
    m_consoleEdit->verticalScrollBar()->setValue(
        m_consoleEdit->verticalScrollBar()->maximum());
}

void MainWindow::setButtonsEnabled(bool generateEnabled, bool runEnabled, bool stopEnabled)
{
    m_generateBtn->setEnabled(generateEnabled);
    m_runBtn->setEnabled(runEnabled);
    m_stopBtn->setEnabled(stopEnabled);
}