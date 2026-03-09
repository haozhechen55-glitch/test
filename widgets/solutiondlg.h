#ifndef SOLUTIONDLG_H
#define SOLUTIONDLG_H

#include <QDialog>
#include <QLineEdit>
#include "framelessbasedialog.h"

namespace Ui {
class SolutionDlg;
}

class SolutionDlg : public  FramelessBaseDialog
{
    Q_OBJECT

public:
    explicit SolutionDlg(QWidget *parent = nullptr);
    ~SolutionDlg();
    void saveSolver();

private slots:
    void on_pushButton_time_clicked();

    void on_pushButton_MPM_clicked();

    void on_pushButton_cancel1_clicked();

    void on_pushButton_cancel2_clicked();

private:
    Ui::SolutionDlg *ui;

// 实现添加节点
public:
    void setTargetNodeName(const QString &name);
    QString getSolverYaml(); // 获取头部 Solver/Model 设置
    QString getMPMYaml();    // 获取 MPM 参数设置
    QString getTimeYaml();   // 获取 Time 设置

signals:
    void sigAddSolution(QString name);

private:
    QString m_currentEditingNode;
    QLineEdit *lineEdit_interface_damp;
    QLineEdit *lineEdit_FEM_damp;
};

#endif // SOLUTIONDLG_H
