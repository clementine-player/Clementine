import clementine

from PythonQt.QtCore    import QUrl
from PythonQt.QtNetwork import QNetworkRequest

import base64
import hashlib
import hmac
import logging
import time
import urllib
import xml.etree.ElementTree

LOGGER = logging.getLogger("amazon_covers")


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

  API_URL = 'http://ecs.amazonaws.com/onca/xml?{0}'
  AWS_ACCESS_KEY = 'AKIAJ4QO3GQTSM3A43BQ'
  AWS_SECRET_ACCESS_KEY = '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'

  def __init__(self, parent):
    clementine.CoverProvider.__init__(self, "Amazon", parent)

    # basic API's arguments (search in all categories)
    self.api_base_args = {
      'Service'       : 'AWSECommerceService',
      'Version'       : '2009-11-01',
      'Operation'     : 'ItemSearch',
      'SearchIndex'   : 'All',
      'ResponseGroup' : 'Images',
      'AWSAccessKeyId': self.AWS_ACCESS_KEY
    }
    self.network = clementine.NetworkAccessManager(self)

  def SendRequest(self, query):
    url = QUrl.fromEncoded(self.API_URL.format(self.PrepareAmazonRESTUrl(query)))
    LOGGER.info("Sending request to '%s'", url)

    return self.network.get(QNetworkRequest(url))

  def ParseReply(self, reply):
    parsed = []

    # watch out for connection problems
    try:
      xml_body = str(reply.readAll())

      # watch out for empty input
      if len(xml_body) == 0:
        return parsed

      root = xml.etree.ElementTree.fromstring(xml_body)

      # strip the namespaces from all of the parsed items
      for el in root.getiterator():
        ns_pos = el.tag.find('}')
        if ns_pos != -1:
          el.tag = el.tag[(ns_pos + 1):]

    except Exception as ex:
      LOGGER.exception(ex)
      return parsed

    # decode the result
    try:
      items = []

      is_valid = root.find('Items/Request/IsValid')
      total_results = root.find('Items/TotalResults')

      # if we have a valid response with any results...
      if is_valid is not None and is_valid != 'False' and \
         total_results is not None and total_results != '0':
        query = root.find('Items/Request/ItemSearchRequest/Keywords').text

        # remember them all
        for item in root.findall('Items/Item'):
          final_url = None
          current_url = item.find('LargeImage/URL')

          if current_url is None:
            current_url = item.find('MediumImage/URL')

          if current_url is None:
            continue

          current = clementine.CoverSearchResult()
          current.description = str(query)
          current.image_url = str(current_url.text)

          parsed.append(current)

    except KeyError as ex:
      LOGGER.exception(ex)

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
    query_list = ["%s=%s" % (k, urllib.quote(v))
                  for k, v in copy_args.items()]

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
    hm = hmac.new(key = self.AWS_SECRET_ACCESS_KEY,
                  msg = string_to_sign,
                  digestmod = hashlib.sha256)
    # final step... convert the HMAC to base64, then encode it
    signature = urllib.quote(base64.b64encode(hm.digest()))

    return query_string + '&Signature=' + signature


amazon_script = AmazonCoversScript()
