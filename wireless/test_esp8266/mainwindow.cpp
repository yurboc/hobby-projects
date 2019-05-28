#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    m_serial = new QSerialPort(this);
    setupUi(this);

    connect(m_serial, &QSerialPort::readyRead, this, &MainWindow::readData);
    connect(&m_commandParser, &AtCommandParser::gotNewState,
            this, &MainWindow::handleState);

    m_commandSender.addCommand("RST", "AT+RST");
    m_commandSender.addCommand("AT", "AT");
    m_commandSender.addCommand("VERSION", "AT+VERSION");
    m_commandSender.addCommand("STAMAC", "AT+CIPSTAMAC");

    m_commandSender.addTransition(ACTION_READY, "RST", "AT");
    m_commandSender.addTransition(ACTION_OK, "AT", "VERSION");
    m_commandSender.addTransition(ACTION_OK, "VERSION", "STAMAC");
}

void MainWindow::readData()
{
    qint64 cnt = m_serial->bytesAvailable();
    leStatus->setText(QString("Bytes available: %1").arg(cnt));

    QByteArray rxData = m_serial->readAll();
    ptRxData->insertPlainText(rxData);

    // Put data to Parser
    for (QByteArray::const_iterator it = rxData.begin(); it != rxData.end(); it++) {
        m_commandParser.putData(*it);
    }
}

void MainWindow::handleState(QString stateText)
{
    // Show state
    leParserMode->setText(stateText);

    // Send next AT command
    sendCommandByState(stateText);
}

void MainWindow::on_pbConnect_clicked()
{
    m_serial->setPortName(leComPortName->text());
    m_serial->setBaudRate(QSerialPort::Baud115200);
    m_serial->setDataBits(QSerialPort::Data8);
    m_serial->setStopBits(QSerialPort::OneStop);
    m_serial->setParity(QSerialPort::NoParity);

    m_serial->open(QSerialPort::ReadWrite);
    if (m_serial->isOpen()) {
        leStatus->setText("Port opened");
    }
    else {
        leStatus->setText("Can't open port");
    }
}

void MainWindow::on_pbSend_clicked()
{
    QByteArray dataToSend(leTxText->text().toStdString().data());
    sendDataToEsp8266(dataToSend, true);
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

void MainWindow::on_pbStart_clicked()
{
    m_commandParser.reset();
    on_pbConnect_clicked();
    on_pbRst_clicked();
}

void MainWindow::sendDataToEsp8266(QByteArray data, bool addCrLf)
{
    if (addCrLf)
        data.append("\r\n");

    m_serial->write(data);
}

void MainWindow::sendCommandByState(QString state)
{
    QByteArray data;
    if (state == "READY") {
        data = "AT+CWQAP";
        m_commandParser.setPendingCommand("CWQAP");
        sendDataToEsp8266(data, true);
    }
    else if (state == "CWQAP") {
        data = "AT+CIPMUX=1";
        m_commandParser.setPendingCommand("CIPMUX");
        sendDataToEsp8266(data, true);
    }
    else if (state == "CIPMUX") {
        data = "AT+CWJAP_CUR=\"TestSSID\",\"TestPASSWD\"";
        m_commandParser.setPendingCommand("CWJAP"); // expected: GOT IP
        sendDataToEsp8266(data, true);
    }
    else if (state == "CWJAP") {
        data = "AT+CIFSR";
        m_commandParser.setPendingCommand("CIFSR");
        sendDataToEsp8266(data, true);
    }
    else if (state == "CIFSR") {
        data = "AT+CIPSERVER=1,80";
        m_commandParser.setPendingCommand("SERVER");
        sendDataToEsp8266(data, true);
    }
}
