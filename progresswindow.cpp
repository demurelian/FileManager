#include "progresswindow.h"
#include "ui_progresswindow.h"

ProgressWindow::ProgressWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ProgressWindow)
{
    ui->setupUi(this);
    ui->progressBar->setValue(0);
    currentStatus = -0;
    maxStatus = -1;
    setWindowTitle("Прогресс операции");
}

ProgressWindow::~ProgressWindow()
{
    delete ui;
}

void ProgressWindow::setSuccessValue(const int& val)
{
    if (maxStatus == -1)
    maxStatus = val;
    ui->progressBar->setMaximum(maxStatus);
    show();
}

void ProgressWindow::changeStatus(int val)
{
    if (maxStatus != 0) {
        currentStatus = val;
        ui->progressBar->setValue(currentStatus);
        if (val+1 == maxStatus) {
            on_success();
        }
    }
}

void ProgressWindow::on_success()
{
    maxStatus = -1;
    deleteLater();
}

