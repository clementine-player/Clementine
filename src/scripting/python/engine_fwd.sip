namespace Engine {

%TypeHeaderCode
#include "engines/engine_fwd.h"
%End

  enum State { Empty, Idle, Playing, Paused };
  enum TrackChangeType { First, Manual, Auto };
};
