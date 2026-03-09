#ifndef FRAMELESSBASEDIALOG_H
#define FRAMELESSBASEDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QIcon>
#include <QGraphicsDropShadowEffect>

class FramelessBaseDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FramelessBaseDialog(QWidget *parent = nullptr);

    // 设置左上角图标
    void setWindowIconImage(const QIcon &icon);
    // 设置标题文字
    void setWindowTitleText(const QString &title);
    // 获取内容布局接口
    QVBoxLayout* contentLayout() const { return m_contentLayout; }

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void changeEvent(QEvent *event) override;
private:
    void initUi();

private:
    QWidget* m_mainContainer;    // 整体容器
    QWidget* m_titleBar;         // 标题栏
    QLabel* m_iconLabel;
    QLabel* m_titleLabel;
    QVBoxLayout* m_contentLayout; // 外部添加控件的布局

    bool m_isPressed = false;
    QPoint m_startMovePos;
    const int m_titleHeight = 30; // 稍微调窄一点更精致
    const int m_shadowMargin = 30;

private:
    void updateAppearance(bool isActive);
    QGraphicsDropShadowEffect* m_shadowEffect;
    bool m_isActive = false;
};

#endif
