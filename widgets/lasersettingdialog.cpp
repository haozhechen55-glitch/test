#include "LaserSettingDialog.h"
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QLabel>
#include "toastdlg.h"
#include <QDir>
#include <QTextStream>
#include <QFile>
#include <QDebug>
#include <QRegExp> // 用于分割字符串

LaserSettingDialog::LaserSettingDialog(QWidget *parent)
    : FramelessBaseDialog(parent), yamlSection("")
{
    setFixedSize(800, 450);
    setWindowTitleText("Laser Settings");
    mainContainer = new QWidget(this);
    containerLayout = new QVBoxLayout(mainContainer);

    // 给容器设置一个名字，方便 QSS 精确锁定
    mainContainer->setObjectName("mainContainer");

    // 应用全局样式
    mainContainer->setStyleSheet(
        "#mainContainer, #mainContainer * {"
        "   color: #333333;"
        "   font: 700 9pt 'Yu Gothic';"
        "}"
        );


    tableWidget = new QTableWidget(ROW_COUNT, COL_COUNT, this);
    tableWidget->setObjectName("laserTableWidget");
    tableWidget->setHorizontalHeaderLabels(
        QStringList() << "Time" << "x" << "y" << "z" << "Radius" << "Power"
        );
    tableWidget->horizontalHeader()->setStretchLastSection(true);
    tableWidget->verticalHeader()->setVisible(false);
    tableWidget->setAlternatingRowColors(true);

    for (int row = 0; row < ROW_COUNT; ++row) {
        for (int col = 0; col < COL_COUNT; ++col) {
            tableWidget->setItem(row, col, new QTableWidgetItem(""));
        }
    }

    // --- 修改开始：Import 区域 ---
    pathEdit = new QLineEdit(this);
    pathEdit->setObjectName("filePathEdit");
    pathEdit->setReadOnly(true); // 设置为只读，只能通过按钮选择
    pathEdit->setPlaceholderText("Select a data file (.txt, .csv)...");

    browseButton = new QPushButton("...", this);
    browseButton->setFixedWidth(40); // 浏览按钮宽度
    browseButton->setCursor(Qt::PointingHandCursor);

    pathLayout = new QHBoxLayout;
    pathLayout->addWidget(new QLabel("import:", this));
    pathLayout->addWidget(pathEdit);
    pathLayout->addWidget(browseButton); // 添加浏览按钮
    // --- 修改结束 ---

    okButton = new QPushButton("OK", this);
    cancelButton = new QPushButton("Cancel", this);
    buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);

    containerLayout->addWidget(tableWidget);
    containerLayout->addLayout(pathLayout);
    containerLayout->addLayout(buttonLayout);
    containerLayout->setContentsMargins(15, 10, 15, 15);
    this->contentLayout()->addWidget(mainContainer);

    connect(okButton, &QPushButton::clicked, this, &LaserSettingDialog::onOkClicked);
    connect(cancelButton, &QPushButton::clicked, this, &LaserSettingDialog::onCancelClicked);
    // 连接浏览按钮信号
    connect(browseButton, &QPushButton::clicked, this, &LaserSettingDialog::onBrowseClicked);
}

LaserSettingDialog::~LaserSettingDialog() {}

// 辅助函数：验证字符串是否为有效浮点数
bool isValidNumber(const QString& str) {
    if (str.isEmpty()) return false;
    bool ok;
    str.toDouble(&ok);
    return ok;
}

// 新增：浏览文件槽函数
void LaserSettingDialog::onBrowseClicked() {
    QString fileName = QFileDialog::getOpenFileName(
        this,
        tr("Import Data File"),
        QDir::currentPath(),
        tr("Data Files (*.txt *.csv *.dat);;All Files (*)")
        );

    if (fileName.isEmpty()) {
        return;
    }

    pathEdit->setText(fileName);
    loadDataFromFile(fileName);
}

// 新增：读取文件并填充表格
void LaserSettingDialog::loadDataFromFile(const QString &fileName) {
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        Toast::instance().show(Toast::TINFO, "Failed to open file.", this);
        return;
    }

    QTextStream in(&file);

    // 清空现有表格内容（不删行，只清空文本）
    for (int r = 0; r < ROW_COUNT; ++r) {
        for (int c = 0; c < COL_COUNT; ++c) {
            if (tableWidget->item(r, c)) tableWidget->item(r, c)->setText("");
        }
    }

    int row = 0;
    while (!in.atEnd() && row < ROW_COUNT) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;

        // 使用正则表达式分割：逗号(,) 或 空白字符(\s)
        QStringList parts = line.split(QRegExp("[,\\s]+"), QString::SkipEmptyParts);

        // 填充列
        int colsToFill = qMin(parts.size(), COL_COUNT);
        for (int col = 0; col < colsToFill; ++col) {
            if (tableWidget->item(row, col)) {
                tableWidget->item(row, col)->setText(parts[col]);
            }
        }
        row++;
    }

    file.close();
    Toast::instance().show(Toast::TINFO, QString("Imported %1 rows.").arg(row), this);
}

void LaserSettingDialog::onOkClicked() {
    // 1. 收集所有非空行，并验证完整性与数值有效性
    QVector<QVector<QString>> validRows; // 存储有效的完整行

    for (int row = 0; row < ROW_COUNT; ++row) {
        QVector<QString> rowData;
        bool hasAnyData = false;

        for (int col = 0; col < COL_COUNT; ++col) {
            QString text = tableWidget->item(row, col) ? tableWidget->item(row, col)->text().trimmed() : "";
            if (!text.isEmpty()) {
                hasAnyData = true;
                if (!isValidNumber(text)) {
                    Toast::instance().show(Toast::TINFO,
                                           QString("Invalid number in Row %1, Col %2: '%3'").arg(row + 1).arg(col + 1).arg(text),
                                           this);
                    return;
                }
            }
            rowData.append(text);
        }

        if (hasAnyData) {
            // 检查是否 6 项全填
            for (int col = 0; col < COL_COUNT; ++col) {
                if (rowData[col].isEmpty()) {
                    Toast::instance().show(Toast::TINFO,
                                           QString("Incomplete row %1: All 6 fields must be filled if any is non-empty.").arg(row + 1),
                                           this);
                    return;
                }
            }
            validRows.append(rowData);
        }
    }

    // 2. 如果没有有效行，使用默认值
    if (validRows.isEmpty()) {
        Toast::instance().show(Toast::TINFO,QString("Data is empty!"),this);
        return;
    }

    // 3. 按列提取数据
    QStringList timeList, xList, yList, zList, radiusList, powerList;
    for (const auto& row : validRows) {
        timeList.append(row[0]);
        xList.append(row[1]);
        yList.append(row[2]);
        zList.append(row[3]);
        radiusList.append(row[4]);
        powerList.append(row[5]);
    }

    // 4. 检查所有列长度是否一致
    int count = timeList.size();
    if (xList.size() != count || yList.size() != count ||
        zList.size() != count || radiusList.size() != count || powerList.size() != count) {
        Toast::instance().show(Toast::TINFO, "Data count mismatch between columns.", this);
        return;
    }

    //组装
    QString filePath = QDir::currentPath() + "/out.yml";
    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        Toast::instance().show(Toast::TINFO, QStringLiteral("Yml file open failed"), this);
        return;
    }

    QTextStream in(&file);
    QString fileContent = in.readAll();
    file.close();

    // 5. 生成 YAML
    yamlSection = QString(
                      "laser:\n"
                      "    time: [%1]\n"
                      "    power: [%2]\n"
                      "    x: [%3]\n"
                      "    y: [%4]\n"
                      "    z: [%5]\n"
                      "    radius: [%6]\n"
                      ).arg(timeList.join(","))
                      .arg(powerList.join(","))
                      .arg(xList.join(","))
                      .arg(yList.join(","))
                      .arg(zList.join(","))
                      .arg(radiusList.join(","));

    QStringList lines = fileContent.split('\n');

    int bgMeshStartIndex = -1;
    int bgMeshEndIndex = -1;

    for (int i = 0; i < lines.size(); ++i) {
        if (lines[i].trimmed() == "laser:") {
            bgMeshStartIndex = i;
            break;
        }
    }

    if (bgMeshStartIndex != -1) {
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

        for (int i = bgMeshEndIndex; i >= bgMeshStartIndex; --i) {
            lines.removeAt(i);
        }

        QStringList newLines = yamlSection.split('\n');
        int insertIndex = bgMeshStartIndex;
        for (const QString& newLine : newLines) {
            lines.insert(insertIndex++, newLine);
        }

    } else {
        lines.append("");
        QStringList newLines = yamlSection.split('\n');
        lines.append(newLines);
    }

    QString newFileContent = lines.join('\n');

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        Toast::instance().show(Toast::TINFO, QStringLiteral("Write to yml file failed"), this);
        return;
    }

    QTextStream out(&file);
    out << newFileContent;
    file.close();

    emit sigLasterName("laser1");
    accept();
}

void LaserSettingDialog::onCancelClicked() {
    reject();
}

QString LaserSettingDialog::getYamlSection() const {
    return yamlSection;
}
