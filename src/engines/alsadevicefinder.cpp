/* This file is part of Clementine.
   Copyright 2017, Jonas Kvinge <jonas@jkvinge.net>

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

#include <alsa/asoundlib.h>
#include <errno.h>
#include <stdio.h>
#include <boost/scope_exit.hpp>

#include <QList>
#include <QString>
#include <QVariant>

#include <core/logging.h>

#include "alsadevicefinder.h"
#include "devicefinder.h"

AlsaDeviceFinder::AlsaDeviceFinder() : DeviceFinder("alsasink") {}

QList<DeviceFinder::Device> AlsaDeviceFinder::ListDevices() {
  QList<Device> ret;

  snd_pcm_stream_name(SND_PCM_STREAM_PLAYBACK);

  int card = -1;
  snd_ctl_card_info_t* cardinfo;
  snd_ctl_card_info_alloca(&cardinfo);
  while (true) {
    int result = snd_card_next(&card);
    if (result < 0) {
      qLog(Error) << "Unable to get soundcard:" << snd_strerror(result);
      break;
    }
    if (card < 0) break;

    char str[32];
    snprintf(str, sizeof(str)-1, "hw:%d", card);

    snd_ctl_t* handle;
    result = snd_ctl_open(&handle, str, 0);
    if (result < 0) {
      qLog(Error) << "Unable to open soundcard" << card << ":"
                  << snd_strerror(result);
      continue;
    }
    BOOST_SCOPE_EXIT(&handle) { snd_ctl_close(handle); }
    BOOST_SCOPE_EXIT_END

    result = snd_ctl_card_info(handle, cardinfo);
    if (result < 0) {
      qLog(Error) << "Control hardware failure for card" << card << ":"
                  << snd_strerror(result);
      continue;
    }

    int dev = -1;
    snd_pcm_info_t* pcminfo;
    snd_pcm_info_alloca(&pcminfo);
    while (true) {
      result = snd_ctl_pcm_next_device(handle, &dev);
      if (result < 0) {
        qLog(Error) << "Unable to get PCM for card" << card << ":"
                    << snd_strerror(result);
        continue;
      }
      if (dev < 0) break;

      snd_pcm_info_set_device(pcminfo, dev);
      snd_pcm_info_set_subdevice(pcminfo, 0);
      snd_pcm_info_set_stream(pcminfo, SND_PCM_STREAM_PLAYBACK);

      result = snd_ctl_pcm_info(handle, pcminfo);
      if (result < 0) {
        if (result != -ENOENT)
          qLog(Error) << "Unable to get control digital audio info for card"
                      << card << ":" << snd_strerror(result);
        continue;
      }

      Device device;
      device.description = QString("%1 %2")
                               .arg(snd_ctl_card_info_get_name(cardinfo))
                               .arg(snd_pcm_info_get_name(pcminfo));
      device.device_property_value = QString("hw:%1,%2").arg(card).arg(dev);
      device.icon_name = GuessIconName(device.description);
      ret.append(device);
    }
  }

  snd_config_update_free_global();

  return ret;
}
