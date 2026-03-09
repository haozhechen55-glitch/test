#ifndef BGGRIDSETTING_H
#define BGGRIDSETTING_H

#include <QDialog>
#include"framelessbasedialog.h"
namespace Ui {
class BGgridSetting;
}

class BGgridSetting : public FramelessBaseDialog
{
    Q_OBJECT

public:
    explicit BGgridSetting(QWidget *parent = nullptr);
    ~BGgridSetting();

    // 设置当前编辑的节点名称（空表示新建）
    void setTargetNodeName(const QString &name);

    //获取当前设置的值
    void getGridValues(double &xmin, double &ymin, double &zmin,
                       double &xmax, double &ymax, double &zmax,
                       double &dx, double &dy, double &dz);
    void writeJsonFile();

    QString getYamlSection(const QString &fixedBoundaryYaml = QString()) const;

private slots:
    void on_comboBox_currentIndexChanged(int index);

    void on_pushButton_OK_clicked();

    void on_pushButton_Cancel_clicked();

private:
    Ui::BGgridSetting *ui;
    QString m_currentEditingNode;
signals:
    void sigName(QString name);
    void sigJsonWriteFinish();
};

#endif // BGGRIDSETTING_H
