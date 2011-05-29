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

#include "objectdecorators.h"
#include "playlist/playlistitem.h"

void ObjectDecorators::deleteLater(QObject* self) {
  self->deleteLater();
}
bool ObjectDecorators::inherits(QObject* self, const char* class_name) {
  return self->inherits(class_name);
}

QString ObjectDecorators::type(PlaylistItemPtr* self) const {
  return (*self)->type();
}
PlaylistItem::Options ObjectDecorators::options(PlaylistItemPtr* self) const {
  return (*self)->options();
}
bool ObjectDecorators::InitFromQuery(PlaylistItemPtr* self, const SqlRow& query) {
  return (*self)->InitFromQuery(query);
}
void ObjectDecorators::BindToQuery(PlaylistItemPtr* self, QSqlQuery* query) const {
  (*self)->BindToQuery(query);
}
void ObjectDecorators::Reload(PlaylistItemPtr* self) {
  (*self)->Reload();
}
void ObjectDecorators::BackgroundReload(PlaylistItemPtr* self) {
  (*self)->BackgroundReload();
}
Song ObjectDecorators::Metadata(PlaylistItemPtr* self) const {
  return (*self)->Metadata();
}
QUrl ObjectDecorators::Url(PlaylistItemPtr* self) const {
  return (*self)->Url();
}
void ObjectDecorators::SetTemporaryMetadata(PlaylistItemPtr* self, const Song& metadata) {
  (*self)->SetTemporaryMetadata(metadata);
}
void ObjectDecorators::ClearTemporaryMetadata(PlaylistItemPtr* self) {
  (*self)->ClearTemporaryMetadata();
}
bool ObjectDecorators::HasTemporaryMetadata(PlaylistItemPtr* self) const {
  return (*self)->HasTemporaryMetadata();
}
void ObjectDecorators::SetBackgroundColor(PlaylistItemPtr* self, short priority, const QColor& color) {
  (*self)->SetBackgroundColor(priority, color);
}
bool ObjectDecorators::HasBackgroundColor(PlaylistItemPtr* self, short priority) const {
  return (*self)->HasBackgroundColor(priority);
}
void ObjectDecorators::RemoveBackgroundColor(PlaylistItemPtr* self, short priority) {
  (*self)->RemoveBackgroundColor(priority);
}
QColor ObjectDecorators::GetCurrentBackgroundColor(PlaylistItemPtr* self) const {
  return (*self)->GetCurrentBackgroundColor();
}
bool ObjectDecorators::HasCurrentBackgroundColor(PlaylistItemPtr* self) const {
  return (*self)->HasCurrentBackgroundColor();
}
void ObjectDecorators::SetForegroundColor(PlaylistItemPtr* self, short priority, const QColor& color) {
  (*self)->SetForegroundColor(priority, color);
}
bool ObjectDecorators::HasForegroundColor(PlaylistItemPtr* self, short priority) const {
  return (*self)->HasForegroundColor(priority);
}
void ObjectDecorators::RemoveForegroundColor(PlaylistItemPtr* self, short priority) {
  (*self)->RemoveForegroundColor(priority);
}
QColor ObjectDecorators::GetCurrentForegroundColor(PlaylistItemPtr* self) const {
  return (*self)->GetCurrentForegroundColor();
}
bool ObjectDecorators::HasCurrentForegroundColor(PlaylistItemPtr* self) const {
  return (*self)->HasCurrentForegroundColor();
}
bool ObjectDecorators::IsLocalLibraryItem(PlaylistItemPtr* self) const {
  return (*self)->IsLocalLibraryItem();
}

