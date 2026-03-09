// InitialTemperatureDialog.cpp
#include "InitialTemperatureDialog.h"
#include <QLabel>
#include <QMessageBox>
#include "toastdlg.h"
#include <QDir>
#include <QTextStream>
#include <QFile>
#include<QDebug>
#include "ui_initialtemperaturedialog.h" // 引入生成的头文件

InitialTemperatureDialog::InitialTemperatureDialog(QWidget *parent)
    : FramelessBaseDialog(parent)
    , ui(new Ui::initialtemperaturedialog) // 初始化 ui
    , temperatureValue(0.0)
{
    // 1. 设置标题（基类方法）
    setWindowTitleText("Set Initial Temperature");

    // 2. 实例化 mainContainer
    mainContainer = new QWidget(this);
    mainContainer->setObjectName("mainContainer"); // 方便样式表定位

    // 3. 【嫁接】将 UI 界面构建在 mainContainer 上
    // 注意：不是 ui->setupUi(this)，因为 this 已经被 FramelessBaseDialog 的布局占用了
    ui->setupUi(mainContainer);

    // 4. 将 mainContainer 加入到基类的内容布局中
    // 这样你的 UI 界面就会显示在无边框窗口的中间区域
    this->contentLayout()->addWidget(mainContainer);

    // 5. 调整窗口大小
    this->resize(320, 310);
    // 6. 连接信号槽
    // 注意：现在控件都在 ui 指针里面，例如 ui->okButton
    connect(ui->okButton, &QPushButton::clicked, this, &InitialTemperatureDialog::onOkClicked);
    connect(ui->cancelButton, &QPushButton::clicked, this, &InitialTemperatureDialog::onCancelClicked);
}

InitialTemperatureDialog::~InitialTemperatureDialog() {
    delete ui;
}

double InitialTemperatureDialog::getTemperature() const {
    return temperatureValue;
}

void InitialTemperatureDialog::syncFromUI()
{
    bool ok;
    double temp = ui->valueEdit->text().trimmed().toDouble(&ok);
    if (ok) temperatureValue = temp;
}

void InitialTemperatureDialog::onOkClicked()
{
    QString valueStr = ui->valueEdit->text().trimmed();
    if (valueStr.isEmpty()) {
        Toast::instance().show(Toast::TINFO, "Please enter a valid temperature value.", this);
        return;
    }

    bool ok;
    double temp = valueStr.toDouble(&ok);
    if (!ok) {
        Toast::instance().show(Toast::TINFO, "Invalid number format. Please enter a numeric value.", this);
        return;
    }

    temperatureValue = temp;

    Toast::instance().show(Toast::TINFO, QString("Reference temperature set to %1").arg(temperatureValue), this);

    QString finalName = m_currentEditingNode.isEmpty() ? "InitTemp" : m_currentEditingNode;
    emit sigAddInitTemp(finalName);
    accept();
}

QString InitialTemperatureDialog::getReferenceTemperature() const
{
    if (temperatureValue == 0.0) return QString();
    return QString::number(temperatureValue);
}

void InitialTemperatureDialog::onCancelClicked() {
    reject(); // 关闭对话框并返回 Rejected
}

void InitialTemperatureDialog::setTargetNodeName(const QString &name) {
    m_currentEditingNode = name;
}
