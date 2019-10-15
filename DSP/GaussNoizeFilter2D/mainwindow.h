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
    void on_genButton_clicked();

    void on_addRowButton_clicked();

    void on_delRowButton_clicked();

    void on_openButton_clicked();

    void on_clearButton_clicked();

    void on_noiseButton_clicked();

    void on_logCheckBox_stateChanged(int arg1);

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
