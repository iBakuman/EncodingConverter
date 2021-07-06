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
    selectionModel = new QItemSelectionModel(model); // 与数据模型关联
    ui.treeView->setModel(model);
    ui.treeView->setSelectionModel(selectionModel);

    // 设置多选
    ui.treeView->setSelectionMode(QAbstractItemView::ExtendedSelection);

    connect(selectionModel, SIGNAL(currentRowChanged(const QModelIndex&, const QModelIndex&)),
            this, SLOT(when_currentRowChanged(const QModelIndex&, const QModelIndex&)));
}


bool EncodingConverter::saveFile(const QModelIndex& fileIndex, const QString& savePath) {
    if (model->isDir(fileIndex) || !fileIndex.isValid()) {
        // 目录不需要转换
        return false;
    }

    QFile rFile(model->filePath(fileIndex));
    if (!rFile.exists() | !rFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false; // 如果文件打开失败或者文件不存在则跳过
    }

    // 获得保存文件的绝对路径
    QString nameToSave = savePath + "/" + model->fileName(fileIndex);
    QFile wFile(nameToSave);
    if (!wFile.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    QTextStream in(&rFile);
    setCodec(in, true); // 设置输入编码
    QString srcContext = in.readAll(); // 获得源文件内容

    QTextStream out(&wFile);
    setCodec(out, false); // 设置输出编码
    out << srcContext; // 写入文件

    rFile.close();
    wFile.close();
    return true;
}

// 打开目录，返回目录的路径
QString EncodingConverter::chooseDir() {
    QFileDialog chooseDirDlg;

    QString srcDirStr = chooseDirDlg.getExistingDirectory(this,
                                                          "选择源文件目录",
                                                          QDir::currentPath());

    return srcDirStr;
}

// isSrc为true表示设置读取时编码，反之设置写入时编码
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
            qTextStream.setGenerateByteOrderMark(true); // 带BOM
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
    // 选择源文件的文件夹

    QString srcDirStr = chooseDir();
    if (srcDirStr.isEmpty())
        return;
    // 1.修复了未选择文件夹时清空了原来的路径以及内容预览的问题***important
    ui.srcDirText->clear(); // 清除原来的内容
    ui.srcPreview->clear();
    ui.dstPreview->clear();
    ui.srcDirText->setText(srcDirStr);

    // 设置根目录
    ui.treeView->setRootIndex(model->index(srcDirStr));
    // 设置列的宽度根据内容调整
    for (int i = 0; i < model->columnCount(); ++i) // View中不能够得到列数，需要借助Model的方法
        ui.treeView->resizeColumnToContents(i);
}

void EncodingConverter::on_btnDstDir_clicked() {
    ui.dstDirText->clear(); // 清除原来的内容
    QString srcDirStr = chooseDir();
    ui.dstDirText->setText(srcDirStr);
}

// 开始转换
void EncodingConverter::on_btnStart_clicked() {
    QString dstDirStr = ui.dstDirText->text().trimmed();

    if (dstDirStr.isEmpty()) {
        QMessageBox::warning(this, "警告", "目标文件夹未指定!");
        return;
    }

    if (dstDirStr == ui.srcDirText->text()) {
        QMessageBox::warning(this, "警告", "目标文件夹和源文件夹相同!");
        return;
    }

    // 获取被选择的文件
    QModelIndexList selectedIndexes = selectionModel->selectedRows();
    if (selectedIndexes.empty()) {
        QMessageBox::information(this, "提示", "没有选择要转换的文件");
        return;
    }

    if (srcCodec == dstCodec) {
        // 编码判断放在确保有文件之后更合理***important
        QMessageBox::information(this, "提示", "源文件编码和目标编码相同，无需转换!");
        return;
    }

    int cnt = selectedIndexes.count(); // 选择的文件的数量
    QStringList failures; // 转换失败的文件的名称
    for (const QModelIndex& curIndex : selectedIndexes) {
        if (model->isDir(curIndex))
            continue;
        if (!saveFile(curIndex, dstDirStr))
            failures.append(model->fileName(curIndex));
    }

    if (failures.empty()) {
        QString text;
        // error_1
        // text += "转换成功：\n共转换" + cnt + "个文件";
        // error_2
        // text += "转换成功：\n共转换" + cnt;
        // text += "个文件";

        text += "转换成功：\n共转换" + QString::number(cnt) + "个文件";
        QMessageBox::information(this, "消息", text);
    }
    else {
        QString text;
        text += "转化失败，转换失败的文件如下：\n";
        foreach(const QString& str, failures) {
            text += str + "\n";
        }
        QMessageBox::warning(this, "警告", text);
    }
}

void EncodingConverter::on_cboDstEn_currentIndexChanged(int index) {
    dstCodec = static_cast<Codec>(ui.cboDstEn->currentIndex());
    QModelIndex curIndex = ui.treeView->currentIndex();
    contentPreview(curIndex, false); // 目标文件预览
}

void EncodingConverter::on_cboSrcEn_currentIndexChanged(int index) {
    srcCodec = static_cast<Codec>(ui.cboSrcEn->currentIndex());
    QModelIndex curIndex = ui.treeView->currentIndex();
    contentPreview(curIndex, true); // 源文件预览
}

bool EncodingConverter::contentPreview(const QModelIndex& fileIndex, bool isSrc) {
    if (!fileIndex.isValid())
        return false;

    QString filepath = model->filePath(fileIndex);

    if (model->isDir(fileIndex)) // 如果文件时目录则跳过，文件是否可能不存在?
        return false;

    QFile aFile(filepath);

    if (!aFile.open(QIODevice::ReadOnly | QIODevice::Text))
        return false; // 打开文件失败直接返回

    QTextStream in(&aFile);
    setCodec(in, true); // 设置文件字符集

    QTextEdit* targetEdit = nullptr;
    if (isSrc)
        targetEdit = ui.srcPreview;
    else
        targetEdit = ui.dstPreview;

    int lastPos = targetEdit->verticalScrollBar()->value(); // 获得当前滚动条位置
    // 清除原来的信息
    targetEdit->clear();
    // 附加内容
    targetEdit->append(in.readAll());
    targetEdit->verticalScrollBar()->setValue(lastPos); // 回到上次浏览位置

    aFile.close(); // 释放资源
    return true;
}

// 选择模型的槽函数，当选择某一个源文件时将文件的内容显示到文本框中预览
void EncodingConverter::when_currentRowChanged(const QModelIndex& current, const QModelIndex& previous) {
    Q_UNUSED(previous);
    if (!current.isValid()) // 不是有效的模型索引的则直接返回
        return;

    // 源文件和目标文件同时预览，默认都是GBK编码
    contentPreview(current, true);
    contentPreview(current, false);

    // 设置从开头显示
    ui.srcPreview->verticalScrollBar()->setValue(0);
    ui.dstPreview->verticalScrollBar()->setValue(0);
}
