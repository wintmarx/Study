#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <antennacell.h>
#include <vector>

typedef std::vector<AntennaCell*> AntennasGrid;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void GenerateGrid(AntennasGrid &grid, double w, double h, uint cellsx, uint cellsy);
    Ui::MainWindow *ui;
    AntennasGrid grid;
};

#endif // MAINWINDOW_H
