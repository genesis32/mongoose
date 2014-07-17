/*
 *  2DVector.h
 *  Armageddon
 *
 *  Created by ddm on 3/28/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef VECTOR2D_H
#define VECTOR2D_H

typedef float vec_t;
typedef vec_t vec2d_t[2];

typedef float pt_t;
typedef pt_t  pt2d_t[2];

void  Vec2_Normalize(vec2d_t vec);
void  Pt2_GetHeadingTo(vec2d_t outv, const pt2d_t frm, const pt2d_t to);

bool  Pt2_Equals(const pt2d_t pt1, const pt2d_t pt2);
float Pt2_Distance(const pt2d_t pt1, const pt2d_t pt2);

float Vec2_DotProduct(const vec2d_t vec1, const vec2d_t vec2);


#endif