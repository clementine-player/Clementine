/***************************************************************************
 copyright            : (C) 2013 - 2018 by Stephen F. Booth
 email                : me@sbooth.org
 ***************************************************************************/

/***************************************************************************
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License version   *
 *   2.1 as published by the Free Software Foundation.                     *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful, but   *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA         *
 *   02110-1301  USA                                                       *
 *                                                                         *
 *   Alternatively, this file is available under the Mozilla Public        *
 *   License Version 1.1.  You may obtain a copy of the License at         *
 *   http://www.mozilla.org/MPL/                                           *
 ***************************************************************************/

#include <tbytevector.h>
#include <tdebug.h>
#include <id3v2tag.h>
#include <tstringlist.h>
#include <tpropertymap.h>
#include <tagutils.h>

#include "dsffile.h"

using namespace TagLib;

// The DSF specification is located at http://dsd-guide.com/sites/default/files/white-papers/DSFFileFormatSpec_E.pdf

class DSF::File::FilePrivate
{
public:
  FilePrivate() :
  properties(0),
  tag(0)
  {
  }

  ~FilePrivate()
  {
    delete properties;
    delete tag;
  }

  long long fileSize;
  long long metadataOffset;
  Properties *properties;
  ID3v2::Tag *tag;
};

////////////////////////////////////////////////////////////////////////////////
// static members
////////////////////////////////////////////////////////////////////////////////

bool DSF::File::isSupported(IOStream *stream)
{
    // A DSF file has to start with "DSD "
    const ByteVector id = Utils::readHeader(stream, 4, false);
    return id.startsWith("DSD ");
}

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

DSF::File::File(FileName file, bool readProperties,
                Properties::ReadStyle propertiesStyle) :
  TagLib::File(file),
  d(new FilePrivate())
{
  if(isOpen())
    read(readProperties, propertiesStyle);
}

DSF::File::File(IOStream *stream, bool readProperties,
                Properties::ReadStyle propertiesStyle) :
  TagLib::File(stream),
  d(new FilePrivate())
{
  if(isOpen())
    read(readProperties, propertiesStyle);
}

DSF::File::~File()
{
  delete d;
}

ID3v2::Tag *DSF::File::tag() const
{
  return d->tag;
}

PropertyMap DSF::File::properties() const
{
  return d->tag->properties();
}

PropertyMap DSF::File::setProperties(const PropertyMap &properties)
{
  return d->tag->setProperties(properties);
}

DSF::Properties *DSF::File::audioProperties() const
{
  return d->properties;
}

bool DSF::File::save()
{
  if(readOnly()) {
    debug("DSF::File::save() -- File is read only.");
    return false;
  }

  if(!isValid()) {
    debug("DSF::File::save() -- Trying to save invalid file.");
    return false;
  }

  // Three things must be updated: the file size, the tag data, and the metadata offset

  if(d->tag->isEmpty()) {
    long long newFileSize = d->metadataOffset ? d->metadataOffset : d->fileSize;

    // Update the file size
    if(d->fileSize != newFileSize) {
      insert(ByteVector::fromLongLong(newFileSize, false), 12, 8);
      d->fileSize = newFileSize;
    }

    // Update the metadata offset to 0 since there is no longer a tag
    if(d->metadataOffset) {
      insert(ByteVector::fromLongLong(0ULL, false), 20, 8);
      d->metadataOffset = 0;
    }

    // Delete the old tag
    truncate(newFileSize);
  }
  else {
    ByteVector tagData = d->tag->render();

    long long newMetadataOffset = d->metadataOffset ? d->metadataOffset : d->fileSize;
    long long newFileSize = newMetadataOffset + tagData.size();
    long long oldTagSize = d->fileSize - newMetadataOffset;

    // Update the file size
    if(d->fileSize != newFileSize) {
      insert(ByteVector::fromLongLong(newFileSize, false), 12, 8);
      d->fileSize = newFileSize;
    }

    // Update the metadata offset
    if(d->metadataOffset != newMetadataOffset) {
      insert(ByteVector::fromLongLong(newMetadataOffset, false), 20, 8);
      d->metadataOffset = newMetadataOffset;
    }

    // Delete the old tag and write the new one
    insert(tagData, newMetadataOffset, static_cast<size_t>(oldTagSize));
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////
// private members
////////////////////////////////////////////////////////////////////////////////


void DSF::File::read(bool readProperties, Properties::ReadStyle propertiesStyle)
{
  // A DSF file consists of four chunks: DSD chunk, format chunk, data chunk, and metadata chunk
  // The file format is not chunked in the sense of a RIFF File, though

  // DSD chunk
  ByteVector chunkName = readBlock(4);
  if(chunkName != "DSD ") {
    debug("DSF::File::read() -- Not a DSF file.");
    setValid(false);
    return;
  }

  long long chunkSize = readBlock(8).toLongLong(false);

  // Integrity check
  if(28 != chunkSize) {
    debug("DSF::File::read() -- File is corrupted, wrong chunk size");
    setValid(false);
    return;
  }

  d->fileSize = readBlock(8).toLongLong(false);

  // File is malformed or corrupted
  if(d->fileSize != length()) {
    debug("DSF::File::read() -- File is corrupted wrong length");
    setValid(false);
    return;
  }

  d->metadataOffset = readBlock(8).toLongLong(false);

  // File is malformed or corrupted
  if(d->metadataOffset > d->fileSize) {
    debug("DSF::File::read() -- Invalid metadata offset.");
    setValid(false);
    return;
  }

  // Format chunk
  chunkName = readBlock(4);
  if(chunkName != "fmt ") {
    debug("DSF::File::read() -- Missing 'fmt ' chunk.");
    setValid(false);
    return;
  }

  chunkSize = readBlock(8).toLongLong(false);

  d->properties = new Properties(readBlock(chunkSize), propertiesStyle);

  // Skip the data chunk

  // A metadata offset of 0 indicates the absence of an ID3v2 tag
  if(0 == d->metadataOffset)
    d->tag = new ID3v2::Tag();
  else
    d->tag = new ID3v2::Tag(this, d->metadataOffset);
}

