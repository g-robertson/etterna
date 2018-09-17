#include <tommath.h>
#ifdef BN_MP_OR_C
/* LibTomMath, multiple-precision integer library -- Tom St Denis
 *
 * LibTomMath is a library that provides multiple-precision
 * integer arithmetic as well as number theoretic functionality.
 *
 * The library was designed directly after the MPI library by
 * Michael Fromberger but has been written from scratch with
 * additional optimizations in place.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 *
 * Tom St Denis, tomstdenis@gmail.com, http://math.libtomcrypt.com
 */

/* OR two ints together */
int mp_or(mp_int *a, mp_int *b, mp_int *c)
{
	int res, ix, px;
	mp_int t, *x;

	if (a->used > b->used)
	{
		if ((res = mp_init_copy(&t, a)) != MP_OKAY)
		{
			return res;
		}
		px = b->used;
		x = b;
	}
	else
	{
		if ((res = mp_init_copy(&t, b)) != MP_OKAY)
		{
			return res;
		}
		px = a->used;
		x = a;
	}

	for (ix = 0; ix < px; ix++)
	{
		t.dp[ix] |= x->dp[ix];
	}
	mp_clamp(&t);
	mp_exch(c, &t);
	mp_clear(&t);
	return MP_OKAY;
}
#endif

/* $Source$ */
/* $Revision: 24838 $ */
/* $Date: 2007-01-23 23:16:57 -0600 (Tue, 23 Jan 2007) $ */
