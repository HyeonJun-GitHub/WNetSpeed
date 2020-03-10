//
//  ViewController.m
//  WNetSpeedSampleCode
//
//  Created by 김현준 on 2020/03/08.
//  Copyright © 2020 김현준. All rights reserved.
//

#define USE_C 1

#import "ViewController.h"
#if USE_C
#import "WNetSpeed.h"
@interface ViewController()
#else
#import "WNetSpeedObjC.h"
@interface ViewController()<WNetSpeedDelegate>
#endif

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
                vc.uploadTextfield.stringValue = [NSString stringWithFormat:@"%.1f MB/s",average];
                vc.uploadTextfield2.stringValue = [NSString stringWithFormat:@"%.1f Mbps",average*8];
            }else{
                double average = AverageCurrentSpeed(WNetSpeedModeDownload,WN_MByte_S);
                if (average > 1000)return;
                vc.downloadTextfield.stringValue = [NSString stringWithFormat:@"%.1f MB/s",average];
                vc.downloadTextfield2.stringValue = [NSString stringWithFormat:@"%.1f Mbps",average*8];
            }
            
        });
    }
}

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view.
#if USE_C
    WNStart(&WNCallBack,(__bridge void *)(self));
}
#else
    WNStart(self);
}

-(void)downloadSpeed:(unsigned long long)speed {
    double average = AverageCurrentSpeed(WNetSpeedModeDownload,WN_MByte_S);
    dispatch_async(dispatch_get_main_queue(), ^{
        self.downloadTextfield.stringValue = [NSString stringWithFormat:@"%.1f MB/s",average];
        self.downloadTextfield2.stringValue = [NSString stringWithFormat:@"%.1f Mbps",average*8];
    });
}

-(void)uploadSpeed:(unsigned long long)speed {
    double average = AverageCurrentSpeed(WNetSpeedModeUpload,WN_MByte_S);
    dispatch_async(dispatch_get_main_queue(), ^{
        self.uploadTextfield.stringValue = [NSString stringWithFormat:@"%.1f MB/s",average];
        self.uploadTextfield2.stringValue = [NSString stringWithFormat:@"%.1f Mbps",average*8];
    });
}
#endif

- (void)setRepresentedObject:(id)representedObject {
    [super setRepresentedObject:representedObject];
    // Update the view, if already loaded.
}

@end
