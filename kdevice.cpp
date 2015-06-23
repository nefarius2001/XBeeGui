#include "kdevice.h"

kDevice::kDevice()
{
    addr16=0;
    type=eTypeUndefined;
    lastreceived=QTime(0,0,0,0);
    flashcrc32=0;
}


kDevice::kDevice(uint16_t addr16_new)
{
    addr16=addr16_new;
    type=eTypeUndefined;
    lastreceived=QTime(0,0,0,0);
    flashcrc32=0;
}
