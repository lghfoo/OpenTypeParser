#pragma once
#pragma pack(1)
#include"pch.h"
//Table Record
struct TableRecord {
	Tag tableTag;
	uint32 checkSum;
	Offset32 offset;
	uint32 length;
	string toString() {
		stringstream ss;
		string tag = "";
		for (int i = 0; i < 4; i++) {
			tag += tableTag[i];
		}
		ss << "---" << tag << " Table---" << "\n";
		ss << "checkSum: " << checkSum << "\n";
		ss << "offset: " << offset << "\n";
		ss << "length: " << length << "\n";
		return ss.str();
	}
};

//OffsetTable
struct OffsetTable {
	uint32 sfntVersion;
	uint16 numTables;
	uint16 searchRange;
	uint16 entrySelector;
	uint16 rangeShift;
	string toString() {
		stringstream ss;
		ss << "---Offset Table---" << "\n";
		ss <<"sfntVersion: "<< sfntVersion << "\n";
		ss << "numTables: " << numTables << "\n";
		ss << "searchRange: " << searchRange << "\n";
		ss << "entrySelector: " << entrySelector << "\n";
		ss << "rangeShift: " << rangeShift << "\n";
		return ss.str();
	}
};


#pragma pack()