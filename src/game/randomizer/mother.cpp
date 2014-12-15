/*****************************************************************************
					The Dark Mod GPL Source Code

					This file is part of the The Dark Mod Source Code, originally based
					on the Doom 3 GPL Source Code as published in 2011.

					The Dark Mod Source Code is free software: you can redistribute it
					and/or modify it under the terms of the GNU General Public License as
					published by the Free Software Foundation, either version 3 of the License,
					or (at your option) any later version. For details, see LICENSE.TXT.

					Project: The Dark Mod (http://www.thedarkmod.com/)

					$Revision$ (Revision of last commit)
					$Date$ (Date of last commit)
					$Author$ (Author of last commit)

					******************************************************************************/
/************************** MOTHER.CPP ****************** AgF 1999-03-03 *
*  'Mother-of-All' random number generator                               *
*                                                                        *
*  This is a multiply-with-carry type of random number generator         *
*  invented by George Marsaglia.  The algorithm is:                      *
*  S = 2111111111*X[n-4] + 1492*X[n-3] + 1776*X[n-2] + 5115*X[n-1] + C   *
*  X[n] = S modulo 2^32                                                  *
*  C = floor(S / 2^32)                                                   *
*                                                                        *
*  IMPORTANT:
*  This implementation uses a long double for C. Note that not all       *
*  computers and compilers support the long double (80-bit) floating     *
*  point format. It is recommended to use a Borland or Gnu compiler on   *
*  a PC. The Microsoft compiler doesn't support the long double format.  *
*  You will get an error message if your system doesn't support this.    *
*                                                                        *
* � 2002 A. Fog. GNU General Public License www.gnu.org/copyleft/gpl.html*
*************************************************************************/

#include "precompiled_game.h"
#pragma hdrstop

static bool versioned = RegisterVersionedFile( "$Id$" );

#include "randomc.h"

// constructor:
TRandomMotherOfAll::TRandomMotherOfAll( uint32 seed ) {
	// Check that compiler supports 80-bit long double:
	assert( sizeof( long double ) > 9 );
	// initialize
	RandomInit( seed );
}

// returns a random number between 0 and 1:
double TRandomMotherOfAll::Random() {
	long double c;
	c = ( long double )2111111111.0 * x[3] +
		1492.0 * ( x[3] = x[2] ) +
		1776.0 * ( x[2] = x[1] ) +
		5115.0 * ( x[1] = x[0] ) +
		x[4];
	x[4] = floorl( c );
	x[0] = c - x[4];
	x[4] = x[4] * ( 1. / ( 65536.*65536. ) );
	return x[0];
}

// returns integer random number in desired interval:
int TRandomMotherOfAll::IRandom( int min, int max ) {
	int iinterval = max - min + 1;
	if( iinterval <= 0 ) {
		return 0x80000000;    // error
	}
	int i = int( iinterval * Random() ); // truncate
	if( i >= iinterval ) {
		i = iinterval - 1;
	}
	return min + i;
}

// this function initializes the random number generator:
void TRandomMotherOfAll::RandomInit( uint32 seed ) {
	int i;
	uint32 s = seed;
	// make random numbers and put them into the buffer
	for( i = 0; i < 5; i++ ) {
		s = s * 29943829 - 1;
		x[i] = s * ( 1. / ( 65536.*65536. ) );
	}
	// randomize some more
	for( i = 0; i < 19; i++ ) {
		Random();
	}
}