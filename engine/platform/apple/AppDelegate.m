
@class GameViewController;
#import "AppDelegate.h"
#import "GameViewController.h"
#if OSX
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
#endif

//#if OSX
//NSWindow *window;
GameViewController* view_controller;
//#endif
@interface AppDelegate ()

@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
#if OSX
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
    [window setFrame:frame display:YES];
    
    int windowLevel = CGShieldingWindowLevel();
    [window setLevel:windowLevel];
    view_controller = [[GameViewController alloc] init];
    [window setContentViewController:view_controller];
#elif IOS
    CGRect frame = [UIScreen mainScreen].bounds;
    self.window = [[UIWindow alloc] initWithFrame:frame];
    self.window.backgroundColor = [UIColor blueColor];
    GameViewController *vc = [[GameViewController alloc] init];//:@"ViewController" bundle:nil];
    self.window.rootViewController = vc;
    [self.window makeKeyAndVisible];
#endif
}

- (void)applicationWillTerminate:(NSNotification *)aNotification {
    // Insert code here to tear down your application
}

#ifdef OSX
- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender {
    return YES;
}
#endif

@end
