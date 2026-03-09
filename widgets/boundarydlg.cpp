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
        // 这一步会将 mainContainer 从原来的位置“移动”到内容布局中
        this->contentLayout()->addWidget(ui->mainContainer);

        // 确保容器可见（有时候从 setupUi 出来默认可能是隐藏的，视 UI 设置而定）
        ui->mainContainer->setVisible(true);
    }
    // 3. 设置标题栏文本
    setWindowTitleText("Material Editor");
    this->contentLayout()->setContentsMargins(5, 5, 5, 5);

    onBoundaryTypeChanged();
    onBoundaryTypeChanged();
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
}

BoundaryDlg::~BoundaryDlg()
{
    delete ui;
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

    // 3. 校验 location：每个非空 direction 对应的 location 不能为空
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

    // 4. 读取 out.yml
    QString filePath = QDir::currentPath() + "/out.yml";
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        Toast::instance().show(Toast::TINFO, QStringLiteral("Yml file open failed"), this);
        return;
    }

    QTextStream in(&file);
    QString fileContent = in.readAll();
    file.close();

    // 5. 检查是否存在 background_mesh 段（必须包含 min/max/divide）
    if (!fileContent.contains("background_mesh:") ||
        !fileContent.contains("min:") ||
        !fileContent.contains("max:") ||
        !fileContent.contains("divide:")) {
        Toast::instance().show(Toast::TINFO, QStringLiteral("Please set grid data (background_mesh) first."), this);
        return;
    }

    // 6. 动态构建 fixed_boundary 内容
    QStringList fixedBoundaryLines;
    fixedBoundaryLines << "    fixed_boundary:";

    // 处理 x 方向
    // 处理 X 方向
    if (!direction_x.trimmed().isEmpty()) {
        QStringList locList;

        // 只有非空（trimmed 后）才加入
        QString xmin = locationxmin.trimmed();
        QString xmax = locationxmax.trimmed();

        if (!xmin.isEmpty()) {
            locList << xmin;
        }
        if (!xmax.isEmpty()) {
            locList << xmax;
        }

        // 关键：只有 locList 不为空才生成
        if (!locList.isEmpty()) {
            fixedBoundaryLines << QString("      - direction: [%1]").arg(direction_x.trimmed());
            // join 会自动处理：1个元素 → "a"，2个 → "a,b"，不会产生 "a," 或 ",b"
            fixedBoundaryLines << QString("        location: [%1]").arg(locList.join(","));
        } else {
            // 不应该发生，但保险起见
            Toast::instance().show(Toast::TINFO, QStringLiteral("X direction location is empty."), this);
            return;
        }
    }

    // 处理 y 方向
    if (!direction_y.trimmed().isEmpty()) {
        QStringList locList;

        // 只有非空（trimmed 后）才加入
        QString ymin = locationymin.trimmed();
        QString ymax = locationymax.trimmed();

        if (!ymin.isEmpty()) {
            locList << ymin;
        }
        if (!ymax.isEmpty()) {
            locList << ymax;
        }

        // 关键：只有 locList 不为空才生成
        if (!locList.isEmpty()) {
            fixedBoundaryLines << QString("      - direction: [%1]").arg(direction_y.trimmed());
            // join 会自动处理：1个元素 → "a"，2个 → "a,b"，不会产生 "a," 或 ",b"
            fixedBoundaryLines << QString("        location: [%1]").arg(locList.join(","));
        } else {
            // 不应该发生，但保险起见
            Toast::instance().show(Toast::TINFO, QStringLiteral("Y direction location is empty."), this);
            return;
        }
    }

    // 处理 z 方向
    if (!direction_z.trimmed().isEmpty()) {
        QStringList locList;

        // 只有非空（trimmed 后）才加入
        QString zmin = locationzmin.trimmed();
        QString zmax = locationzmax.trimmed();

        if (!zmin.isEmpty()) {
            locList << zmin;
        }
        if (!zmax.isEmpty()) {
            locList << zmax;
        }

        // 关键：只有 locList 不为空才生成
        if (!locList.isEmpty()) {
            fixedBoundaryLines << QString("      - direction: [%1]").arg(direction_z.trimmed());
            // join 会自动处理：1个元素 → "a"，2个 → "a,b"，不会产生 "a," 或 ",b"
            fixedBoundaryLines << QString("        location: [%1]").arg(locList.join(","));
        } else {
            // 不应该发生，但保险起见
            Toast::instance().show(Toast::TINFO, QStringLiteral("Z direction location is empty."), this);
            return;
        }
    }

    // 7. 按行处理文件内容
    QStringList lines = fileContent.split('\n');
    int bgMeshStartIndex = -1;
    int bgMeshEndIndex = -1;

    // 找到 background_mesh: 起始行
    for (int i = 0; i < lines.size(); ++i) {
        if (lines[i].trimmed() == "background_mesh:") {
            bgMeshStartIndex = i;
            break;
        }
    }

    if (bgMeshStartIndex == -1) {
        Toast::instance().show(Toast::TINFO, QStringLiteral("background_mesh section not found."), this);
        return;
    }

    // 找到 background_mesh 段结束行（下一个顶级字段）
    bgMeshEndIndex = bgMeshStartIndex;
    for (int i = bgMeshStartIndex + 1; i < lines.size(); ++i) {
        QString line = lines[i];
        if (line.trimmed().isEmpty() || line.startsWith('#')) {
            bgMeshEndIndex = i;
            continue;
        }
        if (line.startsWith(' ') || line.startsWith('\t')) {
            bgMeshEndIndex = i;
            continue;
        } else {
            bgMeshEndIndex = i - 1;
            break;
        }
    }

    // 8. 在 background_mesh 段末尾插入 fixed_boundary
    // 先删除已有的 fixed_boundary（如果存在）
    int fixedBoundaryStart = -1;
    for (int i = bgMeshStartIndex + 1; i <= bgMeshEndIndex; ++i) {
        if (lines[i].trimmed() == "fixed_boundary:") {
            fixedBoundaryStart = i;
            break;
        }
    }

    if (fixedBoundaryStart != -1) {
        // 找到 fixed_boundary 结束位置
        int fbEnd = fixedBoundaryStart;
        for (int i = fixedBoundaryStart + 1; i <= bgMeshEndIndex; ++i) {
            QString line = lines[i];
            if (line.startsWith("      - direction:") || line.startsWith("        location:")) {
                fbEnd = i;
                continue;
            } else if (line.trimmed().isEmpty() || line.startsWith(' ') == false) {
                break;
            } else {
                fbEnd = i;
            }
        }
        // 删除旧的 fixed_boundary
        for (int i = fbEnd; i >= fixedBoundaryStart; --i) {
            lines.removeAt(i);
            bgMeshEndIndex--; // 更新段结束位置
        }
    }

    // 插入新的 fixed_boundary
    int insertPos = bgMeshEndIndex;
    for (const QString& line : fixedBoundaryLines) {
        lines.insert(insertPos++, line);
    }

    // 9. 写回文件
    QString newFileContent = lines.join('\n');
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        Toast::instance().show(Toast::TINFO, QStringLiteral("Write to yml file failed"), this);
        return;
    }

    QTextStream out(&file);
    out << newFileContent;
    file.close();

    emit sigName("boundary1");
    Toast::instance().show(Toast::TINFO, QStringLiteral("Boundary conditions saved successfully."), this);
    accept(); // 关闭对话框
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

    // 2. 获取 LineEdit 的值（仅当类型为 fixed_value 时才需要校验）
    QString ymin_val, ymax_val, zmin_val, zmax_val, xmin_val, xmax_val;

    // 校验 fixed_value 对应的输入
    if (ymin_type == "fixed_value") {
        ymin_val = ui->lineEdit_ymin_value->text().trimmed();
        if (ymin_val.isEmpty()) {
            Toast::instance().show(Toast::TINFO, "Y min value is required for fixed_value.", this);
            return;
        }
    }
    if (ymax_type == "fixed_value") {
        ymax_val = ui->lineEdit_ymax_value->text().trimmed();
        if (ymax_val.isEmpty()) {
            Toast::instance().show(Toast::TINFO, "Y max value is required for fixed_value.", this);
            return;
        }
    }
    if (zmin_type == "fixed_value") {
        zmin_val = ui->lineEdit_zmin_value->text().trimmed();
        if (zmin_val.isEmpty()) {
            Toast::instance().show(Toast::TINFO, "Z min value is required for fixed_value.", this);
            return;
        }
    }
    if (zmax_type == "fixed_value") {
        zmax_val = ui->lineEdit_zmax_value->text().trimmed();
        if (zmax_val.isEmpty()) {
            Toast::instance().show(Toast::TINFO, "Z max value is required for fixed_value.", this);
            return;
        }
    }
    if (xmin_type == "fixed_value") {
        xmin_val = ui->lineEdit_xmin_value->text().trimmed();
        if (xmin_val.isEmpty()) {
            Toast::instance().show(Toast::TINFO, "X min value is required for fixed_value.", this);
            return;
        }
    }
    if (xmax_type == "fixed_value") {
        xmax_val = ui->lineEdit_xmax_value->text().trimmed();
        if (xmax_val.isEmpty()) {
            Toast::instance().show(Toast::TINFO, "X max value is required for fixed_value.", this);
            return;
        }
    }

    // 3. 构建 boundary_type 部分
    QString boundaryTypeSection =
        QString("        boundary_type: \n"
                "            xmin: %1\n"
                "            xmax: %2\n"
                "            ymin: %3\n"
                "            ymax: %4\n"
                "            zmin: %5\n"
                "            zmax: %6\n")
            .arg(xmin_type).arg(xmax_type)
            .arg(ymin_type).arg(ymax_type)
            .arg(zmin_type).arg(zmax_type);

    // 4. 构建 boundary_value 部分（动态）
    QStringList boundaryValueLines;
    boundaryValueLines << "        boundary_value:";

    if (xmin_type == "fixed_value") boundaryValueLines << QString("            xmin: %1").arg(xmin_val);
    if (xmax_type == "fixed_value") boundaryValueLines << QString("            xmax: %1").arg(xmax_val);
    if (ymin_type == "fixed_value") boundaryValueLines << QString("            ymin: %1").arg(ymin_val);
    if (ymax_type == "fixed_value") boundaryValueLines << QString("            ymax: %1").arg(ymax_val);
    if (zmin_type == "fixed_value") boundaryValueLines << QString("            zmin: %1").arg(zmin_val);
    if (zmax_type == "fixed_value") boundaryValueLines << QString("            zmax: %1").arg(zmax_val);

    // 如果没有 fixed_value，boundary_value 为空对象（YAML 允许）
    // if (boundaryValueLines.size() == 1) {
    //     boundaryValueLines << "            {}";
    // }

    QString boundaryValueSection = boundaryValueLines.join('\n');

    // 5. 组合完整的 field 段（注意 initial_value 需要从其他地方获取）
    // [新增逻辑]：尝试从现有的 out.yml 中读取 reference_temperature 作为初始场温度
    QString initial_temp = "300"; // 默认值，以防读取失败

    // 读取文件查找 reference_temperature
    QFile fileRead(QDir::currentPath() + "/out.yml");
    if (fileRead.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream stream(&fileRead);
        while (!stream.atEnd()) {
            QString line = stream.readLine();
            // 查找 material 段落下的 reference_temperature
            if (line.trimmed().startsWith("reference_temperature:")) {
                QStringList parts = line.split(":");
                if (parts.size() >= 2) {
                    initial_temp = parts[1].trimmed();
                }
                break; // 找到后就停止
            }
        }
        fileRead.close();
    }
    QString fieldSection = QString(
                               "field: \n"
                               "    temperature:\n"
                               "        initial_value: %1\n"
                               "%2"
                               "%3\n"
                               ).arg(initial_temp)
                               .arg(boundaryTypeSection)
                               .arg(boundaryValueSection);

    // 6. 读取 out.yml 并替换/追加 field 段（逻辑与 material 类似）
    QString filePath = QDir::currentPath() + "/out.yml";
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        Toast::instance().show(Toast::TINFO, "Failed to open out.yml", this);
        return;
    }
    QString fileContent = file.readAll();
    file.close();

    QStringList lines = fileContent.split('\n');
    int fieldStartIndex = -1;
    for (int i = 0; i < lines.size(); ++i) {
        if (lines[i].trimmed() == "field:") {
            fieldStartIndex = i;
            break;
        }
    }

    // 查找 field 段结束位置
    int fieldEndIndex = fieldStartIndex;
    if (fieldStartIndex != -1) {
        for (int i = fieldStartIndex + 1; i < lines.size(); ++i) {
            QString line = lines[i];
            if (line.trimmed().isEmpty() || line.startsWith('#')) {
                fieldEndIndex = i;
                continue;
            }
            if (line.startsWith(' ') || line.startsWith('\t')) {
                fieldEndIndex = i;
                continue;
            } else {
                fieldEndIndex = i - 1;
                break;
            }
        }

        // 删除旧 field 段
        for (int i = fieldEndIndex; i >= fieldStartIndex; --i) {
            lines.removeAt(i);
        }
    }

    // 插入新 field 段
    QStringList newLines = fieldSection.split('\n');
    int insertPos = (fieldStartIndex != -1) ? fieldStartIndex : lines.size();
    if (fieldStartIndex == -1) lines.append(""); // 追加时加空行

    for (const QString& line : newLines) {
        if (!line.trimmed().isEmpty() || fieldStartIndex != -1) {
            lines.insert(insertPos++, line);
        }
    }

    // 7. 写回文件
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        Toast::instance().show(Toast::TINFO, "Failed to write out.yml", this);
        return;
    }
    file.write(lines.join('\n').toUtf8());
    file.close();

    accept();
}
void BoundaryDlg::on_pushButton_cancel_2_clicked()
{
    close();
}

