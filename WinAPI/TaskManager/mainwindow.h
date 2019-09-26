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
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_searchBtn_clicked();

    void on_launchBtn_clicked();

    void on_terminateBtn_clicked();

    void check_procs();

private:
    Ui::MainWindow *ui;
    QTimer *timer;

};

#endif // MAINWINDOW_H
