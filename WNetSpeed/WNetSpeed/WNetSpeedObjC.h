//
//  WNetSpeedObjC.h
//  Genie
//
//  Created by 김현준 on 2020/03/08.
//  Copyright © 2020 wally. All rights reserved.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN
@protocol WNetSpeedDelegate <NSObject>
@optional
- (void)downloadSpeed:(unsigned long long)speed;
- (void)uploadSpeed:(unsigned long long)speed;
@end

///초당 스피드 로그구분
typedef enum WNetSpeedDebugLog {
    WN_dByte_S,
    WN_KByte_S,
    WN_MByte_S,
}WNetSpeedDebugLog;

typedef enum WNetSpeedMode {
    WNetSpeedModeUpload,
    WNetSpeedModeDownload,
}WNetSpeedMode;


@interface WNetSpeedObjC : NSObject
@property (nonatomic, weak) id<WNetSpeedDelegate> delegate;

void WNStart(id target);
void WNStop(void);

__attribute__((overloadable)) double AverageCurrentSpeed(WNetSpeedMode mode);
__attribute__((overloadable)) double AverageCurrentSpeed(WNetSpeedMode mode, WNetSpeedDebugLog logType);
unsigned long long CurrentSpeed(WNetSpeedDebugLog logType);

@end

NS_ASSUME_NONNULL_END
