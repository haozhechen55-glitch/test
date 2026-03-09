#ifndef TOASTDLG_H
#define TOASTDLG_H

#include <QObject>
#include <QRect>
class ToastDlg;

class Toast: public QObject
{
public:
    enum Level
    {
        TINFO, TWARN, TERROR
    };
private:
    Toast();
public:
    static Toast& instance();
public:
    void show(Level level, const QString& text);

    void show(Level level, const QString& text, QWidget *pt);

private:
    void timerEvent(QTimerEvent *event) override;

private:
    ToastDlg* mDlg;
    int mTimerId{0};
    QRect mGeometry;
};

#endif // TOASTDLG_H

