================================================================================
  FileOrganizer AI – Projekt zaliczeniowy JPO 2025/2026
  Ocena docelowa: 5.0
================================================================================

OPIS APLIKACJI
--------------
FileOrganizer AI to aplikacja desktopowa napisana w C++/Qt, która przy pomocy
lokalnego modelu językowego (Ollama) generuje skrypty Python automatyzujące
organizację plików na dysku. Użytkownik wypełnia formularz (katalog źródłowy,
akcja, kryterium grupowania itp.), a model LLM generuje gotowy skrypt Python,
który aplikacja następnie wykonuje automatycznie.

Przykłady zadań:
  - Posortuj pliki w C:\Pobrane wg rozszerzenia (przenosząc do podkatalogów)
  - Zmień nazwy zdjęć w C:\Zdjęcia wg wzorca {date}_{index}_{name}
  - Spakuj do ZIP wszystkie pliki .docx z katalogu C:\Dokumenty
  - Skopiuj tylko pliki .mp3 z C:\Muzyka do D:\Backup, grupując wg roku

WYMAGANIA
---------
Oprogramowanie:
  - Windows 10/11 (lub Linux)
  - Qt 6.x (https://www.qt.io/download)
  - CMake >= 3.16
  - Kompilator C++17 (MSVC 2019+, GCC 11+, MinGW)
  - Python 3.x dostępny w PATH (https://www.python.org/)
  - Ollama (https://ollama.com/) + wybrany model językowy

Model językowy (przykłady):
  - llama3:       ollama pull llama3
  - mistral:      ollama pull mistral
  - bielik:       ollama pull bielik   (model polski)
  - phi3:         ollama pull phi3     (mały i szybki)

INSTALACJA I URUCHOMIENIE
--------------------------
1. Zainstaluj Qt 6 i CMake.

2. Zainstaluj Ollama:
   https://ollama.com/download
   
   Pobierz wybrany model:
   > ollama pull llama3

3. Uruchom serwis Ollama w tle:
   > ollama serve
   (Domyślny port: http://localhost:11434)

4. Skonfiguruj i zbuduj projekt:
   > cd FileOrganizer
   > mkdir build
   > cd build
   > cmake .. -DCMAKE_BUILD_TYPE=Release
   > cmake --build . --config Release

   Windows MSVC (Qt Creator automatycznie wykryje CMakeLists.txt):
   Otwórz projekt w Qt Creator → Configure → Build

5. Uruchom aplikację:
   > .\Release\FileOrganizer.exe   (Windows)
   > ./FileOrganizer               (Linux)

URUCHAMIANIE TESTÓW JEDNOSTKOWYCH
-----------------------------------
Po zbudowaniu projektu (patrz wyżej):

   > cd build
   > ctest --output-on-failure -V

Lub bezpośrednio:
   > .\FileOrganizerTests.exe      (Windows)
   > ./FileOrganizerTests          (Linux)

GENEROWANIE DOKUMENTACJI DOXYGEN
----------------------------------
Wymaga zainstalowanego Doxygen (https://www.doxygen.nl/):

   > cd build
   > cmake --build . --target doxygen

Dokumentacja zostanie wygenerowana w katalogu docs/html/index.html

STRUKTURA PROJEKTU
-------------------
FileOrganizer/
├── CMakeLists.txt          – konfiguracja budowania
├── README.txt              – ten plik
├── requirements.txt        – zależności
├── include/
│   ├── MainWindow.h        – nagłówek głównego okna GUI
│   ├── OllamaClient.h      – nagłówek klienta REST Ollama
│   ├── PromptBuilder.h     – nagłówek budowania promptów
│   └── ScriptRunner.h      – nagłówek uruchamiania skryptów
├── src/
│   ├── main.cpp            – punkt wejścia
│   ├── MainWindow.cpp      – implementacja GUI (Qt Widgets)
│   ├── OllamaClient.cpp    – implementacja komunikacji z Ollama
│   ├── PromptBuilder.cpp   – implementacja promptów i parsowania
│   └── ScriptRunner.cpp    – implementacja uruchamiania Python
├── tests/
│   ├── test_PromptBuilder.cpp – testy Qt Test dla PromptBuilder
│   └── test_ScriptRunner.cpp  – testy Qt Test dla ScriptRunner
├── docs/                   – generowana dokumentacja Doxygen
└── resources/
    └── resources.qrc       – zasoby Qt

ARCHITEKTURA (wielowątkowość)
------------------------------
  Wątek główny (UI):   MainWindow, ScriptRunner (QProcess)
  Wątek roboczy:       OllamaClient (QNetworkAccessManager)

  OllamaClient działa na osobnym wątku QThread, dzięki czemu
  długotrwałe oczekiwanie na odpowiedź modelu nie blokuje GUI.
  Komunikacja między wątkami odbywa się przez sygnały Qt
  (automatycznie serializowane przez QueuedConnection).

OBSŁUGA BŁĘDÓW
---------------
  - Brak połączenia z Ollama → komunikat w konsoli + informacja dla użytkownika
  - Nieprawidłowa odpowiedź JSON → komunikat błędu
  - Timeout odpowiedzi modelu (120s) → komunikat błędu
  - Brak Pythona w PATH → ostrzeżenie przy starcie
  - Błąd wykonania skryptu → stderr wyświetlony w konsoli
  - Brak wymaganych pól → QMessageBox z opisem błędu

AUTORZY
--------
  Projekt zaliczeniowy – Języki Programowania Obiektowego 2025/2026
================================================================================