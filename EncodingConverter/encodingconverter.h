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
		// 以下资源释放顺序不能颠倒，不然退出程序时程序会崩溃***important
		delete selectionModel;
		delete model;
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

	/*
	 * 保存转换后的文件
	 * @param content
	 */
	bool saveFile(const QString& content, const QString& aFileName);
	QString chooseDir();
	void setCodec(QTextStream& qTextStream, bool isSrc);


private slots:
	void on_actOpenDir_triggered();
	void on_btnSrcDir_clicked();
	void on_btnDstDir_clicked();
	
	// 转换按钮被点击时调用的函数
	void on_btnStart_clicked();
	
	// 目标文件编码发生变化时触发
	void on_cboDstEn_currentIndexChanged(int index);
	// 源文件编码发生变化时触发
	void on_cboSrcEn_currentIndexChanged(int index);
	
	/* 内容预览，执行成功返回true，反之返回false
	 * @param filePath---文件路径
	 * @param isSrc------是否是源文件预览，为false则表示是目标文件预览
	 */
	bool contentPreview(const QModelIndex& fileIndex, bool isSrc);

	/*
	 * 当选择的文件发生改变时被调用的槽函数                                         
	 * @param current-----当前选择的文件的模型索引
	 * @param previous----上一次选择的文件的模型索引
	 */
	void when_currentRowChanged(const QModelIndex& current, const QModelIndex& previous);
};
