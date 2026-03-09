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
    void writeMatPointToYml(const QString& type, const QString& Dx, const QString& Dy, const QString& Dz,
                                         const QString& Xmin, const QString& Ymin, const QString& Zmin,
                                        const QString& Xmax, const QString& Ymax, const QString& Zmax);

    void writeJsonFile();
private slots:

    void on_pushButton_OK_clicked();

    void on_pushButton_Cancel_clicked();

signals:
    void sigAddMatPoint(QString name);

private:
    Ui::MatPointSetting *ui;
signals:
    void sigName(QString name);
    void sigJsonWriteFinish();
};

#endif // MATPOINTSETTING_H
