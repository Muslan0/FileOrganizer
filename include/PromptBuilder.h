#pragma once

#include <QString>

/**
 * @file PromptBuilder.h
 * @brief Budowanie promptów systemowych i użytkownika dla modelu Ollama.
 *
 * Oddziela logikę tworzenia treści zapytań od warstwy GUI i komunikacji sieciowej.
 */

/**
 * @struct OrganizeParams
 * @brief Parametry zadania organizacji plików podane przez użytkownika.
 */
struct OrganizeParams {
    QString sourceDir;        ///< Katalog źródłowy do organizacji
    QString targetDir;        ///< Katalog docelowy (może być taki sam jak źródłowy)
    QString sortBy;           ///< Kryterium sortowania: "extension", "date", "size", "name"
    QString action;           ///< Akcja: "move", "copy", "rename", "archive"
    QString renamePattern;    ///< Wzorzec zmiany nazw, np. "{date}_{name}"
    QString filterExtensions; ///< Rozszerzenia do filtrowania, np. "*.jpg *.png"
    QString extraInstructions;///< Dodatkowe uwagi/życzenia użytkownika
};

/**
 * @class PromptBuilder
 * @brief Fabryka promptów dla modelu językowego.
 *
 * Buduje system prompt oraz user prompt na podstawie parametrów użytkownika,
 * instruując model do generacji skryptu Python automatyzującego organizację plików.
 */
class PromptBuilder
{
public:
    /**
     * @brief Zwraca stały system prompt dla zadania organizacji plików.
     * @return Treść system promptu.
     */
    static QString buildSystemPrompt();

    /**
     * @brief Buduje user prompt na podstawie podanych parametrów.
     * @param params Parametry zadania wypełnione przez użytkownika.
     * @return Gotowy user prompt do wysłania do modelu.
     */
    static QString buildUserPrompt(const OrganizeParams& params);

    /**
     * @brief Waliduje parametry – sprawdza czy kluczowe pola nie są puste.
     * @param params Parametry do sprawdzenia.
     * @param errorMsg Wyjściowy komunikat błędu (jeśli walidacja nie przeszła).
     * @return true jeśli parametry są poprawne, false w przeciwnym razie.
     */
    static bool validate(const OrganizeParams& params, QString& errorMsg);

    /**
     * @brief Wyodrębnia czysty kod Python z odpowiedzi modelu.
     *
     * Model może zwrócić kod owinięty w markdown (```python ... ```).
     * Ta metoda wyciąga sam kod.
     *
     * @param rawResponse Surowa odpowiedź modelu.
     * @return Czysty kod Python gotowy do uruchomienia.
     */
    static QString extractPythonCode(const QString& rawResponse);
};