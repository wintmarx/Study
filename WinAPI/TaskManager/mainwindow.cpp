#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <windows.h>

#include <QFileDialog>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(check_procs()));
    timer->start(500); // И запустим таймер
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_searchBtn_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                "C://Windows/system32",
                                                QFileDialog::ShowDirsOnly
                                                | QFileDialog::DontResolveSymlinks);
    ui->dirPathEdit->setText(dir);
    ui->allExecList->clear();
    QString execFile;
    WIN32_FIND_DATA FindFileData;
    HANDLE hf;
    hf=FindFirstFile(dir.append("/*.exe").toStdWString().c_str(), &FindFileData);
    if (hf!=INVALID_HANDLE_VALUE)
    {
        do
        {
            //add item
            execFile = QString::fromWCharArray(&(FindFileData.cFileName[0]));
            ui->allExecList->addItem(execFile);
        }
        while (FindNextFile(hf,&FindFileData)!=0);
        FindClose(hf);
    }
}

void MainWindow::on_launchBtn_clicked()
{
    if(ui->allExecList->count() == 0)
    {
        return;
    }
    PROCESS_INFORMATION pi;
    STARTUPINFO cif;
    ZeroMemory(&cif,sizeof(STARTUPINFO));
    if (CreateProcess(ui->dirPathEdit->text()
                      .append("/")
                      .append(ui->allExecList->currentItem()->text())
                      .toStdWString()
                      .c_str(),
                      NULL, NULL, NULL, FALSE, NULL, NULL, NULL, &cif, &pi) == TRUE)
    {
        QListWidgetItem *item = new QListWidgetItem();
        item->setText(ui->allExecList->currentItem()->text());
        item->setData(Qt::UserRole, QVariant((uint)pi.hProcess));
        ui->activeExecList->addItem(item);
    }
}

void MainWindow::on_terminateBtn_clicked()
{
    if(ui->activeExecList->count() == 0)
    {
        return;
    }
    HANDLE h = (HANDLE)ui->activeExecList->currentItem()->data(Qt::UserRole).toUInt();
    if(h == nullptr)
    {
        return;
    }
    WCHAR name[50];
    GetModuleFileName((HMODULE)h, name, 50);
    QString strt(QString::fromStdWString(name));
    if( TerminateProcess(h, NO_ERROR) == FALSE)
    {
        return;
    }
}

void MainWindow::check_procs()
{
    DWORD status = NO_ERROR;
    HANDLE h = 0;
    for(int i = 0; i < ui->activeExecList->count(); i++)
    {
        h = (HANDLE)ui->activeExecList->item(i)->data(Qt::UserRole).toUInt();
        if(GetExitCodeProcess(h, &status) == FALSE || status == STILL_ACTIVE)
        {
            continue;
        }
        CloseHandle(h);
        delete ui->activeExecList->item(i);
    }
}
