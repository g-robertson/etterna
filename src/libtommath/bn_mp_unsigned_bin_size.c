#include <tommath.h>
#ifdef BN_MP_UNSIGNED_BIN_SIZE_C
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

/* get the size for an unsigned equivalent */
int mp_unsigned_bin_size(mp_int *a)
{
	int size = mp_count_bits(a);
	return (size / 8 + ((size & 7) != 0 ? 1 : 0));
}
#endif

/* $Source$ */
/* $Revision: 24838 $ */
/* $Date: 2007-01-23 23:16:57 -0600 (Tue, 23 Jan 2007) $ */
