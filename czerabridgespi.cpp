#include <unistd.h>

#include <QIODevice>
#include <QByteArray>
#include <QFile>

#include "czerabridgespi.h"

CZeraBridgeSPI::CZeraBridgeSPI()
{
}

bool CZeraBridgeSPI::BootLCA(QIODevice *pIODevice, const QString &strLCABootFileName)
{
    bool bOK = true;
    QFile fileFpgaBin(strLCABootFileName);
    if(fileFpgaBin.open(QIODevice::ReadOnly))
    {
        qint64 fileSize = fileFpgaBin.size();
        QByteArray data = fileFpgaBin.read(fileSize);
        qint64 sendSize = pIODevice->write(data);
        if(sendSize != fileSize)
        {
            qWarning("FPGA bootfile'%s' was not send completely (send %lli of %lli)!",
                      qPrintable(strLCABootFileName),
                      sendSize, fileSize);
            bOK = false;
        }
        else
        {
            qInfo("FPGA bootfile'%s' was send successfully (%lli bytes send)",
                      qPrintable(strLCABootFileName),
                      fileSize);

        }
        fileFpgaBin.close();
    }
    else
    {
        qWarning("FPGA bootfile'%s' could not be opened!", qPrintable(strLCABootFileName));
        bOK = false;
    }
    return bOK;
}

#define BRIDGE_SPI_FRAME_LEN 5

/* Note: kernel currently supports synchronous I/O only */
bool CZeraBridgeSPI::ExecCommand(QIODevice *pIODevice, BRIDGE_CMDS cmd, QByteArray *pParamData)
{
    bool bOK = true;
    if(!pIODevice->isOpen())
    {
        qWarning("ExecCommand: SPI device not open!");
        bOK = false;
    }
    else
    {
        bool bReadCmd;
        switch(cmd)
        {
        case BRIDGE_CMD_READ_VERSION:
        case BRIDGE_CMD_READ_PCB1:
        case BRIDGE_CMD_READ_PCB2:
        case BRIDGE_CMD_READ_DEVICE:
            bReadCmd = true;
            break;
        default:
            bReadCmd = false;
            break;
        }

        /* Transfer 1: read cmd / write */
        m_SendData.clear();
        /* cmd */
        m_SendData.append(bReadCmd ? (char)cmd | 0x80: (char)cmd);
        /* cmd param */
        for(int iParam=0; iParam<BRIDGE_SPI_FRAME_LEN-1;iParam++)
        {
            if(pParamData && pParamData->size() > iParam)
                m_SendData.append(pParamData->at(iParam));
            else
                m_SendData.append((char)0);
        }
        bOK = pIODevice->write(m_SendData) == BRIDGE_SPI_FRAME_LEN;
        m_ReceiveData.clear();
        if(bOK)
        {
            if(bReadCmd)
            {
                /* Transfer 2: read data */
                m_ReceiveData = pIODevice->read(BRIDGE_SPI_FRAME_LEN);
                bOK = m_ReceiveData.size() == BRIDGE_SPI_FRAME_LEN;
                if(!bOK)
                    qWarning("Reading command response was not completed!");
            }
        }
        else
            qWarning("Sending command was not completed!");
    }
    return bOK;
}
