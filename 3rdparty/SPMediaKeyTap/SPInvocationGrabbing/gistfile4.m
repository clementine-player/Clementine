@interface MyClass : NSObject
-(BOOL)areTheNewViewersGoneYet:(Duck*)duck;
@end
...
MyClass *myInstance = [[MyClass alloc] init];
id invocationGrabber = [[[SPInvocationGrabber alloc] initWithTarget:myInstance] autorelease];


[invocationGrabber areTheNewViewersGoneYet:[Duck yellowDuck]]; // line 9


NSInvocation *invocationForAreTheNewViewersGoneYet = [invocationGrabber invocation];
