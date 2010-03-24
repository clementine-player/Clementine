/* This file is part of Clementine.

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

#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include <iostream>

class QNetworkRequest;
class QString;
class QUrl;

std::ostream& operator <<(std::ostream& stream, const QString& str);
std::ostream& operator <<(std::ostream& stream, const QUrl& url);
std::ostream& operator <<(std::ostream& stream, const QNetworkRequest& req);

#define EXPOSE_SIGNAL0(n) \
    void Emit##n() { emit n(); }
#define EXPOSE_SIGNAL1(n, t1) \
    void Emit##n(const t1& a1) { emit n(a1); }

#endif  // TEST_UTILS_H
