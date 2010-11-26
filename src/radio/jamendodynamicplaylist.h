#ifndef JAMENDODYNAMICPLAYLIST_H
#define JAMENDODYNAMICPLAYLIST_H

#include "smartplaylists/generator.h"

class NetworkAccessManager;

class JamendoDynamicPlaylist : public smart_playlists::Generator {
  Q_OBJECT
 public:
  JamendoDynamicPlaylist();

  virtual QString type() const { return "Jamendo"; }
  virtual void Load(const QByteArray& data);
  virtual QByteArray Save() const;

  virtual PlaylistItemList Generate();

  virtual bool is_dynamic() const { return true; }
  virtual PlaylistItemList GenerateMore(int count);

 private:
  void Fetch();

  NetworkAccessManager* network_;
  int current_page_;
  PlaylistItemList current_items_;
  int current_index_;

  static const int kPageSize = 20;

  static const char* kTopTracksMonthUrl;
};

#endif
