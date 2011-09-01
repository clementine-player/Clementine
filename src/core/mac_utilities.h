// Only include this from Objective-C++ files

#include <QKeySequence>

@class NSEvent;

namespace mac {

QKeySequence KeySequenceFromNSEvent(NSEvent* event);

}
