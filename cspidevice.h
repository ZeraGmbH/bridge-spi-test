#ifndef CSPIDEVICE_H
#define CSPIDEVICE_H

#include <linux/spi/spidev.h>
#include <QFile>

class CSPIDevice : public QFile
{
    Q_OBJECT
public:
    CSPIDevice(int bus, int channel);
    virtual bool open(OpenMode flags) Q_DECL_OVERRIDE;
    virtual void close() Q_DECL_OVERRIDE;

    bool setMode(quint8 Mode);
    bool setLSBFirst(bool lsbFirst);
    bool setBitsPerWord(quint8 bitsPerWord);
    bool setBitSpeed(quint32 bitSpeedHz);
protected:
    virtual qint64 readData(char *data, qint64 maxlen) Q_DECL_OVERRIDE;
    virtual qint64 writeData(const char *data, qint64 len) Q_DECL_OVERRIDE;
private:
    // At least beaglebo. does not support LSBFirst. We implement a software fallback
    bool m_bSWReverseRequired;
    quint8 m_u8LSBFirstOnOpen;
};

#endif // CSPIDEVICE_H
