/***************************************************************************
    copyright           : (C) 2011 by Mathias Panzenböck
    email               : grosser.meister.morti@gmx.net
 ***************************************************************************/

/***************************************************************************
 *   This library is free software; you can redistribute it and/or modify  *
 *   it  under the terms of the GNU Lesser General Public License version  *
 *   2.1 as published by the Free Software Foundation.                     *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful, but   *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,            *
 *   MA  02110-1301  USA                                                   *
 ***************************************************************************/

#ifndef TAGLIB_ITFILE_H
#define TAGLIB_ITFILE_H

#include "tfile.h"
#include "audioproperties.h"
#include "taglib_export.h"
#include "modfilebase.h"
#include "modtag.h"
#include "itproperties.h"

namespace TagLib {

  namespace IT {

    class TAGLIB_EXPORT File : public Mod::FileBase {
      public:
        /*!
         * Contructs a Impulse Tracker file from \a file. If \a readProperties
         * is true the file's audio properties will also be read using
         * \a propertiesStyle. If false, \a propertiesStyle is ignored.
         */
        File(FileName file, bool readProperties = true,
             AudioProperties::ReadStyle propertiesStyle =
             AudioProperties::Average);

        /*!
         * Contructs a Impulse Tracker file from \a stream. If \a readProperties
         * is true the file's audio properties will also be read using
         * \a propertiesStyle. If false, \a propertiesStyle is ignored.
         *
         * \note TagLib will *not* take ownership of the stream, the caller is
         * responsible for deleting it after the File object.
         */
        File(IOStream *stram, bool readProperties = true,
             AudioProperties::ReadStyle propertiesStyle =
             AudioProperties::Average);

        /*!
         * Destroys this instance of the File.
         */
        virtual ~File();

        Mod::Tag *tag() const;

        /*!
         * Forwards to Mod::Tag::properties().
         * BIC: will be removed once File::toDict() is made virtual
         */
        PropertyMap properties() const;

        /*!
         * Forwards to Mod::Tag::setProperties().
         * BIC: will be removed once File::setProperties() is made virtual
         */
        PropertyMap setProperties(const PropertyMap &);

        /*!
         * Returns the IT::Properties for this file. If no audio properties
         * were read then this will return a null pointer.
         */
        IT::Properties *audioProperties() const;

        /*!
         * Save the file.
         * This is the same as calling save(AllTags);
         *
         * \note Saving Impulse Tracker tags is not supported.
         */
        bool save();


      private:
        File(const File &);
        File &operator=(const File &);

        void read(bool readProperties);

        class FilePrivate;
        FilePrivate *d;
    };
  }
}

#endif
