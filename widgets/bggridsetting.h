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
    //获取当前设置的值
    void getGridValues(double &xmin, double &ymin, double &zmin,
                       double &xmax, double &ymax, double &zmax,
                       double &dx, double &dy, double &dz);
    void writeBackgroundMeshToYml(const QString& xmin, const QString& ymin, const QString& zmin,
                                                 const QString& xmax, const QString& ymax, const QString& zmax,
                                  const QString& nx, const QString& ny, const QString& nz);

    void writeJsonFile();
private slots:
    void on_comboBox_currentIndexChanged(int index);

    void on_pushButton_OK_clicked();

    void on_pushButton_Cancel_clicked();

private:
    Ui::BGgridSetting *ui;
signals:
    void sigName(QString name);
    void sigJsonWriteFinish();
};

#endif // BGGRIDSETTING_H
