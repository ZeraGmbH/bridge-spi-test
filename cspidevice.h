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
};

#endif // CSPIDEVICE_H
