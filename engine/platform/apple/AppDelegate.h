//
//  AppDelegate.h
//  metalizertest
//
//  Created by Ray Olen Garner on 2018/12/10.
//  Copyright © 2018 DENA. All rights reserved.
//
#if OSX
#import <Cocoa/Cocoa.h>
@interface AppDelegate : NSObject <NSApplicationDelegate>

@property (assign) IBOutlet NSWindow *window;

@end
#elif IOS
#import <UIKit/UIKit.h>

@interface AppDelegate : UIResponder <UIApplicationDelegate>

@property (strong, nonatomic) UIWindow *window;

@end

#endif

