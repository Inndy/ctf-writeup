/*
 * freerainbowtables is a project for generating, distributing, and using
 * perfect rainbow tables
 *
 * Copyright 2009, 2010 Daniël Niggebrugge <niggebrugge@fox-it.com>
 * Copyright 2009, 2010, 2011, 2012 James Nobis <quel@quelrod.net>
 *
 * This file is part of freerainbowtables.
 *
 * freerainbowtables is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * freerainbowtables is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with freerainbowtables.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _GLOBAL_H
#define _GLOBAL_H

#ifndef __STDC_FORMAT_MACROS
	#define __STDC_FORMAT_MACROS /* for PRI macros */
#endif

#ifndef __STDC_LIMIT_MACROS
	#define __STDC_LIMIT_MACROS /* for uint64_t, UINT64_MAX, etc. */
#endif

#include <stdio.h>
#include <stdint.h>
#if defined(_WIN32) && !defined(__GNUC__)
	#include "inttypes.h"
#else
	#include "/usr/include/inttypes.h"
#endif

/*
 * XXX
 * add universal support for unsigned 64-bit int values
 * 0x00FFFFFFFFFFFFFFI64
 * 0x00FFFFFFFFFFFFFFllu
 */

#if defined(_WIN32) && !defined(__GNUC__)
	#ifndef INT64_MAX
		#error "INT64_MAX not defined"
		#define int64_t __int64
	#endif

	#ifndef UINT64_MAX
		#error "UINT64_MAX not defined"
		#define uint64_t unsigned __int64
	#endif

	#ifndef UINT32_MAX
		#error "UINT32_MAX not defined"
		#define uint32_t unsigned __int32
	#endif

	#ifndef UINT16_MAX
		#error "UINT16_MAX not defined"
		#define uint16_t unsigned short
	#endif

	#ifndef UINT8_MAX
		#error "UINT8_MAX not defined"
		#define uint8_t unsigned char
	#endif
#else
	#ifndef INT64_MAX
		#error "INT64_MAX not defined"
	#endif

	#ifndef UINT64_MAX
		#error "UINT64_MAX not defined"
	#endif

	#ifndef UINT32_MAX
		#error "UINT32_MAX not defined"
	#endif

	#ifndef UINT16_MAX
		#error "UINT16_MAX not defined"
	#endif

	#ifndef UINT8_MAX
		#error "UINT8_MAX not defined"
	#endif
#endif

struct uint24_t
{
	uint8_t data[3];
};

#endif /* !GLOBAL_H */
