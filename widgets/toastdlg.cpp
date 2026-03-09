

#include "toastdlg.h"
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QEvent>
#include <QDebug>
#include <QGuiApplication>
#include <QScreen>
#include <QGraphicsDropShadowEffect>

       class ToastDlg : public QDialog {
private:
    QLabel* mLabel;
    QWidget* mContainer;
    QWidget* mTopBar; // 顶部彩色指示条

public:
    ToastDlg() {
        setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
        setAttribute(Qt::WA_TranslucentBackground, true);
        setAttribute(Qt::WA_ShowWithoutActivating, true);

        // 1. 外部布局：留出足够的阴影呼吸空间
        auto* rootLayout = new QVBoxLayout(this);
        rootLayout->setContentsMargins(20, 20, 20, 20);

        // 2. 主容器
        mContainer = new QWidget(this);
        mContainer->setObjectName("ToastContainer");
        auto* containerLayout = new QVBoxLayout(mContainer); // 改用垂直布局
        containerLayout->setContentsMargins(0, 0, 0, 0);
        containerLayout->setSpacing(0);

        // 3. 顶部彩色指示条（像对话框的窄标题栏）
        mTopBar = new QWidget(mContainer);
        mTopBar->setFixedHeight(4); // 4像素高度的精致顶部线条

        // 4. 文字区域
        mLabel = new QLabel(mContainer);
        mLabel->setAlignment(Qt::AlignCenter);

        containerLayout->addWidget(mTopBar);
        containerLayout->addWidget(mLabel);

        rootLayout->addWidget(mContainer);
        setLayout(rootLayout);

        // 5. 强化阴影：让它看起来更正式、更有悬浮在空中的质感
        QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(mContainer);
        shadow->setBlurRadius(25);          // 大范围模糊
        shadow->setColor(QColor(0, 0, 0, 55)); // 适中的透明度
        shadow->setOffset(0, 5);            // 纵向偏移，增加高度感
        mContainer->setGraphicsEffect(shadow);
    }

    void updateLevelStyle(Toast::Level level) {
        QString themeColor;
        switch (level) {
        case Toast::TWARN:  themeColor = "#F2994A"; break; // 柔和橙
        case Toast::TERROR: themeColor = "#EB5757"; break; // 柔和红
        case Toast::TINFO:
        default:            themeColor = "#2F80ED"; break; // 商务蓝
        }

        // 6. 容器样式：使用微灰背景，增加高级感
        mContainer->setStyleSheet(
            "QWidget#ToastContainer {"
            "  background-color: #FAFAFA;" // 非死板纯白，带一点点灰度更高级
            "  border: 1px solid #E0E0E0;" // 极细的浅灰框线
            "  border-radius: 4px;"
            "}"
            );

        // 顶部线条样式
        mTopBar->setStyleSheet(QString(
                                   "background-color: %1;"
                                   "border-top-left-radius: 4px;"
                                   "border-top-right-radius: 4px;"
                                   ).arg(themeColor));

        // 7. 文字样式：加大间距，使用更深的灰色
        mLabel->setStyleSheet(
            "QLabel {"
            "  color: #2D2D2D;" // 接近黑色但更柔和
            "  background: transparent;"
            "  font-family: 'Microsoft YaHei';"
            "  font-size: 14px;"
            "  font-weight: 500;"
            "  padding: 25px 40px;" // 宽阔的内边距，使其看起来很大气
            "}"
            );

        mContainer->setMinimumWidth(350); // 增加宽度，使其更像对话框
    }

    void showMessage(Toast::Level level, const QString& text, QWidget *parent) {
        mLabel->setText(text);
        updateLevelStyle(level);
        this->adjustSize();

        if (parent) {
            // 完美居中于传入的父对话框
            QPoint centerPos = parent->mapToGlobal(parent->rect().center());
            this->move(centerPos.x() - this->width() / 2, centerPos.y() - this->height() / 2);
        } else {
            QRect screenRect = QGuiApplication::primaryScreen()->geometry();
            this->move(screenRect.center() - this->rect().center());
        }
        QDialog::show();
    }
};

// --- 以下包装代码无需变动 ---
Toast::Toast() { mDlg = new ToastDlg; }
Toast &Toast::instance() { static Toast thiz; return thiz; }
void Toast::show(Toast::Level level, const QString &text) { mDlg->showMessage(level, text, nullptr); if (mTimerId != 0) killTimer(mTimerId); mTimerId = startTimer(2200); }
void Toast::show(Level level, const QString &text, QWidget *pt) { mDlg->showMessage(level, text, pt); if (mTimerId != 0) killTimer(mTimerId); mTimerId = startTimer(2200); }
void Toast::timerEvent(QTimerEvent *event) { Q_UNUSED(event); killTimer(mTimerId); mTimerId = 0; mDlg->accept(); }
