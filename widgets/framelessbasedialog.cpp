#include "framelessbasedialog.h"

FramelessBaseDialog::FramelessBaseDialog(QWidget *parent) : QDialog(parent)
{
    // 设置无边框和透明背景
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::Dialog);
    setAttribute(Qt::WA_TranslucentBackground);

    initUi();


    // 样式表优化：
    this->setStyleSheet(
        // 主容器：1px 纯黑或深灰色边框
        "QWidget#MainContainer { "
        "   background-color: #F0F0F0; "
        "   border: 1px solid #000000; "
        "}"
        // 标题栏：纯白色背景，下方有一条浅灰色分割线
        "QWidget#TitleBar { "
        "   background-color: #FFFFFF; "
        "   border-bottom: 1px solid #E0E0E0; "
        "}"
        // 标题文字：标准 Windows 黑色字体
        "QLabel#TitleLabel { "
        "   color: #000000; "
        "   font-family: 'Microsoft YaHei', 'Segoe UI'; "
        "   font-size: 12px; "
        "   font-weight: normal; "
        "}"
        );

    // 初始状态为非激活
    updateAppearance(false);
}

void FramelessBaseDialog::initUi()
{
    QVBoxLayout* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(m_shadowMargin, m_shadowMargin, m_shadowMargin, m_shadowMargin);

    m_mainContainer = new QWidget(this);
    m_mainContainer->setObjectName("MainContainer");
    rootLayout->addWidget(m_mainContainer);

    // 创建阴影效果（先不设置参数）
    m_shadowEffect = new QGraphicsDropShadowEffect(this);
    m_mainContainer->setGraphicsEffect(m_shadowEffect);

    QVBoxLayout* containerLayout = new QVBoxLayout(m_mainContainer);
    containerLayout->setContentsMargins(0, 0, 0, 0);
    containerLayout->setSpacing(0);

    // 标题栏
    m_titleBar = new QWidget(m_mainContainer);
    m_titleBar->setObjectName("TitleBar");
    m_titleBar->setFixedHeight(m_titleHeight);

    QHBoxLayout* titleLayout = new QHBoxLayout(m_titleBar);
    titleLayout->setContentsMargins(8, 0, 8, 0);
    titleLayout->setSpacing(6);

    m_iconLabel = new QLabel(m_titleBar);
    m_iconLabel->setFixedSize(16, 16);
    m_iconLabel->setScaledContents(true);
    m_iconLabel->setVisible(false);

    m_titleLabel = new QLabel(m_titleBar);
    m_titleLabel->setObjectName("TitleLabel");

    titleLayout->addWidget(m_iconLabel);
    titleLayout->addWidget(m_titleLabel);
    titleLayout->addStretch();

    // 内容区
    QWidget* contentArea = new QWidget(m_mainContainer);
    m_contentLayout = new QVBoxLayout(contentArea);
    m_contentLayout->setContentsMargins(15, 15, 15, 15);
    m_contentLayout->setSpacing(10);

    containerLayout->addWidget(m_titleBar);
    containerLayout->addWidget(contentArea);
}
void FramelessBaseDialog::setWindowIconImage(const QIcon &icon) {
    if (!icon.isNull()) {
        m_iconLabel->setPixmap(icon.pixmap(16, 16));
        m_iconLabel->setVisible(true);
    }
}

void FramelessBaseDialog::setWindowTitleText(const QString &title) {
    m_titleLabel->setText(title);
}

// --- 鼠标拖动实现 ---
void FramelessBaseDialog::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        QPoint posInContainer = m_mainContainer->mapFromParent(event->pos());
        if (m_titleBar->geometry().contains(posInContainer)) {
            m_isPressed = true;
            m_startMovePos = event->globalPos() - this->frameGeometry().topLeft();
            event->accept();
        }
    }
}

void FramelessBaseDialog::mouseMoveEvent(QMouseEvent *event) {
    if (m_isPressed) {
        move(event->globalPos() - m_startMovePos);
        event->accept();
    }
}

void FramelessBaseDialog::mouseReleaseEvent(QMouseEvent *event) {
    m_isPressed = false;
}

void FramelessBaseDialog::updateAppearance(bool isActive)
{
    m_isActive = isActive;

    // 设置边框颜色和阴影
    QString borderColor = isActive ? "#000000" : "#A0A0A0";  // 黑 or 灰
    int blurRadius = isActive ? 25 : 10;
    int offset = isActive ? 4 : 2;
    int alpha = isActive ? 150 : 60;

    // 更新阴影
    m_shadowEffect->setBlurRadius(blurRadius);
    m_shadowEffect->setOffset(0, offset);
    m_shadowEffect->setColor(QColor(0, 0, 0, alpha));

    // 更新样式表（仅更新主容器边框）
    m_mainContainer->setStyleSheet(QString(
                                       "QWidget#MainContainer { "
                                       "   background-color: #F0F0F0; "
                                       "   border: 1px solid %1; "
                                       "}"
                                       "QWidget#TitleBar { "
                                       "   background-color: #FFFFFF; "
                                       "   border-top: 1px solid %1; "
                                       "   border-left: 1px solid %1; "
                                       "   border-right: 1px solid %1; "
                                       "   border-bottom: none; "
                                       "}"
                                       "QLabel#TitleLabel { "
                                       "   color: #000000; "
                                       "   font-family: 'Microsoft YaHei', 'Segoe UI'; "
                                       "   font-size: 12px; "
                                       "   font-weight: normal; "
                                       "}"
                                       ).arg(borderColor));
}


void FramelessBaseDialog::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::ActivationChange) {
        updateAppearance(isActiveWindow());
    }
    QDialog::changeEvent(event);
}
