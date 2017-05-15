#include "mainWindow.h"
#include "ui_mainWindow.h"

MainWindow::MainWindow(QWidget *parent) :    QMainWindow(parent),    ui(new Ui::MainWindow)
{
    ui->setupUi(this); // Указателя ui дава достъп до елементите на графичния интерфейс.
    displayMode = TXT;
    rxCursorPosition = 0;
    sPort = new QSerialPort(this); // Указател през който ще се работи с порта. ВАЖНО: Погледни файла SerialDemo.pro!
    connect(sPort, SIGNAL(readyRead()), this, SLOT(readSerial()));
    connect(sPort, SIGNAL(bytesWritten(qint64)), this, SLOT(onBytesWritten(qint64)));

    //Прогрес бар показва изпращането на файл, който отнема повече време
    progBarSending = new QProgressBar(this);
    ui->statusBar->addWidget(progBarSending);
    progBarSending->setMaximum(5);
    progBarSending->setValue(0);
    progBarSending->setVisible(false);

    lblStatus = new QLabel(this);
    ui->statusBar->addWidget(lblStatus);
    lblStatus->setText(tr("Sending file..."));
    lblStatus->setVisible(false);

    //радиобутони за избор режима на изобразяване на приетите данни
    connect(ui->rbTxt, SIGNAL(clicked()), this, SLOT(displayModeChanged()));
    connect(ui->rbDec, SIGNAL(clicked()), this, SLOT(displayModeChanged()));
    connect(ui->rbHex, SIGNAL(clicked()), this, SLOT(displayModeChanged()));
}

MainWindow::~MainWindow()
{
    if(sPort->isOpen())  sPort->close();
    delete sPort;
    delete ui;
}

void MainWindow::on_pbOpenPort_clicked()
{
    /*
     * Натиснат е бутона "OPEN". Тази функция е SLOT и е генерирана автоматично,
     * като от редактора на графичен интерфейс върху бутона "CLOSE" с десен бутон
     * се отваря контекстното меню, избира се "Go to slot" и "clicked()".
     *
     * Настройват се параметрите на порта и се прави опит да се отвори.
     */

    // Указателя ui дава достъп до елементите на графичния интерфейс.
    QSerialPort::DataBits dataBits;
    QSerialPort::StopBits stopBits;
    QSerialPort::Parity parity;
    QSerialPort::BaudRate baud;

    QString portName = ui->linePortName->text().trimmed(); // Премахват се случайно добавени интервали в името. Няма проверка за валидност.

    switch(ui->spinDataBits->value())
    {
    case 7:
        dataBits = QSerialPort::Data7;
        break;
    case 8:
        dataBits = QSerialPort::Data8;
        break;
    default:
        dataBits = QSerialPort::Data8;
    }

    switch(ui->spinStopBits->value())
    {
    case 1:
        stopBits = QSerialPort::OneStop;
        break;
    case 2:
        stopBits = QSerialPort::TwoStop;
        break;
    default:
        stopBits = QSerialPort::TwoStop;
    }

    switch(ui->comboBaudrate->currentIndex())
    {
    case 0:
        baud = QSerialPort::Baud1200;
        break;
    case 1:
        baud = QSerialPort::Baud2400;
        break;
    case 2:
        baud = QSerialPort::Baud4800;
        break;
    case 3:
        baud = QSerialPort::Baud9600;
        break;
    case 4:
        baud = QSerialPort::Baud19200;
        break;
    case 5:
        baud = QSerialPort::Baud38400;
        break;
    case 6:
        baud = QSerialPort::Baud57600;
        break;
    case 7:
        baud = QSerialPort::Baud115200;
        break;
    default:
        baud = QSerialPort::Baud9600;
    }

    switch(ui->comboParity->currentIndex())
    {
    case 0:
        parity = QSerialPort::NoParity;
        break;
    case 1:
        parity = QSerialPort::OddParity;
        break;
    case 2:
        parity = QSerialPort::EvenParity;
        break;
    default:
        parity = QSerialPort::NoParity;
    }

    sPort->setPortName(portName);
    sPort->setDataBits(dataBits);
    sPort->setStopBits(stopBits);
    sPort->setParity(parity);
    sPort->setBaudRate(baud);

    bool openResult = sPort->open(QIODevice::ReadWrite);
    if(openResult)
    {
        //Порта е успешно отворен за четене и запис.
        //Блокираме бутона "OPEN", разблокираме бутона "CLOSE" и поставяме съобщение в StatusBar-a
        ui->pbOpenPort->setEnabled(false);
        ui->pbClosePort->setEnabled(true);
        ui->statusBar->showMessage(tr("Serial port is open!"));
    }
    else
    {
        //Порта не е отворен. Извеждаме диалог със съобщение за грешка и поставяме съобщение в StatusBar-a
        //Прочита се кода за грешка и се показва в диалога.
        int errCode = sPort->error();
        QString errReason;
        switch (errCode)
        {
        case 1:
            errReason = tr("DeviceNotFound");
            break;
        case 2:
            errReason = tr("PermissionError");
            break;
        default:
            errReason = QString("ErrorCode=%1").arg(errCode);
            break;
        }
        QMessageBox::critical(this, tr("ERROR!"), tr("Failed openning serial port!\nReason: ") + errReason);
        ui->statusBar->showMessage(tr("Serial port is closed!"));
    }
}

void MainWindow::on_pbClosePort_clicked()
{
    /*
     * Натиснат е бутона "CLOSE". Тази функция е SLOT и е генерирана автоматично,
     * като от редактора на графичен интерфейс върху бутона "CLOSE" с десен бутон
     * се отваря контекстното меню, избира се "Go to slot" и "clicked()".
     *
     * Затваряме порта, променяме състоянието на бутоните "OPEN" и "CLOSE" и и поставяме съобщение в StatusBar-a
     */

    if(sPort->isOpen())
    {
        sPort->close();
        ui->pbOpenPort->setEnabled(true);
        ui->pbClosePort->setEnabled(false);
        ui->statusBar->showMessage(tr("Serial port is closed!"));
    }
}

void MainWindow::on_pbWriteByte_clicked()
{
    //Изпраща един байт по сериен порт. По същия начин с QByteArray могат да се изпратят произволин брой байтове.
    QByteArray txBuf;

    if(!sPort->isOpen())
    {
        //Ако серииния порт не е отворен, вади предупреждение и излиза
        QMessageBox::warning(this, tr("WARNING"), tr("The serial port is not open!"));
        return;
    }

    txBuf[0] = (char) ui->spinWriteByte->value();
    sPort->write(txBuf);
    sPort->flush();
}

void MainWindow::on_pbWriteLine_clicked()
{
    //Изпраща низа от lineWriteLine като добавя и CR,LF
    QByteArray txBuf;

    if(!sPort->isOpen())
    {
        //Ако серииния порт не е отворен, вади предупреждение и излиза
        QMessageBox::warning(this, tr("WARNING"), tr("The serial port is not open!"));
        return;
    }

    QString line = ui->lineWriteLine->text();
    txBuf = line.toLatin1(); //toLatin1 е метод на класа QString и връща QByteArray с ASCII символите на низа.
    txBuf.append('\r');
    txBuf.append('\n');
    sPort->write(txBuf);
    sPort->flush();
}

void MainWindow::readSerial()
{
    //Този слот обслужва събитието QSerialPort::readyRead(). Извиква се когато има поне един байт получен по серен порт
    //Връзката SIGNASL-SLOT става в конструктора на MainWindow
    QByteArray rxBuf;
    QString str;

    rxBuf = sPort->readAll();
    switch(displayMode)
    {
    case TXT:
        ui->textBrowserRX->insertPlainText(QString(rxBuf));
        break;
    case HEX:
        for( int i=0 ; i!=rxBuf.count() ; i++)
        {
            str = QString::number(rxBuf[i],16);
            if(str.length()==1) str = "0" + str;
            ui->textBrowserRX->insertPlainText(str);
            rxCursorPosition++;
            if(rxCursorPosition==7) ui->textBrowserRX->insertPlainText("  ");
            else
                if(rxCursorPosition==15)
                {
                    ui->textBrowserRX->insertPlainText("\n");
                    rxCursorPosition = 0;
                }
                else ui->textBrowserRX->insertPlainText(" ");
        }
        break;
    case DEC:
        for( int i=0 ; i!=rxBuf.count() ; i++)
        {
            str = QString::number(rxBuf[i],10);
            if(str.length()==1) str = "  " + str;
            if(str.length()==2) str = " " + str;
            ui->textBrowserRX->insertPlainText(str);
            rxCursorPosition++;
            if(rxCursorPosition==7) ui->textBrowserRX->insertPlainText("  ");
            else
                if(rxCursorPosition==15)
                {
                    ui->textBrowserRX->insertPlainText("\n");
                    rxCursorPosition = 0;
                }
                else ui->textBrowserRX->insertPlainText(" ");
        }
        break;
    }
}

void MainWindow::onBytesWritten(qint64 count)
{
    qDebug() << " onBytesWritten count=" << count;

    //Придвижваме прогрес бара според броя изпратени байтове
    int bytesSent = progBarSending->value();
    int bytesSoFar = bytesSent + (int) count;
    progBarSending->setValue(bytesSent + (int) count);
    //Диагностично съобщение в конзолата
    qDebug() << "Sent " << bytesSoFar << "so far...";
    //Проверка дали е изпратен целия файл
    if(bytesSoFar==progBarSending->maximum())
    {
        qDebug() << "Sending file complete!";
        //Файла е изпратен. Скриваме progBarSending и lblStatus
        progBarSending->setVisible(false);
        lblStatus->setVisible(false);
        //Показваме съобщение в статус бара
        ui->statusBar->showMessage("Sending file complete!");
        //Бутоните за изпращане се разблокират
        ui->pbWriteByte->setEnabled(true);
        ui->pbWriteLine->setEnabled(true);
        ui->pbSendFile->setEnabled(true);
    }
}

void MainWindow::displayModeChanged()
{
    //Променя формата на показване на приетите данни
    if(ui->rbTxt->isChecked()) displayMode = TXT;
    else
        if(ui->rbHex->isChecked()) displayMode = HEX;
        else displayMode = DEC;

    ui->textBrowserRX->clear();
    rxCursorPosition = 0;

    qDebug() << "Display mode changed: " << displayMode;
}

void MainWindow::on_pbClearRx_clicked()
{
    //Изчиства приетите данни от textBrowserRX
    ui->textBrowserRX->clear();
}

void MainWindow::on_pbSelectFile_clicked()
{
    //Избира името на файла който да се прати по сериен порт
    QString fileName = QFileDialog::getOpenFileName(this, tr("Select file to send"), tr("."));
    ui->lineFileName->setText(fileName);
}

void MainWindow::on_pbSendFile_clicked()
{
    //В статус бара се показва прогрес бар за изпращаните данни. За времето на изпращане
    // някои бутони са блокирани.
    //При всяко изпразване на буфера на предаване се вика слота onBytesWritten()
    // Там се увеличава прогресбара и при приключване там се разблокират бутоните за изпращане

    if(!sPort->isOpen())
    {
        //Ако серииния порт не е отворен, вади предупреждение и излиза
        QMessageBox::warning(this, tr("WARNING"), tr("The serial port is not open!"));
        return;
    }

    //Изпраща избрания файл по сериен порт ако името е коректно. Иначе вади предупредителен диалог
    QFile file(ui->lineFileName->text());
    if(!file.exists())
    {
        QMessageBox::warning(this, tr("WARNING"), tr("Selected file doesn't exist!"));
        return;
    }

    bool result = file.open(QIODevice::ReadOnly);
    if(!result)
    {
        QMessageBox::critical(this, tr("ERROR"), tr("Failed openning the file for reading!"));
        return;
    }

    QByteArray txBuf = file.readAll();
    file.close();
    qDebug() << "File size is " << txBuf.count() << " bytes.";
    //Показваме прогрес бар и съобщение в статуса
    lblStatus->setVisible(true);
    progBarSending->setVisible(true);
    progBarSending->setMaximum(txBuf.count());
    progBarSending->setValue(0);
    //Бутоните за изпращане се блокират
    ui->pbWriteByte->setEnabled(false);
    ui->pbWriteLine->setEnabled(false);
    ui->pbSendFile->setEnabled(false);
    //започва изпращането
    sPort->write(txBuf);
    sPort->flush();
}
