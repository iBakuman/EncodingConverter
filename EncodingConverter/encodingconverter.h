#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_encodingconverter.h"

#include <QTextCodec>
#include <QFileSystemModel>

class EncodingConverter : public QMainWindow {
	Q_OBJECT

public:
	EncodingConverter(QWidget* parent = Q_NULLPTR);
	~EncodingConverter() {	// 释放资源
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
	QFileSystemModel* model;// 数据模型
	QItemSelectionModel* selectionModel;// 选择模型

	Ui::EncodingConverterClass ui;
	void saveFile(const QString& content, const QString& aFileName);
	QString chooseDir();
	void setCodec(QTextStream& qTextStream, bool isSrc);


private slots:
	void on_actOpenDir_triggered();
	void on_btnSrcDir_clicked();
	void on_btnDstDir_clicked();
	void on_btnStart_clicked();
	// 目标文件编码发生变化时触发
	void on_cboDstEn_currentIndexChanged(int index);
	// 源文件编码发生变化时触发
	void on_cboSrcEn_currentIndexChanged(int index);
	// 源文件内容预览，执行成功返回true，反之返回false
	bool srcPreview(QString filepath);

	void when_currentChanged(const QModelIndex& current, const QModelIndex& previous);
	void when_currentRowChanged(const QModelIndex& current, const QModelIndex& previous);
};
