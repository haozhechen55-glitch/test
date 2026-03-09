#ifndef TEMPERATUREVALUEDIALOG_H
#define TEMPERATUREVALUEDIALOG_H

#include <QDialog>
#include <QTableWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>

class TemperatureValueDialog : public QDialog {
    Q_OBJECT

public:
    explicit TemperatureValueDialog(QWidget *parent = nullptr);
    ~TemperatureValueDialog();

    // 获取最终的输出字符串
    QString getOutput() const;

private slots:
    void onOkClicked();
    void onCancelClicked();

private:
    QTableWidget *tableWidget;
    QPushButton *okButton;
    QPushButton *cancelButton;
    QVBoxLayout *mainLayout;
    QHBoxLayout *buttonLayout;

    QString outputString; // 存储最终输出

    // 辅助函数：检查是否为空或无效
    bool isValidCell(int row, int col);
};

#endif // TEMPERATUREVALUEDIALOG_H
