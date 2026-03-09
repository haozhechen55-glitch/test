#include "powderdlg.h"
#include "ui_powderdlg.h"
#include "toastdlg.h" // 确保您有这个 Toast 提示类，如果没有可替换为 QMessageBox
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QFileDialog>
#include <QDebug>
#include <QFileInfo>
#include <QMessageBox> // 以防没有 Toast
#include <QJsonDocument>
#include <QJsonObject>
#include <QCoreApplication>

PowderDlg::PowderDlg(QWidget *parent) :
    FramelessBaseDialog(parent),
    ui(new Ui::PowderDlg)
{
    ui->setupUi(this);

    // 适配无边框窗口逻辑：将 UI 设计的内容容器放入父类的布局中
    if (ui->maincontainer_powder) {
        this->contentLayout()->addWidget(ui->maincontainer_powder);
        ui->maincontainer_powder->setVisible(true);
    }

    setWindowTitleText("Powder Setting");
}

PowderDlg::~PowderDlg()
{
    delete ui;
}

void PowderDlg::setTargetNodeName(const QString &name)
{
    m_currentEditingNode = name;
    if(!name.isEmpty()) {
        setWindowTitleText("Edit Powder: " + name);
    } else {
        setWindowTitleText("Powder Setting");
    }
}

void PowderDlg::on_toolButton_browse_clicked()
{
    // 1. 打开文件选择对话框
    QString srcFilePath = QFileDialog::getOpenFileName(this,
                                                       tr("Select Particle File"),
                                                       QDir::homePath(),
                                                       tr("VTK Files (*.vtk);;All Files (*.*)"));
    if (srcFilePath.isEmpty()) return;

    // 2. 确定目标路径 (工程目录下的 input 文件夹)
    QString projectRoot = QDir::currentPath();
    QString inputDir = projectRoot + "/input";

    // 确保 input 文件夹存在
    QDir dir(inputDir);
    if (!dir.exists()) {
        if (!dir.mkpath(".")) {
            QMessageBox::warning(this, "Error", "Cannot create input directory: " + inputDir);
            return;
        }
    }

    // 3. 构建目标文件路径
    QFileInfo fileInfo(srcFilePath);
    QString fileName = fileInfo.fileName();
    QString destFilePath = inputDir + "/" + fileName;

    // 4. 执行拷贝 (如果存在则覆盖)
    // 如果源文件和目标文件一样（用户就在 input 目录里选的），则不操作
    if (QFileInfo(srcFilePath).absoluteFilePath() != QFileInfo(destFilePath).absoluteFilePath()) {
        if (QFile::exists(destFilePath)) {
            QFile::remove(destFilePath);
        }

        if (QFile::copy(srcFilePath, destFilePath)) {
            // 成功拷贝
            // Toast::instance().show(Toast::TSUCCESS, "File copied to input folder.", this);
        } else {
            QMessageBox::warning(this, "Error", "Failed to copy file to input folder.");
            return;
        }
    }

    // 5. 更新 UI 显示为相对路径 (input/xxx.vtk)
    ui->lineEdit_file->setText("input/" + fileName);
}

void PowderDlg::on_pushButton_save_clicked()
{
    writeToYaml();

    // 发送信号
    QString finalName = m_currentEditingNode.isEmpty() ? "Powder" : m_currentEditingNode;
    emit sigAddPowder(finalName);
    emit sigJsonWriteFinish();

    // 提示保存成功并关闭
    // Toast::instance().show(Toast::TINFO, "Powder settings saved.", this);
    this->close();
}

void PowderDlg::writeToYaml()
{
    // 1. 获取 UI 数据
    QString type = ui->comboBox_type->currentText();
    QString fileVal = ui->lineEdit_file->text(); // 此时应为 "input/xxx.vtk"
    QString interval = ui->lineEdit_interval->text();

    // 2. 构造 YAML 块
    QString newSection = QString(
                             "powder:\n"
                             "    type: %1\n"
                             "    file: %2\n"
                             "    particle_inteval: %3\n"
                             ).arg(type).arg(fileVal).arg(interval);

    // 3. 读取现有文件
    QString filePath = QDir::currentPath() + "/out.yml";
    QFile file(filePath);
    QStringList lines;

    // 如果文件存在，先读取所有内容
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString content = file.readAll();
        lines = content.split('\n');
        file.close();
    }

    // 4. 查找旧的 powder 块并删除
    int startIndex = -1;
    int endIndex = -1;

    for (int i = 0; i < lines.size(); ++i) {
        if (lines[i].trimmed().startsWith("powder:")) {
            startIndex = i;
            break;
        }
    }

    if (startIndex != -1) {
        // 向下寻找块的结束
        endIndex = startIndex;
        for (int i = startIndex + 1; i < lines.size(); ++i) {
            QString line = lines[i];
            QString trimmed = line.trimmed();

            // 遇到空行或下一个顶级 Key (无缩进且不是注释) 则视为结束
            if (trimmed.isEmpty() || line.startsWith("#")) {
                endIndex = i;
                continue;
            }
            if (!line.startsWith(" ") && !line.startsWith("\t")) {
                endIndex = i - 1; // 回退一行，因为当前行是新 Key
                break;
            }
            endIndex = i;
        }

        // 从后往前删除，避免索引错位
        for (int i = endIndex; i >= startIndex; --i) {
            if (i < lines.size()) lines.removeAt(i);
        }
    }

    // 5. 插入新块
    QStringList newLines = newSection.split('\n');

    if (startIndex != -1) {
        // 原地插入
        int insertIdx = startIndex;
        for(const QString& l : newLines) {
            if(!l.trimmed().isEmpty()) lines.insert(insertIdx++, l);
        }
    } else {
        // 追加到末尾
        if(!lines.isEmpty() && !lines.last().isEmpty()) lines.append("");
        for(const QString& l : newLines) {
            if(!l.trimmed().isEmpty()) lines.append(l);
        }
    }

    // 6. 写回文件
    if (file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        QTextStream out(&file);
        out << lines.join("\n");
        file.close();
        qDebug() << "Powder settings written to" << filePath;
    } else {
        qCritical() << "Failed to open out.yml for writing:" << filePath;
    }
}

void PowderDlg::writeJsonFile()
{
    // 1. 获取当前 UI 设置
    QString fileRelPath = ui->lineEdit_file->text(); // 例如 "input/particle.vtk"

    // 如果没有选择文件，就不写入配置，直接返回
    if (fileRelPath.isEmpty()) return;

    // 2. 读取现有的 config_temp.json
    // 注意：如果有工程目录，应该优先去工程目录下找；这里为了简单，先用 QDir::currentPath()
    // 因为 loadProjectLogic 里已经把 currentPath 切换到工程目录了，所以是安全的。
    QString configPath = QDir::currentPath() + "/config_temp.json";

    QJsonObject rootObj;
    QFile fileRead(configPath);
    if (fileRead.open(QIODevice::ReadOnly)) {
        QByteArray data = fileRead.readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        rootObj = doc.object();
        fileRead.close();
    }

    // 3. 构建 powder 节点
    QJsonObject powderObj;
    powderObj["type"] = "read_file"; // 对应 main.cpp 里的判断
    powderObj["file"] = fileRelPath; // 对应 main.cpp 里的读取路径

    // 写入根节点
    rootObj["powder"] = powderObj;

    // 4. 写回文件
    QFile fileWrite(configPath);
    if (fileWrite.open(QIODevice::WriteOnly)) {
        QJsonDocument newDoc(rootObj);
        fileWrite.write(newDoc.toJson());
        fileWrite.close();
    }
}
