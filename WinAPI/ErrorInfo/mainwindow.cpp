#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <windows.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_searchBtn_clicked()
{
    LPWSTR bufPtr = nullptr;
    QString rawCode = ui->errorCode->text();
    unsigned long err = 0;

    if(rawCode.contains("0x"))
    {
        err = rawCode.toULong(nullptr, 16);
    }
    else if(rawCode[0] == '0')
    {
        err = rawCode.toULong(nullptr, 8);
    }
    else
    {
        err = rawCode.toULong();
    }

    FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                   FORMAT_MESSAGE_FROM_SYSTEM |
                   FORMAT_MESSAGE_IGNORE_INSERTS,
                   nullptr, err, 0, (LPWSTR)&bufPtr, 0, nullptr);
    const QString result =
        (bufPtr) ? QString::fromUtf16((const ushort*)bufPtr).trimmed() :
                   QString("Unknown Error %1").arg(err);
    LocalFree(bufPtr);
    ui->errorDesc->setText(result);

}
