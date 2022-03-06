#import "MyObject.h"

@implementation MyObject
{
    NSMutableArray<NSStatusItem*>* o;
}

MyClassImpl::MyClassImpl( void )
    : self( NULL )
{  
}

MyClassImpl::~MyClassImpl( void )
{
    [(id)self dealloc];
}

void MyClassImpl::init( void )
{    
    self = [[MyObject alloc] init];
}

int MyClassImpl::doSomethingWith( int aParameter )
{
    return [(id)self doSomethingWith:aParameter];
}

void MyClassImpl::logMyMessage( char *aCStr )
{
    [(id)self logMyMessage:aCStr];
}

- (int) doSomethingWith:(int) aParameter
{
    someVar = 2222;
    NSLog(@"doSomethingWith");
    NSLog(@"%d", aParameter);
    int result = 42;
    return result;
}

- (void) logMyMessage:(char *) aCStr
{
    //NSLog( aCStr );
    NSLog(@"%d", self->someVar);
}

@end