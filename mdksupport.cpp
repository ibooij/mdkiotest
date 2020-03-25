#include "mdksupport.h"
#include "mdklocalfileio.h"

void registerMediaIoClasses()
{
    MdkLocalFileIO::registerOnce();
}
