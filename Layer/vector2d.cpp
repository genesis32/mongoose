/*
 *  2DVector.cpp
 *  Armageddon
 *
 *  Created by ddm on 3/28/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include "util.h"
#include "Vector2d.h"

void Vec2_Normalize(vec2d_t vec)
{
	float len = fast_sqrt(vec[0] * vec[0] + vec[1] * vec[1]);
	vec[0] /= len;
	vec[1] /= len;
}

void Pt2_GetHeadingTo(vec2d_t outv, const pt2d_t frm, const pt2d_t to)
{
	outv[0] = to[0] - frm[0];
	outv[1] = to[1] - frm[1];
	
	Vec2_Normalize(outv);
}

bool Pt2_Equals(const pt2d_t pt1, const pt2d_t pt2)
{
	return pt2[0] == pt1[0] && pt2[1] == pt1[1]; 
}

float Vec2_DotProduct(const vec2d_t vec1, const vec2d_t vec2)
{
	return (vec1[0] * vec2[0] + vec1[1] * vec2[1]);
}

float Pt2_Distance(const pt2d_t pt1, const pt2d_t pt2)
{
	float xd = pt2[0] - pt1[0]; 
	float yd = pt2[1] - pt1[1];
	
	float distance = fast_sqrt(xd*xd + yd*yd);
	
	return distance;	
}

