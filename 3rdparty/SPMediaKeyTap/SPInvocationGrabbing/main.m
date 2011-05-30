#import <Cocoa/Cocoa.h>
#import "NSObject+SPInvocationGrabbing.h"

@interface Foo : NSObject {
    int a;
}
-(void)startIt;
-(void)theBackgroundStuff;
-(void)theForegroundStuff;
@end

@implementation Foo
-(void)startIt;
{
    NSLog(@"Starting out on the main thread...");
    a = 3;
    [[self inBackground] theBackgroundStuff];
}
-(void)theBackgroundStuff;
{
    NSLog(@"Woah, this is a background thread!");
    a += 6;
    [[self onMainAsync:YES] theForegroundStuff];
}
-(void)theForegroundStuff;
{
    NSLog(@"Hey presto: %d", a);
}
@end

int main() {
    NSAutoreleasePool *pool = [NSAutoreleasePool new];
    Foo *foo = [Foo new];
    [foo startIt];
    [[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:1]];
    [pool release];
    return 0;
}