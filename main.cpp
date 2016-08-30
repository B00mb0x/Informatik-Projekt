#include "arduinoshow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ArduinoShow w;
    w.show();

    return a.exec();
}
