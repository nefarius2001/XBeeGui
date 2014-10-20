#include "kdevice.h"

kDevice::kDevice()
{
    addr16=0;
    type=eTypeUndefined;
    lastreceived=QTime(0,0,0,0);
    flashcrc32=0;
}

