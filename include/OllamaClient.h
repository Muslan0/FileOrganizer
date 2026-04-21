#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QString>
#include <functional>

/**
 * @file OllamaClient.h
 * @brief Klient HTTP komunikujący się z lokalnym serwerem Ollama przez REST API.
 *
 * Klasa obsługuje wysyłanie zapytań do endpointu /api/generate oraz
 * odbieranie odpowiedzi w trybie niestrumieniowym (stream: false).
 */

/**
 * @class OllamaClient
 * @brief Asynchroniczny klient REST dla Ollama.
 *
 * Wysyła zapytania POST do http://localhost:11434/api/generate
 * i emituje sygnały po zakończeniu lub błędzie.
 */
class OllamaClient : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Konstruktor.
     * @param parent Rodzic Qt (opcjonalny).
     */
    explicit OllamaClient(QObject* parent = nullptr);

    /**
     * @brief Wysyła zapytanie do modelu Ollama.
     * @param model  Nazwa modelu, np. "llama3" lub "bielik".
     * @param system Prompt systemowy (rola / kontekst modelu).
     * @param prompt Zapytanie użytkownika.
     */
    void sendRequest(const QString& model,
                     const QString& system,
                     const QString& prompt);

    /**
     * @brief Sprawdza dostępność serwisu Ollama przez prosty GET.
     * @return true jeśli serwis odpowiada na localhost:11434.
     */
    bool isServiceAvailable();

    /**
     * @brief Anuluje bieżące zapytanie do modelu (jeśli trwa).
     */
    void cancelRequest();

signals:
    /** @brief Emitowany gdy model zwróci odpowiedź. */
    void responseReady(const QString& response);

    /** @brief Emitowany w przypadku błędu sieciowego lub odpowiedzi. */
    void errorOccurred(const QString& errorMessage);

    /** @brief Emitowany gdy żądanie jest w toku (true) lub zakończone (false). */
    void loadingChanged(bool loading);

private slots:
    void onReplyFinished(QNetworkReply* reply);

private:
    QNetworkAccessManager* m_networkManager; ///< Menedżer połączeń Qt
    QNetworkReply*         m_currentReply;   ///< Bieżące zapytanie (do anulowania)
    static constexpr const char* BASE_URL = "http://localhost:11434";
};