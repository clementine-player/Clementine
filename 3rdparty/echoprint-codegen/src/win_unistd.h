#ifndef _UNISTD_H
#define _UNISTD_H        1

/* This file intended to serve as a drop-in replacement for
 *  unistd.h on Windows
 *  Please add functionality as neeeded
 http://stackoverflow.com/questions/341817/is-there-a-replacement-for-unistd-h-for-windows-visual-c
 */

#include <stdlib.h>
#include <io.h>
/*
#include <getopt.h>

 getopt from: http://www.pwilson.net/sample.html. */

#define srandom srand
#define random rand

#define F_OK 0

#define W_OK 2
#define R_OK 4

#define access _access
#define ftruncate _chsize

/* stdio */
#define popen _popen
#define pclose _pclose
/* float.h */
#define finite _finite

#define ssize_t int

#endif /* unistd.h  */

