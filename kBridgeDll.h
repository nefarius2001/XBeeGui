#ifndef KBRIDGEDLL_H
#define KBRIDGEDLL_H

#include <stdint.h>
#include <QLibrary>

typedef  uint8_t   (*DmxGetOut_t)(uint8_t index);
typedef  uint8_t * (*DmxGetOutPointer_t)(void);
typedef  void   (*DmxSetOut_t)(uint8_t index, uint8_t value);

#define DmxDataFallback_NUMEL 512

class KBridgeDll
{
public:

    QLibrary * libKDmxBridge;
    DmxGetOutPointer_t DmxGetOutPointer;
    uint8_t *pDmxData;
    uint8_t   DmxDataFallback[DmxDataFallback_NUMEL];

    KBridgeDll();
};

#endif // KBRIDGEDLL_H
