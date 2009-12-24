
#ifndef ENGINE_FWD_H
#define ENGINE_FWD_H

/// Used by eg engineobserver.h, and thus we reduce header dependencies on enginebase.h

namespace Engine
{
    class SimpleMetaBundle;
    class Base;

    /**
     * You should return:
     * Playing when playing,
     * Paused when paused
     * Idle when you still have a URL loaded (ie you have not been told to stop())
     * Empty when you have been told to stop(), or an error occurred and you stopped yourself
     *
     * It is vital to be Idle just after the track has ended!
     */
    enum State { Empty, Idle, Playing, Paused };
}

typedef Engine::Base EngineBase;

#endif
