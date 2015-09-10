/*
 * freerainbowtables is a project for generating, distributing, and using
 * perfect rainbow tables
 *
 * Copyright 2011 Steve Thomas (Sc00bz)
 * Copyright 2011, 2012 James Nobis <quel@quelrod.net>
 *
 * This file is part of freerainbowtables.
 *
 * freerainbowtables is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 *
 * freerainbowtables is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with freerainbowtables.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef ARCHITECTURE_H
#define ARCHITECTURE_H

#if defined (__GLIBC__)
	#include <endian.h>
	#if (__BYTE_ORDER == __LITTLE_ENDIAN)
		#define ARC_LITTLE_ENDIAN
	#elif (__BYTE_ORDER == __BIG_ENDIAN)
		#define ARC_BIG_ENDIAN
	#else
		#error "Unknown machine endianness"
	#endif
#elif defined(_BIG_ENDIAN) && !defined(_LITTLE_ENDIAN)
	#define ARC_BIG_ENDIAN
#elif defined(_LITTLE_ENDIAN) && !defined(_BIG_ENDIAN)
	#define ARC_LITTLE_ENDIAN
#elif defined(__sparc)  || defined(__sparc__)   || \
      defined(_POWER)   || defined(__powerpc__) || \
      defined(__ppc__)  || defined(__hpux)      || \
      defined(_MIPSEB)  || defined(_POWER)      || \
      defined(__s390__)
	#define ARC_BIG_ENDIAN
#elif defined(__i386__)  || defined(__alpha__)  || \
      defined(__ia64)    || defined(__ia64__)   || \
      defined(_M_IX86)   || defined(_M_IA64)    || \
      defined(_M_ALPHA)  || defined(__amd64)    || \
      defined(__amd64__) || defined(_M_AMD64)   || \
      defined(__x86_64)  || defined(__x86_64__) || \
      defined(_M_X64)
	#define ARC_LITTLE_ENDIAN
#else
	#error "Unknown machine endianness"
#endif

#if defined(_M_X64) || defined(__x86_64__)
	#define ARC_x86_64
	#define ARC_x86
#elif defined(_M_IX86) || defined(__i386__)
	#define ARC_x86_32
	#define ARC_x86
#endif

#if defined(_LP64)     || defined(__LP64__)   || \
    defined(__ia64)    || defined(__ia64__)   || \
    defined(_M_IA64)   || defined(__amd64)    || \
    defined(__amd64__) || defined(_M_AMD64)   || \
    defined(__x86_64)  || defined(__x86_64__) || \
    defined(_M_X64)    || defined(_WIN64)
	#define ARC_64
#endif

#endif
