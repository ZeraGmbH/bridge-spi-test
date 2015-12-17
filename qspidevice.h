#ifndef QSPIDevice_H
#define QSPIDevice_H

#include <linux/spi/spidev.h>
#include <QFile>

class QSPIDevicePrivate;

class QSPIDevice : public QFile
{
public:
    QSPIDevice(const QString & name);
    QSPIDevice(int bus, int channel);
    virtual bool open(OpenMode flags) Q_DECL_OVERRIDE;
    virtual void close() Q_DECL_OVERRIDE;
    virtual bool isSequential() const Q_DECL_OVERRIDE;
    bool setMode(quint8 Mode);
    bool setLSBFirst(bool lsbFirst);
    bool setBitsPerWord(quint8 bitsPerWord);
    bool setBitSpeed(quint32 bitSpeedHz);
protected:
    virtual qint64 readData(char *data, qint64 maxlen) Q_DECL_OVERRIDE;
    virtual qint64 writeData(const char *data, qint64 len) Q_DECL_OVERRIDE;
private:
    QSPIDevicePrivate *d_ptr;
    Q_DECLARE_PRIVATE(QSPIDevice)
};

#endif // QSPIDevice_H
