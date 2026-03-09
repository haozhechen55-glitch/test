QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17
# 针对 MSVC 编译器的编码设置
win32-msvc* {
    QMAKE_CXXFLAGS += /source-charset:utf-8 /execution-charset:utf-8
}


SOURCES += \
    draggablewidget.cpp \
    main.cpp \
    mainwindow.cpp \
    vtkloaderthread.cpp \
    widgets/bggridsetting.cpp \
    widgets/boundarydlg.cpp \
    widgets/framelessbasedialog.cpp \
    widgets/gravitydialog.cpp \
    widgets/initialtemperaturedialog.cpp \
    widgets/lasersettingdialog.cpp \
    widgets/materialeditor.cpp \
    widgets/matpointsetting.cpp \
    widgets/outputsettingdialog.cpp \
    widgets/powderdlg.cpp \
    widgets/solutiondlg.cpp \
    widgets/temperaturevaluedialog.cpp \
    widgets/toastdlg.cpp

HEADERS += \
    draggablewidget.h \
    mainwindow.h \
    vtkloaderthread.h \
    widgets/bggridsetting.h \
    widgets/boundarydlg.h \
    widgets/framelessbasedialog.h \
    widgets/gravitydialog.h \
    widgets/initialtemperaturedialog.h \
    widgets/lasersettingdialog.h \
    widgets/materialeditor.h \
    widgets/matpointsetting.h \
    widgets/outputsettingdialog.h \
    widgets/powderdlg.h \
    widgets/solutiondlg.h \
    widgets/temperaturevaluedialog.h \
    widgets/toastdlg.h

FORMS += \
    mainwindow.ui \
    widgets/bggridsetting.ui \
    widgets/boundarydlg.ui \
    widgets/gravitydialog.ui \
    widgets/initialtemperaturedialog.ui \
    widgets/materialeditor.ui \
    widgets/matpointsetting.ui \
    widgets/outputsettingdialog.ui \
    widgets/powderdlg.ui \
    widgets/solutiondlg.ui



# === VTK 配置 ===
# 根据构建模式自动选择 VTK 路径
CONFIG(debug, debug|release) {
    VTK_BUILD_DIR = $$PWD/vtkLib/debug
    VTK_SUFFIX = d
    message("VTK: Using Debug libraries from $$VTK_BUILD_DIR")
} else {
    VTK_BUILD_DIR = $$PWD/vtkLib/release
    VTK_SUFFIX =
    message("VTK: Using Release libraries from $$VTK_BUILD_DIR")
}

INCLUDEPATH += $$VTK_BUILD_DIR/include/vtk-9.5
LIBS        += -L$$VTK_BUILD_DIR/lib
# === VTK 库列表 ===
# 1. 定义所需的所有模块名（不带版本号和Debug/Release后缀）
VTK_MODULES = \
    vtksys \
    vtkCommonCore \
    vtkCommonDataModel \
    vtkCommonExecutionModel \
    vtkCommonMisc \
    vtkCommonTransforms \
    vtkIOCore \
    vtkIOXML \
    vtkFiltersCore \
    vtkFiltersGeometry \
    vtkRenderingCore \
    vtkRenderingOpenGL2 \
    vtkInteractionStyle \
    vtkGUISupportQt \
    vtkRenderingQt \# 关键：提供 QVTKOpenGLNativeWidget 实现
    vtkInteractionWidgets \
    vtkRenderingAnnotation \
    vtkCommonComputationalGeometry \
    vtkIOLegacy \
    vtkFiltersSources \
    vtkFiltersCore \
    vtkIOCore \
    vtkCommonDataModel \
    vtkCommonExecutionModel

# 3. 循环链接所有模块
for(MODULE, VTK_MODULES) {
    # 格式化库名：-lvtkModuleName-9.5[d]
    LIBS += -l$$MODULE-9.5$$VTK_SUFFIX
}


# OpenGL 依赖（Windows 下显式链接）
win32 {
    LIBS += -lopengl32
}


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    image.qrc

DISTFILES +=

win32: RC_ICONS = logo.ico
