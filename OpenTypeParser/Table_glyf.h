#pragma once
#include"pch.h"
#include"Table_loca.h"
#pragma pack(1)
struct GlyphHeader {
	int16 numberOfContours;//If the number of contours is greater than or equal to zero, this is a simple glyph. If negative, this is a composite glyph ¡ª the value -1 should be used for composite glyphs.
	int16 xMin;
	int16 yMin;
	int16 xMax;
	int16 yMax;
	GlyphHeader() {}
	GlyphHeader(BigEndianToLittleEndianStream* stream) {
		numberOfContours = stream->nextInt16();
		xMin = stream->nextInt16();
		yMin = stream->nextInt16();
		xMax = stream->nextInt16();
		yMax = stream->nextInt16();
	}
	string toString() {
		stringstream ss;
		ss << "numberOfContours: " << numberOfContours << "\n";
		ss << "xMin: " << xMin << "\n";
		ss << "yMin: " << yMin << "\n";
		ss << "xMax: " << xMax << "\n";
		ss << "yMax: " << yMax << "\n";
		return ss.str();
	}
};

struct GlyphBase {
	GlyphHeader* header;
	virtual string toString() = 0;
	virtual string dataAsString(uint16 unitsPerEm) = 0;
};


//Simple Glyph Flags
#define ON_CURVE_POINT 0x01
#define X_SHORT_VECTOR 0x02
#define Y_SHORT_VECTOR 0x04
#define REPEAT_FLAG 0x08
#define X_IS_SAME_OR_POSITIVE_X_SHORT_VECTOR 0x10
#define Y_IS_SAME_OR_POSITIVE_Y_SHORT_VECTOR 0x20
#define OVERLAP_SIMPLE 0x40
#define SIMPLE_GLYPH_FLAG_RESERVED 0x80

struct SimpleGlyphTable: GlyphBase {
	uint16* endPtsOfContours; //endPtsOfContours[numberOfContours]	Array of point indices for the last point of each contour, in increasing numeric order.
	uint16 instructionLength;
	uint8* instructions;
	vector<uint8> flags;
	vector<int16> xCoords;
	vector<int16> yCoords;

	virtual string toString() {
		stringstream ss;
#define PRINT_ARR(arr, arrLen) ss<<"[";for(int i = 0; i < arrLen; i++){ss<<arr[i];if(i != arrLen-1)ss<<", ";}ss<<"]";
		
		ss << "endPtsOfContours: ";
		PRINT_ARR(endPtsOfContours, header->numberOfContours);
		ss << "\n";
		if (header->numberOfContours <= 0)return ss.str();
		ss << "instructionLength: " << instructionLength << "\n";
		
		ss << "instructions: ";
		PRINT_ARR(instructions, instructionLength);
		ss << "\n";

		ss << "flags: \n[";
		for (int i = 0; i < flags.size(); i++) {
			ss << flagAsString(flags[i]);
			if (i != flags.size() - 1) {
				ss << "\n";
			}
		}
		ss << "]\n";

		ss << "xCoords: ";
		PRINT_ARR(xCoords, xCoords.size());
		ss << "\n";

		ss << "yCoords: ";
		PRINT_ARR(yCoords, yCoords.size());
		ss << "\n";
#undef PRINT_ARR(arr, arrlen)
		return ss.str();
	}
	
	SimpleGlyphTable(BigEndianToLittleEndianStream* stream, GlyphHeader* header) {
		this->header = header;
		if (header->numberOfContours <= 0)return;
		endPtsOfContours = new uint16[header->numberOfContours];
		for (int i = 0; i < header->numberOfContours; i++) {
			endPtsOfContours[i] = stream->nextUint16();
		}
		instructionLength = stream->nextUint16();
		instructions = new uint8[instructionLength];
		for (int i = 0; i < instructionLength; i++) {
			instructions[i] = stream->nextUint8();
		}

		uint32 possibleFlagNum = endPtsOfContours[header->numberOfContours - 1] + 1;
		flags = vector<uint8>();
		while (possibleFlagNum--) {
			uint8 flag = stream->nextUint8();
			flags.push_back(flag);
			if (flag & REPEAT_FLAG) {
				uint8 repeatTimes = stream->nextUint8();
				for (int i = 0; i < repeatTimes; i++) {
					flags.push_back(flag);
					possibleFlagNum--;
				}
			}
		}

		xCoords = vector<int16>();
		for (auto flag : flags) {
			if (flag & X_SHORT_VECTOR) {
				if (flag & X_IS_SAME_OR_POSITIVE_X_SHORT_VECTOR) {
					xCoords.push_back(stream->nextUint8());
				}
				else {
					xCoords.push_back(-(stream->nextUint8()));
				}
			}
			else {
				if (flag & X_IS_SAME_OR_POSITIVE_X_SHORT_VECTOR) {
					xCoords.push_back(0);
				}
				else {
					xCoords.push_back(stream->nextInt16());
				}
			}
		}

		yCoords = vector<int16>();
		for (auto flag : flags) {
			if (flag & Y_SHORT_VECTOR) {
				if (flag & Y_IS_SAME_OR_POSITIVE_Y_SHORT_VECTOR) {
					yCoords.push_back(stream->nextUint8());
				}
				else {
					yCoords.push_back(-(stream->nextUint8()));
				}
			}
			else {
				if (flag & Y_IS_SAME_OR_POSITIVE_Y_SHORT_VECTOR) {
					yCoords.push_back(0);
				}
				else {
					yCoords.push_back(stream->nextInt16());
				}
			}
		}

	}

	string flagAsString(uint8 flag) {
		stringstream ss;
		vector<string>fs = vector<string>();
		if (flag&ON_CURVE_POINT)fs.push_back("ON_CURVE_POINT");
		if (flag&X_SHORT_VECTOR)fs.push_back("X_SHORT_VECTOR");
		if (flag&Y_SHORT_VECTOR)fs.push_back("Y_SHORT_VECTOR");
		if (flag&REPEAT_FLAG)fs.push_back("REPEAT_FLAG");
		if (flag&X_IS_SAME_OR_POSITIVE_X_SHORT_VECTOR)fs.push_back("X_IS_SAME_OR_POSITIVE_X_SHORT_VECTOR");
		if (flag&Y_IS_SAME_OR_POSITIVE_Y_SHORT_VECTOR)fs.push_back("Y_IS_SAME_OR_POSITIVE_Y_SHORT_VECTOR");
		if (flag&OVERLAP_SIMPLE)fs.push_back("OVERLAP_SIMPLE");
		if (flag&SIMPLE_GLYPH_FLAG_RESERVED)fs.push_back("Reserved");
		for (int i = 0; i < fs.size(); i++) {
			ss << fs[i];
			if (i != fs.size() - 1) {
				ss << " & ";
			}
		}
		return ss.str();
	}

	string dataAsString(uint16 unitsPerEm) override {
		stringstream ss;
		ss << "contour num:" << header->numberOfContours << "\n";
		vector<uint16>contourOffset;
		contourOffset.push_back(0);
		for (int i = 0; i < header->numberOfContours - 1; i++) {
			contourOffset.push_back(endPtsOfContours[i] + 1);
		}
		int16 absoX = 0;
		int16 absoY = 0;
		for (int i = 0; i < header->numberOfContours; i++) {
			ss << "flags:";
			for (int j = contourOffset[i]; j <= endPtsOfContours[i]; j++) {
				ss << (flags[j] & ON_CURVE_POINT) ? "1" : "0";
			}
			ss << "\n";

			ss << "points:";
			for (int j = contourOffset[i]; j <= endPtsOfContours[i]; j++) {
				absoX += xCoords[j];
				absoY += yCoords[j];
				float scaleFac = 1.0;
				ss << (float(absoX) / float(unitsPerEm) - 0.5) / 0.5 * scaleFac << " "
					<< (float(absoY) / float(unitsPerEm) - 0.5) / 0.5 * scaleFac;
				if (j != endPtsOfContours[i]) {
					ss << ",";
				}
			}
			ss << "\n";
		}
		return ss.str();
	}
};

//Composite Glyph Flags
#define ARG_1_AND_2_ARE_WORDS 0x0001
#define ARGS_ARE_XY_VALUES 0x0002
#define ROUND_XY_TO_GRID 0x0004
#define WE_HAVE_A_SCALE 0x0008
#define MORE_COMPONENTS 0x0020
#define WE_HAVE_AN_X_AND_Y_SCALE 0x0040
#define WE_HAVE_A_TWO_BY_TWO 0x0080
#define WE_HAVE_INSTRUCTIONS 0x0100
#define USE_MY_METRICS 0x0200
#define OVERLAP_COMPOUND 0x0400
#define SCALED_COMPONENT_OFFSET 0x0800
#define UNSCALED_COMPONENT_OFFSET 0x1000
#define COMPOSITE_GLYPH_FLAG_RESERVED 0xE010

struct CompositeTransformBase {
	virtual string toString() = 0;
	virtual string dataAsString() = 0;
};

struct CompositeTransform_A_SCALE : CompositeTransformBase {
	F2DOT14 scale;
	CompositeTransform_A_SCALE(BigEndianToLittleEndianStream* stream) {
		scale = stream->nextF2DOT14();
	}

	virtual string toString() override{
		stringstream ss;
		ss << "scale: " << scale << "\n";
		return ss.str();
	}

	virtual string dataAsString() override {
		stringstream ss;
		ss << "scale:" << scale << "\n";
		return ss.str();
	}
};

struct CompositeTransform_AN_X_AND_Y_SCALE : CompositeTransformBase {
	F2DOT14 xscale;
	F2DOT14 yscale;
	CompositeTransform_AN_X_AND_Y_SCALE(BigEndianToLittleEndianStream* stream) {
		xscale = stream->nextF2DOT14();
		yscale = stream->nextF2DOT14();
	}

	virtual string toString() override{
		stringstream ss;
		ss << "xscale: " << xscale << "\n";
		ss << "yscale: " << yscale << "\n";
		return ss.str();
	}

	virtual string dataAsString() override {
		stringstream ss;
		ss << "scale:" << xscale << "," << yscale << "\n";
		return ss.str();
	}
};

struct CompositeTransform_A_TWO_BY_TWO : CompositeTransformBase{
	F2DOT14 xscale;
	F2DOT14 scale01;
	F2DOT14 scale10;
	F2DOT14 yscale;
	CompositeTransform_A_TWO_BY_TWO(BigEndianToLittleEndianStream* stream) {
		xscale = stream->nextF2DOT14();
		scale01 = stream->nextF2DOT14();
		scale10 = stream->nextF2DOT14();
		yscale = stream->nextF2DOT14();
	}

	virtual string toString() override {
		stringstream ss;
		ss << "xscale: " << xscale << "\n";
		ss << "scale01: " << scale01 << "\n";
		ss << "scale10: " << scale10 << "\n";
		ss << "yscale: " << yscale << "\n";
		return ss.str();
	}

	virtual string dataAsString() override {
		stringstream ss;
		ss << "scale:" << xscale << "," << scale01 << "," << scale10 << "," << yscale << "\n";
		return ss.str();
	}
};

struct CompositeGlyphTableData {
	uint16 flags;
	uint16 glyphIndex;
	int32 argument1;
	int32 argument2;
	CompositeTransformBase* transform;
	CompositeGlyphTableData(BigEndianToLittleEndianStream* stream) {
		transform = nullptr;
		flags = stream->nextUint16();
		glyphIndex = stream->nextUint16();
		if (flags & ARG_1_AND_2_ARE_WORDS) {
			if (flags & ARGS_ARE_XY_VALUES) {
				argument1 = stream->nextInt16();
				argument2 = stream->nextInt16();
			}
			else {
				argument1 = stream->nextUint16();
				argument2 = stream->nextUint16();
			}
		}
		else {
			if (flags & ARGS_ARE_XY_VALUES) {
				argument1 = stream->nextInt8();
				argument2 = stream->nextInt8();
			}
			else {
				argument1 = stream->nextUint8();
				argument2 = stream->nextUint8();
			}
		}

		if (flags & WE_HAVE_A_SCALE) {
			transform = new CompositeTransform_A_SCALE(stream);
		}
		else if (flags & WE_HAVE_AN_X_AND_Y_SCALE) {
			transform = new CompositeTransform_AN_X_AND_Y_SCALE(stream);
		}
		else if (flags & WE_HAVE_A_TWO_BY_TWO) {
			transform = new CompositeTransform_A_TWO_BY_TWO(stream); 
		}

	}

	string toString() {
		stringstream ss;
		ss << "flags: " << flagAsString(flags) << "\n";
		ss << "glyphIndex: " << glyphIndex << "\n";
		ss << "argument1: " << argument1 << "\n";
		ss << "argument2: " << argument2 << "\n";
		if (transform != nullptr) {
			ss << "transform: \n" << transform->toString();
		}
		return ss.str();
	}

	string dataAsString(uint16 unitsPerEm) {
		stringstream ss;
		ss << "glyph index:" << glyphIndex << "\n";
		if (flags&ARGS_ARE_XY_VALUES) {
			ss << "xy offsets:" << float(argument1) / float(unitsPerEm) / 0.5f << "," << float(argument2) / float(unitsPerEm) /0.5f << "\n";
		}
		else {
			ss << "pin point:" << argument1 << "," << argument2 << "\n";
		}
		if (transform == nullptr) {
			ss << "scale:null\n";
		}
		else {
			ss << transform->dataAsString();
		}
		return ss.str();
	}

	string flagAsString(uint16 flag) {
		stringstream ss;
		vector<string>fs = vector<string>();
		if (flag&ARG_1_AND_2_ARE_WORDS)fs.push_back("ARG_1_AND_2_ARE_WORDS");
		if (flag&ARGS_ARE_XY_VALUES)fs.push_back("ARGS_ARE_XY_VALUES");
		if (flag&ROUND_XY_TO_GRID)fs.push_back("ROUND_XY_TO_GRID");
		if (flag&WE_HAVE_A_SCALE)fs.push_back("WE_HAVE_A_SCALE");
		if (flag&MORE_COMPONENTS)fs.push_back("MORE_COMPONENTS");
		if (flag&WE_HAVE_AN_X_AND_Y_SCALE)fs.push_back("WE_HAVE_AN_X_AND_Y_SCALE");
		if (flag&WE_HAVE_A_TWO_BY_TWO)fs.push_back("WE_HAVE_A_TWO_BY_TWO");
		if (flag&WE_HAVE_INSTRUCTIONS)fs.push_back("WE_HAVE_INSTRUCTIONS");
		if (flag&USE_MY_METRICS)fs.push_back("USE_MY_METRICS");
		if (flag&OVERLAP_COMPOUND)fs.push_back("OVERLAP_COMPOUND");
		if (flag&SCALED_COMPONENT_OFFSET)fs.push_back("SCALED_COMPONENT_OFFSET");
		if (flag&UNSCALED_COMPONENT_OFFSET)fs.push_back("UNSCALED_COMPONENT_OFFSET");
		if (flag&COMPOSITE_GLYPH_FLAG_RESERVED)fs.push_back("Reserved");
		for (int i = 0; i < fs.size(); i++) {
			ss << fs[i];
			if (i != fs.size() - 1) {
				ss << " & ";
			}
		}
		return ss.str();
	}
};

struct CompositeGlyphTable: GlyphBase{
	vector<CompositeGlyphTableData*>datas;
	uint16 numInstr;
	uint8* instr;
	virtual string toString() {
		stringstream ss;
		ss << "datas: \n[";
		for (auto data : datas) {
			ss << data->toString() << "\n";
		}
		ss << "]\n";
		ss << "numInstr: " << numInstr << "\n";
		ss << "instr: [";
		for (int i = 0; i < numInstr; i++) {
			ss << instr[i];
			if (i != numInstr - 1) {
				ss << ", ";
			}
		}
		ss << "]\n";
		return ss.str();
	}
	CompositeGlyphTable(){}
	CompositeGlyphTable(BigEndianToLittleEndianStream* stream, GlyphHeader* header) {
		this->header = header;
		numInstr = 0;
		uint16 flags;
		do {
			CompositeGlyphTableData* data = new CompositeGlyphTableData(stream);
			datas.push_back(data);
			flags = data->flags;
		} while (flags & MORE_COMPONENTS);
		if (flags & WE_HAVE_INSTRUCTIONS) {
			numInstr = stream->nextUint16();
			instr = new uint8[numInstr];
			for (int i = 0; i < numInstr; i++) {
				instr[i] = stream->nextUint8();
			}
		}
	}

	virtual string dataAsString(uint16 unitsPerEm) override {
		stringstream ss;
		ss << "component num:" << datas.size() << "\n";
		for (int i = 0; i < datas.size(); i++) {
			ss << datas[i]->dataAsString(unitsPerEm);
		}
		return ss.str();
	}
};

struct GlyfTable {
	GlyphBase** glyphs;
	GlyfTable() {}
	GlyfTable(BigEndianToLittleEndianStream* stream, LocaTable* locaTable) {
		uint32 baseOffset = stream->getPos();
		glyphs = new GlyphBase*[locaTable->n - 1];
		for (int i = 0; i < locaTable->n - 1; i++) {
			glyphs[i] = nullptr;
			if (locaTable->offsets[i + 1] - locaTable->offsets[i] == 0)continue;
			stream->setPos(baseOffset + locaTable->offsets[i]);
			GlyphHeader* header = new GlyphHeader(stream);
			GlyphBase* glyph;
			if (header->numberOfContours >= 0) {
				glyph = new SimpleGlyphTable(stream, header);
			}
			else {
				glyph = new CompositeGlyphTable(stream, header);
			}
			glyphs[i] = glyph;
		}
	}

	string toString() {
		stringstream ss;
		return ss.str();
	}
};
#pragma pack()