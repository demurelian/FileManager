#ifndef WORKER_H
#define WORKER_H

#include <QObject>
#include <QDateTime>
#include <QFileSystemModel>
#include <QDir>
#include <QMessageBox>

class Worker : public QObject
{
    Q_OBJECT
public:
    explicit Worker(QObject *parent = nullptr);
    void changeCurrentPath(QString path);
    void changeOperationPath(QString path);
    QString getCurrentPath();
    QString getOperationPath();

public slots:
    void copy(QString targetPath);
    void del();
    void move(QString targetPath);
    void rename(QString name);
    void properties();
    void makeDir(QString name);
    void delAll();
    void runBackup(QString srcPath, QString bcPath);

signals:
    void backupFinished();
    void statusChanged(int val);
    void successStatusGetted(const int& val);
    void workloadIsEmpty();
    void delAll_finished();
    void criticalOperationError(QString errorString);
    void properties_finished(QString information);
    void copy_finished();
    void del_finished();
    void move_finished();

private:
    void contentDifference(QDir &sDir, QDir &bDir, QFileInfoList &diffList);
    void recursiveContentList(QDir &dir, QFileInfoList &contentList);
    qint64 dirSize(QDir &dir, qint64 &size);

private:
    QString currentPath, operationPath;
    int status;
    bool moveOperation;
};

#endif // WORKER_H
