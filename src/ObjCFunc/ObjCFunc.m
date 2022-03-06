#import <Foundation/Foundation.h>
#include "ObjCFunc.h"
@implementation myClass
//Your objective c code here....
-(void)hello:(int)num1
{
NSLog(@"Hello!!!!!!");
}
@end

//clang -framework Foundation ObjCFunc.m -c