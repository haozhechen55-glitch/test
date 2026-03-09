#ifndef POWDERDLG_H
#define POWDERDLG_H

#include "framelessbasedialog.h"
#include <QWidget>

namespace Ui {
class PowderDlg;
}

class PowderDlg : public FramelessBaseDialog
{
    Q_OBJECT

public:
    explicit PowderDlg(QWidget *parent = nullptr);
    ~PowderDlg();

    // 设置在编辑模式下显示的节点名称
    void setTargetNodeName(const QString &name);
    void writeJsonFile();

signals:
    // 通知主界面添加/更新树节点
    void sigAddPowder(QString name);
    // 通知主界面 YAML 已更新，可以刷新预览
    void sigJsonWriteFinish();

private slots:
    // 保存按钮槽函数
    void on_pushButton_save_clicked();
    // 浏览文件按钮槽函数
    void on_toolButton_browse_clicked();

private:
    Ui::PowderDlg *ui;
    QString m_currentEditingNode;

    // 写入 YAML 的核心逻辑
    void writeToYaml();
};

#endif // POWDERDLG_H
