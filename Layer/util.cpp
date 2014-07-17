/*
 *  util.cpp
 *  Armageddon
 *
 *  Created by ddm on 4/11/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include "math.h"
#include "util.h"

// |error| < 0.005
float fast_atan2( float y, float x )
{
	if ( x == 0.0f )
	{
		if ( y > 0.0f ) return PIBY2_FLOAT;
		if ( y == 0.0f ) return 0.0f;
		return -PIBY2_FLOAT;
	}
	float atan;
	float z = y/x;
	if ( fabsf( z ) < 1.0f )
	{
		atan = z/(1.0f + 0.28f*z*z);
		if ( x < 0.0f )
		{
			if ( y < 0.0f ) return atan - PI_FLOAT;
			return atan + PI_FLOAT;
		}
	}
	else
	{
		atan = PIBY2_FLOAT - z/(z*z + 0.28f);
		if ( y < 0.0f ) return atan - PI_FLOAT;
	}
	return atan;
}

float fast_sqrt(float val)  {
	union
	{
		int tmp;
		float val;
	} u;
	u.val = val;
	u.tmp -= 1<<23; /* Remove last bit so 1.0 gives 1.0 */
	/* tmp is now an approximation to logbase2(val) */
	u.tmp >>= 1; /* divide by 2 */
	u.tmp += 1<<29; /* add 64 to exponent: (e+127)/2 =(e/2)+63, */
	/* that represents (e/2)-64 but we want e/2 */
	return u.val;
}

