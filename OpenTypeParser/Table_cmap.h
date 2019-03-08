#pragma once
#include"pch.h"
#pragma pack(1)
//cmap Table
struct EncodingRecord {
	uint16 platformID;
	uint16 encodingID;
	Offset32 offset; //	Byte offset from beginning of table to the subtable for this encoding.
	EncodingRecord() {}
	EncodingRecord(BigEndianToLittleEndianStream* stream) {
		platformID = stream->nextUint16();
		encodingID = stream->nextUint16();
		offset = stream->nextUint32();
	}
	string toString() {
		stringstream ss;
		ss << "platformID: " << platformID << "\n";
		ss << "encodingID: " << encodingID << "\n";
		ss << "offset: " << offset << "\n";
		return ss.str();
	}
};

struct CmapTableHeader {
	uint16 version;
	uint16 numTables;
	EncodingRecord** encodingRecords;
	CmapTableHeader() {
		encodingRecords = nullptr;
	}

	CmapTableHeader(BigEndianToLittleEndianStream* stream) {
		encodingRecords = nullptr;
		version = stream->nextUint16();
		numTables = stream->nextUint16();
		encodingRecords = new EncodingRecord*[numTables];
		for (int i = 0; i < numTables; i++) {
			encodingRecords[i] = new EncodingRecord(stream);
		}
	}

	string toString(){
		stringstream ss;
		ss << "--Cmap Table Header--" << "\n";
		ss << "version: " << version << "\n";
		ss << "numTables: " << numTables << "\n";
		for (int i = 0; i < numTables; i++) {
			ss << "--Encoding Table Record #" << i << "--\n";
			ss << encodingRecords[i]->toString() << "\n";
		}
		return ss.str();
	}

	EncodingRecord* findRecord(uint16 platformID, uint16 encodingID) {
		if (encodingRecords == nullptr)return nullptr;
		for (int i = 0; i < numTables; i++) {
			if (encodingRecords[i]->platformID == platformID && encodingRecords[i]->encodingID == encodingID) {
				return encodingRecords[i];
			}
		}
		return nullptr;
	}
};

//Segment mapping to delta values
//the standard character-to-glyph-index mapping table for the Windows platform for fonts that support Unicode BMP characters
struct CmapSubtable_Format4 {
	uint16 format;
	uint16 length; //the length in bytes of the subtable
	uint16 language; //For requirements on use of the language field
	uint16 segCountX2; //2 ¡Á segCount
	uint16 searchRange; //2 ¡Á (2**floor(log2(segCount)))
	uint16 entrySelector; //log2(searchRange/2)
	uint16 rangeShift; //2 ¡Á segCount - searchRange
	uint16* endCode; //endCode[segCount] End characterCode for each segment, last=0xFFFF
	uint16 reservedPad; //set to 0
	uint16* startCode; //startCode[segCount] Start character code for each segment
	int16* idDelta; //idDelta[segCount]	Delta for all character codes in segment
	uint16* idRangeOffset; //idRangeOffset[segCount]	Offsets into glyphIdArray or 0
	uint16* glyphIdArray; //glyphIdArray[ ]	Glyph index array (arbitrary length)
	uint16 glyphIdArrayLength;
	CmapSubtable_Format4(){}
	CmapSubtable_Format4(BigEndianToLittleEndianStream* stream) {
		uint32 oriPos = stream->getPos();
		format = stream->nextUint16();
		length = stream->nextUint16();
		language = stream->nextUint16();
		segCountX2 = stream->nextUint16();

		uint16 segCount = segCountX2 / 2;
		endCode = new uint16[segCount];
		startCode = new uint16[segCount];
		idDelta = new int16[segCount];
		idRangeOffset = new uint16[segCount];

		searchRange = stream->nextUint16();
		entrySelector = stream->nextUint16();
		rangeShift = stream->nextUint16();

		for (int i = 0; i < segCount; i++) endCode[i] = stream->nextUint16();
		reservedPad = stream->nextUint16();
		for (int i = 0; i < segCount; i++) startCode[i] = stream->nextUint16();
		for (int i = 0; i < segCount; i++) idDelta[i] = stream->nextInt16();
		for (int i = 0; i < segCount; i++) idRangeOffset[i] = stream->nextUint16();
		uint32 curPos = stream->getPos();
		glyphIdArrayLength = (length - (curPos - oriPos)) / 2;
		if (glyphIdArrayLength > 0) {
			glyphIdArray = new uint16[glyphIdArrayLength];
			for (int i = 0; i < glyphIdArrayLength; i++) {
				glyphIdArray[i] = stream->nextUint16();
			}
		}
	}
	string toString() {
		stringstream ss;
		ss << "format: " << format << "\n";
		ss << "length: " << length << "\n";
		ss << "language: " << language << "\n";
		ss << "segCountX2: " << segCountX2 << "\n";
		ss << "searchRange: " << searchRange << "\n";
		ss << "entrySelector: " << entrySelector << "\n";
		ss << "rangeShift: " << rangeShift << "\n";
		uint16 segCount = segCountX2 / 2;
#define PRINT_ARR(arr, arrLen) ss<<"[";for(int i = 0; i < arrLen; i++){ss<<arr[i];if(i != arrLen-1)ss<<", ";}ss<<"]";
		ss << "endCode: ";
		PRINT_ARR(endCode, segCount);
		ss << "\n";
		ss << "reservedPad: " << reservedPad << "\n";
		ss << "startCode: ";
		PRINT_ARR(startCode, segCount);
		ss << "\n";
		ss << "idDelta: ";
		PRINT_ARR(idDelta, segCount);
		ss << "\n";
		ss << "idRangeOffset: ";
		PRINT_ARR(idRangeOffset, segCount);
		ss << "\n";
		ss << "glyphIdArray: ";
		PRINT_ARR(glyphIdArray, glyphIdArrayLength);
		ss << "\n";
		ss << "glyphIdArrayLength: " << glyphIdArrayLength << "\n";
#undef PRINT_ARR(arr, arrlen)
		return ss.str();
	}

	int32 findGlyph(uint16 code) {
		uint16 segCount = segCountX2 / 2;
		for (uint16 i = 0; i < segCount; i++) {
			if (endCode[i] >= code) {
				if (startCode[i] <= code) {
					if (idRangeOffset[i] != 0) {
						//uint16 index = (idRangeOffset[i] / 2 + segCount - i) + code - startCode[i];
						uint16 index = (idRangeOffset[i] / 2 + i - segCount) + code - startCode[i];
						if (index < glyphIdArrayLength) {
							return glyphIdArray[index] + idDelta[i];
						}
						//cout << "index(" << index << ") > glyphIdArrayLength(" << glyphIdArrayLength << ")" << endl;
						return -1;
					}
					return idDelta[i] + code;
				}
				else return -1;
			}
		}
		return -1;
	}
};

#pragma pack()