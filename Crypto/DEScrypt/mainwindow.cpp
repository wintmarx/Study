#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "des.h"
#include <ctime>
#include <QFileDialog>

DES des;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    srand(time(NULL) | clock());
    ui->encryptButton->setEnabled(false);
    ui->decryptButton->setEnabled(false);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_generateKeyButton_clicked()
{
    ui->keyTextEdit->document()->setPlainText(QString::number(des.generateKey(), 16).toUpper());
    ui->encryptButton->setEnabled(true);
    ui->decryptButton->setEnabled(true);
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

void MainWindow::on_decryptButton_clicked()
{
    ui->encryptButton->setEnabled(false);
    ui->decryptButton->setEnabled(false);

    QString sourceText;
    uint32_t resultLength;
    QString string;
    uint64_t key;

    //decrypt------------------
    sourceText = ui->sourceTextEdit->document()->toPlainText();
    key = *((uint64_t*)str2Bytes(ui->keyTextEdit->document()->toPlainText()));
    char* decryptedText;
    resultLength = des.decrypt(str2Bytes(sourceText), &decryptedText, sourceText.size()/2, key);
    if(resultLength <= 0)
    {
        return;
    }
    string.clear();
    for(uint32_t i = 0; i < resultLength; i++)
    {
        string.append(decryptedText[i]);
    }
    ui->decryptedTextEdit->document()->setPlainText(string);
    //--------------------------

    //encrypt-------------------
    sourceText = ui->decryptedTextEdit->document()->toPlainText();
    char* encryptedText;
    resultLength = des.encrypt(sourceText.toLatin1().data(), &encryptedText, sourceText.size(), key);
    if(resultLength <= 0)
    {
        return;
    }
    string = bytes2Str(encryptedText, resultLength);
    ui->encryptedTextEdit->document()->setPlainText(string.toUpper());
    //--------------------------



    ui->encryptButton->setEnabled(true);
    ui->decryptButton->setEnabled(true);
}

void MainWindow::on_encryptButton_clicked()
{
    ui->encryptButton->setEnabled(false);
    ui->decryptButton->setEnabled(false);

    QString sourceText;
    uint32_t resultLength;
    QString string;
    uint64_t key;

    //encrypt--------------
    sourceText = ui->sourceTextEdit->document()->toPlainText();
    key = *((uint64_t*)str2Bytes(ui->keyTextEdit->document()->toPlainText()));
    char* encryptedText;
    resultLength = des.encrypt(sourceText.toLatin1().data(), &encryptedText, sourceText.size(), key);
    if(resultLength <= 0)
    {
        return;
    }
    string = bytes2Str(encryptedText, resultLength);
    ui->encryptedTextEdit->document()->setPlainText(string.toUpper());
    //---------------------

    //decrypt--------------
    sourceText = ui->encryptedTextEdit->document()->toPlainText();
    char* decryptedText;
    resultLength = des.decrypt(str2Bytes(sourceText), &decryptedText, resultLength, key);
    if(resultLength <= 0)
    {
        return;
    }
    string.clear();
    for(uint32_t i = 0; i < resultLength; i++)
    {
        string.append(decryptedText[i]);
    }
    ui->decryptedTextEdit->document()->setPlainText(string);
    //-----------------------

    ui->encryptButton->setEnabled(true);
    ui->decryptButton->setEnabled(true);
}

void MainWindow::on_loadFileButton_clicked()
{
    QString filePath = QFileDialog::getOpenFileName();

    QFile file(filePath);
    if(!file.open(QIODevice::ReadOnly)) {
        return;
    }

    QTextStream in(&file);
    QString text;
    while(!in.atEnd()) {
        text.append(in.readLine());
    }
    ui->sourceTextEdit->setPlainText(text);

    file.close();
}
