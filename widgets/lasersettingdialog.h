#ifndef LASERSETTINGDIALOG_H
#define LASERSETTINGDIALOG_H

#include <QDialog>
#include <QTableWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>  // 新增
#include "framelessbasedialog.h"

class LaserSettingDialog : public FramelessBaseDialog {
    Q_OBJECT

public:
    explicit LaserSettingDialog(QWidget *parent = nullptr);
    ~LaserSettingDialog();

    // 获取 YAML 格式的 laser 段内容（不含 "laser:" 前缀）
    QString getYamlSection() const;

    // 设置当前编辑的节点名称（空表示新建）
    void setTargetNodeName(const QString &name);

private slots:
    void onOkClicked();
    void onCancelClicked();
    void onBrowseClicked(); // 新增：浏览按钮槽函数

private:
    static const int ROW_COUNT = 15;
    static const int COL_COUNT = 6;

    QTableWidget *tableWidget;
    QLineEdit *pathEdit;
    QPushButton *browseButton; // 新增：浏览按钮
    QPushButton *okButton;
    QPushButton *cancelButton;

    QWidget *mainContainer;
    QVBoxLayout *containerLayout;
    QHBoxLayout *pathLayout;
    QHBoxLayout *buttonLayout;

    QString yamlSection; // 存储生成的 YAML 内容
    QString m_currentEditingNode; // 当前编辑的节点名称

    // 辅助函数：将一列数据转换为 "[a,b,c]" 格式
    QString columnToYamlList(int colIndex) const;
    // 新增：从文件加载数据
    void loadDataFromFile(const QString &fileName);

signals:
    void sigLasterName(QString name);
};

#endif // LASERSETTINGDIALOG_H
