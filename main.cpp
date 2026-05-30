#include "mainwindow.h"

#include <QApplication>
#include <QSettings>
#include <QStandardPaths>
#include <QFile>
#include <QDir>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    QFile styleFile(":/styles/styles.qss");

    if (!styleFile.open(QFile::ReadOnly))
        throw std::runtime_error("Nu sa putut incarca datele pentru interfata grafica");

    QString style = styleFile.readAll();
    qApp->setStyleSheet(style);

    return a.exec();
}
