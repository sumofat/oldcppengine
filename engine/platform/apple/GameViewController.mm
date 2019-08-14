
#import "GameViewController.h"

#include "../../metalizer/RendererInclude.h"
#import  "../../metalizer/renderer/MTKViewDelegateView.h"
//NOTE(Ray):
//#import "SpriteExample.h"
#import "../../engine.h"
#if OSX
#import "../../external/imgui/examples/imgui_impl_osx.h"
#import "../../external/imgui/examples/imgui_impl_osx.mm"

//Things in the platform layer just return or fill out the raw results.
//The engine will tranlate those to useful resutls.
static NSView *globalview;

void OnIMGUIEvent(NSEvent* event)
{
    ImGui_ImplOSX_HandleEvent(event, globalview);
}
#elif IOS
static UIView *globalview;
#endif
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
        float2 dim = float2(screenFrame.size.width,screenFrame.size.height);
#elif IOS
        CGRect screenFrame = [[UIScreen mainScreen]bounds];
        float2 dim = float2(screenFrame.size.width,screenFrame.size.height) * [UIScreen mainScreen].nativeScale;
#endif
       
        CreateDeviceResult device_create_result = RendererCode::InitGraphics(dim,0);
        if(device_create_result.is_init)
        {
            self.view = (__bridge MTKViewDelegateView*)PlatformGraphicsAPI_Metal::GetView();
            globalview = self.view;
#if OSX
            //[self.view.window makeFirstResponder:self.view];
            //[self.view.window acceptsMouseMovedEvents];
            //[self.view.window setAcceptsMouseMovedEvents:YES];
            
            PlatformInputAPI_Metal::SetOnMouseUp(OnIMGUIEvent);
            PlatformInputAPI_Metal::SetOnMouseDown(OnIMGUIEvent);
            //NOTE(Ray):By default off unless left mouse pressed so as not to flodd event queue
            PlatformInputAPI_Metal::SetOnMouseMoved(OnIMGUIEvent);
            PlatformInputAPI_Metal::SetOnScrollWheel(OnIMGUIEvent);
            PlatformInputAPI_Metal::SetOnMouseDragged(OnIMGUIEvent);
            PlatformInputAPI_Metal::SetOnKeyDown(OnIMGUIEvent);
            PlatformInputAPI_Metal::SetOnKeyUp(OnIMGUIEvent);
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
#if OSX
        ImGui_ImplOSX_Init();
#elif IOS
#endif
    }
    return self;
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    NSLog(@"VIEW DID LOAD");
}

@end
