#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTimer>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->sliderT1->setRange(THREAD_PRIORITY_LOWEST, THREAD_PRIORITY_HIGHEST);
    ui->sliderT2->setRange(THREAD_PRIORITY_LOWEST, THREAD_PRIORITY_HIGHEST);
    SetProcessAffinityMask(GetCurrentProcess(), 1);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_timer_tick()
{
    ui->barT1->setValue(static_cast<int>(static_cast<float>(threadParam1)/(threadParam1 + threadParam2) * 100));
    ui->barT2->setValue(static_cast<int>(static_cast<float>(threadParam2)/(threadParam1 + threadParam2) * 100));
    threadParam1 = 0;
    threadParam2 = 0;
}

DWORD WINAPI ThreadProc1(CONST LPVOID lpParam)
{
    MainWindow *mw = (MainWindow*)lpParam;
    double finalValue = 0;
    while(1)
    {
        if(mw->ui->checkBoxT1->isChecked())
        {
            Sleep(0);
            continue;
        }
        finalValue = cos( sin( pow(cos(mw->threadParam1), 2))); //+ pow(sin(mw->threadParam1), 5) + pow(cos(finalValue), 4);
        mw->threadParam1++;
    }
    ExitThread(0);
}

DWORD WINAPI ThreadProc2(CONST LPVOID lpParam)
{
    MainWindow *mw = (MainWindow*)lpParam;
    double finalValue = 0;
    while(1)
    {
        if(mw->ui->checkBoxT2->isChecked())
        {
            Sleep(0);
            continue;
        }
        finalValue = cos( sin( pow(cos(mw->threadParam2), 2))); //+ pow(sin(mw->threadParam2), 5) ) + pow(cos(finalValue), 4) );
        mw->threadParam2++;
    }
    ExitThread(0);
}

void MainWindow::on_sliderT1_valueChanged(int value)
{
    if(thread1 == nullptr)
    {
        return;
    }
    SetThreadPriority(thread1, value);
}

void MainWindow::on_sliderT2_valueChanged(int value)
{
    if(thread2 == nullptr)
    {
        return;
    }
    SetThreadPriority(thread2, value);
}

void MainWindow::on_startBtn_clicked()
{
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(on_timer_tick()));
    timer->start(500);
    thread1 = CreateThread(nullptr, 0, &ThreadProc1, this, 0, nullptr);
    thread2 = CreateThread(nullptr, 0, &ThreadProc2, this, 0, nullptr);
}

void MainWindow::on_stopBtn_clicked()
{

}
