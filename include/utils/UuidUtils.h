#ifndef __UUID_UTILS_H_
#define __UUID_UTILS_H_
#include "NacosString.h"

#define UUID_LEN_BYTES 16
class UuidUtils
{
private:
    static int fd;
public:
    static NacosString generateUuid();
    static void Init();
    static void DeInit();
};

#endif
