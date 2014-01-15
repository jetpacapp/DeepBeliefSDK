//
//  JPViewController.m
//  jpcnn_ios
//
//  Created by Peter Warden on 1/15/14.
//  Copyright (c) 2014 Jetpac, Inc. All rights reserved.
//

#import "JPViewController.h"

void run_image_test(const char* graphFileName, const char* imageFileName);

@interface JPViewController ()

@end

@implementation JPViewController

- (void)viewDidLoad
{
    [super viewDidLoad];

    NSString* networkPath = [[NSBundle mainBundle] pathForResource:@"imagenet" ofType:@"ntwk"];
    NSString* imagePath = [[NSBundle mainBundle] pathForResource:@"lena" ofType:@"png"];

    run_image_test([networkPath UTF8String], [imagePath UTF8String]);
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

@end
