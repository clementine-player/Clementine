import clementine

from servicebase import DigitallyImportedServiceBase

from PythonQt.QtCore    import QSettings, QUrl
from PythonQt.QtNetwork import QNetworkCookie, QNetworkCookieJar, QNetworkRequest

import logging

LOGGER = logging.getLogger("di.service")


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

  def __init__(self, model, settings_dialog_callback):
    DigitallyImportedServiceBase.Init(self, model, settings_dialog_callback)

    self.last_username_password = None
    self.MaybeReloadCookies()

  def MaybeReloadCookies(self):
    if self.last_username_password == (self.username, self.password):
      return
    self.last_username_password = (self.username, self.password)

    LOGGER.debug("Setting network cookies after config change")

    # If a username and password were set by the user then set them in the
    # cookies we pass to www.di.fm
    self.network = clementine.NetworkAccessManager(self)
    if len(self.username) and len(self.password):
      cookie_jar = QNetworkCookieJar()
      cookie_jar.setCookiesFromUrl([
        QNetworkCookie("_amember_ru", self.username.encode("utf-8")),
        QNetworkCookie("_amember_rp", self.password.encode("utf-8")),
      ], QUrl("http://www.di.fm/"))
      self.network.setCookieJar(cookie_jar)

  def LoadStation(self, key):
    self.MaybeReloadCookies()
    playlist_url = self.PLAYLISTS[self.audio_type]["url"] % key

    LOGGER.info("Getting playlist URL '%s'" % playlist_url)

    # Start fetching the playlist.  Can't use a SongLoader to do this because
    # we have to use the cookies we set in ReloadSettings()
    self.load_station_reply = self.network.get(QNetworkRequest(QUrl(playlist_url)))
    self.load_station_reply.connect("finished()", self.LoadPlaylistFinished)
