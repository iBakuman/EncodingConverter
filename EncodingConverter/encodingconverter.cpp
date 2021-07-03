#include "encodingconverter.h"
#include <QFileDialog>
#include <QDebug>
#include <QMessageBox>
#include <QTextCodec>

#pragma  execution_character_set("utf-8")

EncodingConverter::EncodingConverter(QWidget* parent)
	: QMainWindow(parent) {
	ui.setupUi(this);

	QFont font;
	font.setFamily("Consolas");
	ui.srcDirText->setFont(font);
	ui.dstDirText->setFont(font);
	model = new QFileSystemModel();
	model->setRootPath(QDir::currentPath());
	selectionModel = new QItemSelectionModel(model);// 与数据模型关联
	ui.treeView->setModel(model);
	ui.treeView->setSelectionModel(selectionModel);
	// ui.treeView->setSelectionMode(QAbstractItemView::MultiSelection);
	// ui.treeView->setSelectionMode(QAbstractItemView::ContiguousSelection);
	// 设置多选
	ui.treeView->setSelectionMode(QAbstractItemView::ExtendedSelection);

	connect(selectionModel, SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)),
		this, SLOT(when_currentChanged(const QModelIndex&, const QModelIndex&)));
	connect(selectionModel, SIGNAL(currentRowChanged(const QModelIndex&, const QModelIndex&)),
		this, SLOT(when_currentRowChanged(const QModelIndex&, const QModelIndex&)));
}

void EncodingConverter::saveFile(const QString& content, const QString& aFileName) {
	if (aFileName.isEmpty())
		return;
	QFile aFile(aFileName);

	if (!aFile.open(QIODevice::WriteOnly | QIODevice::Text))
		return;

	QTextStream out(&aFile);
	setCodec(out, false);
	out << content;
	aFile.close();

}

// 打开目录，返回目录的路径
QString EncodingConverter::chooseDir() {
	QFileDialog chooseDirDlg;

	QString srcDirStr = chooseDirDlg.getExistingDirectory(this,
		"选择源文件目录",
		QDir::currentPath());

	return srcDirStr;
}

// isSrc为true表示给源文件设置编码，反之给目标文件设置编码
void EncodingConverter::setCodec(QTextStream& qTextStream, bool isSrc) {
	Codec tmp = srcCodec;
	if (!isSrc)
		tmp = dstCodec;
	switch (tmp) {
	case GBK: {
		qTextStream.setCodec("GBK");
	}
			break;
	case UTF_WITH_BOM: {
		qTextStream.setCodec("UTF-8");
		qTextStream.setGenerateByteOrderMark(true); // 带BOM
	}
					 break;
	case UTF_8_WITHOUT_BOM: {
		qTextStream.setCodec("UTF-8");
		qTextStream.setGenerateByteOrderMark(false);
	}
						  break;
	}

}


void EncodingConverter::on_actOpenDir_triggered() {
	qDebug() << "actOpenDir triggered!";

	QString srcDirStr = chooseDir();
	if (srcDirStr.isEmpty())
		return;

	QDir srcDir(srcDirStr);
	if (!srcDir.exists())
		return;


}

void EncodingConverter::on_btnSrcDir_clicked() {
	// 选择源文件的文件夹
	ui.srcDirText->clear(); // 清除原来的内容
	QString srcDirStr = chooseDir();
	if (srcDirStr.isEmpty())
		return;
	ui.srcDirText->setText(srcDirStr);
	// ui.treeView->setModel(model);
	ui.treeView->setRootIndex(model->index(srcDirStr));
	// 设置列的宽度
	for (int i = 0; i < model->columnCount(); ++i)// View中不能够得到列数，需要借助Model的方法
		ui.treeView->resizeColumnToContents(i);
}

void EncodingConverter::on_btnDstDir_clicked() {
	ui.dstDirText->clear(); // 清除原来的内容
	QString srcDirStr = chooseDir();
	ui.dstDirText->setText(srcDirStr);
}

// 开始转换
void EncodingConverter::on_btnStart_clicked() {
	QString srcDirStr = ui.srcDirText->text().trimmed();
	QString dstDirStr = ui.dstDirText->text().trimmed();


	if (srcCodec == dstCodec) {
		QMessageBox::information(this, "提示", "源文件编码和目标编码相同，无需转换!");
		return;
	}

	if (srcDirStr.isEmpty() || dstDirStr.isEmpty()) {
		QMessageBox::warning(this, "警告", "源文件夹或目标文件夹未指定!");
		return;
	}

	QDir srcDir(srcDirStr);
	QFileInfoList fileInfos = srcDir.entryInfoList(QDir::Files); // 遍历普通文件
	QListIterator<QFileInfo> ite(fileInfos);
	ui.srcPreview->clear();// 清空原来内容
	while (ite.hasNext()) {
		const QFileInfo& curFileInfo = ite.next();
		QString fileName = curFileInfo.fileName();
		QString filePath = curFileInfo.absoluteFilePath();
		QFile curFile(filePath);
		if (!curFile.exists()) // 如果文件不存在
			continue;

		if (!curFile.open(QIODevice::ReadOnly | QIODevice::Text))
			continue;

		QTextStream in(&curFile);
		setCodec(in, true);

		QString content = in.readAll();
		ui.srcPreview->append(fileName);
		ui.srcPreview->append(content);
		ui.srcPreview->append("------------------------");

		saveFile(content, ui.dstDirText->text() + "/" + fileName);

		qDebug() << filePath;
		qDebug() << fileName;
		curFile.close();
	}
}

void EncodingConverter::on_cboDstEn_currentIndexChanged(int index) {
	dstCodec = static_cast<Codec>(ui.cboDstEn->currentIndex());
}

void EncodingConverter::on_cboSrcEn_currentIndexChanged(int index) {
	srcCodec = static_cast<Codec>(ui.cboSrcEn->currentIndex());
}

void EncodingConverter::when_currentChanged(const QModelIndex& current, const QModelIndex& previous) {
	qDebug() << "currentChanged()";

	// 使用selectedIndexes会拥有其他信息
	QModelIndexList selectedIndexes = selectionModel->selectedIndexes();
	qDebug() << selectedIndexes.count();
	if (selectedIndexes.count() > 1)
		return;
	foreach(const QModelIndex& curModelIndex, selectedIndexes) {
		qDebug() << model->fileName(curModelIndex);
	}
	ui.srcPreview->clear();
}

void EncodingConverter::when_currentRowChanged(const QModelIndex& current, const QModelIndex& previous) {
	qDebug() << "currentRowChanged()";
	QModelIndexList selectedIndexes = selectionModel->selectedRows();
	qDebug() << selectedIndexes.count();
}
