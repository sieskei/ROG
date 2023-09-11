//
//  MyWrapper.h
//  Gadget
//
//  Created by Miroslav Yozov on 9.09.23.
//

#ifndef MyWrapper_h
#define MyWrapper_h

#import <Foundation/Foundation.h>

@interface HIDWrapper : NSObject

- (instancetype)init;
- (int)initDriver;

@end

#endif /* MyWrapper_h */
