#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDebug>
#include "cspidevice.h"
#include "czerabridgespi.h"


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QCommandLineParser parser;
    parser.setApplicationDescription("ZERA Bridge SPI test application");
    parser.addHelpOption();

    /* --------------- define params --------------- */
    // option for spi-bus
    QCommandLineOption busOption(QStringList() << "b" << "bus", "SPI bus number", "bus-no");
    parser.addOption(busOption);

    // option for spi-channel
    QCommandLineOption channelOption(QStringList() << "c" << "channel", "SPI channel number", "channel-no");
    parser.addOption(channelOption);

    // option for spi-speed
    QCommandLineOption speedOption(QStringList() << "s" << "speed", "SPI speed [Hz]", "speed");
    parser.addOption(speedOption);

    // option for spi-mode
    QCommandLineOption modeOption(QStringList() << "m" << "mode", "SPI mode [0-3]", "mode-no");
    parser.addOption(modeOption);

    // option for spi-bits-per-word
    QCommandLineOption bitsOption(QStringList() << "i" << "bitsperword", "SPI bits per word", "bitsperword");
    parser.addOption(bitsOption);

    // (boolean) option for spi-mode
    QCommandLineOption lsbOption(QStringList() << "l" << "lsbfirst", "SPI LSB first instead of MSB first");
    parser.addOption(lsbOption);

    // option for fpga-boot file
    QCommandLineOption fpgaBootOption(QStringList() << "f" << "fpgabootfile", "FPGA boot file name", "filename");
    parser.addOption(fpgaBootOption);

    // option for spi-command
    QCommandLineOption cmdOption(QStringList() << "o" << "command", "Bridge SPI-command number", "command-no");
    parser.addOption(cmdOption);

    /* --------------- extract params --------------- */
    parser.process(a);

    bool optValOK;
    bool optionsOK = true;

    int spiBus = 1;
    QString strOptVal = parser.value(busOption);
    if(!strOptVal.isEmpty())
    {
        int iVal = strOptVal.toInt(&optValOK);
        if(optValOK)
            spiBus = iVal;
        else
        {
            qWarning("Invalid value for SPI bus %s!\n", qPrintable(strOptVal));
            optionsOK = false;
        }
    }

    int spiChannel = 0;
    strOptVal = parser.value(channelOption);
    if(!strOptVal.isEmpty())
    {
        int iVal = strOptVal.toInt(&optValOK);
        if(optValOK)
            spiChannel = iVal;
        else
        {
            qWarning("Invalid value for SPI channel %s!\n", qPrintable(strOptVal));
            optionsOK = false;
        }
    }

    quint32 spiSpeed = 16000000; /* see BB-SPIDEVx-00A0.dts */
    strOptVal = parser.value(speedOption);
    if(!strOptVal.isEmpty())
    {
        quint32 u32Val = strOptVal.toULong(&optValOK);
        if(optValOK)
            spiSpeed = u32Val;
        else
        {
            qWarning("Invalid value for SPI speed %s!\n", qPrintable(strOptVal));
            optionsOK = false;
        }
    }

    quint8 spiMode = 3;
    strOptVal = parser.value(modeOption);
    if(!strOptVal.isEmpty())
    {
        quint8 u8Val = strOptVal.toUShort(&optValOK);
        if(optValOK)
            spiMode = u8Val;
        else
        {
            qWarning("Invalid value for SPI mode %s!\n", qPrintable(strOptVal));
            optionsOK = false;
        }
    }

    quint8 spiBits = 8;
    strOptVal = parser.value(bitsOption);
    if(!strOptVal.isEmpty())
    {
        quint8 u8Val = strOptVal.toUShort(&optValOK);
        if(optValOK)
            spiBits = u8Val;
        else
        {
            qWarning("Invalid value for SPI bits per word %s!\n", qPrintable(strOptVal));
            optionsOK = false;
        }
    }

    bool execCmd = false;
    int spiCmd = 0;
    strOptVal = parser.value(cmdOption);
    if(!strOptVal.isEmpty())
    {
        int iVal = strOptVal.toInt(&optValOK);
        if(optValOK)
        {
            spiCmd = iVal;
            execCmd = true;
        }
        else
        {
            qWarning("Invalid value for SPI command %s!\n", qPrintable(strOptVal));
            optionsOK = false;
        }
    }

    QString strFpgaBootFileName = parser.value(fpgaBootOption);

    bool lsbFirst = parser.isSet(lsbOption);

    /* check for unknown arguments */
    if(parser.positionalArguments().size())
    {
        qWarning("Invalid command line parameters '%s'!\n", qPrintable(parser.positionalArguments().join(" ")));
        optionsOK = false;
    }

    if(!optionsOK)
        parser.showHelp(-1);

    /* if something with cmd params went wrong, the application has exited here */

    /* --------------- execute commands --------------- */
    /*return a.exec();*/

    // Now setup required objects
    CSPIDevice spiDevice(spiBus, spiChannel);
    CZeraBridgeSPI bridge;

    if(!spiDevice.open(QIODevice::ReadWrite))
    {
        qWarning("Device %s could not be opened!\n", qPrintable(spiDevice.fileName()));
        return -1;
    }
    if(!spiDevice.setBitSpeed(spiSpeed))
        return -1;
    if(!spiDevice.setMode(spiMode))
        return -1;
    if(!spiDevice.setLSBFirst(lsbFirst))
        return -1;
    if(!spiDevice.setBitsPerWord(spiBits))
        return -1;
    if(!strFpgaBootFileName.isEmpty())
    {
        if(!bridge.BootLCA(&spiDevice, strFpgaBootFileName))
            return -1;
    }
    if(execCmd)
    {
        if(!bridge.ExecCommand(&spiDevice, (BRIDGE_CMDS)spiCmd))
            return -1;
        qInfo() << "Cmd Send: " << bridge.sendDataAsHex();
        qInfo() << "Cmd Receive: " << bridge.receiveDataAsHex();
    }
    spiDevice.close();

    return 0;
}
