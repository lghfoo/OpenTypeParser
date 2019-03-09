
# Open Type Parser  
## 简介  
>根据[微软文档](https://docs.microsoft.com/en-us/typography/opentype/spec/)，将OpenType文件解析成由直线和贝塞尔曲线构成的图形数据文件，并根据解析得到的图形数据文件用OpenGL描绘出文字的轮廓。

## 演示
* 解析字体文件  
![image](https://github.com/lghfoo/OpenTypeParser/blob/master/Demo/parse.gif)
* 根据图形数据描绘轮廓
	* 普通unicode字符（符号、数字和字母）  
	![image](https://github.com/lghfoo/OpenTypeParser/blob/master/Demo/deparse_unicode_symbol.gif)
	* 普通unicode字符（中文）  
	![image](https://github.com/lghfoo/OpenTypeParser/blob/master/Demo/deparse_unicode_ch.gif)
	* [iconfont](https://www.iconfont.cn/)  
	![image](https://github.com/lghfoo/OpenTypeParser/blob/master/Demo/deparse_iconfont.gif)

## 输入
基本支持Win10的Fonts文件夹下的单一字体文件以及iconfont字体文件
## 解析
1. OpenType文件是用大端的方式存储数据，所以有BigEndianToLittleEndianStream这个类将数据从大端转为小端  

2. [OpenType](https://docs.microsoft.com/en-us/typography/opentype/spec/otff)文件是由一系列的表格组成的。普通的OpenType文件大概可以分为三部分

|部分|说明|
|----|----|
|OffsetTable|大小为12个字节，从整个文件的0字节处开始，记录表格数目等信息|
|TableRecords|紧跟在OffsetTable之后，有多少个表格，就有多少个TableRecord。每个TableRecord记录着表格的名字、大小、检验和以及表格在文件中的偏移量|
|Tables|一系列的表格(OS/2、maxp、cmap、glyf...)|
4. 目前已解析的表格有
	* cmap
	* head
	* loca
	* maxp
	* glyf

5. 表格简介  

* **cmap**：将unicode映射为glyph的id，映射过程需要用到的成员有

	|成员|说明|
	|----|----|
	|segCountX2|段的数目\*2，数据是分成许多段的，表格没有直接存储段的数目，而是存储段的数目*2，将其除以2后即可得段的数目|
	|endCode[]|endCode[i]表示第i段的结束unicode|
	|startCode[]|startCode[i]表示第i段的起始unicode|
	|idDelta[]|idDelta[i]表示第i段的id偏移量|
	|idRangeOffset[]|idRangeOffset[i]用来计算第i段在glyphIdArray中的偏移量|
	|glyphIdArray[]|存储glyphId的数组|　　
　　在解析cmap表格时，表格中除了glyphIdArray[]的其他数组的长度都是segCountX2/2，而glyphIdArray[]的大小是通过整个表格的大小减去表格中其他数据的大小得到的。  
　　将unicode映射为glyphId的过程比较复杂。OpenType文件中存储着一系列的glyph，根据存储的顺序，第0个glyph的的glyphId就是0。微软给出的[映射过程](https://docs.microsoft.com/en-us/typography/opentype/spec/cmap)如下：
>Each segment is described by a startCode and endCode, along with an idDelta and an idRangeOffset, which are used for mapping the character codes in the segment. The segments are sorted in order of increasing endCode values, and the segment values are specified in four parallel arrays. You search for the first endCode that is greater than or equal to the character code you want to map. If the corresponding startCode is less than or equal to the character code, then you use the corresponding idDelta and idRangeOffset to map the character code to a glyph index (otherwise, the missingGlyph is returned). For the search to terminate, the final start code and endCode values must be 0xFFFF. This segment need not contain any valid mappings. (It can just map the single character code 0xFFFF to missingGlyph). However, the segment must be present.
If the idRangeOffset value for the segment is not 0, the mapping of character codes relies on glyphIdArray. The character code offset from startCode is added to the idRangeOffset value. This sum is used as an offset from the current location within idRangeOffset itself to index out the correct glyphIdArray value. This obscure indexing trick works because glyphIdArray immediately follows idRangeOffset in the font file. The C expression that yields the glyph index is:
```C
glyphId = *(idRangeOffset[i]/2 + (c - startCount[i]) + &idRangeOffset[i])
```
>The value  _c_  is the character code in question, and  _i_  is the segment index in which  _c_  appears. If the value obtained from the indexing operation is not 0 (which indicates missingGlyph), idDelta[i] is added to it to get the glyph index. The idDelta arithmetic is modulo 65536.
>If the idRangeOffset is 0, the idDelta value is added directly to the character code offset (i.e.  `idDelta[i] + c`) to get the corresponding glyph index. Again, the idDelta arithmetic is modulo 65536.

由此可得
```C
//将unicode映射为glyphId
int32 findGlyph(uint16 code) {
	uint16 segCount = segCountX2 / 2;
	//遍历每个段
	for (uint16 i = 0; i < segCount; i++) {
		//如果存在i使得startCode[i]<=code && code<=endCode[i]
		if (endCode[i] >= code) {
			if (startCode[i] <= code) {

				//那么首先判断idRangeOffset[i]是否为0
				if (idRangeOffset[i] != 0) {
					//如果idRangeOffset[i]不为0
					//那么根据映射公式
					//glyphId = *(idRangeOffset[i]/2 + (c - startCount[i]) + &idRangeOffset[i])
					//来获取glyphId
					//上面的公式的前提是glyphIdArray immediately follows idRangeOffset
					//也就glyphIdArray和idRangeOffset在内存里是紧密相邻的
					//然而在这里glyphIdArray和idRangeOffset是分别new出来的，所以无法直接使用公式
					//此时可以证明 idRangeOffset[i]/2 + &idRangeOffset[i] == idRangeOffset[i]/2 + i - segCount
					//所以可得索引为 (idRangeOffset[i] / 2 + i - segCount) + code - startCode[i];
					uint16 index = (idRangeOffset[i] / 2 + i - segCount) + code - startCode[i];
					if (index < glyphIdArrayLength) {
						//最后还要加上idDelta[i]
						return glyphIdArray[index] + idDelta[i];
					}
					return -1;
				}
				//如果idRangeOffset[i]为0，那么直接返回idDelta[i] + code
				return idDelta[i] + code;
			}
			else return -1;
		}
	}
	return -1;
}
```
---
* [**glyf**](https://docs.microsoft.com/en-us/typography/opentype/spec/glyf)：存储每个glyph的数据。每一个文字都对应着一个glyph。解析glyf表格的过程非常复杂，它依赖于另外一个表格：

	* loca：存储着每个glyph在glyf表格中的偏移量  
	
	而loca表格又依赖另外两个表格：
	* maxp：记录着glyph的个数
	* head：记录着loca中offset的数据格式，有offset32和offset16两种格式

	解析完loca、maxp和head三个表格后，就可以解析glyf表格了。一共有两种glyph，一种是简单的glyph；另一种是复合的glyph，由多个简单的glyph组成。每个glyph都有一个header记录着基本信息，这里简单介绍一下，具体请参考微软文档以及"[**Table_glyf.h**](https://github.com/lghfoo/OpenTypeParser/blob/master/OpenTypeParser/Table_glyf.h)"：
	* 简单的glyph
		* 由多个contour组成
		* contour由多个点组成
		* contour的每个点都有一个flag（但表格中存储的flag数目是不确定的，因为它经过压缩），还有xy值（x、y值的数目也是不确定的，与flag有关；格式也是不确定的，可能有符号，可能无符号，可能1个字节，也可能2个字节，取决于flag），每个点可能位于直线上，也可能是贝塞尔曲线的控制点

		解析过程如下：
		```C
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
		```
	* 复合的glyph
		 * 由多个简单glyph组成
		 * 多个简单glyph组成复合glyph时，每个简单glyph可能要进行平移或者缩放等操作

		具体的解析过程见"[**Table_glyf.h**](https://github.com/lghfoo/OpenTypeParser/blob/master/OpenTypeParser/Table_glyf.h)"
## 输出
输出文件的内容是按块划分的，有的块只有一行，有的块有多行，总共有四种块
* [pass]：只有一行，内容为pass，表示空的glyph
* [contour num:n]：简单glyph，第一行是contour num:n，表示由n个contour构成，每个contour包含一行flags和一行points，所以这个块一共占据1+n\*2行
* [component num:n]：复合glyph，第一行是component num:n，表示由n个简单glyph构成，每个glyph有三行：glyph index、xy offsets或者pin point、scale，所以这个块一共占据1+n\*3行
* [map]：只有一行，只出现一次，且出现在文件的最后。是一个数组，长度为65536。map[i]表示unicode为i的文字所对应的glyphId。

对于简单的glyph的一个contour的flags和points：
flags的长度就是点的个数，如果flags[i]==1，那么points[i]在直线的端点上；如果flags[i]==0，那么points[i]在贝塞尔曲线的控制点上。contour只由直线和二阶贝塞尔曲线构成。如果flags[i]==0并且flags[i+1]==0，那么说明有两条贝塞尔曲线相接，这两条曲线的接点会被省略。根据贝塞尔曲线的[数学特点](https://pages.mtu.edu/~shene/COURSES/cs3621/NOTES/spline/Bezier/bezier-der.html)，当两条同阶的曲线相接时，（微软文档中很难找到关于这一点的说明，我猜）为了保证一阶导连续，接点应该处于两个控制点的中心，也就是说可以根据两条曲线的控制点推导出接点，所以这个接点在字体文件中是被省略的，在这里也是被省略的，所以会出现flags中连续出现两个0的情况。

>如果有  
flags:111  
points:p0,p1,p2  
那么这个contour由三条直线构成，分别是(p0->p1)，(p1->p2)，(p2->p0)  

>如果有  
flags:101  
points:p0,p1,p2  
那么这个contour由一条贝塞尔曲线和一条直线构成，分别是(p0->p1->p2)，(p2->p0)  

>如果有  
flags:100  
points:p0,p1,p2  
那么这个contour由两个贝塞尔曲线构成，分别是(p0->p1->(p1+p2)/2)，((p1+p2)/2->p2->p0)  

具体的格式可以参考"[OpenTypeParser.h](https://github.com/lghfoo/OpenTypeParser/blob/master/OpenTypeParser/OpenTypeParser.h)"中的**exportGlyphs**方法  
解析输出文件的过程可以参考[GlyphDataParser](https://github.com/lghfoo/GlyphDataParser)中的"[glyphdataparser.h](https://github.com/lghfoo/GlyphDataParser/blob/master/glyphdataparser.h)"文件

## 补充
[GlyphDataParser](https://github.com/lghfoo/GlyphDataParser)只是临时用来检验输出的文件内容是否正确，只实现了描绘文字轮廓的功能。对于中文字体，解析的过程会比较慢，大约需要1-2分钟。