#ifndef BLINKYTAPE_H
#define BLINKYTAPE_H

#include <QObject>
#include <QList>
#include <QtSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QColor>

/// Some defines that should go into a processor-specific class
#define FLASH_MEMORY_AVAILABLE 0x7000  // Amount of application space in the flash
#define FLASH_MEMORY_PAGE_SIZE 0x80    // Size of a page of memory in our flash

/// Connect to a BlinkyTape over a serial port, and manage sending data to it.
class BlinkyTape : public QObject
{
    Q_OBJECT
public:
    static QList<QSerialPortInfo> findBlinkyTapes();
    static QList<QSerialPortInfo> findBlinkyTapeBootloaders();
    static QRgb correctBrightness(QRgb uncorrected);

    BlinkyTape(QObject *parent = 0, int ledCount = 60);

    // TODO: Destructor!

    bool isConnected();

    bool open(QSerialPortInfo info);
    void close();

    void sendUpdate(QByteArray colors);

    bool getPortInfo(QSerialPortInfo &info);

    // Atempt to reset the strip by setting it's baud rate to 1200 and closing it.
    void reset();

private:
    QSerialPort* serial;
    int ledCount;

signals:
    void connectionStatusChanged(bool status);

private slots:
    void handleSerialError(QSerialPort::SerialPortError error);

};

#endif // BLINKYTAPE_H
