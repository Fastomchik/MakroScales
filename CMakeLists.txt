cmake_minimum_required(VERSION 3.16)

project(MakroScales VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network SerialPort Concurrent)

set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    homepage.h
    homepage.cpp
    settingspage.h
    settingspage.cpp
    counterspage.h
    counterspage.cpp
    logspage.h
    logspage.cpp
    clientsocket.h
    clientsocket.cpp
    serversocket.h
    serversocket.cpp
    bridgelinxtocab.h
    bridgelinxtocab.cpp
    appcontroller.h
    appcontroller.cpp
    constants.h
    constants.cpp
)

# Создаем исполняемый файл
add_executable(MakroScales ${PROJECT_SOURCES}
    Icon.qrc
)

# Линкуем необходимые библиотеки
target_link_libraries(MakroScales PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
    Qt6::SerialPort
    Qt6::Concurrent
)

# Для Linux добавляем зависимости
find_package(Threads REQUIRED)
target_link_libraries(MakroScales PRIVATE Threads::Threads)

# Установка приложения для Linux
include(GNUInstallDirs)
install(TARGETS MakroScales
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Копирование ресурсов и переводов
install(FILES ${QM_FILES}
    DESTINATION ${CMAKE_INSTALL_DATADIR}/translations
)
