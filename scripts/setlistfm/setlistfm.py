import json
import sys

from traceback import print_exc

from PyQt4.QtCore import QObject
from PyQt4.QtCore import QString
from PyQt4.QtCore import QUrl
from PyQt4.QtCore import QVariant
from PyQt4.QtCore import SIGNAL
from PyQt4.QtGui import QAction
from PyQt4.QtNetwork import QNetworkRequest

import clementine
import urllib


class SetlistFmScript(QObject):

  def __init__(self):
    QObject.__init__(self)

    # maps QNetworkReply to artist of action which created it
    # every thread knows what artist it's looking for
    self.artist_map = {}

    self.task_id = None
    self.network = clementine.NetworkAccessManager(self)

    self.action = QAction("fill_with_setlist", self)
    self.action.setText("Load latest setlist")
    self.connect(self.action, SIGNAL("activated()"), self.load_setlist_activated)

    clementine.ui.AddAction('library_context_menu', self.action)

  def load_setlist_activated(self):
    # wait for the last call to finish
    if self.task_id is not None:
      return
    
    # find the first artist
    artist = ""
    for song in clementine.library_view.GetSelectedSongs():
      if len(song.artist()) > 0:
        artist = str(song.artist())
        break

    # ignore the call if there's no artist in selection
    if len(artist) == 0:
      return

    # start the progress spinner
    self.task_id = clementine.task_manager.StartTask(self.tr("Getting setlist"))

    # finally - request for the setlists of artist
    reply = self.network.get(QNetworkRequest(self.get_setlist_fm_url(artist)))
    self.artist_map[reply] = artist

    reply.finished.connect(self.load_setlist_activated_finalize)

  def load_setlist_activated_finalize(self):
    reply = self.sender()
    reply.deleteLater()

    if self.task_id is None:
      return

    # stop the progress spinner
    clementine.task_manager.SetTaskFinished(self.task_id)
    self.task_id = None

    artist = self.artist_map.pop(reply)
      
    # get the titles of songs from the latest setlist available
    # on setlist.fm for the artist
    titles = self.parse_setlist_fm_reply(reply.readAll().data())

    if len(titles) == 0:
      return

    # we uppercase the titles to make the plugin case insensitive
    titles = map(lambda title: title.upper(), titles)

    # get song ids for titles
    query = clementine.LibraryQuery()

    query.SetColumnSpec('ROWID, title')
    query.AddWhere('UPPER(artist)', artist.upper())
    query.AddWhere('UPPER(title)', titles, 'IN')

    # maps titles to ids; also removes possible title duplicates
    # from the query
    title_map = {}
    if clementine.library.ExecQuery(query):
      while query.Next():
        # be super cautious and throw out the faulty ones
        to_int = query.Value(0).toInt()
        to_str = query.Value(1).toString()
        
        if to_int[1]:
          title_map[str(to_str).upper()] = to_int[0]

    if len(title_map) > 0:
      # get complete song objects for ids
      from_lib = clementine.library.GetSongsById(title_map.values())

      # maps ids to song objects
      lib_title_map = {}
      for song in from_lib:
        lib_title_map[song.id()] = song

      unavailable = []

      into_playlist = []
      # iterate over titles to preserve ordering of songs
      # in the setlist
      for title in titles:
        try:
          # fill the list
          into_playlist.append(lib_title_map[title_map[title]])
        except KeyError:
          # TODO: do something with songs not in library?
          unavailable.append(title)

      # finally - fill the playlist with songs!
      current = clementine.playlists.current()
      if current != None and len(into_playlist) > 0:
        current.InsertLibraryItems(into_playlist)

  def parse_setlist_fm_reply(self, response):
    try:
      response = json.loads(response)

      count = response['setlists']['@total']
      # only if we have at least one set
      if count != None and count > 0:
        # look for the first one with any information
        for setlist in response['setlists']['setlist']:
          result = []

          sets = setlist['sets']
          if len(sets) > 0:
            # this may or may not be an array - make sure it always is!
            final_sets = sets['set'] if len(sets['set']) > 1 else [sets['set']]

            # from all the sets (like main + encore)
            for set in final_sets:
              # this may or may not be an array - make sure it always is!
              final_songs = set['song'] if len(set['song']) > 1 else [set['song']]
              
              for song in final_songs:
                result.append(song['@name'])
              
          if len(result) > 0:
            return result

      return []
    
    except:
      print "Unexpected error:", sys.exc_info()[0]
      print_exc()
      
      return (None, [])

  def get_setlist_fm_url(self, artist):
    return QUrl('http://api.setlist.fm/rest/0.1/search/setlists.json?artistName=' + urllib.quote(artist))
		

script = SetlistFmScript()