#include "spotify_utilities.h"

#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSPathUtilities.h>

namespace utilities {

QString GetUserCacheDirectory() {
  NSAutoreleasePool* pool = [NSAutoreleasePool alloc];
  [pool init];

  NSArray* paths = NSSearchPathForDirectoriesInDomains(
      NSCachesDirectory,
      NSUserDomainMask,
      YES);
  QString ret;
  if ([paths count] > 0) {
    NSString* user_path = [paths objectAtIndex:0];
    ret = QString::fromUtf8([user_path UTF8String]);
  } else {
    ret = "~/Library/Caches";
  }
  [pool drain];
  return ret;
}

}
