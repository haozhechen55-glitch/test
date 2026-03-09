// GravityDialog.h
#ifndef GRAVITYDIALOG_H
#define GRAVITYDIALOG_H

#include <QDialog>
#include"framelessbasedialog.h"

// 引入 UI 命名空间的前置声明
namespace Ui {
class GravityDialog;
}
class GravityDialog : public FramelessBaseDialog {
    Q_OBJECT

public:
    explicit GravityDialog(QWidget *parent = nullptr);
    ~GravityDialog();

    // 获取用户输入的重力值
    double getGravityX() const;
    double getGravityY() const;
    double getGravityZ() const;

    // 获取名称（可选，不写入 YAML）
    QString getName() const;

    // 返回 gravity: YAML 段落
    QString getYamlSection() const;

    // 从 UI 控件同步内部成员变量（反序列化后调用）
    void syncFromUI();

private slots:
    void on_pushButton_ok_clicked();
    void on_pushButton_cancel_clicked();

private:
    Ui::GravityDialog *ui; // 唯一的 UI 指针

    double gravityX, gravityY, gravityZ;
    QString name;
    // 实现添加节点功能
public:
    // 【标准接口】设置编辑目标
    void setTargetNodeName(const QString &name);

signals:
    // 【标准信号】通知主界面添加/修改节点
    void sigAddGravity(QString name);

private:
    // 【标准变量】记录当前正在编辑的节点名（用于区分新建/编辑或回显）
    QString m_currentEditingNode;
};

#endif // GRAVITYDIALOG_H
