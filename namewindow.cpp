#include "namewindow.h"
#include "ui_namewindow.h"

NameWindow::NameWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NameWindow)
{
    ui->setupUi(this);
    ui->okButton->setEnabled(false);
}

NameWindow::~NameWindow()
{
    delete ui;
}

void NameWindow::setText(QString text)
{
    ui->textLabel->setText(text);
}

void NameWindow::setTitle(QString title)
{
    setWindowTitle(title);
}

void NameWindow::on_okButton_clicked()
{
    emit nameGetted(ui->nameEdit->text());
    deleteLater();
}

void NameWindow::on_nameEdit_textChanged(const QString &arg1)
{
    if (ui->nameEdit->text().size() > 0) {
        ui->okButton->setEnabled(true);
    } else {
        ui->okButton->setEnabled(false);
    }
}
