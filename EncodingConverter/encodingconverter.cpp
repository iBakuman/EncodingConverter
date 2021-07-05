#include "encodingconverter.h"
#include <QFileDialog>
#include <QScrollBar>
#include <QDebug>
#include <QMessageBox>
#include <QTextCodec>
#include <QDir>

#pragma  execution_character_set("utf-8")

EncodingConverter::EncodingConverter(QWidget* parent)
    : QMainWindow(parent) {
    ui.setupUi(this);

    model = new QFileSystemModel();
    model->setRootPath(QDir::currentPath());
    selectionModel = new QItemSelectionModel(model); // ������ģ�͹���
    ui.treeView->setModel(model);
    ui.treeView->setSelectionModel(selectionModel);

    // ���ö�ѡ
    ui.treeView->setSelectionMode(QAbstractItemView::ExtendedSelection);

    connect(selectionModel, SIGNAL(currentRowChanged(const QModelIndex&, const QModelIndex&)),
            this, SLOT(when_currentRowChanged(const QModelIndex&, const QModelIndex&)));

}

bool EncodingConverter::saveFile(const QString& content, const QString& aFileName) {
    if (aFileName.isEmpty())
        return false;
    QFile aFile(aFileName);

    if (!aFile.exists() || !aFile.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    QTextStream out(&aFile);
    setCodec(out, false);
    out << content;
    aFile.close();
    return true;

}

// ��Ŀ¼������Ŀ¼��·��
QString EncodingConverter::chooseDir() {
    QFileDialog chooseDirDlg;

    QString srcDirStr = chooseDirDlg.getExistingDirectory(this,
                                                          "ѡ��Դ�ļ�Ŀ¼",
                                                          QDir::currentPath());

    return srcDirStr;
}

// isSrcΪtrue��ʾ���ö�ȡʱ���룬��֮����д��ʱ����
void EncodingConverter::setCodec(QTextStream& qTextStream, bool isSrc) {
    Codec tmp = srcCodec;
    if (!isSrc)
        tmp = dstCodec;
    switch (tmp) {
        case GBK: {
            qTextStream.setCodec("GBK");
            break;
        }
        case UTF_WITH_BOM: {
            qTextStream.setCodec("UTF-8");
            qTextStream.setGenerateByteOrderMark(true); // ��BOM
            break;
        }
        case UTF_8_WITHOUT_BOM: {
            qTextStream.setCodec("UTF-8");
            qTextStream.setGenerateByteOrderMark(false);
            break;
        }
    }

}


void EncodingConverter::on_actOpenDir_triggered() {
    QString srcDirStr = chooseDir();

    if (srcDirStr.isEmpty())
        return;

    QDir srcDir(srcDirStr);
    if (!srcDir.exists())
        return;

}

void EncodingConverter::on_btnSrcDir_clicked() {
    // ѡ��Դ�ļ����ļ���

    QString srcDirStr = chooseDir();
    if (srcDirStr.isEmpty())
        return;
    // 1.�޸���δѡ���ļ���ʱ�����ԭ����·���Լ�����Ԥ��������***important
    ui.srcDirText->clear(); // ���ԭ��������
    ui.srcPreview->clear();
    ui.dstPreview->clear();
    ui.srcDirText->setText(srcDirStr);

    // ���ø�Ŀ¼
    ui.treeView->setRootIndex(model->index(srcDirStr));
    // �����еĿ�ȸ������ݵ���
    for (int i = 0; i < model->columnCount(); ++i) // View�в��ܹ��õ���������Ҫ����Model�ķ���
        ui.treeView->resizeColumnToContents(i);
}

void EncodingConverter::on_btnDstDir_clicked() {
    ui.dstDirText->clear(); // ���ԭ��������
    QString srcDirStr = chooseDir();
    ui.dstDirText->setText(srcDirStr);
}

// ��ʼת��
void EncodingConverter::on_btnStart_clicked() {
    QString dstDirStr = ui.dstDirText->text().trimmed();

    QModelIndexList selectedIndexes = selectionModel->selectedRows();
    if (selectedIndexes.empty()) {
        QMessageBox::information(this, "��ʾ", "û��ѡ��Ҫת�����ļ�");
        return;
    }

    if (srcCodec == dstCodec) {
        // �����жϷ���ȷ�����ļ�֮�������***important
        QMessageBox::information(this, "��ʾ", "Դ�ļ������Ŀ�������ͬ������ת��!");
        return;
    }

    int failures = 0; // ת��ʧ�ܵĴ���
    for (const QModelIndex& curIndex : selectedIndexes) {
        if (model->isDir(curIndex)) // Ŀ¼����Ҫת��
            continue;
        QFile aFile(model->filePath(curIndex));
        if (!aFile.exists() | !aFile.open(QIODevice::WriteOnly | QIODevice::Text))
            continue; // ����ļ���ʧ�ܻ����ļ�������������
        QTextStream out(&aFile);

    }
}

void EncodingConverter::on_cboDstEn_currentIndexChanged(int index) {
    dstCodec = static_cast<Codec>(ui.cboDstEn->currentIndex());
    QModelIndex curIndex = ui.treeView->currentIndex();
    contentPreview(curIndex, false); // Ŀ���ļ�Ԥ��
}

void EncodingConverter::on_cboSrcEn_currentIndexChanged(int index) {
    srcCodec = static_cast<Codec>(ui.cboSrcEn->currentIndex());
    QModelIndex curIndex = ui.treeView->currentIndex();
    contentPreview(curIndex, true); // Դ�ļ�Ԥ��
}

bool EncodingConverter::contentPreview(const QModelIndex& fileIndex, bool isSrc) {
    if (!fileIndex.isValid())
        return false;

    QString filepath = model->filePath(fileIndex);

    if (model->isDir(fileIndex)) // ����ļ�ʱĿ¼���������ļ��Ƿ���ܲ�����?
        return false;

    QFile aFile(filepath);

    if (!aFile.open(QIODevice::ReadOnly | QIODevice::Text))
        return false; // ���ļ�ʧ��ֱ�ӷ���

    QTextStream in(&aFile);
    setCodec(in, true); // �����ļ��ַ���

    QTextEdit* targetEdit = nullptr;
    if (isSrc)
        targetEdit = ui.srcPreview;
    else
        targetEdit = ui.dstPreview;

    int lastPos = targetEdit->verticalScrollBar()->value(); // ��õ�ǰ������λ��
    // ���ԭ������Ϣ
    targetEdit->clear();
    // ��������
    targetEdit->append(in.readAll());
    targetEdit->verticalScrollBar()->setValue(lastPos); // �ص��ϴ����λ��

    aFile.close(); // �ͷ���Դ
    return true;
}

// ѡ��ģ�͵Ĳۺ�������ѡ��ĳһ��Դ�ļ�ʱ���ļ���������ʾ���ı�����Ԥ��
void EncodingConverter::when_currentRowChanged(const QModelIndex& current, const QModelIndex& previous) {
    Q_UNUSED(previous);
    if (!current.isValid()) // ������Ч��ģ����������ֱ�ӷ���
        return;

    // Դ�ļ���Ŀ���ļ�ͬʱԤ����Ĭ�϶���GBK����
    contentPreview(current, true);
    contentPreview(current, false);

    // ���ôӿ�ͷ��ʾ
    ui.srcPreview->verticalScrollBar()->setValue(0);
    ui.dstPreview->verticalScrollBar()->setValue(0);
}
