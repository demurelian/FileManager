#ifndef PROGRESSWINDOW_H
#define PROGRESSWINDOW_H

#include <QDialog>

namespace Ui {
class ProgressWindow;
}

class ProgressWindow : public QDialog
{
    Q_OBJECT

public:
    explicit ProgressWindow(QWidget *parent = nullptr);
    ~ProgressWindow();
public slots:
    void setSuccessValue(const int& val);
    void changeStatus(int val);

private:
    void on_success();

private:
    Ui::ProgressWindow *ui;
    int currentStatus;
    int maxStatus;
};

#endif // PROGRESSWINDOW_H
