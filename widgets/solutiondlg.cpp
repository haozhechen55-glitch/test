#include "solutiondlg.h"
#include "ui_solutiondlg.h"
#include"toastdlg.h"
#include <QDir>
#include <QTextStream>
#include <QFile>
#include<QDebug>

SolutionDlg::SolutionDlg(QWidget *parent)
    : FramelessBaseDialog(parent)
    , ui(new Ui::SolutionDlg)
{
    ui->setupUi(this);
    if (ui->maincontainer_sol) {
        // 这一步会将 mainContainer 从原来的位置“移动”到内容布局中
        this->contentLayout()->addWidget(ui->maincontainer_sol);

        // 确保容器可见（有时候从 setupUi 出来默认可能是隐藏的，视 UI 设置而定）
        ui->maincontainer_sol->setVisible(true);
    }
    // 3. 设置标题栏文本
    setWindowTitleText("Solution");
}

SolutionDlg::~SolutionDlg()
{
    delete ui;
}

// 实现 getSolverYaml
QString SolutionDlg::getSolverYaml()
{
    QString solver   = ui->comboBox_solver->currentText();
    QString model    = ui->comboBox_model->currentText();
    QString fluidStr = ui->comboBox_solve_fluid->currentText();
    QString solidStr = ui->comboBox_solve_solid->currentText();

    // 构造头部字符串
    return QString("# \n"
                   "\n"
                   "solver: %1\n"
                   "model: %2\n"
                   "\n"
                   "solve_fluid: %3\n"
                   "solve_solid: %4\n")
        .arg(solver).arg(model).arg(fluidStr).arg(solidStr);
}

// 实现 getMPMYaml
QString SolutionDlg::getMPMYaml()
{
    QString ppc1 = ui->lineEdit_point_per_cell_1->text().trimmed();
    QString ppc2 = ui->lineEdit_point_per_cell_2->text().trimmed();
    QString ppc3 = ui->lineEdit_point_per_cell_3->text().trimmed();
    bool rearrange = ui->checkBox_rearrange->isChecked();
    QString minPpc = ui->lineEdit_min_point_per_cell->text().trimmed();
    QString maxPpc = ui->lineEdit_max_point_per_cell->text().trimmed();
    QString MPM_damp = ui->lineEdit_MPM_damp->text().trimmed();
    QString PIC_damp = ui->lineEdit_PIC_damp->text().trimmed();
    // 默认值，界面上如果没有对应控件，可以硬编码或添加控件
    QString interface_damp = "0";
    QString FEM_damp = "0";

    if (ppc1.isEmpty()) ppc1 = "3"; // 防止为空的保护机制
    if (ppc2.isEmpty()) ppc2 = "3";
    if (ppc3.isEmpty()) ppc3 = "3";

    return QString("interface_damp: %1\n"
                   "MPM_damp: %2\n"
                   "# FEM_damp: %3\n"
                   "PIC_damp: %4\n"
                   "\n"
                   "point_per_cell: [%5,%6,%7]\n"
                   "rearrange: %8\n"
                   "max_point_per_cell: %9\n"
                   "min_point_per_cell: %10\n")
        .arg(interface_damp).arg(MPM_damp).arg(FEM_damp).arg(PIC_damp)
        .arg(ppc1).arg(ppc2).arg(ppc3)
        .arg(rearrange ? "true" : "false")
        .arg(maxPpc).arg(minPpc);
}

// 实现 getTimeYaml
QString SolutionDlg::getTimeYaml()
{
    QString dt      = ui->lineEdit_dt->text().trimmed();
    QString end     = ui->lineEdit_end->text().trimmed();
    QString interval = ui->lineEdit_write_inteval->text().trimmed();

    if(dt.isEmpty()) dt = "1e-7";
    if(end.isEmpty()) end = "1e-3";
    if(interval.isEmpty()) interval = "1e-5";

    return QString("time:\n"
                   "    dt: %1\n"
                   "    end: %2\n"
                   "    write_inteval: %3\n")
        .arg(dt).arg(end).arg(interval);
}

// 原有的按钮槽函数可以保留为空，或者调用上面的函数打印一下 Log，
// 但实际保存功能将移交给 MainWindow。
void SolutionDlg::saveSolver() { /* 可选：仅做界面校验 */ }
 void SolutionDlg::on_pushButton_time_clicked()
    {
        // 1. 数据校验 (防止用户填空值)
        if (ui->lineEdit_dt->text().isEmpty() ||
            ui->lineEdit_end->text().isEmpty() ||
            ui->lineEdit_write_inteval->text().isEmpty())
        {
            // 红色提示：有错误
            Toast::instance().show(Toast::TERROR, "Error: Time parameters cannot be empty!", this);
            return;
        }

        // 2. (可选) 逻辑检查，比如结束时间不能小于步长
        if (ui->lineEdit_dt->text().toDouble() > ui->lineEdit_end->text().toDouble()) {
            Toast::instance().show(Toast::TWARN, "Warning: dt is larger than end time!", this);
            return;
        }

        // 3. 正向反馈：告诉用户“没问题了”
        // 关键话术：表明这是“暂存”或“已确认”，并引导去主界面保存
        Toast::instance().show(Toast::TINFO,
                               "Time settings confirmed.\n(Please click 'Save' in Main Window to write file)",
                               this);
    }
    void SolutionDlg::on_pushButton_MPM_clicked()
    {
        // 1. 校验
        if (ui->lineEdit_point_per_cell_1->text().isEmpty()) {
            Toast::instance().show(Toast::TERROR, "Error: Point Per Cell required.", this);
            return;
        }

        // 2. 反馈
        Toast::instance().show(Toast::TINFO,
                               "MPM parameters ready.\nDon't forget to Save the project!",
                               this);
    }


    void SolutionDlg::on_pushButton_cancel1_clicked()
    {
        reject();
    }

    void SolutionDlg::on_pushButton_cancel2_clicked()
    {
        reject();
    }

void SolutionDlg::setTargetNodeName(const QString &name)
{
    m_currentEditingNode = name;
    // 设置标题...
}




