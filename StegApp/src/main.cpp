#include "headers/mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    //For noise option. Header should be encoded in LSB and include noise information. The rest encoded accordingly, That way user does not need to tell noise amount to decode
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    w.setFixedSize(798,600);
    return a.exec();
}
