#include "temperaturevaluedialog.h"
#include <QTableWidgetItem>
#include <QDoubleValidator>
#include <QDebug>
#include"toastdlg.h"
#include<QHeaderView>
TemperatureValueDialog::TemperatureValueDialog(QWidget *parent)
    : QDialog(parent), outputString("") {
    setWindowTitle("Temperature Value<T,E>");
    setFixedSize(600, 400);

    tableWidget = new QTableWidget(50, 2, this); // 10行2列
    tableWidget->setHorizontalHeaderLabels(QStringList() << "Temperature" << "Value");
    tableWidget->horizontalHeader()->setStretchLastSection(true);
    tableWidget->verticalHeader()->setVisible(false);

    // 设置默认值（可选）
    for (int i = 0; i < 10; ++i) {
        QTableWidgetItem *tempItem = new QTableWidgetItem("");
        QTableWidgetItem *valueItem = new QTableWidgetItem("");
        tableWidget->setItem(i, 0, tempItem);
        tableWidget->setItem(i, 1, valueItem);
    }

    okButton = new QPushButton("OK", this);
    cancelButton = new QPushButton("Cancel", this);

    buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);

    mainLayout = new QVBoxLayout(this);
    //mainLayout->addWidget(new QLabel("Material", this));
    mainLayout->addWidget(tableWidget);
    mainLayout->addLayout(buttonLayout);

    connect(okButton, &QPushButton::clicked, this, &TemperatureValueDialog::onOkClicked);
    connect(cancelButton, &QPushButton::clicked, this, &TemperatureValueDialog::onCancelClicked);
}

TemperatureValueDialog::~TemperatureValueDialog() {}

QString TemperatureValueDialog::getOutput() const {
    return outputString;
}

void TemperatureValueDialog::onOkClicked() {
    QVector<double> temps, values;

    for (int row = 0; row < 50; ++row) {
        QString tempStr = tableWidget->item(row, 0) ? tableWidget->item(row, 0)->text().trimmed() : "";
        QString valStr = tableWidget->item(row, 1) ? tableWidget->item(row, 1)->text().trimmed() : "";

        if (!tempStr.isEmpty() && !valStr.isEmpty()) {
            bool ok1, ok2;
            double temp = tempStr.toDouble(&ok1);
            double val = valStr.toDouble(&ok2);

            if (ok1 && ok2) {
                temps.append(temp);
                values.append(val);
            } else {
                Toast::instance().show(Toast::TINFO, QString("Please ensure that both temperature and value are numbers"), this);
                return;
            }
        } else if (!tempStr.isEmpty() || !valStr.isEmpty()) {
            Toast::instance().show(Toast::TINFO, QString("Incomplete input"), this);
            return;
        }
    }

    // 生成输出字符串
    QString output = " [";
    for (int i = 0; i < temps.size(); ++i) {
        if (i > 0) output += ",";
        output += QString::number(temps[i]);
    }
    output += "] /  [";
    for (int i = 0; i < values.size(); ++i) {
        if (i > 0) output += ",";
        output += QString::number(values[i]);
    }
    output += "]";

    outputString = output; // 保存到成员变量
    qDebug() << output; // 输出到控制台

    accept(); // 关闭对话框并返回成功
}

void TemperatureValueDialog::onCancelClicked() {
    reject(); // 关闭对话框并返回取消
}

