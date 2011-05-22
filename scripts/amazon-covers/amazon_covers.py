import clementine

from PythonQt.QtCore    import QUrl, QString
from PythonQt.QtNetwork import QNetworkRequest
from xml.etree.ElementTree import fromstring

import urllib
import time
import hashlib
import base64
import hmac


class AmazonCoversScript():

  def __init__(self):
    # create and register our factory
    self.factory = AmazonCoverProviderFactory()


class AmazonCoverProviderFactory(clementine.CoverProviderFactory):

  def __init__(self):
    clementine.CoverProviderFactory.__init__(self)
    # register in the repository of factories
    clementine.cover_providers.AddProviderFactory(self)

  def CreateCoverProvider(self, parent):
    return AmazonCoverProvider(parent)


class AmazonCoverProvider(clementine.CoverProvider):
  """
  Most of the Amazon API related code here comes from a plugin (which I wrote) for
  an open source application called Cardapio.
  """

  def __init__(self):
    clementine.CoverProvider.__init__(self, "Amazon")

    self.api_url = 'http://ecs.amazonaws.com/onca/xml?{0}'

    self.aws_access_key = 'AKIAJ4QO3GQTSM3A43BQ'
    self.aws_secret_access_key = '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'

    # basic API's arguments (search in all categories)
    self.api_base_args = {
      'Service'       : 'AWSECommerceService',
      'Version'       : '2009-11-01',
      'Operation'     : 'ItemSearch',
      'SearchIndex'   : 'All',
      'ResponseGroup' : 'Images',
      'AWSAccessKeyId': self.aws_access_key
    }
    self.network = clementine.NetworkAccessManager(self)    

    # register in the repository of cover providers
    clementine.cover_providers.AddCoverProvider(self)

  def SendRequest(self, query):
    url = QUrl.fromEncoded(self.api_url.format(self.PrepareAmazonRESTUrl(query)))
    return self.network.get(QNetworkRequest(url))

  def ParseReply(self, reply):
    parsed = []

    # watch out for connection problems
    try:
      xml_body = str(reply.readAll())

      # watch out for empty input
      if len(xml_body) == 0:
        return parsed

      root = fromstring(xml_body)

      # strip the namespaces from all of the parsed items
      for el in root.getiterator():
        ns_pos = el.tag.find('}')
        if ns_pos != -1:
          el.tag = el.tag[(ns_pos + 1):]

    except Exception as ex:
      print 'error while preparing reply for parsing', ex
      return parsed

    # decode the result
    try:
      items = []

      is_valid = root.find('Items/Request/IsValid')
      total_results = root.find('Items/TotalResults')

      # if we have a valid response with any results...
      if (not is_valid is None) and is_valid != 'False' and (not total_results is None) and total_results != '0':
        query = root.find('Items/Request/ItemSearchRequest/Keywords').text

        # remember them all
        for i, item in enumerate(root.findall('Items/Item')):
          final_url = None
          current_url = item.find('LargeImage/URL')

          if current_url == None:
            current_url = item.find('MediumImage/URL')

          if current_url == None:
            continue

          current = clementine.CoverSearchResult()
          current.description = QString(query)
          current.image_url = QString(current_url.text)

          parsed.append(current)

    except KeyError as ex:
      print 'incorrect response structure', ex

    return parsed

  def PrepareAmazonRESTUrl(self, text):
    """
    Prepares a RESTful URL according to Amazon's strict querying policies.
    Deals with the variable part of the URL only (the one after the '?').
    """

    # additional required API arguments
    copy_args = self.api_base_args.copy()
    copy_args['Keywords'] = str(text)
    copy_args['Timestamp'] = time.strftime('%Y-%m-%dT%H:%M:%SZ', time.gmtime())

    # turn the argument map into a list of encoded request parameter strings
    query_list = map(
      lambda (k, v): (k + "=" + urllib.quote(v)),
      copy_args.items()
    )

    # sort the list (by parameter name)
    query_list.sort()

    # turn the list into a partial URL string
    query_string = "&".join(query_list)

    # prepare a string on which we will base the AWS signature
    string_to_sign = """GET
{0}
/onca/xml
{1}""".format('ecs.amazonaws.com', query_string)

    # create HMAC for the string (using SHA-256 and our secret API key)
    hm = hmac.new(key = self.aws_secret_access_key,
                  msg = string_to_sign,
                  digestmod = hashlib.sha256)
    # final step... convert the HMAC to base64, then encode it
    signature = urllib.quote(base64.b64encode(hm.digest()))

    return query_string + '&Signature=' + signature


script = AmazonCoversScript()
