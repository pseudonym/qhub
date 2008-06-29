/* 
 * Copyright (C) 2001-2003 Jacek Sieka, j_s@telia.com
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

#ifndef QHUB_TIGERHASH_H
#define QHUB_TIGERHASH_H

#include "types.h"

namespace qhub {

class TigerHash {
public:
	/** Hash size in bytes */
	enum { HASH_SIZE = 24 };

	TigerHash();
	~TigerHash() {}

	/** Calculates the Tiger hash of the data. */
	void update(const void* data, uint32_t len);
	/** Call once all data has been processed. */
	uint8_t* finalize();

	uint8_t* getResult() { return reinterpret_cast<uint8_t*>(res); };
private:
	enum { BLOCK_SIZE = 512/8 };
	/** 512 bit blocks for the compress function */
	uint8_t tmp[BLOCK_SIZE];
	/** State / final hash value */
	uint64_t res[3];
	/** Total number of bytes compressed */
	uint64_t pos;
};

} // namespace qhub

#endif // QHUB_TIGERHASH_H
