#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QFile>
#include <QSPIDevice>
#include <QBridgeFmtSpiHelper>


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QCommandLineParser parser;
    parser.setApplicationDescription("ZERA Bridge SPI test application");
    parser.addHelpOption();

    /* --------------- define params --------------- */
    // option for spi-bus
    QCommandLineOption busOption(QStringList() << "b" << "bus", "SPI bus number", "bus-no");
    busOption.setDefaultValue("2");
    parser.addOption(busOption);

    // option for spi-bus-ram
    QCommandLineOption busOptionRAM(QStringList() << "B" << "BUS", "SPI bus number for RAM", "bus-no");
    busOptionRAM.setDefaultValue("1");
    parser.addOption(busOptionRAM);

    // option for spi-channel
    QCommandLineOption channelOption(QStringList() << "c" << "channel", "SPI channel number", "channel-no");
    channelOption.setDefaultValue("0");
    parser.addOption(channelOption);

    // option for spi-channel-ram
    QCommandLineOption channelOptionRAM(QStringList() << "C" << "CHANNEL", "SPI channel number for RAM", "channel-no");
    channelOptionRAM.setDefaultValue("0");
    parser.addOption(channelOptionRAM);

    // option for spi-speed
    QCommandLineOption speedOption(QStringList() << "s" << "speed", "SPI speed [Hz]", "speed");
    speedOption.setDefaultValue("16000000"); /* see BB-SPIDEVx-00A0.dts */
    parser.addOption(speedOption);

    // option for spi-mode
    QCommandLineOption modeOption(QStringList() << "m" << "mode", "SPI mode [0-3]", "mode-no");
    modeOption.setDefaultValue("3");
    parser.addOption(modeOption);

    // option for spi-bits-per-word
    QCommandLineOption bitsOption(QStringList() << "i" << "bitsperword", "SPI bits per word", "bitsperword");
    bitsOption.setDefaultValue("8");
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

    // option for hexadecimal transparent I/O
    QCommandLineOption transIOHexOption(QStringList() << "x" << "hexdata", "I/O hexadecimal data", "hexdata");
    parser.addOption(transIOHexOption);

    // option write data to RAM
    QCommandLineOption writeRAMHexOption(QStringList() << "w" << "write", "Write decimal data to RAM", "hexaddress");
    parser.addOption(writeRAMHexOption);

    // option read data to RAM
    QCommandLineOption readRAMHexOption(QStringList() << "r" << "read", "Read data from RAM", "hexaddress,hexwordlen");
    parser.addOption(readRAMHexOption);

    // option RAM data file
    QCommandLineOption ramDataOption(QStringList() << "d" << "datafilename", "Data Filename for RAM I/O", "filename");
    parser.addOption(ramDataOption);

    // option remote-debug
    QCommandLineOption remoteServerOption(QStringList() << "R" << "remote", "Connect remote device server", "IP:port");
    parser.addOption(remoteServerOption);

    /* --------------- extract params --------------- */
    parser.process(a);

    bool optValOK;
    bool optionsOK = true;

    QString strOptVal = parser.value(busOption);
    int spiBus = strOptVal.toInt(&optValOK);
    if(!optValOK)
    {
        qWarning("Invalid value for control SPI bus %s!\n", qPrintable(strOptVal));
        optionsOK = false;
    }

    strOptVal = parser.value(busOptionRAM);
    int spiBusRAM = strOptVal.toInt(&optValOK);
    if(!optValOK)
    {
        qWarning("Invalid value for RAM SPI bus %s!\n", qPrintable(strOptVal));
        optionsOK = false;
    }

    strOptVal = parser.value(channelOption);
    int spiChannel = strOptVal.toInt(&optValOK);
    if(!optValOK)
    {
        qWarning("Invalid value for control SPI channel %s!\n", qPrintable(strOptVal));
        optionsOK = false;
    }

    strOptVal = parser.value(channelOptionRAM);
    int spiChannelRAM = strOptVal.toInt(&optValOK);
    if(!optValOK)
    {
        qWarning("Invalid value for RAM SPI channel %s!\n", qPrintable(strOptVal));
        optionsOK = false;
    }

    strOptVal = parser.value(speedOption);
    quint32 spiSpeed = strOptVal.toULong(&optValOK);
    if(!optValOK)
    {
        qWarning("Invalid value for SPI speed %s!\n", qPrintable(strOptVal));
        optionsOK = false;
    }

    strOptVal = parser.value(modeOption);
    quint8 spiMode = strOptVal.toUShort(&optValOK);
    if(!optValOK)
    {
        qWarning("Invalid value for SPI mode %s!\n", qPrintable(strOptVal));
        optionsOK = false;
    }

    strOptVal = parser.value(bitsOption);
    quint8 spiBits = strOptVal.toUShort(&optValOK);
    if(!optValOK)
    {
        qWarning("Invalid value for SPI bits per word %s!\n", qPrintable(strOptVal));
        optionsOK = false;
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
    QString strSendHex = parser.value(transIOHexOption);
    QByteArray dataSendTrans;
    if(!strSendHex.isEmpty())
    {
        if(strSendHex.size() % 2 == 0)
        {
            bool bConversionOK = true;
            for(int iHex=0; iHex<strSendHex.size() && bConversionOK; iHex+=2)
            {
                QString strHexNum = strSendHex.mid(iHex, 2);
                quint8 u8Val = strHexNum.toShort(&bConversionOK, 16);
                if(bConversionOK)
                    dataSendTrans.append(u8Val);
            }
            if(!bConversionOK)
            {
                qWarning("Parameter %s is not a valid hexadecimal number!\n", qPrintable(strSendHex));
                optionsOK = false;
            }
        }
        else
        {
            qWarning("Hexadecimal number %s must have even length!\n", qPrintable(strSendHex));
            optionsOK = false;
        }
    }

    bool lsbFirst = parser.isSet(lsbOption);

    QString strWriteRAMHex = parser.value(writeRAMHexOption);
    quint32 u32WriteRAMAddr = 0;
    if(!strWriteRAMHex.isEmpty())
    {
        bool bConversionOK = true;
        u32WriteRAMAddr = strWriteRAMHex.toLong(&bConversionOK, 16);
        if(!bConversionOK)
        {
            qWarning("Parameter %s is not a valid hexadecimal address!\n", qPrintable(strWriteRAMHex));
            optionsOK = false;
        }

    }

    QString strReadRAMHex = parser.value(readRAMHexOption);
    quint32 u32ReadRAMAddr = 0;
    quint32 u32ReadRAMCount = 0;
    if(!strReadRAMHex.isEmpty())
    {
        QStringList ParamList = strReadRAMHex.split(QString(","));
        bool bConversionOK1 = true;
        bool bConversionOK2 = true;
        if(ParamList.count() == 2)
        {
            u32ReadRAMAddr = ParamList[0].toLong(&bConversionOK1, 16);
            u32ReadRAMCount = ParamList[1].toLong(&bConversionOK2, 10);
        }
        if(!bConversionOK1 || !bConversionOK2)
        {
            qWarning("Parameter %s is not a valid hexadecimal number!\n", qPrintable(strReadRAMHex));
            optionsOK = false;
        }

    }

    QString strRemoteServerAddressPort = parser.value(remoteServerOption);
    QString strRemoteIP;
    qint16 ui16Port = 0;
    bool bValidRemoteSetting = false;
    if(!strRemoteServerAddressPort.isEmpty())
    {
        bValidRemoteSetting = true;
        QStringList strList = strRemoteServerAddressPort.split(":");
        if(strList.size() == 2)
        {
            // IP
            if(!strList.at(0).isEmpty())
                strRemoteIP = strList.at(0);
            else
                strRemoteIP = "localhost";
            // Port
            ui16Port = strList.at(1).toInt(&bValidRemoteSetting);
        }
        else
            bValidRemoteSetting = false;

        if(!bValidRemoteSetting)
        {
            qWarning("Parameter %s is not a valid hexadecimal number!\n", qPrintable(strReadRAMHex));
            optionsOK = false;
        }
    }

    /* check for unknown arguments */
    if(parser.positionalArguments().size())
    {
        qWarning("Invalid command line parameters '%s'!\n", qPrintable(parser.positionalArguments().join(" ")));
        optionsOK = false;
    }

    if(!strWriteRAMHex.isEmpty() && !strReadRAMHex.isEmpty())
    {
        qWarning("RAM read and write is not possible at the same time!\n");
        optionsOK = false;
    }

    if(!optionsOK)
        parser.showHelp(-1);

    /* if something with cmd params went wrong, the application has exited here */

    /* prepare RAM input/output */
    QFile RamDataFile;
    TRam16Data RamIoData;

    if(!strWriteRAMHex.isEmpty() || !strReadRAMHex.isEmpty())
    {
        QString strRAMDataFileName = parser.value(ramDataOption);
        /* we have either read or write - checked above */
        bool bOpen = false;
        if(!strWriteRAMHex.isEmpty())
        {
            /* Write data -> read file */
            if(strRAMDataFileName.isEmpty())
                bOpen = RamDataFile.open(stdin, QIODevice::ReadOnly);
            else
            {
                RamDataFile.setFileName(strRAMDataFileName);
                bOpen = RamDataFile.open(QIODevice::ReadOnly);
            }
            if(bOpen)
            {
                /* parse input to write */
                char buf[1024];
                bool bParseOK = true;
                while(!RamDataFile.atEnd())
                {
                    RamDataFile.readLine(buf, sizeof(buf));
                    QString str(buf);
                    str=str.replace("\r", "").replace("\n", "").trimmed();
                    if(!str.isEmpty() && !str.startsWith("#"))
                    {
                        QStringList strList = str.split(" ");
                        for(int iVal=0; iVal<strList.count(); iVal++)
                        {
                            str = strList[iVal];
                            if(!str.isEmpty())
                            {
                                bool bNumberOK = false;
                                qint64 ui64Val = str.toInt(&bNumberOK);
                                if(bNumberOK)
                                {
                                    /* RAM is only 16 Bit wide */
                                    if(ui64Val >= -32768 && ui64Val <= 32767)
                                        RamIoData.append((qint16)ui64Val);
                                    else
                                    {
                                        qWarning("RAM data value out of range: \"%s\"!\n", qPrintable(str));
                                        bParseOK = false;
                                    }
                                }
                                else
                                {
                                    qWarning("RAM data value cannot be converted to decimal number: \"%s\"!\n", qPrintable(str));
                                    bParseOK = false;
                                }

                            }
                        }

                    }
                }
                RamDataFile.close();
                if(!bParseOK)
                    return -1;
            }
            else
            {
                qWarning("Could not open data input file\"%s\"!\n", strRAMDataFileName.isEmpty() ? "stdin" : qPrintable(strRAMDataFileName));
                return -1;
            }
        }
        else
        {
            /* Read data -> write file */
            if(strRAMDataFileName.isEmpty())
                bOpen = RamDataFile.open(stdout, QIODevice::WriteOnly);
            else
            {
                RamDataFile.setFileName(strRAMDataFileName);
                bOpen = RamDataFile.open(QIODevice::WriteOnly);
            }
            if(!bOpen)
            {
                qWarning("Could not open data output file\"%s\"!\n", strRAMDataFileName.isEmpty() ? "stdout" : qPrintable(strRAMDataFileName));
                return -1;
            }
        }
    }

    /* --------------- execute commands --------------- */

    // Now setup required objects
    if(bValidRemoteSetting)
        QSPIDevice::setRemoteServer(strRemoteIP, ui16Port);

    QSPIDevice spiDevice(spiBus, spiChannel);
    QBridgeFmtSpiHelper bridge;

    /* Open SPI cfor control */
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

    /* boot FPGA? */
    if(!strFpgaBootFileName.isEmpty())
    {
        if(!bridge.BootLCA(&spiDevice, strFpgaBootFileName))
            return -1;
    }

    /* write to RAM ? */
    if(!strWriteRAMHex.isEmpty())
    {
        QSPIDevice spiDeviceRAM(spiBusRAM, spiChannelRAM);
        if(!spiDeviceRAM.open(QIODevice::WriteOnly))
        {
            qWarning("Device %s could not be opened!\n", qPrintable(spiDeviceRAM.fileName()));
            return -1;
        }
        if(!spiDeviceRAM.setBitSpeed(spiSpeed))
            return -1;
        if(!spiDeviceRAM.setMode(spiMode))
            return -1;
        if(!spiDeviceRAM.setLSBFirst(lsbFirst))
            return -1;
        if(!spiDeviceRAM.setBitsPerWord(spiBits))
            return -1;

        if(!bridge.PrepareWriteRam(&spiDevice, u32WriteRAMAddr))
            return -1;
        if(!bridge.WriteRam(&spiDeviceRAM, RamIoData))
            return -1;

        spiDeviceRAM.close();
    }

    /* read from RAM ? */
    if(!strReadRAMHex.isEmpty())
    {
        QSPIDevice spiDeviceRAM(spiBusRAM, spiChannelRAM);
        if(!spiDeviceRAM.open(QIODevice::ReadOnly))
        {
            qWarning("Device %s could not be opened!\n", qPrintable(spiDeviceRAM.fileName()));
            return -1;
        }
        if(!spiDeviceRAM.setBitSpeed(spiSpeed))
            return -1;
        if(!spiDeviceRAM.setMode(spiMode))
            return -1;
        if(!spiDeviceRAM.setLSBFirst(lsbFirst))
            return -1;
        if(!spiDeviceRAM.setBitsPerWord(spiBits))
            return -1;

        if(!bridge.PrepareReadRam(&spiDevice, u32ReadRAMAddr))
            return -1;
        if(!bridge.ReadRam(&spiDeviceRAM, RamIoData, u32ReadRAMCount))
            return -1;

        if(RamDataFile.isOpen())
        {
            QTextStream outStream(&RamDataFile);
            for(quint32 ui32Word=0; ui32Word<u32ReadRAMCount; ui32Word++)
                outStream <<  QString::number(RamIoData[ui32Word]) + QString("\n");
            RamDataFile.close();
        }
        spiDeviceRAM.close();
    }

    /* Exec command ? */
    if(execCmd)
    {
        if(!bridge.ExecCommand(&spiDevice, (BRIDGE_CMDS)spiCmd))
            return -1;
        qInfo() << "Cmd Send: " << bridge.GetSendRawData().toHex().toUpper();
        qInfo() << "Cmd Receive: " << bridge.GetReceiveRawData().toHex().toUpper();
    }

    /* generic IO ? */
    if(dataSendTrans.size() > 0)
    {
        QByteArray dataReceive;
        if(!spiDevice.sendReceive(dataSendTrans, dataReceive))
            return -1;
        qInfo() << "Data Send: " << dataSendTrans.toHex().toUpper();
        qInfo() << "Data Receive: " << dataReceive.toHex().toUpper();
    }
    spiDevice.close();

    return 0;
}
