#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>
#include "ObjCFunc.h"

@implementation myClass


    NSMutableArray<NSStatusItem*>* o;


-(void)hello:(int)num1
{
    NSLog(@"%d", num1);
    o=[NSMutableArray array];

    NSStatusItem*item=[[NSStatusBar systemStatusBar] statusItemWithLength:NSVariableStatusItemLength];
    item.button.title=@"foo";
    item.visible=YES;
    //[item addObserver:self forKeyPath:@"button.effectiveAppearance" options:NSKeyValueObservingOptionNew|NSKeyValueObservingOptionInitial context:nil];
    //[item addObserver:self forKeyPath:@"button.frame" options:NSKeyValueObservingOptionNew|NSKeyValueObservingOptionInitial context:nil];
    [o addObject:item];

    NSLog(@"Hello!!!!!!");
}

@end

// clang -c ObjCFunc.m