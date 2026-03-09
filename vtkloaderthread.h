// VtkLoaderThread.h
#ifndef VTKLOADER_THREAD_H
#define VTKLOADER_THREAD_H

#include <QThread>
#include <QString>
#include <QList>

class VtkLoaderThread : public QThread
{
    Q_OBJECT

public:
    explicit VtkLoaderThread(QObject *parent = nullptr);
    void setDirectory(const QString &dirPath);

signals:
    void fileLoadingProgress(int percent);          // 加载进度 [0~100]
    void fileListReady(const QList<QString> &files); // 文件列表准备就绪
    void loadingFinished();                         // 全部加载完成
    void loadingError(const QString &error);        // 错误提示

protected:
    void run() override;

private:
    QString m_dirPath;
    bool m_cancel = false;
};

#endif // VTKLOADER_THREAD_H
