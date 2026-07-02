#include "APIScreen.h"
#include "Paperweight.h"
#include "Hardweight.h"
#include <UIKit/UIKit.h>
#include <dispatch/dispatch.h>

static APILevel::Level g_selected = APILevel::NONE;

APILevel::Level APILevel::getSelected() {
    return g_selected;
}

static void initPaperweight() {
    if (g_selected != APILevel::NONE) return;
    g_selected = APILevel::PAPERWEIGHT;
    Paperweight::init();
    printf("[MCCL] Paperweight API selected\n");
}

static void initHardweight() {
    if (g_selected != APILevel::NONE) return;
    g_selected = APILevel::HARDWEIGHT;
    Hardweight::init();
    printf("[MCCL] Hardweight API selected\n");
}

void showAPISelector() {
    dispatch_async(dispatch_get_main_queue(), ^{
        UIAlertController* alert = [UIAlertController
            alertControllerWithTitle:@"Minecraft Client Mod"
            message:@"Select API tier"
            preferredStyle:UIAlertControllerStyleAlert];

        UIAlertAction* paperweight = [UIAlertAction
            actionWithTitle:@"Paperweight"
            style:UIAlertActionStyleDefault
            handler:^(UIAlertAction* action) {
                initPaperweight();
            }];

        UIAlertAction* hardweight = [UIAlertAction
            actionWithTitle:@"Hardweight"
            style:UIAlertActionStyleDestructive
            handler:^(UIAlertAction* action) {
                initHardweight();
            }];

        [alert addAction:paperweight];
        [alert addAction:hardweight];

        UIWindow* window = [UIApplication sharedApplication].keyWindow;
        UIViewController* root = window.rootViewController;
        while (root.presentedViewController) {
            root = root.presentedViewController;
        }
        [root presentViewController:alert animated:YES completion:nil];
    });
}
