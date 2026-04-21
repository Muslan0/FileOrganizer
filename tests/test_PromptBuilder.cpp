/**
 * @file test_PromptBuilder.cpp
 * @brief Testy jednostkowe dla klasy PromptBuilder.
 *
 * Weryfikuje poprawność budowania promptów, walidacji parametrów
 * i ekstrakcji kodu Python z odpowiedzi modelu.
 */

#include <QtTest/QtTest>
#include "PromptBuilder.h"

/**
 * @class TestPromptBuilder
 * @brief Klasa testowa dla PromptBuilder.
 */
class TestPromptBuilder : public QObject
{
    Q_OBJECT

private slots:
    /** @brief Test: system prompt nie jest pusty i zawiera kluczowe słowa. */
    void testSystemPromptNotEmpty()
    {
        QString sp = PromptBuilder::buildSystemPrompt();
        QVERIFY(!sp.isEmpty());
        QVERIFY(sp.contains("Python", Qt::CaseInsensitive));
        QVERIFY(sp.contains("```python", Qt::CaseInsensitive));
    }

    /** @brief Test: user prompt zawiera katalog źródłowy. */
    void testUserPromptContainsSourceDir()
    {
        OrganizeParams p;
        p.sourceDir = "C:\\Test\\Folder";
        p.action    = "move";
        p.sortBy    = "extension";
        QString up = PromptBuilder::buildUserPrompt(p);
        QVERIFY(up.contains("C:\\Test\\Folder"));
        QVERIFY(up.contains("move"));
        QVERIFY(up.contains("extension"));
    }

    /** @brief Test: user prompt zawiera wzorzec zmiany nazw gdy podany. */
    void testUserPromptRenamePattern()
    {
        OrganizeParams p;
        p.sourceDir     = "C:\\Test";
        p.action        = "rename";
        p.sortBy        = "name";
        p.renamePattern = "{date}_{index}_{name}";
        QString up = PromptBuilder::buildUserPrompt(p);
        QVERIFY(up.contains("{date}_{index}_{name}"));
    }

    /** @brief Test: walidacja odrzuca pusty katalog źródłowy. */
    void testValidationRejectsEmptySource()
    {
        OrganizeParams p;
        p.sourceDir = "";
        p.action    = "move";
        p.sortBy    = "extension";
        QString err;
        QVERIFY(!PromptBuilder::validate(p, err));
        QVERIFY(!err.isEmpty());
    }

    /** @brief Test: walidacja odrzuca brak akcji. */
    void testValidationRejectsEmptyAction()
    {
        OrganizeParams p;
        p.sourceDir = "C:\\Test";
        p.action    = "";
        p.sortBy    = "extension";
        QString err;
        QVERIFY(!PromptBuilder::validate(p, err));
    }

    /** @brief Test: walidacja odrzuca rename bez wzorca. */
    void testValidationRejectsRenameWithoutPattern()
    {
        OrganizeParams p;
        p.sourceDir     = "C:\\Test";
        p.action        = "rename";
        p.sortBy        = "name";
        p.renamePattern = "";
        QString err;
        QVERIFY(!PromptBuilder::validate(p, err));
    }

    /** @brief Test: walidacja akceptuje kompletne parametry. */
    void testValidationAcceptsValidParams()
    {
        OrganizeParams p;
        p.sourceDir = "C:\\Test";
        p.action    = "move";
        p.sortBy    = "extension";
        QString err;
        QVERIFY(PromptBuilder::validate(p, err));
        QVERIFY(err.isEmpty());
    }

    /** @brief Test: ekstrakcja kodu z bloku markdown python. */
    void testExtractPythonCodeFromMarkdown()
    {
        QString raw = "Oto kod:\n```python\nimport os\nprint('hello')\n```\nKoniec.";
        QString code = PromptBuilder::extractPythonCode(raw);
        QVERIFY(code.contains("import os"));
        QVERIFY(code.contains("print('hello')"));
        QVERIFY(!code.contains("Oto kod:"));
        QVERIFY(!code.contains("Koniec."));
        QVERIFY(!code.contains("```"));
    }

    /** @brief Test: ekstrakcja kodu z bloku bez etykiety języka. */
    void testExtractPythonCodeFromMarkdownNoLabel()
    {
        QString raw = "```\nimport shutil\n```";
        QString code = PromptBuilder::extractPythonCode(raw);
        QVERIFY(code.contains("import shutil"));
    }

    /** @brief Test: ekstrakcja kodu gdy brak bloków markdown. */
    void testExtractPythonCodeNoMarkdown()
    {
        QString raw = "import os\nprint('test')";
        QString code = PromptBuilder::extractPythonCode(raw);
        QVERIFY(code.contains("import os"));
    }

    /** @brief Test: pusty prompt zwraca pusty kod. */
    void testExtractEmptyResponse()
    {
        QString code = PromptBuilder::extractPythonCode("");
        QVERIFY(code.isEmpty());
    }
};

QTEST_MAIN(TestPromptBuilder)
#include "test_PromptBuilder.moc"