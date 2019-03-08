#pragma once
#include<iostream>
#include<fstream>
#include"Table.h"
#include"Table_cmap.h"
#include"Table_head.h"
#include"Table_maxp.h"
#include"Table_loca.h"
#include"Table_glyf.h"
#include"BigEndianToLittleEndianStream.h"
using namespace std;
class OpenTypeParser {
private:
	OffsetTable* offsetTable;
	TableRecord** tableRecords;
	HeadTable* headTable;
	MaxpTable* maxpTable;
	LocaTable* locaTable;
	GlyfTable* glyfTable;
	CmapSubtable_Format4* cmapTable;
	BigEndianToLittleEndianStream* stream;
public:
	OpenTypeParser() {
		offsetTable = nullptr;
		tableRecords = nullptr;
		headTable = nullptr;
		maxpTable = nullptr;
		locaTable = nullptr;
		glyfTable = nullptr;
		stream = nullptr;
	}

	bool parse(const char* filename) {
		ifstream fileIn(filename, ios_base::binary);
		if (!fileIn) {
			cout << "error: can't open file \"" << filename << "\"" << endl;
			return false;
		}

		fileIn.seekg(0, ifstream::beg);
		ifstream::pos_type start = fileIn.tellg();
		fileIn.seekg(0, ifstream::end);
		ifstream::pos_type end = fileIn.tellg();
		fileIn.seekg(0, ifstream::beg);
		int size = end - start;

		unsigned char* data = new unsigned char[size];
		fileIn.read((char*)data, size);
		fileIn.close();

		if (stream != nullptr) {
			stream->reset(data, size);
		}
		else {
			stream = new BigEndianToLittleEndianStream(data, size);
		}

		if(offsetTable == nullptr) offsetTable = new OffsetTable();
		offsetTable->sfntVersion = stream->nextUint32();
		offsetTable->numTables = stream->nextUint16();
		offsetTable->searchRange = stream->nextUint16();
		offsetTable->entrySelector = stream->nextUint16();
		offsetTable->rangeShift = stream->nextUint16();

		if (tableRecords != nullptr) {
			delete[]tableRecords;
		}
		tableRecords = new TableRecord*[offsetTable->numTables];
		for (int i = 0; i < offsetTable->numTables; i++) {
			tableRecords[i] = new TableRecord();

			for (int j = 0; j < 4; j++) {
				tableRecords[i]->tableTag[j] = stream->nextUint8();
			}
			tableRecords[i]->checkSum = stream->nextUint32();
			tableRecords[i]->offset = stream->nextUint32();
			tableRecords[i]->length = stream->nextUint32();
		}
		
		
		parseCmapTable(stream);

#define MAKE_NULL(t) if(t != nullptr){delete t; t = nullptr;}
		MAKE_NULL(headTable)
		MAKE_NULL(maxpTable);
		MAKE_NULL(locaTable);
#undef MAKE_NULL
		parseGlyfTable(stream);
		return true;
	}

	void exportGlyphs(const char* filename = "glyphs") {

		ofstream fileOut(filename, ios::binary);
		if (!fileOut) {
			cout << "error: can't open file: \"" << filename << "\"" << endl;
			return;
		}
		//glyph data
		GlyphBase** glyphs = glyfTable->glyphs;
		for (int i = 0; i < locaTable->n - 1; i++) {
			if (glyphs[i] != nullptr) {
				fileOut << glyphs[i]->dataAsString(headTable->unitsPerEm);
			}
			else {
				fileOut << "pass\n";
			}
		}
		//code map to index
		fileOut << "map:";
		for (int32 i = 0; i < 65536; i++) {
			fileOut << cmapTable->findGlyph(i);
			if (i != 65535)fileOut << ",";
		}
		fileOut << "\n";
		fileOut.close();
	}

private:

	TableRecord* findRecord(const char* tag) {
		if (offsetTable == nullptr || tableRecords == nullptr)return nullptr;
		for (int i = 0; i < offsetTable->numTables; i++) {
			TableRecord* record = tableRecords[i];
			bool flag = true;
			for (int i = 0; i < 4; i++) {
				if (tag[i] != record->tableTag[i]) {
					flag = false;
				}
			}
			if (flag) {
				return record;
			}
		}
		return nullptr;
	}

	void parseCmapTable(BigEndianToLittleEndianStream* stream) {
		TableRecord* cmapTableRecord = findRecord("cmap");
		if (cmapTableRecord == nullptr) {
			cout << "error: can't find camp table" << endl;
			return;
		}

		stream->setPos(cmapTableRecord->offset);
		CmapTableHeader* header = new CmapTableHeader(stream);
		
		EncodingRecord* record = header->findRecord(3, 1);
		if (record == nullptr) {
			cout << "error: can't find the encoding record" << endl;
			return;
		}

		stream->setPos(cmapTableRecord->offset + record->offset);
		cmapTable = new CmapSubtable_Format4(stream);
	}

	void parseGlyfTable(BigEndianToLittleEndianStream* stream) {
		TableRecord* glyfTableRecord = findRecord("glyf");
		if (glyfTableRecord == nullptr) {
			cout << "error: can't find glyf table" << endl;
			return;
		}

		if (headTable == nullptr) {
			parseHeadTable(stream);
			if (headTable == nullptr) {
				cout << "error: lack of head table" << endl;
				return;
			}
		}
		
		if (maxpTable == nullptr) {
			parseMaxpTable(stream);
			if (maxpTable == nullptr) {
				cout << "error: lack of maxp table" << endl;
				return;
			}
		}

		if (locaTable == nullptr) {
			parseLocaTable(stream);
			if (locaTable == nullptr) {
				cout << "error: lack of loca table" << endl;
				return;
			}
		}

		stream->setPos(glyfTableRecord->offset);
		glyfTable = new GlyfTable(stream, locaTable);
	}

	void parseHeadTable(BigEndianToLittleEndianStream* stream) {
		TableRecord* headTableRecord = findRecord("head");
		if (headTableRecord == nullptr) {
			cout << "error: can't find head table" << endl;
			return;
		}
		stream->setPos(headTableRecord->offset);
		headTable = new HeadTable(stream);
	}

	void parseMaxpTable(BigEndianToLittleEndianStream* stream) {
		TableRecord* maxpTableRecord = findRecord("maxp");
		if (maxpTableRecord == nullptr) {
			cout << "error: can't find maxp table" << endl;
			return;
		}
		stream->setPos(maxpTableRecord->offset);
		maxpTable = new MaxpTable(stream);
	}

	void parseLocaTable(BigEndianToLittleEndianStream* stream) {
		TableRecord* locaTableRecord = findRecord("loca");
		if (locaTableRecord == nullptr) {
			cout << "error: can't find loca table" << endl;
			return;
		}
		stream->setPos(locaTableRecord->offset);
		locaTable = new LocaTable(stream, maxpTable, headTable);
	}
};