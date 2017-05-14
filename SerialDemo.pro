#-------------------------------------------------
#
# Project created by QtCreator 2017-05-12T14:19:43
#
#-------------------------------------------------

QT       += core gui

QT      += serialport       # Това трябва да се добави за да може да се ползва класа QSerialPort.

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SerialDemo
TEMPLATE = app


SOURCES += main.cpp\
        mainWindow.cpp

HEADERS  += mainWindow.h

FORMS    += mainWindow.ui

RESOURCES += \
    res.qrc

#Икона на приложението - този файл е текстов и съдържа един ред в който се указва път към файла с иконата
RC_FILE = sdemoIcon.rc

OTHER_FILES += \
    sdemoIcon.rc
