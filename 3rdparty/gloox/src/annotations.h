/*
  Copyright (c) 2005-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/



#ifndef ANNOTATIONS_H__
#define ANNOTATIONS_H__

#include "macros.h"

#include "annotationshandler.h"
#include "privatexml.h"
#include "privatexmlhandler.h"

#include <string>
#include <list>

namespace gloox
{

  class Tag;

  /**
   * @brief This is an implementation of XEP-0145 (Annotations).
   *
   * You can use this class to store arbitrary notes about a roster item on the server
   * (and to retrieve them later on).
   * To retrieve all stored annotations for the current user's roster you have to create
   * a class which inherits from AnnotationsHandler. This handler receives retrieved notes.
   *
   * @code
   * class MyClass : public AnnotationsHandler
   * {
   *   public:
   *     // ...
   *     void myFuncRetrieve();
   *     void myFuncStore();
   *     void handleAnnotations( const AnnotationsList &aList );
   *
   *   private:
   *     Annotations* m_notes;
   *     AnnotationsList m_list;
   * };
   *
   * void MyClass::myFuncRetrieve()
   * {
   *   [...]
   *   m_notes = new Annotations( m_client );
   *   m_notes->requestAnnotations();
   * }
   *
   * void MyClass::handleAnnotations( const AnnotationsList &aList )
   * {
   *   m_list = aList;
   * }
   * @endcode
   *
   * To store an additional note you have to fetch the currently stored notes first,
   * add your new note to the list of notes, and transfer them all together back to the
   * server. This protocol does not support storage of 'deltas', that is, when saving
   * notes all previously saved notes are overwritten.
   *
   * @code
   * void MyClass::myFuncStore()
   * {
   *   annotationsListItem item;
   *   item.jid = "me@example.com";
   *   item.cdate = "2006-02-04T15:23:21Z";
   *   item.note = "some guy at example.com";
   *   m_list.push_back( item );
   *
   *   item.jid = "abc@def.com";
   *   item.cdate = "2006-01-24T15:23:21Z";
   *   item.mdate = "2006-02-04T05:11:46Z";
   *   item.note = "some other guy";
   *   m_list.push_back( item );
   *
   *   m_notes->storeAnnotations( m_list );
   * }
   * @endcode
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.3
   */
  class GLOOX_API Annotations : public PrivateXML, public PrivateXMLHandler
  {
    public:
      /**
       * Constructs a new Annotations object.
       * @param parent The ClientBase to use for communication.
       */
      Annotations( ClientBase* parent );

      /**
       * Virtual destructor.
       */
      virtual ~Annotations();

      /**
       * Use this function to store notes (annotations to contacts in a roster) on the server.
       * Make sure you store the whole set of annotations, not a 'delta'.
       * @param aList A list of notes to store.
       */
      void storeAnnotations( const AnnotationsList& aList );

      /**
       * Use this function to initiate retrieval of annotations. Use registerAnnotationsHandler()
       * to register an object which will receive the lists of notes.
       */
      void requestAnnotations();

      /**
       * Use this function to register a AnnotationsHandler.
       * @param ah The AnnotationsHandler which shall receive retrieved notes.
       */
      void registerAnnotationsHandler( AnnotationsHandler* ah )
        { m_annotationsHandler = ah; }

      /**
       * Use this function to un-register the AnnotationsHandler.
       */
      void removeAnnotationsHandler()
        { m_annotationsHandler = 0; }

      // reimplemented from PrivateXMLHandler
      virtual void handlePrivateXML( const Tag* xml );

      // reimplemented from PrivateXMLHandler
      virtual void handlePrivateXMLResult( const std::string& uid, PrivateXMLResult pxResult );

    private:
      AnnotationsHandler* m_annotationsHandler;

  };

}

#endif // ANNOTATIONS_H__
