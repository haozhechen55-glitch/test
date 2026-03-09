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

    QString filePath = QDir::currentPath() + "/out.yml";
    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        Toast::instance().show(Toast::TINFO, "Failed to open out.yml for reading.", this);
        return;
    }

    QTextStream in(&file);
    QString fileContent = in.readAll();
    file.close();

    // 构建新的 reference_temperature 行
    QString newLine = QString("    reference_temperature: %1").arg(temperatureValue);

    QStringList lines = fileContent.split('\n');

    int refTempStartIndex = -1;
    int refTempEndIndex = -1;

    // 1. 找到 material: 下的 reference_temperature: 行
    // 我们先找到 material: 开头的行
    int materialStartIndex = -1;
    for (int i = 0; i < lines.size(); ++i) {
        if (lines[i].trimmed() == "material:") {
            materialStartIndex = i;
            break;
        }
    }

    if (materialStartIndex == -1) {
        Toast::instance().show(Toast::TINFO, "material: section not found in out.yml.", this);
        return;
    }

    // 2. 从 material: 开始，查找 reference_temperature: 行
    for (int i = materialStartIndex + 1; i < lines.size(); ++i) {
        QString line = lines[i];
        // 检查是否是 reference_temperature: 行（必须缩进）
        if (line.startsWith(' ') && line.contains("reference_temperature:")) {
            refTempStartIndex = i;
            break;
        }
    }

    if (refTempStartIndex != -1) {
        // 3. 替换该行
        lines[refTempStartIndex] = newLine;
        qDebug() << "Updated reference_temperature to" << temperatureValue;
    } else {
        // 4. 如果没有找到，尝试插入到 material: 段内
        // 找到 material: 段结束位置（下一个顶级字段或文件末尾）
        int materialEndIndex = materialStartIndex;
        for (int i = materialStartIndex + 1; i < lines.size(); ++i) {
            QString line = lines[i];
            if (line.trimmed().isEmpty() || line.startsWith('#')) {
                materialEndIndex = i;
                continue;
            }
            if (line.startsWith(' ') || line.startsWith('\t')) {
                materialEndIndex = i;
                continue;
            } else {
                materialEndIndex = i - 1;
                break;
            }
        }

        // 插入到 material 段末尾
        lines.insert(materialEndIndex + 1, newLine);
        qDebug() << "Inserted reference_temperature at line" << materialEndIndex + 1;
    }

    // 5. 写回文件
    QString newFileContent = lines.join('\n');
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        Toast::instance().show(Toast::TINFO, "Failed to write to out.yml.", this);
        return;
    }

    QTextStream out(&file);
    out << newFileContent;
    file.close();

    Toast::instance().show(Toast::TINFO, QString("Reference temperature set to %1").arg(temperatureValue), this);

    QString finalName = m_currentEditingNode.isEmpty() ? "InitTemp" : m_currentEditingNode;
    emit sigAddInitTemp(finalName);
    accept();
}

void InitialTemperatureDialog::onCancelClicked() {
    reject(); // 关闭对话框并返回 Rejected
}

void InitialTemperatureDialog::setTargetNodeName(const QString &name) {
    m_currentEditingNode = name;
}
