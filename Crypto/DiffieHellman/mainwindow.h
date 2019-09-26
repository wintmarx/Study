#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_generateKeyButton_clicked();

    void on_recvButton_clicked();

    void on_sendButton_clicked();

    void on_loadFileButton_clicked();

    void on_genButton_clicked();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
