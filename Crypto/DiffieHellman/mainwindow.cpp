#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "rsa.h"
#include <ctime>
#include <QFileDialog>

RSA rsa;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    srand(time(NULL) | clock());
}

bool isGenClicked = false;

MainWindow::~MainWindow()
{
    delete ui;
}

QString bytes2Str(char* bytes, int size) {
  char hex[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B','C','D','E','F'};
  QString str;
  for (int i = 0; i < size; ++i) {
    str.append(QChar(hex[(bytes[size - i - 1] & 0xF0) >> 4]));
    str.append(QChar(hex[ bytes[size - i - 1] & 0x0F      ]));
  }
  //qDebug("bytes2Str: bytes: %#llX", *((uint64_t*)bytes));
  //qDebug("bytes2Str: str: %s", str.toLatin1().data());
  return str;
}

char* str2Bytes(QString string)
{
    char *bytes = new char[string.size()/2];
    char tmp[3];
    tmp[2] = '\0';
    for(int i = 0; i < string.size()/2; i++)
    {
        tmp[0] = string.at(2 * i).toLatin1();
        tmp[1] = string.at(2 * i + 1).toLatin1();
        bytes[string.size()/2 - 1 - i] = strtol(tmp, nullptr, 16);
    }
    //qDebug("bytes2Str: bytes: %#llX", *((uint64_t*)bytes));
    //qDebug("str2Bytes: str: %s", string.toLatin1().data());
    return bytes;
}

//recv
void MainWindow::on_recvButton_clicked()
{
    if(ui->privEdit->text().isEmpty())
    {
        return;
    }

    DHKey r = rsa.recv(ui->privEdit->text().toUInt());
    if(r.n == 0)
    {
        return;
    }

    ui->nEdit->setText(QString::number(r.n));
    ui->gEdit->setText(QString::number(r.g));
    ui->keyEdit->setText(QString::number(r.priv));
    ui->recvEdit->setText(QString::number(r.pub));
}

//send
void MainWindow::on_sendButton_clicked()
{
    if(ui->nEdit->text().isEmpty() || ui->gEdit->text().isEmpty() || ui->privEdit->text().isEmpty())
    {
        return;
    }
    DHKey k;

    k.n = ui->nEdit->text().toUInt();
    k.g = ui->gEdit->text().toUInt();
    k.priv = ui->privEdit->text().toUInt();

    rsa.send(k);
}

void MainWindow::on_genButton_clicked()
{
    DHKey k = rsa.generateKeys();
    ui->nEdit->setText(QString::number(k.n));
    ui->gEdit->setText(QString::number(k.g));
    isGenClicked = true;
}
