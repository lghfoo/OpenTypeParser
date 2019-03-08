#pragma once
#include"pch.h"
#pragma pack(1)
struct HeadTable {
	uint16 majorVersion;
	uint16 minorVersion;
	Fixed fontRevision;
	uint32 checkSumAdjustment;
	uint32 magicNumber;
	uint16 flags;
	uint16 unitsPerEm;
	LONGDATETIME created;
	LONGDATETIME modified;
	int16 xMin;
	int16 yMin;
	int16 xMax;
	int16 yMax;
	uint16 macStyle;
	uint16 lowestRecPPEM;
	int16 fontDirectionHint;
	int16 indexToLocFormat;
	int16 glyphDataFormat;
	HeadTable() {}
	HeadTable(BigEndianToLittleEndianStream* stream) {
		majorVersion = stream->nextUint16();
		minorVersion = stream->nextUint16();
		fontRevision = stream->nextFixed();
		checkSumAdjustment = stream->nextUint32();
		magicNumber = stream->nextUint32();
		flags = stream->nextUint16();
		unitsPerEm = stream->nextUint16();
		created = stream->nextLONGDATETIME();
		modified = stream->nextLONGDATETIME();
		xMin = stream->nextInt16();
		yMin = stream->nextInt16();
		xMax = stream->nextInt16();
		yMax = stream->nextInt16();
		macStyle = stream->nextUint16();
		lowestRecPPEM = stream->nextUint16();
		fontDirectionHint = stream->nextInt16();
		indexToLocFormat = stream->nextInt16();
		glyphDataFormat = stream->nextInt16();
	}
	string toString() {
		stringstream ss;
		ss << "majorVersion: " << majorVersion << "\n";
		ss << "minorVersion: " << minorVersion << "\n";
		ss << "fontRevision: " << fontRevision << "\n";
		ss << "checkSumAdjustment: " << checkSumAdjustment << "\n";
		ss << "magicNumber: " << magicNumber << "\n";
		ss << "flags: " << flags << "\n";
		ss << "unitsPerEm: " << unitsPerEm << "\n";
		ss << "created: " << created << "\n";
		ss << "modified: " << modified << "\n";
		ss << "xMin: " << xMin << "\n";
		ss << "yMin: " << yMin << "\n";
		ss << "xMax: " << xMax << "\n";
		ss << "yMax: " << yMax << "\n";
		ss << "macStyle: " << macStyle << "\n";
		ss << "lowestRecPPEM: " << lowestRecPPEM << "\n";
		ss << "fontDirectionHint: " << fontDirectionHint << "\n";
		ss << "indexToLocFormat: " << indexToLocFormat << "\n";
		ss << "glyphDataFormat: " << glyphDataFormat << "\n";
		return ss.str();
	}
};
#pragma pack()