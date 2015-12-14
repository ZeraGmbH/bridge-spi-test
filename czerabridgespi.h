#ifndef CZERASPIBRIDGE_H
#define CZERASPIBRIDGE_H

class QByteArray;
class QString;
class QIODevice;

enum BRIDGE_CMDS
{
    BRIDGE_CMD_READ_VERSION = 0,
    BRIDGE_CMD_READ_PCB1 = 1,
    BRIDGE_CMD_READ_PCB2 = 2,
    BRIDGE_CMD_READ_DEVICE = 3,
};

class CZeraBridgeSPI
{
public:
    CZeraBridgeSPI();
    bool BootLCA(QIODevice *pIODevice, const QString& strLCABootFileName, bool bSWLSBFirst);
    bool ExecCommand(QIODevice *pIODevice, enum BRIDGE_CMDS cmd, bool bSWLSBFirst, QByteArray *pParamData = NULL);
    const QByteArray& sendData() { return m_SendData; }
    const QByteArray& receiveData() { return m_ReceiveData; }
    const QString sendDataAsHex();
    const QString receiveDataAsHex();
protected:
    QByteArray m_SendData;
    QByteArray m_ReceiveData;
};

#endif // CZERASPIBRIDGE_H
