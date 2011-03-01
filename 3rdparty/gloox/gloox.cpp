/*
  Copyright (c) 2005-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#include "gloox.h"

namespace gloox
{

  const std::string XMLNS_CLIENT            = "jabber:client";
  const std::string XMLNS_COMPONENT_ACCEPT  = "jabber:component:accept";
  const std::string XMLNS_COMPONENT_CONNECT = "jabber:component:connect";

  const std::string XMLNS_DISCO_INFO        = "http://jabber.org/protocol/disco#info";
  const std::string XMLNS_DISCO_ITEMS       = "http://jabber.org/protocol/disco#items";
  const std::string XMLNS_DISCO_PUBLISH     = "http://jabber.org/protocol/disco#publish";
  const std::string XMLNS_ADHOC_COMMANDS    = "http://jabber.org/protocol/commands";
  const std::string XMLNS_COMPRESSION       = "http://jabber.org/protocol/compress";
  const std::string XMLNS_OFFLINE           = "http://jabber.org/protocol/offline";

  const std::string XMLNS_CHAT_STATES       = "http://jabber.org/protocol/chatstates";
  const std::string XMLNS_AMP               = "http://jabber.org/protocol/amp";
  const std::string XMLNS_IBB               = "http://jabber.org/protocol/ibb";
  const std::string XMLNS_FEATURE_NEG       = "http://jabber.org/protocol/feature-neg";
  const std::string XMLNS_CHATNEG           = "http://jabber.org/protocol/chatneg";

  const std::string XMLNS_XHTML_IM          = "http://jabber.org/protocol/xhtml-im";
  const std::string XMLNS_DELAY             = "urn:xmpp:delay";
  const std::string XMLNS_ROSTER            = "jabber:iq:roster";
  const std::string XMLNS_VERSION           = "jabber:iq:version";
  const std::string XMLNS_REGISTER          = "jabber:iq:register";

  const std::string XMLNS_PRIVACY           = "jabber:iq:privacy";
  const std::string XMLNS_AUTH              = "jabber:iq:auth";
  const std::string XMLNS_PRIVATE_XML       = "jabber:iq:private";
  const std::string XMLNS_LAST              = "jabber:iq:last";
  const std::string XMLNS_SEARCH            = "jabber:iq:search";

  const std::string XMLNS_IQ_OOB            = "jabber:iq:oob";
  const std::string XMLNS_X_DATA            = "jabber:x:data";
  const std::string XMLNS_X_EVENT           = "jabber:x:event";
  const std::string XMLNS_X_OOB             = "jabber:x:oob";
  const std::string XMLNS_X_DELAY           = "jabber:x:delay";

  const std::string XMLNS_X_GPGSIGNED       = "jabber:x:signed";
  const std::string XMLNS_X_GPGENCRYPTED    = "jabber:x:encrypted";
  const std::string XMLNS_VCARD_TEMP        = "vcard-temp";
  const std::string XMLNS_X_VCARD_UPDATE    = "vcard-temp:x:update";
  const std::string XMLNS_BOOKMARKS         = "storage:bookmarks";

  const std::string XMLNS_ANNOTATIONS       = "storage:rosternotes";
  const std::string XMLNS_ROSTER_DELIMITER  = "roster:delimiter";
  const std::string XMLNS_XMPP_PING         = "urn:xmpp:ping";
  const std::string XMLNS_SI                = "http://jabber.org/protocol/si";
  const std::string XMLNS_SI_FT             = "http://jabber.org/protocol/si/profile/file-transfer";

  const std::string XMLNS_BYTESTREAMS       = "http://jabber.org/protocol/bytestreams";
  const std::string XMLNS_MUC               = "http://jabber.org/protocol/muc";
  const std::string XMLNS_MUC_USER          = "http://jabber.org/protocol/muc#user";
  const std::string XMLNS_MUC_ADMIN         = "http://jabber.org/protocol/muc#admin";
  const std::string XMLNS_MUC_UNIQUE        = "http://jabber.org/protocol/muc#unique";

  const std::string XMLNS_MUC_OWNER         = "http://jabber.org/protocol/muc#owner";
  const std::string XMLNS_MUC_ROOMINFO      = "http://jabber.org/protocol/muc#roominfo";
  const std::string XMLNS_MUC_ROOMS         = "http://jabber.org/protocol/muc#rooms";
  const std::string XMLNS_MUC_REQUEST       = "http://jabber.org/protocol/muc#request";

  const std::string XMLNS_PUBSUB            = "http://jabber.org/protocol/pubsub";
  const std::string XMLNS_PUBSUB_ERRORS     = "http://jabber.org/protocol/pubsub#errors";
  const std::string XMLNS_PUBSUB_EVENT      = "http://jabber.org/protocol/pubsub#event";
  const std::string XMLNS_PUBSUB_OWNER      = "http://jabber.org/protocol/pubsub#owner";

  const std::string XMLNS_CAPS              = "http://jabber.org/protocol/caps";
  const std::string XMLNS_FT_FASTMODE       = "http://affinix.com/jabber/stream";

  const std::string XMLNS_STREAM            = "http://etherx.jabber.org/streams";
  const std::string XMLNS_XMPP_STREAM       = "urn:ietf:params:xml:ns:xmpp-streams";
  const std::string XMLNS_XMPP_STANZAS      = "urn:ietf:params:xml:ns:xmpp-stanzas";
  const std::string XMLNS_STREAM_TLS        = "urn:ietf:params:xml:ns:xmpp-tls";
  const std::string XMLNS_STREAM_SASL       = "urn:ietf:params:xml:ns:xmpp-sasl";

  const std::string XMLNS_STREAM_BIND       = "urn:ietf:params:xml:ns:xmpp-bind";
  const std::string XMLNS_STREAM_SESSION    = "urn:ietf:params:xml:ns:xmpp-session";
  const std::string XMLNS_STREAM_IQAUTH     = "http://jabber.org/features/iq-auth";
  const std::string XMLNS_STREAM_IQREGISTER = "http://jabber.org/features/iq-register";
  const std::string XMLNS_STREAM_COMPRESS   = "http://jabber.org/features/compress";

  const std::string XMLNS_HTTPBIND          = "http://jabber.org/protocol/httpbind";
  const std::string XMLNS_XMPP_BOSH         = "urn:xmpp:xbosh";
  const std::string XMLNS_RECEIPTS          = "urn:xmpp:receipts";
  const std::string XMLNS_NICKNAME          = "http://jabber.org/protocol/nick";

  const std::string XMLNS_JINGLE            = "urn:xmpp:tmp:jingle";
  const std::string XMLNS_JINGLE_AUDIO_RTP  = "urn:xmpp:tmp:jingle:apps:audio-rtp";
  const std::string XMLNS_JINGLE_ICE_UDP    = "urn:xmpp:tmp:jingle:transports:ice-udp";
  const std::string XMLNS_JINGLE_RAW_UDP    = "urn:xmpp:tmp:jingle:transports:raw-udp";
  const std::string XMLNS_JINGLE_VIDEO_RTP  = "urn:xmpp:tmp:jingle:apps:video-rtp";

  const std::string XMLNS_SHIM              = "http://jabber.org/protocol/shim";
  const std::string XMLNS_ATTENTION         = "urn:xmpp:attention:0";

  const std::string XMPP_STREAM_VERSION_MAJOR = "1";
  const std::string XMPP_STREAM_VERSION_MINOR = "0";
  const std::string GLOOX_VERSION           = "1.0";
  const std::string GLOOX_CAPS_NODE         = "http://camaya.net/gloox";

  const std::string XMLNS = "xmlns";
  const std::string TYPE = "type";
  const std::string EmptyString = "";
}

const char* gloox_version()
{
  return gloox::GLOOX_VERSION.c_str();
}
