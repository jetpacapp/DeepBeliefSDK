/*
Math-NEON:  Neon Optimised Math Library based on cmath
Contact:    lachlan.ts@gmail.com
Copyright (C) 2009  Lachlan Tychsen - Smith aka Adventus

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 3 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef __MATH_NEON_H__ 
#define __MATH_NEON_H__ 

#if !defined(__i386__) && defined(__arm__)
//if defined neon ASM routines are used, otherwise all calls to *_neon 
//functions are rerouted to their equivalent *_c function.
#define __MATH_NEON			

//Default Floating Point value ABI: 0=softfp, 1=hardfp. Only effects *_neon routines.
//You can access the hardfp versions directly via the *_hard suffix. 
//You can access the softfp versions directly via the *_soft suffix. 
#define __MATH_FPABI 	1	

#endif

#ifdef GCC
#define ALIGN(A) __attribute__ ((aligned (A))
#else
#define ALIGN(A)
#endif

#ifndef _MATH_H
#define M_PI		3.14159265358979323846	/* pi */
#define M_PI_2		1.57079632679489661923	/* pi/2 */
#define M_PI_4		0.78539816339744830962	/* pi/4 */
#define M_E			2.7182818284590452354	/* e */
#define M_LOG2E		1.4426950408889634074	/* log_2 e */
#define M_LOG10E	0.43429448190325182765	/* log_10 e */
#define M_LN2		0.69314718055994530942	/* log_e 2 */
#define M_LN10		2.30258509299404568402	/* log_e 10 */
#define M_1_PI		0.31830988618379067154	/* 1/pi */
#define M_2_PI		0.63661977236758134308	/* 2/pi */
#define M_2_SQRTPI	1.12837916709551257390	/* 2/sqrt(pi) */
#define M_SQRT2		1.41421356237309504880	/* sqrt(2) */
#define M_SQRT1_2	0.70710678118654752440	/* 1/sqrt(2) */
#endif 

#if __MATH_FPABI == 1
#define powf_neon		powf_neon_hfp
#else
#define powf_neon		powf_neon_sfp
#endif

 
/* 
function:	powf
return: 	x raised to the power of n, x ** n.
expression: r = x ** y	
notes:		computed using e ** (y * ln(x))
*/
float 		powf_c(float x, float n);
float 		powf_neon_sfp(float x, float n);
float 		powf_neon_hfp(float x, float n);

#endif
