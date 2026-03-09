#include "outputsettingdialog.h"
#include "ui_outputsettingdialog.h"

#include "toastdlg.h"
#include <QDir>
#include <QTextStream>
#include <QFile>
#include <QDebug>

OutputSettingDialog::OutputSettingDialog(QWidget *parent)
    : FramelessBaseDialog(parent) // 1. 调用基类构造函数
    , ui(new Ui::OutputSettingDialog)
    , writeAllFields(true), detailedLog(false)
{
    ui->setupUi(this);

    // 2. 关键步骤：把 mainContainer 塞进无边框窗口的 contentLayout
    if (ui->mainContainer) {
        this->contentLayout()->addWidget(ui->mainContainer);
        ui->mainContainer->setVisible(true);
    }

    // 3. 使用自定义的标题栏设置函数
    setWindowTitleText("Output Settings");

    // 4. 设置窗口大小（根据你的基类实现，可能需要在这里手动 resize）
    this->resize(320, 200);

    // 设置默认 UI 状态
    ui->checkBox_write_all_fields->setChecked(true);
    ui->checkBox_detailed_log->setChecked(false);
}

OutputSettingDialog::~OutputSettingDialog()
{
    delete ui;
}

bool OutputSettingDialog::getWriteAllFields() const { return writeAllFields; }
bool OutputSettingDialog::getDetailedLog() const { return detailedLog; }

void OutputSettingDialog::on_pushButton_ok_clicked() {
    writeAllFields = ui->checkBox_write_all_fields->isChecked();
    detailedLog = ui->checkBox_detailed_log->isChecked();

    // writeOutputToYml(writeAllFields, detailedLog); // <--- 【删除或注释掉这一行】

    QString finalName = m_currentEditingNode.isEmpty() ? "Output" : m_currentEditingNode;
    emit sigAddOutput(finalName);
    accept();
}

void OutputSettingDialog::on_pushButton_cancel_clicked() {
    reject();
}

QString OutputSettingDialog::getOutputYaml()
{
    // 获取当前界面控件的状态
    bool wAll = ui->checkBox_write_all_fields->isChecked();
    bool detail = ui->checkBox_detailed_log->isChecked();

    // 按照标准格式拼接字符串
    // 注意：这里使用了 Qt 的字符串格式化 .arg()
    return QString("output:\n"
                   "    vtk: \n"
                   "        write_all_fields: %1\n"
                   "        background_mesh:\n"
                   "            node:\n"
                   "                variable: [Temperature, VolumeFraction]\n"
                   "            cell:\n"
                   "                variable: [Tag, VolumeFraction]\n"
                   "        # deform_mesh:\n"
                   "        #     node:\n"
                   "        #         variable: [Stress, Temperature, MisesStress, Velocity]\n"
                   "        #     cell:\n"
                   "        #         variable: [FEM, Temperature]\n"
                   "        point:\n"
                   "            variable: [Tag, Temperature, Stress, MisesStress]\n"
                   "    log:\n"
                   "        detailed: %2\n")
        .arg(wAll ? "true" : "false")
        .arg(detail ? "true" : "false");
}

void OutputSettingDialog::writeOutputToYml(bool writeAllFields, bool detailedLog)
{

}

void OutputSettingDialog::setTargetNodeName(const QString &name) {
    m_currentEditingNode = name;
    }
