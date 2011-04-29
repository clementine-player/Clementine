import clementine

from servicebase import DigitallyImportedServiceBase

from PyQt4.QtCore    import QSettings, QUrl
from PyQt4.QtNetwork import QNetworkCookie, QNetworkCookieJar, QNetworkRequest

class DigitallyImportedService(DigitallyImportedServiceBase):
  HOMEPAGE_URL = QUrl("http://www.di.fm/")
  HOMEPAGE_NAME = "di.fm"
  STREAM_LIST_URL = QUrl("http://listen.di.fm/")
  ICON_FILENAME = "icon-small.png"
  SERVICE_NAME = "DigitallyImported"
  SERVICE_DESCRIPTION = "Digitally Imported"
  URL_SCHEME = "digitallyimported"

  # These have to be in the same order as in the settings dialog
  PLAYLISTS = [
    {"premium": False, "url": "http://listen.di.fm/public3/%s.pls"},
    {"premium": True,  "url": "http://www.di.fm/listen/%s/premium.pls"},
    {"premium": False, "url": "http://listen.di.fm/public2/%s.pls"},
    {"premium": True,  "url": "http://www.di.fm/listen/%s/64k.pls"},
    {"premium": True,  "url": "http://www.di.fm/listen/%s/128k.pls"},
    {"premium": False, "url": "http://listen.di.fm/public5/%s.asx"},
    {"premium": True,  "url": "http://www.di.fm/listen/%s/64k.asx"},
    {"premium": True,  "url": "http://www.di.fm/listen/%s/128k.asx"},
  ]

  def __init__(self, model):
    DigitallyImportedServiceBase.__init__(self, model)

  def ReloadSettings(self):
    DigitallyImportedServiceBase.ReloadSettings(self)

    # If a username and password were set by the user then set them in the
    # cookies we pass to www.di.fm
    self.network = clementine.NetworkAccessManager(self)
    if len(self.username) and len(self.password):
      cookie_jar = QNetworkCookieJar()
      cookie_jar.setCookiesFromUrl([
        QNetworkCookie("_amember_ru", self.username),
        QNetworkCookie("_amember_rp", self.password),
      ], QUrl("http://www.di.fm/"))
      self.network.setCookieJar(cookie_jar)

  def LoadStation(self, key):
    playlist_url = self.PLAYLISTS[self.audio_type]["url"] % key

    # Start fetching the playlist.  Can't use a SongLoader to do this because
    # we have to use the cookies we set in ReloadSettings()
    reply = self.network.get(QNetworkRequest(QUrl(playlist_url)))
    reply.finished.connect(self.LoadPlaylistFinished)
