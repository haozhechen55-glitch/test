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

void MatPointSetting::setTargetNodeName(const QString &name)
{
    m_currentEditingNode = name;
    if (name.isEmpty()) {
        setWindowTitleText("Material Point Settings");
    } else {
        setWindowTitleText("Edit Mat-Point: " + name);
    }
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
QString MatPointSetting::getYamlSection() const
{
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

    if (type.isEmpty() || Dx.isEmpty()) return QString();

    QString yaml;
    yaml += "point:\n";
    yaml += "    type: " + type + "\n";
    yaml += "    min: [" + Xmin + ", " + Ymin + ", " + Zmin + "]\n";
    yaml += "    max: [" + Xmax + ", " + Ymax + ", " + Zmax + "]\n";
    yaml += "    size: [" + Dx + ", " + Dy + ", " + Dz + "]\n";
    return yaml;
}

void MatPointSetting::on_pushButton_Cancel_clicked()
{
    close();
}

