cmake_minimum_required(VERSION 3.16)
project(FileOrganizer VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network)

# Opcjonalny Doxygen
find_package(Doxygen OPTIONAL_COMPONENTS dot)

# ── Główna aplikacja ──────────────────────────────────────────────────────────
add_executable(FileOrganizer
    src/main.cpp
    src/MainWindow.cpp
    src/OllamaClient.cpp
    src/ScriptRunner.cpp
    src/PromptBuilder.cpp
    include/MainWindow.h
    include/OllamaClient.h
    include/ScriptRunner.h
    include/PromptBuilder.h
    resources/resources.qrc
)

target_include_directories(FileOrganizer PRIVATE include)

target_link_libraries(FileOrganizer PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
)

# ── Testy jednostkowe ─────────────────────────────────────────────────────────
enable_testing()
find_package(Qt6 COMPONENTS Test QUIET)

if(Qt6Test_FOUND)
    # Test 1: PromptBuilder
    add_executable(TestPromptBuilder
        tests/test_PromptBuilder.cpp
        src/PromptBuilder.cpp
        include/PromptBuilder.h
    )
    target_include_directories(TestPromptBuilder PRIVATE include)
    target_link_libraries(TestPromptBuilder PRIVATE
        Qt6::Core
        Qt6::Widgets
        Qt6::Network
        Qt6::Test
    )
    add_test(NAME TestPromptBuilder COMMAND TestPromptBuilder)

    # Test 2: ScriptRunner
    # Nagłówki z Q_OBJECT MUSZĄ być wymienione – MOC je przetwarza na podstawie
    # listy plików w add_executable, nie na podstawie #include w kodzie.
    add_executable(TestScriptRunner
        tests/test_ScriptRunner.cpp
        src/ScriptRunner.cpp
        src/OllamaClient.cpp
        src/PromptBuilder.cpp
        include/ScriptRunner.h
        include/OllamaClient.h
        include/PromptBuilder.h
    )
    target_include_directories(TestScriptRunner PRIVATE include)
    target_link_libraries(TestScriptRunner PRIVATE
        Qt6::Core
        Qt6::Widgets
        Qt6::Network
        Qt6::Test
    )
    add_test(NAME TestScriptRunner COMMAND TestScriptRunner)
endif()

# ── Doxygen ───────────────────────────────────────────────────────────────────
if(DOXYGEN_FOUND)
    set(DOXYGEN_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/docs)
    set(DOXYGEN_PROJECT_NAME "FileOrganizer")
    set(DOXYGEN_EXTRACT_ALL YES)
    set(DOXYGEN_RECURSIVE YES)
    doxygen_add_docs(doxygen
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/src
        COMMENT "Generowanie dokumentacji Doxygen"
    )
endif()