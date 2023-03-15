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


#define MALLOC malloc
#define FREE free

namespace open
{

OpenBuffer::OpenBuffer(size_t capacity)
	:cap_(capacity),
	offset_(0),
	size_(0),
	head_(NULL),
	tail_(NULL),
	rbuffer_(NULL),
	readLen_(0),
	readOffset_(0)
{
}

OpenBuffer::~OpenBuffer()
{
	Node* bnode = 0;
	while(head_ != NULL)
	{
		bnode = head_;
		head_ = bnode->next_;

		FREE(bnode->data_);
		FREE(bnode);
	}
	head_ = NULL;
	tail_ = NULL;
	if (rbuffer_)
	{
		FREE(rbuffer_);
		rbuffer_ = NULL;
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
	if (!rbuffer_ || readLen_ - readOffset_ < len)
	{
		merge();
	}
	if (len == 0)
	{
		return size_;
	}
	if (!rbuffer_)
	{
		assert(false);
		return -1;
	}
	if (data)
	{
		memcpy(data, rbuffer_ + readOffset_, len);
	}
	readLen_ -= len;
	readOffset_ += len;
	size_ -= len;
	return size_;
}

void OpenBuffer::merge()
{
	if (size_ <= readLen_)
	{
		return;
	}
	unsigned char* origin = rbuffer_;
	rbuffer_ = (unsigned char*)MALLOC(size_);
	if (!rbuffer_)
	{
		rbuffer_ = origin;
		assert(false);
		return;
	}
	size_t offset = 0;
	if (origin)
	{
		if (readLen_ > 0)
		{
			memcpy(rbuffer_, origin + readOffset_, readLen_);
		}
		FREE(origin);
		offset = readLen_;
	}

	unsigned char* data = rbuffer_ + offset;
	Node* bnode = 0;
	while(head_ != NULL)
	{
		bnode = head_;
		head_ = bnode->next_;
		memcpy(data, bnode->data_, bnode->size_);
		data += bnode->size_;
		FREE(bnode->data_);
		FREE(bnode);
	}
	readLen_ = size_;
	readOffset_ = 0;

	memcpy(tmp, rbuffer_, readLen_);
}

int64_t OpenBuffer::push(const void* data, size_t len)
{
	if (!tail_ || offset_ >= cap_ || len >= cap_ - offset_)
	{
		Node* bnode = (Node*)MALLOC(sizeof(Node));
		if (!bnode)
		{
			assert(false);
			return -1;
		}
		bnode->data_ = (unsigned char*)MALLOC(len >= cap_ ? len : cap_);
		if (!bnode->data_)
		{
			FREE(bnode);
			assert(false);
			return -1;
		}
		memcpy(bnode->data_, data, len);
		offset_ = len;
		bnode->size_ = len;
		bnode->next_ = NULL;
		if (head_ == NULL) {
			assert(tail_ == NULL);
			head_ = tail_ = bnode;
		}
		else {
			tail_->next_ = bnode;
			tail_ = bnode;
		}
		size_ += len;
		return size_;
	}
	assert(offset_ < cap_);
	assert(len < cap_ - offset_);
	memcpy(tail_->data_ + offset_, data, len);
	tail_->size_ += len;
	offset_ += len;
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
	if (size_ > readLen_)
	{
		merge();
	}
	if (size_ >= 1)
	{
		unsigned char* p = rbuffer_ + readOffset_;
		if (!(p[0] & 0x80))
		{
			n = p[0];
			readLen_ -= 1;
			readOffset_ += 1;
			size_ -= 1;
			return size_;
		}
		if (size_ <= 1) return -1;
	}
	unsigned char* p = rbuffer_ + readOffset_;
	uint32_t r = p[0] & 0x7f;
	for (int i = 1; i < 10; i++)
	{
		r |= ((p[i] & 0x7f) << (7 * i));
		if (!(p[i] & 0x80))
		{
			n = r;
			++i;
			readLen_ -= i;
			readOffset_ += i;
			size_ -= i;
			return size_;
		}
	}
	return -1;
}


//int64_t OpenBuffer::readPush(void* msg, size_t len)
//{
//	if (msg == NULL || len <= 0){
//		return -1;
//	}
//	Node* bnode = (Node*)MALLOC(sizeof(Node));
//	if (!bnode) {
//		return -1;
//	}
//	bnode->msg_ = (unsigned char*)MALLOC(len);
//	if (!bnode->msg_) {
//		FREE(bnode);
//		return -1;
//	}
//	memcpy(bnode->msg_, msg, len);
//	bnode->sz_   = len;
//	bnode->next_ = NULL;
//
//	if (readHead_ == NULL) {
//		assert(readTail_ == NULL);
//		readHead_ = readTail_ = bnode;
//	} else {
//		readTail_->next_ = bnode;
//		readTail_ = bnode;
//	}
//	readSize_ += len;
//	return readSize_;
//}
//
//void OpenBuffer::popNode()
//{
//	Node* bnode = readHead_;
//	readSize_ -= bnode->sz_ - readOffset_;
//	readOffset_ = 0;
//	readHead_ = bnode->next_;
//	if (readHead_ == NULL) {
//		readTail_ = NULL;
//	}
//	FREE(bnode->msg_);
//	FREE(bnode);
//}
//
//int64_t OpenBuffer::readHeadLen(size_t len)
//{
//	if (len > 4 || len < 1) return -2;
//	if (len > readSize_) return -1;
//	Node* bnode = readHead_;
//	const unsigned char* ptr = (const unsigned char*)bnode->msg_ + readOffset_;
//	size_t i = 0;
//	int64_t sz = 0;
//	for (; i < len; i++) {
//		sz <<= 8;
//		sz |= *ptr;
//		readOffset_++;
//		if (bnode->sz_ - readOffset_ == 0){
//			popNode();
//			if(i < len)	break;
//			bnode = readHead_;
//			assert(bnode);
//			ptr = (const unsigned char*)bnode->msg_ + readOffset_;
//		} else {
//			ptr++;
//		}
//	}
//	readSize_ -= len;
//	return sz;
//}
//
//int64_t OpenBuffer::readBuffer(unsigned char* data, int len)
//{
//	if (data == NULL) return -2;
//	if(len > readSize_) return -1;
//
//	Node* bnode = readHead_;
//	size_t size = 0;
//	size_t rd_len = len;
//	do{
//		size = bnode->sz_ - readOffset_;
//		if (rd_len >= size){
//			memcpy(data, bnode->msg_ + readOffset_, size);
//			popNode();
//			bnode = readHead_;
//			rd_len -= size;
//			data += size;
//		} else {
//			memcpy(data, bnode->msg_ + readOffset_, rd_len);
//			readOffset_ += rd_len;
//			readSize_ -= rd_len;
//			rd_len = 0;
//		}
//	}while(rd_len > 0);
//	return len;
//}
//
//void OpenBuffer::adjustPack(int len, int type)
//{
//	if (type == 0)
//	{
//		if (len >= readCap_) {
//	    	while(len >= readCap_)	readCap_ *= 2;
//	        FREE(readData_);
//			readData_ = (unsigned char*)MALLOC(readCap_);
//	    }
//	    else if (readCap_ > MIN_PACK && readCap_ > len*2) {
//			readCap_ /= 2;
//	        FREE(readData_);
//			readData_ = (unsigned char*)MALLOC(readCap_);
//	    }
//	} else {
//		if (len >= writeCap_) {
//	    	while(len >= writeCap_) writeCap_ *= 2;
//	        FREE(writeData_);
//			writeData_ = (unsigned char*)MALLOC(writeCap_);
//	    }
//	    else if (writeCap_ > MIN_PACK && writeCap_ > len * 2) {
//			writeCap_ /= 2;
//	        FREE(writeData_);
//			writeData_ = (unsigned char*)MALLOC(writeCap_);
//	    }
//	}
//}
//
//int64_t OpenBuffer::readPack() 
//{
//	int64_t rlen = packLen_;
//	if (rlen <= 0)
//	{
//		rlen = readHeadLen(headLen_);
//		if (rlen <= 0) return -1;
//		packLen_ = rlen;
//	}
//	if(rlen > readSize_) return -1;
//	packLen_ = 0;
//	readLen_ = rlen;
//	adjustPack(rlen, 0);
//	read(readData_, rlen);
//	return rlen;
//}
//
//int64_t OpenBuffer::writePush(unsigned char* msg, size_t len)
//{
//	if (msg == NULL || len <= 0){
//		return -1;
//	}
//	Node* bnode = (Node*)MALLOC(sizeof(Node));
//	if (!bnode)
//	{
//		return -1;
//	}
//	bnode->msg_ = (unsigned char*)MALLOC(len);
//	if (!bnode->msg_)
//	{
//		FREE(bnode);
//		return -1;
//	}
//	memcpy(bnode->msg_, msg, len);
//	bnode->sz_   = len;
//	bnode->next_ = NULL;
//
//	if (writeHead_ == NULL) 
//	{
//		assert(writeTail_ == NULL);
//		writeHead_ = writeTail_ = bnode;
//	} 
//	else 
//	{
//		writeTail_->next_ = bnode;
//		writeTail_ = bnode;
//	}
//	writeSize_ += len;
//	return writeSize_;
//}
//
//int64_t OpenBuffer::writePack()
//{
//	int64_t max_len = 0;
//	int64_t head_len = headLen_;
//	switch(head_len){
//		case 1:
//			max_len = 0xff;
//		break;
//		case 2:
//			max_len = 0xffff;
//		break;
//		case 4:
//			max_len = 0xffffffff;
//		break;
//		default:
//			return -1;
//		break;
//	}
//	size_t len = writeSize_;
//	if (len > max_len){
//		assert(0);
//		return -1;
//	}
//	int64_t wlen = head_len + len;
//	writeLen_ = wlen;
//	adjustPack(wlen, 1);
//
//	unsigned char* data = writeData_ + head_len;
//	Node* bnode = 0;
//	while(writeHead_ != NULL)
//	{
//		bnode = writeHead_;
//		writeHead_ = bnode->next_;
//
//		memcpy(data, bnode->msg_, bnode->sz_);
//		data += bnode->sz_;
//
//		FREE(bnode->msg_);
//		FREE(bnode);
//	}
//	writeTail_ = NULL;
//	writeSize_ = 0;
//	unsigned char* pdata = writeData_;
//	int i = head_len-1;
//	for (; i >= 0; i--) {
//		*(pdata+i) = len & 0xff;
//		len >>= 8;
//	}
//	return writeLen_;
//}

};