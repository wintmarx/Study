#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <windows.h>
#include <QTimer>

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
    void on_closeBtn_clicked();

    void on_textEdit_textChanged();

    void Update();
    void closeEvent(QCloseEvent *event);

private:
    Ui::MainWindow *ui;
public:
    void SetReceiverRole();
    void SetSenderRole();
    HANDLE ipcRoleDetectionEvent;
    const char *ipcRoleDetectionEventName = "ipcRoleDetectionEvent";
    HANDLE ipcNewMessageEvent;
    const char *ipcNewMessageEventName = "ipcNewMessageEvent";
    bool isRoleChanged = false;
    bool isReceiver = false;
    bool isNewMessageReceived = false;

    HANDLE mapFile;

    QTimer *updateTimer;
};

#endif // MAINWINDOW_H
