#include "concurrentrun.h"

namespace ConcurrentRun {

void Run(
    QThreadPool* threadpool,
    std::tr1::function<void ()> function) {
  (new ThreadFunctorVoid(function))->Start(threadpool);
}

}  // namespace ConcurrentRun
