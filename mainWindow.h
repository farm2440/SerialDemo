#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QDebug>
#include <QtSerialPort/QSerialPort>
#include <QMessageBox>              // Диалози за съобщения
#include <QByteArray>               //Този клас се ползва за буфери

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
    void on_pbOpenPort_clicked();

    void on_pbClosePort_clicked();

    void on_pbWriteByte_clicked();

    void on_pbWriteLine_clicked(); // Тези 4 слота са генерирани автоматично от редактора на графичния интерфейс

    void readSerial(); //Този слот обслужва събитието QSerialPort::readyRead(). Извиква се когато има поне един байт получен по серен порт

    void displayModeChanged(); //Този слод се вика от промяна на радиобутоните TXT/HEX/DEC

private:
    Ui::MainWindow *ui;

    QSerialPort *sPort;

    enum DisplayMode {TXT, HEX, DEC} displayMode;
    int rxCursorPosition; // Помощна променлива за изобразяване на приетите данни в textBrowserRX
};

#endif // MAINWINDOW_H
