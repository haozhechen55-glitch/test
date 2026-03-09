#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QCloseEvent>
#include <QIcon>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QCheckBox>
#include <QCoreApplication>
#include <qscrollbar.h>
#include <QMessageBox>

#include <vtkXMLUnstructuredGridReader.h>
#include <vtkDataSet.h>
#include <vtkDataArray.h>
#include <vtkProperty.h>
#include <vtkPointData.h>
#include <vtkLookupTable.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkDataSetMapper.h>
#include <vtkActor.h>
#include <vtkOutputWindow.h>
#include <vtkAxesActor.h>
#include <vtkTextProperty.h>
#include <vtkDataSetSurfaceFilter.h>
#include <vtkStaticCleanPolyData.h>

#include <vtkUnstructuredGrid.h>
#include <vtkPolyData.h>
#include <vtkGenericDataObjectReader.h>
#include <vtkCamera.h>
#include <vtkTextActor.h>
#include <vtkScalarBarRepresentation.h>

#include <QFileDialog>
#include <QFileInfo>
#include <QRegExp>
#include <algorithm>
#include <QSettings>
#include <QMessageBox> // 用于显示错误信息
#include <QJsonDocument>

MainWindow::MainWindow(DraggableWidget *parent)
    : DraggableWidget(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    initTree();
    writeDefaultYml();

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &MainWindow::playNextFrame);
    vtkOutputWindow::GetInstance()->GlobalWarningDisplayOff();
    ui->progressBar->setTextVisible(false);
    this->setupVtkPipeline();

    //初始化 QFutureWatcher 并连接信号
    vtuWatcher = new QFutureWatcher<vtkSmartPointer<vtkDataSet>>(this);
    connect(vtuWatcher, &QFutureWatcher<vtkSmartPointer<vtkDataSet>>::finished,
            this, &MainWindow::onVtuLoadFinished);

    m_bggridDlg = new BGgridSetting(this);
    m_bggridDlg->setWindowModality(Qt::ApplicationModal); // 设置为模态

    m_matPointdlg = new MatPointSetting(this);
    m_matPointdlg->setWindowModality(Qt::ApplicationModal); // 设置为模态

    m_material = new MaterialEditor(this);
    m_material->setWindowModality(Qt::ApplicationModal); // 设置为模态

    m_LaserDlg = new LaserSettingDialog(this);
    m_LaserDlg->setWindowModality(Qt::ApplicationModal); // 设置为模态

    m_initialDlg = new InitialTemperatureDialog(this);
    m_initialDlg->setWindowModality(Qt::ApplicationModal); // 设置为模态

    m_load = new GravityDialog(this);
    m_load->setWindowModality(Qt::ApplicationModal); // 设置为模态

    m_boundaryDlg = new BoundaryDlg(this);
    m_boundaryDlg->setWindowModality(Qt::ApplicationModal); // 设置为模态

    m_output = new OutputSettingDialog(this);
    m_output->setWindowModality(Qt::ApplicationModal); // 设置为模态

    m_solution = new SolutionDlg(this);
    m_solution->setWindowModality(Qt::ApplicationModal); // 设置为模态

    m_powderDlg = new PowderDlg(this);
    m_powderDlg->setWindowModality(Qt::ApplicationModal);

    connect(m_bggridDlg,&BGgridSetting::sigName,this,&MainWindow::onAddBGgrid);
    connect(m_bggridDlg,&BGgridSetting::sigJsonWriteFinish,this,&MainWindow::onJsonDataUpdate);
    connect(m_matPointdlg,&MatPointSetting::sigAddMatPoint,this,&MainWindow::onAddMatPoint);
    connect(m_matPointdlg,&MatPointSetting::sigJsonWriteFinish,this,&MainWindow::onJsonDataUpdate);
    // 连接信号：当对话框点击保存时，通知主界面
    connect(m_powderDlg, &PowderDlg::sigAddPowder, this, &MainWindow::onAddPowder);
    // 连接信号：当 JSON 更新后，刷新预览（PreVisGenerator）
    connect(m_powderDlg, &PowderDlg::sigJsonWriteFinish, this, &MainWindow::onJsonDataUpdate);

    connect(m_material,&MaterialEditor::sigAddMaterial,this,&MainWindow::onAddMaterial);
    connect(m_boundaryDlg,&BoundaryDlg::sigName,this,&MainWindow::onAddBoundary);
    connect(m_LaserDlg, &LaserSettingDialog::sigLasterName, this, &MainWindow::onAddLaserItem);
    connect(m_load, &GravityDialog::sigAddGravity, this, &MainWindow::onAddGravity);
    connect(m_initialDlg, &InitialTemperatureDialog::sigAddInitTemp, this, &MainWindow::onAddInitTemp);
    connect(m_output, &OutputSettingDialog::sigAddOutput, this, &MainWindow::onAddOutput);
    connect(m_solution, &SolutionDlg::sigAddSolution, this, &MainWindow::onAddSolution);

    QList<int> sizes;
    sizes << 800 << 200;
    ui->splitter->setSizes(sizes);
    // 禁止第二个控件（索引为1，即日志区）被完全折叠隐藏
    ui->splitter->setCollapsible(1, false);

    showMaximized();


}

MainWindow::~MainWindow()
{
    // 在析构函数中检查并终止正在运行的异步任务
    if (vtuWatcher->isRunning()) {
        vtuWatcher->cancel();
        vtuWatcher->waitForFinished();
    }
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    // 弹出询问对话框
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "退出确认",
                                  "您想要在退出前保存工程吗？\n(Do you want to save before exit?)",
                                  QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

    if (reply == QMessageBox::Yes) {
        // --- 用户选择“是” ---

        // 1. 调用保存功能
        on_toolButton_save_clicked();

        // 2. 检查保存结果（防止用户点击保存后，在新建工程弹窗里点了取消，导致实际上没存）
        if (m_currentProjectDir.isEmpty()) {
            // 如果工程目录仍然为空，说明保存被取消了，那么也不应该关闭窗口
            event->ignore();
        } else {
            // 保存成功，允许关闭
            event->accept();
        }

    } else if (reply == QMessageBox::No) {
        // --- 用户选择“否” ---
        // 直接关闭，不保存
        event->accept();

    } else {
        // --- 用户选择“取消” ---
        // 撤销关闭操作，停留在当前界面
        event->ignore();
    }
}
//实现 changeEvent
void MainWindow::changeEvent(QEvent *event)
{
    // 如果是窗口状态改变事件（如最大化、最小化、还原）
    if (event->type() == QEvent::WindowStateChange) {
        if (this->isMaximized()) {
            // 当前是最大化状态，按钮应该显示“还原”图标
            ui->toolButton_max->setText("❐");
            ui->toolButton_max->setToolTip("Restore");
        } else {
            // 当前是正常状态，按钮应该显示“最大化”图标
            ui->toolButton_max->setText("☐");
            ui->toolButton_max->setToolTip("Maximize");
        }
    }
    DraggableWidget::changeEvent(event);
}


void MainWindow::on_toolButton_min_clicked()
{
    this->showMinimized();
}


void MainWindow::on_toolButton_max_clicked()
{
    if (this->isMaximized()) {
        this->showNormal();     // 如果已最大化，则还原
    } else {
        this->showMaximized();  // 否则最大化
    }
}


void MainWindow::on_toolButton_close_clicked()
{
    this->close();
}

void MainWindow::initTree()
{
    ui->listWidget_frames->hide();
    // 初始化模型
    m_model = new QStandardItemModel(this);
    m_model->setHorizontalHeaderLabels(QStringList() << "Navigation");
    // 设置视图
    ui->treeView_left->setModel(m_model);
    ui->treeView_left->header()->hide();            // 隐藏表头
    ui->treeView_left->setIndentation(30);          // 缩进
    ui->treeView_left->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->treeView_left->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->treeView_left->expandAll();

    // 启用自定义右键菜单
    ui->treeView_left->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->treeView_left, &QTreeView::customContextMenuRequested,
            this, &MainWindow::onTreeCustomContextMenu);                                                                                                        //HZ  Chen
    // 创建一级目录
    QStringList categories = {"Body","B-G Grid","Mat-Point","Material", "Component","Laser","Init-conditions","Load","Boundary","Powder","OutPut","Solution"};            //HZ  Chen
    QVector<QString> iconPaths = {
        ":/img/tree/body.png",
        ":/img/tree/grid.png",
        ":/img/tree/MatPoint.png",
        ":/img/tree/material.png",
        ":/img/tree/component.png",                                                                                                                             //HZ Chen
        ":/img/tree/laser.png",
        ":/img/tree/init.png",
        ":/img/tree/load.png",
        ":/img/tree/boundary.png",
        ":/img/tree/powder.png",
        ":/img/tree/output.png",
        ":/img/tree/Solution.png"
    };

    for (int i = 0; i < categories.size(); ++i) {
        QStandardItem *item = new QStandardItem(QIcon(iconPaths[i]), categories[i]);
        item->setEditable(false);
        m_model->appendRow(item);
        m_rootItemsMap[categories[i]] = item;  // 存入映射表，便于后续查找
    }

    // 连接点击信号（可选）
    connect(ui->treeView_left, &QTreeView::clicked, this, [=](const QModelIndex &index) {
        QString text = m_model->data(index, Qt::DisplayRole).toString();
        qDebug() << "Clicked:" << text;
        if(text.contains("Body"))
        {

        }
        if(text.contains("B-G Grid"))
        {
            m_bggridDlg->show();
        }
        if(text.contains("Mat-Point"))
        {
            m_matPointdlg->show();
        }
        if(text.contains("Material"))
        {
            m_material->show();
        }
        if(text.contains("Laser"))
        {
            m_LaserDlg->show();
        }
        if(text.contains("Component"))
        {

        }
        if(text.contains("Init-conditions"))
        {
            m_initialDlg ->show();                                                          //HZ Chen
        }
        if(text.contains("Load"))
        {
            m_load->show();
        }
        if(text.contains("OutPut"))
        {
            m_output->show();
        }
        if(text.contains("Solution"))
        {
            m_solution->show();
        }
        if(text.contains("Boundary"))
        {
            m_boundaryDlg->show();
        }
        // 你可以在这里触发页面切换、功能调用等
    });
}

void MainWindow::addSubItems(const QString &parentName, const QStringList &subItems)
{
    if (!m_rootItemsMap.contains(parentName)) {
        qWarning() << "Parent item not found:" << parentName;
        return;
    }

    QStandardItem *parentItem = m_rootItemsMap[parentName];

    for (const QString &sub : subItems) {
        QStandardItem *subItem = new QStandardItem(sub);
        subItem->setEditable(false);
        parentItem->appendRow(subItem);
    }

    // 自动展开父节点（可选）
    QModelIndex parentIndex = m_model->indexFromItem(parentItem);
    ui->treeView_left->expand(parentIndex);
}

bool MainWindow::writeDefaultYml()
{
    QString filePath = QDir::currentPath() + "/out.yml";
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        // 处理文件打开失败的情况
        return false;
    }
    QTextStream out(&file);
    out << "# \n";
    out << "\n";
    out << "solver: SMP\n";
    out << "model: SLM\n";
    out << "\n";
    out << "solve_fluid: true\n";
    out << "solve_solid: true\n";

    file.close();
    return true;
}

void MainWindow::onAddLaserItem(QString name)
{
    if (m_isEditMode && m_editingItem != nullptr)
    {
        // --- 编辑模式：只修改现有节点的名字 ---
        m_editingItem->setText(name);

        // 可以在这里打印日志或更新内部数据映射
        qDebug() << "Updated node to:" << name;
    }
    else
    {
        QStringList nameList;
        nameList<<name;
        addSubItems("Laser",nameList);
    }
    // 操作完成后，建议重置状态，防止干扰下次操作
    m_isEditMode = false;
    m_editingItem = nullptr;
}

void MainWindow::onAddBoundary(QString name)
{
    if (m_isEditMode && m_editingItem != nullptr)
    {
        // --- 编辑模式：只修改现有节点的名字 ---
        m_editingItem->setText(name);

        // 可以在这里打印日志或更新内部数据映射
        qDebug() << "Updated node to:" << name;
    }
    else
    {
    QStringList nameList;
    nameList<<name;
    addSubItems("Boundary",nameList);
    }
    // 操作完成后，建议重置状态，防止干扰下次操作
    m_isEditMode = false;
    m_editingItem = nullptr;
}

void MainWindow::onAddBGgrid(QString name)
{
    if (m_isEditMode && m_editingItem != nullptr)
    {
        // --- 编辑模式：只修改现有节点的名字 ---
        m_editingItem->setText(name);

        // 可以在这里打印日志或更新内部数据映射
        qDebug() << "Updated node to:" << name;
    }
    else
    {
    QStringList nameList;
    nameList<<name;
    addSubItems("B-G Grid",nameList);
    }
    // 操作完成后，建议重置状态，防止干扰下次操作
    m_isEditMode = false;
    m_editingItem = nullptr;
}

void MainWindow::onAddMatPoint(QString name)
{

    if (m_isEditMode && m_editingItem != nullptr)
    {
        // --- 编辑模式：只修改现有节点的名字 ---
        m_editingItem->setText(name);

        // 可以在这里打印日志或更新内部数据映射
        qDebug() << "Updated node to:" << name;
    }
    else
    {
    QStringList nameList;
    nameList<<name;
    addSubItems("Mat-Point",nameList);
    }
    // 操作完成后，建议重置状态，防止干扰下次操作
    m_isEditMode = false;
    m_editingItem = nullptr;
}

void MainWindow::onAddMaterial(QString name)
{
    if (m_isEditMode && m_editingItem != nullptr)
    {
        // --- 编辑模式：只修改现有节点的名字 ---
        m_editingItem->setText(name);

        // 可以在这里打印日志或更新内部数据映射
        qDebug() << "Updated node to:" << name;
    }
    else
    {
    QStringList nameList;
    nameList<<name;
    addSubItems("Material",nameList);
    }
    // 操作完成后，建议重置状态，防止干扰下次操作
    m_isEditMode = false;
    m_editingItem = nullptr;
}

// mainwindow.cpp

// === Gravity (Load) ===
void MainWindow::onAddGravity(QString name) {
    // 【统一风格】判断是否为编辑模式
    if (m_isEditMode && m_editingItem) {
        m_editingItem->setText(name); // 只改名，不新增
        qDebug() << "Updated Gravity node to:" << name;
    } else {
        // 新增模式：调用 addSubItems 或手动 appendRow
        addSubItems("Load", QStringList() << name);
    }
    // 【统一风格】重置状态
    m_isEditMode = false;
    m_editingItem = nullptr;
}

// === Init-conditions ===
void MainWindow::onAddInitTemp(QString name) {
    if (m_isEditMode && m_editingItem) {
        m_editingItem->setText(name);
    } else {
        addSubItems("Init-conditions", QStringList() << name);
    }
    m_isEditMode = false;
    m_editingItem = nullptr;
}

// === Output ===
void MainWindow::onAddOutput(QString name) {
    if (m_isEditMode && m_editingItem) {
        m_editingItem->setText(name);
    } else {
        addSubItems("OutPut", QStringList() << name);
    }
    m_isEditMode = false;
    m_editingItem = nullptr;
}

// === Solution ===
void MainWindow::onAddSolution(QString name) {
    if (m_isEditMode && m_editingItem) {
        m_editingItem->setText(name);
    } else {
        addSubItems("Solution", QStringList() << name);
    }
    m_isEditMode = false;
    m_editingItem = nullptr;
}

void MainWindow::onAddPowder(QString name)
{
    if (m_isEditMode && m_editingItem != nullptr) {
        m_editingItem->setText(name);
    } else {
        addSubItems("Powder", QStringList() << name);
    }
    m_isEditMode = false;
    m_editingItem = nullptr;
}

//Preprocess—B-G grid
void MainWindow::on_toolButton_B_G_grid_clicked()
{
    m_bggridDlg->show();

}


void MainWindow::on_toolButton_MatPoint_clicked()
{
    m_matPointdlg->show();
}


void MainWindow::on_toolButton_open_5_clicked()
{
    m_material->show();
}


void MainWindow::on_toolButton_open_Laser_clicked()
{
    m_LaserDlg->show();
}


void MainWindow::on_toolButton_Global_clicked()
{
    m_material->show();
}


void MainWindow::on_toolButton_Inital_clicked()
{
    m_initialDlg->show();
}


void MainWindow::on_toolButton_Load_clicked()
{
    m_load->show();
}


void MainWindow::on_toolButton_open_Boundary_clicked()
{
    m_boundaryDlg->show();
}


void MainWindow::on_toolButton_output_clicked()
{
    m_output->show();
}


void MainWindow::on_toolButton_solution_clicked()
{
    m_solution->show();
}

void MainWindow::on_toolButton_powder_clicked()
{
    // 点击按钮显示对话框
    if (m_powderDlg) {
        m_powderDlg->setTargetNodeName(""); // 清空编辑状态，视为新建/全局设置
        m_powderDlg->show();
    }
}

void MainWindow::on_toolButton_run_clicked()
{
    this->on_toolButton_save_clicked();
    // 如果已有正在运行的进程，先终止（可选）
    if (m_process && m_process->state() == QProcess::Running) {
        m_process->terminate();
        m_process->waitForFinished(1000);
    }

    // 第一次使用时创建 QProcess
    if (!m_process)
    {
        m_process = new QProcess(this);
        // 连接信号：实时读取输出
        connect(m_process, &QProcess::readyReadStandardOutput, this, [this]() {
            QByteArray data = m_process->readAllStandardOutput();
            QString text = QString::fromLocal8Bit(data);
            ui->textEdit_log->append(text);
            ui->textEdit_log->verticalScrollBar()->setValue(ui->textEdit_log->verticalScrollBar()->maximum());
        });
        connect(m_process, &QProcess::readyReadStandardError, this, [this]() {
            QByteArray data = m_process->readAllStandardError();
            QString text = QString::fromLocal8Bit(data);
            ui->textEdit_log->append("<font color='red'>" + text + "</font>");
            ui->textEdit_log->verticalScrollBar()->setValue(ui->textEdit_log->verticalScrollBar()->maximum());
        });
        connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                this, [this](int exitCode, QProcess::ExitStatus exitStatus) {
                    QString msg = QString("\n--- Process finished with exit code %1 ---").arg(exitCode);
                    if (exitStatus == QProcess::CrashExit) msg += " (crashed)";
                    ui->textEdit_log->append(msg);
                });
    }

    // 1. 准备路径
    QString appDir = QCoreApplication::applicationDirPath();
    QString exePath = appDir + "/AM-FEMP2/muse.exe";

    // --- 定义 out.yml 的路径 (保持绝对路径) ---
    QString yamlPath;
    if (!m_currentProjectDir.isEmpty()) {
        yamlPath = m_currentProjectDir + "/out.yml";
    } else {
        yamlPath = QDir::currentPath() + "/out.yml";
    }

    // --- 【修改点 1】：定义工作目录 (Working Directory) ---
    QString runDir;
    if (!m_currentProjectDir.isEmpty()) {
        // 【核心修改】：工作目录必须是工程根目录，因为 particle.vtk 在这里！
        runDir = m_currentProjectDir;

        // 【可选】：虽然不在 output 跑，但我们还是帮 solver 把 output 文件夹建好
        // 万一 solver 需要往里面写东西但不会自己创建文件夹
        QDir outputDir(runDir + "/output");
        if (!outputDir.exists()) {
            outputDir.mkpath(".");
        }
    } else {
        // 如果没有工程，回退到 exe 所在目录
        runDir = appDir + "/AM-FEMP2";
    }

    // 2. 检查文件是否存在
    if (!QFile::exists(yamlPath)) {
        ui->textEdit_log->append("<font color='red'>Error: Configuration file not found: " + yamlPath + "</font>");
        return;
    }
    // 检查 particle.vtk 是否存在 (为了调试方便，建议加上这个检查)
    if (!QFile::exists(runDir + "/particle.vtk")) {
        ui->textEdit_log->append("<font color='orange'>Warning: particle.vtk not found in working directory: " + runDir + "</font>");
    }

    if (!QFile::exists(exePath)) {
        ui->textEdit_log->append("<font color='red'>Error: " + exePath + " not found!</font>");
        return;
    }

    // 3. 启动求解器
    ui->textEdit_log->append("Starting: " + exePath);
    ui->textEdit_log->append("Working Directory: " + runDir); // 打印确认路径

    // 【修改点 2】：设置工作目录为工程根目录
    m_process->setWorkingDirectory(runDir);

    // 启动 (参数传递 yaml 的路径)
    m_process->start(exePath, QStringList() << yamlPath);

    if (!m_process->waitForStarted()) {
        ui->textEdit_log->append("<font color='red'>Failed to start muse.exe!</font>");
    }

}



// 【修改点 1】实现全量 YAML 组装逻辑
void MainWindow::generateFullYaml()
{
    QString filePath = m_currentProjectDir + "/out.yml"; // 确保路径正确
    QFile file(filePath);
    QStringList preservedLines;

    // === A. 定义主界面“接管”了哪些 Key (遇到这些开头的旧内容要扔掉，用新的替换) ===
    QSet<QString> managedKeys = {
        // SolutionDlg 负责的:
        "solver:", "model:", "solve_fluid:", "solve_solid:",
        "interface_damp:", "MPM_damp:", "FEM_damp:", "PIC_damp:",
        "point_per_cell:", "rearrange:", "max_point_per_cell:", "min_point_per_cell:",
        "time:",
        // OutputSettingDialog 负责的:
        "output:"
    };

    // === B. 读取旧文件，保留“未接管”的部分 (如 laser, background_mesh) ===
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        bool skipSection = false; // 标记是否正在跳过一个被接管的段落

        while (!in.atEnd()) {
            QString line = in.readLine();
            QString trimmed = line.trimmed();

            // 1. 保留空行和注释
            if (trimmed.isEmpty() || trimmed.startsWith("#")) {
                if (!skipSection) preservedLines.append(line);
                continue;
            }

            // 2. 检查是否是顶级 Key (无缩进)
            if (!line.startsWith(" ") && !line.startsWith("\t")) {
                QString key = trimmed.split(":").first() + ":";

                if (managedKeys.contains(key)) {
                    // 这是一个我们要重写的 Key，开启跳过模式
                    skipSection = true;
                } else {
                    // 这是一个我们要保留的 Key (比如 laser:)，关闭跳过模式
                    skipSection = false;
                    preservedLines.append(line);
                }
            } else {
                // 3. 处理有缩进的子项
                if (!skipSection) {
                    preservedLines.append(line); // 如果不在跳过模式，就保留
                }
            }
        }
        file.close();
    }

    // === C. 组装新内容 ===
    QString newContent;

    // 1. [Solution] 头部信息 (Solver, Model...)
    // 确保你已经在 solutiondlg.cpp 里实现了 getSolverYaml()
    if (m_solution) newContent += m_solution->getSolverYaml() + "\n";

    // 2. [Solution] MPM 参数
    if (m_solution) newContent += m_solution->getMPMYaml() + "\n";

    // 3. [Solution] Time 参数
    if (m_solution) newContent += m_solution->getTimeYaml() + "\n";

    // 4. [Preserved] 插入保留的旧内容 (Laser, Grid 等)
    if (!preservedLines.isEmpty()) {
        newContent += preservedLines.join("\n") + "\n\n";
    }

    // 5. [Output] 输出设置 (放在最后比较整齐)
    // 调用刚才在 OutputSettingDialog 新写的函数
    if (m_output) newContent += m_output->getOutputYaml() + "\n";

    // === D. 写入文件 ===
    if (file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        QTextStream out(&file);
        out << newContent;
        file.close();
        // 可以在这里打印日志： qDebug() << "out.yml updated successfully.";
    } else {
        QMessageBox::warning(this, "Error", "Failed to write out.yml");
    }
}



void MainWindow::on_toolButton_new_clicked()
{
    // 1. 用户选择父级目录
    QString parentDir = QFileDialog::getExistingDirectory(this, "选择工程保存位置", QDir::currentPath());
    if (parentDir.isEmpty()) return;

    // 2. 输入项目名
    bool ok;
    QString projectName = QInputDialog::getText(this, "新建工程", "请输入工程名称:", QLineEdit::Normal, "NewProject", &ok);
    if (!ok || projectName.isEmpty()) return;

    // 3. 创建文件夹结构
    m_currentProjectDir = parentDir + "/" + projectName;
    QDir dir(m_currentProjectDir);
    if (dir.exists()) {
        QMessageBox::warning(this, "警告", "目录已存在，请重新命名。");
        return;
    }

    dir.mkpath(".");
    dir.mkdir("input");
    dir.mkdir("output");

    // 【关键步骤 1】切换当前工作目录到新工程目录
    QDir::setCurrent(m_currentProjectDir);

    // 4. 初始化一个空的 .sim 文件
    QString simPath = m_currentProjectDir + "/" + projectName + ".sim";
    QFile file(simPath);
    if (file.open(QIODevice::WriteOnly)) {
        QJsonObject root;
        root["ProjectName"] = projectName;
        root["CreateTime"] = QDateTime::currentDateTime().toString();
        file.write(QJsonDocument(root).toJson());
        file.close();
    }

    // 【关键步骤 2】在新目录下生成默认的 out.yml
    // 因为工作目录已经切换了，writeDefaultYml 里的 QDir::currentPath() 现在指向新目录
    if (writeDefaultYml()) {
        ui->textEdit_log->append("Initialized default out.yml in " + m_currentProjectDir);
    } else {
        QMessageBox::critical(this, "Error", "Failed to create out.yml in new project!");
        return;
    }

    QMessageBox::information(this, "成功", "工程已初始化完成。");
}

// 序列化：将 Dialog 里的输入控件转为 JsonObject
QJsonObject MainWindow::serializeDialog(QWidget *dlg)
{
    if (!dlg) return QJsonObject();
    QJsonObject obj;
    // 处理所有 LineEdit (保留科学计数法字符串)
    for (QLineEdit *e : dlg->findChildren<QLineEdit *>()) {
        if (!e->objectName().isEmpty()) obj[e->objectName()] = e->text();
    }
    // 处理 ComboBox
    for (QComboBox *c : dlg->findChildren<QComboBox *>()) {
        if (!c->objectName().isEmpty()) obj[c->objectName()] = c->currentIndex();
    }
    // 处理 CheckBox
    for (QCheckBox *b : dlg->findChildren<QCheckBox *>()) {
        if (!b->objectName().isEmpty()) obj[b->objectName()] = b->isChecked();
    }

    // 处理 QLabel
    for (QLabel *l : dlg->findChildren<QLabel *>()) {
        if (!l->objectName().isEmpty()) obj[l->objectName()] = l->text();
    }

    // 处理 QTableWidget
    for (QTableWidget *t : dlg->findChildren<QTableWidget *>()) {
        QString name = t->objectName();
        if (name.isEmpty()) continue;

        QJsonArray tableArray;
        for (int r = 0; r < t->rowCount(); ++r) {
            QJsonArray rowArray;
            for (int c = 0; c < t->columnCount(); ++c) {
                QTableWidgetItem *item = t->item(r, c);
                rowArray.append(item ? item->text() : "");
            }
            tableArray.append(rowArray);
        }
        obj[name] = tableArray;
    }

    return obj;
}

// 反序列化：将 JsonObject 数据填回 Dialog
void MainWindow::deserializeDialog(QWidget *dlg, const QJsonObject &data)
{
    if (!dlg || data.isEmpty()) return;
    for (auto it = data.begin(); it != data.end(); ++it) {
         QString key = it.key();
        QJsonValue val = it.value();

        QLineEdit *e = dlg->findChild<QLineEdit *>(key);
        if (e) { e->setText(it.value().toString()); continue; }

        QComboBox *c = dlg->findChild<QComboBox *>(key);
        if (c) { c->setCurrentIndex(it.value().toInt()); continue; }

        QCheckBox *b = dlg->findChild<QCheckBox *>(key);
        if (b) { b->setChecked(it.value().toBool()); continue; }

        QLabel *l = dlg->findChild<QLabel *>(key);
        if (l) { l->setText(it.value().toString()); continue; }

        QTableWidget *t = dlg->findChild<QTableWidget *>(key);
        if (t && val.isArray()) {
            QJsonArray tableArray = val.toArray();
            // 如果需要根据数据动态调整行数，取消下面注释
            // t->setRowCount(tableArray.size());
            for (int r = 0; r < qMin(tableArray.size(), t->rowCount()); ++r) {
                QJsonArray rowData = tableArray[r].toArray();
                for (int c = 0; c < qMin(rowData.size(), t->columnCount()); ++c) {
                    if (!t->item(r, c)) t->setItem(r, c, new QTableWidgetItem());
                    t->item(r, c)->setText(rowData[c].toString());
                }
            }
            continue;
        }
    }
}


void MainWindow::on_toolButton_save_clicked()
{
    if (m_currentProjectDir.isEmpty()) {
        on_toolButton_new_clicked(); // 如果没新建就保存，自动跳转新建
        if (m_currentProjectDir.isEmpty()) return;
    }

    // 1. 保存 .sim 文件
    QJsonObject root;
    root["BGgridSetting"] = serializeDialog(m_bggridDlg);
    root["MatPointSetting"] = serializeDialog(m_matPointdlg);
    root["MaterialEditor"] = serializeDialog(m_material);
    root["LaserSetting"] = serializeDialog(m_LaserDlg);
    root["InitialTemperatureDialog"] = serializeDialog(m_initialDlg);
    root["GravityDialog"] = serializeDialog(m_load);
    root["BoundaryDlg"] = serializeDialog(m_boundaryDlg);
    root["PowderDlg"] = serializeDialog(m_powderDlg);
    root["OutputSettingDialog"] = serializeDialog(m_output);
    root["SolutionDlg"] = serializeDialog(m_solution);


    QString projectName = QFileInfo(m_currentProjectDir).fileName();
    QString simPath = m_currentProjectDir + "/" + projectName + ".sim";

    QFile file(simPath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QJsonDocument(root).toJson());
        file.close();
    }

    generateFullYaml();

    QMessageBox::information(this, "保存", "工程配置文件(.sim) 与 求解文件(out.yml) 已更新。");


}

void MainWindow::on_toolButton_open_clicked()
{
    if (vtuWatcher->isRunning()) {
        vtuWatcher->cancel();
        vtuWatcher->waitForFinished();
    }

    if (this->m_timer->isActive()) {
        this->m_timer->stop();
    }

    // 打开新文件序列时，必须清空旧的全局范围缓存
    m_globalDataRanges.clear();

    // 【关键修改】这里必须使用 actorPost 和 dsMapperPost
    if (this->actorPost && this->dsMapperPost) {
        this->actorPost->SetMapper(this->dsMapperPost);
        this->actorPost->GetProperty()->SetRepresentationToSurface();
        this->actorPost->GetProperty()->SetEdgeVisibility(true);
    }

    QSettings settings("setting", "PATH");
    QString savedPath = settings.value("lastPath", QDir::currentPath()).toString();

    // 弹出对话框
    QString selectedFile = QFileDialog::getOpenFileName(
        this, tr("选择数据文件"), savedPath,
        tr("All Supported Files (*.sim *.vtu *.vtk);;"
           "Project Files (*.sim);;"
           "Data Files (*.vtu *.vtk);;"
           "All Files (*.*)"));

    if (selectedFile.isEmpty()) return;

    QFileInfo fileInfo(selectedFile);
    QString suffix = fileInfo.suffix().toLower();
    if (suffix == "sim") {
        loadProjectLogic(selectedFile);
    } else {
        processVtkSequence(selectedFile);
    }

    // 将新路径保存到配置
    QString newPath = QFileInfo(selectedFile).absolutePath();
    settings.setValue("lastPath", newPath);
}


void MainWindow::loadProjectLogic(const QString &simFilePath)
{
    QFile file(simFilePath);
    if (!file.open(QIODevice::ReadOnly)) return;

    QByteArray jsonData = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(jsonData);
    if (doc.isNull()) return;

    QJsonObject root = doc.object();

    // 设置当前工程目录
    QFileInfo info(simFilePath);
    this->m_currentProjectDir = info.absolutePath(); // 记录工程根目录

    // 【新增】将系统的当前工作目录切换到工程目录
    // 这样后续所有 dialog 写入 "config_temp.json" 时，都会写到这个文件夹里
    QDir::setCurrent(m_currentProjectDir);

    // 恢复各个对话框的状态（如果指针为空则 new）
    auto restore = [&](QWidget** dlgPtr, const QString& key, auto factory) {
        if (root.contains(key)) {
            if (!(*dlgPtr)) *dlgPtr = factory();
            // 这里调用我们之前定义的序列化工具
            deserializeDialog(*dlgPtr, root[key].toObject());
        }
    };

    restore((QWidget**)&m_bggridDlg, "BGgridSetting", [this](){ return new BGgridSetting(this); });
    restore((QWidget**)&m_matPointdlg, "MatPointSetting", [this](){ return new MatPointSetting(this); });
    restore((QWidget**)&m_material, "MaterialEditor", [this](){ return new MaterialEditor(this); });
    restore((QWidget**)&m_LaserDlg, "LaserSetting", [this](){ return new LaserSettingDialog(this); });
    restore((QWidget**)&m_initialDlg, "InitialTemperatureDialog", [this](){ return new InitialTemperatureDialog(this); });
    restore((QWidget**)&m_load, "GravityDialog", [this](){ return new GravityDialog(this); });
    restore((QWidget**)&m_boundaryDlg, "BoundaryDlg", [this](){ return new BoundaryDlg(this); });
    restore((QWidget**)&m_powderDlg, "PowderDlg", [this](){ return new PowderDlg(this); });
    restore((QWidget**)&m_output, "OutputSettingDialog", [this](){ return new OutputSettingDialog(this); });
    restore((QWidget**)&m_solution, "SolutionDlg", [this](){ return new SolutionDlg(this); });

    // 【新增修复】强制将加载到界面的数据写入 config_temp.json
    // ==========================================================

    // 1. 更新背景网格数据到 JSON (屏蔽信号，防止提前触发 EXE)
    if (m_bggridDlg) {
        m_bggridDlg->blockSignals(true);
        m_bggridDlg->writeJsonFile();
        m_bggridDlg->blockSignals(false);
    }

    // 2. 更新材料点数据到 JSON
    if (m_matPointdlg) {
        m_matPointdlg->blockSignals(true);
        m_matPointdlg->writeJsonFile();
        m_matPointdlg->blockSignals(false);
    }

    // 3. 更新粉末数据到 JSON
    if (m_powderDlg) {
        m_powderDlg->blockSignals(true);
        m_powderDlg->writeJsonFile(); // 调用刚才新写的函数
        m_powderDlg->blockSignals(false);
    }
    // 这会启动 PreVisGenerator.exe，生成 preview.vtu，并自动显示在前处理 Tab 页
    this->onJsonDataUpdate();

    qDebug() << "Project loaded. Triggered PreVisGenerator for preview.";
}

void MainWindow::processVtkSequence(const QString &selectedFile)
{
    // 【关键修改】使用 dsMapperPost 和 gaussianMapperPost
    if (this->dsMapperPost) this->dsMapperPost->SetInputDataObject(nullptr);
    if (this->gaussianMapperPost) this->gaussianMapperPost->SetInputData(nullptr);

    this->currentDataSet = nullptr;

    // 3. 强制模式回归 Surface
    ui->comboBox_rendering->blockSignals(true);
    ui->comboBox_rendering->setCurrentText("Surface");
    ui->comboBox_rendering->blockSignals(false);

    // 清空参数输入框
    ui->lineEdit_PointSize->clear();
    ui->LineEdit_GaussianRadius->clear();

    // 初始化 VTK 管线
    if (!this->vtkPipelineInitialized) {
        this->setupVtkPipeline();
        this->vtkPipelineInitialized = true;
    }

    // 5. 识别文件序列并进行【自然排序】
    QFileInfo fileInfo(selectedFile);
    QString dirPath = fileInfo.absolutePath();
    QString fileName = fileInfo.fileName();

    QRegExp rx("^(.*[._]?)(\\d+)(\\.[^.]+)$");
    this->fileList.clear();
    ui->listWidget_frames->clear();

    if (rx.indexIn(fileName) != -1) {
        QString prefix = rx.cap(1);
        QString suffix = rx.cap(3);
        QDir dir(dirPath);
        QStringList filters;
        filters << prefix + "*" + suffix;

        // 获取文件列表
        QFileInfoList list = dir.entryInfoList(filters, QDir::Files, QDir::Unsorted);

        // 自然排序逻辑
        std::sort(list.begin(), list.end(), [](const QFileInfo &a, const QFileInfo &b) {
            auto extractNumber = [](QString name) -> qlonglong {
                QString numStr;
                for (int i = 0; i < name.length(); ++i) {
                    if (name[i].isDigit()) {
                        int j = i;
                        while (j < name.length() && name[j].isDigit()) {
                            numStr.append(name[j]);
                            j++;
                        }
                        break;
                    }
                }
                return numStr.isEmpty() ? -1 : numStr.toLongLong();
            };

            qlonglong nA = extractNumber(a.fileName());
            qlonglong nB = extractNumber(b.fileName());

            if (nA != nB) return nA < nB;
            return a.fileName() < b.fileName();
        });

        for (const QFileInfo &info : list) {
            this->fileList.append(info.absoluteFilePath());
            ui->listWidget_frames->addItem(info.fileName());
        }

        // 定位索引
        this->currentFrameIndex = 0;
        for(int i = 0; i < fileList.size(); ++i) {
            if(fileList[i] == selectedFile) {
                this->currentFrameIndex = i;
                break;
            }
        }
    } else {
        // 单文件
        this->fileList.append(selectedFile);
        ui->listWidget_frames->addItem(fileName);
        this->currentFrameIndex = 0;
    }

    if (ui->listWidget_frames->count() > 0) {
        ui->listWidget_frames->setCurrentRow(this->currentFrameIndex);
    }

    this->isFirstLoadOfSequence = true;
    ui->listWidget_frames->show();
    ui->lineEdit_PointSize->clear();
    ui->LineEdit_GaussianRadius->clear();

    if (!this->fileList.isEmpty()) {
        this->startLoadFrame(this->currentFrameIndex);
    }

    // 【新增】只要加载了序列，就应该切换到后处理 Tab
    ui->tabWidget_center->setCurrentIndex(1);
}


void MainWindow::on_toolButton_last_clicked()
{
    // 【修改】如果任务正在进行，则禁止操作
    if (this->fileList.isEmpty() || vtuWatcher->isRunning()) return;

    int newIndex = this->currentFrameIndex - 1;
    if (newIndex < 0)
    {
        newIndex = this->fileList.count() - 1; // 循环到最后一帧
    }
    this->startLoadFrame(newIndex); // 【修改】使用异步加载
}


void MainWindow::on_toolButton_next_clicked()
{
    // 【修改】如果任务正在进行，则禁止操作
    if (this->fileList.isEmpty() || vtuWatcher->isRunning()) return;

    int newIndex = this->currentFrameIndex + 1;
    if (newIndex >= this->fileList.count())
    {
        newIndex = 0; // 循环到第一帧
    }
    this->startLoadFrame(newIndex); // 【修改】使用异步加载
}


void MainWindow::on_toolButton_play_clicked()
{
    // 1. 安全检查
    if (this->fileList.isEmpty()) return;

    // 2. 判断定时器状态
    if (this->m_timer->isActive())
    {
        // ==========================
        // 当前正在播放 -> 执行暂停
        // ==========================
        this->m_timer->stop();

        // 【UI变换】：既然暂停了，图标要变回“播放”箭头，提示用户可以继续播放
        ui->toolButton_play->setIcon(QIcon(":/img/play.png"));
        ui->toolButton_play->setToolTip(tr("开始播放"));
    }
    else
    {
        // ==========================
        // 当前是暂停状态 -> 执行播放
        // ==========================

        // 逻辑：如果当前帧没有加载（索引为-1），从第0帧开始
        if (this->currentFrameIndex == -1) {
            this->startLoadFrame(0);
        }
        this->m_timer->start(100);

        // 【UI变换】：既然开始跑了，图标要变成“双竖线(暂停)”，提示用户可以暂停
        ui->toolButton_play->setIcon(QIcon(":/img/pause.png"));
        ui->toolButton_play->setToolTip(tr("暂停播放"));
    }
}

// 自动播放时调用
void MainWindow::playNextFrame()
{
    // 只有在当前没有加载任务时，才启动下一帧加载
    if (!vtuWatcher->isRunning()) {
        this->on_toolButton_next_clicked();
    }
}


void MainWindow::on_comboBox_attributes_currentTextChanged(const QString &arg1)
{
    // 切换着色属性时，不需要重新加载文件，只需要更新着色管线
    ui->comboBox_fenliang->clear();
    updateComponentComboBox();
    this->updateVtkColoring(arg1);
}

void MainWindow::setupVtkPipeline()
{
    // ==================================================
    // 1. 初始化 Pre-process (前处理) 管线
    // ==================================================
    // 1.1 渲染器与窗口绑定
    this->rendererPre = vtkSmartPointer<vtkRenderer>::New();
    this->rendererPre->SetBackground(1.0, 1.0, 1.0); // 浅灰色背景，区分前处理

    auto renderWindowPre = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    ui->widget_vtk_pre->setRenderWindow(renderWindowPre); // 绑定 UI: widget_vtk_pre
    renderWindowPre->AddRenderer(this->rendererPre);

    // 1.2 Mapper 和 Actor
    this->mapperPre = vtkSmartPointer<vtkDataSetMapper>::New();
    this->actorPre = vtkSmartPointer<vtkActor>::New();
    this->actorPre->SetMapper(this->mapperPre);
    this->rendererPre->AddActor(this->actorPre);

    // 1.3 前处理坐标轴
    auto axesActorPre = vtkSmartPointer<vtkAxesActor>::New();
    this->axesWidgetPre = vtkSmartPointer<vtkOrientationMarkerWidget>::New();
    this->axesWidgetPre->SetOrientationMarker(axesActorPre);
    this->axesWidgetPre->SetInteractor(ui->widget_vtk_pre->interactor()); // 绑定 Pre 交互器
    this->axesWidgetPre->SetEnabled(1);
    this->axesWidgetPre->InteractiveOn();


    // ==================================================
    // 2. 初始化 Post-process (后处理) 管线
    // ==================================================
    // 2.1 渲染器与窗口绑定
    this->rendererPost = vtkSmartPointer<vtkRenderer>::New();
    this->rendererPost->SetBackground(1.0, 1.0, 1.0); // 纯白背景

    auto renderWindowPost = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    ui->widget_vtk_post->setRenderWindow(renderWindowPost); // 绑定 UI: widget_vtk_post
    renderWindowPost->AddRenderer(this->rendererPost);

    // 2.2 颜色表 (LUT)
    colorLUT = vtkSmartPointer<vtkLookupTable>::New();
    colorLUT->SetHueRange(0.667, 0.0); // 蓝到红
    colorLUT->SetNumberOfTableValues(256);
    colorLUT->Build();

    // 2.3 两种 Mapper
    dsMapperPost = vtkSmartPointer<vtkDataSetMapper>::New();
    dsMapperPost->SetLookupTable(colorLUT);

    gaussianMapperPost = vtkSmartPointer<vtkPointGaussianMapper>::New();
    gaussianMapperPost->SetLookupTable(colorLUT);
    gaussianMapperPost->SetSplatShaderCode(
        "//VTK::Color::Impl\n"
        "float dist = dot(offsetVCVSOutput.xy, offsetVCVSOutput.xy);\n"
        "if (dist > 1.0) discard;\n"
        "float diffuse = sqrt(1.0 - dist);\n"
        "ambientColor *= diffuse;\n"
        "diffuseColor *= diffuse;\n"
        );

    // 2.4 Actor
    this->actorPost = vtkSmartPointer<vtkActor>::New();
    this->actorPost->SetMapper(dsMapperPost); // 默认使用普通 Mapper
    this->rendererPost->AddActor(this->actorPost);
    // 2.5 色卡 (ScalarBar) - 仅 Post 需要
    this->scalarBarWidget = vtkSmartPointer<vtkScalarBarWidget>::New();
    this->scalarBarWidget->SetInteractor(ui->widget_vtk_post->interactor()); // 绑定 Post 交互器
    this->scalarBarWidget->SetRepositionable(true);
    this->scalarBarWidget->SetResizable(true);
    this->scalarBarWidget->CreateDefaultRepresentation();

    // 设置色卡样式 (省略部分样式代码，保持你原有的即可)
    vtkScalarBarRepresentation* rep =
        vtkScalarBarRepresentation::SafeDownCast(this->scalarBarWidget->GetRepresentation());

    if (rep) {
        // 设置位置 (左下角坐标，0.0~1.0)
        // x=0.88 (靠右), y=0.25 (垂直居中附近)
        rep->GetPositionCoordinate()->SetValue(0.88, 0.1);

        // 设置大小 (宽度和高度，0.0~1.0)
        // width=0.08 (宽度), height=0.5 (高度)
        rep->GetPosition2Coordinate()->SetValue(0.04, 0.35);
    }

    // 3. 继续设置内部 Actor 的样式 (文字、标题等)
    vtkScalarBarActor* barActor = this->scalarBarWidget->GetScalarBarActor();
    barActor->SetLookupTable(colorLUT);
    barActor->SetTitle("Loading...");

    // 强制垂直方向 (让 Representation 知道它是竖着的)
    barActor->SetOrientationToVertical();

    // 刻度设置
    barActor->SetNumberOfLabels(8);
    barActor->SetLabelFormat("%.1e");
    barActor->GetLabelTextProperty()->SetColor(0, 0, 0);
    barActor->GetLabelTextProperty()->SetFontSize(12);
    barActor->GetLabelTextProperty()->BoldOff();
    barActor->GetLabelTextProperty()->ShadowOff();

    // 标题设置
    vtkTextProperty* titleProp = barActor->GetTitleTextProperty();
    titleProp->SetColor(0, 0, 0);
    titleProp->SetFontSize(14);
    titleProp->BoldOn();
    titleProp->ShadowOff();
    titleProp->SetJustificationToCentered();

    // 初始关闭
    this->scalarBarWidget->Off();

    // 2.6 后处理坐标轴
    auto axesActorPost = vtkSmartPointer<vtkAxesActor>::New();
    this->axesWidgetPost = vtkSmartPointer<vtkOrientationMarkerWidget>::New();
    this->axesWidgetPost->SetOrientationMarker(axesActorPost);
    this->axesWidgetPost->SetInteractor(ui->widget_vtk_post->interactor()); // 绑定 Post 交互器
    this->axesWidgetPost->SetEnabled(1);
    this->axesWidgetPost->InteractiveOn();
}



// 【修改】辅助函数：启动异步加载任务
void MainWindow::startLoadFrame(int index)
{
    if (vtuWatcher->isRunning()) {
        qDebug() << "Warning: Previous load task is still running. Aborting new request.";
        return;
    }

    if (index >= 0 && index < this->fileList.count())
    {
        this->currentFrameIndex = index;
        const QString& filePath = this->fileList.at(index);

        qDebug() << "Starting Async Load Frame" << index << ":" << filePath;

        // 设置 UI 状态
        //this->setCursor(Qt::WaitCursor);
        ui->progressBar->setValue(20);
        ui->toolButton_last->setEnabled(false); // 禁用控制按钮
        ui->toolButton_next->setEnabled(false);

        // // 在子线程中执行耗时的 I/O 操作
        // QFuture<vtkSmartPointer<vtkDataSet>> future = QtConcurrent::run([filePath]() -> vtkSmartPointer<vtkDataSet> {

        //     // --- 子线程操作 ---
        //     auto localReader = vtkSmartPointer<vtkXMLUnstructuredGridReader>::New();
        //     localReader->SetFileName(filePath.toStdString().c_str());
        //     localReader->Update(); // <-- 耗时操作，在子线程执行

        //     // 返回 SmartPointer
        //     vtkUnstructuredGrid* rawData = localReader->GetOutput();
        //     vtkDataSet* dataSetPtr = (vtkDataSet*)rawData;
        //     return dataSetPtr;
        // });
        QFuture<vtkSmartPointer<vtkDataSet>> future = QtConcurrent::run([filePath]() -> vtkSmartPointer<vtkDataSet> {
            vtkSmartPointer<vtkDataSet> dataSet = nullptr;

            if (filePath.endsWith(".vtu", Qt::CaseInsensitive)) {
                auto reader = vtkSmartPointer<vtkXMLUnstructuredGridReader>::New();
                reader->SetFileName(filePath.toLocal8Bit().constData());
                reader->Update();
                dataSet = reader->GetOutput();
            }
            else if (filePath.endsWith(".vtk", Qt::CaseInsensitive)) {
                auto reader = vtkSmartPointer<vtkGenericDataObjectReader>::New();
                reader->SetFileName(filePath.toLocal8Bit().constData());
                reader->Update();
                // 自动识别数据类型并转换为基类 vtkDataSet
                dataSet = vtkDataSet::SafeDownCast(reader->GetOutput());
            }
            return dataSet;
        });

        // 将任务交给 Watcher 监控
        vtuWatcher->setFuture(future);
    }
}

void MainWindow::onVtuLoadFinished()
{
    ui->progressBar->setValue(100);
    this->setCursor(Qt::ArrowCursor);
    ui->toolButton_last->setEnabled(true);
    ui->toolButton_next->setEnabled(true);

    vtkSmartPointer<vtkDataSet> data = vtuWatcher->result();
    if (!data || data->GetNumberOfPoints() == 0) return;
    this->currentDataSet = data;


    vtkPointData* pd = data->GetPointData();

    // 检查是否存在 radius 数组
    vtkDataArray* radiusArray = pd->GetArray("radius");
    if (radiusArray) {
        qDebug() << "Detected 'radius' array. Using Glyph3D for spheres.";

        auto sphereSource = vtkSmartPointer<vtkSphereSource>::New();
        sphereSource->SetThetaResolution(12);
        sphereSource->SetPhiResolution(12);

        auto glyph3D = vtkSmartPointer<vtkGlyph3D>::New();
        glyph3D->SetSourceConnection(sphereSource->GetOutputPort());
        glyph3D->SetInputData(data);

        // 1. 设置缩放模式为按标量缩放
        glyph3D->SetScaleModeToScaleByScalar();

        // 2. 【核心修复】指定 radius 数组用于缩放
        // 需要包含 #include <vtkDataObject.h>
        glyph3D->SetInputArrayToProcess(0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_POINTS, "radius");

        // 3. 设置比例因子。如果 radius 是真实半径，由于 SphereSource 默认直径为1，
        // 所以 Factor 设为 2.0 得到的就是真实物理尺寸。
        glyph3D->SetScaleFactor(2.0);

        glyph3D->Update();

        this->dsMapperPost->SetInputConnection(glyph3D->GetOutputPort());
        this->actorPost->SetMapper(this->dsMapperPost);

        ui->LineEdit_GaussianRadius->setText("Physical Radius");
        ui->LineEdit_GaussianRadius->setEnabled(false);
    }
    else
    {
        ui->LineEdit_GaussianRadius->setEnabled(true);
        // --- 逻辑 B：普通模型显示 (原有逻辑) ---
        qDebug() << "No 'radius' array. Standard model rendering.";
        QString mode = ui->comboBox_rendering->currentText();

        if (mode == "Point Gaussian") {
            auto surfaceFilter = vtkSmartPointer<vtkDataSetSurfaceFilter>::New();
            surfaceFilter->SetInputData(this->currentDataSet);
            surfaceFilter->Update(); // 必须 Update

            this->gaussianMapperPost->SetInputData(surfaceFilter->GetOutput());
            this->actorPost->SetMapper(this->gaussianMapperPost);
        } else {
            dsMapperPost->SetInputDataObject(this->currentDataSet);
            this->actorPost->SetMapper(dsMapperPost);
        }
    }


    updateAttributeComboBoxSmart();
    updateVtkColoring(ui->comboBox_attributes->currentText());

    if (this->isFirstLoadOfSequence) {
        this->rendererPost->ResetCamera();
        this->isFirstLoadOfSequence = false;
    }
    ui->widget_vtk_post->renderWindow()->Render();

    // 【新增】 自动切换到“后处理”页面 (假设索引1是后处理)
    ui->tabWidget_center->setCurrentIndex(1);

    ui->listWidget_frames->blockSignals(true);
    ui->listWidget_frames->setCurrentRow(this->currentFrameIndex);
    ui->listWidget_frames->blockSignals(false);
}

vtkRenderer* MainWindow::getCurrentRenderer()
{
    // 这里的 tabWidget_center 是我们在 UI 里新加的那个 Tab 组件的名字
    // currentIndex() == 0 代表第一个页面（Pre-process）
    if (ui->tabWidget_center->currentIndex() == 0) {
        return this->rendererPre;
    } else {
        return this->rendererPost;
    }
}

// 智能属性更新函数
void MainWindow::updateAttributeComboBoxSmart()
{
    if (!this->currentDataSet) return;

    vtkPointData* pd = this->currentDataSet->GetPointData();
    int numArrays = pd->GetNumberOfArrays();

    QStringList newAttributes;
    for (int i = 0; i < numArrays; ++i) {
        newAttributes << QString::fromUtf8(pd->GetArrayName(i));
    }

    // 获取当前 ComboBox 已有的属性列表
    QStringList oldAttributes;
    for (int i = 0; i < ui->comboBox_attributes->count(); ++i) {
        oldAttributes << ui->comboBox_attributes->itemText(i);
    }

    // 如果新旧列表不一致，才重新填充（避免频繁刷新导致选择丢失）
    if (newAttributes != oldAttributes) {
        QString currentSelected = ui->comboBox_attributes->currentText();

        ui->comboBox_attributes->blockSignals(true); // 暂时阻塞信号
        ui->comboBox_attributes->clear();
        if (newAttributes.isEmpty()) {
            ui->comboBox_attributes->addItem("No Scalar Data Found");
        } else {
            ui->comboBox_attributes->addItems(newAttributes);
            // 尝试保持之前选中的属性名
            int idx = ui->comboBox_attributes->findText(currentSelected);
            if (idx != -1) ui->comboBox_attributes->setCurrentIndex(idx);
        }
        ui->comboBox_attributes->blockSignals(false);
    }

    //根据当前选中的属性维度，更新分量下拉框
    updateComponentComboBox();
    // 强制执行一次着色更新，确保新帧数据范围生效
   // updateVtkColoring(ui->comboBox_attributes->currentText());
}

void MainWindow::updateComponentComboBox()
{
    QString attrName = ui->comboBox_attributes->currentText();
    if (attrName == "No Scalar Data Found" || !this->currentDataSet) {
        ui->comboBox_fenliang->clear();
        return;
    }

    vtkDataArray* arr = this->currentDataSet->GetPointData()->GetArray(attrName.toStdString().c_str());
    if (!arr) return;

    int comps = arr->GetNumberOfComponents();
    QString currentComp = ui->comboBox_fenliang->currentText();

    ui->comboBox_fenliang->blockSignals(true);
    ui->comboBox_fenliang->clear();

    if (comps == 1) {
        ui->comboBox_fenliang->addItem("Scalar");
    }
    else if (comps == 3) {
        // 3分量矢量：模长 + X, Y, Z
        ui->comboBox_fenliang->addItems({"Magnitude", "X", "Y", "Z"});
    }
    else if (comps == 6) {
        // 6分量张量：模长 + 六个独立分量
        // 修改点：在这里也加入了 "Magnitude"
        ui->comboBox_fenliang->addItems({"Magnitude", "XX", "YY", "ZZ", "XY", "YZ", "XZ"});
    }

    // 尝试恢复之前的分量选择（如果之前选的是 Magnitude，换到另一个 6 分量属性时依然会保持 Magnitude）
    int idx = ui->comboBox_fenliang->findText(currentComp);
    if (idx != -1) {
        ui->comboBox_fenliang->setCurrentIndex(idx);
    } else {
        ui->comboBox_fenliang->setCurrentIndex(0);
    }

    ui->comboBox_fenliang->blockSignals(false);

    // 触发渲染更新
    updateVtkColoring(attrName);
}

void MainWindow::updateVtkColoring(const QString& attributeName)
{
    // 基础检查：无数据或属性无效时，隐藏色卡
    if (attributeName.isEmpty() || attributeName == "No Scalar Data Found" || !this->currentDataSet) {
        if (this->scalarBarWidget) this->scalarBarWidget->Off();
        if (this->customTitleActor) this->customTitleActor->SetVisibility(false);
        return;
    }

    // 1. 获取基础数据数组
    vtkDataArray* dataArray = this->currentDataSet->GetPointData()->GetArray(attributeName.toStdString().c_str());
    if (!dataArray) return;

    // 2. 解析分量索引 (Component Index)
    QString compStr = ui->comboBox_fenliang->currentText();
    int componentIndex = -1; // VTK 默认 -1 为 Magnitude

    // 映射表：将 UI 文字转为数组索引
    static QMap<QString, int> compMap = {
        {"X", 0}, {"Y", 1}, {"Z", 2},
        {"XX", 0}, {"YY", 1}, {"ZZ", 2}, {"XY", 3}, {"YZ", 4}, {"ZX", 5}
    };
    if (compMap.contains(compStr)) {
        componentIndex = compMap[compStr];
    }

    // =========================================================
    // 【核心修复】: 动态计算全局范围 (Global Range)
    // =========================================================
    double currentRange[2];
    // 获取"当前帧"该分量的范围
    dataArray->GetRange(currentRange, componentIndex);

    // 生成唯一键值 (例如 "Stress_0" 代表 Stress 的 X 分量)
    QString cacheKey = attributeName + "_" + QString::number(componentIndex);

    if (!m_globalDataRanges.contains(cacheKey)) {
        // 如果是第一次遇到这个属性，直接存入
        m_globalDataRanges[cacheKey] = qMakePair(currentRange[0], currentRange[1]);
    } else {
        // 如果之前存过，对比并更新“历史最大/最小值”
        auto& globalRange = m_globalDataRanges[cacheKey];
        if (currentRange[0] < globalRange.first) globalRange.first = currentRange[0];
        if (currentRange[1] > globalRange.second) globalRange.second = currentRange[1];

        // 强制使用全局范围，而不是当前帧范围
        currentRange[0] = globalRange.first;
        currentRange[1] = globalRange.second;
    }
    // =========================================================

    // 3. 配置 Mapper
    vtkMapper* currentMapper = this->actorPost->GetMapper();
    if (!currentMapper) return;

    currentMapper->ScalarVisibilityOn();
    currentMapper->SetScalarModeToUsePointFieldData();
    currentMapper->SelectColorArray(attributeName.toStdString().c_str());
    currentMapper->SetArrayComponent(componentIndex);

    // 应用全局范围，确保颜色刻度不跳动
    currentMapper->SetScalarRange(currentRange[0], currentRange[1]);
    currentMapper->SetLookupTable(this->colorLUT);

    if (auto dsm = vtkDataSetMapper::SafeDownCast(currentMapper)) {
        dsm->SetColorModeToMapScalars();
    }
    else if (auto gm = vtkPointGaussianMapper::SafeDownCast(currentMapper)) {
        gm->EmissiveOff();
    }

    // 4. 开启色卡 Widget
    this->scalarBarWidget->On();

    // 5. 更新自定义标题 (如果你还保留这个功能)
    // =========================================================
    // 【核心修复】: 将标题直接设置给 draggable widget
    // =========================================================

    // 1. 组合标题文字
    QString displayTitle = attributeName;
    if (componentIndex != -1 && compStr != "Scalar") {
        displayTitle += " (" + compStr + ")";
    }

    // 2. 将文字设置给 scalarBarWidget
    // 这样文字就会成为色卡的一部分，随鼠标拖动而移动
    vtkScalarBarActor* barActor = this->scalarBarWidget->GetScalarBarActor();
    barActor->SetTitle(displayTitle.toStdString().c_str());

    // 3. 开启 Widget
    this->scalarBarWidget->On();

    // 【重要】：原来的 customTitleActor 代码可以删除了
    // 因为独立的文字无法跟随 Widget 拖动
    if (this->customTitleActor) {
        this->customTitleActor->SetVisibility(false);
    }

    ui->widget_vtk_post->renderWindow()->Render();
}

void MainWindow::applyStandardView(double x, double y, double z, double vx, double vy, double vz)
{
    // 【修改点 1】 获取当前激活的 Renderer，而不是死板地用 this->renderer
    vtkRenderer* targetRen = getCurrentRenderer();
    if (!targetRen) return;

    vtkCamera* camera = targetRen->GetActiveCamera();

    // 1. 重置相机到模型中心
    targetRen->ResetCamera();

    // 2. 获取最佳距离
    double dist = camera->GetDistance();
    double* fp = camera->GetFocalPoint();

    // 3. 设置位置
    camera->SetPosition(fp[0] + x * dist, fp[1] + y * dist, fp[2] + z * dist);
    camera->SetViewUp(vx, vy, vz);

    // 4. 刷新渲染
    targetRen->ResetCameraClippingRange();

    // 【修改点 2】 刷新对应的窗口
    // 只有 RenderWindow 才能刷新，所以我们要通过 Renderer 找到它的 Window
    targetRen->GetRenderWindow()->Render();
}

// 前视图 (+X方向看往中心)
void MainWindow::on_toolButto_front_clicked()
{
    applyStandardView(1, 0, 0, 0, 0, 1);
}

// 后视图 (-X方向看往中心)
void MainWindow::on_toolButto_back_clicked()
{
    applyStandardView(-1, 0, 0, 0, 0, 1);
}

// 右视图 (+Y方向看往中心)
void MainWindow::on_toolButton_right_clicked()
{
    applyStandardView(0, 1, 0, 0, 0, 1);
}

// 左视图 (-Y方向看往中心)
void MainWindow::on_toolButto_left_clicked()
{
    applyStandardView(0, -1, 0, 0, 0, 1);
}

// 顶视图 (+Z方向看往中心)
void MainWindow::on_toolButto_top_clicked()
{
    applyStandardView(0, 0, 1, 0, 1, 0); // 此时 Y 轴为屏幕向上
}

// 底视图 (-Z方向看往中心)
void MainWindow::on_toolButto_bottom_clicked()
{
    applyStandardView(0, 0, -1, 0, 1, 0);
}


// ==========================================================================
//                           新增/修改的完整逻辑代码
// ==========================================================================

// 1. 右键菜单逻辑（包含所有根节点的添加入口）
void MainWindow::onTreeCustomContextMenu(const QPoint &pos)
{
    QModelIndex index = ui->treeView_left->indexAt(pos);
    m_currentRightClickIndex = index; // 保存当前点击位置

    QMenu menu(this);

    if (index.isValid()) {
        QStandardItem *item = m_model->itemFromIndex(index);
        QStandardItem *parent = item->parent();

        // ----------------------------------------------------
        // 情况 A：点击的是子项 (有父节点) -> 显示 Edit / Rename / Delete
        // ----------------------------------------------------
        if (parent) {
            QAction *actEdit = menu.addAction(QIcon(":/img/edit.png"), "Edit");
            connect(actEdit, &QAction::triggered, this, &MainWindow::onEditTreeItem);

            QAction *actRename = menu.addAction("Rename");
            connect(actRename, &QAction::triggered, this, &MainWindow::onRenameTreeItem);

            menu.addSeparator();

            QAction *actDel = menu.addAction(QIcon(":/img/close.png"), "Delete");
            connect(actDel, &QAction::triggered, this, &MainWindow::onDeleteTreeItem);
        }
        // ----------------------------------------------------
        // 情况 B：点击的是根节点 (无父节点) -> 显示 Add
        // ----------------------------------------------------
        else {
            QString rootName = item->text();

            // 为不同的根节点绑定不同的弹窗逻辑
            menu.addAction("Add to " + rootName, this, [=](){

            // 【核心修改 2】重置为新建模式
            m_isEditMode = false;
            m_editingItem = nullptr;

                if(rootName == "Laser") {
                    m_LaserDlg->show();
                }
                else if(rootName == "Material") {
                    m_material->show();
                }
                else if(rootName == "B-G Grid") {
                    m_bggridDlg->show();
                }
                else if(rootName == "Mat-Point") {
                    m_matPointdlg->show();
                }
                else if(rootName == "Init-conditions") {
                    m_initialDlg->show();
                }
                else if(rootName == "Load") {
                    m_load->show();
                }
                else if(rootName == "Solution") {
                    m_solution->show();
                }
                else if(rootName == "Boundary") {
                    m_boundaryDlg->show();
                }
                else if(rootName == "OutPut") {
                    m_output->show();
                }
                else if(rootName == "Load") {
                    m_load->setTargetNodeName(""); // 标记新建
                    m_load->show();
                }
                else if(rootName == "Init-conditions") {
                    m_initialDlg->setTargetNodeName("");
                    m_initialDlg->show();
                }
                else if(rootName == "OutPut") {
                    m_output->setTargetNodeName("");
                    m_output->show();
                }
                else if(rootName == "Solution") {
                    m_solution->setTargetNodeName("");
                    m_solution->show();
                }
                else if(rootName == "Body") {
                    // Body 目前没有对应的弹窗，可以在这里加日志或提示
                    qDebug() << "Add to Body clicked (No Dialog implemented)";
                }
                else if(rootName == "Component") {
                    qDebug() << "Add to Component clicked (No Dialog implemented)";
                }
            });
        }
    }
    menu.exec(ui->treeView_left->mapToGlobal(pos));
}

// 2. 接收 LaserDialog 信号，添加节点到树

// 3. 完整的编辑逻辑 (根据父节点类型打开对应窗口)
void MainWindow::onEditTreeItem()
{
    if (!m_currentRightClickIndex.isValid()) return;

    QStandardItem *item = m_model->itemFromIndex(m_currentRightClickIndex);
    QStandardItem *parent = item->parent();

    if (!parent) return; // 根节点不可编辑

    QString parentName = parent->text();

    // 【核心修改 1】进入编辑模式，记录目标
    m_isEditMode = true;
    m_editingItem = item; // 记住这个指针，等会儿改它名字

    if (parentName == "Load") {
        m_load->setTargetNodeName(item->text()); // 标记编辑 + 传名
        m_load->show();
    }
    else if (parentName == "Init-conditions") {
        m_initialDlg->setTargetNodeName(item->text());
        m_initialDlg->show();
    }
    else if (parentName == "OutPut") {
        m_output->setTargetNodeName(item->text());
        m_output->show();
    }
    else if (parentName == "Solution") {
        m_solution->setTargetNodeName(item->text());
        m_solution->show();
    }
    else if (parentName == "Laser") {
        m_LaserDlg->show();
    }
    else if (parentName == "Material") {
        m_material->show();
    }
    else if (parentName == "B-G Grid") {
        m_bggridDlg->show();
    }
    else if (parentName == "Mat-Point") {
        m_matPointdlg->show();
    }
    else if (parentName == "Init-conditions") {
        m_initialDlg->show();
    }
    else if (parentName == "Load") {
        m_load->show();
    }
    else if (parentName == "Boundary") {
        m_boundaryDlg->show();
    }
    else if (parentName == "Solution") {
        m_solution->show();
    }
    else if (parentName == "OutPut") {
        m_output->show();
    }
    else {
        qDebug() << "No edit dialog for parent:" << parentName;
    }
}

// 4. 完整的删除逻辑 (Laser 特殊处理 YAML，其他默认删除)
void MainWindow::onDeleteTreeItem()
{
    if (!m_currentRightClickIndex.isValid()) return;

    QStandardItem *item = m_model->itemFromIndex(m_currentRightClickIndex);
    QStandardItem *parent = item->parent();
    QString itemName = item->text();

    // 弹窗确认
    if (QMessageBox::question(this, "Confirm Delete",
                              "Are you sure you want to delete '" + itemName + "'?") != QMessageBox::Yes) {
        return;
    }

    // 打印日志方便调试
    if (parent) {
        qDebug() << "Deleting item from tree:" << itemName << " (Parent:" << parent->text() << ")";
    }

    // 纯粹的 UI 删除，不涉及任何文件操作
    m_model->removeRow(m_currentRightClickIndex.row(), m_currentRightClickIndex.parent());
    m_currentRightClickIndex = QModelIndex(); // 重置索引
}

// 5. 重命名逻辑
void MainWindow::onRenameTreeItem()
{
    if (!m_currentRightClickIndex.isValid()) return;
    QStandardItem *item = m_model->itemFromIndex(m_currentRightClickIndex);

    bool ok;
    QString newName = QInputDialog::getText(this, "Rename", "New Name:", QLineEdit::Normal, item->text(), &ok);

    if (ok && !newName.isEmpty()) {
        item->setText(newName);
    }
}


void MainWindow::on_listWidget_frames_itemClicked(QListWidgetItem *item)
{
    int index = ui->listWidget_frames->row(item);
    if (index < 0 || index >= this->fileList.count()) return;
    if (vtuWatcher->isRunning()) return;

    if (this->m_timer->isActive()) this->m_timer->stop();
    this->startLoadFrame(index);
}


void MainWindow::on_comboBox_rendering_currentTextChanged(const QString &text)
{
    if (!this->currentDataSet) return;

    if (text == "Point Gaussian") {
        setupPointGaussianMapper();
    } else {
        dsMapperPost->SetInputDataObject(this->currentDataSet);
        this->actorPost->SetMapper(dsMapperPost);

        if (text == "Surface") {
            this->actorPost->GetProperty()->SetRepresentationToSurface();
            this->actorPost->GetProperty()->SetEdgeVisibility(true);
        } else if (text == "Points") {
            this->actorPost->GetProperty()->SetRepresentationToPoints();
            this->actorPost->GetProperty()->SetPointSize(ui->lineEdit_PointSize->text().toFloat());
        } else if (text == "OutLine") {
            this->actorPost->GetProperty()->SetRepresentationToWireframe();
            this->actorPost->GetProperty()->SetEdgeVisibility(false);
        }
    }

    updateVtkColoring(ui->comboBox_attributes->currentText());
    ui->widget_vtk_post->renderWindow()->Render();
}


void MainWindow::setupPointGaussianMapper()
{
    if (!this->currentDataSet) return;

    QApplication::setOverrideCursor(Qt::WaitCursor);
    qDebug() << "Activating Point Gaussian Mapper...";

    try {
        // 1. 提取几何表面（UnstructuredGrid -> PolyData）
        // 因为 gaussianMapper 必须接收 PolyData
        auto surfaceFilter = vtkSmartPointer<vtkDataSetSurfaceFilter>::New();
        surfaceFilter->SetInputData(this->currentDataSet);
        surfaceFilter->Update();

        // 2. 更新成员变量 gaussianMapper 的数据
        this->gaussianMapperPost->SetInputData(surfaceFilter->GetOutput());
        this->actorPost->SetMapper(this->gaussianMapperPost);
        // 3. 自动计算或恢复半径
        // 如果输入框有值则用输入框的，否则重新计算
        double radius = ui->LineEdit_GaussianRadius->text().toFloat();
        if (radius <= 0) {
            double bounds[6];
            this->currentDataSet->GetBounds(bounds);
            double diag = sqrt(pow(bounds[1] - bounds[0], 2) +
                               pow(bounds[3] - bounds[2], 2) +
                               pow(bounds[5] - bounds[4], 2));
            radius = diag * 0.005;
            ui->LineEdit_GaussianRadius->setText(QString::number(radius, 'g', 4));
        }
        this->gaussianMapperPost->SetScaleFactor(radius);

        // 5. 同步颜色映射
        updateVtkColoring(ui->comboBox_attributes->currentText());

        qDebug() << "Point Gaussian activation complete.";

    } catch (...) {
        qDebug() << "Error occurred during Point Gaussian setup.";
    }

    QApplication::restoreOverrideCursor();
    ui->widget_vtk_post->renderWindow()->Render();
}

void MainWindow::on_LineEdit_GaussianRadius_editingFinished()
{
    // 只有在当前是高斯渲染模式时，修改半径才有意义
    if (ui->comboBox_rendering->currentText() == "Point Gaussian") {
        float radius = ui->LineEdit_GaussianRadius->text().toFloat();
        if (radius > 0) {
            this->gaussianMapperPost->SetScaleFactor(radius);
            ui->widget_vtk_post->renderWindow()->Render();
        }
    }
}


void MainWindow::on_lineEdit_PointSize_editingFinished()
{
    if (ui->comboBox_rendering->currentText() == "Points") {
        float size = ui->lineEdit_PointSize->text().toFloat();
        this->actorPost->GetProperty()->SetPointSize(size);
        ui->widget_vtk_post->renderWindow()->Render();
    }
}

void MainWindow::onJsonDataUpdate()
{
    qDebug() << "config_temp.json changed! Updating Pre-process view..." << endl;

    QProcess *process = new QProcess(this);
    // =========================================================
    // 【核心修改】设置 exe 运行的工作目录
    // =========================================================
    // 如果当前有工程目录，就去工程目录运行；否则还在 exe 目录运行（防止没打开工程时报错）
    QString workingDir = m_currentProjectDir.isEmpty()
                             ? QCoreApplication::applicationDirPath()
                             : m_currentProjectDir;

    process->setWorkingDirectory(workingDir);

    // 注意：exe 程序的路径必须是绝对路径，否则跑到工程目录里就找不到 exe 了
    QString program = QCoreApplication::applicationDirPath() + "/PreVisGenerator.exe";

    QStringList arguments;
    arguments << "--input" << "config_temp.json" << "--output" << "preview.vtu";

    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [=](int exitCode, QProcess::ExitStatus exitStatus){

                if (exitStatus == QProcess::NormalExit && exitCode == 0) {
                    // 1. 读取生成的预览文件
                    auto reader = vtkSmartPointer<vtkXMLUnstructuredGridReader>::New();
                    QString vtuPath = workingDir + "/preview.vtu";
                    reader->SetFileName(vtuPath.toStdString().c_str());
                    reader->Update();

                    // 2. 更新 Pre 专用 Mapper
                    // 使用 EXE 生成的 RGB 颜色 ("Colors" 数组)
                    this->mapperPre->SetInputConnection(reader->GetOutputPort());
                    this->mapperPre->SetScalarModeToUsePointFieldData();
                    this->mapperPre->SelectColorArray("Colors");
                    this->mapperPre->SetColorModeToDirectScalars();

                    // 3. 刷新 Pre 窗口
                    if (this->rendererPre) {
                        this->rendererPre->ResetCamera();
                        ui->widget_vtk_pre->renderWindow()->Render();
                    }

                    // 4. 自动切换到 Pre-process Tab (Index 0)
                    ui->tabWidget_center->setCurrentIndex(0);

                    qDebug() << "Pre-process view updated.";
                } else {
                    qDebug() << "PreVisGenerator failed. Code:" << exitCode;
                }
                process->deleteLater();
            });

    process->start(program, arguments);
}
void MainWindow::on_comboBox_fenliang_currentTextChanged(const QString &arg1)
{
    QString currentAttribute = ui->comboBox_attributes->currentText();

    // 避免在初始化或清空状态下触发
    if (!currentAttribute.isEmpty() && currentAttribute != "No Scalar Data Found") {
        updateVtkColoring(currentAttribute);
    }
}

QJsonObject serializeDialog(QWidget *dlg)
{
    if (!dlg) return QJsonObject();
    QJsonObject obj;
    // 处理所有 LineEdit (保留科学计数法字符串)
    for (QLineEdit *e : dlg->findChildren<QLineEdit *>()) {
        if (!e->objectName().isEmpty()) obj[e->objectName()] = e->text();
    }
    // 处理 ComboBox
    for (QComboBox *c : dlg->findChildren<QComboBox *>()) {
        if (!c->objectName().isEmpty()) obj[c->objectName()] = c->currentIndex();
    }
    // 处理 CheckBox
    for (QCheckBox *b : dlg->findChildren<QCheckBox *>()) {
        if (!b->objectName().isEmpty()) obj[b->objectName()] = b->isChecked();
    }
    return obj;
}
void deserializeDialog(QWidget *dlg, const QJsonObject &data)
{
    if (!dlg || data.isEmpty()) return;
    for (auto it = data.begin(); it != data.end(); ++it) {
        QString key = it.key();
        QLineEdit *e = dlg->findChild<QLineEdit *>(key);
        if (e) { e->setText(it.value().toString()); continue; }

        QComboBox *c = dlg->findChild<QComboBox *>(key);
        if (c) { c->setCurrentIndex(it.value().toInt()); continue; }

        QCheckBox *b = dlg->findChild<QCheckBox *>(key);
        if (b) { b->setChecked(it.value().toBool()); continue; }
    }
}
void MainWindow::saveToSimFile(const QString &fileName) {

}

void MainWindow::loadFromSimFile(const QString &fileName) {

}

