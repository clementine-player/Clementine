/* This file is part of Clementine.
   Copyright 2011, David Sansome <me@davidsansome.com>

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

// Note: this file is licensed under the Apache License instead of GPL because
// it is used by the Spotify blob which links against libspotify and is not GPL
// compatible.

#include <QCoreApplication>
#include <QStringList>

#include <gst/gst.h>

#include "spotifyclient.h"
#include "core/logging.h"

int main(int argc, char** argv) {
  QCoreApplication a(argc, argv);
  QCoreApplication::setApplicationName("Clementine");
  QCoreApplication::setOrganizationName("Clementine");
  QCoreApplication::setOrganizationDomain("clementine-player.org");

  logging::Init();

  gst_init(nullptr, nullptr);

  const QStringList arguments(a.arguments());

  if (arguments.length() != 2) {
    qFatal("Usage: %s port", argv[0]);
  }

  SpotifyClient client;
  client.Init(arguments[1].toInt());

  return a.exec();
}
