#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>
#import "MyObject-C-Interface.h"

@interface MyObject : NSObject <NSApplicationDelegate>
{
    int someVar;
}

- (int)  doSomethingWith:(int) aParameter;
- (void) logMyMessage:(char *) aCStr;

@end