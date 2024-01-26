#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QFileSystemModel>
#include <QDir>
#include <QDateTime>
#include <QThread>
#include <QMessageBox>

#include "progresswindow.h"
#include "namewindow.h"
#include "worker.h"

namespace Ui {
class MainWindow;
}


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_sourceDirView_doubleClicked(const QModelIndex &index);
    void on_backupDirView_doubleClicked(const QModelIndex &index);
    void on_sourceDirView_clicked(const QModelIndex &index);
    void on_backupDirView_clicked(const QModelIndex &index);

    void on_srcGoToRootButton_clicked();
    void on_bcGoToRootButton_clicked();

    void on_srcGoBackButton_clicked();
    void on_bcGoBackButton_clicked();

    void on_srcCopyButton_clicked();
    void on_bcCopyButton_clicked();

    void on_srcDeleteButton_clicked();
    void on_bcDeleteButton_clicked();

    void on_srcMoveButton_clicked();
    void on_bcMoveButton_clicked();

    void on_srcRenameButton_clicked();
    void on_bcRenameButton_clicked();

    void on_srcMkDirButton_clicked();
    void on_bcMkDirButton_clicked();

    void on_srcDeleteAllButton_clicked();
    void on_bcDeleteAllButton_clicked();

    void on_copyAllButton_clicked();

    void copyAllReadyToStart();
    void on_prop_finished(QString information);
    void on_copy_finished();
    void on_delete_finished();
    void on_move_finished();
    void on_deleteAll_finished();

    void onCriticalError(QString errorString);

signals:
    void startBackupOperation(QString srcPath, QString bcPath);
    void startSrcCopy(QString targetPath);
    void startBcCopy(QString targetPath);
    void startSrcDelete();
    void startBcDelete();
    void startSrcMove(QString targetPath);
    void startBcMove(QString targetPath);
    void startSrcDelAll();
    void startBcDelAll();

private:
    void dirView(const QModelIndex &index, QString &currentPath);
    void disableAllButtons();
    void disableSrcButtons();
    void enableSrcButtons();
    void disableBcButtons();
    void enableBcButtons();


private:
    Ui::MainWindow *ui;
    QFileSystemModel *model;
    QString currentSrcPath, currentBcPath, srcOperationPath, bcOperationPath, errorString;
    bool srcOperationInProgress, bcOperationInProgress;
    Worker *cmnWorker, *bcWorker, *srcWorker;
    QThread *cmnThread;
    QThread *srcThread;
    QThread *bcThread;
    ProgressWindow *progressWindow;
    NameWindow *nameWindow;
};


#endif // MAINWINDOW_H
