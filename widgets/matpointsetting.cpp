#include "matpointsetting.h"
#include "ui_matpointsetting.h"
#include"toastdlg.h"
#include <QDir>
#include <QTextStream>
#include <QFile>
#include<QDebug>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>
MatPointSetting::MatPointSetting(QWidget *parent)
    : FramelessBaseDialog(parent)
    , ui(new Ui::MatPointSetting)
{
    // 1. 初始化 UI (此时布局会乱，但控件对象已生成并绑定到 this)
    ui->setupUi(this);

    this->contentLayout()->addWidget(ui->Material_Points_widget);

    // 5. 基础配置
    setWindowTitleText("Material Point Settings");


}

MatPointSetting::~MatPointSetting()
{
    delete ui;
}


void MatPointSetting::on_pushButton_OK_clicked()
{
    QString name = ui->lineEdit_name->text().trimmed();
    QString type = ui->comboBox_type->currentText().trimmed();
    QString Dx = ui->lineEdit_Dx->text().trimmed();
    QString Dy = ui->lineEdit_Dy->text().trimmed();
    QString Dz = ui->lineEdit_Dz->text().trimmed();
    QString Xmin = ui->lineEdit_Xmin->text().trimmed();
    QString Ymin = ui->lineEdit_Ymin->text().trimmed();
    QString Zmin = ui->lineEdit_Zmin->text().trimmed();
    QString Xmax = ui->lineEdit_Xmax->text().trimmed();
    QString Ymax = ui->lineEdit_Ymax->text().trimmed();
    QString Zmax = ui->lineEdit_Zmax->text().trimmed();

    if(name.isEmpty() || type.isEmpty() || Dx.isEmpty() || Dy.isEmpty() || Dz.isEmpty()  || Xmin.isEmpty() || Ymin.isEmpty() || Zmin.isEmpty() || Xmax.isEmpty() || Ymax.isEmpty() || Zmax.isEmpty())
    {
        Toast::instance().show(Toast::TINFO, QStringLiteral("Data is incomplete"), this);
        return;
    }
    emit sigAddMatPoint(name);
    writeMatPointToYml(type, Dx, Dy, Dz, Xmin, Ymin,Zmin,Xmax,Ymax,Zmax);
    //组装json并发送信号
    writeJsonFile();
    accept(); // 关闭对话框并返回Accepted
}

void MatPointSetting::writeJsonFile()
{
    QString Xmin = ui->lineEdit_Xmin->text().trimmed();
    QString Ymin = ui->lineEdit_Ymin->text().trimmed();
    QString Zmin = ui->lineEdit_Zmin->text().trimmed();
    QString Xmax = ui->lineEdit_Xmax->text().trimmed();
    QString Ymax = ui->lineEdit_Ymax->text().trimmed();
    QString Zmax = ui->lineEdit_Zmax->text().trimmed();

    QString filePath = "config_temp.json";
    QJsonObject rootObj;
    QFile file(filePath);
    if (file.exists()) {
        if (file.open(QIODevice::ReadOnly)) {
            QByteArray jsonData = file.readAll();
            file.close();

            QJsonDocument doc = QJsonDocument::fromJson(jsonData);
            if (doc.isObject()) {
                rootObj = doc.object(); // 加载现有内容
            }
        }
    }
    QJsonObject pointObj;
    // 直接放入 QString，生成的 JSON 会带引号："-400e-6"
    pointObj.insert("min", QJsonArray({ Xmin, Ymin, Zmin }));
    pointObj.insert("max", QJsonArray({ Xmax, Ymax, Zmax }));

    QJsonObject powderObj;
    powderObj.insert("type", "read_file");
    powderObj.insert("file", "particle.vtk");

    rootObj.insert("Point", pointObj);
    rootObj.insert("powder", powderObj);
    // 保存到 JSON 文件
    if (file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        QJsonDocument saveDoc(rootObj);
        file.write(saveDoc.toJson(QJsonDocument::Indented));
        file.close();
        qDebug() << "Config saved successfully.";
    } else {
        qDebug() << "Failed to open file for writing:" << file.errorString();
    }
    emit sigJsonWriteFinish();
}
// 函数名修正为 writeMatPointToYml 以匹配 particle_region
void MatPointSetting::writeMatPointToYml(const QString& type, const QString& Dx, const QString& Dy, const QString& Dz,                               
                                         const QString& Xmin, const QString& Ymin, const QString& Zmin,
                                         const QString& Xmax, const QString& Ymax, const QString& Zmax)
{
    QString filePath = QDir::currentPath() + "/out.yml";
    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        // 如果文件不存在或无法读取，提示错误
        // Toast::instance().show(Toast::TINFO, QStringLiteral("Yml file open failed"), this);
        return;
    }

    QTextStream in(&file);
    QString fileContent = in.readAll();
    file.close();

    // 构建新的 point 段内容，使用 QString::arg 拼接
    QString newMatPointSection =
        QString("point:\n"
                "    type: %1\n" // type 不再是 block，而是由参数传入，但示例是 block
                "    min: [%2, %3, %4]\n"
                "    max: [%5, %6, %7]\n"
                "    size: [%8, %9, %10]\n")
            .arg(type)
            .arg(Xmin).arg(Ymin).arg(Zmin)
            .arg(Xmax).arg(Ymax).arg(Zmax)
            .arg(Dx).arg(Dy).arg(Dz);

    QStringList lines = fileContent.split('\n');

    int matPointStartIndex = -1;
    int matPointEndIndex = -1;

    // 1. 找到 particle_region: 所在的行
    for (int i = 0; i < lines.size(); ++i) {
        if (lines[i].trimmed() == "point:") {
            matPointStartIndex = i;
            break;
        }
    }

    if (matPointStartIndex != -1) {
        // 2. 从找到的行开始，向后查找，直到遇到下一个顶级字段（行首无缩进且不是空行/注释）
        // 或者文件结束
        matPointEndIndex = matPointStartIndex;
        for (int i = matPointStartIndex + 1; i < lines.size(); ++i) {
            QString line = lines[i];
            // 检查是否是顶级字段：行首不是空格或制表符，且不是空行或注释行（注释行以 # 开头）
            if (line.trimmed().isEmpty() || line.startsWith('#')) {
                // 空行或注释行，继续
                matPointEndIndex = i;
                continue;
            }
            // 检查行首是否有缩进
            if (line.startsWith(' ') || line.startsWith('\t')) {
                // 有缩进，属于当前块，继续
                matPointEndIndex = i;
                continue;
            } else {
                // 没有缩进，是下一个顶级字段，停止
                matPointEndIndex = i - 1; // 结束于上一行
                break;
            }
        }


        // 从后往前删除，避免索引变化
        for (int i = matPointEndIndex; i >= matPointStartIndex; --i) {
            lines.removeAt(i);
        }
        qDebug() << "Removed existing particle_region section from line" << matPointStartIndex << "to" << matPointEndIndex;
        qDebug() << "Lines count after removal:" << lines.size();

        // 4. 在原来的位置 (matPointStartIndex) 插入新的 particle_region 段
        QStringList newLines = newMatPointSection.split('\n');
        // 将新段的每一行插入到原来的位置
        int insertIndex = matPointStartIndex;
        for (const QString& newLine : newLines) {
            lines.insert(insertIndex++, newLine);
        }
        qDebug() << "Inserted new particle_region section at line" << matPointStartIndex;

    } else {
        // 如果没有找到 particle_region 段，则追加到文件末尾
        // 为了保持格式，先添加一个空行
        lines.append("");
        QStringList newLines = newMatPointSection.split('\n');
        lines.append(newLines);
        qDebug() << "Appended new particle_region section to the end.";
    }

    // 5. 重新组合文件内容并写入
    QString newFileContent = lines.join('\n');

    // 重新打开文件进行写入（覆盖）
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        // Toast::instance().show(Toast::TINFO, QStringLiteral("Write to yml file failed"), this);
        return;
    }

    QTextStream out(&file);
    out << newFileContent;
    file.close();
}

void MatPointSetting::on_pushButton_Cancel_clicked()
{
    close();
}

