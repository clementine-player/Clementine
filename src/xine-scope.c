/* Author: Max Howell <max.howell@methylblue.com>, (C) 2004
   Copyright: See COPYING file that comes with this distribution

   This has to be a c file or for some reason it won't link! (GCC 3.4.1)
*/

/* gcc doesn't like inline for me */
#define inline
/* need access to port_ticket */
#define XINE_ENGINE_INTERNAL

#include "xine-scope.h"
#include <xine/post.h>
#include <xine/xine_internal.h>

typedef struct scope_plugin_s scope_plugin_t;

struct scope_plugin_s
{
    post_plugin_t post;

    metronom_t metronom;
    int        channels;
    MyNode    *list;
};

/*************************
 * post plugin functions *
 *************************/

static int
scope_port_open( xine_audio_port_t *port_gen, xine_stream_t *stream, uint32_t bits, uint32_t rate, int mode )
{
    #define port ((post_audio_port_t*)port_gen)
    #define this ((scope_plugin_t*)((post_audio_port_t*)port_gen)->post)

    _x_post_rewire( (post_plugin_t*)port->post );
    _x_post_inc_usage( port );

    port->stream = stream;
    port->bits = bits;
    port->rate = rate;
    port->mode = mode;

    this->channels = _x_ao_mode2channels( mode );

    return port->original_port->open( port->original_port, stream, bits, rate, mode );
}

static void
scope_port_close( xine_audio_port_t *port_gen, xine_stream_t *stream )
{
    MyNode *node;

    /* ensure the buffers are deleted during the next XineEngine::timerEvent() */
    for( node = this->list->next; node != this->list; node = node->next )
        node->vpts = node->vpts_end = -1;

    port->stream = NULL;
    port->original_port->close( port->original_port, stream );

    _x_post_dec_usage( port );
}

static void
scope_port_put_buffer( xine_audio_port_t *port_gen, audio_buffer_t *buf, xine_stream_t *stream )
{
/* FIXME With 8-bit samples the scope won't work correctly. For a special 8-bit code path,
         the sample size could be checked like this: if( port->bits == 8 ) */

    const int num_samples = buf->num_frames * this->channels;
    metronom_t *myMetronom = &this->metronom;
    MyNode *new_node;

    /* I keep my own metronom because xine wouldn't for some reason */
    memcpy( &this->metronom, stream->metronom, sizeof(metronom_t) );

    new_node             = malloc( sizeof(MyNode) );
    new_node->vpts       = myMetronom->got_audio_samples( myMetronom, buf->vpts, buf->num_frames );
    new_node->num_frames = buf->num_frames;
    new_node->mem        = malloc( num_samples * 2 );
    memcpy( new_node->mem, buf->mem, num_samples * 2 );

    {
        int64_t
        K  = myMetronom->pts_per_smpls; /*smpls = 1<<16 samples*/
        K *= num_samples;
        K /= (1<<16);
        K += new_node->vpts;

        new_node->vpts_end = K;
    }

    port->original_port->put_buffer( port->original_port, buf, stream );

    /* finally we should append the current buffer to the list
     * this is thread-safe due to the way we handle the list in the GUI thread */
    new_node->next   = this->list->next;
    this->list->next = new_node;

    #undef port
    #undef this
}

static void
scope_dispose( post_plugin_t *this )
{
    MyNode *list = ((scope_plugin_t*)this)->list;
    MyNode *prev;
    MyNode *node = list;

    /* Free all elements of the list (a ring buffer) */
    do {
        prev = node->next;

        free( node->mem );
        free( node );

        node = prev;
    }
    while( node != list );


    free( this );
}


/************************
 * plugin init function *
 ************************/

xine_post_t*
scope_plugin_new( xine_t *xine, xine_audio_port_t *audio_target )
{
    scope_plugin_t *scope_plugin = xine_xmalloc( sizeof(scope_plugin_t) );
    post_plugin_t  *post_plugin  = (post_plugin_t*)scope_plugin;

    {
        post_in_t         *input;
        post_out_t        *output;
        post_audio_port_t *port;

        _x_post_init( post_plugin, 1, 0 );

        port = _x_post_intercept_audio_port( post_plugin, audio_target, &input, &output );
        port->new_port.open       = scope_port_open;
        port->new_port.close      = scope_port_close;
        port->new_port.put_buffer = scope_port_put_buffer;

        post_plugin->xine_post.audio_input[0] = &port->new_port;
        post_plugin->xine_post.type = PLUGIN_POST;

        post_plugin->dispose = scope_dispose;
    }

    /* code is straight from xine_init_post()
       can't use that function as it only dlopens the plugins
       and our plugin is statically linked in */

    post_plugin->running_ticket = xine->port_ticket;
    post_plugin->xine = xine;

    /* scope_plugin_t init */
    scope_plugin->list = xine_xmalloc( sizeof(MyNode) );
    scope_plugin->list->next = scope_plugin->list;

    return &post_plugin->xine_post;
}

MyNode*
scope_plugin_list( void *post )
{
    return ((scope_plugin_t*)post)->list;
}

int
scope_plugin_channels( void *post )
{
    return ((scope_plugin_t*)post)->channels;
}

metronom_t*
scope_plugin_metronom( void *post )
{
    return &((scope_plugin_t*)post)->metronom;
}
