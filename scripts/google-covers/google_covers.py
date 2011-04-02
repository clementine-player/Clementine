import clementine

from PyQt4.QtCore    import QString, QUrl
from PyQt4.QtNetwork import QNetworkRequest

from simplejson import loads
import urllib

class GoogleImagesCoversScript(clementine.CoverProvider):

  def __init__(self):
    clementine.CoverProvider.__init__(self, "Google Images")

    self.api_url = 'https://ajax.googleapis.com/ajax/services/search/images?{0}'
    self.api_args = {
      'v'   : '1.0',
      # at most five results
      'rsz' : '5'
    }
    self.network = clementine.NetworkAccessManager(self)    
    self.queries = {}

    # register in the repository of cover providers
    clementine.cover_providers.AddCoverProvider(self)

  def SendRequest(self, query):
    url = self.GetQueryURL(query)

    reply = self.network.get(QNetworkRequest(url))
    self.queries[reply] = query

    return reply

  def ParseReply(self, reply):
    results = loads(str(reply.readAll()))

    parsed = []

    if 'Error' in results:
      return parsed

    query = self.queries.pop(reply)
    for result in results['responseData']['results']:
      current = clementine.CoverSearchResult()
      current.description = query
      current.image_url = QString(result['url'])

      parsed.append(current)

    return parsed

  def GetQueryURL(self, query):
    current_args = self.api_args.copy()
    current_args['q'] = query

    return QUrl(self.api_url.format(urllib.urlencode(current_args)))


script = GoogleImagesCoversScript()
