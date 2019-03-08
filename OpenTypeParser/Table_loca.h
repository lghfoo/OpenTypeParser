#pragma once
#include"pch.h"
#include"Table_head.h"
#include"Table_maxp.h"
#pragma pack(1)
struct LocaTable {
	uint16 n;
	Offset32* offsets;
	LocaTable() {
		offsets = nullptr;
	}
	LocaTable(BigEndianToLittleEndianStream* stream, MaxpTable* maxpTable, HeadTable* headTable) {
		offsets = nullptr;
		this->n = maxpTable->numGlyphs + 1;
		offsets = new Offset32[n];
		for (int i = 0; i < n; i++) {
			if (headTable->indexToLocFormat) {
				offsets[i] = stream->nextOffset32();
			}
			else {
				offsets[i] = stream->nextOffset16() * 2;
			}
		}
		
	}

	~LocaTable() {
		if (offsets != nullptr) {
			delete[]offsets;
			offsets = nullptr;
		}
	}

	string toString() {
		stringstream ss;
		ss << "n: " << n << "\n";
		ss << "offsets: [";
		for (int i = 0; i < n; i++) {
			ss << offsets[i];
			if (i != n - 1)ss << ", ";
		}
		ss << "]\n";
		return ss.str();
	}
};
#pragma pack()