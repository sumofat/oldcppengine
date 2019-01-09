
#import "GameViewController.h"

#include "../../../../RendererInclude.h"

#import "../../../../renderer/MTKViewDelegateView.h"

//NOTE(Ray):
//#import "SpriteExample.h"

@implementation GameViewController
{
}

- (instancetype)init
{
    self = [super initWithNibName:nil bundle:nil];
    if (self != nil)
    {
#if OSX
        CGRect screenFrame = [[NSScreen mainScreen]frame];
#elif IOS
        CGRect screenFrame = [[UIScreen mainScreen]bounds];
#endif
        float2 dim = float2(screenFrame.size.width,screenFrame.size.height);
        CreateDeviceResult device_create_result = RendererCode::InitGraphics(dim,0);
        if(device_create_result.is_init)
        {
            self.view = (MTKViewDelegateView*)PlatformGraphicsAPI_Metal::GetView();
#if OSX
            [self.view.window setBackgroundColor: NSColor.whiteColor];
            [self.view.window setCollectionBehavior:NSWindowCollectionBehaviorFullScreenPrimary];
            [self.view.window setFrame:screenFrame display:YES];
            [self.view.window toggleFullScreen:self];
            [self.view setFrameSize:screenFrame.size];
#endif
            RendererCode::SetGraphicsRenderCallback(OnPlatformRender);
            PlatformOutput(true, "RenderGraphics device Initialized!!\n");
        }
        else
        {
            Assert(false);
        }

//TODO(Ray):Here we will do all the engine start up stuff

//Then start the game loops and initialization
//        InitApp(dim);
    }
    return self;
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    NSLog(@"VIEW DID LOAD");
}

@end
