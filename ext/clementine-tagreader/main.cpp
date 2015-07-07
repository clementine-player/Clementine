/* This file is part of Clementine.
   Copyright 2011, David Sansome <me@davidsansome.com>

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

#include "tagreaderworker.h"
#include "core/logging.h"

#include <QCoreApplication>
#include <QLocalSocket>
#include <QSslSocket>
#include <QStringList>

#include <iostream>
#include <sys/time.h>

int main(int argc, char** argv) {
  QCoreApplication a(argc, argv);
  QStringList args(a.arguments());

  if (args.count() != 2) {
    std::cerr << "This program is used internally by Clementine to parse tags "
                 "in music files\n"
                 "without exposing the whole application to crashes caused by "
                 "malformed\n"
                 "files.  It is not meant to be run on its own.\n";
    return 1;
  }

  // Seed random number generator
  timeval time;
  gettimeofday(&time, nullptr);
  qsrand((time.tv_sec * 1000) + (time.tv_usec / 1000));

  logging::Init();
  qLog(Info) << "TagReader worker connecting to" << args[1];

  // Connect to the parent process.
  QLocalSocket socket;
  socket.connectToServer(args[1]);
  if (!socket.waitForConnected(2000)) {
    std::cerr << "Failed to connect to the parent process.\n";
    return 1;
  }

  QSslSocket::addDefaultCaCertificates(
      QSslCertificate::fromPath(":/certs/godaddy-root.pem", QSsl::Pem));
  QSslSocket::addDefaultCaCertificates(
      QSslCertificate::fromPath(":/certs/Equifax_Secure_Certificate_Authority.pem", QSsl::Pem));

  TagReaderWorker worker(&socket);

  return a.exec();
}
