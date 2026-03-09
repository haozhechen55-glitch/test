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
        // 这一步会将 mainContainer 从原来的位置"移动"到内容布局中
        this->contentLayout()->addWidget(ui->maincontainer_sol);

        // 确保容器可见（有时候从 setupUi 出来默认可能是隐藏的，视 UI 设置而定）
        ui->maincontainer_sol->setVisible(true);
    }
    // 3. 设置标题栏文本
    setWindowTitleText("Solution");

    // 创建 interface_damp 和 FEM_damp 控件（UI 文件中未定义，在此动态添加）
    lineEdit_interface_damp = new QLineEdit("0", this);
    lineEdit_interface_damp->setObjectName("lineEdit_interface_damp");
    lineEdit_interface_damp->setVisible(false);

    lineEdit_FEM_damp = new QLineEdit("0", this);
    lineEdit_FEM_damp->setObjectName("lineEdit_FEM_damp");
    lineEdit_FEM_damp->setVisible(false);
}

SolutionDlg::~SolutionDlg()
{
    delete ui;
}

// 实现 getSolverYaml
QString SolutionDlg::getSolverYaml()
{
    QString solver   = ui->comboBox_solver->currentText();
    QString model = ui->comboBox_model->currentText();
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
    QString interface_damp = lineEdit_interface_damp->text().trimmed();
    QString FEM_damp = lineEdit_FEM_damp->text().trimmed();

    if (ppc1.isEmpty()) ppc1 = "3"; // 防止为空的保护机制
    if (ppc2.isEmpty()) ppc2 = "3";
    if (ppc3.isEmpty()) ppc3 = "3";

    QString result = QString("interface_damp: %1\n"
                   "MPM_damp: %2\n"
                   "# FEM_damp: %3\n"
                   "PIC_damp: %4\n"
                   "\n"
                   "point_per_cell: [%5,%6,%7]\n"
                   "rearrange: %8\n"
                   "max_point_per_cell: %9\n")
        .arg(interface_damp, MPM_damp, FEM_damp, PIC_damp)
        .arg(ppc1, ppc2, ppc3)
        .arg(rearrange ? "true" : "false")
        .arg(maxPpc);
    result += QString("min_point_per_cell: %1\n").arg(minPpc);
    return result;
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

        // 3. 正向反馈：告诉用户"没问题了"
        // 关键话术：表明这是"暂存"或"已确认"，并引导去主界面保存
        Toast::instance().show(Toast::TINFO,
                               "Time settings confirmed.\n(Please click 'Save' in Main Window to write file)",
                               this);
        emit sigAddSolution("Solution");
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
        emit sigAddSolution("Solution");
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




