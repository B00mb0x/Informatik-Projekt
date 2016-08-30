//die headerdateien einbinden
#include "arduinoshow.h"
#include "ui_arduinoshow.h"
#include <QThread>

ArduinoShow::ArduinoShow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ArduinoShow)
{
    ui->setupUi(this);
    serialPort = new QSerialPort(this);

    // setzt die Punkte der Highscoreliste am anfang auf 0
    for (int i = 0; i<11; i++){
        scores[i]=0;
    }

    connect(serialPort, SIGNAL(error(QSerialPort::SerialPortError)), this, SLOT(handleError(QSerialPort::SerialPortError)));
    connect(serialPort, SIGNAL(readyRead()), this, SLOT(readArduinoData()));
}

ArduinoShow::~ArduinoShow()
{
    delete ui;
}

void ArduinoShow::on_listButton_clicked()
{
    //Zeigt verfügbare Ports im Textbrowser an
    QList<QSerialPortInfo> portInfoList = QSerialPortInfo::availablePorts();
    for (int i =0; i<portInfoList.size(); i++){
        QSerialPortInfo &info = portInfoList[i];
        ui->textBrowser->append(info.portName());
    }
}

void ArduinoShow::on_connectButton_clicked()
{
   //verbindet sich mit dem eingegebenen port
   serialPort->setPortName(ui->textEdit->toPlainText());
   serialPort->setBaudRate(QSerialPort::Baud9600);
   serialPort->setDataBits(QSerialPort::Data8);
   serialPort->setParity(QSerialPort::NoParity);
   serialPort->setStopBits(QSerialPort::OneStop);
   serialPort->setFlowControl(QSerialPort::NoFlowControl);

   //Meldet wenn das Programm mit dem Arduino verbunden ist
   if (serialPort->open(QIODevice::ReadWrite)){
      ui->textBrowser->append("Connected");
   //Gibt eine Fehlermeldung aus, wenn das Programm sich nicht mit dem Arduino verbinden kann
   }else{
      ui->textBrowser->append("Failed");
   }
}

void ArduinoShow::readArduinoData(){
    //liest den Score vom Arduino und zeigt ihn im TextBrowser an
    QByteArray data = serialPort->readAll();
    QString str=data;
    scores[10]= str.toInt();
    ui->textBrowser->append(QString::number(scores[10]));

    //sortiert den neuen score in die Highscoreliste (einfach, da die Liste vorsortiert ist)
    name[10]=userName;
    for(int i=0; i<10; i++){
        if(scores[i]<=scores[10]){
            int temp =scores[i];
            scores[i]=scores[10];
            scores[10]= temp;
            QString tempName = name[i];
            name[i]=name[10];
            name[10]= tempName;
        }
    }
}


void ArduinoShow::on_disconnectButton_clicked()
{
    //trennt die Verbindung zum Arduino
    serialPort->close();
    ui->textBrowser->append("closed");
}

void ArduinoShow::handleErrors(QSerialPort::SerialPortError error){
    //Gibt Fehlermeldung bei Problemen in der Seriellen Kommunikation aus
    if(error == QSerialPort::ResourceError){
    serialPort->close();
    ui->textBrowser->append("Error");}

    }

void ArduinoShow::on_highscoreButton_clicked()
{
    //Zeigt die Highscoreliste an und gibt bei nicht eingegebenen Namen "Unbekannter Name" aus
    ui->textBrowser->append("Highscores:");
    for(int i=0; i<10; i++){
        if(name[i]== ""){
            name[i]= "Unbekannter Name";
        }
        ui->textBrowser->append(QString::number(scores[i]) + " " + name[i]);
    }
}

void ArduinoShow::on_startButton_clicked()
{
    //Schickt signal an den Arduino zum starten des Programms
    userName = ui->nameEdit->toPlainText();//liest den Spielernamen aus
    QByteArray data = "1";
    serialPort->write(data);
}

void ArduinoShow::on_clearButton_clicked()
{
    //Löscht den Text im TextBrowser
    ui->textBrowser->clear();
}
