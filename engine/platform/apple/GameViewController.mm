
#import "GameViewController.h"

#include "../../yoyo/RendererInclude.h"
#import  "MTKViewDelegateView.h"

//NOTE(Ray):
//#import "SpriteExample.h"
#import "../../engine.h"
//Things in the platform layer just return or fill out the raw results.
//The engine will tranlate those to useful resutls.
void PullMouseState(PlatformState* ps)
{
    NSPoint mouseLoc;
    mouseLoc = [NSEvent mouseLocation]; //get current mouse position
    NSLog(@"Mouse location: %f %f", mouseLoc.x, mouseLoc.y);

}

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

            RendererCode::SetGraphicsRenderCallback(Engine::Update);
            PlatformOutput(true, "RenderGraphics device Initialized!!\n");

        }
        else
        {
            Assert(false);
        }

        //TODO(Ray):Ensure that we dont run update until after engine startup is complete.
        Engine::Init(dim);
    }
    return self;
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    NSLog(@"VIEW DID LOAD");
}

@end
