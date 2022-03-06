#import <Foundation/Foundation.h>
#include "ObjCCall.h"
#include "ObjCFunc.h"
void ObjCCall::objectiveC_Call()
{
//Objective C code calling.....
myClass *obj=[[myClass alloc]init]; //Allocating the new object for the objective C   class we created
[obj hello:(100)];   //Calling the function we defined
}