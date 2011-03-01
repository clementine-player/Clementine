/*
  Copyright (c) 2004-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/



#ifndef PRIVATEXML_H__
#define PRIVATEXML_H__

#include "iqhandler.h"
#include "privatexmlhandler.h"

#include <string>
#include <list>
#include <map>

namespace gloox
{

  class ClientBase;
  class Tag;
  class Stanza;

  /**
   * @brief This class implements XEP-0049 (Private XML Storage).
   *
   * @author Jakob Schroeter <js@camaya.net>
   */
  class GLOOX_API PrivateXML : public IqHandler
  {
    public:
      /**
       * Constructor.
       * Creates a new PrivateXML client that registers as IqHandler
       * with @c ClientBase.
       * @param parent The ClientBase used for XMPP communication
       */
      PrivateXML( ClientBase* parent );

      /**
       * Virtual destructor.
       */
      virtual ~PrivateXML();

      /**
       * Use this function to request the private XML stored in the given namespace.
       * @param tag Child element of the query element used to identify the requested XML fragment.
       * @param xmlns The namespace which qualifies the tag.
       * @param pxh The handler to receive the result.
       * @return The ID of the sent query.
       */
      std::string requestXML( const std::string& tag, const std::string& xmlns, PrivateXMLHandler* pxh );

      /**
       * Use this function to store private XML stored in the given namespace.
       * @param tag The XML to store. This is the complete tag including the unique namespace.
       * It is deleted automatically after sending it.
       * @param pxh The handler to receive the result.
       * @return The ID of the sent query.
       */
      std::string storeXML( const Tag* tag, PrivateXMLHandler* pxh );

      // reimplemented from IqHandler.
      virtual bool handleIq( const IQ& iq ) { (void)iq; return false; }

      // reimplemented from IqHandler.
      virtual void handleIqID( const IQ& iq, int context );

    protected:
      ClientBase* m_parent;

    private:
#ifdef PRIVATEXML_TEST
    public:
#endif
      /**
       * @brief An implementation of the Private XML Storage protocol as StanzaExtension.
       *
       * @author Jakob Schroeter <js@camaya.net>
       * @since 1.0
       */
      class Query : public StanzaExtension
      {
        public:
          /**
           * Constructs a new Query object suitable for use with Private XML Storage.
           * @param tag The private XML's element name.
           * @param xmlns The private XML's namespace.
           */
          Query( const std::string& tag, const std::string& xmlns )
            : StanzaExtension( ExtPrivateXML )
          {
            m_privateXML = new Tag( tag, XMLNS, xmlns );
          }

          /**
           * Constructs a new Query object suitable for storing an XML fragment in
           * Private XML Storage.
           * @param tag The private XML element to store. The Query object will own the Tag.
           */
          Query( const Tag* tag = 0 );

          /**
           * Destructor.
           */
          ~Query() { delete m_privateXML; }

          /**
           * Returns the private XML fragment. The Tag is owned by the Query object.
           * @return The stored private XML fragment.
           */
          const Tag* privateXML() const { return m_privateXML; }

          // reimplemented from StanzaExtension
          virtual const std::string& filterString() const;

          // reimplemented from StanzaExtension
          virtual StanzaExtension* newInstance( const Tag* tag ) const
          {
            return new Query( tag );
          }

          // reimplemented from StanzaExtension
          virtual Tag* tag() const;

          // reimplemented from StanzaExtension
          virtual StanzaExtension* clone() const
          {
            Query* q = new Query();
            q->m_privateXML = m_privateXML ? m_privateXML->clone() : 0;
            return q;
          }

        private:
          const Tag* m_privateXML;

      };

      enum IdType
      {
        RequestXml,
        StoreXml
      };

      typedef std::map<std::string, PrivateXMLHandler*> TrackMap;

      TrackMap m_track;
  };

}

#endif // PRIVATEXML_H__
