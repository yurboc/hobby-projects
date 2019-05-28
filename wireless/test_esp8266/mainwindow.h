#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_mainwindow.h"
#include <QtSerialPort/QtSerialPort>

#include "atcommandparser.h"
#include "atcommandsender.h"

class MainWindow : public QMainWindow, private Ui::MainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
private slots:
    void readData();
    void handleState(QString stateText);
    void on_pbConnect_clicked();
    void on_pbSend_clicked();
    void on_pbAt_clicked();
    void on_pbRst_clicked();
    void on_pbCwjap_clicked();
    void on_pbCwqap_clicked();
    void on_pbVer_clicked();
    void on_pbStart_clicked();

private:
    void sendDataToEsp8266(QByteArray data, bool addCrLf = false);
    void sendCommandByState(QString state);

    QSerialPort *m_serial;
    AtCommandParser m_commandParser;
    AtCommandSender m_commandSender;
};

#endif // MAINWINDOW_H
