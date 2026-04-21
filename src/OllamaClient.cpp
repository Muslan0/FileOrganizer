#include "OllamaClient.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>
#include <QTimer>

/**
 * @file OllamaClient.cpp
 * @brief Implementacja klienta REST dla Ollama.
 */

OllamaClient::OllamaClient(QObject* parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_currentReply(nullptr)
{
    connect(m_networkManager, &QNetworkAccessManager::finished,
            this, &OllamaClient::onReplyFinished);
}

void OllamaClient::sendRequest(const QString& model,
                               const QString& system,
                               const QString& prompt)
{
    emit loadingChanged(true);

    QUrl url(QString("%1/api/generate").arg(BASE_URL));
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    // Timeout 120 sekund (duże modele mogą być wolne)
    request.setTransferTimeout(120000);

    QJsonObject body;
    body["model"]  = model;
    body["system"] = system;
    body["prompt"] = prompt;
    body["stream"] = false;

    QJsonDocument doc(body);
    m_currentReply = m_networkManager->post(request, doc.toJson());
}

bool OllamaClient::isServiceAvailable()
{
    // Synchroniczne sprawdzenie – używane tylko przy starcie aplikacji
    QNetworkAccessManager checker;
    QNetworkRequest req(QUrl(QString("%1/api/tags").arg(BASE_URL)));
    req.setTransferTimeout(3000);

    QNetworkReply* reply = checker.get(req);

    // Prosta pętla oczekiwania (max 3s)
    QTimer timer;
    timer.setSingleShot(true);
    QEventLoop loop;
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    timer.start(3000);
    loop.exec();

    bool ok = (reply->error() == QNetworkReply::NoError);
    reply->deleteLater();
    return ok;
}

void OllamaClient::cancelRequest()
{
    if (m_currentReply && m_currentReply->isRunning()) {
        m_currentReply->abort();
        m_currentReply = nullptr;
        emit loadingChanged(false);
        emit errorOccurred("Generowanie anulowane przez użytkownika.");
    }
}

void OllamaClient::onReplyFinished(QNetworkReply* reply)
{
    emit loadingChanged(false);

    if (reply->error() != QNetworkReply::NoError) {
        QString errMsg;
        if (reply->error() == QNetworkReply::ConnectionRefusedError) {
            errMsg = "Nie można połączyć się z Ollama.\n"
                     "Upewnij się, że serwis jest uruchomiony: ollama serve\n"
                     "Adres: http://localhost:11434";
        } else if (reply->error() == QNetworkReply::OperationCanceledError) {
            errMsg = "Przekroczono limit czasu oczekiwania na odpowiedź modelu.\n"
                     "Spróbuj z mniejszym modelem lub uprość zapytanie.";
        } else {
            errMsg = QString("Błąd sieciowy: %1").arg(reply->errorString());
        }
        emit errorOccurred(errMsg);
        reply->deleteLater();
        return;
    }

    QByteArray data = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);

    if (doc.isNull() || !doc.isObject()) {
        emit errorOccurred("Błąd: nieprawidłowa odpowiedź JSON od Ollama.");
        reply->deleteLater();
        return;
    }

    QJsonObject obj = doc.object();

    // Sprawdź czy model istnieje
    if (obj.contains("error")) {
        emit errorOccurred(QString("Błąd Ollama: %1").arg(obj["error"].toString()));
        reply->deleteLater();
        return;
    }

    QString response = obj["response"].toString();
    if (response.isEmpty()) {
        emit errorOccurred("Błąd: model zwrócił pustą odpowiedź.");
        reply->deleteLater();
        return;
    }

    emit responseReady(response);
    reply->deleteLater();
}