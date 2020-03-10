//
//  ViewController.m
//  WNetSpeedSampleiOS
//
//  Created by 김현준 on 2020/03/10.
//  Copyright © 2020 Wally. All rights reserved.
//

#import "ViewController.h"

#include "WNetSpeed.h"

@interface ViewController ()

@end

@implementation ViewController

static void WNCallBack(unsigned long long speed,int upload,void *target)
{
    @synchronized (target) {
        dispatch_async(dispatch_get_main_queue(), ^{
            ViewController *vc = (__bridge ViewController *)target;
            if (upload == 1) {
                double average = AverageCurrentSpeed(WNetSpeedModeUpload,WN_MByte_S);
                if (average > 1000)return;
                vc.uploadTextfield.text = [NSString stringWithFormat:@"%.1f MB/s",average];
                vc.uploadTextfield2.text = [NSString stringWithFormat:@"%.1f Mbps",average*8];
            }else{
                double average = AverageCurrentSpeed(WNetSpeedModeDownload,WN_MByte_S);
                NSLog(@"%@",@(average/(1024*1024)));
                if (average > 1000)return;
                vc.downloadTextfield.text = [NSString stringWithFormat:@"%.1f MB/s",average];
                vc.downloadTextfield2.text = [NSString stringWithFormat:@"%.1f Mbps",average*8];
            }
        });
    }
}

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view.
    
    WNStart(&WNCallBack,(__bridge void *)(self));
}


@end
