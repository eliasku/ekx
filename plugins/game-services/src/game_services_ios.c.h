#import <GameKit/GameKit.h>

BOOL GC_gameCenterEnabled = NO;
NSString* GC_leaderboardIdentifier;
UIViewController* GC_rootViewController;

@interface GameCenterDelegate: NSObject<GKGameCenterControllerDelegate>
@end

@implementation GameCenterDelegate
- (void)gameCenterViewControllerDidFinish:(GKGameCenterViewController*)gameCenterViewController {
    [GC_rootViewController dismissViewControllerAnimated:YES completion:nil];
}

@end

GameCenterDelegate* game_center_delegate;

/** Game Center **/
void GC_authenticateLocalPlayer(void) {
    game_center_delegate = [GameCenterDelegate new];
    GC_rootViewController = ek_app_delegate.window.rootViewController;
    GKLocalPlayer* localPlayer = [GKLocalPlayer localPlayer];

    localPlayer.authenticateHandler = ^(UIViewController* viewController, NSError* error) {
        if (viewController != nil) {
            [GC_rootViewController presentViewController:viewController animated:YES completion:nil];
        } else {
            if ([GKLocalPlayer localPlayer].authenticated) {
                GC_gameCenterEnabled = YES;

                // Get the default leaderboard identifier.
                [[GKLocalPlayer localPlayer] loadDefaultLeaderboardIdentifierWithCompletionHandler:^(
                        NSString* leaderboardIdentifier, NSError* error) {

                    if (error != nil) {
                        NSLog(@"%@", [error localizedDescription]);
                    } else {
                        GC_leaderboardIdentifier = leaderboardIdentifier;
                    }
                }];
            } else {
                NSLog(@"GameCenter is disabled by user: %@", [error localizedDescription]);
                GC_gameCenterEnabled = NO;
            }
        }
    };
}

void ek_game_services_init(void) {
    log_debug("game-services initialize");
    GC_authenticateLocalPlayer();
}

void ek_leaderboard_show(const char* ignored) {
    if (GC_leaderboardIdentifier != nil) {
        GKGameCenterViewController* gcViewController = [[GKGameCenterViewController alloc] init];
        gcViewController.gameCenterDelegate = game_center_delegate;

        gcViewController.viewState = GKGameCenterViewControllerStateLeaderboards;
        gcViewController.leaderboardIdentifier = GC_leaderboardIdentifier;
        //    gcViewController.viewState = GKGameCenterViewControllerStateAchievements;

        [GC_rootViewController presentViewController:gcViewController animated:YES completion:nil];
    }
}

void ek_leaderboard_submit(const char* ignored, int score) {
    if (GC_leaderboardIdentifier != nil) {
        GKScore* gkScore = [[GKScore alloc] initWithLeaderboardIdentifier:GC_leaderboardIdentifier];
        gkScore.value = score;

        [GKScore reportScores:@[gkScore] withCompletionHandler:^(NSError* error) {
            if (error != nil) {
                NSLog(@"%@", [error localizedDescription]);
            }
        }];
    }
}

void ek_achievement_update(const char* id, int increment) {
    // TODO: incremental ID
    if (increment != 0) return;

    NSString* s_achievement_id = [NSString stringWithUTF8String:id];
    GKAchievement* ach = [[GKAchievement alloc] initWithIdentifier:s_achievement_id];
    ach.percentComplete = 100.0f;

    NSArray* achievements = @[ach];
    [GKAchievement reportAchievements:achievements withCompletionHandler:^(NSError* error) {
        if (error != nil) {
            NSLog(@"%@", [error localizedDescription]);
        }
    }];
}

void ek_achievement_show(void) {
    if(!GC_gameCenterEnabled) {
        // TODO: handle disabled game center with alert pop-up
        return;
    }
    GKGameCenterViewController* gcViewController = [[GKGameCenterViewController alloc] init];
    gcViewController.gameCenterDelegate = game_center_delegate;
    gcViewController.viewState = GKGameCenterViewControllerStateAchievements;
    [GC_rootViewController presentViewController:gcViewController animated:YES completion:nil];
}
