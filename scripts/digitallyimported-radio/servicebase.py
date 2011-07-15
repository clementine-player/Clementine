import clementine

import PythonQt
from PythonQt.QtCore    import QSettings, QUrl
from PythonQt.QtGui     import QAction, QDesktopServices, QIcon, QMenu, \
                               QStandardItem
from PythonQt.QtNetwork import QNetworkRequest

import json
import logging
import operator
import os.path
import weakref

LOGGER = logging.getLogger("di.servicebase")


class DigitallyImportedUrlHandler(clementine.UrlHandler):
  def __init__(self, url_scheme, service):
    clementine.UrlHandler.__init__(self, None)
    # Avoid circular references
    self.service_weakref = weakref.ref(service)
    self.url_scheme = url_scheme

    self.last_original_url = None
    self.task_id = None

  def scheme(self):
    return self.url_scheme

  def StartLoading(self, original_url):
    if self.service_weakref() is None:
      return
    service = self.service_weakref()

    result = clementine.UrlHandler_LoadResult()

    if self.task_id is not None:
      return result
    if service.PLAYLISTS[service.audio_type]["premium"] and \
       (len(service.username) == 0 or len(service.password) == 0):
      service.StreamError(self.tr("You have selected a Premium-only audio type but do not have any account details entered"))
      return result

    key = original_url.host()
    LOGGER.info("Loading station %s", key)
    service.LoadStation(key)

    # Save the original URL so we can emit it in the finished signal later
    self.last_original_url = original_url

    # Tell the user what's happening
    self.task_id = clementine.task_manager.StartTask(self.tr("Loading stream"))

    result.type_ = clementine.UrlHandler_LoadResult.WillLoadAsynchronously
    result.original_url_ = original_url
    return result

  def LoadPlaylistFinished(self, reply):
    if self.task_id is None:
      return
    if self.service_weakref() is None:
      return
    service = self.service_weakref()

    # Stop the spinner in the status bar
    clementine.task_manager.SetTaskFinished(self.task_id)
    self.task_id = None

    # Try to parse the playlist
    parser = clementine.PlaylistParser(clementine.library)
    songs = parser.LoadFromDevice(reply)

    LOGGER.info("Loading station finished, got %d songs", len(songs))

    # Failed to get the playlist?
    if len(songs) == 0:
      service.StreamError("Error loading playlist '%s'" % reply.url().toString())
      return

    result = clementine.UrlHandler_LoadResult()
    result.original_url_ = self.last_original_url

    # Take the first track in the playlist
    result.type_ = clementine.UrlHandler_LoadResult.TrackAvailable
    result.media_url_ = songs[0].url()

    self.AsyncLoadComplete(result)


class DigitallyImportedServiceBase(clementine.InternetService):
  # Set these in subclasses
  HOMEPAGE_URL = None
  HOMEPAGE_NAME = None
  STREAM_LIST_URL = None
  ICON_FILENAME = None
  SERVICE_NAME = None
  SERVICE_DESCRIPTION = None
  PLAYLISTS = []
  URL_SCHEME = None

  SETTINGS_GROUP = "digitally_imported"

  def Init(self, model, settings_dialog_callback):
    clementine.InternetService.__init__(self, self.SERVICE_NAME, model)

    # We must hold a weak reference to the callback or else it makes a circular
    # reference between the services and Plugin from main.py.
    self.settings_dialog_callback = weakref.ref(settings_dialog_callback)

    self.url_handler = DigitallyImportedUrlHandler(self.URL_SCHEME, self)
    clementine.player.RegisterUrlHandler(self.url_handler)

    self.network = clementine.NetworkAccessManager(None)
    self.path = os.path.dirname(__file__)

    self.audio_type = 0
    self.username = ""
    self.password = ""

    self.context_index = None
    self.menu = None
    self.root = None
    self.task_id = None
    self.refresh_streams_reply = None
    self.load_station_reply = None
    self.items = []

    self.ReloadSettings()

  def ReloadSettings(self):
    settings = QSettings()
    settings.beginGroup(self.SETTINGS_GROUP)

    self.audio_type = int(settings.value("audio_type", 0))
    self.username = unicode(settings.value("username", ""))
    self.password = unicode(settings.value("password", ""))

  def CreateRootItem(self):
    self.root = QStandardItem(QIcon(os.path.join(self.path, self.ICON_FILENAME)),
                              self.SERVICE_DESCRIPTION)
    self.root.setData(True, clementine.InternetModel.Role_CanLazyLoad)
    return self.root

  def LazyPopulate(self, parent):
    if parent == self.root:
      # Download the list of streams the first time the user expands the root
      self.RefreshStreams()

  def ShowContextMenu(self, index, global_pos):
    if not self.menu:
      self.menu = QMenu()

      for action in self.GetPlaylistActions():
          self.menu.addAction(action)
      self.menu.addAction(clementine.IconLoader.Load("download"),
        self.tr("Open " + self.HOMEPAGE_NAME + " in browser"), self.Homepage)
      self.menu.addAction(clementine.IconLoader.Load("view-refresh"),
        self.tr("Refresh streams"), self.RefreshStreams)

      self.menu.addSeparator()

      self.menu.addAction(clementine.IconLoader.Load("configure"),
        self.tr("Configure..."), self.settings_dialog_callback())

    self.context_index = index
    self.menu.popup(global_pos)

  def GetCurrentIndex(self):
    return self.context_index

  def Homepage(self):
    QDesktopServices.openUrl(self.HOMEPAGE_URL)

  def RefreshStreams(self):
    if self.task_id is not None:
      return

    LOGGER.info("Getting stream list from '%s'", self.STREAM_LIST_URL)

    # Request the list of stations
    self.refresh_streams_reply = self.network.get(QNetworkRequest(self.STREAM_LIST_URL))
    self.refresh_streams_reply.connect("finished()", self.RefreshStreamsFinished)

    # Give the user some indication that we're doing something
    self.task_id = clementine.task_manager.StartTask(self.tr("Getting streams"))

  def RefreshStreamsFinished(self):
    if self.refresh_streams_reply is None:
      return
    if self.task_id is None:
      return

    # Stop the spinner in the status bar
    clementine.task_manager.SetTaskFinished(self.task_id)
    self.task_id = None

    # Read the data and parse the json object inside
    json_data = self.refresh_streams_reply.readAll().data()
    streams = json.loads(json_data)

    # Sort by name
    streams = sorted(streams, key=operator.itemgetter("name"))

    LOGGER.info("Loaded %d streams", len(streams))

    # Now we have the list of streams, so clear any existing items in the list
    # and insert the new ones
    if self.root.hasChildren():
      self.root.removeRows(0, self.root.rowCount())

    for stream in streams:
      song = clementine.Song()
      song.set_title(stream["name"])
      song.set_artist(self.SERVICE_DESCRIPTION)
      song.set_url(QUrl("%s://%s" % (self.URL_SCHEME, stream["key"])))

      item = QStandardItem(QIcon(":last.fm/icon_radio.png"), stream["name"])
      item.setData(stream["description"], PythonQt.QtCore.Qt.ToolTipRole)
      item.setData(clementine.InternetModel.PlayBehaviour_SingleItem, clementine.InternetModel.Role_PlayBehaviour)
      item.setData(song, clementine.InternetModel.Role_SongMetadata)
      self.root.appendRow(item)

      # Keep references to the items otherwise Python will delete them
      self.items.append(item)

  def playlistitem_options(self):
    return clementine.PlaylistItem.Options(clementine.PlaylistItem.PauseDisabled)

  def LoadStation(self, key):
    raise NotImplementedError()

  def LoadPlaylistFinished(self):
    self.url_handler.LoadPlaylistFinished(self.load_station_reply)
