cmake_minimum_required(VERSION 3.16)

project(MakroScales VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(THREADS_PREFER_PTHREAD_FLAG ON)

# Явно указываем необходимые компоненты Qt
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network SerialPort Concurrent LinguistTools)

set(TS_FILES MakroScales_ru_RU.ts)

set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    homepage.h
    settingspage.h
    counterspage.h
    logspage.h
    logspage.cpp
    settingspage.cpp
    homepage.cpp
    counterspage.cpp
    clientsocket.h
    serversocket.h
    serversocket.cpp
    clientsocket.cpp
    constants.h
    constants.cpp
)

# Создаем исполняемый файл
add_executable(MakroScales ${PROJECT_SOURCES}
    Icon.qrc
    bridgelinxtocab.h bridgelinxtocab.cpp
    appcontroller.h
    appcontroller.cpp)

# Добавляем переводы правильно
qt6_add_translation(QM_FILES ${TS_FILES})
target_sources(MakroScales PRIVATE ${QM_FILES})

# Линкуем необходимые библиотеки
target_link_libraries(MakroScales PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
    Qt6::SerialPort
    Qt6::Concurrent
)

# Устанавливаем свойства для macOS
if(APPLE)
    set_target_properties(MakroScales PROPERTIES
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
        MACOSX_BUNDLE TRUE
    )
endif()

# Устанавливаем свойства для Windows
if(WIN32)
    set_target_properties(MakroScales PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()

# Установка приложения
include(GNUInstallDirs)
install(TARGETS MakroScales
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
