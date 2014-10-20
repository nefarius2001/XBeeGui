#ifndef FLASHBUFFER_H
#define FLASHBUFFER_H

#include <QByteArray>
#include <QString>
#include <stdint.h>

class Flashbuffer
{
public:
    Flashbuffer(int iOffset, int iSize);
    QByteArray buffer;
    qint64 offset;
    qint64 size;
    quint32 GetCrc();
    int readIntelHex(QString sFile);
private:
    quint32 Crc32    (quint32 Crc, quint32 Data);
    quint32 Crc32Fast(quint32 Crc, quint32 Data);

};

#endif // FLASHBUFFER_H
