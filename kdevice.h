#ifndef KDEVICE_H
#define KDEVICE_H
#include <QMap>
#include <QVariant>
#include <QTime>
#include <stdint.h>

class kDevice
{
public:
    kDevice();
    kDevice(uint16_t addr16_new);
    QMap<QString,QVariant> values;
    QMap<QString,QTime>    valueReceiveTimes;
    QTime lastreceived;
    int type;
    uint16_t addr16;
    QByteArray flash;
    quint32 flashcrc32;

    enum eeType{
        eTypeUndefined,
        eTypeForgotten,
        eTypeStm32F373
    };
};

#endif // KDEVICE_H
