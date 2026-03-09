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

void GravityDialog::syncFromUI()
{
    bool ok;
    double x = ui->lineEdit_x->text().trimmed().toDouble(&ok);
    if (ok) gravityX = x;
    double y = ui->lineEdit_y->text().trimmed().toDouble(&ok);
    if (ok) gravityY = y;
    double z = ui->lineEdit_z->text().trimmed().toDouble(&ok);
    if (ok) gravityZ = z;
    name = ui->lineEdit_name->text().trimmed();
}

void GravityDialog::on_pushButton_ok_clicked()
{
    bool ok;
    QString xStr = ui->lineEdit_x->text().trimmed();
    QString yStr = ui->lineEdit_y->text().trimmed();
    QString zStr = ui->lineEdit_z->text().trimmed();

    if (xStr.isEmpty() || yStr.isEmpty() || zStr.isEmpty()) {
        Toast::instance().show(Toast::TINFO, "Please fill all X, Y, Z fields.", this);
        return;
    }

    double x = xStr.toDouble(&ok);
    if (!ok) { Toast::instance().show(Toast::TINFO, "Invalid number format for X.", this); return; }
    double y = yStr.toDouble(&ok);
    if (!ok) { Toast::instance().show(Toast::TINFO, "Invalid number format for Y.", this); return; }
    double z = zStr.toDouble(&ok);
    if (!ok) { Toast::instance().show(Toast::TINFO, "Invalid number format for Z.", this); return; }

    gravityX = x;
    gravityY = y;
    gravityZ = z;
    name = ui->lineEdit_name->text().trimmed();

    Toast::instance().show(Toast::TINFO, QString("Gravity set to [%1, %2, %3]").arg(gravityX).arg(gravityY).arg(gravityZ), this);

    QString finalName = this->name.isEmpty() ? (m_currentEditingNode.isEmpty() ? "Gravity" : m_currentEditingNode) : this->name;
    emit sigAddGravity(finalName);
    accept();
}

QString GravityDialog::getYamlSection() const
{
    return QString("gravity: [%1, %2, %3]\n").arg(gravityX).arg(gravityY).arg(gravityZ);
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
