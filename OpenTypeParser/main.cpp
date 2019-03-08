// OpenTypeParser.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include"OpenTypeParser.h"
vector<string>* split(const string& str, const char& ch);
void printUsage();
int main()
{
	OpenTypeParser* parser = new OpenTypeParser();
	string cmd = "";
	printUsage();
	cout << ">>>";
	while (getline(cin, cmd)) {
		vector<string>* args = split(cmd, ' ');
		if (args->size() < 2 || args->at(0) != "p") {
			cout << "error: arguments too less" << endl;
			printUsage();
		}
		else {
			if (!parser->parse(args->at(1).c_str())) {
				cout << "parse failed" << endl;
				delete args;
				cout << ">>>";
				continue;
			};
			if (args->size() == 2) {
				parser->exportGlyphs();
			}
			else {
				parser->exportGlyphs(args->at(2).c_str());
			}
		}
		cout << "parse successfully" << endl;
		delete args;
		cout << ">>>";
	}
}
void printUsage() {
	cout << "usage: p <font_file_to_be_parsed> [output_file]" << endl;
	cout << "example1: p D:\\a.ttf" << endl;
	cout << "example2: p D:\\a.ttf D:\\a.txt" << endl;
	cout << "default output filename: \"glyphs\"" << endl;
}

vector<string>* split(const string& str, const char& ch) {
	vector<string>* ret = new vector<string>();
	size_t offset = 0;
	int pos = -1;
	while ((pos = str.find_first_of(ch, offset)) != string::npos) {
		if (pos != offset) {
			ret->push_back(str.substr(offset, pos - offset));
		}
		offset = pos + 1;
	}
	if (offset != str.length()) {
		ret->push_back(str.substr(offset, str.length() - offset));
	}

	return ret;
}

