import clementine

from PythonQt.QtCore    import QUrl
from PythonQt.QtNetwork import QNetworkRequest

import json
import urllib


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

  def __init__(self, parent):
    clementine.CoverProvider.__init__(self, "Google Images", parent)

    self.api_url = 'https://ajax.googleapis.com/ajax/services/search/images?{0}'
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
    print 'sending request'

    url = self.GetQueryURL(query)

    reply = self.network.get(QNetworkRequest(url))
    self.queries[reply] = query

    return reply

  def ParseReply(self, reply):
    print 'parsing reply'

    results = json.loads(str(reply.readAll()))

    parsed = []

    if 'Error' in results:
      return parsed

    query = self.queries.pop(reply)
    for result in results['responseData']['results']:
      current = clementine.CoverSearchResult()

      current.description = query
      current.image_url = result['url']

      parsed.append(current)

    return parsed

  def GetQueryURL(self, query):
    current_args = self.api_args.copy()
    current_args['q'] = query

    return QUrl(self.api_url.format(urllib.urlencode(current_args)))


script = GoogleImagesCoversScript()
