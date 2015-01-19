/* This file is part of Clementine.
   Copyright 2012, David Sansome <me@davidsansome.com>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>
   Copyright 2014, John Maguire <john.maguire@gmail.com>

   Clementine is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Clementine is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Clementine.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef COVERS_ALBUMCOVERLOADEROPTIONS_H_
#define COVERS_ALBUMCOVERLOADEROPTIONS_H_

#include <QImage>

struct AlbumCoverLoaderOptions {
  AlbumCoverLoaderOptions()
      : desired_height_(120),
        scale_output_image_(true),
        pad_output_image_(true) {}

  int desired_height_;
  bool scale_output_image_;
  bool pad_output_image_;
  QImage default_output_image_;
};

#endif  // COVERS_ALBUMCOVERLOADEROPTIONS_H_
