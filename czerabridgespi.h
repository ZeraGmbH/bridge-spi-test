#ifndef CZERASPIBRIDGE_H
#define CZERASPIBRIDGE_H

#include <QVector>
#include <QByteArray>
#include <QString>
#include <QIODevice>

enum BRIDGE_CMDS
{
    BRIDGE_CMD_READ_VERSION         = 0x00,
    BRIDGE_CMD_READ_PCB1            = 0x01,
    BRIDGE_CMD_READ_PCB2            = 0x02,
    BRIDGE_CMD_READ_DEVICE          = 0x03,
    BRIDGE_CMD_SETUP_RAM_ACCESS     = 0x10,
};

typedef QVector<qint16> TRam16Data;

class CZeraBridgeSPI
{
public:
    CZeraBridgeSPI();
    bool BootLCA(QIODevice *pIODevice, const QString& strLCABootFileName);
    bool ExecCommand(QIODevice *pIODevice, enum BRIDGE_CMDS cmd, QByteArray *pParamData = NULL);
    const QByteArray& GetSendRawData() { return m_SendRawData; }
    const QByteArray& GetReceiveRawData() { return m_ReceiveRawData; }
    bool WriteRam(QIODevice *pIODeviceCtl, QIODevice *pIODeviceData, const TRam16Data& data, const quint32 ui32Address);
    bool ReadRam(QIODevice *pIODeviceCtl, QIODevice *pIODeviceData, TRam16Data& data, const quint32 ui32Address, const quint32 ui32WordCount);
protected:
    QByteArray m_SendRawData;
    QByteArray m_ReceiveRawData;
};

#endif // CZERASPIBRIDGE_H
