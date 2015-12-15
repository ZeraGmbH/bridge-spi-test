#include <unistd.h>

#include <QIODevice>
#include <QByteArray>
#include <QFile>

#include "czerabridgespi.h"

CZeraBridgeSPI::CZeraBridgeSPI()
{
}

// taken from http://stackoverflow.com/questions/2602823/in-c-c-whats-the-simplest-way-to-reverse-the-order-of-bits-in-a-byte
static quint8 reverse(quint8 b) {
   b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
   b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
   b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
   return b;
}

QByteArray reverseArray(const QByteArray& data)
{
    QByteArray ret;
    for(int i=0; i<data.size(); i++)
        ret.append(reverse(data.at(i)));
    return ret;
}

bool CZeraBridgeSPI::BootLCA(QIODevice *pIODevice, const QString &strLCABootFileName, bool bSWLSBFirst)
{
    bool bOK = true;
    QFile fileFpgaBin(strLCABootFileName);
    if(fileFpgaBin.open(QIODevice::ReadOnly))
    {
        qint64 fileSize = fileFpgaBin.size();
        QByteArray data = fileFpgaBin.read(fileSize);
        if(bSWLSBFirst)
            data = reverseArray(data);
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
bool CZeraBridgeSPI::ExecCommand(QIODevice *pIODevice, BRIDGE_CMDS cmd, bool bSWLSBFirst, QByteArray *pParamData)
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
        if(bSWLSBFirst)
            m_SendData = reverseArray(m_SendData);
        bOK = pIODevice->write(m_SendData) == BRIDGE_SPI_FRAME_LEN;
        m_ReceiveData.clear();
        if(bOK)
        {
            if(bReadCmd)
            {
                /* Transfer 2: read data */
                m_ReceiveData = pIODevice->read(BRIDGE_SPI_FRAME_LEN);
                if(bSWLSBFirst)
                    m_ReceiveData = reverseArray(m_ReceiveData);
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

const QString CZeraBridgeSPI::sendDataAsHex()
{
    QString strRet;
    for(int iByte=0; iByte<m_SendData.size(); iByte++)
        strRet += QString::asprintf("%02X", m_SendData.at(iByte));
    return strRet;
}

const QString CZeraBridgeSPI::receiveDataAsHex()
{
    QString strRet;
    for(int iByte=0; iByte<m_ReceiveData.size(); iByte++)
        strRet += QString::asprintf("%02X", m_ReceiveData.at(iByte));
    return strRet;
}

