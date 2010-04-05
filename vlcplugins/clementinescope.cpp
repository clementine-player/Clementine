/* This file is part of Clementine.

   Clementine is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Clementine is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Clementine.  If not, see <http://www.gnu.org/licenses/>.
*/

#define __PLUGIN__
#define MODULE_STRING "clementine_scope"
#define MODULE_NAME clementine_scope

#include "engines/vlcengine.h"

#include <vlc/vlc.h>
#include <vlc/plugins/vlc_common.h>
#include <vlc/plugins/vlc_plugin.h>
#include <vlc/plugins/vlc_aout.h>

#include <QtGlobal>

// Forward declarations
static int  Open         ( vlc_object_t * );
static void DoWork( aout_instance_t *p_aout, aout_filter_t *p_filter,
                    aout_buffer_t *p_in_buf, aout_buffer_t *p_out_buf );

// Module definition
vlc_module_begin ()
  set_description("Internal plugin for Clementine's analyzer")
  set_shortname("clementine_scope")
  set_category( CAT_AUDIO )
  set_subcategory( SUBCAT_AUDIO_VISUAL )
  set_capability( "audio filter", 0 )
  set_callbacks( Open, 0 )
  add_shortcut( "clementine_scope")
vlc_module_end ()


// Called once for every track
static int Open( vlc_object_t *p_this ) {
  aout_filter_t     *p_filter = (aout_filter_t *)p_this;

  if( ( p_filter->input.i_format != VLC_FOURCC('f','l','3','2') &&
        p_filter->input.i_format != VLC_FOURCC('f','i','3','2') ) ) {
    return VLC_EGENERIC;
  }

  p_filter->pf_do_work = DoWork;
  p_filter->b_in_place= 1;

  return VLC_SUCCESS;
}

// Called continuously while a track is playing
static void DoWork( aout_instance_t *p_aout, aout_filter_t *p_filter,
                    aout_buffer_t *p_in_buf, aout_buffer_t *p_out_buf ) {
  Q_UNUSED(p_aout);
  Q_UNUSED(p_out_buf);
  VlcEngine::SetScopeData(
      reinterpret_cast<float*>(p_in_buf->p_buffer),
      p_in_buf->i_nb_samples * aout_FormatNbChannels(&p_filter->input));
}
