#include "worker.h"

Worker::Worker(QObject *parent)
    : QObject{parent}
{
    status = -1;
    moveOperation = false;
}

void Worker::changeCurrentPath(QString path)
{
    currentPath = path;
}

void Worker::changeOperationPath(QString path)
{
    operationPath = path;
}

QString Worker::getCurrentPath()
{
    return currentPath;
}

QString Worker::getOperationPath()
{
    return operationPath;
}

void Worker::copy(QString targetPath)
{
    //Переменная, отвечающая за информацию о операционном файле
    QFileInfo fileInfo(operationPath);
    //Создаем целевой имя файла
    QString target = targetPath + "/" + fileInfo.fileName();
    //Если работаем с файлом
    if (fileInfo.isFile()) {
        //Создаем переменную для работы с файлом
        QFile temp(operationPath);
        temp.remove(target);
        //Копируем его
        temp.copy(target);
    }
    //Если работаем с папкой
    if (fileInfo.isDir()) {
        //Создаем переменную для работы с папкой
        QDir tempDir(targetPath);
        //Создаем соответствующую папку в каталоге
        tempDir.mkdir(fileInfo.fileName());
        targetPath += "/" + fileInfo.fileName();
        //Реализуем копирование папки через функцию
        //Резервного копирования
        runBackup(operationPath, targetPath);
    }
    //Если мы копируем не для операции перемещения
    if (!moveOperation)
        emit copy_finished();
}

void Worker::del()
{
    QFileInfo fileInfo(operationPath);
    QFileInfoList contentList = QFileInfoList();
    if (fileInfo.isDir()) {
        QDir dir(operationPath);
        recursiveContentList(dir, contentList);
        if (contentList.size() > 0) {
            emit successStatusGetted(contentList.size());
            status = 0;
        } else {
            emit workloadIsEmpty();
        }
        foreach(QFileInfo fileInfo2, contentList) {
            if (fileInfo2.isDir()) {
                QDir dir2(fileInfo2.filePath());
                dir2.removeRecursively();
            }
            if (fileInfo2.isFile()) {
                QFile file(fileInfo2.filePath());
                if (!file.remove()) {
                    if (file.errorString() != "Системе не удается найти указанный путь.") {
                        QString errorString = "Не удалось удалить файл \"" + fileInfo2.fileName() + "\":" + file.errorString();
                        emit criticalOperationError(errorString);
                    }
                }
            }
            status++;
            emit statusChanged(status);
        }
        dir.cdUp();
        dir.rmdir(operationPath);
    }
    if (fileInfo.isFile()) {
        QFile file(fileInfo.filePath());
        if (!file.remove()) {
            if (file.errorString() != "Системе не удается найти указанный путь.") {
                QString errorString = "Не удалось удалить файл \"" + fileInfo.fileName() + "\":" + file.errorString();
                emit criticalOperationError(errorString);
            }
        }
    }
    status = -1;
    if (!moveOperation)
        emit del_finished();
}


void Worker::move(QString targetPath)
{
    moveOperation = true;
    copy(targetPath);
    del();
    moveOperation = false;
    emit move_finished();
}

void Worker::rename(QString name)
{
    QFileInfo fileInfo(operationPath);
    QString newName = fileInfo.absoluteDir().path() + "/" + name;
    if (fileInfo.completeSuffix() != "") {
       newName += "." + fileInfo.completeSuffix();
    }
    if (fileInfo.isDir()) {
        QDir dir(operationPath);
        if (!dir.rename(operationPath, newName)) {
            QMessageBox::critical(nullptr, "Ошибка", "Не удалось переименовать папку.");
        }
    }
    if (fileInfo.isFile()) {
        QFile file(operationPath);
        if (!file.rename(operationPath, newName)) {
            QMessageBox::critical(nullptr, "Ошибка", file.errorString());
        }
    }
}

void Worker::properties()
{
    QFileInfo fileInfo(operationPath);
    QString information = "";
    if (fileInfo.isDir()) {
        information += "Тип: папка\n";
    } else {
        information += "Тип: файл " + fileInfo.suffix() + '\n';
    }
    information += "Имя: " + fileInfo.baseName() + '\n';
    QString sizeAbbreviation = "Б";
    qint64 intSize = 0;
    float size;
    if (fileInfo.isDir()) {
        QDir dir(operationPath);
        dirSize(dir, intSize);
        size = intSize;
    } else {
        size = fileInfo.size();
    }
    int sizeDegree = 1;
    while (size > 1024) {
        size /= 1024;
        sizeDegree++;
    }
    switch (sizeDegree) {
    case 2:
        sizeAbbreviation = "КБ";
        break;
    case 3:
        sizeAbbreviation = "МБ";
        break;
    case 4:
        sizeAbbreviation = "ГБ";
        break;
    case 5:
        sizeAbbreviation = "ТБ";
        break;
    }
    information += "Размер: " + QString::number(round(size*100)/100) + " " + sizeAbbreviation + '\n';
    information += "Расположение: " + fileInfo.absoluteDir().path() + '\n';
    information += "Создан: " + fileInfo.birthTime().toString("dd-MM-yyyy, hh:mm:ss") + '\n';
    information += "Изменен: " + fileInfo.lastModified().toString("dd-MM-yyyy, hh:mm:ss");
    emit properties_finished(information);
}

void Worker::makeDir(QString name)
{
    QDir localSrc = QDir(currentPath);
    localSrc.mkdir(name);
}

void Worker::delAll()
{
    QFileInfoList contentList = QFileInfoList();
    QDir dir(currentPath);
    recursiveContentList(dir, contentList);
    if (contentList.size() > 0) {
        emit successStatusGetted(contentList.size());
        status = 0;
    } else {
        emit workloadIsEmpty();
    }
    foreach(QFileInfo fileInfo2, contentList) {
        if (fileInfo2.isDir()) {
            QDir dir2(fileInfo2.filePath());
            dir2.removeRecursively();
        }
        if (fileInfo2.isFile()) {
            QFile file(fileInfo2.filePath());
            if (!file.remove()) {
                if (file.errorString() != "Системе не удается найти указанный путь.") {
                    QString errorString = "Не удалось удалить файл \"" + fileInfo2.fileName() + "\":" + file.errorString();
                    emit criticalOperationError(errorString);
                }
            }
            //criticalerorstring
        }
        status++;
        emit statusChanged(status);
    }
    emit delAll_finished();
}

void Worker::runBackup(QString srcPath, QString bcPath)
{
    //Создание двух директорий, соответствующих
    //путям источника и резерва, для работы с ними
    QDir srcDir = QDir(srcPath);
    QDir bcDir = QDir(bcPath);
    //Список, хранящий информацию о файлах
    QFileInfoList diffList = QFileInfoList();
    //Рекурсивная функция, определяющая разницу
    //содержимого директорий
    contentDifference(srcDir, bcDir, diffList);
    //В эту функцию мы отправляли список, хранящий
    //информацию о файлах - соответственно, если
    //он не пуст (т.е. его размер не равен нулю)
    //мы зададим окну прогресса максимальный статус
    if (diffList.size() > 0) {
        emit successStatusGetted(diffList.size());
    } else {
        //в противном случае пошлем сигнал,
        //что список пуст и объем работы нулевой.
        emit workloadIsEmpty();
    }
    //Инициализация переменной, отвечающей за статус
    //прохождения операции
    status = 0;
    //Цикл для работы со списком файлов
    foreach(QFileInfo diffInfo, diffList) {
        //Добавление дополнительного разделения,
        //в случае, если один из каталогов - корень
        QString bcPathBugFix = bcDir.absolutePath();
        if (srcDir.isRoot()) {
            bcPathBugFix += '/';
        }
        //Создание нового имени файла
        //для корректного копирования
        QString backupPath = diffInfo.filePath().replace(srcDir.absolutePath(), bcPathBugFix/*bcDir.absolutePath()*/);
        //Проверка - текущий объект в списке - это
        //файл (не директория)?
        if (diffInfo.isFile()) {
            //Копируем файл
            QFile temp(diffInfo.filePath());
            temp.remove(backupPath);
            if (!temp.copy(diffInfo.absoluteFilePath(), backupPath)) {
                break;
            }
        }
        //Проверка - папка?
        if (diffInfo.isDir()) {
            //Создаем соотв. папку
            bcDir.mkdir(backupPath);
        }
        status++;
        //Посылаем сигнал окну прогресса
        //для изменения текущего статуса
        emit statusChanged(status);
    }
    emit backupFinished();
    status = -1;
}

void Worker::contentDifference(QDir &sDir, QDir &bDir, QFileInfoList &diffList)
{
    foreach(QFileInfo sInfo, sDir.entryInfoList(QDir::Files|QDir::Dirs|QDir::NoDotAndDotDot, QDir::Name|QDir::DirsFirst)) {
        bool fileExists = false;
        foreach (QFileInfo dInfo, bDir.entryInfoList(QDir::Files|QDir::Dirs|QDir::NoDotAndDotDot, QDir::Name|QDir::DirsFirst)) {
            if (sInfo.fileName() == dInfo.fileName()) {
                if (sInfo.isDir() || (sInfo.lastModified() <= dInfo.lastModified()))
                     fileExists = true;
            }
        }
        if (!fileExists)
            diffList.append(sInfo);
        if (sInfo.isFile())
            continue;
        if (fileExists) {
            sDir.cd(sInfo.fileName());
            bDir.cd(sInfo.fileName());
            contentDifference(sDir, bDir, diffList);
            sDir.cdUp();
            bDir.cdUp();
        }
        else {
            sDir.cd(sInfo.fileName());
            recursiveContentList(sDir, diffList);
            sDir.cdUp();
        }
    }
}

void Worker::recursiveContentList(QDir &dir, QFileInfoList &contentList)
{
    foreach(QFileInfo fileInfo, dir.entryInfoList(QDir::Files|QDir::Dirs|QDir::NoDotAndDotDot, QDir::Name|QDir::DirsFirst)) {
        contentList.append(fileInfo);
        if (fileInfo.isDir() && dir.cd(fileInfo.fileName())) {
            recursiveContentList(dir, contentList);
            dir.cdUp();
        }
    }
}

qint64 Worker::dirSize(QDir &dir, qint64 &size)
{
    foreach(QFileInfo fileInfo, dir.entryInfoList(QDir::Files|QDir::Dirs|QDir::NoDotAndDotDot, QDir::Name|QDir::DirsFirst)) {
        if (fileInfo.isDir() && dir.cd(fileInfo.fileName())) {
            dirSize(dir, size);
            dir.cdUp();
        }
        if (fileInfo.isFile()) {
            size += fileInfo.size();
        }
    }
}
