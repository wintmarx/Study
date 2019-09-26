#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <windows.h>

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

    void on_sliderT1_valueChanged(int value);

    void on_sliderT2_valueChanged(int value);

    void on_startBtn_clicked();

    void on_stopBtn_clicked();

    void on_timer_tick();

private:
    Ui::MainWindow *ui;
    HANDLE thread1;
    HANDLE thread2;
    bool isThread1Sleep = false;
    bool isThread2Sleep = false;
    long threadParam1;
    long threadParam2;
    friend DWORD WINAPI ThreadProc1(CONST LPVOID lpParam);
    friend DWORD WINAPI ThreadProc2(CONST LPVOID lpParam);
    QTimer *timer;
};

#endif // MAINWINDOW_H
