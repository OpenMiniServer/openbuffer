# OpenBuffer
OpenBuffer 用于读写解析二进制流，解决TCP粘包问题，针对网络数据流而设计。

**OpenLinyou开源项目致力于跨平台服务器框架，在VS或者XCode上写代码，无需任何改动就可以编译运行在Linux上，甚至是安卓和iOS.**
OpenLinyou：https://github.com/openlinyou
https://gitee.com/linyouhappy

## 跨平台支持
Windows、linux、Mac、iOS、Android等跨平台设计

## 编译和执行
请安装cmake工具，用cmake可以构建出VS或者XCode工程，就可以在vs或者xcode上编译运行。
源代码：https://github.com/openlinyou/openbuffer
https://gitee.com/linyouhappy/openbuffer
```
git clone https://github.com/openlinyou/openbuffer
cd ./openbuffer
mkdir build
cd build
cmake ..
#如果是win32，在该目录出现openbuffer.sln，点击它就可以启动vs写代码调试
make
./test
```

## 全部源文件
+ src/openbuffer.h
+ src/openbuffer.cpp

## 技术特点
OpenBuffer的技术特点：
1. 对于添加数据push操作，使用节点链表来管理内存。每次push数据，数据存储到一个节点上，超过节点容量，就会创建新的节点进行存储。
2. 从OpenBuffer读取数据，先把全部节点上的数据进行合并，存储到连续的内存上，然后释放对应节点的内存。
3. 支持读写固定长度整数和不固定长度整数。

## 1.序列化与反序列化
支持整数，可变长整数序列化。采用小端编码。
```C++
#include <assert.h>
#include <string.h>
#include <string>
#include <vector>
#include "openbuffer.h"

using namespace open;

int main()
{
    OpenBuffer openBuffer(256);

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
```

## 2.解析网络数据包
模拟socket数据包，解析http报文。解决HTTP沾包问题。
```C++
#include <assert.h>
#include <string.h>
#include <string>
#include <vector>
#include "openbuffer.h"

using namespace open;

int main()
{
    std::vector<std::string> datas = {
        "HTTP/1.1 200 OK@&Connection: keep-alive@&Content-Type: application/x-javascript@&",
        "Date: Sat, 18 Mar 2023 08:11:44 GMT@&Strict-Transport-Security: max-age=31536000@&Traceco",
        "de: 24764974122629742602031816@&Vary: Accept-Encoding@&"
    };
    std::string body = "Hello OpenBuffer!!Hello OpenBuffer!!";
    datas.push_back("content-length:" + std::to_string(body.size()) + "@&");
    datas.push_back("@&" + body);
    datas.push_back("@&");
    OpenBuffer openBuffer;
    for (size_t x = 0; x < 10000; x++)
    {
        openBuffer.clear();
        bool isHeader = true;
        size_t k = 0;
        std::string head;
        for (size_t i = 0; i < datas.size(); ++i)
        {
            openBuffer.push(datas[i].data(), datas[i].size());
            if (isHeader)
            {
                unsigned char* tmp = openBuffer.data();
                for (; k < openBuffer.size() - 3; k++)
                {
                    //find @&@&
                    if (tmp[k] == '@' && tmp[k + 1] == '&' && tmp[k + 2] == '@' && tmp[k + 3] == '&')
                        break;
                }
                if (k >= openBuffer.size() - 3) continue;

                k += 4;
                openBuffer.pop(head, k);
                isHeader = false;
            }
        }
        std::string test = body + "@&";
        std::string buffer;
        buffer.append((const char*)openBuffer.data(), openBuffer.size());
        assert(test == buffer);
    }
    return 0;
}
```