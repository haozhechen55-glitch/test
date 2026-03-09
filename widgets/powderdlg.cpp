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
    // 写入前处理预览配置 (config_temp.json)
    writeJsonFile();

    // 发送信号
    QString finalName = m_currentEditingNode.isEmpty() ? "Powder" : m_currentEditingNode;
    emit sigAddPowder(finalName);

    // 触发主界面刷新预览
    emit sigJsonWriteFinish();

    this->close();
}

QString PowderDlg::getYamlSection() const
{
    QString type = ui->comboBox_type->currentText();
    QString fileVal = ui->lineEdit_file->text();
    QString interval = ui->lineEdit_interval->text();

    if (type.isEmpty() && fileVal.isEmpty()) return QString();

    QString yaml;
    yaml += "powder:\n";
    yaml += "    type: " + type + "\n";
    yaml += "    file: " + fileVal + "\n";
    yaml += "    particle_inteval: " + interval + "\n";
    return yaml;
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
