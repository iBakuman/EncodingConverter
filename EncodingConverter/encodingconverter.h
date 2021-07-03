#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_encodingconverter.h"

#include <QTextCodec>
#include <QFileSystemModel>

class EncodingConverter : public QMainWindow {
Q_OBJECT

public:
	EncodingConverter(QWidget* parent = Q_NULLPTR);
	~EncodingConverter() {	// �ͷ���Դ
		delete model;
		delete selectionModel;
	}

private:
	enum Codec {
		GBK,
		UTF_WITH_BOM,
		UTF_8_WITHOUT_BOM
	};

	Codec srcCodec = GBK;
	Codec dstCodec = GBK;
	QFileSystemModel* model;// ����ģ��
	QItemSelectionModel* selectionModel;// ѡ��ģ��
	
	Ui::EncodingConverterClass ui;
	void saveFile(const QString& content, const QString& aFileName);
	QString chooseDir();
	void setCodec(QTextStream& qTextStream, bool isSrc);
	

private slots:
	void on_actOpenDir_triggered();
	void on_btnSrcDir_clicked();
	void on_btnDstDir_clicked();
	void on_btnStart_clicked();
	void on_cboDstEn_currentIndexChanged(int index);
	void on_cboSrcEn_currentIndexChanged(int index);

	void when_currentChanged(const QModelIndex& current, const QModelIndex& previous);
	void when_currentRowChanged(const QModelIndex& current, const QModelIndex& previous);
};
