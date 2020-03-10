//
//  WNetSpeedTempC.h
//  WNetSpeedSampleCode
//
//  Created by 김현준 on 2020/03/09.
//  Copyright © 2020 김현준. All rights reserved.
//

#ifndef WNetSpeed_h
#define WNetSpeed_h

#include <stdio.h>
/*
 수요일 11시 리뷰 가능여부
 데이터세이프
 */
///초당 스피드 로그구분
typedef enum WNetSpeedSize {
    WN_dByte_S,
    WN_KByte_S,
    WN_MByte_S,
}WNetSpeedSize;

typedef enum WNetSpeedMode {
    WNetSpeedModeUpload,
    WNetSpeedModeDownload,
}WNetSpeedMode;

#pragma mark - Interface

void WNStart(void (*callback)(unsigned long long speed,int upload,void *target), void *target);
void WNStop(void);
__attribute__((overloadable)) double AverageCurrentSpeed(WNetSpeedMode mode);
__attribute__((overloadable)) double AverageCurrentSpeed(WNetSpeedMode mode, WNetSpeedSize logType);


#endif /* WNetSpeed_h */
