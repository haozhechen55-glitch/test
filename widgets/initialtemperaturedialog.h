// InitialTemperatureDialog.h
#ifndef INITIALTEMPERATUREDIALOG_H
#define INITIALTEMPERATUREDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include"framelessbasedialog.h"

namespace Ui {
class initialtemperaturedialog;
}

class InitialTemperatureDialog : public FramelessBaseDialog {
    Q_OBJECT

public:
    explicit InitialTemperatureDialog(QWidget *parent = nullptr);
    ~InitialTemperatureDialog();

    // 获取用户输入的温度值
    double getTemperature() const;

private slots:
    void onOkClicked();
    void onCancelClicked();

private:
    Ui::initialtemperaturedialog *ui;
    QWidget *mainContainer;

    double temperatureValue; // 存储最终值

    // 实现添加节点
public:
    void setTargetNodeName(const QString &name);

signals:
    void sigAddInitTemp(QString name);

private:
    QString m_currentEditingNode;
};

#endif // INITIALTEMPERATUREDIALOG_H
