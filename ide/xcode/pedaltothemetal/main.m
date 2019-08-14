
#if OSX
#import <Cocoa/Cocoa.h>


#import "AppDelegate.h":
//static AppDelegate* appDelegate;
int main(int argc, const char * argv[])
{
    //@autoreleasepool
    {
        NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
        [NSApplication sharedApplication];
        AppDelegate* appDelegate = [[AppDelegate alloc] init];
        [NSApp setDelegate:appDelegate];
        [NSApp run];
        [pool release];
    }
    return 0;
}

#elif IOS

#import <UIKit/UIKit.h>
#import "AppDelegate.h"

int main(int argc, char * argv[])
{
    @autoreleasepool
    {
        return UIApplicationMain(argc, argv, nil, NSStringFromClass([AppDelegate class]));
    }
}

#endif
