//
//  Renderer.h
//  pedaltothemetal
//
//  Created by Ray Olen Garner on 2019/01/09.
//  Copyright © 2019 DeNA. All rights reserved.
//

#import <MetalKit/MetalKit.h>

// Our platform independent renderer class.   Implements the MTKViewDelegate protocol which
//   allows it to accept per-frame update and drawable resize callbacks.
@interface Renderer : NSObject <MTKViewDelegate>

-(nonnull instancetype)initWithMetalKitView:(nonnull MTKView *)view;

@end

