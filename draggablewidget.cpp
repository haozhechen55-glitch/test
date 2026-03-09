#include "draggablewidget.h"
#include <QApplication>
#include <QScreen>

DraggableWidget::DraggableWidget(QWidget *parent)
    : QMainWindow(parent)
{
    this->setMinimumSize(200, 150);
    this->setMouseTracking(true); // 必须开启，用于实时改变光标样式
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowMinMaxButtonsHint);

    // 设置简单背景
    this->setStyleSheet("background-color: white; border: 1px solid #CCCCCC;");
}

// 计算鼠标在哪个边缘
DraggableWidget::Direction DraggableWidget::calculateDirection(const QPoint &cursorGlobalPoint)
{
    if (!isEnableResize || isMaximized()) return NONE;

    // 获取窗口在屏幕上的绝对几何范围
    QRect rect = this->frameGeometry();
    int x = cursorGlobalPoint.x();
    int y = cursorGlobalPoint.y();

    bool onLeft = (x >= rect.left() && x <= rect.left() + PADDING);
    bool onRight = (x <= rect.right() && x >= rect.right() - PADDING);
    bool onTop = (y >= rect.top() && y <= rect.top() + PADDING);
    bool onBottom = (y <= rect.bottom() && y >= rect.bottom() - PADDING);

    if (onTop && onLeft) return LEFTTOP;
    if (onBottom && onRight) return RIGHTBOTTOM;
    if (onBottom && onLeft) return LEFTBOTTOM;
    if (onTop && onRight) return RIGHTTOP;
    if (onTop) return UP;
    if (onBottom) return DOWN;
    if (onLeft) return LEFT;
    if (onRight) return RIGHT;

    return NONE;
}

void DraggableWidget::updateCursorShape(const QPoint &globalPos)
{
    Direction d = calculateDirection(globalPos);
    switch(d) {
    case LEFTTOP: case RIGHTBOTTOM: setCursor(Qt::SizeFDiagCursor); break;
    case RIGHTTOP: case LEFTBOTTOM: setCursor(Qt::SizeBDiagCursor); break;
    case UP: case DOWN: setCursor(Qt::SizeVerCursor); break;
    case LEFT: case RIGHT: setCursor(Qt::SizeHorCursor); break;
    default: setCursor(Qt::ArrowCursor); break;
    }
}

void DraggableWidget::handleMaximizeRestore()
{
    if (isMaximized()) {
        showNormal();
    } else {
        showMaximized();
    }
}

void DraggableWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && event->pos().y() <= TITLE_HEIGHT) {
        handleMaximizeRestore();
    }
}

void DraggableWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        QPoint globalPos = event->globalPos();
        dir = calculateDirection(globalPos);

        if (dir != NONE) {
            isLeftPressDown = true;
            mIsResizing = true;
        } else if (event->pos().y() <= TITLE_HEIGHT) {
            isLeftPressDown = true;
            mIsResizing = false;
            // 记录点击位置相对于窗口左上角的偏移
            dragPosition = event->pos();
        }
    }
    QMainWindow::mousePressEvent(event);
}

void DraggableWidget::mouseMoveEvent(QMouseEvent *event)
{
    QPoint globalPos = event->globalPos();

    // 未按下时更新光标样式
    if (!isLeftPressDown) {
        updateCursorShape(globalPos);
        QMainWindow::mouseMoveEvent(event);
        return;
    }

    // 情况 A: 正在调整大小
    if (mIsResizing && !isMaximized()) {
        QRect rect = this->geometry();
        switch(dir) {
        case LEFT: rect.setLeft(globalPos.x()); break;
        case RIGHT: rect.setRight(globalPos.x()); break;
        case UP: rect.setTop(globalPos.y()); break;
        case DOWN: rect.setBottom(globalPos.y()); break;
        case LEFTTOP: rect.setTopLeft(globalPos); break;
        case RIGHTTOP: rect.setTopRight(globalPos); break;
        case LEFTBOTTOM: rect.setBottomLeft(globalPos); break;
        case RIGHTBOTTOM: rect.setBottomRight(globalPos); break;
        default: break;
        }
        if (rect.width() >= minimumWidth() && rect.height() >= minimumHeight()) {
            this->setGeometry(rect);
        }
    }
    // 情况 B: 正在拖拽窗口
    else {
        if (isMaximized()) {
            // 【优化】最大化时拖动逻辑
            // 计算鼠标在标题栏的横向比例
            double factor = static_cast<double>(event->pos().x()) / this->width();
            showNormal();

            // 计算还原后，鼠标应在的位置，确保平滑过渡
            int newX = globalPos.x() - static_cast<int>(this->width() * factor);
            int newY = globalPos.y() - event->pos().y();
            this->move(newX, newY);

            // 重新更新 dragPosition 避免还原瞬间产生跳变
            dragPosition = QPoint(static_cast<int>(this->width() * factor), event->pos().y());
        } else {
            move(globalPos - dragPosition);
            emit posMoveSignal(this->x(), this->y());
        }
    }
}

void DraggableWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if (mIsResizing) emit resizeWinSignal();
        isLeftPressDown = false;
        mIsResizing = false;
        dir = NONE;
    }
    QMainWindow::mouseReleaseEvent(event);
}
