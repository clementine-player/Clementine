#include "spotify_utilities.h"

#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSFileManager.h>
#import <Foundation/NSPathUtilities.h>

namespace utilities {

QString GetUserDataDirectory() {
  NSAutoreleasePool* pool = [NSAutoreleasePool alloc];
  [pool init];

  NSArray* paths = NSSearchPathForDirectoriesInDomains(NSCachesDirectory,
                                                       NSUserDomainMask, YES);
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

QString GetSettingsDirectory() {
  NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
  NSArray* paths = NSSearchPathForDirectoriesInDomains(
      NSApplicationSupportDirectory, NSUserDomainMask, YES);
  NSString* ret;
  if ([paths count] > 0) {
    ret = [paths objectAtIndex:0];
  } else {
    ret = @"~/Library/Application Support";
  }
  ret = [ret stringByAppendingString:@"/Clementine/spotify-settings"];

  NSFileManager* file_manager = [NSFileManager defaultManager];
  [file_manager createDirectoryAtPath:ret
          withIntermediateDirectories:YES
                           attributes:nil
                                error:nil];

  QString path = QString::fromUtf8([ret UTF8String]);
  [pool drain];
  return path;
}
}
