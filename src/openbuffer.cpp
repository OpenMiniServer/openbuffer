/***************************************************************************
 * Copyright (C) 2023-, openlinyou, <linyouhappy@outlook.com>
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 ***************************************************************************/

#include "openbuffer.h"
#include <assert.h>
#include <string.h>
#include <stdlib.h>


namespace open
{

OpenBuffer::OpenBuffer(size_t capacity)
	:size_(0),
	offset_(0),
	cap_(0),
	miniCap_(capacity),
	buffer_(NULL)
{
}

OpenBuffer::~OpenBuffer()
{
	clear();
}

unsigned char* OpenBuffer::data() 
{ 
	if (offset_ + size_ <= cap_)
	{
		buffer_[offset_ + size_] = 0;
	}
	else
	{
		buffer_[cap_] = 0;
		assert(false);
	}
	return buffer_ + offset_; 
}

void OpenBuffer::clear()
{
	size_    = 0;
	offset_  = 0;
	cap_     = 0;
	if (buffer_)
	{
		delete buffer_;
		buffer_ = NULL;
	}
}

int64_t OpenBuffer::pop(void* data, size_t len)
{
	if (data == NULL || len <= 0) 
	{
		assert(false);
		return -1;
	}
	if (size_ < len)
	{
		return -1;
	}
	if (!buffer_)
	{
		assert(false);
		return -1;
	}
	memcpy(data, buffer_ + offset_, len);
	offset_ += len;
	size_   -= len;
	return size_;
}

int64_t OpenBuffer::push(const void* data, size_t len)
{
	size_t newSize = size_ + len;
	int64_t leftCap = cap_ - offset_;
	size_t offset = 0;
	if (leftCap < (int64_t)newSize)
	{
		if (buffer_ && newSize < cap_)
		{
			for (size_t i = 0; i < size_; i++)
			{
				buffer_[i] = buffer_[offset_ + i];
			}
			memset(buffer_ + size_, 0, cap_ + 1 - size_);
			offset = size_;
		}
		else
		{
			unsigned char* origin = buffer_;
			cap_ = miniCap_;
			while (newSize > cap_)
			{
				cap_ *= 2;
			}
			buffer_ = new unsigned char[cap_ + 2];
			if (!buffer_)
			{
				buffer_ = origin;
				assert(false);
				return -1;
			}
			memset(buffer_, 0, cap_ + 2);
			if (origin)
			{
				if (size_ > 0)
				{
					memcpy(buffer_, origin + offset_, size_);
				}
				delete origin;
				offset = size_;
			}
		}
		offset_ = 0;
	}
	else
	{
		offset = offset_ + size_;
	}
	memcpy(buffer_ + offset, data, len);
	size_ += len;
	return size_;
}

int64_t OpenBuffer::pushVInt32(const uint32_t& n)
{
	uint8_t p[10] = { 0 };
	while (true)
	{
		if (n < 0x80) {
			p[0] = (uint8_t)n;
			break;
		}
		p[0] = (uint8_t)(n | 0x80);
		if (n < 0x4000) {
			p[1] = (uint8_t)(n >> 7);
			break;
		}
		p[1] = (uint8_t)((n >> 7) | 0x80);
		if (n < 0x200000) {
			p[2] = (uint8_t)(n >> 14);
			break;
		}
		p[2] = (uint8_t)((n >> 14) | 0x80);
		if (n < 0x10000000) {
			p[3] = (uint8_t)(n >> 21);
			break;
		}
		p[3] = (uint8_t)((n >> 21) | 0x80);
		p[4] = (uint8_t)(n >> 28);
		break;
	}
	return push(&p, strlen((const char*)p));
}

int64_t OpenBuffer::pushVInt64(const uint64_t& n)
{
	if ((n & 0xffffffff) == n) {
		return pushVInt32((uint32_t)n);
	}
	uint8_t p[10] = { 0 };
	uint64_t num = n;
	int64_t i = 0;
	do {
		p[i] = (uint8_t)(num | 0x80);
		num >>= 7;
		++i;
	} while (num >= 0x80);
	p[i] = (uint8_t)num;
	return push(&p, strlen((const char*)p));
}

int64_t OpenBuffer::popVInt64(uint64_t& n)
{
	if (size_ <= 0) return -1;
	if (size_ >= 1)
	{
		unsigned char* p = buffer_ + offset_;
		if (!(p[0] & 0x80))
		{
			n = p[0];
			offset_ += 1;
			size_ -= 1;
			return size_;
		}
		if (size_ <= 1) return -1;
	}
	unsigned char* p = buffer_ + offset_;
	uint32_t r = p[0] & 0x7f;
	for (int i = 1; i < 10; i++)
	{
		r |= ((p[i] & 0x7f) << (7 * i));
		if (!(p[i] & 0x80))
		{
			n = r;
			++i;
			offset_ += i;
			size_ -= i;
			return size_;
		}
	}
	return -1;
}

};