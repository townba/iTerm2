#import <Foundation/Foundation.h>
#import "AATree.h"
#import "iTermTuple.h"

NS_ASSUME_NONNULL_BEGIN

@class IntervalTreeEntry;
@protocol IntervalTreeObject;

@interface Interval : NSObject<NSCopying>
// Negative locations have special meaning. Don't use them.
@property(nonatomic, assign) long long location;
@property(nonatomic, assign) long long length;
@property(nonatomic, readonly) long long limit;

+ (instancetype)intervalWithLocation:(long long)location length:(long long)length;
+ (Interval *)maxInterval;
// One more than the largest value in the interval.
- (BOOL)intersects:(Interval *)other;
- (BOOL)isEqualToInterval:(Interval *)interval;

// Serialized value.
- (NSDictionary *)dictionaryValue;

@end

@protocol IntervalTreeImmutableObject<NSObject>
@property(nonatomic, readonly) IntervalTreeEntry *entry;

// Serialized value.
- (NSDictionary *)dictionaryValue;

// A parallel object whose state will be eventually consistent with this one.
- (id<IntervalTreeObject>)doppelganger;
@end

@protocol IntervalTreeObject <IntervalTreeImmutableObject, NSObject>
// Deserialize from dictionaryValue.
- (instancetype)initWithDictionary:(NSDictionary *)dict;

@property(nullable, nonatomic, weak) IntervalTreeEntry *entry;

// Serialized value.
- (NSDictionary *)dictionaryValue;

- (instancetype)copyOfIntervalTreeObject;
@end

@protocol IntervalTreeImmutableEntry<NSObject>
@property(nonatomic, readonly) Interval *interval;
@property(nonatomic, readonly) id<IntervalTreeImmutableObject> object;
@end

// A node in the interval tree will contain one or more entries, each of which has an interval and an object. All intervals should have the same location.
@interface IntervalTreeEntry : NSObject<IntervalTreeImmutableEntry>
@property(nonatomic, retain) Interval *interval;
@property(nonatomic, retain) id<IntervalTreeObject> object;

+ (IntervalTreeEntry *)entryWithInterval:(Interval *)interval object:(id<IntervalTreeObject>)object;
@end

@protocol IntervalTreeReading<NSObject>

@property(nonatomic, readonly) NSString *debugString;
- (NSArray<id<IntervalTreeImmutableObject>> *)objectsInInterval:(Interval *)interval;
- (NSArray<id<IntervalTreeImmutableObject>> *)allObjects;
- (BOOL)containsObject:(id<IntervalTreeImmutableObject> _Nullable)object;

// Returns the object with the highest limit
- (NSArray<id<IntervalTreeImmutableObject>> * _Nullable)objectsWithLargestLimit;
// Returns the object with the smallest limit
- (NSArray<id<IntervalTreeImmutableObject>> * _Nullable)objectsWithSmallestLimit;

// Returns the object with the largest location
- (NSArray<id<IntervalTreeImmutableObject>> *_Nullable)objectsWithLargestLocation;

// Returns the object with the largest location before (but NOT AT) |location|.
- (NSArray<id<IntervalTreeImmutableObject>> *_Nullable)objectsWithLargestLocationBefore:(long long)location;

- (NSArray<id<IntervalTreeImmutableObject>> *_Nullable)objectsWithLargestLimitBefore:(long long)limit;
- (NSArray<id<IntervalTreeImmutableObject>> *_Nullable)objectsWithSmallestLimitAfter:(long long)limit;

// Enumerates backwards by location (NOT LIMIT)
- (NSEnumerator<IntervalTreeImmutableObject> *)reverseEnumeratorAt:(long long)start;

- (NSEnumerator<IntervalTreeImmutableObject> *)reverseLimitEnumeratorAt:(long long)start;
- (NSEnumerator<IntervalTreeImmutableObject> *)forwardLimitEnumeratorAt:(long long)start;
- (NSEnumerator<IntervalTreeImmutableObject> *)reverseLimitEnumerator;
- (NSEnumerator<IntervalTreeImmutableObject> *)forwardLimitEnumerator;

// Serialize, adding offset to interval locations (useful for taking the tail
// of an interval tree).
- (NSDictionary *)dictionaryValueWithOffset:(long long)offset;
@end


@interface IntervalTree : NSObject <AATreeDelegate, IntervalTreeReading>

@property(nonatomic, readonly) NSInteger count;

// Deserialize
- (instancetype)initWithDictionary:(NSDictionary *)dict;

// |object| should implement -hash.
- (void)addObject:(id<IntervalTreeObject>)object withInterval:(Interval *)interval;
- (void)removeObject:(id<IntervalTreeObject>)object;

// NOTE: This leaves the entry set on objects so you can recover the interval. You must nil it out
// before adding an object to another tree.
- (void)removeAllObjects;
- (iTermTuple<Interval *, id<IntervalTreeObject>> *)dropFirst;

- (void)sanityCheck;

// For subclasses;
- (void)restoreFromDictionary:(NSDictionary *)dict;

@end

NS_ASSUME_NONNULL_END
