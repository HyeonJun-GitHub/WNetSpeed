//
//  WNetSpeed.c
//  WNetSpeedSampleCode
//
//  Created by 김현준 on 2020/03/09.
//  Copyright © 2020 김현준. All rights reserved.
//

#include "WNetSpeed.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ifaddrs.h>
#include <net/if.h>

#include <pthread.h>
#include <signal.h>
#include <sys/time.h>

#define null (void *)0

static int const WNetSpeedZeroCount = 20;
static int const WNetSpeedMemoryMax = 1000;
static int const MSEC = 1000000;
static float const WNetSpeedCheckDelay = (MSEC/2) * 0.000001;

struct WNetThread {
    pthread_t p_thread;             //단일 쓰레드
    int thr_id;                     //id
    int status;                     //상태값
    struct itimerval timer;         //타이머
};

struct NetSpeedHistory {
    WNetSpeedMode mode;             //모드
    unsigned int cnt;               //카운트
    unsigned long long total;       //전체 합
    unsigned long long speed[WNetSpeedMemoryMax]; //값
    unsigned int zeroCnt;           //종료를 알림
};

typedef struct {
    struct NetSpeedHistory *uploadSpeeds;
    struct NetSpeedHistory *downloadSpeeds;
    _Bool firstTime;
    unsigned long long downloadData;
    unsigned long long lastDownloadData;
    unsigned long long uploadData;
    unsigned long long lastUploadData;
    struct WNetThread *wnThread;
    
    void *target;
    void (*WNCallback)(unsigned long long speed,int upload,void *target);
}WNetSpeed;

#pragma mark - WNetSpeed Define Function

void WNetCurrentSpeed(void);
void SetValue(struct NetSpeedHistory *history,unsigned long long value);

#pragma mark - WNetSpeed initialized
void Initialized(struct NetSpeedHistory *history) {
    history->cnt = 0;
    history->total = 0;
    history->zeroCnt = 0;
//    memset(history->speed, 0, WNetSpeedMemoryMax * sizeof(unsigned long long));
}

WNetSpeed *shareInstance() {
    static WNetSpeed *instance = NULL;
    if (instance == null) {
        instance = malloc(sizeof(*instance));
        instance->firstTime = 1;
        instance->downloadData = 0;
        instance->uploadData = 0;
        instance->downloadSpeeds = &((struct NetSpeedHistory){0});
        instance->downloadSpeeds->mode = WNetSpeedModeDownload;
        instance->uploadSpeeds = &((struct NetSpeedHistory){0});
        instance->uploadSpeeds->mode = WNetSpeedModeUpload;
        instance->wnThread = &((struct WNetThread){0});
    }
    
    return instance;
}

#pragma mark - WNetSpeed timer

void timer_handler (int signum) {
    WNetCurrentSpeed();
}

void *StartNetThread(void *sender)
{
    struct WNetThread *thread = (struct WNetThread *)sender;
    
    //타이머 생성..
    struct sigaction sa;
    struct itimerval timer;
    
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &timer_handler;
    sigaction(SIGVTALRM, &sa, NULL);
    
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = MSEC/2;
    
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = MSEC/2;
    
    setitimer(ITIMER_VIRTUAL, &timer, NULL);
    thread->timer = timer;
    while (1);
    
    return NULL;
}

#pragma mark - WNetSpeed Data

void WNetCurrentSpeed(void) {
    WNetSpeed *wn = shareInstance();
    struct ifaddrs *addrs;
    if (getifaddrs(&addrs) == 0) {
        while (addrs != NULL) {
            struct if_data *ifa_data = (struct if_data*)addrs->ifa_data;
            if(ifa_data){
                wn->downloadData += ifa_data->ifi_ibytes;
                wn->uploadData += ifa_data->ifi_obytes;
            }
            addrs = addrs->ifa_next;
        }
    }
    freeifaddrs(addrs);
    
    if (wn->firstTime == 0) {
        unsigned long long speed;
        speed = (wn->downloadData - wn->lastDownloadData)/WNetSpeedCheckDelay;
        SetValue(wn->downloadSpeeds,speed);
        if (wn->WNCallback != NULL) {
            wn->WNCallback(speed,1,wn->target);
        }
        
        speed = (wn->uploadData - wn->lastUploadData)/WNetSpeedCheckDelay;
        SetValue(wn->uploadSpeeds,speed);
        if (wn->WNCallback != NULL) {
            wn->WNCallback(speed,0,wn->target);
        }
    }else{
        wn->firstTime = 0;
    }
    wn->lastDownloadData = wn->downloadData;
    wn->lastUploadData = wn->uploadData;
    wn->downloadData = 0;
    wn->uploadData = 0;
}

void SetValue(struct NetSpeedHistory *history,unsigned long long value) {
    
    //5초 사용검토 후 초기화..
    if (value == 0) {
        history->zeroCnt++;
        if (history->zeroCnt > WNetSpeedZeroCount) {
            if (history->mode == WNetSpeedModeDownload) {
                printf("Download Initialized Memory\n");
            }else{
                printf("Upload Initialized Memory\n");
            }
            Initialized(history);
            return;
        }
        if (history->mode == WNetSpeedModeDownload) {
            printf("Download zeroCnt : %d\n",history->zeroCnt);
        }else{
            printf("Upload zeroCnt : %d\n",history->zeroCnt);
        }
        return;
    }
    
    if (history->cnt > WNetSpeedMemoryMax) {
        //디스크에 저장.
        
        //초기화
        Initialized(history);
    }
    
    history->speed[history->cnt] = value;
    history->cnt++;
    history->total += value;
    history->zeroCnt = 0;
}

#pragma mark - WNetSpeed interface
void WNStart(void (*callback)(unsigned long long speed,int upload,void *target),void *target) {
    WNetSpeed *wn = shareInstance();
    wn->WNCallback = callback;
    wn->target = target;
    Initialized(wn->uploadSpeeds);
    Initialized(wn->downloadSpeeds);
    
    int thrId = pthread_create(&wn->wnThread->p_thread, NULL, StartNetThread, (struct WNetThread *)wn->wnThread);
    wn->wnThread->thr_id = thrId;
    if (wn->wnThread->thr_id < 0)
    {
        perror("스래드 생성 생성 중 오류발생");
        exit(0);
    }
}

void WNStop(void) {
//    WNetSpeed *wn = shareInstance();
//    pthread_kill(wn->wnThread->p_thread, 0);
}

__attribute__((overloadable)) double AverageCurrentSpeed(WNetSpeedMode mode) {
    WNetSpeed *wn = shareInstance();
    struct NetSpeedHistory *history = mode==WNetSpeedModeUpload?wn->uploadSpeeds:wn->downloadSpeeds;
    if (history->cnt == 0 || history->total == 0) {
        return 0;
    }
    return history->total / history->cnt;
}

__attribute__((overloadable)) double AverageCurrentSpeed(WNetSpeedMode mode, WNetSpeedSize logType) {
    if (logType == WN_dByte_S)return AverageCurrentSpeed(mode);
    if (logType == WN_KByte_S)return AverageCurrentSpeed(mode) / 1024;
    if (logType == WN_MByte_S)return AverageCurrentSpeed(mode) / (1024*1024);
    
    return 0;
}
