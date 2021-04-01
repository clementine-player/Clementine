/* This file is part of Clementine.
   Copyright 2021, Jim Broadus <jbroadus@gmail.com>

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

#include "internetsettingscategory.h"

#include "internet/digitally/digitallyimportedsettingspage.h"
#include "internet/magnatune/magnatunesettingspage.h"
#include "internet/podcasts/podcastsettingspage.h"
#include "internet/radiobrowser/radiobrowsersettingspage.h"
#include "internet/subsonic/subsonicsettingspage.h"
#include "internetshowsettingspage.h"

#ifdef HAVE_LIBLASTFM
#include "internet/lastfm/lastfmsettingspage.h"
#endif

#ifdef HAVE_GOOGLE_DRIVE
#include "internet/googledrive/googledrivesettingspage.h"
#endif

#ifdef HAVE_DROPBOX
#include "internet/dropbox/dropboxsettingspage.h"
#endif

#ifdef HAVE_BOX
#include "internet/box/boxsettingspage.h"
#endif

#ifdef HAVE_SKYDRIVE
#include "internet/skydrive/skydrivesettingspage.h"
#endif

#ifdef HAVE_SEAFILE
#include "internet/seafile/seafilesettingspage.h"
#endif

#ifdef HAVE_SPOTIFY
#include "internet/spotify/spotifysettingspage.h"
#endif

InternetSettingsCategory::InternetSettingsCategory(SettingsDialog* dialog)
    : SettingsCategory(SettingsDialog::Page_InternetShow,
                       new InternetShowSettingsPage(dialog), dialog) {
  AddChildren();
}

void InternetSettingsCategory::AddChildren() {
#ifdef HAVE_LIBLASTFM
  AddPage(SettingsDialog::Page_Lastfm, new LastFMSettingsPage(dialog_));
#endif

#ifdef HAVE_GOOGLE_DRIVE
  AddPage(SettingsDialog::Page_GoogleDrive,
          new GoogleDriveSettingsPage(dialog_));
#endif

#ifdef HAVE_DROPBOX
  AddPage(SettingsDialog::Page_Dropbox, new DropboxSettingsPage(dialog_));
#endif

#ifdef HAVE_BOX
  AddPage(SettingsDialog::Page_Box, new BoxSettingsPage(dialog_));
#endif

#ifdef HAVE_SKYDRIVE
  AddPage(SettingsDialog::Page_Skydrive, new SkydriveSettingsPage(dialog_));
#endif

#ifdef HAVE_SPOTIFY
  AddPage(SettingsDialog::Page_Spotify, new SpotifySettingsPage(dialog_));
#endif

#ifdef HAVE_SEAFILE
  AddPage(SettingsDialog::Page_Seafile, new SeafileSettingsPage(dialog_));
#endif

#ifdef HAVE_AMAZON_CLOUD_DRIVE
  AddPage(SettingsDialog::Page_AmazonCloudDrive,
          new AmazonSettingsPage(dialog_));
#endif

  AddPage(SettingsDialog::Page_Magnatune, new MagnatuneSettingsPage(dialog_));
  AddPage(SettingsDialog::Page_DigitallyImported,
          new DigitallyImportedSettingsPage(dialog_));
  AddPage(SettingsDialog::Page_Subsonic, new SubsonicSettingsPage(dialog_));
  AddPage(SettingsDialog::Page_Podcasts, new PodcastSettingsPage(dialog_));
  AddPage(SettingsDialog::Page_RadioBrowser,
          new RadioBrowserSettingsPage(dialog_));

  sortChildren(0, Qt::AscendingOrder);
}
