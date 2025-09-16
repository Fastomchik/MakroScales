#include "appcontroller.h"
#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Создаем контроллер приложения
    AppController appController;
    appController.initialize();

    // Создаем главное окно и передаем необходимые страницы через конструктор
    MainWindow window(appController.homePage(),
                      appController.settingsPage(),
                      appController.countersPage(),
                      appController.logsPage());

    window.show();

    return app.exec();
}
