/* This file is part of Clementine.
   Copyright 2010, David Sansome <me@davidsansome.com>

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

#ifndef SCREENSAVER_H
#define SCREENSAVER_H

class Screensaver {
 public:
  virtual ~Screensaver() {}

  static const char* kGnomeService;
  static const char* kGnomePath;
  static const char* kGnomeInterface;

  static const char* kKdeService;
  static const char* kKdePath;
  static const char* kKdeInterface;

  virtual void Inhibit() = 0;
  virtual void Uninhibit() = 0;

  static Screensaver* GetScreensaver();

 private:
  static Screensaver* screensaver_;
};

#endif
