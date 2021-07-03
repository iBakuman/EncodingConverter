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
	selectionModel = new QItemSelectionModel(model);// ������ģ�͹���
	ui.treeView->setModel(model);
	ui.treeView->setSelectionModel(selectionModel);
	// ui.treeView->setSelectionMode(QAbstractItemView::MultiSelection);
	// ui.treeView->setSelectionMode(QAbstractItemView::ContiguousSelection);
	// ���ö�ѡ
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

// ��Ŀ¼������Ŀ¼��·��
QString EncodingConverter::chooseDir() {
	QFileDialog chooseDirDlg;

	QString srcDirStr = chooseDirDlg.getExistingDirectory(this,
		"ѡ��Դ�ļ�Ŀ¼",
		QDir::currentPath());

	return srcDirStr;
}

// isSrcΪtrue��ʾ��Դ�ļ����ñ��룬��֮��Ŀ���ļ����ñ���
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
		qTextStream.setGenerateByteOrderMark(true); // ��BOM
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
	// ѡ��Դ�ļ����ļ���
	ui.srcDirText->clear(); // ���ԭ��������
	QString srcDirStr = chooseDir();
	if (srcDirStr.isEmpty())
		return;
	ui.srcDirText->setText(srcDirStr);
	// ui.treeView->setModel(model);
	ui.treeView->setRootIndex(model->index(srcDirStr));
	// �����еĿ��
	for (int i = 0; i < model->columnCount(); ++i)// View�в��ܹ��õ���������Ҫ����Model�ķ���
		ui.treeView->resizeColumnToContents(i);
}

void EncodingConverter::on_btnDstDir_clicked() {
	ui.dstDirText->clear(); // ���ԭ��������
	QString srcDirStr = chooseDir();
	ui.dstDirText->setText(srcDirStr);
}

// ��ʼת��
void EncodingConverter::on_btnStart_clicked() {
	QString srcDirStr = ui.srcDirText->text().trimmed();
	QString dstDirStr = ui.dstDirText->text().trimmed();


	if (srcCodec == dstCodec) {
		QMessageBox::information(this, "��ʾ", "Դ�ļ������Ŀ�������ͬ������ת��!");
		return;
	}

	if (srcDirStr.isEmpty() || dstDirStr.isEmpty()) {
		QMessageBox::warning(this, "����", "Դ�ļ��л�Ŀ���ļ���δָ��!");
		return;
	}

	QDir srcDir(srcDirStr);
	QFileInfoList fileInfos = srcDir.entryInfoList(QDir::Files); // ������ͨ�ļ�
	QListIterator<QFileInfo> ite(fileInfos);
	ui.srcPreview->clear();// ���ԭ������
	while (ite.hasNext()) {
		const QFileInfo& curFileInfo = ite.next();
		QString fileName = curFileInfo.fileName();
		QString filePath = curFileInfo.absoluteFilePath();
		QFile curFile(filePath);
		if (!curFile.exists()) // ����ļ�������
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

	// ʹ��selectedIndexes��ӵ��������Ϣ
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
