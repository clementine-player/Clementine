/*
  Copyright (c) 2005-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/



#ifndef ANNOTATIONSHANDLER_H__
#define ANNOTATIONSHANDLER_H__

#include "macros.h"

#include <string>
#include <list>

namespace gloox
{

  /**
   * This describes a single note item.
   */
  struct AnnotationsListItem
  {
    std::string jid;                /**< The JID of the roster item this note is about */
    std::string cdate;              /**< Creation date of this note. */
    std::string mdate;              /**< Date of last modification of this note. */
    std::string note;               /**< The note. */
  };

  /**
   * A list of note items.
   */
  typedef std::list<AnnotationsListItem> AnnotationsList;

  /**
   * @brief A virtual interface which can be reimplemented to receive notes with help of
   * the Annotations object.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.3
   */
  class GLOOX_API AnnotationsHandler
  {
    public:
      /**
       * Virtual destructor.
       */
      virtual ~AnnotationsHandler() {}

      /**
       * This function is called when notes arrive from the server.
       * @param aList A list of notes.
       */
      virtual void handleAnnotations( const AnnotationsList &aList ) = 0;
  };

}

#endif // ANNOTATIONSHANDLER_H__
