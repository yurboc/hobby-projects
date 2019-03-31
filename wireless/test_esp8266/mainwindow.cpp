#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    serial = new QSerialPort(this);
    setupUi(this);

    connect(serial, &QSerialPort::readyRead, this, &MainWindow::readData);
}

void MainWindow::readData()
{
    qint64 cnt = serial->bytesAvailable();
    leStatus->setText(QString("Bytes available: %1").arg(cnt));

    QByteArray rxData = serial->readAll();
    //rxData.replace("\r\n","{CRLF}");
    //rxData.replace("\r","{CR}");
    //rxData.replace("\n","{LF}");
    ptRxData->insertPlainText(rxData);
}

void MainWindow::on_pbConnect_clicked()
{
    serial->setPortName(leComPortName->text());
    serial->setBaudRate(QSerialPort::Baud115200);
    serial->setDataBits(QSerialPort::Data8);
    serial->setStopBits(QSerialPort::OneStop);
    serial->setParity(QSerialPort::NoParity);

    serial->open(QSerialPort::ReadWrite);
    if (serial->isOpen()) {
        leStatus->setText("Port opened");
    }
    else {
        leStatus->setText("Can't open port");
    }
}

void MainWindow::on_pbSend_clicked()
{
    QByteArray dataToSend(leTxText->text().toStdString().data());
    dataToSend.append("\r\n");
    serial->write(dataToSend);
}

void MainWindow::on_pbAt_clicked()
{
    ptRxData->clear();
    leTxText->setText("AT");
    on_pbSend_clicked();
}

void MainWindow::on_pbRst_clicked()
{
    ptRxData->clear();
    leTxText->setText("AT+RST");
    on_pbSend_clicked();
}

void MainWindow::on_pbCwjap_clicked()
{
    ptRxData->clear();
    leTxText->setText("AT+CWJAP?");
    on_pbSend_clicked();
}

void MainWindow::on_pbCwqap_clicked()
{
    ptRxData->clear();
    leTxText->setText("AT+CWQAP");
    on_pbSend_clicked();
}

void MainWindow::on_pbVer_clicked()
{
    ptRxData->clear();
    leTxText->setText("AT+GMR");
    on_pbSend_clicked();
}
