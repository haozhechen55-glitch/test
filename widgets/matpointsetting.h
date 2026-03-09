#ifndef MATPOINTSETTING_H
#define MATPOINTSETTING_H

#include "framelessbasedialog.h"
namespace Ui {
class MatPointSetting;
}

class MatPointSetting : public FramelessBaseDialog
{
    Q_OBJECT

public:
    explicit MatPointSetting(QWidget *parent = nullptr);
    ~MatPointSetting();

    // 设置当前编辑的节点名称（空表示新建）
    void setTargetNodeName(const QString &name);

    void writeJsonFile();

    QString getYamlSection() const;

private slots:

    void on_pushButton_OK_clicked();

    void on_pushButton_Cancel_clicked();

signals:
    void sigAddMatPoint(QString name);

private:
    Ui::MatPointSetting *ui;
    QString m_currentEditingNode;
signals:
    void sigName(QString name);
    void sigJsonWriteFinish();
};

#endif // MATPOINTSETTING_H
