#include <sys/ioctl.h>
#include <linux/types.h>
#include <QString>
#include "cspidevice.h"

CSPIDevice::CSPIDevice(int bus, int channel)
{
    QString strFileName = QString("/dev/spidev%1.%2").arg(bus).arg(channel);
    setFileName(strFileName);
}

bool CSPIDevice::open(OpenMode flags)
{
    flags |= QIODevice::Unbuffered;
    qInfo("SPI opening %s...", qPrintable(fileName()));
    bool bOpen = exists() && QFile::open(flags);
    if(bOpen)
    {
        /* Check if we opened SPI by a simple ioctl check */
        __u8 dummy;
        __u32 dummy32;
        bOpen =
            ioctl(handle(), SPI_IOC_RD_MODE, &dummy) >= 0 &&
            ioctl(handle(), SPI_IOC_RD_LSB_FIRST, &dummy) >= 0 &&
            ioctl(handle(), SPI_IOC_RD_BITS_PER_WORD, &dummy) >= 0 &&
            ioctl(handle(), SPI_IOC_WR_MAX_SPEED_HZ, &dummy32) >= 0;

        if(!bOpen)
        {
            qCritical("%s does not support SPI ioctls!", qPrintable(fileName()));
            close();
        }
    }
    return bOpen;
}

void CSPIDevice::close()
{
    qInfo("SPI closing %s...", qPrintable(fileName()));
    return QFile::close();
}

bool CSPIDevice::setMode(quint8 Mode)
{
    qInfo("SPI set mode %u...", Mode);
    __u8 mode = 0;
    switch(Mode)
    {
    case 0:
        mode = SPI_MODE_0;
        break;
    case 1:
        mode = SPI_MODE_1;
        break;
    case 2:
        mode = SPI_MODE_2;
        break;
    case 3:
        mode = SPI_MODE_3;
        break;
    default:
        qCritical("SetMode: invalid mode %u!", Mode);
        return false;
    }

    if(!isOpen())
    {
        qCritical("SetMode: SPI device not open!");
        return false;
    }
    else
    {
        if(ioctl(handle(), SPI_IOC_WR_MODE, &mode) < 0)
            qCritical("CSPIDevice::SetMode failed!");
    }
    return true;
}

bool CSPIDevice::setLSBFirst(bool lsbFirst)
{
    qInfo("SPI LSBFirst %u...", lsbFirst);
    bool bOK = true;
    if(!isOpen())
    {
        qCritical("SetLSBFirst: SPI device not open!");
        bOK = false;
    }
    else
    {
        __u8 lsb = (__u8)lsbFirst;
        bOK = ioctl(handle(), SPI_IOC_WR_LSB_FIRST, &lsb) >= 0;
        if(!bOK)
            qCritical("CSPIDevice::SetLSBFirst failed!");
    }
    return bOK;
}

bool CSPIDevice::setBitsPerWord(quint8 bitsPerWord)
{
    qInfo("SPI bits per word %u...", bitsPerWord);
    bool bOK = true;
    if(!isOpen())
    {
        qCritical("SetBitsPerWord: SPI device not open!");
        bOK = false;
    }
    else
    {
        __u8 bits = (__u8)bitsPerWord;
        bOK = ioctl(handle(), SPI_IOC_WR_BITS_PER_WORD, &bits) >= 0;
        if(!bOK)
            qCritical("CSPIDevice::SetBitsPerWord failed!");
    }
    return bOK;
}

bool CSPIDevice::setBitSpeed(quint32 bitSpeedHz)
{
    qInfo("SPI bitspeed %u Hz...", bitSpeedHz);
    bool bOK = true;
    if(!isOpen())
    {
        qCritical("SetBitSpeed: SPI device not open!");
        bOK = false;
    }
    else
    {
        __u32 hz = (__u32)bitSpeedHz;
        bOK = ioctl(handle(), SPI_IOC_WR_MAX_SPEED_HZ, &hz) >= 0;
        if(!bOK)
            qCritical("CSPIDevice::SetBitSpeed failed!");
    }
    return bOK;
}
