#include <QApplication>
#include <QPushButton>

#include "database/DatabaseManager.h"
#include "widget/MainWindow.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    DatabaseManager::instance().initDatabase();
    if (!DatabaseManager::instance().openDatabase())
        return -1;

    MainWindow w;
    w.show();
    return QApplication::exec();
}
