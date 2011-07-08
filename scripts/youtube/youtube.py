import clementine

from PythonQt.Qt import QUrl
from PythonQt.QtGui import QAction
from PythonQt.QtGui import QDesktopServices
from PythonQt.QtNetwork import QNetworkRequest

import json

class Plugin:
  def __init__(self):
    self.action = QAction("Find on YouTube", None)
    self.action.connect("activated()", self.SearchYoutube)
    clementine.ui.AddAction("song_menu", self.action)
    self.network = clementine.NetworkAccessManager()

  def SearchYoutube(self):
    selection = clementine.playlists.current_selection().indexes()
    title = selection[clementine.Playlist.Column_Title].data()
    artist = selection[clementine.Playlist.Column_Artist].data()
    query = title + ' ' + artist
    url = QUrl('https://gdata.youtube.com/feeds/api/videos')
    url.addQueryItem('q', query)
    url.addQueryItem('alt', 'json')
    url.addQueryItem('max-results', 1)
    reply = self.network.get(QNetworkRequest(url))

    def SearchFinished():
      data = json.loads(str(reply.readAll()))
      feed = data['feed']
      try:
        print feed['entry'][0]['media$group']['media$player'][0]['url']
        youtube_url = feed['entry'][0]['media$group']['media$player'][0]['url']
        QDesktopServices.openUrl(QUrl.fromEncoded(str(youtube_url)))
      except Exception, e:
        print e

    reply.connect("finished()", SearchFinished)


plugin = Plugin()
