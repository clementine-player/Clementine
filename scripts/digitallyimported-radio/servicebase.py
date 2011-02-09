import clementine

from PyQt4.QtCore    import QSettings, QUrl
from PyQt4.QtGui     import QAction, QDesktopServices, QIcon, QMenu, \
                            QStandardItem
from PyQt4.QtNetwork import QNetworkRequest
import PyQt4.QtCore

import json
import operator
import os.path

class DigitallyImportedServiceBase(clementine.RadioService):
  # Set these in subclasses
  HOMEPAGE_URL = None
  HOMEPAGE_NAME = None
  STREAM_LIST_URL = None
  ICON_FILENAME = None
  SERVICE_NAME = None
  SERVICE_DESCRIPTION = None
  PLAYLISTS = []

  SETTINGS_GROUP = "digitally_imported"

  SettingsDialogRequested = PyQt4.QtCore.pyqtSignal()

  def __init__(self, model):
    clementine.RadioService.__init__(self, self.SERVICE_NAME, model)

    self.network = clementine.NetworkAccessManager(self)
    self.path = os.path.dirname(__file__)

    self.audio_type = 0
    self.username = ""
    self.password = ""

    self.context_index = None
    self.last_original_url = None
    self.menu = None
    self.root = None
    self.task_id = None

    self.ReloadSettings()

  def ReloadSettings(self):
    settings = QSettings()
    settings.beginGroup(self.SETTINGS_GROUP)

    self.audio_type = int(settings.value("audio_type", 0).toPyObject())
    self.username = unicode(settings.value("username", "").toPyObject().toUtf8())
    self.password = unicode(settings.value("password", "").toPyObject().toUtf8())

  def CreateRootItem(self):
    self.root = QStandardItem(QIcon(os.path.join(self.path, self.ICON_FILENAME)),
                              self.SERVICE_DESCRIPTION)
    self.root.setData(True, clementine.RadioModel.Role_CanLazyLoad)
    return self.root

  def LazyPopulate(self, parent):
    if parent == self.root:
      # Download the list of streams the first time the user expands the root
      self.RefreshStreams()

  def ShowContextMenu(self, index, global_pos):
    if not self.menu:
      self.menu = QMenu()
      self.menu.addAction(clementine.IconLoader.Load("media-playback-start"),
        self.tr("Append to current playlist"), self.AddToPlaylist)
      self.menu.addAction(clementine.IconLoader.Load("media-playback-start"),
        self.tr("Replace current playlist"), self.LoadToPlaylist)
      self.menu.addAction(clementine.IconLoader.Load("document-new"),
        self.tr("Open in new playlist"), self.OpenInNewPlaylist)

      self.menu.addSeparator()

      self.menu.addAction(clementine.IconLoader.Load("download"),
        self.tr("Open " + self.HOMEPAGE_NAME + " in browser"), self.Homepage)
      self.menu.addAction(clementine.IconLoader.Load("view-refresh"),
        self.tr("Refresh streams"), self.RefreshStreams)

      self.menu.addSeparator()

      self.menu.addAction(clementine.IconLoader.Load("configure"),
        self.tr("Configure..."), self.SettingsDialogRequested.emit)

    self.context_index = index
    self.menu.popup(global_pos)

  def AddToPlaylist(self):
    self.AddItemToPlaylist(self.context_index, False)

  def LoadToPlaylist(self):
    self.AddItemToPlaylist(self.context_index, True)

  def OpenInNewPlaylist(self):
    self.AddItemToPlaylist(self.context_index, False, False, True)

  def Homepage(self):
    QDesktopServices.openUrl(self.HOMEPAGE_URL)

  def RefreshStreams(self):
    if self.task_id is not None:
      return

    # Request the list of stations
    reply = self.network.get(QNetworkRequest(self.STREAM_LIST_URL))
    reply.finished.connect(self.RefreshStreamsFinished)

    # Give the user some indication that we're doing something
    self.task_id = clementine.task_manager.StartTask(self.tr("Getting streams"))

  def RefreshStreamsFinished(self):
    # Get the QNetworkReply that called this slot
    reply = self.sender()
    reply.deleteLater()

    if self.task_id is None:
      return

    # Stop the spinner in the status bar
    clementine.task_manager.SetTaskFinished(self.task_id)
    self.task_id = None

    # Read the data and parse the json object inside
    json_data = reply.readAll().data()
    streams = json.loads(json_data)

    # Sort by name
    streams = sorted(streams, key=operator.itemgetter("name"))

    # Now we have the list of streams, so clear any existing items in the list
    # and insert the new ones
    if self.root.hasChildren():
      self.root.removeRows(0, self.root.rowCount())

    for stream in streams:
      item = QStandardItem(QIcon(":last.fm/icon_radio.png"), stream["name"])
      item.setData(stream["description"], PyQt4.QtCore.Qt.ToolTipRole)
      item.setData("digitallyimported://%s" % stream["key"], clementine.RadioModel.Role_Url)
      item.setData(clementine.RadioModel.PlayBehaviour_SingleItem, clementine.RadioModel.Role_PlayBehaviour)
      item.setData(stream["name"], clementine.RadioModel.Role_Title)
      item.setData(self.SERVICE_DESCRIPTION, clementine.RadioModel.Role_Artist)
      self.root.appendRow(item)

  def playlistitem_options(self):
    return clementine.PlaylistItem.Options(
      clementine.PlaylistItem.SpecialPlayBehaviour |
      clementine.PlaylistItem.PauseDisabled)

  def StartLoading(self, original_url):
    result = clementine.PlaylistItem.SpecialLoadResult()

    if self.task_id is not None:
      return result
    if original_url.scheme() != "digitallyimported":
      return result
    if self.PLAYLISTS[self.audio_type]["premium"] and \
       (len(self.username) == 0 or len(self.password) == 0):
      self.StreamError.emit(self.tr("You have selected a Premium-only audio type but do not have any account details entered"))
      return result

    key = original_url.host()
    self.LoadStation(key)

    # Save the original URL so we can emit it in the finished signal later
    self.last_original_url = original_url

    # Tell the user what's happening
    self.task_id = clementine.task_manager.StartTask(self.tr("Loading stream"))

    result.type_ = clementine.PlaylistItem.SpecialLoadResult.WillLoadAsynchronously
    result.original_url_ = original_url
    return result

  def LoadStation(self, key):
    raise NotImplementedError()

  def LoadPlaylistFinished(self):
    # Get the QNetworkReply that called this slot
    reply = self.sender()
    reply.deleteLater()

    if self.task_id is None:
      return

    # Stop the spinner in the status bar
    clementine.task_manager.SetTaskFinished(self.task_id)
    self.task_id = None

    # Try to parse the playlist
    parser = clementine.PlaylistParser(clementine.library)
    songs = parser.Load(reply)

    # Failed to get the playlist?
    if len(songs) == 0:
      self.StreamError.emit("Error loading playlist '%s'" % reply.url().toString())
      return

    result = clementine.PlaylistItem.SpecialLoadResult()
    result.original_url_ = self.last_original_url

    # Take the first track in the playlist
    result.type_ = clementine.PlaylistItem.SpecialLoadResult.TrackAvailable
    result.media_url_ = QUrl(songs[0].filename())

    self.AsyncLoadFinished.emit(result)
