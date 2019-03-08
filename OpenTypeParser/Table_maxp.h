#pragma once
#include"pch.h"
#pragma pack(1)
struct MaxpTable {
	Fixed version;
	uint16 numGlyphs;

	uint16 maxPoints;
	uint16 maxContours;
	uint16 maxCompositePoints;
	uint16 maxCompositeContours;
	uint16 maxZones;
	uint16 maxTwilightPoints;
	uint16 maxStorage;
	uint16 maxFunctionDefs;
	uint16 maxInstructionDefs;
	uint16 maxStackElements;
	uint16 maxSizeOfInstructions;
	uint16 maxComponentElements;
	uint16 maxComponentDepth;
public:
	MaxpTable(){}
	MaxpTable(BigEndianToLittleEndianStream* stream) {
		version = stream->nextFixed();
		numGlyphs = stream->nextUint16();
		if (version == 0x00005000) return;
		maxPoints = stream->nextUint16();
		maxContours = stream->nextUint16();
		maxCompositePoints = stream->nextUint16();
		maxCompositeContours = stream->nextUint16();
		maxZones = stream->nextUint16();
		maxTwilightPoints = stream->nextUint16();
		maxStorage = stream->nextUint16();
		maxFunctionDefs = stream->nextUint16();
		maxInstructionDefs = stream->nextUint16();
		maxStackElements = stream->nextUint16();
		maxSizeOfInstructions = stream->nextUint16();
		maxComponentElements = stream->nextUint16();
		maxComponentDepth = stream->nextUint16();
	}

	string toString() {
		stringstream ss;
		ss << "version: " << version << "\n";
		ss << "numGlyphs: " << numGlyphs << "\n";
		if (version == 0x00005000)return ss.str();
		ss << "maxPoints: " << maxPoints << "\n";
		ss << "maxContours: " << maxContours << "\n";
		ss << "maxCompositePoints: " << maxCompositePoints << "\n";
		ss << "maxCompositeContours: " << maxCompositeContours << "\n";
		ss << "maxZones: " << maxZones << "\n";
		ss << "maxTwilightPoints: " << maxTwilightPoints << "\n";
		ss << "maxStorage: " << maxStorage << "\n";
		ss << "maxFunctionDefs: " << maxFunctionDefs << "\n";
		ss << "maxInstructionDefs: " << maxInstructionDefs << "\n";
		ss << "maxStackElements: " << maxStackElements << "\n";
		ss << "maxSizeOfInstructions: " << maxSizeOfInstructions << "\n";
		ss << "maxComponentElements: " << maxComponentElements << "\n";
		ss << "maxComponentDepth: " << maxComponentDepth << "\n";
		return ss.str();
	}
};

#pragma pack()