#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include"draggablewidget.h"
#include <QActionGroup>
#include <QStandardItemModel>
#include <QProcess>
#include"widgets/bggridsetting.h"
#include"widgets/matpointsetting.h"
#include"widgets/materialeditor.h"
#include"widgets/lasersettingdialog.h"
#include"widgets/initialtemperaturedialog.h"
#include"widgets/gravitydialog.h"
#include"widgets/boundarydlg.h"
#include"widgets/outputsettingdialog.h"
#include"widgets/solutiondlg.h"
#include"widgets/powderdlg.h"
// 引入必要的头文件                                                                             //hzChen  新增右键编辑导航树子项
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QInputDialog> // 用于重命名
#include <QEvent> // <---引入事件头文件

// 多线程组件
#include <QtConcurrent/QtConcurrent>
#include <QFutureWatcher>

#include <QVTKOpenGLNativeWidget.h>
#include <QPointer>

// VTK 核心头文件
#include <vtkSmartPointer.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkDataSetMapper.h>
#include <vtkActor.h>
#include <vtkXMLUnstructuredGridReader.h>
#include <vtkPointData.h>
#include <vtkOutputWindow.h>
#include <vtkDataSet.h> //用于异步传递数据
#include<vtkOrientationMarkerWidget.h>
#include<vtkScalarBarActor.h>
#include <vtkScalarBarWidget.h> // <--- 【新增】引入 Widget 头文件
#include <vtkProperty.h>
#include <vtkPointGaussianMapper.h>
#include <vtkGeometryFilter.h> // 用于将 UnstructuredGrid 转为 PolyData
#include<QListWidgetItem>

#include <vtkGenericDataObjectReader.h>
#include <vtkGlyph3D.h>
#include <vtkSphereSource.h>
#include <vtkPolyData.h>


// 前向声明 VTK 类
class vtkDataObject;
class vtkXMLUnstructuredGridReader;

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public DraggableWidget
{
    Q_OBJECT

public:
    MainWindow(DraggableWidget *parent = nullptr);
    ~MainWindow();
public:
    void initTree();
    void addSubItems(const QString &parentName, const QStringList &subItems);
    bool writeDefaultYml();
    void saveToSimFile(const QString &fileName);
    void loadFromSimFile(const QString &fileName);
    QJsonObject serializeDialog(QWidget *dlg);
    void deserializeDialog(QWidget *dlg, const QJsonObject &data);
    void loadProjectLogic(const QString &simFilePath);
    void processVtkSequence(const QString &selectedFile);
protected:
    //重写改变事件，用于监听窗口状态变化
    void changeEvent(QEvent *event) override;
    // 【新增】重写关闭事件，拦截退出操作
    void closeEvent(QCloseEvent *event) override;

    void applyStandardView(double x, double y, double z, double vx, double vy, double vz);
    void updateAttributeComboBoxSmart();
    void updateComponentComboBox();
    void setupPointGaussianMapper();

private slots:
    void on_toolButton_min_clicked();
    void on_toolButton_max_clicked();
    void on_toolButton_close_clicked();
    void on_toolButton_B_G_grid_clicked();
    void on_toolButton_MatPoint_clicked();
    void on_toolButton_open_5_clicked();
    void on_toolButton_open_Laser_clicked();
    void on_toolButton_Global_clicked();
    void on_toolButton_Inital_clicked();
    void on_toolButton_Load_clicked();
    void on_toolButton_open_Boundary_clicked();
    void on_toolButton_output_clicked();
    void on_toolButton_solution_clicked();
    void on_toolButton_run_clicked();
    void on_toolButton_powder_clicked();

    void on_toolButton_open_clicked();
    void on_toolButton_last_clicked();
    void on_toolButton_next_clicked();
    void on_toolButton_play_clicked();
    void on_comboBox_attributes_currentTextChanged(const QString &arg1);

    void playNextFrame();

    // 异步加载完成后的槽函数
    void onVtuLoadFinished();
    // 右键菜单槽函数                                                                                      //hzchen
    void onTreeCustomContextMenu(const QPoint &pos);

    // 具体动作槽函数
    void onEditTreeItem();
    void onDeleteTreeItem();
    void onRenameTreeItem();

    void on_toolButto_left_clicked();

    void on_toolButton_right_clicked();

    void on_toolButto_front_clicked();

    void on_toolButto_back_clicked();

    void on_toolButto_top_clicked();

    void on_toolButto_bottom_clicked();

    void on_listWidget_frames_itemClicked(QListWidgetItem *item);

    void on_comboBox_rendering_currentTextChanged(const QString &arg1);

    void on_LineEdit_GaussianRadius_editingFinished();

    void on_lineEdit_PointSize_editingFinished();


    void on_comboBox_fenliang_currentTextChanged(const QString &arg1);

    void on_toolButton_new_clicked();

    void on_toolButton_save_clicked();

public slots:
    void onAddMatPoint(QString name);
    void onAddMaterial(QString name);
    void onAddBGgrid(QString name);
    void onAddBoundary(QString name);
    // 接收 Laser 对话框信号
    void onAddLaserItem(QString name);
    //config_temp.json
    void onJsonDataUpdate();
    void onAddGravity(QString name);
    void onAddInitTemp(QString name);
    void onAddOutput(QString name);
    void onAddSolution(QString name);void onAddPowder(QString name);


private:
    Ui::MainWindow *ui;

    QStandardItemModel * m_model;
    QMap<QString, QStandardItem*> m_rootItemsMap;

    BGgridSetting * m_bggridDlg = nullptr;
    MatPointSetting * m_matPointdlg=nullptr;
    MaterialEditor * m_material=nullptr;
    LaserSettingDialog * m_LaserDlg = nullptr;
    InitialTemperatureDialog * m_initialDlg=nullptr;
    GravityDialog * m_load=nullptr;
    BoundaryDlg * m_boundaryDlg=nullptr;
    OutputSettingDialog * m_output = nullptr;
    SolutionDlg * m_solution=nullptr;
    PowderDlg *m_powderDlg = nullptr;

    QProcess *m_process = nullptr;

    QString m_currentProjectDir="";

    // mainwindow.h 中的 private 部分添加：
    QMap<QString, QAction*> m_actionMap; // 存储 名字 -> 动作 的映射
    // 记录当前右键选中的节点索引
    QModelIndex m_currentRightClickIndex;
    // 【新增】用于标记当前的操作模式
    bool m_isEditMode = false;
    // 【新增】用于记录当前正在编辑的树节点指针
    QStandardItem* m_editingItem = nullptr;

    void generateFullYaml();



private:
    // ==========================================
    // VTK 核心组件 - 双 Tab 架构
    // ==========================================

    // --- 1. Pre-process (前处理) 专用组件 ---
    // 用于显示 config.json -> EXE -> preview.vtu 的静态预览
    vtkSmartPointer<vtkRenderer> rendererPre;
    vtkSmartPointer<vtkActor> actorPre;
    vtkSmartPointer<vtkDataSetMapper> mapperPre;
    vtkSmartPointer<vtkOrientationMarkerWidget> axesWidgetPre; // 前处理坐标轴

    // --- 2. Post-process (后处理) 专用组件 ---
    // 用于显示 case.block.xxxx.vtu 序列回放
    vtkSmartPointer<vtkRenderer> rendererPost;
    vtkSmartPointer<vtkActor> actorPost;

    // 后处理复杂的 Mapper (支持高斯点 / 普通网格切换)
    vtkSmartPointer<vtkDataSetMapper> dsMapperPost;
    vtkSmartPointer<vtkPointGaussianMapper> gaussianMapperPost;

    // 后处理专用挂件
    vtkSmartPointer<vtkOrientationMarkerWidget> axesWidgetPost; // 后处理坐标轴
    vtkSmartPointer<vtkScalarBarWidget> scalarBarWidget;        // 色卡 (仅后处理需要)
    vtkSmartPointer<vtkLookupTable> colorLUT;                   // 颜色表 (Post专用)

    // --- 通用/数据缓存 ---
    // 用于存储后处理的全局范围 (Key: "属性名_分量", Value: {min, max})
    QMap<QString, QPair<double, double>> m_globalDataRanges;

    // 当前后处理正在显示的数据集 (用于切换 Surface/Points 模式时引用)
    vtkSmartPointer<vtkDataSet> currentDataSet;

    // Reader 仅作为局部工具或初始化占位
    vtkSmartPointer<vtkXMLUnstructuredGridReader> reader;

    // 异步加载任务监控器
    QFutureWatcher<vtkSmartPointer<vtkDataSet>>* vtuWatcher;

    vtkSmartPointer<vtkTextActor> customTitleActor; //色卡

    // 启动异步加载任务
    void startLoadFrame(int index);
    void setupVtkPipeline();

    void updateVtkColoring(const QString& attributeName);
    QList<QString> fileList;
    int currentFrameIndex = -1;
    QTimer *m_timer;

    bool vtkPipelineInitialized = false;
    bool isFirstLoadOfSequence = true; // 标志位：是否是新打开的序列
    // 【新增】辅助函数：获取当前激活 Tab 的 Renderer (用于视图按钮)
    vtkRenderer* getCurrentRenderer();
};
#endif // MAINWINDOW_H
