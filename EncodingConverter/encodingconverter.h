#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_encodingconverter.h"

class EncodingConverter : public QMainWindow
{
    Q_OBJECT

public:
    EncodingConverter(QWidget *parent = Q_NULLPTR);

private:
    Ui::EncodingConverterClass ui;
};
