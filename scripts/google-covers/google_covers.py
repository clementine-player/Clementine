import clementine

from PythonQt.QtCore    import QUrl
from PythonQt.QtNetwork import QNetworkRequest

import json
import logging
import urllib

LOGGER = logging.getLogger("google_images")


class GoogleImagesCoverProvider(clementine.CoverProvider):
  API_URL = 'https://ajax.googleapis.com/ajax/services/search/images?{0}'

  def __init__(self, parent=None):
    clementine.CoverProvider.__init__(self, "Google Images", parent)

    self.api_args = {
      'v'     : '1.0',
      # at most five results
      'rsz'   : '5',
      # only larger sizes
      'imgsz' : 'large|xlarge'
    }
    self.network = clementine.NetworkAccessManager()

  def StartSearch(self, artist, album, id):
    url = self.GetQueryURL(artist + " " + album)
    LOGGER.info("Id %d - sending request to '%s'" % (id, url))

    reply = self.network.get(QNetworkRequest(url))

    def QueryFinished():
      LOGGER.debug("Id %d - finished" % id)

      self.SearchFinished(id, self.ParseReply(artist, album, reply))

    reply.connect("finished()", QueryFinished)
    return True

  def ParseReply(self, artist, album, reply):
    results = json.loads(str(reply.readAll()))

    parsed = []

    if "responseStatus" not in results or results["responseStatus"] != 200:
      LOGGER.warning("Error parsing reply: %s", results["responseDetails"])
      return parsed

    query = "%s - %s" % (artist, album)

    LOGGER.info("Parsing reply for query '%s'" % query)
    for result in results['responseData']['results']:
      current = clementine.CoverSearchResult()

      current.description = query
      current.image_url = result['url']

      parsed.append(current)

    return parsed

  def GetQueryURL(self, query):
    current_args = self.api_args.copy()
    current_args['q'] = query

    return QUrl(self.API_URL.format(urllib.urlencode(current_args)))


provider = GoogleImagesCoverProvider()
clementine.cover_providers.AddProvider(provider)
