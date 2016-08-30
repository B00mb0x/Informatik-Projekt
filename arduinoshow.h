#ifndef ARDUINOSHOW_H
#define ARDUINOSHOW_H

#include <QWidget>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

namespace Ui {
class ArduinoShow;
}

class ArduinoShow : public QWidget
{
    Q_OBJECT

public:
    explicit ArduinoShow(QWidget *parent = 0);
    ~ArduinoShow();

private slots:
    void on_listButton_clicked();

    void on_connectButton_clicked();

    void readArduinoData();

    void on_disconnectButton_clicked();

    void handleErrors(QSerialPort::SerialPortError);

    void on_highscoreButton_clicked();

    void on_startButton_clicked();

    void on_clearButton_clicked();

private:
    Ui::ArduinoShow *ui;
    QSerialPort *serialPort;
    int scores[11];//Speichert 11 Punkte für die Highscoreliste
    QString name[11];//Speichert die Namen zu den Punkteständen
    QString userName= "";//Name des Spielers
};

#endif // ARDUINOSHOW_H
