#pragma once

#define int8 char
#define int16 short
#define int32 int
#define uint8 unsigned char
#define uint16 unsigned short
#define uint32 unsigned int
#define Fixed int32 //32-bit signed fixed-point number (16.16)
#define FWORD int16  //describes a quantity in font design units
#define UFWORD uint16 //describes a quantity in font design units
#define F2DOT14 int16 //16-bit signed fixed number with the low 14 bits of fraction (2.14).
#define LONGDATETIME long long //Date represented in number of seconds since 12:00 midnight, January 1, 1904. The value is represented as a signed 64-bit integer.
#define Offset16 uint16 //Short offset to a table, same as uint16, NULL offset = 0x0000
#define Offset32 uint32 //Long offset to a table, same as uint32, NULL offset = 0x00000000

typedef uint8 Tag[4];//Array of four uint8s (length = 32 bits) used to identify a table, design-variation axis, script, language system, feature, or baseline
