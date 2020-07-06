//
//  PdKeychainBindingsController.h
//  PdKeychainBindingsController
//
//  Created by Carl Brown on 7/10/11.
//  Copyright 2011 PDAgent, LLC. Released under MIT License.
//

#import <Foundation/Foundation.h>
#import "PdKeychainBindings.h"


@interface PdKeychainBindingsController : NSObject {
@private
    PdKeychainBindings *_keychainBindings;
    NSMutableDictionary *_valueBuffer;
}

+ (PdKeychainBindingsController *)sharedKeychainBindingsController;
- (PdKeychainBindings *) keychainBindings;

- (id)values;    // accessor object for PdKeychainBindings values. This property is observable using key-value observing.

- (NSString*)stringForKey:(NSString*)key;
- (BOOL)storeString:(NSString*)string forKey:(NSString*)key;
- (BOOL)storeString:(NSString*)string forKey:(NSString*)key accessibleAttribute:(CFTypeRef)accessibleAttribute;
- (void)setValue:(id)value forKeyPath:(NSString *)keyPath accessibleAttribute:(CFTypeRef)accessibleAttribute;
@end

