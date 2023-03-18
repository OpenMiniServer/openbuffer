/***************************************************************************
 * Copyright (C) 2023-, openlinyou, <linyouhappy@outlook.com>
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 ***************************************************************************/

#ifndef HEADER_OPEN_BUFFER_H
#define HEADER_OPEN_BUFFER_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <string>

#ifndef DATA_BIG_ENDIAN
#ifdef _BIG_ENDIAN_
#if _BIG_ENDIAN_
#define DATA_BIG_ENDIAN 1
#endif
#endif
#ifndef DATA_BIG_ENDIAN
#if defined(__hppa__) || \
            defined(__m68k__) || defined(mc68000) || defined(_M_M68K) || \
            (defined(__MIPS__) && defined(__MIPSEB__)) || \
            defined(__ppc__) || defined(__POWERPC__) || defined(_M_PPC) || \
            defined(__sparc__) || defined(__powerpc__) || \
            defined(__mc68000__) || defined(__s390x__) || defined(__s390__)
#define DATA_BIG_ENDIAN 1
#endif
#endif
#ifndef DATA_BIG_ENDIAN
#define DATA_BIG_ENDIAN  0
#endif
#endif

namespace open
{ 

class OpenBuffer 
{
	size_t size_;
	size_t offset_;
	size_t cap_;
	size_t miniCap_;
	unsigned char* buffer_;
public:
	OpenBuffer(size_t capacity = 256);
	~OpenBuffer();
	size_t inline size() { return size_; }
	unsigned char* data() { return buffer_ + offset_; }
	void clear();
	int64_t push(const void* data, size_t len);
	int64_t push(const std::string& data) 
	{ 
		return push(data.data(), data.size()); 
	}
	int64_t pop(void* data, size_t len);
	int64_t pop(std::string& data, size_t len) 
	{
		data.resize(len);
		return pop((void*)data.data(), data.size());
	};
	
	inline int64_t pushUInt8(unsigned char c)
	{
		return push(&c, 1);
	}

	inline int64_t popUInt8(unsigned char& c)
	{
		return pop(&c, 1);
	}

	inline int64_t pushUInt16(unsigned short w)
	{
		char p[2] = { 0 };
#if DATA_BIG_ENDIAN
		* (unsigned char*)(p + 0) = (w & 255);
		*(unsigned char*)(p + 1) = (w >> 8);
#else
		* (unsigned short*)(p) = w;
#endif
		return push(&p, sizeof(p));
	}

	inline int64_t popUInt16(unsigned short& w)
	{
		char p[2] = {0};
		int64_t ret = pop(p, sizeof(p));
		if (ret >= 0)
		{
#if DATA_BIG_ENDIAN
			w = *(const unsigned char*)(p + 1);
			w = *(const unsigned char*)(p + 0) + (*w << 8);
#else
			w = *(const unsigned short*)p;
#endif
		}
		return ret;
	}

	inline int64_t pushUInt32(uint32_t l)
	{
		char p[4] = { 0 };
#if DATA_BIG_ENDIAN
		* (unsigned char*)(p + 0) = (unsigned char)((l >> 0) & 0xff);
		*(unsigned char*)(p + 1) = (unsigned char)((l >> 8) & 0xff);
		*(unsigned char*)(p + 2) = (unsigned char)((l >> 16) & 0xff);
		*(unsigned char*)(p + 3) = (unsigned char)((l >> 24) & 0xff);
#else
		* (uint32_t*)p = l;
#endif
		return push(&p, sizeof(p));
	}

	inline int64_t popUInt32(uint32_t& l)
	{
		char p[4] = { 0 };
		int64_t ret = pop(p, sizeof(p));
		if (ret >= 0)
		{
#if DATA_BIG_ENDIAN
			l = *(const unsigned char*)(p + 3);
			l = *(const unsigned char*)(p + 2) + (*l << 8);
			l = *(const unsigned char*)(p + 1) + (*l << 8);
			l = *(const unsigned char*)(p + 0) + (*l << 8);
#else 
			l = *(const uint32_t*)p;
#endif
		}
		return ret;
	}

	inline int64_t pushUInt64(uint64_t l)
	{
		char p[8] = { 0 };
#if DATA_BIG_ENDIAN
		* (unsigned char*)(p + 0) = (unsigned char)((l >> 0) & 0xff);
		*(unsigned char*)(p + 1) = (unsigned char)((l >> 8) & 0xff);
		*(unsigned char*)(p + 2) = (unsigned char)((l >> 16) & 0xff);
		*(unsigned char*)(p + 3) = (unsigned char)((l >> 24) & 0xff);
		*(unsigned char*)(p + 0) = (unsigned char)((l >> 32) & 0xff);
		*(unsigned char*)(p + 1) = (unsigned char)((l >> 40) & 0xff);
		*(unsigned char*)(p + 2) = (unsigned char)((l >> 48) & 0xff);
		*(unsigned char*)(p + 3) = (unsigned char)((l >> 56) & 0xff);
#else
		* (uint64_t*)p = l;
#endif
		return push(&p, sizeof(p));
	}

	inline int64_t popUInt64(uint64_t& l)
	{
		char p[8] = { 0 };
		int64_t ret = pop(p, sizeof(p));
		if (ret >= 0)
		{
#if DATA_BIG_ENDIAN
			l = *(const unsigned char*)(p + 7);
			l = *(const unsigned char*)(p + 6) + (*l << 8);
			l = *(const unsigned char*)(p + 5) + (*l << 8);
			l = *(const unsigned char*)(p + 4) + (*l << 8);
			l = *(const unsigned char*)(p + 3) + (*l << 8);
			l = *(const unsigned char*)(p + 2) + (*l << 8);
			l = *(const unsigned char*)(p + 1) + (*l << 8);
			l = *(const unsigned char*)(p + 0) + (*l << 8);
#else 
			l = *(const uint64_t*)p;
#endif
		}
		return ret;
	}

	int64_t pushVInt32(const uint32_t& n);
	int64_t pushVInt64(const uint64_t& n);
	int64_t popVInt64(uint64_t& n);
};

};

#endif //HEADER_OPEN_BUFFER_H