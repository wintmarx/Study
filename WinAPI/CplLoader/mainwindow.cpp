#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <windows.h>

#include <QFileDialog>
#include <cpl.h>
#include <QtWin>

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
    QString filename = QFileDialog::getOpenFileName(this, tr("Open СPL"), ".", tr("(*.cpl)"));
    ui->filePathEdit->setText(filename);
}

void MainWindow::on_launchBtn_clicked()
{
    LPCWSTR str = (const wchar_t*) ui->filePathEdit->text().utf16();
    HMODULE hDLL = LoadLibrary(str);
    APPLET_PROC cplAppletFunc = (APPLET_PROC)GetProcAddress(hDLL, "CPlApplet");
    HWND fgWnd = GetForegroundWindow();
    CPLINFO cplInfo;
    WCHAR buff[1024];

    cplAppletFunc(fgWnd, CPL_INIT, 0, 0);

    cplAppletFunc(fgWnd, CPL_INQUIRE, 0, (LONG)&cplInfo);

    LoadString(hDLL, (UINT)cplInfo.idName, (LPWSTR)buff, sizeof(buff));
    ui->cplNameEdit->setText(QString::fromWCharArray(buff));

    LoadString(hDLL, (UINT)cplInfo.idInfo, (LPWSTR)buff, sizeof(buff));
    ui->cplInfoEdit->setText(QString::fromWCharArray(buff));

    ui->icon->setPixmap(QtWin::fromHICON(LoadIcon(hDLL, (LPCWSTR)cplInfo.idIcon)));
    ui->icon->setScaledContents( true );

    cplAppletFunc(fgWnd, CPL_DBLCLK, 0, 0);
}
