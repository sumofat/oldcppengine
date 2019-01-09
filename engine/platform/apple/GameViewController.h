#if OSX
#import <Cocoa/Cocoa.h>
#elif IOS
#import <UIKit/UIKit.h>
#endif
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>

#if OSX
// Our macOS view controller.
@interface GameViewController : NSViewController
#elif IOS
@interface GameViewController : UIViewController

#endif
@end
