#include "boundarydlg.h"
#include "ui_boundarydlg.h"
#include"toastdlg.h"
#include <QDir>
#include <QTextStream>
#include <QFile>
#include<QDebug>
BoundaryDlg::BoundaryDlg(QWidget *parent)
    : FramelessBaseDialog(parent)
    , ui(new Ui::BoundaryDlg)
{
    ui->setupUi(this);
    if (ui->mainContainer) {
        // 这一步会将 mainContainer 从原来的位置"移动"到内容布局中
        this->contentLayout()->addWidget(ui->mainContainer);

        // 确保容器可见（有时候从 setupUi 出来默认可能是隐藏的，视 UI 设置而定）
        ui->mainContainer->setVisible(true);
    }
    // 3. 设置标题栏文本
    setWindowTitleText("Boundary Settings");
    this->contentLayout()->setContentsMargins(5, 5, 5, 5);

    // 在 BoundaryFieldDialog 的构造函数中（或其他初始化位置）
    connect(ui->comboBox_xmin, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &BoundaryDlg::onBoundaryTypeChanged);
    connect(ui->comboBox_xmax, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &BoundaryDlg::onBoundaryTypeChanged);
    connect(ui->comboBox_ymin, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &BoundaryDlg::onBoundaryTypeChanged);
    connect(ui->comboBox_ymax, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &BoundaryDlg::onBoundaryTypeChanged);
    connect(ui->comboBox_zmin, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &BoundaryDlg::onBoundaryTypeChanged);
    connect(ui->comboBox_zmax, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &BoundaryDlg::onBoundaryTypeChanged);

    // 初始化 UI 状态（在信号连接之后调用，只调用一次）
    onBoundaryTypeChanged();
}

BoundaryDlg::~BoundaryDlg()
{
    delete ui;
}

void BoundaryDlg::setTargetNodeName(const QString &name)
{
    m_currentEditingNode = name;
    if (name.isEmpty()) {
        setWindowTitleText("Boundary Settings");
    } else {
        setWindowTitleText("Edit Boundary: " + name);
    }
}

void BoundaryDlg::onBoundaryTypeChanged()
{
    // X min
    if (ui->comboBox_xmin->currentText() == "fixed_value") {
        ui->lineEdit_xmin_value->setEnabled(true);
    } else {
        ui->lineEdit_xmin_value->setEnabled(false);
        ui->lineEdit_xmin_value->clear(); // 可选：清空无效值
    }

    // X max
    if (ui->comboBox_xmax->currentText() == "fixed_value") {
        ui->lineEdit_xmax_value->setEnabled(true);
    } else {
        ui->lineEdit_xmax_value->setEnabled(false);
        ui->lineEdit_xmax_value->clear();
    }

    // Y min
    if (ui->comboBox_ymin->currentText() == "fixed_value") {
        ui->lineEdit_ymin_value->setEnabled(true);
    } else {
        ui->lineEdit_ymin_value->setEnabled(false);
        ui->lineEdit_ymin_value->clear();
    }

    // Y max
    if (ui->comboBox_ymax->currentText() == "fixed_value") {
        ui->lineEdit_ymax_value->setEnabled(true);
    } else {
        ui->lineEdit_ymax_value->setEnabled(false);
        ui->lineEdit_ymax_value->clear();
    }

    // Z min
    if (ui->comboBox_zmin->currentText() == "fixed_value") {
        ui->lineEdit_zmin_value->setEnabled(true);
    } else {
        ui->lineEdit_zmin_value->setEnabled(false);
        ui->lineEdit_zmin_value->clear();
    }

    // Z max
    if (ui->comboBox_zmax->currentText() == "fixed_value") {
        ui->lineEdit_zmax_value->setEnabled(true);
    } else {
        ui->lineEdit_zmax_value->setEnabled(false);
        ui->lineEdit_zmax_value->clear();
    }
}

void BoundaryDlg::on_pushButton_OK_mechanical_clicked()
{
    // 1. 获取用户输入
    QString direction_x = ui->lineEdit_x->text().trimmed();
    QString direction_y = ui->lineEdit_y->text().trimmed();
    QString direction_z = ui->lineEdit_z->text().trimmed();

    QString locationxmin = ui->lineEdit_xmin->text().trimmed();
    QString locationxmax = ui->lineEdit_xmax->text().trimmed();
    QString locationymin = ui->lineEdit_ymin->text().trimmed();
    QString locationymax = ui->lineEdit_ymax->text().trimmed();
    QString locationzmin = ui->lineEdit_zmin->text().trimmed();
    QString locationzmax = ui->lineEdit_zmax->text().trimmed();

    // 2. 校验 direction：至少有一个方向非空
    if (direction_x.isEmpty() && direction_y.isEmpty() && direction_z.isEmpty()) {
        Toast::instance().show(Toast::TINFO, QStringLiteral("At least one direction (x/y/z) must be specified."), this);
        return;
    }

    // 3. 校验 location
    if (!direction_x.isEmpty() && locationxmin.isEmpty() && locationxmax.isEmpty()) {
        Toast::instance().show(Toast::TINFO, QStringLiteral("Location for x-direction cannot be empty."), this);
        return;
    }
    if (!direction_y.isEmpty() && locationymin.isEmpty() && locationymax.isEmpty()) {
        Toast::instance().show(Toast::TINFO, QStringLiteral("Location for y-direction cannot be empty."), this);
        return;
    }
    if (!direction_z.isEmpty() && locationzmin.isEmpty() && locationzmax.isEmpty()) {
        Toast::instance().show(Toast::TINFO, QStringLiteral("Location for z-direction cannot be empty."), this);
        return;
    }

    QString bName = ui->lineEdit_name->text().trimmed();
    if (bName.isEmpty()) bName = "Boundary";
    emit sigName(bName);
    Toast::instance().show(Toast::TINFO, QStringLiteral("Boundary conditions saved successfully."), this);
    accept();
}


void BoundaryDlg::on_pushButton_cancle_clicked()
{
    close();
}


void BoundaryDlg::on_pushButton_OK_Temperature_clicked()
{
    // 1. 获取 ComboBox 的值
    QString xmin_type = ui->comboBox_xmin->currentText();
    QString xmax_type = ui->comboBox_xmax->currentText();
    QString ymin_type = ui->comboBox_ymin->currentText();
    QString ymax_type = ui->comboBox_ymax->currentText();
    QString zmin_type = ui->comboBox_zmin->currentText();
    QString zmax_type = ui->comboBox_zmax->currentText();

    // 2. 校验 fixed_value 对应的输入
    if (ymin_type == "fixed_value" && ui->lineEdit_ymin_value->text().trimmed().isEmpty()) {
        Toast::instance().show(Toast::TINFO, "Y min value is required for fixed_value.", this);
        return;
    }
    if (ymax_type == "fixed_value" && ui->lineEdit_ymax_value->text().trimmed().isEmpty()) {
        Toast::instance().show(Toast::TINFO, "Y max value is required for fixed_value.", this);
        return;
    }
    if (zmin_type == "fixed_value" && ui->lineEdit_zmin_value->text().trimmed().isEmpty()) {
        Toast::instance().show(Toast::TINFO, "Z min value is required for fixed_value.", this);
        return;
    }
    if (zmax_type == "fixed_value" && ui->lineEdit_zmax_value->text().trimmed().isEmpty()) {
        Toast::instance().show(Toast::TINFO, "Z max value is required for fixed_value.", this);
        return;
    }
    if (xmin_type == "fixed_value" && ui->lineEdit_xmin_value->text().trimmed().isEmpty()) {
        Toast::instance().show(Toast::TINFO, "X min value is required for fixed_value.", this);
        return;
    }
    if (xmax_type == "fixed_value" && ui->lineEdit_xmax_value->text().trimmed().isEmpty()) {
        Toast::instance().show(Toast::TINFO, "X max value is required for fixed_value.", this);
        return;
    }

    QString bName = ui->lineEdit_name_2->text().trimmed();
    if (bName.isEmpty()) bName = "Boundary";
    emit sigName(bName);
    accept();
}
void BoundaryDlg::on_pushButton_cancel_2_clicked()
{
    close();
}

QString BoundaryDlg::getFixedBoundaryYaml() const
{
    QString direction_x = ui->lineEdit_x->text().trimmed();
    QString direction_y = ui->lineEdit_y->text().trimmed();
    QString direction_z = ui->lineEdit_z->text().trimmed();

    if (direction_x.isEmpty() && direction_y.isEmpty() && direction_z.isEmpty())
        return QString();

    QStringList lines;
    lines << "    fixed_boundary:";

    auto addDirection = [&](const QString &dir, const QString &locMin, const QString &locMax) {
        if (dir.isEmpty()) return;
        QStringList locList;
        if (!locMin.isEmpty()) locList << locMin;
        if (!locMax.isEmpty()) locList << locMax;
        if (!locList.isEmpty()) {
            lines << QString("      - direction: [%1]").arg(dir);
            lines << QString("        location: [%1]").arg(locList.join(","));
        }
    };

    addDirection(direction_x, ui->lineEdit_xmin->text().trimmed(), ui->lineEdit_xmax->text().trimmed());
    addDirection(direction_y, ui->lineEdit_ymin->text().trimmed(), ui->lineEdit_ymax->text().trimmed());
    addDirection(direction_z, ui->lineEdit_zmin->text().trimmed(), ui->lineEdit_zmax->text().trimmed());

    if (lines.size() <= 1) return QString();
    return lines.join("\n") + "\n";
}

QString BoundaryDlg::getFieldYamlSection(const QString &initialTemp) const
{
    QString xmin_type = ui->comboBox_xmin->currentText();
    QString xmax_type = ui->comboBox_xmax->currentText();
    QString ymin_type = ui->comboBox_ymin->currentText();
    QString ymax_type = ui->comboBox_ymax->currentText();
    QString zmin_type = ui->comboBox_zmin->currentText();
    QString zmax_type = ui->comboBox_zmax->currentText();

    // 如果所有都是默认值（未设置），返回空
    if (xmin_type.isEmpty() && xmax_type.isEmpty() &&
        ymin_type.isEmpty() && ymax_type.isEmpty() &&
        zmin_type.isEmpty() && zmax_type.isEmpty())
        return QString();

    QString yaml;
    yaml += "field:\n";
    yaml += "    temperature:\n";
    yaml += "        initial_value: " + initialTemp + "\n";
    yaml += "        boundary_type:\n";
    yaml += "            xmin: " + xmin_type + "\n";
    yaml += "            xmax: " + xmax_type + "\n";
    yaml += "            ymin: " + ymin_type + "\n";
    yaml += "            ymax: " + ymax_type + "\n";
    yaml += "            zmin: " + zmin_type + "\n";
    yaml += "            zmax: " + zmax_type + "\n";

    // boundary_value 部分（仅 fixed_value 类型需要）
    QStringList valLines;
    if (xmin_type == "fixed_value") valLines << "            xmin: " + ui->lineEdit_xmin_value->text().trimmed();
    if (xmax_type == "fixed_value") valLines << "            xmax: " + ui->lineEdit_xmax_value->text().trimmed();
    if (ymin_type == "fixed_value") valLines << "            ymin: " + ui->lineEdit_ymin_value->text().trimmed();
    if (ymax_type == "fixed_value") valLines << "            ymax: " + ui->lineEdit_ymax_value->text().trimmed();
    if (zmin_type == "fixed_value") valLines << "            zmin: " + ui->lineEdit_zmin_value->text().trimmed();
    if (zmax_type == "fixed_value") valLines << "            zmax: " + ui->lineEdit_zmax_value->text().trimmed();

    if (!valLines.isEmpty()) {
        yaml += "        boundary_value:\n";
        yaml += valLines.join("\n") + "\n";
    }

    return yaml;
}

