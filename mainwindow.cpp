#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    bcOperationInProgress = false;
    srcOperationInProgress = false;
    currentBcPath = currentSrcPath = srcOperationPath = bcOperationPath = "";
    errorString = "";

    model = new QFileSystemModel(this);
    model->setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);
    model->setRootPath("");
    ui->sourceDirView->setModel(model);
    ui->backupDirView->setModel(model);

    cmnWorker = new Worker();
    cmnThread = new QThread(this);    //объект не должен иметь родителя для отправки в поток
    cmnWorker->moveToThread(cmnThread);
    cmnThread->start();
    connect(this, SIGNAL(destroyed()), cmnThread, SLOT(quit()));
    connect(this, SIGNAL(startBackupOperation(QString, QString)), cmnWorker, SLOT(runBackup(QString, QString)));
    connect(cmnWorker, SIGNAL(backupFinished()), this, SLOT(copyAllReadyToStart()));

    srcWorker = new Worker();
    bcWorker = new Worker();
    srcThread = new QThread(this);
    bcThread = new QThread(this);
    connect(this, SIGNAL(destroyed()), srcThread, SLOT(quit()));
    connect (this, SIGNAL(destroyed()), bcThread, SLOT(quit()));
    srcWorker->moveToThread(srcThread);
    srcThread->start();
    bcWorker->moveToThread(bcThread);
    bcThread->start();

    connect(srcWorker, SIGNAL(criticalOperationError(QString)), this, SLOT(onCriticalError(QString)));
    connect(bcWorker, SIGNAL(criticalOperationError(QString)), this, SLOT(onCriticalError(QString)));

    connect(this, SIGNAL(startSrcDelAll()), srcWorker, SLOT(delAll()));
    connect(this, SIGNAL(startBcDelAll()), bcWorker, SLOT(delAll()));
    connect(srcWorker, SIGNAL(delAll_finished()), this, SLOT(on_deleteAll_finished()));
    connect(bcWorker, SIGNAL(delAll_finished()), this, SLOT(on_deleteAll_finished()));

    connect(ui->srcPropButton, SIGNAL(clicked()), srcWorker, SLOT(properties()));
    connect(ui->bcPropButton, SIGNAL(clicked()), bcWorker, SLOT(properties()));
    connect(bcWorker, SIGNAL(properties_finished(QString)), this, SLOT(on_prop_finished(QString)));
    connect(srcWorker, SIGNAL(properties_finished(QString)), this, SLOT(on_prop_finished(QString)));

    connect(this, SIGNAL(startSrcCopy(QString)), srcWorker, SLOT(copy(QString)));
    connect(this, SIGNAL(startBcCopy(QString)), bcWorker, SLOT(copy(QString)));
    connect(srcWorker, SIGNAL(copy_finished()), this, SLOT(on_copy_finished()));
    connect(bcWorker, SIGNAL(copy_finished()), this, SLOT(on_copy_finished()));

    connect(this, SIGNAL(startSrcDelete()), srcWorker, SLOT(del()));
    connect(this, SIGNAL(startBcDelete()), bcWorker, SLOT(del()));
    connect(srcWorker, SIGNAL(del_finished()), this, SLOT(on_delete_finished()));
    connect(bcWorker, SIGNAL(del_finished()), this, SLOT(on_delete_finished()));

    connect(this, SIGNAL(startSrcMove(QString)), srcWorker, SLOT(move(QString)));
    connect(this, SIGNAL(startBcMove(QString)), bcWorker, SLOT(move(QString)));
    connect(srcWorker, SIGNAL(move_finished()), this, SLOT(on_move_finished()));
    connect(bcWorker, SIGNAL(move_finished()), this, SLOT(on_move_finished()));

    disableAllButtons();

    setWindowTitle("Файловый Менеджер");

    ui->srcPathView->setReadOnly(true);
    ui->bcPathView->setReadOnly(true);

    ui->srcDeleteButton->setIcon(QIcon("delete.png"));
    ui->srcDeleteButton->setToolTip("Удалить файл");
    ui->srcDeleteButton->setIconSize({20,20});
    ui->srcDeleteButton->setFixedSize({20, 20});
    ui->srcDeleteButton->setStyleSheet("QPushButton{background: transparent;}");

    ui->bcDeleteButton->setIcon(QIcon("delete.png"));
    ui->bcDeleteButton->setToolTip("Удалить файл");
    ui->bcDeleteButton->setIconSize({20,20});
    ui->bcDeleteButton->setFixedSize({20, 20});
    ui->bcDeleteButton->setStyleSheet("QPushButton{background: transparent;}");

    ui->srcCopyButton->setIcon(QIcon("copy.png"));
    ui->srcCopyButton->setToolTip("Копировать файл");
    ui->srcCopyButton->setIconSize({20, 20});
    ui->srcCopyButton->setFixedSize({20, 20});
    ui->srcCopyButton->setStyleSheet("QPushButton{background: transparent;}");

    ui->bcCopyButton->setIcon(QIcon("copy.png"));
    ui->bcCopyButton->setToolTip("Копировать файл");
    ui->bcCopyButton->setIconSize({20, 20});
    ui->bcCopyButton->setFixedSize({20, 20});
    ui->bcCopyButton->setStyleSheet("QPushButton{background: transparent;}");

    ui->srcMoveButton->setIcon(QIcon("move.png"));
    ui->srcMoveButton->setToolTip("Переместить");
    ui->srcMoveButton->setIconSize({20, 20});
    ui->srcMoveButton->setFixedSize({20,20});
    ui->srcMoveButton->setStyleSheet("QPushButton{background: transparent;}");

    ui->bcMoveButton->setIcon(QIcon("move_left.png"));
    ui->bcMoveButton->setToolTip("Переместить");
    ui->bcMoveButton->setIconSize({20, 20});
    ui->bcMoveButton->setFixedSize({20,20});
    ui->bcMoveButton->setStyleSheet("QPushButton{background: transparent;}");

    ui->srcRenameButton->setIcon(QIcon("rename.png"));
    ui->srcRenameButton->setToolTip("Переименовать");
    ui->srcRenameButton->setIconSize({20, 20});
    ui->srcRenameButton->setFixedSize({20,20});
    ui->srcRenameButton->setStyleSheet("QPushButton{background: transparent;}");

    ui->bcRenameButton->setIcon(QIcon("rename.png"));
    ui->bcRenameButton->setToolTip("Переименовать");
    ui->bcRenameButton->setIconSize({20, 20});
    ui->bcRenameButton->setFixedSize({20,20});
    ui->bcRenameButton->setStyleSheet("QPushButton{background: transparent;}");

    ui->srcPropButton->setIcon(QIcon("properties.png"));
    ui->srcPropButton->setToolTip("Свойства");
    ui->srcPropButton->setIconSize({20, 20});
    ui->srcPropButton->setFixedSize({20,20});
    ui->srcPropButton->setStyleSheet("QPushButton{background: transparent;}");

    ui->bcPropButton->setIcon(QIcon("properties.png"));
    ui->bcPropButton->setToolTip("Свойства");
    ui->bcPropButton->setIconSize({20, 20});
    ui->bcPropButton->setFixedSize({20,20});
    ui->bcPropButton->setStyleSheet("QPushButton{background: transparent;}");

    ui->srcMkDirButton->setIcon(QIcon("create_dir.png"));
    ui->srcMkDirButton->setToolTip("Создать папку");
    ui->srcMkDirButton->setIconSize({20, 20});
    ui->srcMkDirButton->setFixedSize({20,20});
    ui->srcMkDirButton->setStyleSheet("QPushButton{background: transparent;}");

    ui->bcMkDirButton->setIcon(QIcon("create_dir.png"));
    ui->bcMkDirButton->setToolTip("Создать папку");
    ui->bcMkDirButton->setIconSize({20, 20});
    ui->bcMkDirButton->setFixedSize({20,20});
    ui->bcMkDirButton->setStyleSheet("QPushButton{background: transparent;}");

    ui->srcDeleteAllButton->setIcon(QIcon("delete_all.png"));
    ui->srcDeleteAllButton->setToolTip("Удалить все содержимое");
    ui->srcDeleteAllButton->setIconSize({20, 20});
    ui->srcDeleteAllButton->setFixedSize({20,20});
    ui->srcDeleteAllButton->setStyleSheet("QPushButton{background: transparent;}");

    ui->bcDeleteAllButton->setIcon(QIcon("delete_all.png"));
    ui->bcDeleteAllButton->setToolTip("Удалить все содержимое");
    ui->bcDeleteAllButton->setIconSize({20, 20});
    ui->bcDeleteAllButton->setFixedSize({20,20});
    ui->bcDeleteAllButton->setStyleSheet("QPushButton{background: transparent;}");

    ui->copyAllButton->setIcon(QIcon("backup.png"));
    ui->copyAllButton->setToolTip("Резервное копирование файлов");
    ui->copyAllButton->setIconSize({20, 20});
    ui->copyAllButton->setFixedSize({80,20});
    ui->copyAllButton->setStyleSheet("QPushButton{background: transparent;}");

    ui->srcGoToRootButton->setIcon(QIcon("go_to_root.png"));
    ui->srcGoToRootButton->setToolTip("Выход в корень");
    ui->srcGoToRootButton->setIconSize({15, 20});
    ui->srcGoToRootButton->setFixedSize({15,20});
    ui->srcGoToRootButton->setStyleSheet("QPushButton{background: transparent;}");

    ui->bcGoToRootButton->setIcon(QIcon("go_to_root.png"));
    ui->bcGoToRootButton->setToolTip("Выход в корень");
    ui->bcGoToRootButton->setIconSize({15, 20});
    ui->bcGoToRootButton->setFixedSize({15,20});
    ui->bcGoToRootButton->setStyleSheet("QPushButton{background: transparent;}");

    ui->srcGoBackButton->setIcon(QIcon("go_back.png"));
    ui->srcGoBackButton->setToolTip("Назад");
    ui->srcGoBackButton->setIconSize({15, 20});
    ui->srcGoBackButton->setFixedSize({15,20});
    ui->srcGoBackButton->setStyleSheet("QPushButton{background: transparent;}");

    ui->bcGoBackButton->setIcon(QIcon("go_back.png"));
    ui->bcGoBackButton->setToolTip("Назад");
    ui->bcGoBackButton->setIconSize({15, 20});
    ui->bcGoBackButton->setFixedSize({15,20});
    ui->bcGoBackButton->setStyleSheet("QPushButton{background: transparent;}");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_sourceDirView_doubleClicked(const QModelIndex &index)
{
    dirView(index, currentSrcPath);
    //задает текущий путь в поле отображения
    ui->srcPathView->setText(currentSrcPath);
    //меняет текущий путь внитру рабочего класса источника
    srcWorker->changeCurrentPath(currentSrcPath);
    QDir dir(currentSrcPath);
    //проверка, перешел ли пользователь в корневой каталог
    if (dir.isRoot()) {
        //в таком случае включаем кнопку выхода в корень
        ui->srcGoToRootButton->setEnabled(true);
        if (currentBcPath != "") {
            //если в области резерва также открыт каталог
            //включаем кнопку резервного копирования
            ui->copyAllButton->setEnabled(true);
        }
    } else {
        //если пользователь перешел не в корень,
        //а в какой-то дочерний каталог -
        //включаем кнопку перехода в родительский каталог
        ui->srcGoBackButton->setEnabled(true);
    }
    if (srcOperationInProgress == false) {
        //если в потоке источника не выполняется операция
        //включаем кнопки работы с директорией
        ui->srcDeleteAllButton->setEnabled(true);
        ui->srcMkDirButton->setEnabled(true);
        disableSrcButtons();
    }
}


void MainWindow::on_backupDirView_doubleClicked(const QModelIndex &index)
{
    dirView(index, currentBcPath);
    ui->bcPathView->setText(currentBcPath);
    bcWorker->changeCurrentPath(currentBcPath);
    QDir dir(currentBcPath);
    if (dir.isRoot()) {
        ui->bcGoToRootButton->setEnabled(true);
        if (currentSrcPath != "") {
            ui->copyAllButton->setEnabled(true);
        }
    } else {
        ui->bcGoBackButton->setEnabled(true);
    }
    if (bcOperationInProgress == false) {
        ui->bcDeleteAllButton->setEnabled(true);
        ui->bcMkDirButton->setEnabled(true);
        disableBcButtons();
    }
}

void MainWindow::on_srcGoToRootButton_clicked()
{
    //устанавливаем индекс корня файловой системы
    ui->sourceDirView->setRootIndex(model->index(""));
    //обнуляем текущий путь источника
    currentSrcPath = "";
    //записываем текущий пустой путь в соотв. поле
    ui->srcPathView->setText("");

    //выключаем функциональные кнопки
    disableSrcButtons();
    ui->srcDeleteAllButton->setEnabled(false);
    ui->srcMkDirButton->setEnabled(false);
    ui->copyAllButton->setEnabled(false);
    ui->bcCopyButton->setEnabled(false);
    ui->bcMoveButton->setEnabled(false);
    //и кнопку выхода в корень
    ui->srcGoToRootButton->setEnabled(false);
}

void MainWindow::on_bcGoToRootButton_clicked()
{
    ui->backupDirView->setRootIndex(model->index(""));
    currentBcPath = "";
    ui->bcPathView->setText("");

    disableBcButtons();
    ui->bcDeleteAllButton->setEnabled(false);
    ui->bcMkDirButton->setEnabled(false);
    ui->copyAllButton->setEnabled(false);
    ui->srcCopyButton->setEnabled(false);
    ui->srcMoveButton->setEnabled(false);
    ui->bcGoToRootButton->setEnabled(false);
}


void MainWindow::on_srcGoBackButton_clicked()
{
    //Создаем объект класса QDir для работы
    QDir dir(currentSrcPath);
    //Вызываем библиотечный метод выхода в родительский каталог
    dir.cdUp();
    //Подстраиваем интерфейс под новую директорию
    ui->sourceDirView->setRootIndex(model->index(dir.absolutePath()));
    //Обновляем текущий путь
    currentSrcPath = dir.absolutePath();
    //Записываем его в соответствующее поле интерфейса
    ui->srcPathView->setText(dir.absolutePath());
    if (dir.isRoot()) {
        //Если родительский каталог - корневой
        //то выключаем кнопку перехода в род. каталог
        ui->srcGoBackButton->setEnabled(false);
    }
    if (srcOperationInProgress == false) {
        //Если в рабочем классе источника не выполняется
        //какая-то операция - включаем кнопки работы
        //с директорией
        ui->srcDeleteAllButton->setEnabled(true);
        ui->srcMkDirButton->setEnabled(true);
    }
    //Выключаем кнопки работы с файлами
    disableSrcButtons();
}


void MainWindow::on_bcGoBackButton_clicked()
{
    QDir dir(currentBcPath);
    dir.cdUp();
    ui->backupDirView->setRootIndex(model->index(dir.absolutePath()));
    currentBcPath = dir.absolutePath();
    ui->bcPathView->setText(dir.absolutePath());
    if (dir.isRoot()) {
        ui->bcGoBackButton->setEnabled(false);
    }
    if (bcOperationInProgress == false) {
        ui->bcDeleteAllButton->setEnabled(true);
        ui->bcMkDirButton->setEnabled(true);
    }
    disableBcButtons();
}

void MainWindow::dirView(const QModelIndex &index, QString& currentPath)
{
    //sender() - объект, вызвавший функцию
    QListView* listView = (QListView*)sender();
    //QFileInfo - информация о файле или директории
    //на который пользователь дабл-кликнул
    //Также по индексу работает модель файловой системы
    //и поле отображения файлов и директорий
    QFileInfo fileInfo = model->fileInfo(index);
    //Проверка: был ли даблклик по директории
    if (fileInfo.isDir()) {
        //в таком случае делаем переход в директорию
        listView->setRootIndex(index);
        currentPath = model->fileInfo(index).filePath();
    }
}

void MainWindow::onCriticalError(QString errorStr)
{
    errorString += errorStr + '\n';
}


void MainWindow::on_copyAllButton_clicked()
{
    //Окно предупреждения пользователя с уточнением, уверен ли он,
    //что хочет скопировать все файлы источника в каталог резерва
    QMessageBox msgBox(QMessageBox::Warning, "Предупреждение", "Вы уверены, что хотите скопировать все файлы из \"" + currentSrcPath + "\" в \"" + currentBcPath + "\"?");
    msgBox.addButton("Да", QMessageBox::YesRole);
    msgBox.addButton("Отмена", QMessageBox::NoRole);
    //Если пользователь нажмет Да - программа зайдет в условный оператор
    if (!msgBox.exec()) {
        //Выключится кнопка резервного копирования (она включится обратно,
        //если в момент, когда операция выполнится,
        //в области источника и области резерва
        //будут открыты конкретные каталоги
        ui->copyAllButton->setEnabled(false);
        //Создаем окно прогресса
        progressWindow = new ProgressWindow(this);
        //Связываем сигналы объекта рабочего класса с методами
        //окна прогресса, для отображения в нем текущего статуса операции
        connect(cmnWorker, SIGNAL(successStatusGetted(int)), progressWindow, SLOT(setSuccessValue(int)));
        connect(cmnWorker, SIGNAL(statusChanged(int)), progressWindow, SLOT(changeStatus(int)));
        connect(cmnWorker, SIGNAL(workloadIsEmpty()), progressWindow, SLOT(deleteLater()));
        //Вызов сигнала старта операции резервного копирования
        emit startBackupOperation(currentSrcPath, currentBcPath);
    }
}

void MainWindow::copyAllReadyToStart()
{
    QMessageBox::information(this, "Информация", "Копирование успешно завершено");
    if (currentBcPath != "" && currentSrcPath != "") {
        ui->copyAllButton->setEnabled(true);
    }
}

void MainWindow::on_sourceDirView_clicked(const QModelIndex &index)
{
    //По идексу получаем путь и задаем его в соотв. переменную
    srcOperationPath = model->fileInfo(index).filePath();
    //Отправляем путь в рабочий объект источника
    srcWorker->changeOperationPath(srcOperationPath);
    QDir dir(srcOperationPath);
    //Проверяем, кликнул ли пользователь на корневой каталог
    if (!dir.isRoot()) {
        //Если каталог не корневой - включаем функциональные
        //кнопки, в случае, если на данный момент не происходит
        //файловых операций внутри рабочего объекта
        if (srcOperationInProgress == false) {
            enableSrcButtons();
            if (currentBcPath != "") {
                //Если в области резерва открыт каталог
                //включаем кнопки копирования и перемещения
                ui->srcCopyButton->setEnabled(true);
                ui->srcMoveButton->setEnabled(true);
            }
        }
    }
}

void MainWindow::on_backupDirView_clicked(const QModelIndex &index)
{
    bcOperationPath = model->fileInfo(index).filePath();
    bcWorker->changeOperationPath(bcOperationPath);
    QDir dir(bcOperationPath);
    if (!dir.isRoot())
        if (bcOperationInProgress == false) {
            enableBcButtons();
            if (currentSrcPath != "") {
                ui->bcCopyButton->setEnabled(true);
                ui->bcMoveButton->setEnabled(true);
            }
        }
}

void MainWindow::on_srcMkDirButton_clicked()
{
    nameWindow = new NameWindow(this);
    nameWindow->setTitle("Создание папки");
    nameWindow->setText("Задайте имя папке:");
    connect(nameWindow, SIGNAL(nameGetted(QString)), srcWorker, SLOT(makeDir(QString)));
    nameWindow->exec();
    disableSrcButtons();
}

void MainWindow::on_bcMkDirButton_clicked()
{
    nameWindow = new NameWindow(this);
    nameWindow->setTitle("Создание папки");
    nameWindow->setText("Задайте имя папке:");
    connect(nameWindow, SIGNAL(nameGetted(QString)), bcWorker, SLOT(makeDir(QString)));
    nameWindow->exec();
    disableBcButtons();
}


void MainWindow::on_srcRenameButton_clicked()
{
    nameWindow = new NameWindow(this);
    nameWindow->setTitle("Переименование");
    nameWindow->setText("Задайте новое имя:");
    connect(nameWindow, SIGNAL(nameGetted(QString)), srcWorker, SLOT(rename(QString)));
    nameWindow->exec();
    disableSrcButtons();
}

void MainWindow::on_bcRenameButton_clicked()
{
    nameWindow = new NameWindow(this);
    nameWindow->setTitle("Переименование");
    nameWindow->setText("Задайте новое имя:");
    connect(nameWindow, SIGNAL(nameGetted(QString)), bcWorker, SLOT(rename(QString)));
    nameWindow->exec();
    disableBcButtons();
}

void MainWindow::on_srcDeleteButton_clicked()
{
    QFileInfo fileInfo(srcOperationPath);
    QString temp;
    if (fileInfo.isDir()) {
        temp = "папку";
    } else {
        temp = "файл";
    }
    QMessageBox msgBox(QMessageBox::Warning, "Предупреждение", "Вы уверены, что хотите удалить " + temp + " " + fileInfo.fileName() + "?");
    msgBox.addButton("Да", QMessageBox::YesRole);
    msgBox.addButton("Отмена", QMessageBox::NoRole);
    if (!msgBox.exec()) {
        srcOperationInProgress = true;
        progressWindow = new ProgressWindow(this);
        connect(srcWorker, SIGNAL(successStatusGetted(int)), progressWindow, SLOT(setSuccessValue(int)));
        connect(srcWorker, SIGNAL(statusChanged(int)), progressWindow, SLOT(changeStatus(int)));
        connect(srcWorker, SIGNAL(workloadIsEmpty()), progressWindow, SLOT(deleteLater()));
        disableSrcButtons();
        ui->srcDeleteAllButton->setEnabled(false);
        ui->srcMkDirButton->setEnabled(false);
        emit startSrcDelete();
    }
}

void MainWindow::on_bcDeleteButton_clicked()
{
    QFileInfo fileInfo(bcOperationPath);
    QString temp;
    if (fileInfo.isDir()) {
        temp = "папку";
    } else {
        temp = "файл";
    }
    QMessageBox msgBox(QMessageBox::Warning, "Предупреждение", "Вы уверены, что хотите удалить " + temp + " " + fileInfo.fileName() + "?");
    msgBox.addButton("Да", QMessageBox::YesRole);
    msgBox.addButton("Отмена", QMessageBox::NoRole);
    if (!msgBox.exec()) {
        bcOperationInProgress = true;
        progressWindow = new ProgressWindow(this);
        connect(bcWorker, SIGNAL(successStatusGetted(int)), progressWindow, SLOT(setSuccessValue(int)));
        connect(bcWorker, SIGNAL(statusChanged(int)), progressWindow, SLOT(changeStatus(int)));
        connect(bcWorker, SIGNAL(workloadIsEmpty()), progressWindow, SLOT(deleteLater()));
        disableBcButtons();
        ui->bcDeleteAllButton->setEnabled(false);
        ui->bcMkDirButton->setEnabled(false);
        emit startBcDelete();
    }
}

void MainWindow::on_deleteAll_finished()
{
    QString temp;
    if (sender() == srcWorker) {
        temp = "источника";
        srcOperationInProgress = false;
        if (currentSrcPath != "") {
            ui->srcMkDirButton->setEnabled(true);
        }
    } else {
        temp = "резерва";
        bcOperationInProgress = false;
        if (currentBcPath != "") {
            ui->bcMkDirButton->setEnabled(true);
        }
    }
    if (errorString == "") {
        QMessageBox::information(nullptr, "Успешно", "Файлы " + temp + " успешно удалены");
    } else {
        QMessageBox::critical(nullptr, "Ошибки", errorString);
        errorString = "";
    }
}

void MainWindow::on_srcCopyButton_clicked()
{
    //Устанавливаем, что на потоке источника выполняется операция
    srcOperationInProgress = true;
    //Создаем окно прогресса
    progressWindow = new ProgressWindow(this);
    connect(srcWorker, SIGNAL(successStatusGetted(int)), progressWindow, SLOT(setSuccessValue(int)));
    connect(srcWorker, SIGNAL(statusChanged(int)), progressWindow, SLOT(changeStatus(int)));
    connect(srcWorker, SIGNAL(workloadIsEmpty()), progressWindow, SLOT(deleteLater()));
    //Выключаем функциональные кнопки источника
    disableSrcButtons();
    ui->srcDeleteAllButton->setEnabled(false);
    ui->srcMkDirButton->setEnabled(false);
    //Вызываем сигнал начала копирования
    //srcWorker->copy(bcWorker->getCurrentPath());
    emit startSrcCopy(bcWorker->getCurrentPath());
}

void MainWindow::on_bcCopyButton_clicked()
{
    bcOperationInProgress = true;
    progressWindow = new ProgressWindow(this);
    connect(bcWorker, SIGNAL(successStatusGetted(int)), progressWindow, SLOT(setSuccessValue(int)));
    connect(bcWorker, SIGNAL(statusChanged(int)), progressWindow, SLOT(changeStatus(int)));
    connect(bcWorker, SIGNAL(workloadIsEmpty()), progressWindow, SLOT(deleteLater()));
    emit startBcCopy(srcWorker->getCurrentPath());
    disableBcButtons();
    ui->bcDeleteAllButton->setEnabled(false);
    ui->bcMkDirButton->setEnabled(false);
}

void MainWindow::on_srcMoveButton_clicked()
{
    QFileInfo fileInfo(srcOperationPath);
    QString temp;
    if (fileInfo.isDir()) {
        temp = "папку";
    } else {
        temp = "файл";
    }
    QMessageBox msgBox(QMessageBox::Warning, "Предупреждение", "Вы уверены, что хотите переместить " + temp + " " + fileInfo.fileName() + "?");
    msgBox.addButton("Да", QMessageBox::YesRole);
    msgBox.addButton("Отмена", QMessageBox::NoRole);
    if (!msgBox.exec()) {
        srcOperationInProgress = true;
        progressWindow = new ProgressWindow(this);
        connect(srcWorker, SIGNAL(successStatusGetted(int)), progressWindow, SLOT(setSuccessValue(int)));
        connect(srcWorker, SIGNAL(statusChanged(int)), progressWindow, SLOT(changeStatus(int)));
        connect(srcWorker, SIGNAL(workloadIsEmpty()), progressWindow, SLOT(deleteLater()));
        disableSrcButtons();
        ui->srcDeleteAllButton->setEnabled(false);
        ui->srcMkDirButton->setEnabled(false);
        emit startSrcMove(bcWorker->getCurrentPath());
    }
}

void MainWindow::on_bcMoveButton_clicked()
{
    QFileInfo fileInfo(bcOperationPath);
    QString temp;
    if (fileInfo.isDir()) {
        temp = "папку";
    } else {
        temp = "файл";
    }
    QMessageBox msgBox(QMessageBox::Warning, "Предупреждение", "Вы уверены, что хотите переместить " + temp + " " + fileInfo.fileName() + "?");
    msgBox.addButton("Да", QMessageBox::YesRole);
    msgBox.addButton("Отмена", QMessageBox::NoRole);
    if (!msgBox.exec()) {
        bcOperationInProgress = true;
        progressWindow = new ProgressWindow(this);
        connect(bcWorker, SIGNAL(successStatusGetted(int)), progressWindow, SLOT(setSuccessValue(int)));
        connect(bcWorker, SIGNAL(statusChanged(int)), progressWindow, SLOT(changeStatus(int)));
        connect(bcWorker, SIGNAL(workloadIsEmpty()), progressWindow, SLOT(deleteLater()));
        disableBcButtons();
        ui->bcDeleteAllButton->setEnabled(false);
        ui->bcMkDirButton->setEnabled(false);
        emit startBcMove(srcWorker->getCurrentPath());
    }
}

void MainWindow::on_prop_finished(QString information)
{
    QMessageBox::information(nullptr, "Информация", information);
}

void MainWindow::on_copy_finished()
{
    //Если сигнал завершения отправил объект
    //рабочего класса источника
    if (sender() == srcWorker) {
        //Сигнализируем, что операция завершена
        //и поток пуст
        srcOperationInProgress = false;
        //Если в области источника находимся
        //в конкретном каталоге - включаем
        //кнопки работы с директорией
        if (currentSrcPath != "") {
            ui->srcDeleteAllButton->setEnabled(true);
            ui->srcMkDirButton->setEnabled(true);
        }
    } else {
        bcOperationInProgress = false;
        if (currentBcPath != "") {
            ui->bcDeleteAllButton->setEnabled(true);
            ui->bcMkDirButton->setEnabled(true);
        }
    }
    QMessageBox::information(nullptr, "Успешно", "Успешно скопировано");
}

void MainWindow::on_delete_finished()
{
    if (sender() == srcWorker) {
        srcOperationInProgress = false;
        if (currentSrcPath != "") {
            ui->srcDeleteAllButton->setEnabled(true);
            ui->srcMkDirButton->setEnabled(true);
        }
    } else {
        bcOperationInProgress = false;
        if (currentBcPath != "") {
            ui->bcDeleteAllButton->setEnabled(true);
            ui->bcMkDirButton->setEnabled(true);
        }
    }
    if (errorString == "") {
        QMessageBox::information(nullptr, "Успешно", "Успешно удалено");
    } else {
        QMessageBox::critical(nullptr, "Ошибки", errorString);
        errorString = "";
    }
}

void MainWindow::on_move_finished()
{
    if (sender() == srcWorker) {
        srcOperationInProgress = false;
        if (currentSrcPath != "") {
            ui->srcDeleteAllButton->setEnabled(true);
            ui->srcMkDirButton->setEnabled(true);
        }
    } else {
        bcOperationInProgress = false;
        if (currentBcPath != "") {
            ui->bcDeleteAllButton->setEnabled(true);
            ui->bcMkDirButton->setEnabled(true);
        }
    }
    if (errorString == "") {
        QMessageBox::information(nullptr, "Успешно", "Успешно перемещено");
    } else {
        QMessageBox::critical(nullptr, "Ошибки", errorString);
        errorString = "";
    }
}


void MainWindow::on_srcDeleteAllButton_clicked()
{
    QMessageBox msgBox(QMessageBox::Warning, "Предупреждение", "Вы уверены, что хотите удалить все файлы источника?");
    msgBox.addButton("Да", QMessageBox::YesRole);
    msgBox.addButton("Отмена", QMessageBox::NoRole);
    if (!msgBox.exec()) {
        srcOperationInProgress = true;
        progressWindow = new ProgressWindow(this);
        connect(srcWorker, SIGNAL(successStatusGetted(int)), progressWindow, SLOT(setSuccessValue(int)));
        connect(srcWorker, SIGNAL(statusChanged(int)), progressWindow, SLOT(changeStatus(int)));
        connect(srcWorker, SIGNAL(workloadIsEmpty()), progressWindow, SLOT(deleteLater()));
        disableSrcButtons();
        ui->srcDeleteAllButton->setEnabled(false);
        ui->srcMkDirButton->setEnabled(false);
        emit startSrcDelAll();
    }
}


void MainWindow::on_bcDeleteAllButton_clicked()
{
    QMessageBox msgBox(QMessageBox::Warning, "Предупреждение", "Вы уверены, что хотите удалить все файлы резерва?");
    msgBox.addButton("Да", QMessageBox::YesRole);
    msgBox.addButton("Отмена", QMessageBox::NoRole);
    if (!msgBox.exec()) {
        bcOperationInProgress = true;
        progressWindow = new ProgressWindow(this);
        connect(bcWorker, SIGNAL(successStatusGetted(int)), progressWindow, SLOT(setSuccessValue(int)));
        connect(bcWorker, SIGNAL(statusChanged(int)), progressWindow, SLOT(changeStatus(int)));
        connect(bcWorker, SIGNAL(workloadIsEmpty()), progressWindow, SLOT(deleteLater()));
        disableBcButtons();
        ui->bcDeleteAllButton->setEnabled(false);
        ui->bcMkDirButton->setEnabled(false);
        emit startBcDelAll();
    }
}

void MainWindow::disableAllButtons()
{
    ui->srcCopyButton->setEnabled(false);
    ui->srcDeleteButton->setEnabled(false);
    ui->srcMoveButton->setEnabled(false);
    ui->srcRenameButton->setEnabled(false);
    ui->srcPropButton->setEnabled(false);
    ui->srcMkDirButton->setEnabled(false);
    ui->srcDeleteAllButton->setEnabled(false);

    ui->bcCopyButton->setEnabled(false);
    ui->bcDeleteButton->setEnabled(false);
    ui->bcMoveButton->setEnabled(false);
    ui->bcRenameButton->setEnabled(false);
    ui->bcPropButton->setEnabled(false);
    ui->bcMkDirButton->setEnabled(false);
    ui->bcDeleteAllButton->setEnabled(false);

    ui->copyAllButton->setEnabled(false);

    ui->srcGoBackButton->setEnabled(false);
    ui->srcGoToRootButton->setEnabled(false);

    ui->bcGoBackButton->setEnabled(false);
    ui->bcGoToRootButton->setEnabled(false);
}

void MainWindow::disableSrcButtons()
{
    ui->srcCopyButton->setEnabled(false);
    ui->srcDeleteButton->setEnabled(false);
    ui->srcMoveButton->setEnabled(false);
    ui->srcRenameButton->setEnabled(false);
    ui->srcPropButton->setEnabled(false);
    //ui->srcMkDirButton->setEnabled(false);
    //ui->srcDeleteAllButton->setEnabled(false);
}

void MainWindow::enableSrcButtons()
{
    //ui->srcCopyButton->setEnabled(true);
    ui->srcDeleteButton->setEnabled(true);
    //ui->srcMoveButton->setEnabled(true);
    ui->srcRenameButton->setEnabled(true);
    ui->srcPropButton->setEnabled(true);
    //ui->srcMkDirButton->setEnabled(true);
    //ui->srcDeleteAllButton->setEnabled(true);
}

void MainWindow::disableBcButtons()
{
    ui->bcCopyButton->setEnabled(false);
    ui->bcDeleteButton->setEnabled(false);
    ui->bcMoveButton->setEnabled(false);
    ui->bcRenameButton->setEnabled(false);
    ui->bcPropButton->setEnabled(false);
    //ui->bcMkDirButton->setEnabled(false);
    //ui->bcDeleteAllButton->setEnabled(false);
}

void MainWindow::enableBcButtons()
{
    //ui->bcCopyButton->setEnabled(true);
    ui->bcDeleteButton->setEnabled(true);
    //ui->bcMoveButton->setEnabled(true);
    ui->bcRenameButton->setEnabled(true);
    ui->bcPropButton->setEnabled(true);
    //ui->bcMkDirButton->setEnabled(true);
    //ui->bcDeleteAllButton->setEnabled(true);
}
