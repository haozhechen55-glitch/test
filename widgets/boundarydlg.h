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

private slots:
    void on_pushButton_OK_mechanical_clicked();

    void on_pushButton_cancle_clicked();

    void on_pushButton_OK_Temperature_clicked();

    void on_pushButton_cancel_2_clicked();

    void onBoundaryTypeChanged();

private:
    Ui::BoundaryDlg *ui;
signals:
    void sigName(QString name);
};

#endif // BOUNDARYDLG_H
