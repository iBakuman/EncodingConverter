#include "encodingconverter.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    EncodingConverter w;
    w.show();
    return a.exec();
}
