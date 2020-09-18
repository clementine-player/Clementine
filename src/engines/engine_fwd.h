#ifndef ENGINE_FWD_H
#define ENGINE_FWD_H

#include <QString>

/// Used by eg engineobserver.h, and thus we reduce header dependencies on
/// enginebase.h

namespace Engine {
struct SimpleMetaBundle;
class Base;

/**
 * You should return:
 * Playing when playing,
 * Paused when paused
 * Idle when you still have a URL loaded (ie you have not been told to stop())
 * Empty when you have been told to stop(),
 * Error when an error occurred and you stopped yourself
 *
 * It is vital to be Idle just after the track has ended!
 */
enum State { Empty, Idle, Playing, Paused, Error };

enum TrackChangeType {
  // One of:
  First = 0x01,
  Manual = 0x02,
  Auto = 0x04,
  Intro = 0x08,

  // Any of:
  SameAlbum = 0x10,
};
Q_DECLARE_FLAGS(TrackChangeFlags, TrackChangeType)
}  // namespace Engine

typedef Engine::Base EngineBase;

#endif
