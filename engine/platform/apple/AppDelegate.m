
@class GameViewController;
#import "AppDelegate.h"
#import "GameViewController.h"

@interface BorderlessWindow : NSWindow
{
}

@end

@implementation BorderlessWindow

- (BOOL)canBecomeKeyWindow
{
    return YES;
}

- (BOOL)canBecomeMainWindow
{
    return YES;
}

@end
#if OSX
NSWindow *window;
GameViewController* view_controller;

#endif
@interface AppDelegate ()

@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
   // NSRect frame = NSMakeRect(0, 0, 200, 200);
    NSRect frame = [NSScreen mainScreen].frame;
    BorderlessWindow* window  = [[[BorderlessWindow alloc] initWithContentRect:frame
                                                     styleMask:NSWindowStyleMaskBorderless
                                                       backing:NSBackingStoreBuffered
                                                         defer:NO] autorelease];
    
    
    view_controller = [[GameViewController alloc] init];
    [window setBackgroundColor:[NSColor blueColor]];
    [window makeKeyAndOrderFront:NSApp];
    [window setContentViewController:view_controller];
    [window setFrame:frame display:YES animate:YES];
}

- (void)applicationWillTerminate:(NSNotification *)aNotification {
    // Insert code here to tear down your application
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender {
    return YES;
}

@end
