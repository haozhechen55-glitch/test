#ifndef OUTPUTSETTINGDIALOG_H
#define OUTPUTSETTINGDIALOG_H

#include <QDialog>
#include "framelessbasedialog.h" // 1. 引入基类头文件

namespace Ui {
class OutputSettingDialog;
}

// 2. 修改继承关系为 FramelessBaseDialog
class OutputSettingDialog : public FramelessBaseDialog {
    Q_OBJECT

public:
    explicit OutputSettingDialog(QWidget *parent = nullptr);
    ~OutputSettingDialog();

    bool getWriteAllFields() const;
    bool getDetailedLog() const;
    // 【修改点 1】新增：只负责生成 YAML 字符串，不写文件
    QString getOutputYaml();

    // 【修改点 2】废弃：原有的写文件函数（建议保留声明但函数体留空，或者直接删除）
    void writeOutputToYml(bool writeAllFields, bool detailedLog);

private slots:
    void on_pushButton_ok_clicked();
    void on_pushButton_cancel_clicked();

private:
    Ui::OutputSettingDialog *ui;

    bool writeAllFields;
    bool detailedLog;

// 实现添加节点：
public:
    void setTargetNodeName(const QString &name);

signals:
    void sigAddOutput(QString name);

private:
    QString m_currentEditingNode;

};

#endif // OUTPUTSETTINGDIALOG_H
