/* This file is part of Clementine.
   Copyright 2010, David Sansome <me@davidsansome.com>
   Copyright 2015 - 2016, Arun Narayanankutty <n.arun.lifescience@gmail.com>

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

#ifndef IDLEHANDLER_H
#define IDLEHANDLER_H

class IdleHandler {
 public:
  virtual ~IdleHandler() {}

  virtual void Inhibit(const char* reason) = 0;
  virtual void Uninhibit() = 0;
  virtual bool Isinhibited() = 0; 

  static IdleHandler* GetScreensaver();
  static IdleHandler* GetSuspend();

  enum Inhibitor
  {
    Screensaver = 0,
    Suspend = 1
  };
  static Inhibitor inbtr_;

  static const char* kGnomeScreensaverService;
  static const char* kFreedesktopScreensaverService;
  static const char* kGnomePowermanagerService;
  static const char* kFreedesktopPowermanagerService;

 private:
  static IdleHandler* screensaver_;
  static IdleHandler* suspend_;
};

#endif  // IDLEHANDLER_H
