#ifndef DRAGGABLEWIDGET_H
#define DRAGGABLEWIDGET_H

#include <QMainWindow>
#include <QMouseEvent>
#include <QRect>

#define PADDING 6
#define TITLE_HEIGHT 30

class DraggableWidget : public QMainWindow
{
    Q_OBJECT

    enum Direction { UP=0, DOWN, LEFT, RIGHT, LEFTTOP, LEFTBOTTOM, RIGHTBOTTOM, RIGHTTOP, NONE };

public:
    explicit DraggableWidget(QWidget *parent = nullptr);
    void setEnableResize(bool resize) { isEnableResize = resize; }

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override; // 新增：处理双击

private:
    void updateCursorShape(const QPoint &globalPos);
    Direction calculateDirection(const QPoint &globalPos);
    void handleMaximizeRestore(); // 封装最大化切换逻辑

private:
    bool isLeftPressDown = false;
    bool isEnableResize = true;
    bool mIsResizing = false;

    QPoint dragPosition;   // 记录鼠标相对窗口左上角的偏移
    Direction dir = NONE;

signals:
    void posMoveSignal(int x, int y);
    void resizeWinSignal();
};

#endif
