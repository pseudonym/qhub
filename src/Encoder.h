/* 
 * Base32 Encoder stolen from GPL'd DC++:
 * Copyright (C) 2001-2003 Jacek Sieka, j_s@telia.com
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

#ifndef QHUB_ENCODER_H
#define QHUB_ENCODER_H

#include <string>
#include <cassert>

namespace qhub {

class Encoder
{
public:
	static std::string& toBase32(const u_int8_t* src, size_t len, std::string& tgt);
	static std::string toBase32(const u_int8_t* src, size_t len) {
		std::string tmp;
		return toBase32(src, len, tmp);
	}
	/*
	 * convert a single 5 bit value to base32 character
	 */
	static char toBase32(uint8_t src)
	{
		assert(src < 32);
		return base32Alphabet[src];
	}

	static void fromBase32(const char* src, u_int8_t* dst, size_t len);
	static uint8_t fromBase32(char ch)
	{
		return base32Table[uint8_t(ch)];
	}
private:
	static const int8_t base32Table[];
	static const char base32Alphabet[];
};

} // namespace qhub

#endif // QHUB_ENCODER_H
