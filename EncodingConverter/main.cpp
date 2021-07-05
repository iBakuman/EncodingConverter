#include "encodingconverter.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QFont font;
    font.setFamily("Consolas");
    a.setFont(font);// 设置全局字体为Consolas
	
    EncodingConverter w;
    w.show();
    return a.exec();
}
