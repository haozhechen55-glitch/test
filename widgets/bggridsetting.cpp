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

    writeBackgroundMeshToYml(xmin, ymin, zmin, xmax, ymax, zmax, x, y, z);
    writeJsonFile();
    //设置名称
    emit sigName("grid1");
    accept(); // 关闭对话框并返回Accepted
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
    // === 【修改】: 只有当 particle.vtk 真实存在时，才写入 powder 配置 ===
    // 这样空项目打开时，预览程序就不会去读不存在的文件，也就不会弹窗报错了
    if (QFile::exists("particle.vtk")) {
        QJsonObject powderObj;
        powderObj.insert("type", "read_file");
        powderObj.insert("file", "particle.vtk");
        rootObj.insert("powder", powderObj);
    }
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

void BGgridSetting::writeBackgroundMeshToYml(const QString& xmin, const QString& ymin, const QString& zmin,
                                             const QString& xmax, const QString& ymax, const QString& zmax,
                                             const QString& nx, const QString& ny, const QString& nz)
{
    QString filePath = QDir::currentPath() + "/out.yml";
    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        // 如果文件不存在或无法读取，提示错误
        Toast::instance().show(Toast::TINFO, QStringLiteral("Yml file open failed"), this);
        return;
    }

    QTextStream in(&file);
    QString fileContent = in.readAll();
    file.close();

    // 构建新的 background_mesh 段内容，使用 QString::arg 拼接
    QString newBackgroundMeshSection =
        QString("background_mesh:\n"
                "    min: [%1, %2, %3]\n"
                "    max: [%4, %5, %6]\n"
                "    divide: [%7, %8, %9]\n"
                "    fixed_boundary:\n"
                "      - direction: [x]  \n"
                "        location: [xmin,xmax] \n"
                "      - direction: [y]  \n"
                "        location: [ymin,ymax] \n"
                "      - direction: [z]  \n"
                "        location: [zmin] \n")
            .arg(xmin).arg(ymin).arg(zmin)
            .arg(xmax).arg(ymax).arg(zmax)
            .arg(nx).arg(ny).arg(nz);

    QStringList lines = fileContent.split('\n');

    int bgMeshStartIndex = -1;
    int bgMeshEndIndex = -1;

    for (int i = 0; i < lines.size(); ++i) {
        if (lines[i].trimmed() == "background_mesh:") {
            bgMeshStartIndex = i;
            break;
        }
    }

    if (bgMeshStartIndex != -1) {
        // 2. 从找到的行开始，向后查找，直到遇到下一个顶级字段（行首无缩进且不是空行/注释）
        // 或者文件结束
        bgMeshEndIndex = bgMeshStartIndex;
        for (int i = bgMeshStartIndex + 1; i < lines.size(); ++i) {
            QString line = lines[i];
            // 检查是否是顶级字段：行首不是空格或制表符，且不是空行或注释行（注释行以 # 开头）
            // 注意：这里假设顶级字段不以 # 开头
            if (line.trimmed().isEmpty() || line.startsWith('#')) {
                // 空行或注释行，继续
                bgMeshEndIndex = i;
                continue;
            }
            // 检查行首是否有缩进
            if (line.startsWith(' ') || line.startsWith('\t')) {
                // 有缩进，属于当前块，继续
                bgMeshEndIndex = i;
                continue;
            } else {
                // 没有缩进，是下一个顶级字段，停止
                bgMeshEndIndex = i - 1; // 结束于上一行
                break;
            }
        }

        // 3. 删除找到的 background_mesh 段 (从 bgMeshStartIndex 到 bgMeshEndIndex)
        // 从后往前删除，避免索引变化
        for (int i = bgMeshEndIndex; i >= bgMeshStartIndex; --i) {
            lines.removeAt(i);
        }
        qDebug() << "Removed existing background_mesh section from line" << bgMeshStartIndex << "to" << bgMeshEndIndex;
        qDebug() << "Lines count after removal:" << lines.size();

        // 4. 在原来的位置 (bgMeshStartIndex) 插入新的 background_mesh 段
        QStringList newLines = newBackgroundMeshSection.split('\n');
        // 将新段的每一行插入到原来的位置
        int insertIndex = bgMeshStartIndex;
        for (const QString& newLine : newLines) {
            lines.insert(insertIndex++, newLine);
        }
        qDebug() << "Inserted new background_mesh section at line" << bgMeshStartIndex;

    } else {
        // 如果没有找到 background_mesh 段，则追加到文件末尾
        // 为了保持格式，先添加一个空行
        lines.append("");
        QStringList newLines = newBackgroundMeshSection.split('\n');
        lines.append(newLines);
        qDebug() << "Appended new background_mesh section to the end.";
    }

    // 5. 重新组合文件内容并写入
     QString newFileContent = lines.join('\n');

    // 重新打开文件进行写入（覆盖）
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        Toast::instance().show(Toast::TINFO, QStringLiteral("Write to yml file failed"), this);
        return;
    }

    QTextStream out(&file);
    out << newFileContent;
    file.close();
}
