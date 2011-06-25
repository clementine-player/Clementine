#ifndef __WIN_FUNCS_H
#define __WIN_FUNCS_H 1

#include <math.h>
#include <winsock.h>
#include <sys/timeb.h>

#define ROUND_FUNC(type,suff) inline type round##suff(type x) \
{                                                             \
	if (x >= 0.0##suff){                                      \
		type y = floor##suff(x);                              \
		if (x - y >= 0.5##suff)                               \
			y += 1.0##suff;                                   \
		return y;                                             \
    }else{                                                    \
		type y = ceil##suff(x);                               \
		if (y - x >= 0.5##suff)                               \
			y -= 1.0##suff;                                   \
      return y;                                               \
    }                                                         \
}

ROUND_FUNC(float,f)
ROUND_FUNC(double,)


inline void gettimeofday(struct timeval* t,void* timezone)
{       struct _timeb timebuffer;
        _ftime( &timebuffer );
        t->tv_sec=timebuffer.time;
        t->tv_usec=1000*timebuffer.millitm;
}

#endif /* __WIN_FUNCS_H */
