// VtkLoaderThread.cpp
#include "VtkLoaderThread.h"
#include <QDir>
#include <QFileInfo>
#include <QDebug>
#include <QRegularExpression>
#include <algorithm>

VtkLoaderThread::VtkLoaderThread(QObject *parent)
    : QThread(parent)
{
}

void VtkLoaderThread::setDirectory(const QString &dirPath)
{
    m_dirPath = dirPath;
}

void VtkLoaderThread::run()
{
    if (m_dirPath.isEmpty()) {
        emit loadingError("Directory path is empty.");
        return;
    }

    QDir dir(m_dirPath);
    QStringList filters;
    filters << "case.block.*.vtu";

    QFileInfoList fileInfoList = dir.entryInfoList(filters, QDir::Files, QDir::Name);
    if (fileInfoList.isEmpty()) {
        emit loadingError("No 'case.block.*.vtu' files found.");
        return;
    }

    // 提取文件路径
    QList<QString> filePaths;
    for (const QFileInfo &info : fileInfoList) {
        filePaths.append(info.absoluteFilePath());
    }

    // 按数字部分自然排序（解决 case.block.10.vtu 排在 2 前的问题）
    std::sort(filePaths.begin(), filePaths.end(), [](const QString &a, const QString &b) {
        QRegularExpression re(R"(case\.block\.(\d+)\.vtu)");
        auto matchA = re.match(a);
        auto matchB = re.match(b);
        if (matchA.hasMatch() && matchB.hasMatch()) {
            int numA = matchA.captured(1).toInt();
            int numB = matchB.captured(1).toInt();
            return numA < numB;
        }
        return a < b; // fallback
    });

    int total = filePaths.size();
    for (int i = 0; i < total; ++i) {
        if (m_cancel) break;

        // 模拟“预读”以校验文件（可选，也可只传路径）
        // 这里我们只收集路径，真正的 VTK 读取仍在主线程
        // 所以进度只是“扫描进度”

        int percent = (i + 1) * 100 / total;
        emit fileLoadingProgress(percent);
        msleep(5); // 避免信号堆积过快
    }

    emit fileListReady(filePaths);
    emit loadingFinished();
}

// 可选：支持取消（你可添加 cancel() 方法）
