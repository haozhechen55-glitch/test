// GravityDialog.cpp
#include "GravityDialog.h"
#include "ui_gravitydialog.h"
#include "toastdlg.h"
#include <QDir>
#include <QTextStream>
#include <QFile>
#include<QDebug>
GravityDialog::GravityDialog(QWidget *parent)
    : FramelessBaseDialog(parent)
    , ui(new Ui::GravityDialog) // 初始化列表创建 ui 实例
    , gravityX(0.0), gravityY(0.0), gravityZ(-9.8), name("Load1")
{
    // 1. 设置 UI
    ui->setupUi(this);

    // 2. 关键步骤：将 mainContainer 从 ui 中拿出来，放入 FramelessBaseDialog 的内容区域
    // 类似于 SolutionDlg 的处理方式
    if (ui->mainContainer) {
        this->contentLayout()->addWidget(ui->mainContainer);
        ui->mainContainer->setVisible(true);
    }

    // 3. 设置窗口基本属性
    setFixedSize(300, 300);
    setWindowTitleText("Set Gravity");

    // 注意：不再需要手动的 connect，因为我们在 .h 中使用了 on_pushButton_ok_clicked 命名
    // Qt 的 QMetaObject::connectSlotsByName(this) 会在 setupUi 中自动处理连接
}

GravityDialog::~GravityDialog()
{
    delete ui; // 别忘了清理内存
}

double GravityDialog::getGravityX() const { return gravityX; }
double GravityDialog::getGravityY() const { return gravityY; }
double GravityDialog::getGravityZ() const { return gravityZ; }
QString GravityDialog::getName() const { return name; }

void GravityDialog::on_pushButton_ok_clicked()
{
    // 获取并验证 X、Y、Z
    bool ok;
    QString xStr = ui->lineEdit_x->text().trimmed();
    QString yStr = ui->lineEdit_y->text().trimmed();
    QString zStr = ui->lineEdit_z->text().trimmed();

    if (xStr.isEmpty() || yStr.isEmpty() || zStr.isEmpty()) {
        Toast::instance().show(Toast::TINFO, "Please fill all X, Y, Z fields.", this);
        return;
    }

    double x = xStr.toDouble(&ok);
    if (!ok) {
        Toast::instance().show(Toast::TINFO, "Invalid number format for X.", this);
        return;
    }

    double y = yStr.toDouble(&ok);
    if (!ok) {
        Toast::instance().show(Toast::TINFO, "Invalid number format for Y.", this);
        return;
    }

    double z = zStr.toDouble(&ok);
    if (!ok) {
        Toast::instance().show(Toast::TINFO, "Invalid number format for Z.", this);
        return;
    }

    gravityX = x;
    gravityY = y;
    gravityZ = z;
    name = ui->lineEdit_name->text().trimmed(); // 可选保存名称

    QString filePath = QDir::currentPath() + "/out.yml";
    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        Toast::instance().show(Toast::TINFO, "Failed to open out.yml for reading.", this);
        return;
    }

    QTextStream in(&file);
    QString fileContent = in.readAll();
    file.close();

    // 构建新的 gravity 行
    QString newLine = QString("gravity: [%1, %2, %3]").arg(gravityX).arg(gravityY).arg(gravityZ);

    QStringList lines = fileContent.split('\n');

    int gravityStartIndex = -1;
    int gravityEndIndex = -1;

    // 1. 找到 gravity: 行
    for (int i = 0; i < lines.size(); ++i) {
        if (lines[i].trimmed().startsWith("gravity:")) {
            gravityStartIndex = i;
            break;
        }
    }

    if (gravityStartIndex != -1) {
        // 2. 替换该行
        lines[gravityStartIndex] = newLine;
        qDebug() << "Updated gravity to" << gravityX << gravityY << gravityZ;
    } else {
        // 3. 如果没有找到，追加到文件末尾
        lines.append(""); // 空行分隔
        lines.append(newLine);
        qDebug() << "Appended new gravity line.";
    }

    // 4. 写回文件
    QString newFileContent = lines.join('\n');
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        Toast::instance().show(Toast::TINFO, "Failed to write to out.yml.", this);
        return;
    }

    QTextStream out(&file);
    out << newFileContent;
    file.close();

    Toast::instance().show(Toast::TINFO, QString("Gravity set to [%1, %2, %3]").arg(gravityX).arg(gravityY).arg(gravityZ), this);

    QString finalName = this->name.isEmpty() ? (m_currentEditingNode.isEmpty() ? "Gravity" : m_currentEditingNode) : this->name;

    emit sigAddGravity(finalName);

    accept(); // 关闭对话框并返回 Accepted
}

void GravityDialog::on_pushButton_cancel_clicked()
{
    reject(); // 关闭对话框并返回 Rejected
}

// 【实现接口】
void GravityDialog::setTargetNodeName(const QString &name) {
    m_currentEditingNode = name;
    if (name.isEmpty()) setWindowTitleText("Add Load");
    else setWindowTitleText("Edit " + name);
}
