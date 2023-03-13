# openbuffer
openbuffer 用于读写二进制流缓冲

```C++
#include <assert.h>
#include <string.h>
#include <string>
#include <vector>
#include "openbuffer.h"

using namespace open;

int main()
{
    OpenBuffer openBuffer;

    char data[256] = "Hello OpenBuffer!";
    const std::string str = "Hello OpenLinyou!";

    {
        uint32_t len = strlen(data);
        openBuffer.write(&len, sizeof(len));
        openBuffer.write(data, len);
    }
    {
        uint32_t len = str.size();
        openBuffer.writeUInt32(len);
        openBuffer.write(str.data(), len);
    }

    std::vector<char> vectData;
    uint32_t len = 0;
    openBuffer.read(&len, sizeof(len));
    vectData.resize(len);
    openBuffer.read(vectData.data(), len);

    assert(memcmp(vectData.data(), data, len) == 0);

    //
    char ret[256] = {};
    len = 0;
    openBuffer.readUInt32(len);
    openBuffer.read(ret, len);
    assert(str == ret);
    return 0;
}
```