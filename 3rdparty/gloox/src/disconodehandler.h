/*
  Copyright (c) 2004-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/



#ifndef DISCONODEHANDLER_H__
#define DISCONODEHANDLER_H__

#include "gloox.h"
#include "disco.h"

#include <list>
#include <map>
#include <string>

namespace gloox
{

  /**
   * @brief Derived classes can be registered as NodeHandlers for certain nodes with the Disco object.
   *
   * Incoming disco#info and disco#items queries are delegated to their respective handlers.
   *
   * @author Jakob Schroeter <js@camaya.net>
   */
  class GLOOX_API DiscoNodeHandler
  {
    public:
      /**
       * Virtual Destructor.
       */
      virtual ~DiscoNodeHandler() {}

      /**
       * In addition to @c handleDiscoNodeIdentities, this function is used to gather
       * more information on a specific node. It is called when a disco#info query
       * arrives with a node attribute that matches the one registered for this handler.
       * @param from The sender of the request.
       * @param node The node this handler is supposed to handle.
       * @return A list of features supported by this node.
       */
      virtual StringList handleDiscoNodeFeatures( const JID& from, const std::string& node ) = 0;

      /**
       * In addition to @c handleDiscoNodeFeatures, this function is used to gather
       * more information on a specific node. It is called when a disco#info query
       * arrives with a node attribute that matches the one registered for this handler.
       * @param from The sender of the request.
       * @param node The node this handler is supposed to handle.
       * @return A list of identities for the given node. The caller will own the identities.
       */
      virtual Disco::IdentityList handleDiscoNodeIdentities( const JID& from,
                                                             const std::string& node ) = 0;

      /**
       * This function is used to gather more information on a specific node.
       * It is called when a disco#items query arrives with a node attribute that
       * matches the one registered for this handler. If node is empty, items for the
       * root node (no node) shall be returned.
       * @param from The sender of the request.
       * @param to The receiving JID (useful for transports).
       * @param node The node this handler is supposed to handle.
       * @return A list of items supported by this node.
       */
      virtual Disco::ItemList handleDiscoNodeItems( const JID& from, const JID& to,
                                                    const std::string& node = EmptyString ) = 0;

  };

}

#endif // DISCONODEHANDLER_H__
