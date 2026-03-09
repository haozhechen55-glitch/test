#include "bggridsetting.h"
#include "ui_bggridsetting.h"
#include"toastdlg.h"
#include <QDir>
#include <QTextStream>
#include <QFile>
#include<QDebug>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>


BGgridSetting::BGgridSetting(QWidget *parent)
    : FramelessBaseDialog(parent)
    , ui(new Ui::BGgridSetting)
{
    ui->setupUi(this);
    if (ui->verticalLayout_8) {
        this->contentLayout()->addLayout(ui->verticalLayout_8);
    }
    // 3. 设置标题栏内容
    setWindowTitleText("Background Grid Settings");
    // setWindowIconImage(QIcon(":/icons/grid.png")); // 根据需要设置图标
}

BGgridSetting::~BGgridSetting()
{
    delete ui;
}

void BGgridSetting::setTargetNodeName(const QString &name)
{
    m_currentEditingNode = name;
    if (name.isEmpty()) {
        setWindowTitleText("Background Grid Settings");
    } else {
        setWindowTitleText("Edit Grid: " + name);
    }
}

void BGgridSetting::on_comboBox_currentIndexChanged(int index)
{
    if(index == 0)
    {
        ui->label_x->setText("DCell dx");
        ui->label_y->setText("DCell dy");
        ui->label_z->setText("DCell dz");
    }

    if(index == 1)
    {
        ui->label_x->setText("x");
        ui->label_y->setText("y");
        ui->label_z->setText("z");

    }

}


void BGgridSetting::on_pushButton_OK_clicked()
{
    // 收集所有输入值
    QString xmin = ui->lineEdit_XMin->text().trimmed();
    QString ymin = ui->lineEdit_YMin->text().trimmed();
    QString zmin = ui->lineEdit_ZMin->text().trimmed();
    QString xmax = ui->lineEdit_XMax->text().trimmed();
    QString ymax = ui->lineEdit_YMax->text().trimmed();
    QString zmax = ui->lineEdit_ZMax->text().trimmed();
    QString x = ui->lineEdit_x->text().trimmed();
    QString y = ui->lineEdit_y->text().trimmed();
    QString z = ui->lineEdit_z->text().trimmed();

    if (xmin.isEmpty() || ymin.isEmpty() || zmin.isEmpty() ||xmax.isEmpty() || ymax.isEmpty() || zmax.isEmpty()||x.isEmpty()|| y.isEmpty()|| z.isEmpty())
    {
        Toast::instance().show(Toast::TINFO, QStringLiteral("Data is incomplete"), this);
        return;
    }

    writeJsonFile();
    emit sigName("Grid");
    accept();
}


void BGgridSetting::on_pushButton_Cancel_clicked()
{
    close();
}

void BGgridSetting::writeJsonFile()
{

    QString xmin = ui->lineEdit_XMin->text().trimmed();
    QString ymin = ui->lineEdit_YMin->text().trimmed();
    QString zmin = ui->lineEdit_ZMin->text().trimmed();
    QString xmax = ui->lineEdit_XMax->text().trimmed();
    QString ymax = ui->lineEdit_YMax->text().trimmed();
    QString zmax = ui->lineEdit_ZMax->text().trimmed();
    QString x = ui->lineEdit_x->text().trimmed();
    QString y = ui->lineEdit_y->text().trimmed();
    QString z = ui->lineEdit_z->text().trimmed();

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
    QJsonObject background_meshobj;
    // 直接放入 QString，生成的 JSON 会带引号："-400e-6"
    background_meshobj.insert("min", QJsonArray({ xmin, ymin, zmin }));
    background_meshobj.insert("max", QJsonArray({ xmax, ymax, zmax }));
    background_meshobj.insert("divide", QJsonArray({ x, y, z }));

    QJsonObject powderObj;
    powderObj.insert("type", "read_file");
    powderObj.insert("file", "particle.vtk");

    rootObj.insert("background_mesh", background_meshobj);

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

QString BGgridSetting::getYamlSection(const QString &fixedBoundaryYaml) const
{
    QString xmin = ui->lineEdit_XMin->text().trimmed();
    QString ymin = ui->lineEdit_YMin->text().trimmed();
    QString zmin = ui->lineEdit_ZMin->text().trimmed();
    QString xmax = ui->lineEdit_XMax->text().trimmed();
    QString ymax = ui->lineEdit_YMax->text().trimmed();
    QString zmax = ui->lineEdit_ZMax->text().trimmed();
    QString nx = ui->lineEdit_x->text().trimmed();
    QString ny = ui->lineEdit_y->text().trimmed();
    QString nz = ui->lineEdit_z->text().trimmed();

    if (xmin.isEmpty() || nx.isEmpty()) return QString();

    QString yaml;
    yaml += "background_mesh:\n";
    yaml += "    min: [" + xmin + ", " + ymin + ", " + zmin + "]\n";
    yaml += "    max: [" + xmax + ", " + ymax + ", " + zmax + "]\n";
    yaml += "    divide: [" + nx + ", " + ny + ", " + nz + "]\n";
    if (!fixedBoundaryYaml.isEmpty()) {
        yaml += fixedBoundaryYaml;
    }
    return yaml;
}
