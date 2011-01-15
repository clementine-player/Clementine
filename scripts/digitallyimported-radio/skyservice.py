import clementine

from servicebase import DigitallyImportedServiceBase

from PyQt4.QtCore    import QSettings, QString, QUrl
from PyQt4.QtNetwork import QNetworkCookie, QNetworkCookieJar, QNetworkRequest

import re

class SkyFmService(DigitallyImportedServiceBase):
  HOMEPAGE_URL = QUrl("http://www.sky.fm/")
  HOMEPAGE_NAME = "sky.fm"
  STREAM_LIST_URL = QUrl("http://listen.sky.fm/")
  ICON_FILENAME = "icon-sky.png"
  SERVICE_NAME = "sky_fm"
  SERVICE_DESCRIPTION = "SKY.fm"

  HASHKEY_RE = re.compile(r'hashKey\s*=\s*\'([0-9a-f]+)\'')

  # These have to be in the same order as in the settings dialog
  PLAYLISTS = [
    {"premium": False, "url": "http://listen.sky.fm/public3/%s.pls"},
    {"premium": True,  "url": "http://listen.sky.fm/premium_high/%s.pls?hash=%s"},
    {"premium": False, "url": "http://listen.sky.fm/public1/%s.pls"},
    {"premium": True,  "url": "http://listen.sky.fm/premium_medium/%s.pls?hash=%s"},
    {"premium": True,  "url": "http://listen.sky.fm/premium/%s.pls?hash=%s"},
    {"premium": False, "url": "http://listen.sky.fm/public5/%s.asx"},
    {"premium": True,  "url": "http://listen.sky.fm/premium_wma_low/%s.asx?hash=%s"},
    {"premium": True,  "url": "http://listen.sky.fm/premium_wma/%s.asx?hash=%s"},
  ]

  def __init__(self, model):
    DigitallyImportedServiceBase.__init__(self, model)

    self.last_key = None

  def LoadStation(self, key):
    # Non-premium streams can just start loading straight away
    if not self.PLAYLISTS[self.audio_type]["premium"]:
      self.LoadPlaylist(key)
      return

    # Otherwise we have to get the user's hashKey
    request = QNetworkRequest(QUrl("http://www.sky.fm/configure_player.php"))
    postdata = "amember_login=%s&amember_pass=%s" % (
      QUrl.toPercentEncoding(self.username),
      QUrl.toPercentEncoding(self.password))

    reply = self.network.post(request, postdata)
    reply.finished.connect(self.LoadHashKeyFinished)

    self.last_key = key

  def LoadHashKeyFinished(self):
    # Get the QNetworkReply that called this slot
    reply = self.sender()
    reply.deleteLater()

    # Parse the hashKey out of the reply
    data = QString.fromUtf8(reply.readAll())
    match = self.HASHKEY_RE.search(data)

    if match:
      hash_key = match.group(1)
    else:
      clementine.task_manager.SetTaskFinished(self.task_id)
      self.task_id = None
      self.StreamError.emit(self.tr("Invalid SKY.fm username or password"))
      return

    # Now we can load the playlist
    self.LoadPlaylist(self.last_key, hash_key)

  def LoadPlaylist(self, key, hash_key=None):
    playlist_url = self.PLAYLISTS[self.audio_type]["url"]

    if hash_key:
      playlist_url = playlist_url % (key, hash_key)
    else:
      playlist_url = playlist_url % key

    # Start fetching the playlist.  Can't use a SongLoader to do this because
    # we have to use the cookies we set in ReloadSettings()
    reply = self.network.get(QNetworkRequest(QUrl(playlist_url)))
    reply.finished.connect(self.LoadPlaylistFinished)
