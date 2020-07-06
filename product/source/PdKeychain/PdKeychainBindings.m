//
//  PdKeychainBindings.m
//  PdKeychainBindings
//
//  Created by Carl Brown on 7/10/11.
//  Copyright 2011 PDAgent, LLC. Released under MIT License.
//

#import "PdKeychainBindings.h"
#import "PdKeychainBindingsController.h"

@implementation PdKeychainBindings

+ (PdKeychainBindings *)sharedKeychainBindings
{
	return [[PdKeychainBindingsController sharedKeychainBindingsController] keychainBindings];
}

- (id)objectForKey:(NSString *)defaultName {
    //return [[[PdKeychainBindingsController sharedKeychainBindingsController] valueBuffer] objectForKey:defaultName];
    return [[PdKeychainBindingsController sharedKeychainBindingsController] valueForKeyPath:[NSString stringWithFormat:@"values.%@",defaultName]];
}

- (void)setObject:(NSString *)value forKey:(NSString *)defaultName {
    [[PdKeychainBindingsController sharedKeychainBindingsController] setValue:value forKeyPath:[NSString stringWithFormat:@"values.%@",defaultName]];
}

- (void)setObject:(NSString *)value forKey:(NSString *)defaultName accessibleAttribute:(CFTypeRef)accessibleAttribute {
    [[PdKeychainBindingsController sharedKeychainBindingsController] setValue:value forKeyPath:[NSString stringWithFormat:@"values.%@",defaultName] accessibleAttribute:accessibleAttribute];
}

- (void)setString:(NSString *)value forKey:(NSString *)defaultName {
    [[PdKeychainBindingsController sharedKeychainBindingsController] setValue:value forKeyPath:[NSString stringWithFormat:@"values.%@",defaultName]];
}

- (void)setString:(NSString *)value forKey:(NSString *)defaultName accessibleAttribute:(CFTypeRef)accessibleAttribute {
    [[PdKeychainBindingsController sharedKeychainBindingsController] setValue:value forKeyPath:[NSString stringWithFormat:@"values.%@",defaultName] accessibleAttribute:accessibleAttribute];
}

- (void)removeObjectForKey:(NSString *)defaultName {
    [[PdKeychainBindingsController sharedKeychainBindingsController] setValue:nil forKeyPath:[NSString stringWithFormat:@"values.%@",defaultName]];
}

- (NSString *)stringForKey:(NSString *)defaultName {
    return (NSString *) [self objectForKey:defaultName];
}

@end
