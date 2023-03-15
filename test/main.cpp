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
    
    size_t len = strlen(data);
    openBuffer.push(&len, sizeof(len));
    openBuffer.push(data, len);
 
    len = str.size();
    openBuffer.pushUInt32(len);
    openBuffer.push(str.data(), len);

    openBuffer.pushUInt16(1616);
    openBuffer.pushUInt32(3232);
    openBuffer.pushUInt64(6464);

    openBuffer.pushVInt32(0x79);
    openBuffer.pushVInt32(0x80);
    openBuffer.pushVInt32(0x4000);
    openBuffer.pushVInt32(0x10000000);

    openBuffer.pushVInt64(0x10000001);

    std::vector<char> vectData;
    len = 0;
    openBuffer.pop(&len, sizeof(len));
    vectData.resize(len);
    openBuffer.pop(vectData.data(), len);
    assert(memcmp(vectData.data(), data, len) == 0);
    
    char ret[256] = {};
    uint32_t len1 = 0;
    openBuffer.popUInt32(len1);
    openBuffer.pop(ret, len1);
    assert(str == ret);
    
    unsigned short u16 = 0;
    openBuffer.popUInt16(u16);
    assert(u16 == 1616);

    uint32_t u32 = 0;
    openBuffer.popUInt32(u32);
    assert(u32 == 3232);
    uint64_t u64 = 0;
    openBuffer.popUInt64(u64);
    assert(u64 == 6464);


    uint64_t v32 = 0;
    openBuffer.popVInt64(v32);
    assert(v32 == 0x79);

    uint64_t v32_1 = 0;
    openBuffer.popVInt64(v32_1);
    assert(v32_1 == 0x80);

    uint64_t v32_2 = 0;
    openBuffer.popVInt64(v32_2);
    assert(v32_2 == 0x4000);

    uint64_t v32_3 = 0;
    openBuffer.popVInt64(v32_3);
    assert(v32_3 == 0x10000000);

    uint64_t v64 = 0;
    openBuffer.popVInt64(v64);
    assert(v64 == 0x10000001);
    
    return 0;
}