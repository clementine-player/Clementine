import clementine

from PythonQt.QtCore    import QUrl
from PythonQt.QtNetwork import QNetworkRequest

import json
import logging
import urllib

LOGGER = logging.getLogger("google_images")


class GoogleImagesCoversScript():
  def __init__(self):
    # create and register our factory
    self.factory = GoogleImagesCoverProviderFactory()


class GoogleImagesCoverProviderFactory(clementine.CoverProviderFactory):
  def __init__(self):
    clementine.CoverProviderFactory.__init__(self)
    # register in the repository of factories
    clementine.cover_providers.AddProviderFactory(self)

  def CreateCoverProvider(self, parent):
    return GoogleImagesCoverProvider(parent)


class GoogleImagesCoverProvider(clementine.CoverProvider):
  API_URL = 'https://ajax.googleapis.com/ajax/services/search/images?{0}'

  def __init__(self, parent):
    clementine.CoverProvider.__init__(self, "Google Images", parent)

    self.api_args = {
      'v'     : '1.0',
      # at most five results
      'rsz'   : '5',
      # only larger sizes
      'imgsz' : 'large|xlarge'
    }
    self.network = clementine.NetworkAccessManager(self)
    self.queries = {}

  def SendRequest(self, query):
    url = self.GetQueryURL(query)
    LOGGER.info("Sending request to '%s'", url)

    reply = self.network.get(QNetworkRequest(url))
    self.queries[reply] = query

    return reply

  def ParseReply(self, reply):
    results = json.loads(str(reply.readAll()))

    parsed = []

    if "responseStatus" not in results or results["responseStatus"] != 200:
      LOGGER.warning("Error parsing reply: %s", results["responseDetails"])
      return parsed

    query = self.queries[reply]
    LOGGER.info("Parsing reply for query '%s'", query)
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


script = GoogleImagesCoversScript()
