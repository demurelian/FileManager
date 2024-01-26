#ifndef NAMEWINDOW_H
#define NAMEWINDOW_H

#include <QDialog>

namespace Ui {
class NameWindow;
}

class NameWindow : public QDialog
{
    Q_OBJECT

public:
    explicit NameWindow(QWidget *parent = nullptr);
    ~NameWindow();

public slots:
    void setText(QString text);
    void setTitle(QString title);

signals:
    void nameGetted(const QString& name);

private slots:
    void on_okButton_clicked();

    void on_nameEdit_textChanged(const QString &arg1);

private:
    Ui::NameWindow *ui;
};

#endif // NAMEWINDOW_H
