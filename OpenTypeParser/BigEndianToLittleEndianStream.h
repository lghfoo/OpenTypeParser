#pragma once
#include"DataType.h"
class BigEndianToLittleEndianStream
{
private:
	unsigned char* data;
	unsigned int pos;
	unsigned int size;
public:
	BigEndianToLittleEndianStream(unsigned char* data, unsigned int size) {
		reset(data, size);
	}

	BigEndianToLittleEndianStream(unsigned char* data, unsigned int pos, unsigned int size) {
		this->data = data;
		this->pos = pos;
		this->size = size;
	}

	void clear() {
		delete[]data;
		data = nullptr;
	}

	void reset(unsigned char* data, int size) {
		if (this->data != nullptr) {
			clear();
		}
		this->data = data;
		this->pos = 0;
		this->size = size;
	}

	~BigEndianToLittleEndianStream() {

	}

	unsigned int nextUint32() {
		if (pos >= size)return 0;
		unsigned int ret = 0;
		unsigned char* rp = (unsigned char*)&ret;
		*rp = *(data + pos + 3);
		*(rp + 1) = *(data + pos + 2);
		*(rp + 2) = *(data + pos + 1);
		*(rp + 3) = *(data + pos);
		pos += 4;
		return ret;
	}

	int nextInt32() {
		if (pos >= size)return 0;
		int ret = 0;
		unsigned char* rp = (unsigned char*)&ret;
		*rp = *(data + pos + 3);
		*(rp + 1) = *(data + pos + 2);
		*(rp + 2) = *(data + pos + 1);
		*(rp + 3) = *(data + pos);
		pos += 4;
		return ret;
	}

	unsigned short nextUint16() {
		if (pos >= size)return 0;
		unsigned short ret = 0;
		unsigned char* rp = (unsigned char*)&ret;
		*rp = *(data + pos + 1);
		*(rp + 1) = *(data + pos);
		pos += 2;
		return ret;
	}

	short nextInt16() {
		if (pos >= size)return 0;
		short ret = 0;
		unsigned char* rp = (unsigned char*)&ret;
		*rp = *(data + pos + 1);
		*(rp + 1) = *(data + pos);
		pos += 2;
		return ret;
	}

	unsigned char nextUint8() {
		if (pos >= size)return 0;
		return data[pos++];
	}

	char nextInt8() {
		if (pos >= size)return 0;
		return data[pos++];
	}

	Fixed nextFixed() {
		return nextInt32();
	}

	FWORD nextFWORD() {
		return nextInt16();
	}

	UFWORD nextUFWORD() {
		return nextUint16();
	}

	F2DOT14 nextF2DOT14() {
		return nextInt16();
	}

	LONGDATETIME nextLONGDATETIME() {
		if (pos >= size)return 0;
		long long ret = 0;
		unsigned char* rp = (unsigned char*)&ret;
		for (int i = 0; i < 8; i++) {
			*(rp + i) = *(data + pos + (7 - i));
		}
		pos += 8;
		return ret;
	}

	Offset16 nextOffset16() {
		return nextUint16();
	}

	Offset32 nextOffset32() {
		return nextUint32();
	}

	void setPos(unsigned int pos) {
		this->pos = pos;
	}
	
	unsigned int getPos() {
		return this->pos;
	}
};

