#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <cstring>
#include <QCloseEvent>

#define BUF_SIZE 256
#define FILE_MAPPING_NAME L"memobj"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    updateTimer = new QTimer(this);
    updateTimer->start(100);
    connect(updateTimer, SIGNAL(timeout()), this, SLOT(Update()));

    ipcRoleDetectionEvent = OpenEventA(EVENT_ALL_ACCESS, FALSE, ipcRoleDetectionEventName);
    if(ipcRoleDetectionEvent == nullptr)
    {
        ipcRoleDetectionEvent = CreateEventA(nullptr, FALSE, FALSE, ipcRoleDetectionEventName);
        ipcNewMessageEvent = CreateEventA(nullptr, TRUE, FALSE, ipcNewMessageEventName);

        mapFile = CreateFileMapping(INVALID_HANDLE_VALUE,    // use paging file
                          nullptr,                    // default security
                          PAGE_READWRITE,          // read/write access
                          0,                       // maximum object size (high-order DWORD)
                          BUF_SIZE,                // maximum object size (low-order DWORD)
                          FILE_MAPPING_NAME);      // name of mapping object

        if (mapFile == nullptr)
        {
            return;
        }

        SetSenderRole();

        return;
    }
    ipcNewMessageEvent = OpenEventA(EVENT_ALL_ACCESS, FALSE, ipcNewMessageEventName);
    mapFile = OpenFileMapping(
                    FILE_MAP_ALL_ACCESS,   // read/write access
                    FALSE,                 // do not inherit the name
                    FILE_MAPPING_NAME);    // name of mapping object

    if (mapFile == nullptr)
    {
        return;
    }
    SetReceiverRole();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_closeBtn_clicked()
{
    if(!isReceiver)
    {
        PulseEvent(ipcRoleDetectionEvent);
    }
    CloseHandle(ipcRoleDetectionEvent);
    CloseHandle(ipcNewMessageEvent);
    CloseHandle(mapFile);
    QCoreApplication::quit();
}

void MainWindow::closeEvent (QCloseEvent *event)
{
    on_closeBtn_clicked();
}

void MainWindow::on_textEdit_textChanged()
{
    LPTSTR pBuf;
    pBuf = (LPTSTR) MapViewOfFile(mapFile,   // handle to map object
                         FILE_MAP_ALL_ACCESS, // read/write permission
                         0,
                         0,
                         BUF_SIZE);

    QString msg = ui->textEdit->document()->toPlainText();
    msg.toWCharArray(pBuf);
    //memcpy((PVOID)pBuf, (void*)msg.data(), (msg.size() * sizeof(TCHAR)) + 1);

    UnmapViewOfFile(pBuf);

    PulseEvent(ipcNewMessageEvent);
}

void MainWindow::Update()
{
    if(isRoleChanged)
    {
        if(isReceiver)
        {
            SetSenderRole();
        }
        else
        {
            SetReceiverRole();
        }
        isRoleChanged = false;
    }
    if(isReceiver && isNewMessageReceived)
    {
        LPTSTR pBuf;
        pBuf = (LPTSTR) MapViewOfFile(mapFile, // handle to map object
                       FILE_MAP_ALL_ACCESS,  // read/write permission
                       0,
                       0,
                       BUF_SIZE);
        ui->textEdit->document()->setPlainText(QString::fromWCharArray(pBuf));
        UnmapViewOfFile(pBuf);

    }
}

DWORD WINAPI ReceiverProc(CONST LPVOID lpParam)
{
    MainWindow *mw = (MainWindow*)lpParam;
    HANDLE events[2] = {mw->ipcNewMessageEvent, mw->ipcRoleDetectionEvent};

    DWORD triggeredEvent = 0;
    bool isAlive = true;
    while(isAlive)
    {
        triggeredEvent = WaitForMultipleObjects(2, events, FALSE, INFINITE);

        switch(triggeredEvent)
        {
        case WAIT_OBJECT_0:
            //read shared memory
            mw->isNewMessageReceived = true;
            break;
        case WAIT_OBJECT_0 + 1:
            //change role to sender
            mw->isRoleChanged = true;
            isAlive = false;
            break;
        }
    }
    ExitThread(0);
}

void MainWindow::SetReceiverRole()
{
    this->setWindowTitle("Receiver");
    ui->textEdit->setEnabled(false);
    isReceiver = true;
    CreateThread(nullptr, 0, &ReceiverProc, this, 0, nullptr);
}

void MainWindow::SetSenderRole()
{
    this->setWindowTitle("Sender");
    ui->textEdit->setEnabled(true);
    isReceiver = false;
}
