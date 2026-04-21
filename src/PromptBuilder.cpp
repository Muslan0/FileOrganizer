#include "PromptBuilder.h"
#include <QRegularExpression>

/**
 * @file PromptBuilder.cpp
 * @brief Implementacja budowania promptów dla modelu Ollama.
 */

QString PromptBuilder::buildSystemPrompt()
{
    return R"(You are a Python file automation expert. Generate only Python scripts.

STRICT OUTPUT FORMAT - follow exactly:
- Your response MUST start with: ```python
- Your response MUST end with: ```
- NO text before ```python
- NO text after the closing ```
- NO explanations, NO descriptions, ONLY the code block

Example of correct response format:
```python
import os
print("done")
```

SCRIPT RULES:
1. Use only Python standard library: os, shutil, pathlib, zipfile, datetime, sys
2. Handle exceptions with try/except, print errors to stderr
3. Print a summary at the end (how many files processed)
4. Use Windows path separators (\)
5. Never permanently delete files (use archive or move instead of os.remove)
6. If target directory differs from source, create it with os.makedirs
)";
}

QString PromptBuilder::buildUserPrompt(const OrganizeParams& params)
{
    QString prompt;
    prompt += QString("Katalog źródłowy: %1\n").arg(params.sourceDir);

    if (!params.targetDir.isEmpty() && params.targetDir != params.sourceDir) {
        prompt += QString("Katalog docelowy: %1\n").arg(params.targetDir);
    } else {
        prompt += "Katalog docelowy: taki sam jak źródłowy\n";
    }

    prompt += QString("Akcja: %1\n").arg(params.action);
    prompt += QString("Sortowanie/grupowanie według: %1\n").arg(params.sortBy);

    if (!params.filterExtensions.isEmpty()) {
        prompt += QString("Filtruj tylko pliki o rozszerzeniach: %1\n")
        .arg(params.filterExtensions);
    } else {
        prompt += "Filtr rozszerzeń: wszystkie pliki\n";
    }

    if (params.action == "rename" && !params.renamePattern.isEmpty()) {
        prompt += QString("Rename pattern: %1\n").arg(params.renamePattern);
        prompt += "The pattern uses these placeholders - replace them manually in code using str.replace():\n"
                  "  {name}  -> file stem (filename without extension)\n"
                  "  {ext}   -> file extension including dot, e.g. .jpg\n"
                  "  {date}  -> file modification date: datetime.fromtimestamp(f.stat().st_mtime).strftime('%Y%m%d')\n"
                  "  {index} -> zero-padded counter: str(i+1).zfill(3)\n"
                  "  {size}  -> file size in KB: str(f.stat().st_size // 1024) + 'KB'\n"
                  "IMPORTANT: Do NOT use str.format() or f-strings for the pattern.\n"
                  "Instead use: new_name = pattern.replace('{name}', stem).replace('{date}', date).replace('{index}', index).replace('{ext}', ext)\n";
    }

    if (!params.extraInstructions.isEmpty()) {
        prompt += QString("Dodatkowe wymagania: %1\n").arg(params.extraInstructions);
    }

    prompt += "\nWygeneruj kompletny skrypt Python realizujący powyższe zadanie.";
    return prompt;
}

bool PromptBuilder::validate(const OrganizeParams& params, QString& errorMsg)
{
    if (params.sourceDir.trimmed().isEmpty()) {
        errorMsg = "Podaj katalog źródłowy.";
        return false;
    }
    if (params.action.trimmed().isEmpty()) {
        errorMsg = "Wybierz akcję do wykonania.";
        return false;
    }
    if (params.sortBy.trimmed().isEmpty()) {
        errorMsg = "Wybierz kryterium sortowania/grupowania.";
        return false;
    }
    if (params.action == "rename" && params.renamePattern.trimmed().isEmpty()) {
        errorMsg = "Dla akcji 'zmień nazwy' podaj wzorzec nazwy.";
        return false;
    }
    return true;
}

QString PromptBuilder::extractPythonCode(const QString& rawResponse)
{
    // Przypadek 1: blok ```python ... ``` lub ``` ... ```
    QRegularExpression re("```(?:python)?\\s*([\\s\\S]*?)```",
                          QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch match = re.match(rawResponse);
    if (match.hasMatch()) {
        return match.captured(1).trimmed();
    }

    // Przypadek 2: brak backtick-ów – szukamy pierwszej linii z kodem Pythona.
    // Pomijamy etykiety takie jak "python", "Python:", puste linie i zdania opisowe.
    QString clean = rawResponse.trimmed();
    QStringList lines = clean.split('\n');
    int startLine = 0;
    for (int i = 0; i < lines.size(); ++i) {
        QString l = lines[i].trimmed();

        // Pomiń pustą linię, samą etykietę "python"/"Python" lub zdanie opisowe
        if (l.isEmpty()) continue;
        if (l.compare("python", Qt::CaseInsensitive) == 0) continue;
        if (l.startsWith("python:", Qt::CaseInsensitive)) continue;
        if (l.startsWith("here", Qt::CaseInsensitive)) continue;
        if (l.startsWith("sure", Qt::CaseInsensitive)) continue;
        if (l.startsWith("of course", Qt::CaseInsensitive)) continue;
        if (l.startsWith("below", Qt::CaseInsensitive)) continue;

        // Pierwsza linia która wygląda jak kod Pythona
        if (l.startsWith("import ") || l.startsWith("from ") ||
            l.startsWith("def ") || l.startsWith("class ") ||
            l.startsWith("#!") || l.startsWith("# ") ||
            l.startsWith("import os") || l.startsWith("pathlib") ||
            l.startsWith("os.") || l.startsWith("shutil.")) {
            startLine = i;
            break;
        }
    }
    return lines.mid(startLine).join('\n').trimmed();
}