
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
                                                                     styleMask:NSWindowStyleMaskBorderless | NSWindowStyleMaskFullScreen | NSWindowStyleMaskFullSizeContentView
                                                                       backing:NSBackingStoreBuffered
                                                                         defer:NO] autorelease];
    [window setTitleVisibility:NSWindowTitleHidden];
    [window setTitlebarAppearsTransparent:YES];
    [window setBackgroundColor:[NSColor blueColor]];
    [window makeKeyAndOrderFront:NSApp];
    [[window standardWindowButton:NSWindowCloseButton] setHidden:YES];
    [[window standardWindowButton:NSWindowMiniaturizeButton] setHidden:YES];
    [[window standardWindowButton:NSWindowZoomButton] setHidden:YES];
    //[window setBackgroundColor: NSColor.whiteColor];
    //[window setCollectionBehavior:NSWindowCollectionBehaviorFullScreenPrimary];
    [window setFrame:frame display:YES];
    int windowLevel = CGShieldingWindowLevel();
    [window setLevel:windowLevel];
    view_controller = [[GameViewController alloc] init];
    //[window toggleFullScreen:view_controller.view];
    //[self.view setFrameSize:screenFrame.size];
    //[window setFrame:frame display:YES animate:YES];
    //window.styleMask
    //view_controller = [[GameViewController alloc] init];
    [window setContentViewController:view_controller];
}

- (void)applicationWillTerminate:(NSNotification *)aNotification {
    // Insert code here to tear down your application
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender {
    return YES;
}

@end
