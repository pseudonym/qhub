/* 
 * Base32 Encoder stolen from GPL'd DC++:
 * Copyright (C) 2001-2003 Jacek Sieka, j_s@telia.com
 *
 * CRC32 Encoder stolen from GPL'd cksfv:
 * Copyright (C) 2000 Bryan Call <bc@fodder.org>
 * 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef _ENCODER
#define _ENCODER

#include <string>
using namespace std;

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class Encoder
{
public:
	static string& toBase32(const u_int8_t* src, size_t len, string& tgt);
	static string toBase32(const u_int8_t* src, size_t len) {
		string tmp;
		return toBase32(src, len, tmp);
	}
	static void fromBase32(const char* src, u_int8_t* dst, size_t len);
	static u_int32_t toCrc32(const char* p, size_t len);
	static u_int32_t toCrc32(string const& str) { return toCrc32(str.c_str(), str.length()); };
private:
	static const int8_t base32Table[];
	static const char base32Alphabet[];
	static const u_int32_t crc32Table[256];
};

#endif // _ENCODER

/**
 * @file
 * $Id: Encoder.h,v 1.3 2004/03/09 12:20:19 arnetheduck Exp $
 */
