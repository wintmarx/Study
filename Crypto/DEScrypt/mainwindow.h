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

    void on_decryptButton_clicked();

    void on_encryptButton_clicked();

    void on_MainWindow_iconSizeChanged(const QSize &iconSize);

    void on_loadFileButton_clicked();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
