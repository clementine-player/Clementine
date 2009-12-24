/* Author: Max Howell <max.howell@methylblue.com>, (C) 2004
   Copyright: See COPYING file that comes with this distribution

   This has to be a c file or for some reason it won't link! (GCC 3.4.1)
*/

#ifndef XINESCOPE_H
#define XINESCOPE_H

/* need access to some stuff for scope time stamping */
#define METRONOM_INTERNAL

#include <sys/types.h>
#include <xine/metronom.h>

typedef struct my_node_s MyNode;

struct my_node_s
{
    MyNode  *next;
    int16_t *mem;
    int      num_frames;
    int64_t  vpts;
    int64_t  vpts_end;
};

#ifdef __cplusplus
extern "C"
{
#endif
    xine_post_t*
    scope_plugin_new( xine_t*, xine_audio_port_t* );

    /* we sacrifice type-safety here because some GCCs appear broken
     * and choke on redefining the xine_post_t typedef
     */

    MyNode*
    scope_plugin_list( void* );

    int
    scope_plugin_channels( void* );

    metronom_t*
    scope_plugin_metronom( void* );
#ifdef __cplusplus
}
#endif

#endif
