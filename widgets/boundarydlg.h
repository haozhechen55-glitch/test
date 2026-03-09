#ifndef BOUNDARYDLG_H
#define BOUNDARYDLG_H

#include"framelessbasedialog.h"

namespace Ui {
class BoundaryDlg;
}

class BoundaryDlg : public FramelessBaseDialog
{
    Q_OBJECT

public:
    explicit BoundaryDlg(QWidget *parent = nullptr);
    ~BoundaryDlg();

    // 设置当前编辑的节点名称（空表示新建）
    void setTargetNodeName(const QString &name);

    // 返回 fixed_boundary YAML 片段（含4空格缩进，嵌入 background_mesh 段）
    QString getFixedBoundaryYaml() const;
    // 返回完整 field: 顶层段落
    QString getFieldYamlSection(const QString &initialTemp = "300") const;

private slots:
    void on_pushButton_OK_mechanical_clicked();

    void on_pushButton_cancle_clicked();

    void on_pushButton_OK_Temperature_clicked();

    void on_pushButton_cancel_2_clicked();

    void onBoundaryTypeChanged();

private:
    Ui::BoundaryDlg *ui;
    QString m_currentEditingNode;
signals:
    void sigName(QString name);
};

#endif // BOUNDARYDLG_H
