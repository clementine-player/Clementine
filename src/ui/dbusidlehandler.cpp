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

#include "dbusidlehandler.h"

#include <QCoreApplication>
#include <QDBusInterface>
#include <QDBusReply>

#include <stdint.h>

#include "core/logging.h"

DBusIdleHandler::DBusIdleHandler(const QString& service, const QString& path,
                                 const QString& interface)
    : service_(service), path_(path), interface_(interface) {}

void DBusIdleHandler::Inhibit(const char* reason) {
  QDBusInterface iface(service_, path_,
                       interface_, QDBusConnection::sessionBus());
  QDBusReply<quint32> reply;
  if (service_ == IdleHandler::kGnomePowermanagerService) {
    reply =
        iface.call("Inhibit", QCoreApplication::applicationName(), 
                     quint32(0), QObject::tr(reason),
                     quint32(Inhibit_Suspend));
  } else {
    reply =
        iface.call("Inhibit", QCoreApplication::applicationName(),
                     QObject::tr(reason));
  }
  
  if (reply.isValid()) {
    cookie_ = reply.value();
  } else {
    qLog(Warning) << "Failed to inhibit screensaver/suspend dbus service: " 
                  <<  service_ << " path: " << path_ << " interface: " 
                  << interface_;
  }
}

void DBusIdleHandler::Uninhibit() {
  QDBusInterface iface(service_, path_,
                       interface_, QDBusConnection::sessionBus());
  if (service_ == IdleHandler::kGnomePowermanagerService) {
    iface.call("Uninhibit", cookie_);
  } else {
    iface.call("UnInhibit", cookie_);
  }   
}

bool DBusIdleHandler::Isinhibited() {
  QDBusInterface iface(service_, path_,
                       interface_, QDBusConnection::sessionBus());
  QDBusReply<bool> reply;
  if (service_ == IdleHandler::kGnomePowermanagerService) {
    reply = iface.call("IsInhibited", quint32(Inhibit_Suspend));
  } else {
    reply = iface.call("HasInhibit");
  }

  return reply;
}
