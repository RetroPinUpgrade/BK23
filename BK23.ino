/**************************************************************************
    BK23 is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    See <https://www.gnu.org/licenses/>.
*/

#include "RPU_Config.h"
#include "RPU.h"
#include "DropTargets.h"
#include "BK.h"
#include "SelfTestAndAudit.h"
#include "AudioHandler.h"
#include "DisplayHandler.h"
#include "LampAnimations.h"
#include <EEPROM.h>

// todo:
//    * jackpot reset drops during double combat when kicked?
//    * third lock light for jackpots in double combat (maybe cycle lamps?)
//    * triple combat doesn't work - held ball and said lost
//    sound for upper lock (no collect)
//    bonus going up too much for clear (multiple clears?)
//    King's challenge ending announcementannouncement
//    * Magna save should light appropriate inlane
//    * do 6x with 4 & 2 instead of double 3
//    sound effects 74-77
//    laugh if magnet and then outlane
//    * tilt GI out too long
//    king's challenge not long enough - adjustable time
//    * outlane +2 bonus, inlane +5 bonus
//    * switch melee and perfection bank #s
//    Combo names
//    Combo rewards
//    Coop mode
//    HORSE mode
//    
//



/* Callouts needed

    Both red buttons for Single Combat Part 2
    Both red buttons for Single Combat Part 3

    Single combat instructions for different levels
    Single combat 1 - upper & lower bank to qualify death blow
    Single combat 2 - three banks for death blow
    Single combat 3 - all four banks for death blow

    Single combat saucer for deathblow

    Single combat ten seconds left (jackpot qualified)
    single combat ten seconds left (jackpot not qualified)
    Single combat ran out of time





*/


#define USE_SCORE_OVERRIDES
#define BK_MAJOR_VERSION  2023
#define BK_MINOR_VERSION  1
#define DEBUG_MESSAGES  1



/*********************************************************************

    Game specific code

*********************************************************************/

// MachineState
//  0 - Attract Mode
//  negative - self-test modes
//  positive - game play
char MachineState = 0;
boolean MachineStateChanged = true;
#define MACHINE_STATE_ATTRACT         0
#define MACHINE_STATE_INIT_GAMEPLAY   1
#define MACHINE_STATE_INIT_NEW_BALL   2
#define MACHINE_STATE_NORMAL_GAMEPLAY 4
#define MACHINE_STATE_COUNTDOWN_BONUS 99
#define MACHINE_STATE_BALL_OVER       100
#define MACHINE_STATE_MATCH_MODE      110
#define MACHINE_STATE_DIAGNOSTICS     120

#define MACHINE_STATE_ADJUST_FREEPLAY             (MACHINE_STATE_TEST_DONE-1)
#define MACHINE_STATE_ADJUST_BALL_SAVE            (MACHINE_STATE_TEST_DONE-2)
#define MACHINE_STATE_ADJUST_SOUND_SELECTOR       (MACHINE_STATE_TEST_DONE-3)
#define MACHINE_STATE_ADJUST_MUSIC_VOLUME         (MACHINE_STATE_TEST_DONE-4)
#define MACHINE_STATE_ADJUST_SFX_VOLUME           (MACHINE_STATE_TEST_DONE-5)
#define MACHINE_STATE_ADJUST_CALLOUTS_VOLUME      (MACHINE_STATE_TEST_DONE-6)
#define MACHINE_STATE_ADJUST_TOURNAMENT_SCORING   (MACHINE_STATE_TEST_DONE-7)
#define MACHINE_STATE_ADJUST_TILT_WARNING         (MACHINE_STATE_TEST_DONE-8)
#define MACHINE_STATE_ADJUST_AWARD_OVERRIDE       (MACHINE_STATE_TEST_DONE-9)
#define MACHINE_STATE_ADJUST_BALLS_OVERRIDE       (MACHINE_STATE_TEST_DONE-10)
#define MACHINE_STATE_ADJUST_SCROLLING_SCORES     (MACHINE_STATE_TEST_DONE-11)
#define MACHINE_STATE_ADJUST_EXTRA_BALL_AWARD     (MACHINE_STATE_TEST_DONE-12)
#define MACHINE_STATE_ADJUST_SPECIAL_AWARD        (MACHINE_STATE_TEST_DONE-13)
#define MACHINE_STATE_ADJUST_IDLE_MODE            (MACHINE_STATE_TEST_DONE-14)
#define MACHINE_STATE_ADJUST_ALLOW_RESET          (MACHINE_STATE_TEST_DONE-15)
#define MACHINE_STATE_ADJUST_DONE                 (MACHINE_STATE_TEST_DONE-16)

#define GAME_MODE_NONE                              0
#define GAME_MODE_SKILL_SHOT                        1
#define GAME_MODE_UNSTRUCTURED_PLAY                 2
#define GAME_MODE_BALL_IN_SHOOTER_LANE              3
#define GAME_MODE_OFFER_SINGLE_COMBAT               5
#define GAME_MODE_SINGLE_COMBAT_START               6
#define GAME_MODE_SINGLE_COMBAT                     7
#define GAME_MODE_SINGLE_COMBAT_WON                 8
#define GAME_MODE_SINGLE_COMBAT_LOST                9
#define GAME_MODE_OFFER_DOUBLE_COMBAT               10
#define GAME_MODE_DOUBLE_COMBAT_START               11
#define GAME_MODE_DOUBLE_COMBAT                     12
#define GAME_MODE_DOUBLE_COMBAT_OVER                13
#define GAME_MODE_DOUBLE_COMBAT_FIRST_WIN           14
#define GAME_MODE_DOUBLE_COMBAT_LOST                15
#define GAME_MODE_DOUBLE_COMBAT_LEVEL_INCREASED     16
#define GAME_MODE_DOUBLE_COMBAT_LEVEL_SAME          17
#define GAME_MODE_TRIPLE_COMBAT_START               18
#define GAME_MODE_TRIPLE_COMBAT                     19
#define GAME_MODE_TRIPLE_COMBAT_OVER                20
#define GAME_MODE_TRIPLE_COMBAT_FIRST_WIN           21
#define GAME_MODE_TRIPLE_COMBAT_LOST                22
#define GAME_MODE_TRIPLE_COMBAT_LEVEL_INCREASED     23
#define GAME_MODE_TRIPLE_COMBAT_LEVEL_SAME          24
#define GAME_MODE_SEIGE                             30
#define GAME_MODE_KINGS_CHALLENGE_START             31
#define GAME_MODE_WAIT_FOR_BALL_TO_RETURN           60

#define EEPROM_BALL_SAVE_BYTE           100
#define EEPROM_FREE_PLAY_BYTE           101
#define EEPROM_SOUND_SELECTOR_BYTE      102
#define EEPROM_SKILL_SHOT_BYTE          103
#define EEPROM_TILT_WARNING_BYTE        104
#define EEPROM_AWARD_OVERRIDE_BYTE      105
#define EEPROM_BALLS_OVERRIDE_BYTE      106
#define EEPROM_TOURNAMENT_SCORING_BYTE  107
#define EEPROM_SFX_VOLUME_BYTE          108
#define EEPROM_MUSIC_VOLUME_BYTE        109
#define EEPROM_SCROLLING_SCORES_BYTE    110
#define EEPROM_CALLOUTS_VOLUME_BYTE     111
#define EEPROM_GOALS_UNTIL_WIZ_BYTE     112
#define EEPROM_IDLE_MODE_BYTE           113
#define EEPROM_WIZ_TIME_BYTE            114
#define EEPROM_SPINNER_ACCELERATOR_BYTE 115
#define EEPROM_COMBOS_GOAL_BYTE         116
#define EEPROM_ALLOW_RESET_BYTE         117
#define EEPROM_EXTRA_BALL_SCORE_UL      140
#define EEPROM_SPECIAL_SCORE_UL         144


#define SOUND_EFFECT_NONE                     0
#define SOUND_EFFECT_BONUS_COLLECT            2
#define SOUND_EFFECT_DROP_TARGET_HURRY        3
#define SOUND_EFFECT_OUTLANE_UNLIT            4
#define SOUND_EFFECT_LOCK_BOUNCED             5
#define SOUND_EFFECT_OUTLANE_LIT              6
#define SOUND_EFFECT_BUMPER_HIT               7
#define SOUND_EFFECT_DROP_TARGET_COMPLETE     8
#define SOUND_EFFECT_HOOFBEATS                9
#define SOUND_EFFECT_HORSE_NEIGHING           10
#define SOUND_EFFECT_HORSE_CHUFFING           11
#define SOUND_EFFECT_FANFARE_1                12
#define SOUND_EFFECT_FANFARE_2                13
#define SOUND_EFFECT_FANFARE_3                14
#define SOUND_EFFECT_SWORD_1                  15
#define SOUND_EFFECT_SWORD_2                  16
#define SOUND_EFFECT_SWORD_3                  17
#define SOUND_EFFECT_SWORD_4                  18
#define SOUND_EFFECT_SWORD_5                  19
#define SOUND_EFFECT_SWORD_6                  20
#define SOUND_EFFECT_SWORD_7                  21
//#define SOUND_EFFECT_STOP_BACKGROUND          19
//#define SOUND_EFFECT_DROP_TARGET_RESET        16
//#define SOUND_EFFECT_WEAPONS_SELECT_START     17
//#define SOUND_EFFECT_BATTLE_MULTIBALL_START   18
//#define SOUND_EFFECT_BALL_OVER                19
//#define SOUND_EFFECT_FRENZY_BUMPER_HIT        21
#define SOUND_EFFECT_DROP_TARGET_HIT_1        22
#define SOUND_EFFECT_DROP_TARGET_HIT_2        23
#define SOUND_EFFECT_DROP_TARGET_HIT_3        24
#define SOUND_EFFECT_SPINNER_LIT_2            25
#define SOUND_EFFECT_GAME_OVER                26
#define SOUND_EFFECT_TILT_WARNING             28
#define SOUND_EFFECT_MATCH_SPIN               30
#define SOUND_EFFECT_SLING_SHOT               34
#define SOUND_EFFECT_MAGNET                   35
#define SOUND_EFFECT_PORTCULLIS               36
#define SOUND_EFFECT_SPINNER_UNLIT            37
#define SOUND_EFFECT_SWOOSH                   38
#define SOUND_EFFECT_DOOR_SLAM                39
#define SOUND_EFFECT_COUNTDOWN_BONUS_START    40
#define SOUND_EFFECT_COUNTDOWN_BONUS_END      50
#define SOUND_EFFECT_CHURCH_BELL_1            51
#define SOUND_EFFECT_COIN_JINGLE_2            52
#define SOUND_EFFECT_COIN_JINGLE_3            53
#define SOUND_EFFECT_SNARE_FILL_1             54
#define SOUND_EFFECT_SNARE_FILL_2             55
#define SOUND_EFFECT_SNARE_FILL_3             56
#define SOUND_EFFECT_THREE_BELLS              57
#define SOUND_EFFECT_BOOING_1                 58
#define SOUND_EFFECT_BOOING_2                 59
#define SOUND_EFFECT_BOOING_3                 60
#define SOUND_EFFECT_10_SECONDS_LEFT          61
#define SOUND_EFFECT_CROWD_CHEERING           62
#define SOUND_EFFECT_THREE_DINGS              63
#define SOUND_EFFECT_THREE_BLOCKS             64
#define SOUND_EFFECT_BONUS_2X_AWARD           65
#define SOUND_EFFECT_BONUS_3X_AWARD           66
#define SOUND_EFFECT_BONUS_4X_AWARD           67
#define SOUND_EFFECT_BONUS_5X_AWARD           68
#define SOUND_EFFECT_BONUS_6X_AWARD           69
#define SOUND_EFFECT_BONUS_7X_AWARD           70
#define SOUND_EFFECT_BONUS_8X_AWARD           71
#define SOUND_EFFECT_BONUS_9X_AWARD           72
#define SOUND_EFFECT_THREE_ANVILS             73
#define SOUND_EFFECT_SMALL_EXPLOSION          74
#define SOUND_EFFECT_TILT                     75
#define SOUND_EFFECT_SCORE_TICK               76
#define SOUND_EFFECT_LAUGH                    77
#define SOUND_EFFECT_LEVITATE                 78
#define SOUND_EFFECT_SINGLE_ANVIL             79  
#define SOUND_EFFECT_CHURCH_BELL_2            80
#define SOUND_EFFECT_CHURCH_BELL_3            81
#define SOUND_EFFECT_CHURCH_BELL_4            82


#define SOUND_EFFECT_SPINNER_LIT              400
#define NUM_LIT_SPINNER_SOUNDS                4


#define SOUND_EFFECT_WAV_MANDATORY      100
#define SOUND_EFFECT_COIN_DROP_1        100
#define SOUND_EFFECT_COIN_DROP_2        101
#define SOUND_EFFECT_COIN_DROP_3        102
#define SOUND_EFFECT_WIZARD_START_SAUCER  110
#define SOUND_EFFECT_WIZARD_FINAL_SHOT_1  111
#define SOUND_EFFECT_WIZARD_FINAL_SHOT_2  112
#define SOUND_EFFECT_MACHINE_START      120

#define SOUND_EFFECT_SELF_TEST_MODE_START               132
#define SOUND_EFFECT_SELF_TEST_CPC_START                180
#define SOUND_EFFECT_SELF_TEST_AUDIO_OPTIONS_START      190
#define SOUND_EFFECT_SELF_TEST_CRB_OPTIONS_START        210

byte SelfTestStateToCalloutMap[] = {
  134, 135, 133, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, // <- SelfTestAndAudit modes
  // Starting Storm23 specific modes
  153, 154, 155, 156, 157, 158, // Freeplay through Callouts volume
  159, 160, 161, 162, 163, 164, 165, // through special award
  168, 171,  // Status Mode, Allow reset
  0
};


#define SOUND_EFFECT_VP_VOICE_NOTIFICATIONS_START                 300
#define SOUND_EFFECT_VP_BALL_MISSING                              300
#define SOUND_EFFECT_VP_PLAYER_ONE_UP                             301
#define SOUND_EFFECT_VP_PLAYER_TWO_UP                             302
#define SOUND_EFFECT_VP_PLAYER_THREE_UP                           303
#define SOUND_EFFECT_VP_PLAYER_FOUR_UP                            304
//#define SOUND_EFFECT_VP_JACKPOT                                   305
//#define SOUND_EFFECT_VP_SUPER_JACKPOT                             306
#define SOUND_EFFECT_VP_TIMERS_FROZEN                             307

#define SOUND_EFFECT_VP_ADD_PLAYER_1        308
#define SOUND_EFFECT_VP_ADD_PLAYER_2        (SOUND_EFFECT_VP_ADD_PLAYER_1+1)
#define SOUND_EFFECT_VP_ADD_PLAYER_3        (SOUND_EFFECT_VP_ADD_PLAYER_1+2)
#define SOUND_EFFECT_VP_ADD_PLAYER_4        (SOUND_EFFECT_VP_ADD_PLAYER_1+3)

#define SOUND_EFFECT_VP_SHOOT_AGAIN                               312
#define SOUND_EFFECT_VP_BALL_LOCKED                               313
#define SOUND_EFFECT_VP_EXTRA_BALL                                314
#define SOUND_EFFECT_VP_JACKPOT_READY                             315
#define SOUND_EFFECT_VP_LEVEL_1                                   316
#define SOUND_EFFECT_VP_LEVEL_2                                   317
#define SOUND_EFFECT_VP_LEVEL_3                                   318
#define SOUND_EFFECT_VP_LEVEL_4                                   319
#define SOUND_EFFECT_VP_LEVEL_5                                   320
#define SOUND_EFFECT_VP_LEVEL_MAX                                 321
#define SOUND_EFFECT_VP_BONUS_X_INCREASED                         322
#define SOUND_EFFECT_VP_DOUBLE_COMBAT_FIRST_VICTORY               323
#define SOUND_EFFECT_VP_DOUBLE_COMBAT_LOST                        324
#define SOUND_EFFECT_VP_DOUBLE_COMBAT_LEVEL_INCREASED             325
#define SOUND_EFFECT_VP_DOUBLE_COMBAT_LEVEL_SAME                  326
#define SOUND_EFFECT_VP_TRIPLE_COMBAT_FIRST_VICTORY               327
#define SOUND_EFFECT_VP_TRIPLE_COMBAT_LOST                        328
#define SOUND_EFFECT_VP_TRIPLE_COMBAT_LEVEL_INCREASED             329
#define SOUND_EFFECT_VP_TRIPLE_COMBAT_LEVEL_SAME                  330
#define SOUND_EFFECT_VP_RELIC_1                                   331
#define SOUND_EFFECT_VP_RELIC_2                                   332
#define SOUND_EFFECT_VP_RELIC_3                                   333
#define SOUND_EFFECT_VP_RELIC_4                                   334
#define SOUND_EFFECT_VP_RELIC_5                                   335
#define SOUND_EFFECT_VP_RELIC_6                                   336
#define SOUND_EFFECT_VP_RELIC_7                                   337
#define SOUND_EFFECT_VP_RELIC_8                                   338
#define SOUND_EFFECT_VP_RELIC_9                                   339
#define SOUND_EFFECT_VP_PRESS_FOR_SINGLE                          340
#define SOUND_EFFECT_VP_PRESS_FOR_DOUBLE                          341
#define SOUND_EFFECT_VP_SINGLE_COMBAT                             342
#define SOUND_EFFECT_VP_DOUBLE_COMBAT                             343
#define SOUND_EFFECT_VP_TRIPLE_COMBAT                             344
#define SOUND_EFFECT_VP_SINGLE_HINT_PART_3                        345
#define SOUND_EFFECT_VP_DOUBLE_HINT_PART_1                        346
#define SOUND_EFFECT_VP_JACKPOT                                   347
#define SOUND_EFFECT_VP_DOUBLE_JACKPOT                            348
#define SOUND_EFFECT_VP_TRIPLE_JACKPOT                            349
#define SOUND_EFFECT_VP_SUPER_JACKPOT                             350
#define SOUND_EFFECT_VP_MEGA_JACKPOT                              351
#define SOUND_EFFECT_VP_SINGLE_COMBAT_INSTRUCTIONS                352
#define SOUND_EFFECT_VP_DEATH_BLOW_INCREASED                      353
#define SOUND_EFFECT_VP_30_SECONDS                                354
#define SOUND_EFFECT_VP_45_SECONDS                                355
#define SOUND_EFFECT_VP_60_SECONDS                                356
#define SOUND_EFFECT_VP_75_SECONDS                                357
#define SOUND_EFFECT_VP_90_SECONDS                                358
#define SOUND_EFFECT_VP_120_SECONDS                               359
#define SOUND_EFFECT_VP_SINGLE_LOST                               360
#define SOUND_EFFECT_VP_DOUBLE_LOST                               361
#define SOUND_EFFECT_VP_TRIPLE_LOST                               362

#define SOUND_EFFECT_VP_SKILL_SHOT                                365
#define SOUND_EFFECT_VP_SUPER_SKILL_SHOT                          366
#define SOUND_EFFECT_VP_SKILL_SHOT_MISSED                         367
#define SOUND_EFFECT_VP_RETURN_TO_FIGHT                           368
#define SOUND_EFFECT_VP_SAUCER_FOR_DEATHBLOW                      369
#define SOUND_EFFECT_VP_PRESS_FOR_SINGLE_PART_2                   370
#define SOUND_EFFECT_VP_PRESS_FOR_SINGLE_PART_3                   371
#define SOUND_EFFECT_VP_SINGLE_HINT_PART_1                        372
#define SOUND_EFFECT_VP_SINGLE_HINT_PART_2                        373
#define SOUND_EFFECT_VP_SINGLE_TEN_SECONDS_TO_HIT_SAUCER          375
#define SOUND_EFFECT_VP_SINGLE_OPPONENT_RALLIES                   376
#define SOUND_EFFECT_VP_SINGLE_COMBAT_PART_1_COMPLETE             377
#define SOUND_EFFECT_VP_SINGLE_COMBAT_PART_2_COMPLETE             378
#define SOUND_EFFECT_VP_SINGLE_COMBAT_PART_3_COMPLETE             379
#define SOUND_EFFECT_VP_PRESS_FOR_DOUBLE_PART_2                   380
#define SOUND_EFFECT_VP_PRESS_FOR_DOUBLE_PART_3                   381
#define SOUND_EFFECT_VP_DOUBLE_HINT_2_OR_3                        382
#define SOUND_EFFECT_VP_BONUS_X_COLLECT_INSTRUCTIONS              383
#define SOUND_EFFECT_VP_KINGS_CHALLENGE_AVAILABLE                 384
#define SOUND_EFFECT_VP_KINGS_CHALLENGE_1                         385
#define SOUND_EFFECT_VP_KINGS_CHALLENGE_2                         386
#define SOUND_EFFECT_VP_KINGS_CHALLENGE_3                         387
#define SOUND_EFFECT_VP_KINGS_CHALLENGE_4                         388
#define SOUND_EFFECT_VP_KINGS_CHALLENGE_JOUST                     389
#define SOUND_EFFECT_VP_KINGS_CHALLENGE_PERFECTION                390
#define SOUND_EFFECT_VP_KINGS_CHALLENGE_LEVITATE                  391
#define SOUND_EFFECT_VP_KINGS_CHALLENGE_MELEE                     392
#define SOUND_EFFECT_VP_RETURN_TO_1X                              393
#define SOUND_EFFECT_VP_PLAYFIELD_2X                              394
#define SOUND_EFFECT_VP_PLAYFIELD_3X                              395
#define SOUND_EFFECT_VP_PLAYFIELD_4X                              396
#define SOUND_EFFECT_VP_PLAYFIELD_5X                              397
#define SOUND_EFFECT_VP_SINGLE_HINT_0                             398
#define SOUND_EFFECT_VP_DOUBLE_HINT_0                             399
#define SOUND_EFFECT_VP_TRIPLE_HINT_0                             400
#define SOUND_EFFECT_VP_TRIPLE_HINT_1                             401
#define SOUND_EFFECT_VP_TRIPLE_HINT_2                             402
#define SOUND_EFFECT_VP_TRIPLE_HINT_3                             403
#define SOUND_EFFECT_VP_TRIPLE_JACKPOTS_READY                     404




#define SOUND_EFFECT_RALLY_SONG_1         500
#define SOUND_EFFECT_RALLY_SONG_2         501
#define SOUND_EFFECT_RALLY_SONG_3         502
#define SOUND_EFFECT_RALLY_PLUNGE         520
#define SOUND_EFFECT_BACKGROUND_SONG_1    525
#define SOUND_EFFECT_BACKGROUND_SONG_2    526
#define SOUND_EFFECT_BACKGROUND_SONG_3    527
#define NUM_BACKGROUND_SONGS              11
#define SOUND_EFFECT_BATTLE_SONG_1        575
#define NUM_BATTLE_SONGS                  3


#define SOUND_EFFECT_DIAG_START                   1900
#define SOUND_EFFECT_DIAG_CREDIT_RESET_BUTTON     1900
#define SOUND_EFFECT_DIAG_SELECTOR_SWITCH_ON      1901
#define SOUND_EFFECT_DIAG_SELECTOR_SWITCH_OFF     1902
#define SOUND_EFFECT_DIAG_STARTING_ORIGINAL_CODE  1903
#define SOUND_EFFECT_DIAG_STARTING_NEW_CODE       1904
#define SOUND_EFFECT_DIAG_ORIGINAL_CPU_DETECTED   1905
#define SOUND_EFFECT_DIAG_ORIGINAL_CPU_RUNNING    1906
#define SOUND_EFFECT_DIAG_PROBLEM_PIA_U10         1907
#define SOUND_EFFECT_DIAG_PROBLEM_PIA_U11         1908
#define SOUND_EFFECT_DIAG_PROBLEM_PIA_1           1909
#define SOUND_EFFECT_DIAG_PROBLEM_PIA_2           1910
#define SOUND_EFFECT_DIAG_PROBLEM_PIA_3           1911
#define SOUND_EFFECT_DIAG_PROBLEM_PIA_4           1912
#define SOUND_EFFECT_DIAG_PROBLEM_PIA_5           1913
#define SOUND_EFFECT_DIAG_STARTING_DIAGNOSTICS    1914


#define MAX_DISPLAY_BONUS     109

#define TILT_WARNING_DEBOUNCE_TIME      1000


/*********************************************************************

    Machine state and options

*********************************************************************/
byte Credits = 0;
byte SoundSelector = 3;
byte BallSaveNumSeconds = 0;
byte MaximumCredits = 40;
byte BallsPerGame = 3;
byte ScoreAwardReplay = 0;
byte MusicVolume = 6;
byte SoundEffectsVolume = 8;
byte CalloutsVolume = 10;
byte ChuteCoinsInProgress[3];
byte TotalBallsLoaded = 3;
byte TimeRequiredToResetGame = 1;
byte NumberOfBallsInPlay = 0;
byte NumberOfBallsLocked = 0;
byte LampType = 0;
boolean FreePlayMode = false;
boolean HighScoreReplay = true;
boolean MatchFeature = true;
boolean TournamentScoring = false;
boolean ScrollingScores = true;
unsigned long ExtraBallValue = 0;
unsigned long SpecialValue = 0;
unsigned long CurrentTime = 0;
unsigned long HighScore = 0;
unsigned long AwardScores[3];
unsigned long CreditResetPressStarted = 0;
unsigned long SaucerEjectTime = 0;

AudioHandler Audio;



/*********************************************************************

    Game State

*********************************************************************/
byte CurrentPlayer = 0;
byte CurrentBallInPlay = 1;
byte CurrentNumPlayers = 0;
byte Bonus[4];
byte BonusX[4];
byte GameMode = GAME_MODE_SKILL_SHOT;
byte WaitingForBallGameMode = GAME_MODE_NONE;
byte MaxTiltWarnings = 2;
byte NumTiltWarnings = 0;
byte CurrentAchievements[4];

boolean SamePlayerShootsAgain = false;
boolean BallSaveUsed = false;
boolean ExtraBallCollected = false;
boolean SpecialCollected = false;
boolean TimersPaused = true;
boolean AllowResetAfterBallOne = true;

unsigned long CurrentScores[4];
unsigned long BallFirstSwitchHitTime = 0;
unsigned long BallTimeInTrough = 0;
unsigned long GameModeStartTime = 0;
unsigned long GameModeEndTime = 0;
unsigned long LastTiltWarningTime = 0;
unsigned long PlayfieldMultiplier;
unsigned long LastTimeThroughLoop;
unsigned long LastSwitchHitTime;
unsigned long BallSaveEndTime;

#define BALL_SAVE_GRACE_PERIOD  2000

/*********************************************************************

    Game Specific State Variables

*********************************************************************/
byte SpinsTowardsNextGoal[4];
byte SpinnerGoal[4];
byte IdleMode;
byte NumDropTargetHits[4][4];
byte NumDropTargetClears[4][4];
byte LastChanceStatus[4];
byte DropTargetLevel[4];
byte SkillShotTarget;
byte SpinnerStatus;
byte SpinnerPhase;
byte NumberOfComboDefinitions = 0;
byte SingleCombatNumSeconds = 60;
byte SingleCombatLevelCompleted[4];
byte DoubleCombatLevelCompleted[4];
byte TripleCombatLevelCompleted[4];
byte BallSaveOnCombatModes = 20;
byte NumBonusXCollectReminders;
byte TripleCombatJackpotsAvailable;
#define TRIPLE_COMBAT_ALL_JACKPOTS            0x3F
#define TRIPLE_COMBAT_SPINNER_JACKPOT         0x01
#define TRIPLE_COMBAT_MIDDLE_RAMP_JACKPOT     0x02
#define TRIPLE_COMBAT_LOOP_JACKPOT            0x04
#define TRIPLE_COMBAT_SAUCER_JACKPOT          0x08
#define TRIPLE_COMBAT_LOCK_JACKPOT            0x10
#define TRIPLE_COMBAT_UPPER_RAMP_JACKPOT      0x20

byte CombatBankFlags;
#define DROP_BANK_UL_FLAG   0x01
#define DROP_BANK_UR_FLAG   0x02
#define DROP_BANK_LL_FLAG   0x04
#define DROP_BANK_LR_FLAG   0x08

#define LAST_CHANCE_LEFT_QUALIFIED    0x01
#define LAST_CHANCE_RIGHT_QUALIFIED   0x02

/*
#define NUM_BALL_SEARCH_SOLENOIDS   3
byte BallSearchSolenoidToTry;
byte BallSearchSols[NUM_BALL_SEARCH_SOLENOIDS] = {SOL_POP_BUMPER, SOL_RIGHT_SLING, SOL_LEFT_SLING};
#define BALL_SEARCH_POP_INDEX 0
#define BALL_SEARCH_LEFT_SLING_INDEX  1
#define BALL_SEARCH_RIGHT_SLING_INDEX 2
*/
// Machine locks is a curated variable that keeps debounced
// knowledge of how many balls are trapped in different mechanisms.
// It's initialized the first time that attract mode is run,
// and then it's maintained by "UpdateLockStatus", which only
// registers change when a state is changed for a certain
// amount of time.
byte MachineLocks;
byte PlayerLockStatus[4];

#define LOCK_1_ENGAGED        0x10
#define LOCK_2_ENGAGED        0x20
#define LOCK_3_ENGAGED        0x40
#define LOCKS_ENGAGED_MASK    0x70
#define LOCK_1_AVAILABLE      0x01
#define LOCK_2_AVAILABLE      0x02
#define LOCK_3_AVAILABLE      0x04
#define LOCKS_AVAILABLE_MASK  0x07

byte ExtraBallsOrSpecialAvailable[4];
#define EBS_LOWER_EXTRA_BALL_AVAILABLE      0x01
#define EBS_LOWER_SPECIAL_AVAILABLE         0x02
#define EBS_UPPER_EXTRA_BALL_AVAILABLE      0x04

byte KingsChallengeKick; // 1 = top lock, 2 = saucer
byte KingsChallengeStatus[4];
byte KingsChallengePerfectionBank;
byte KingsChallengeRunning;
#define KINGS_CHALLENGE_1_QUALIFIED         0x01
#define KINGS_CHALLENGE_2_QUALIFIED         0x02
#define KINGS_CHALLENGE_3_QUALIFIED         0x04
#define KINGS_CHALLENGE_4_QUALIFIED         0x08
#define KINGS_CHALLENGE_AVAILABLE           0x0F
#define KINGS_CHALLENGE_DROPS               0x07
#define KINGS_CHALLENGE_1_COMPLETE          0x10
#define KINGS_CHALLENGE_2_COMPLETE          0x20
#define KINGS_CHALLENGE_3_COMPLETE          0x40
#define KINGS_CHALLENGE_4_COMPLETE          0x80
#define KINGS_CHALLENGE_JOUST               KINGS_CHALLENGE_1_QUALIFIED
#define KINGS_CHALLENGE_PERFECTION          KINGS_CHALLENGE_2_QUALIFIED
#define KINGS_CHALLENGE_LEVITATE            KINGS_CHALLENGE_3_QUALIFIED
#define KINGS_CHALLENGE_MELEE               KINGS_CHALLENGE_4_QUALIFIED


boolean IdleModeEnabled = true;
boolean OutlaneSpecialLit[4];
boolean DropTargetHurryLamp[4];
boolean CombatJackpotReady;
boolean CombatSuperJackpotReady;
boolean LoopLitToQualifyLock;
boolean MagnaSaveAvailable;
boolean MagnaSoundOn;
boolean LockManagementInProgress;
boolean BonusXCollectAvailable;

unsigned long BonusXCollectAvailableStart;
unsigned long BonusXCollectReminder;
unsigned long BonusXAnimationStart;
unsigned long LastSpinnerHit;
unsigned long LastPopBumperHit;
unsigned long SpinnerLitUntil;
unsigned long KingsChallengeEndTime;
unsigned long KingsChallengeBonus;
unsigned long KingsChallengeBonusChangedTime;
unsigned long LevitateMagnetOnTime;
unsigned long LevitateMagnetOffTime;
unsigned long LastTimeLeftMagnetOn;
unsigned long LastTimeRightMagnetOn;

unsigned long PlayfieldMultiplierExpiration;
//unsigned long BallSearchNextSolenoidTime;
//unsigned long BallSearchSolenoidFireTime[NUM_BALL_SEARCH_SOLENOIDS];
unsigned long TicksCountedTowardsStatus;
unsigned long BallRampKicked;
unsigned long DropTargetResetTime[4];
unsigned long DropTargetHurryTime[4];
unsigned long MagnaStatusLeft[4];
unsigned long MagnaStatusRight[4];
unsigned long LockKickTime[3];
unsigned long SaucerKickTime;
unsigned long ExtraBallAwardStartTime;
unsigned long LastLoopHitTime;

// Combo tracking variables
unsigned long LastLeftInlane;
unsigned long LastRightInlane;
unsigned long CombosAchieved[4];

unsigned long JackpotIncreasedTime;
unsigned long CombatJackpot[4];
unsigned long JackpotBeforeCombat;
#define COMBAT_JACKPOT_BASE_1     100000
#define COMBAT_JACKPOT_BASE_2     125000
#define COMBAT_JACKPOT_BASE_3     150000
#define COMBAT_JACKPOT_STEP       25000
unsigned long BonusAnimationStart;

DropTargetBank DropTargetsUL(3, 1, DROP_TARGET_TYPE_WLLMS_2, 50);
DropTargetBank DropTargetsUR(3, 1, DROP_TARGET_TYPE_WLLMS_2, 50);
DropTargetBank DropTargetsLL(3, 1, DROP_TARGET_TYPE_WLLMS_2, 50);
DropTargetBank DropTargetsLR(3, 1, DROP_TARGET_TYPE_WLLMS_2, 50);

#define IDLE_MODE_NONE                  0
#define IDLE_MODE_BALL_SEARCH           9

#define SKILL_SHOT_AWARD          25000
#define SUPER_SKILL_SHOT_AWARD    50000

#define POP_BUMPER_DEBOUNCE_TIME    200

/******************************************************

   Adjustments Serialization

*/


void ReadStoredParameters() {
  for (byte count = 0; count < 3; count++) {
    ChuteCoinsInProgress[count] = 0;
  }

  HighScore = RPU_ReadULFromEEProm(RPU_HIGHSCORE_EEPROM_START_BYTE, 10000);
  Credits = RPU_ReadByteFromEEProm(RPU_CREDITS_EEPROM_BYTE);
  if (Credits > MaximumCredits) Credits = MaximumCredits;

  ReadSetting(EEPROM_FREE_PLAY_BYTE, 0);
  FreePlayMode = (EEPROM.read(EEPROM_FREE_PLAY_BYTE)) ? true : false;

  BallSaveNumSeconds = ReadSetting(EEPROM_BALL_SAVE_BYTE, 15);
  if (BallSaveNumSeconds > 20) BallSaveNumSeconds = 20;

  SoundSelector = ReadSetting(EEPROM_SOUND_SELECTOR_BYTE, 3);
  if (SoundSelector > 8) SoundSelector = 3;

  MusicVolume = ReadSetting(EEPROM_MUSIC_VOLUME_BYTE, 10);
  if (MusicVolume > 10) MusicVolume = 10;

  SoundEffectsVolume = ReadSetting(EEPROM_SFX_VOLUME_BYTE, 10);
  if (SoundEffectsVolume > 10) SoundEffectsVolume = 10;

  CalloutsVolume = ReadSetting(EEPROM_CALLOUTS_VOLUME_BYTE, 10);
  if (CalloutsVolume > 10) CalloutsVolume = 10;

  Audio.SetMusicVolume(MusicVolume);
  Audio.SetSoundFXVolume(SoundEffectsVolume);
  Audio.SetNotificationsVolume(CalloutsVolume);

  TournamentScoring = (ReadSetting(EEPROM_TOURNAMENT_SCORING_BYTE, 0)) ? true : false;

  MaxTiltWarnings = ReadSetting(EEPROM_TILT_WARNING_BYTE, 2);
  if (MaxTiltWarnings > 2) MaxTiltWarnings = 2;

  AllowResetAfterBallOne = (ReadSetting(EEPROM_ALLOW_RESET_BYTE, 1)) ? true : false;

  byte awardOverride = ReadSetting(EEPROM_AWARD_OVERRIDE_BYTE, 99);
  if (awardOverride != 99) {
    ScoreAwardReplay = awardOverride;
  }

  byte ballsOverride = ReadSetting(EEPROM_BALLS_OVERRIDE_BYTE, 99);
  if (ballsOverride == 3 || ballsOverride == 5) {
    BallsPerGame = ballsOverride;
  } else {
    if (ballsOverride != 99) EEPROM.write(EEPROM_BALLS_OVERRIDE_BYTE, 99);
  }

  ScrollingScores = (ReadSetting(EEPROM_SCROLLING_SCORES_BYTE, 1)) ? true : false;

  ExtraBallValue = RPU_ReadULFromEEProm(EEPROM_EXTRA_BALL_SCORE_UL);
  if (ExtraBallValue % 1000 || ExtraBallValue > 100000) ExtraBallValue = 20000;

  SpecialValue = RPU_ReadULFromEEProm(EEPROM_SPECIAL_SCORE_UL);
  if (SpecialValue % 1000 || SpecialValue > 100000) SpecialValue = 40000;

  AwardScores[0] = RPU_ReadULFromEEProm(RPU_AWARD_SCORE_1_EEPROM_START_BYTE);
  AwardScores[1] = RPU_ReadULFromEEProm(RPU_AWARD_SCORE_2_EEPROM_START_BYTE);
  AwardScores[2] = RPU_ReadULFromEEProm(RPU_AWARD_SCORE_3_EEPROM_START_BYTE);

}


void MoveBallFromOutholeToRamp(boolean sawSwitch = false) {
  if (RPU_ReadSingleSwitchState(SW_OUTHOLE) || sawSwitch) {
    if (CurrentTime == 0 || CurrentTime > (BallRampKicked + 1000)) {
      RPU_PushToSolenoidStack(SOL_OUTHOLE, 16, true);
      if (CurrentTime) BallRampKicked = CurrentTime;
      else BallRampKicked = millis();
    }
  }

}


void QueueDIAGNotification(unsigned short notificationNum) {
  // This is optional, but the machine can play an audio message at boot
  // time to indicate any errors and whether it's going to boot to original
  // or new code.
  Audio.QueuePrioritizedNotification(notificationNum, 0, 10, CurrentTime);
  if (DEBUG_MESSAGES) {
    char buf[128];
    sprintf(buf, "Diag = %d\n", notificationNum);
    Serial.write(buf);
  }
}


void setup() {

  if (DEBUG_MESSAGES) {
    // If debug is on, set up the Serial port for communication
    Serial.begin(115200);
    Serial.write("Starting\n");
  }

  // Set up the Audio handler in order to play boot messages
  CurrentTime = millis();
  if (DEBUG_MESSAGES) Serial.write("Staring Audio\n");
  Audio.InitDevices(AUDIO_PLAY_TYPE_WAV_TRIGGER | AUDIO_PLAY_TYPE_ORIGINAL_SOUNDS);
  Audio.StopAllAudio();

  // Set up the chips and interrupts
  unsigned long initResult = 0;
  if (DEBUG_MESSAGES) Serial.write("Initializing MPU\n");
  initResult = RPU_InitializeMPU(RPU_CMD_BOOT_ORIGINAL_IF_CREDIT_RESET | RPU_CMD_INIT_AND_RETURN_EVEN_IF_ORIGINAL_CHOSEN | RPU_CMD_PERFORM_MPU_TEST, SW_CREDIT_RESET);

  if (DEBUG_MESSAGES) {
    char buf[128];
    sprintf(buf, "Return from init = 0x%04lX\n", initResult);
    Serial.write(buf);
    if (initResult & RPU_RET_6800_DETECTED) Serial.write("Detected 6800 clock\n");
    else if (initResult & RPU_RET_6802_OR_8_DETECTED) Serial.write("Detected 6802/8 clock\n");
    Serial.write("Back from init\n");
  }

  if (initResult & RPU_RET_SELECTOR_SWITCH_ON) QueueDIAGNotification(SOUND_EFFECT_DIAG_SELECTOR_SWITCH_ON);
  else QueueDIAGNotification(SOUND_EFFECT_DIAG_SELECTOR_SWITCH_OFF);
  if (initResult & RPU_RET_CREDIT_RESET_BUTTON_HIT) QueueDIAGNotification(SOUND_EFFECT_DIAG_CREDIT_RESET_BUTTON);

  if (initResult & RPU_RET_DIAGNOSTIC_REQUESTED) {
    QueueDIAGNotification(SOUND_EFFECT_DIAG_STARTING_DIAGNOSTICS);
    // Run diagnostics here:
  }

  if (initResult & RPU_RET_ORIGINAL_CODE_REQUESTED) {
    delay(100);
    QueueDIAGNotification(SOUND_EFFECT_DIAG_STARTING_ORIGINAL_CODE);
    while (Audio.Update(millis()));
    // Arduino should hang if original code is running
    while (1);
  }
  QueueDIAGNotification(SOUND_EFFECT_DIAG_STARTING_NEW_CODE);
  RPU_DisableSolenoidStack();
  RPU_SetDisableFlippers(true);

  // Read parameters from EEProm
  ReadStoredParameters();
  RPU_SetCoinLockout((Credits >= MaximumCredits) ? true : false);

  CurrentScores[0] = BK_MAJOR_VERSION;
  CurrentScores[1] = BK_MINOR_VERSION;
  CurrentScores[2] = RPU_OS_MAJOR_VERSION;
  CurrentScores[3] = RPU_OS_MINOR_VERSION;

  CurrentAchievements[0] = 0;
  CurrentAchievements[1] = 0;
  CurrentAchievements[2] = 0;
  CurrentAchievements[3] = 0;

  DropTargetsUL.DefineSwitch(0, SW_UL_DROP_1);
  DropTargetsUL.DefineSwitch(1, SW_UL_DROP_2);
  DropTargetsUL.DefineSwitch(2, SW_UL_DROP_3);
  DropTargetsUL.DefineResetSolenoid(0, SOL_UL_DROP_RESET);

  DropTargetsUR.DefineSwitch(0, SW_UR_DROP_1);
  DropTargetsUR.DefineSwitch(1, SW_UR_DROP_2);
  DropTargetsUR.DefineSwitch(2, SW_UR_DROP_3);
  DropTargetsUR.DefineResetSolenoid(0, SOL_UR_DROP_RESET);

  DropTargetsLL.DefineSwitch(0, SW_LL_DROP_1);
  DropTargetsLL.DefineSwitch(1, SW_LL_DROP_2);
  DropTargetsLL.DefineSwitch(2, SW_LL_DROP_3);
  DropTargetsLL.DefineResetSolenoid(0, SOL_LL_DROP_RESET);

  DropTargetsLR.DefineSwitch(0, SW_LR_DROP_1);
  DropTargetsLR.DefineSwitch(1, SW_LR_DROP_2);
  DropTargetsLR.DefineSwitch(2, SW_LR_DROP_3);
  DropTargetsLR.DefineResetSolenoid(0, SOL_LR_DROP_RESET);

  NumberOfComboDefinitions = InitBKCombosArray();

  //Audio.InitDevices(AUDIO_PLAY_TYPE_WAV_TRIGGER | AUDIO_PLAY_TYPE_ORIGINAL_SOUNDS);
  //Audio.StopAllAudio();
  //delay(10);
  Audio.SetMusicDuckingGain(30);
  Audio.SetSoundFXDuckingGain(30);
  //Audio.QueueSound(SOUND_EFFECT_HORSE_NEIGHING, AUDIO_PLAY_TYPE_WAV_TRIGGER, CurrentTime+1200);

  PlaySoundEffect(1);
}

byte ReadSetting(byte setting, byte defaultValue) {
  byte value = EEPROM.read(setting);
  if (value == 0xFF) {
    EEPROM.write(setting, defaultValue);
    return defaultValue;
  }
  return value;
}


// This function is useful for checking the status of drop target switches
byte CheckSequentialSwitches(byte startingSwitch, byte numSwitches) {
  byte returnSwitches = 0;
  for (byte count = 0; count < numSwitches; count++) {
    returnSwitches |= (RPU_ReadSingleSwitchState(startingSwitch + count) << count);
  }
  return returnSwitches;
}


////////////////////////////////////////////////////////////////////////////
//
//  Lamp Management functions
//
////////////////////////////////////////////////////////////////////////////

void ShowLockLamps() {

  if (GameMode == GAME_MODE_SKILL_SHOT) {
    byte lampPhase = ((CurrentTime - GameModeStartTime) / 500) % 3;
    RPU_SetLampState(LAMP_LOCK_1, lampPhase == 2);
    RPU_SetLampState(LAMP_LOCK_2, lampPhase == 1);
    RPU_SetLampState(LAMP_LOCK_3, lampPhase == 0);
  } else if (TripleCombatJackpotsAvailable & TRIPLE_COMBAT_LOCK_JACKPOT) {
    RPU_SetLampState(LAMP_LOCK_1, 1, 0, 75);
    RPU_SetLampState(LAMP_LOCK_2, 1, 0, 75);
    RPU_SetLampState(LAMP_LOCK_3, 1, 0, 75);
  } else if (LockLitFromCombo) {
    byte lampPhase = (CurrentTime / 200) % 3;
    RPU_SetLampState(LAMP_LOCK_1, lampPhase == 0);
    RPU_SetLampState(LAMP_LOCK_2, lampPhase == 0);
    RPU_SetLampState(LAMP_LOCK_3, lampPhase == 0);
  } else if (KingsChallengeStatus[CurrentPlayer] & KINGS_CHALLENGE_AVAILABLE) {
    byte lampPhase = (CurrentTime/100)%8;
    RPU_SetLampState(LAMP_LOCK_1, lampPhase==0);
    RPU_SetLampState(LAMP_LOCK_2, lampPhase==0);
    RPU_SetLampState(LAMP_LOCK_3, lampPhase==0);
  } else if (GameMode == GAME_MODE_DOUBLE_COMBAT && CombatJackpotReady) {
    RPU_SetLampState(LAMP_LOCK_1, 1, 0, 230);
    RPU_SetLampState(LAMP_LOCK_2, 1, 0, 230);
    RPU_SetLampState(LAMP_LOCK_3, 1, 0, 230);
  } else if (GameMode == GAME_MODE_TRIPLE_COMBAT && CombatJackpotReady) {
    RPU_SetLampState(LAMP_LOCK_1, 1, 0, 130);
    RPU_SetLampState(LAMP_LOCK_2, 1, 0, 130);
    RPU_SetLampState(LAMP_LOCK_3, 1, 0, 130);
  } else if (GameMode == GAME_MODE_UNSTRUCTURED_PLAY) {
    RPU_SetLampState(LAMP_LOCK_1, PlayerLockStatus[CurrentPlayer] & (LOCK_1_ENGAGED | LOCK_1_AVAILABLE), 0, (PlayerLockStatus[CurrentPlayer]&LOCK_1_AVAILABLE) ? 200 : 0);
    RPU_SetLampState(LAMP_LOCK_2, PlayerLockStatus[CurrentPlayer] & (LOCK_2_ENGAGED | LOCK_2_AVAILABLE), 0, (PlayerLockStatus[CurrentPlayer]&LOCK_2_AVAILABLE) ? 200 : 0);
    RPU_SetLampState(LAMP_LOCK_3, PlayerLockStatus[CurrentPlayer] & (LOCK_3_ENGAGED | LOCK_3_AVAILABLE), 0, (PlayerLockStatus[CurrentPlayer]&LOCK_3_AVAILABLE) ? 200 : 0);
  } else {
    RPU_SetLampState(LAMP_LOCK_1, 0);
    RPU_SetLampState(LAMP_LOCK_2, 0);
    RPU_SetLampState(LAMP_LOCK_3, 0);
  }

}


void ShowBonusLamps() {
  if (GameMode == GAME_MODE_SKILL_SHOT) {
    for (byte count = LAMP_BONUS_1; count <=  LAMP_BONUS_40; count++) RPU_SetLampState(count, 0);
  } else {
    byte bonusToShow = Bonus[CurrentPlayer];
    if (BonusAnimationStart) {
      bonusToShow = 1 + ((CurrentTime - BonusAnimationStart) / 25);
      if (bonusToShow >= Bonus[CurrentPlayer]) {
        BonusAnimationStart = 0;
        bonusToShow = Bonus[CurrentPlayer];
      }
    }

    RPU_SetLampState(LAMP_BONUS_40, (bonusToShow >= 40));
    if (bonusToShow >= 40) bonusToShow -= 40;
    RPU_SetLampState(LAMP_BONUS_30, (bonusToShow >= 30));
    if (bonusToShow >= 30) bonusToShow -= 30;
    RPU_SetLampState(LAMP_BONUS_20, (bonusToShow >= 20));
    if (bonusToShow >= 20) bonusToShow -= 20;
    RPU_SetLampState(LAMP_BONUS_10, (bonusToShow >= 10));
    if (bonusToShow >= 10) bonusToShow -= 10;

    byte effectiveOnes = bonusToShow % 10;
    if (bonusToShow != 0 && effectiveOnes == 0) effectiveOnes = 10;
    for (byte count = 1; count < 10; count++) RPU_SetLampState(LAMP_BONUS_1 + (count - 1), effectiveOnes >= count);
  }
}

void ShowBonusXLamps() {

  if (BonusXCollectAvailableStart) {

    if (CurrentTime > (BonusXCollectAvailableStart + 1500)) {
      BonusXCollectAvailableStart = 0;
    }
    boolean lampDim = (((CurrentTime - BonusXCollectAvailableStart) % 500) / 166) ? true : false;
    boolean lampOn = (((CurrentTime - BonusXCollectAvailableStart) % 500) / 333) ? false : true;

    RPU_SetLampState(LAMP_2X, lampOn && (BonusX[CurrentPlayer] == 1 || BonusX[CurrentPlayer] == 5 || BonusX[CurrentPlayer] == 6), lampDim);
    RPU_SetLampState(LAMP_3X, lampOn && (BonusX[CurrentPlayer] == 2 || BonusX[CurrentPlayer] == 7), lampDim);
    RPU_SetLampState(LAMP_4X, lampOn && (BonusX[CurrentPlayer] == 3 || BonusX[CurrentPlayer] == 5 || BonusX[CurrentPlayer] == 8), lampDim);
    RPU_SetLampState(LAMP_5X, lampOn && (BonusX[CurrentPlayer] == 4 || BonusX[CurrentPlayer] >= 6), lampDim);
  } else {

    int flashVal = 0;

    if (BonusXAnimationStart) {
      if (CurrentTime > (BonusXAnimationStart + 3000)) BonusXAnimationStart = 0;
      flashVal = 150;
    }

    if (flashVal == 0 && BonusX[CurrentPlayer] > 5) flashVal = 400;

    RPU_SetLampState(LAMP_2X, BonusX[CurrentPlayer] == 2 || BonusX[CurrentPlayer] == 6 || BonusX[CurrentPlayer] == 7, 0, flashVal);
    RPU_SetLampState(LAMP_3X, BonusX[CurrentPlayer] == 3 || BonusX[CurrentPlayer] == 8, 0, flashVal);
    RPU_SetLampState(LAMP_4X, BonusX[CurrentPlayer] == 4 || BonusX[CurrentPlayer] == 6 || BonusX[CurrentPlayer] == 9, 0, flashVal);
    RPU_SetLampState(LAMP_5X, BonusX[CurrentPlayer] == 5 || BonusX[CurrentPlayer] >= 7, 0, flashVal);
  }
}


void ShowPlayfieldXAndMagnetLamps() {

  if (KingsChallengeRunning & KINGS_CHALLENGE_LEVITATE) {
    RPU_SetLampState(LAMP_LEFT_MAGNASAVE, LevitateMagnetOffTime ? 1 : 0, 0, 75);
    RPU_SetLampState(LAMP_RIGHT_MAGNASAVE, 0);
  } else if (!MagnaSaveAvailable) {
    RPU_SetLampState(LAMP_LEFT_MAGNASAVE, 0);
    RPU_SetLampState(LAMP_RIGHT_MAGNASAVE, 0);
  } else {
    int leftFlash, rightFlash;
    if (MagnaStatusLeft[CurrentPlayer] <= 1000) leftFlash = 0;
    else leftFlash = 750 - MagnaStatusLeft[CurrentPlayer] / 10;
    if (MagnaStatusRight[CurrentPlayer] <= 1000) rightFlash = 0;
    else rightFlash = 750 - MagnaStatusRight[CurrentPlayer] / 10;
    RPU_SetLampState(LAMP_LEFT_MAGNASAVE, MagnaStatusLeft[CurrentPlayer], 0, leftFlash);
    RPU_SetLampState(LAMP_RIGHT_MAGNASAVE, MagnaStatusRight[CurrentPlayer], 0, rightFlash);
  }

  if (PlayfieldMultiplier == 1) {
    RPU_SetLampState(LAMP_DOUBLE_SCORING, 0);
    RPU_SetLampState(LAMP_TRIPLE_SCORING, 0);
  } else {
    RPU_SetLampState(LAMP_DOUBLE_SCORING, PlayfieldMultiplier == 2 || PlayfieldMultiplier > 3, 0, (PlayfieldMultiplier == 4) ? 250 : 0);
    RPU_SetLampState(LAMP_TRIPLE_SCORING, PlayfieldMultiplier == 3 || PlayfieldMultiplier == 5);
  }
}


void ShowSpinnerAndPopBumperLamp() {
  //LAMP_SPINNER
  //LAMP_POP_BUMPER

  if (TripleCombatJackpotsAvailable & TRIPLE_COMBAT_SPINNER_JACKPOT) {
    RPU_SetLampState(LAMP_SPINNER, 1, 0, 75);
  } else {
    RPU_SetLampState(LAMP_SPINNER, SpinnerStatus, 0, (CurrentTime > (SpinnerLitUntil - 1000)) ? 100 : 500);
  }

  if (KingsChallengeRunning & KINGS_CHALLENGE_MELEE) {
    RPU_SetLampState(LAMP_POP_BUMPER, 1, 0, LastPopBumperHit ? 75 : 500);
  } else {
    RPU_SetLampState(LAMP_POP_BUMPER, 1, 0, LastPopBumperHit ? 75 : 0);
  }
}


void ShowHeadLamps() {
  //RPU_SetLampState(LAMP_HEAD_TIMER_BONUS_BALL
  //RPU_SetLampState(LAMP_HEAD_BALL_IN_PLAY
}


void ShowLaneAndRolloverLamps() {

  boolean ballReady = ((CurrentBallInPlay==BallsPerGame && (PlayerLockStatus[CurrentPlayer]&LOCKS_ENGAGED_MASK)) ? true : false);

  RPU_SetLampState(LAMP_LEFT_OUTLANE, ballReady && ((LastChanceStatus[CurrentPlayer]&LAST_CHANCE_LEFT_QUALIFIED) ? true : false), 0, (PlayerLockStatus[CurrentPlayer]&LOCKS_ENGAGED_MASK) ? 0 : 100);
  RPU_SetLampState(LAMP_RIGHT_OUTLANE, ballReady && ((LastChanceStatus[CurrentPlayer]&LAST_CHANCE_RIGHT_QUALIFIED) ? true : false), 0, (PlayerLockStatus[CurrentPlayer]&LOCKS_ENGAGED_MASK) ? 0 : 100);

  if (TripleCombatJackpotsAvailable & TRIPLE_COMBAT_MIDDLE_RAMP_JACKPOT) {
    RPU_SetLampState(LAMP_MIDDLE_RAMP, 1, 0, 75);
  } else {
    RPU_SetLampState(LAMP_MIDDLE_RAMP, RightRampLitFromCombo, 0, 50);
  }
  RPU_SetLampState(LAMP_RIGHT_INLANE, LastTimeRightMagnetOn ? true : false, 0, 75);

  if (LeftInlaneLitFromLoop) RPU_SetLampState(LAMP_LEFT_INLANE, 1, 0, 125);
  else RPU_SetLampState(LAMP_LEFT_INLANE, LastTimeLeftMagnetOn ? true : false, 0, 75);

  // Saucer Lamp

  if (TripleCombatJackpotsAvailable & TRIPLE_COMBAT_LOOP_JACKPOT) {
    RPU_SetLampState(LAMP_SAUCER, 1, 0, 75);
  } else if (BonusXCollectAvailableStart) {
    boolean lampDim = (((CurrentTime - BonusXCollectAvailableStart) % 500) / 166) ? true : false;
    boolean lampOn = (((CurrentTime - BonusXCollectAvailableStart) % 500) / 333) ? false : true;
    RPU_SetLampState(LAMP_SAUCER, lampOn, lampDim);
  } else if (GameMode == GAME_MODE_SINGLE_COMBAT && CombatJackpotReady) {
    RPU_SetLampState(LAMP_SAUCER, 1, 0, 50);
  } else if (SaucerLitFromCombo) {
    byte lampPhase = ((CurrentTime / 125) % 2);
    RPU_SetLampState(LAMP_SAUCER, lampPhase == 0);
  } else if (KingsChallengeStatus[CurrentPlayer] & KINGS_CHALLENGE_AVAILABLE) {
    byte lampPhase = (CurrentTime/100)%8;
    RPU_SetLampState(LAMP_SAUCER, lampPhase == 0);
  } else {
    RPU_SetLampState(LAMP_SAUCER, 0);
  }

  if (TripleCombatJackpotsAvailable & TRIPLE_COMBAT_LOOP_JACKPOT) {
    RPU_SetLampState(LAMP_LOOP_SPECIAL, 1, 0, 75);
    RPU_SetLampState(LAMP_LOOP_EXTRA_BALL, 1, 0, 75);
  } else if (LoopLitFromCombo) {
    RPU_SetLampState(LAMP_LOOP_SPECIAL, 1, 0, 75);
    RPU_SetLampState(LAMP_LOOP_EXTRA_BALL, 1, 0, 75);
  } else if (LoopLitToQualifyLock) {
    byte lampPhase = (CurrentTime / 150) % 2;
    RPU_SetLampState(LAMP_LOOP_SPECIAL, lampPhase == 0);
    RPU_SetLampState(LAMP_LOOP_EXTRA_BALL, lampPhase == 1);
  } else {
    RPU_SetLampState(LAMP_LOOP_SPECIAL, 0);
    RPU_SetLampState(LAMP_LOOP_EXTRA_BALL, 0);
  }

  if (TripleCombatJackpotsAvailable & TRIPLE_COMBAT_UPPER_RAMP_JACKPOT) {
    RPU_SetLampState(LAMP_UPPER_EXTRA_BALL, 1, 0, 75);
  } else {
    RPU_SetLampState(LAMP_UPPER_EXTRA_BALL, ExtraBallCollected==false && (ExtraBallsOrSpecialAvailable[CurrentPlayer]&EBS_UPPER_EXTRA_BALL_AVAILABLE), 0, 100);
  }

}


byte DropLamps[4] = {LAMP_UPPER_LEFT_DROPS, LAMP_UPPER_RIGHT_DROPS, LAMP_LOWER_LEFT_DROPS, LAMP_LOWER_RIGHT_DROPS};

void ShowDropTargetLamps() {

  byte dropPhase = (CurrentTime/1000)%2;

  if ((KingsChallengeRunning&KINGS_CHALLENGE_DROPS) && dropPhase) {
    byte dropState[4][3];
    for (byte bank=0; bank<4; bank++) {
      for (byte dropNum=0; dropNum<3; dropNum++) {
        dropState[bank][dropNum] = 0;
      }
    }
    
    if (KingsChallengeRunning & KINGS_CHALLENGE_LEVITATE) {
      dropState[2][0] = 1;
      dropState[2][1] = 1;
      dropState[2][2] = 1;
      RPU_SetLampState(DropLamps[2], 1, 0, 100);
    } else {
      RPU_SetLampState(DropLamps[2], 0);
    }
    RPU_SetLampState(DropLamps[0], 0);
    RPU_SetLampState(DropLamps[1], 0);
    RPU_SetLampState(DropLamps[3], 0);

    if (KingsChallengeRunning & KINGS_CHALLENGE_PERFECTION) {
      dropState[KingsChallengePerfectionBank][0] = 1;
      dropState[KingsChallengePerfectionBank][1] = 1;
      dropState[KingsChallengePerfectionBank][2] = 1;
    }
    if (KingsChallengeRunning & KINGS_CHALLENGE_JOUST && CurrentTime>(LastSwitchHitTime+3000)) {
      dropState[0][1] = 1;
      dropState[1][1] = 1;
      dropState[2][1] = 1;
      dropState[3][1] = 1;
    }
    for (byte count = 0; count < 3; count++) {
      RPU_SetLampState(LAMP_ULDROPS_3 + count, dropState[0][count], 0, 200);
      RPU_SetLampState(LAMP_URDROPS_3 + count, dropState[1][count], 0, 200);
      RPU_SetLampState(LAMP_LLDROPS_1 + count, dropState[2][count], 0, 200);
      RPU_SetLampState(LAMP_LRDROPS_3 + count, dropState[3][count], 0, 200);
    }    
  } else {
  
    if (GameMode == GAME_MODE_SKILL_SHOT) {
      RPU_SetLampState(LAMP_ULDROPS_1, SkillShotTarget == 0, 0, 250);
      RPU_SetLampState(LAMP_ULDROPS_2, SkillShotTarget == 1, 0, 250);
      RPU_SetLampState(LAMP_ULDROPS_3, SkillShotTarget == 2, 0, 250);
  
      for (byte count = 0; count < 3; count++) {
        RPU_SetLampState(LAMP_URDROPS_3 + count, 0);
        RPU_SetLampState(LAMP_LLDROPS_1 + count, 0);
        RPU_SetLampState(LAMP_LRDROPS_3 + count, 0);
      }
      RPU_SetLampState(LAMP_UPPER_LEFT_DROPS, 0);
      RPU_SetLampState(LAMP_UPPER_RIGHT_DROPS, 0);
      RPU_SetLampState(LAMP_LOWER_LEFT_DROPS, 0);
      RPU_SetLampState(LAMP_LOWER_RIGHT_DROPS, 0);
    } else if (GameMode == GAME_MODE_SINGLE_COMBAT) {
      byte curStatus0, curStatus1, curStatus2, curStatus3;
      byte lampPhase = (CurrentTime / 200) % 2;
      curStatus0 = DropTargetsUL.GetStatus(false);
      curStatus1 = DropTargetsUR.GetStatus(false);
      curStatus2 = DropTargetsLL.GetStatus(false);
      curStatus3 = DropTargetsLR.GetStatus(false);
  
      RPU_SetLampState(DropLamps[0], lampPhase == 0 && (curStatus0 & 0x07) != 0x07);
      RPU_SetLampState(DropLamps[1], lampPhase == 0 && (curStatus1 & 0x07) != 0x07);
      RPU_SetLampState(DropLamps[2], lampPhase == 0 && (curStatus2 & 0x07) != 0x07);
      RPU_SetLampState(DropLamps[3], lampPhase == 0 && (curStatus3 & 0x07) != 0x07);
  
      byte bitMask0, bitMask1;
      bitMask0 = 0x01;
      bitMask1 = 0x04;
      for (byte count = 0; count < 3; count++) {
  
        RPU_SetLampState(LAMP_ULDROPS_3 + count, lampPhase && (curStatus0 & bitMask0) == 0);
        RPU_SetLampState(LAMP_URDROPS_3 + count, lampPhase && (curStatus1 & bitMask0) == 0);
        RPU_SetLampState(LAMP_LLDROPS_1 + count, lampPhase && (curStatus2 & bitMask0) == 0);
        RPU_SetLampState(LAMP_LRDROPS_3 + count, lampPhase && (curStatus3 & bitMask1) == 0);
        bitMask0 *= 2;
        bitMask1 /= 2;
      }
  
    } else if (GameMode == GAME_MODE_OFFER_DOUBLE_COMBAT) {
      byte lampPhase = (CurrentTime / 100) % 3;
      for (byte count = 0; count < 3; count++) {
        RPU_SetLampState(LAMP_ULDROPS_3 + count, count == lampPhase);
        RPU_SetLampState(LAMP_URDROPS_3 + count, count == lampPhase);
        RPU_SetLampState(LAMP_LLDROPS_1 + count, count == lampPhase);
        RPU_SetLampState(LAMP_LRDROPS_3 + count, count == lampPhase);
      }
    } else if (GameMode == GAME_MODE_DOUBLE_COMBAT_START) {
      byte lampPhase = (CurrentTime / 75) % 4;
      for (byte count = 0; count < 3; count++) {
        RPU_SetLampState(LAMP_ULDROPS_3 + count, lampPhase, lampPhase % 2);
        RPU_SetLampState(LAMP_URDROPS_3 + count, lampPhase, lampPhase % 2);
        RPU_SetLampState(LAMP_LLDROPS_1 + count, lampPhase, lampPhase % 2);
        RPU_SetLampState(LAMP_LRDROPS_3 + count, lampPhase, lampPhase % 2);
      }
    } else if (GameMode == GAME_MODE_DOUBLE_COMBAT) {
      if ( (CombatBankFlags & 0x03) == 0x00 ) {
        byte lampPhase = (CurrentTime / 75) % 10;
        RPU_SetLampState(LAMP_ULDROPS_3, lampPhase == 0);
        RPU_SetLampState(LAMP_ULDROPS_2, lampPhase == 1 || lampPhase == 9);
        RPU_SetLampState(LAMP_ULDROPS_1, lampPhase == 2 || lampPhase == 8);
        RPU_SetLampState(LAMP_URDROPS_1, lampPhase == 3 || lampPhase == 7);
        RPU_SetLampState(LAMP_URDROPS_2, lampPhase == 4 || lampPhase == 6);
        RPU_SetLampState(LAMP_URDROPS_3, lampPhase == 5);
      } else {
        byte lampPhase = (CurrentTime / 400) % 4;
        for (byte count = 0; count < 3; count++) {
          RPU_SetLampState(LAMP_ULDROPS_3 + count, lampPhase && (CombatBankFlags & DROP_BANK_UL_FLAG), lampPhase % 2);
          RPU_SetLampState(LAMP_URDROPS_3 + count, lampPhase && (CombatBankFlags & DROP_BANK_UR_FLAG), lampPhase % 2);
        }
      }
      if ( (CombatBankFlags & 0x0C) == 0x00 ) {
        byte lampPhase = (CurrentTime / 75) % 10;
        RPU_SetLampState(LAMP_LLDROPS_1, lampPhase == 0);
        RPU_SetLampState(LAMP_LLDROPS_2, lampPhase == 1 || lampPhase == 9);
        RPU_SetLampState(LAMP_LLDROPS_3, lampPhase == 2 || lampPhase == 8);
        RPU_SetLampState(LAMP_LRDROPS_1, lampPhase == 3 || lampPhase == 7);
        RPU_SetLampState(LAMP_LRDROPS_2, lampPhase == 4 || lampPhase == 6);
        RPU_SetLampState(LAMP_LRDROPS_3, lampPhase == 5);
      } else {
        byte lampPhase = (CurrentTime / 400) % 4;
        for (byte count = 0; count < 3; count++) {
          RPU_SetLampState(LAMP_LLDROPS_1 + count, lampPhase && (CombatBankFlags & DROP_BANK_LL_FLAG), lampPhase % 2);
          RPU_SetLampState(LAMP_LRDROPS_3 + count, lampPhase && (CombatBankFlags & DROP_BANK_LR_FLAG), lampPhase % 2);
        }
      }
  
      byte banksLeftCount = 4 - CountBits(CombatBankFlags);
      if (banksLeftCount) {
        byte dropBankFlag = 0x01;
        byte lampPhase = (CurrentTime / 500) % banksLeftCount;
        byte litBank = 0;
        for (byte count = 0; count < 4; count++) {
          if ((CombatBankFlags & dropBankFlag) == 0x00) {
            RPU_SetLampState(DropLamps[count], litBank == lampPhase);
            litBank += 1;
          } else {
            RPU_SetLampState(DropLamps[count], 0);
          }
          dropBankFlag *= 2;
        }
      } else {
        for (byte count = 0; count < 4; count++) {
          RPU_SetLampState(DropLamps[count], 0);
        }
      }
  
  
    } else {
      for (byte count = 0; count < 4; count++) {
        if (DropTargetResetTime[count]) {
          RPU_SetLampState(DropLamps[count], DropTargetHurryLamp[count]);
        } else {
          RPU_SetLampState(DropLamps[count], 0);
        }
      }
  
      for (byte count = 0; count < 3; count++) {
        RPU_SetLampState(LAMP_ULDROPS_3 + count, NumDropTargetClears[CurrentPlayer][0] > count);
        RPU_SetLampState(LAMP_URDROPS_3 + count, NumDropTargetClears[CurrentPlayer][1] > count);
        RPU_SetLampState(LAMP_LLDROPS_1 + count, NumDropTargetClears[CurrentPlayer][2] > count);
        RPU_SetLampState(LAMP_LRDROPS_3 + count, NumDropTargetClears[CurrentPlayer][3] > count);
      }
    }
  }
}


void ShowShootAgainLamps() {

  if ( (BallFirstSwitchHitTime == 0 && BallSaveNumSeconds) || (BallSaveEndTime && CurrentTime < BallSaveEndTime) ) {
    unsigned long msRemaining = 5000;
    if (BallSaveEndTime != 0) msRemaining = BallSaveEndTime - CurrentTime;
    RPU_SetLampState(LAMP_SHOOT_AGAIN, 1, 0, (msRemaining < 5000) ? 100 : 500);
    RPU_SetLampState(LAMP_HEAD_SHOOT_AGAIN, 1, 0, (msRemaining < 5000) ? 100 : 500);
  } else {
    RPU_SetLampState(LAMP_SHOOT_AGAIN, SamePlayerShootsAgain);
    RPU_SetLampState(LAMP_HEAD_SHOOT_AGAIN, SamePlayerShootsAgain);
  }
}


boolean RequestedGIState;
unsigned long GIOverrideEndTime;

void SetGeneralIlluminationOn(boolean generalIlluminationOn = true) {
  RequestedGIState = generalIlluminationOn;
  if (GIOverrideEndTime) return;
  RPU_SetContinuousSolenoid(!generalIlluminationOn, SOL_GI_RELAY);
}

void OverrideGeneralIllumination(boolean generalIlluminationOn, unsigned long endTime) {
  GIOverrideEndTime = endTime;
  RPU_SetContinuousSolenoid(!generalIlluminationOn, SOL_GI_RELAY);
}



////////////////////////////////////////////////////////////////////////////
//
//  Machine State Helper functions
//
////////////////////////////////////////////////////////////////////////////
boolean AddPlayer(boolean resetNumPlayers = false) {

  if (Credits < 1 && !FreePlayMode) return false;
  if (resetNumPlayers) CurrentNumPlayers = 0;
  if (CurrentNumPlayers >= 4) return false;

  CurrentNumPlayers += 1;
  RPU_SetDisplay(CurrentNumPlayers - 1, 0, true, 2, true);
  //  RPU_SetDisplayBlank(CurrentNumPlayers - 1, 0x30);

  //  RPU_SetLampState(LAMP_HEAD_1_PLAYER, CurrentNumPlayers==1, 0, 500);
  //  RPU_SetLampState(LAMP_HEAD_2_PLAYERS, CurrentNumPlayers==2, 0, 500);
  //  RPU_SetLampState(LAMP_HEAD_3_PLAYERS, CurrentNumPlayers==3, 0, 500);
  //  RPU_SetLampState(LAMP_HEAD_4_PLAYERS, CurrentNumPlayers==4, 0, 500);

  if (!FreePlayMode) {
    Credits -= 1;
    RPU_WriteByteToEEProm(RPU_CREDITS_EEPROM_BYTE, Credits);
    RPU_SetDisplayCredits(Credits, !FreePlayMode);
    RPU_SetCoinLockout(false);
  }
  if (CurrentNumPlayers == 1) Audio.StopAllAudio();
  QueueNotification(SOUND_EFFECT_VP_ADD_PLAYER_1 + (CurrentNumPlayers - 1), 10);

  RPU_WriteULToEEProm(RPU_TOTAL_PLAYS_EEPROM_START_BYTE, RPU_ReadULFromEEProm(RPU_TOTAL_PLAYS_EEPROM_START_BYTE) + 1);

  return true;
}


unsigned short ChuteAuditByte[] = {RPU_CHUTE_1_COINS_START_BYTE, RPU_CHUTE_2_COINS_START_BYTE, RPU_CHUTE_3_COINS_START_BYTE};
void AddCoinToAudit(byte chuteNum) {
  if (chuteNum > 2) return;
  unsigned short coinAuditStartByte = ChuteAuditByte[chuteNum];
  RPU_WriteULToEEProm(coinAuditStartByte, RPU_ReadULFromEEProm(coinAuditStartByte) + 1);
}


void AddCredit(boolean playSound = false, byte numToAdd = 1) {
  if (Credits < MaximumCredits) {
    Credits += numToAdd;
    if (Credits > MaximumCredits) Credits = MaximumCredits;
    RPU_WriteByteToEEProm(RPU_CREDITS_EEPROM_BYTE, Credits);
    if (playSound) {
      //PlaySoundEffect(SOUND_EFFECT_ADD_CREDIT);
      RPU_PushToSolenoidStack(SOL_BELL, 20, true);
    }
    RPU_SetDisplayCredits(Credits, !FreePlayMode);
    RPU_SetCoinLockout(false);
  } else {
    RPU_SetDisplayCredits(Credits, !FreePlayMode);
    RPU_SetCoinLockout(true);
  }

}

byte SwitchToChuteNum(byte switchHit) {
  byte chuteNum = 0;
  if (switchHit == SW_COIN_2) chuteNum = 1;
  else if (switchHit == SW_COIN_3) chuteNum = 2;
  return chuteNum;
}

boolean AddCoin(byte chuteNum) {
  boolean creditAdded = false;
  if (chuteNum > 2) return false;
  byte cpcSelection = GetCPCSelection(chuteNum);

  // Find the lowest chute num with the same ratio selection
  // and use that ChuteCoinsInProgress counter
  byte chuteNumToUse;
  for (chuteNumToUse = 0; chuteNumToUse <= chuteNum; chuteNumToUse++) {
    if (GetCPCSelection(chuteNumToUse) == cpcSelection) break;
  }

  PlaySoundEffect(SOUND_EFFECT_COIN_DROP_1 + (CurrentTime % 3));

  byte cpcCoins = GetCPCCoins(cpcSelection);
  byte cpcCredits = GetCPCCredits(cpcSelection);
  byte coinProgressBefore = ChuteCoinsInProgress[chuteNumToUse];
  ChuteCoinsInProgress[chuteNumToUse] += 1;

  if (ChuteCoinsInProgress[chuteNumToUse] == cpcCoins) {
    if (cpcCredits > cpcCoins) AddCredit(cpcCredits - (coinProgressBefore));
    else AddCredit(cpcCredits);
    ChuteCoinsInProgress[chuteNumToUse] = 0;
    creditAdded = true;
  } else {
    if (cpcCredits > cpcCoins) {
      AddCredit(1);
      creditAdded = true;
    } else {
    }
  }

  return creditAdded;
}


void AddSpecialCredit() {
  AddCredit(false, 1);
  RPU_PushToTimedSolenoidStack(SOL_BELL, 50, CurrentTime, true);
  RPU_WriteULToEEProm(RPU_TOTAL_REPLAYS_EEPROM_START_BYTE, RPU_ReadULFromEEProm(RPU_TOTAL_REPLAYS_EEPROM_START_BYTE) + 1);
}

void AwardSpecial() {
  if (SpecialCollected) return;
  SpecialCollected = true;
  if (TournamentScoring) {
    CurrentScores[CurrentPlayer] += SpecialValue * PlayfieldMultiplier;
  } else {
    AddSpecialCredit();
  }
}

boolean AwardExtraBall() {
  if (ExtraBallCollected) return false;
  ExtraBallCollected = true;
  ExtraBallAwardStartTime = CurrentTime;
  RPU_PushToTimedSolenoidStack(SOL_BELL, 35, CurrentTime, true);
  RPU_PushToTimedSolenoidStack(SOL_BELL, 35, CurrentTime + 400, true);
  if (TournamentScoring) {
    CurrentScores[CurrentPlayer] += ExtraBallValue * PlayfieldMultiplier;
  } else {
    SamePlayerShootsAgain = true;
    RPU_SetLampState(LAMP_SHOOT_AGAIN, SamePlayerShootsAgain);
    RPU_SetLampState(LAMP_HEAD_SHOOT_AGAIN, SamePlayerShootsAgain);
    QueueNotification(SOUND_EFFECT_VP_EXTRA_BALL, 8);
  }
  return true;
}


void IncreasePlayfieldMultiplier(unsigned long duration) {
  if (PlayfieldMultiplierExpiration) PlayfieldMultiplierExpiration += duration;
  else PlayfieldMultiplierExpiration = CurrentTime + duration;
  PlayfieldMultiplier += 1;
  if (PlayfieldMultiplier > 5) {
    PlayfieldMultiplier = 5;
  } else {
    QueueNotification(SOUND_EFFECT_VP_RETURN_TO_1X + (PlayfieldMultiplier - 1), 1);
  }
}

unsigned long UpperLockSwitchDownTime[3] = {0, 0, 0};
unsigned long UpperLockSwitchUpTime[3] = {0, 0, 0};
unsigned long UpperLockLastChecked = 0;
unsigned long MachineLockDiscrepancyTime = 0;
boolean UpperLockSwitchState[3] = {false, false, false};

byte InitializeMachineLocksBasedOnSwitches() {
  byte returnLocks = 0;

  if (RPU_ReadSingleSwitchState(SW_LOCK_1)) {
    returnLocks |= LOCK_1_ENGAGED;
    // Also update the UpperLockSwitchState variable
    UpperLockSwitchState[0] = true;
  }
  if (RPU_ReadSingleSwitchState(SW_LOCK_2)) {
    returnLocks |= LOCK_2_ENGAGED;
    // Also update the UpperLockSwitchState variable
    UpperLockSwitchState[1] = true;
  }
  if (RPU_ReadSingleSwitchState(SW_LOCK_3)) {
    returnLocks |= LOCK_3_ENGAGED;
    // Also update the UpperLockSwitchState variable
    UpperLockSwitchState[2] = true;
  }
  //  if (includeSaucerLock && RPU_ReadSingleSwitchState(SW_SAUCER)) returnLocks |= LOCK_SAUCER_ENGAGED;

  if (DEBUG_MESSAGES) {
    char buf[256];
    sprintf(buf, "Initializing Machine Locks = 0x%04X\n", returnLocks);
    Serial.write(buf);
  }

  return returnLocks;
}



void UpdateLockStatus() {
  boolean lockSwitchDownTransition;
  boolean lockSwitchUpTransition;

  for (byte count = 0; count < 3; count++) {
    lockSwitchDownTransition = false;
    lockSwitchUpTransition = false;

    if (RPU_ReadSingleSwitchState(SW_LOCK_1 + count)) {
      UpperLockSwitchUpTime[count] = 0;
      if (UpperLockSwitchDownTime[count] == 0) {
        UpperLockSwitchDownTime[count] = CurrentTime;
        if (DEBUG_MESSAGES) {
          char buf[128];
          sprintf(buf, "Down: starting down counter (%lu) for %d\n", CurrentTime, count);
          Serial.write(buf);
        }
      } else if (CurrentTime > (UpperLockSwitchDownTime[count] + 750)) {
        lockSwitchDownTransition = true;
      }
    } else {
      UpperLockSwitchDownTime[count] = 0;
      if (UpperLockSwitchUpTime[count] == 0) {
        UpperLockSwitchUpTime[count] = CurrentTime;
        if (DEBUG_MESSAGES) {
          char buf[128];
          sprintf(buf, "Up: starting up counter (%lu) for %d\n", CurrentTime, count);
          Serial.write(buf);
        }
      } else if (CurrentTime > (UpperLockSwitchUpTime[count] + 750)) {
        lockSwitchUpTransition = true;
      }
    }

    if (lockSwitchUpTransition && UpperLockSwitchState[count]) {
      // if we used to be down & now we're up
      UpperLockSwitchState[count] = false;
      if (DEBUG_MESSAGES) {
        char buf[128];
        sprintf(buf, "ULS - saw up on %d\n", count);
        Serial.write(buf);
      }
    } else if (lockSwitchDownTransition && !UpperLockSwitchState[count]) {
      // if we used to be up & now we're down
      UpperLockSwitchState[count] = true;
      if (DEBUG_MESSAGES) Serial.write("Saw lock sw down - handle it\n");
      HandleLockSwitch(count);
      if (DEBUG_MESSAGES) {
        char buf[128];
        sprintf(buf, "ULS - saw down on %d\n", count);
        Serial.write(buf);
      }
    }
  }

  boolean waitingForKick = false;
  if (LockKickTime[0] || LockKickTime[1] || LockKickTime[2]) waitingForKick = true;

  if (!waitingForKick && LockManagementInProgress == false && (UpperLockLastChecked == 0 || CurrentTime > (UpperLockLastChecked + 200))) {
    UpperLockLastChecked = CurrentTime;
    byte curUpperLock = 0;
    if (UpperLockSwitchState[0]) curUpperLock |= LOCK_1_ENGAGED;
    if (UpperLockSwitchState[1]) curUpperLock |= LOCK_2_ENGAGED;
    if (UpperLockSwitchState[2]) curUpperLock |= LOCK_3_ENGAGED;
    if ( (MachineLocks & LOCKS_ENGAGED_MASK) != curUpperLock ) {
      if (MachineLockDiscrepancyTime == 0) {
        MachineLockDiscrepancyTime = CurrentTime;
      } else if (CurrentTime > (MachineLockDiscrepancyTime + 1000)) {
        // If "MachineLocks" has been out of sync
        // for a full second, we should re-sync it
        // to the switches
        for (byte count = 0; count < 3; count++) UpperLockSwitchState[count] = false;
        MachineLocks &= ~LOCKS_ENGAGED_MASK;
        MachineLocks |= InitializeMachineLocksBasedOnSwitches();
        NumberOfBallsLocked = CountBits(MachineLocks & LOCKS_ENGAGED_MASK);
      }
    } else {
      MachineLockDiscrepancyTime = 0;
    }
  }

}


void UpdatePlayerLocks() {
  byte curLockEngagedStatus = PlayerLockStatus[CurrentPlayer] & LOCKS_ENGAGED_MASK;
  if (curLockEngagedStatus) {
    // Check to see if a lock has been stolen
    if ( (curLockEngagedStatus & MachineLocks) != curLockEngagedStatus ) {
      byte lockToCheck = LOCK_1_ENGAGED;
      byte lockAvail = LOCK_1_AVAILABLE;
      for (byte count = 0; count < 3; count++) {
        if ( (curLockEngagedStatus & lockToCheck) && !(MachineLocks & lockToCheck) ) {
          // If a lock has been stolen, move it from locked to available
          PlayerLockStatus[CurrentPlayer] &= ~lockToCheck;
          PlayerLockStatus[CurrentPlayer] |= lockAvail;
        }
        lockToCheck *= 2;
        lockAvail *= 2;
      }
    }
  }
}

void LockBall(byte lockIndex = 0xFF) {

  if (lockIndex == 0xFF) {
    // We need to determine which lock this is
    lockIndex = CountBits(MachineLocks & LOCKS_ENGAGED_MASK);
    MachineLocks |= (LOCK_1_ENGAGED << lockIndex);
    NumberOfBallsLocked = CountBits(MachineLocks & LOCKS_ENGAGED_MASK);
    if (NumberOfBallsInPlay) NumberOfBallsInPlay -= 1;
    if (DEBUG_MESSAGES) {
      char buf[128];
      sprintf(buf, "Num BIP minus 1 to %d b/c LockBall\n", NumberOfBallsInPlay);
      Serial.write(buf);
    }

    byte playerLock = CountBits(PlayerLockStatus[CurrentPlayer] & LOCKS_ENGAGED_MASK);
    PlayerLockStatus[CurrentPlayer] &= ~(LOCK_1_AVAILABLE << playerLock);
    PlayerLockStatus[CurrentPlayer] |= (LOCK_1_ENGAGED << playerLock);

    if (DEBUG_MESSAGES) {
      char buf[128];
      sprintf(buf, "Ball locked -- PL=0x%02X, ML=0x%02X\n", PlayerLockStatus[CurrentPlayer], MachineLocks);
      Serial.write(buf);
    }
  } else {
    PlayerLockStatus[CurrentPlayer] &= ~(LOCK_1_AVAILABLE << lockIndex);
    PlayerLockStatus[CurrentPlayer] |= (LOCK_1_ENGAGED << lockIndex);
    MachineLocks |= (LOCK_1_ENGAGED << lockIndex);
    NumberOfBallsLocked = CountBits(MachineLocks & LOCKS_ENGAGED_MASK);
  }
}

void ReleaseLockedBall() {
  if (NumberOfBallsLocked) {
    NumberOfBallsLocked -= 1;

    RPU_PushToSolenoidStack(SOL_UPPER_BALL_EJECT, 12, true);

    // Figure out which ball we're kicking
    for (byte count = 0; count < 3; count++) {
      if (MachineLocks & (LOCK_3_ENGAGED >> count)) {
        // remove highest MachineLock that we find and break
        MachineLocks &= ~(LOCK_3_ENGAGED >> count);
        break;
      }
    }

    if (DEBUG_MESSAGES) {
      char buf[256];
      sprintf(buf, "Releasing - Machine Locks = 0x%04X\n", MachineLocks);
      Serial.write(buf);
    }
  }
}

boolean PutBallInPlay() {
  if (NumberOfBallsLocked == 3) {
    // Need to release a locked ball
    ReleaseLockedBall();
    return false;
  } else if (CountBallsInTrough()) {
    RPU_PushToTimedSolenoidStack(SOL_BALL_RAMP_THROWER, 16, CurrentTime + 100);
    NumberOfBallsInPlay += 1;
    if (DEBUG_MESSAGES) {
      char buf[128];
      sprintf(buf, "Num BIP +1 to %d b/c PutBallInPlay\n", NumberOfBallsInPlay);
      Serial.write(buf);
    }
    return true;
  } else {
    // No lock and no balls in trough -- error!
    return false;
  }

  return true;
}


#define ADJ_TYPE_LIST                 1
#define ADJ_TYPE_MIN_MAX              2
#define ADJ_TYPE_MIN_MAX_DEFAULT      3
#define ADJ_TYPE_SCORE                4
#define ADJ_TYPE_SCORE_WITH_DEFAULT   5
#define ADJ_TYPE_SCORE_NO_DEFAULT     6
byte AdjustmentType = 0;
byte NumAdjustmentValues = 0;
byte AdjustmentValues[8];
byte CurrentAdjustmentStorageByte = 0;
byte TempValue = 0;
byte *CurrentAdjustmentByte = NULL;
unsigned long *CurrentAdjustmentUL = NULL;
unsigned long SoundSettingTimeout = 0;
unsigned long AdjustmentScore;



int RunSelfTest(int curState, boolean curStateChanged) {
  int returnState = curState;
  CurrentNumPlayers = 0;

  if (curStateChanged) {
    // Send a stop-all command and reset the sample-rate offset, in case we have
    //  reset while the WAV Trigger was already playing.
    Audio.StopAllAudio();
    RPU_TurnOffAllLamps();
    int modeMapping = SelfTestStateToCalloutMap[-1 - curState];
    Audio.PlaySound((unsigned short)modeMapping, AUDIO_PLAY_TYPE_WAV_TRIGGER, 10);
  } else {
    if (SoundSettingTimeout && CurrentTime > SoundSettingTimeout) {
      SoundSettingTimeout = 0;
      Audio.StopAllAudio();
    }
  }

  // Any state that's greater than MACHINE_STATE_TEST_DONE is handled by the Base Self-test code
  // Any that's less, is machine specific, so we handle it here.
  if (curState >= MACHINE_STATE_TEST_DONE) {
    byte cpcSelection = 0xFF;
    byte chuteNum = 0xFF;
    if (curState == MACHINE_STATE_ADJUST_CPC_CHUTE_1) chuteNum = 0;
    if (curState == MACHINE_STATE_ADJUST_CPC_CHUTE_2) chuteNum = 1;
    if (curState == MACHINE_STATE_ADJUST_CPC_CHUTE_3) chuteNum = 2;
    if (chuteNum != 0xFF) cpcSelection = GetCPCSelection(chuteNum);
    returnState = RunBaseSelfTest(returnState, curStateChanged, CurrentTime, SW_CREDIT_RESET, SW_SLAM);
    if (chuteNum != 0xFF) {
      if (cpcSelection != GetCPCSelection(chuteNum)) {
        byte newCPC = GetCPCSelection(chuteNum);
        Audio.StopAllAudio();
        Audio.PlaySound(SOUND_EFFECT_SELF_TEST_CPC_START + newCPC, AUDIO_PLAY_TYPE_WAV_TRIGGER, 10);
      }
    }
  } else {
    byte curSwitch = RPU_PullFirstFromSwitchStack();

    if (curSwitch == SW_SELF_TEST_SWITCH && (CurrentTime - GetLastSelfTestChangedTime()) > 250) {
      SetLastSelfTestChangedTime(CurrentTime);
      if (RPU_GetUpDownSwitchState()) returnState -= 1;
      else returnState += 1;
    }

    if (curSwitch == SW_SLAM) {
      returnState = MACHINE_STATE_ATTRACT;
    }

    if (curStateChanged) {
      for (int count = 0; count < 4; count++) {
        RPU_SetDisplay(count, 0);
        RPU_SetDisplayBlank(count, 0x00);
      }
      RPU_SetDisplayCredits(0, false);
      RPU_SetDisplayBallInPlay(MACHINE_STATE_TEST_BOOT - curState, true);
      CurrentAdjustmentByte = NULL;
      CurrentAdjustmentUL = NULL;
      CurrentAdjustmentStorageByte = 0;

      AdjustmentType = ADJ_TYPE_MIN_MAX;
      AdjustmentValues[0] = 0;
      AdjustmentValues[1] = 1;
      TempValue = 0;

      switch (curState) {
        case MACHINE_STATE_ADJUST_FREEPLAY:
          CurrentAdjustmentByte = (byte *)&FreePlayMode;
          CurrentAdjustmentStorageByte = EEPROM_FREE_PLAY_BYTE;
          break;
        case MACHINE_STATE_ADJUST_BALL_SAVE:
          AdjustmentType = ADJ_TYPE_LIST;
          NumAdjustmentValues = 5;
          AdjustmentValues[1] = 5;
          AdjustmentValues[2] = 10;
          AdjustmentValues[3] = 15;
          AdjustmentValues[4] = 20;
          CurrentAdjustmentByte = &BallSaveNumSeconds;
          CurrentAdjustmentStorageByte = EEPROM_BALL_SAVE_BYTE;
          break;
        case MACHINE_STATE_ADJUST_SOUND_SELECTOR:
          AdjustmentType = ADJ_TYPE_MIN_MAX;
          AdjustmentValues[1] = 9;
          CurrentAdjustmentByte = &SoundSelector;
          CurrentAdjustmentStorageByte = EEPROM_SOUND_SELECTOR_BYTE;
          break;
        case MACHINE_STATE_ADJUST_MUSIC_VOLUME:
          AdjustmentType = ADJ_TYPE_MIN_MAX;
          AdjustmentValues[0] = 0;
          AdjustmentValues[1] = 10;
          CurrentAdjustmentByte = &MusicVolume;
          CurrentAdjustmentStorageByte = EEPROM_MUSIC_VOLUME_BYTE;
          break;
        case MACHINE_STATE_ADJUST_SFX_VOLUME:
          AdjustmentType = ADJ_TYPE_MIN_MAX;
          AdjustmentValues[0] = 0;
          AdjustmentValues[1] = 10;
          CurrentAdjustmentByte = &SoundEffectsVolume;
          CurrentAdjustmentStorageByte = EEPROM_SFX_VOLUME_BYTE;
          break;
        case MACHINE_STATE_ADJUST_CALLOUTS_VOLUME:
          AdjustmentType = ADJ_TYPE_MIN_MAX;
          AdjustmentValues[0] = 0;
          AdjustmentValues[1] = 10;
          CurrentAdjustmentByte = &CalloutsVolume;
          CurrentAdjustmentStorageByte = EEPROM_CALLOUTS_VOLUME_BYTE;
          break;
        case MACHINE_STATE_ADJUST_TOURNAMENT_SCORING:
          CurrentAdjustmentByte = (byte *)&TournamentScoring;
          CurrentAdjustmentStorageByte = EEPROM_TOURNAMENT_SCORING_BYTE;
          break;
        case MACHINE_STATE_ADJUST_TILT_WARNING:
          AdjustmentValues[1] = 2;
          CurrentAdjustmentByte = &MaxTiltWarnings;
          CurrentAdjustmentStorageByte = EEPROM_TILT_WARNING_BYTE;
          break;
        case MACHINE_STATE_ADJUST_AWARD_OVERRIDE:
          AdjustmentType = ADJ_TYPE_MIN_MAX_DEFAULT;
          AdjustmentValues[1] = 7;
          CurrentAdjustmentByte = &ScoreAwardReplay;
          CurrentAdjustmentStorageByte = EEPROM_AWARD_OVERRIDE_BYTE;
          break;
        case MACHINE_STATE_ADJUST_BALLS_OVERRIDE:
          AdjustmentType = ADJ_TYPE_LIST;
          NumAdjustmentValues = 3;
          AdjustmentValues[0] = 3;
          AdjustmentValues[1] = 5;
          AdjustmentValues[2] = 99;
          CurrentAdjustmentByte = &BallsPerGame;
          CurrentAdjustmentStorageByte = EEPROM_BALLS_OVERRIDE_BYTE;
          break;
        case MACHINE_STATE_ADJUST_SCROLLING_SCORES:
          CurrentAdjustmentByte = (byte *)&ScrollingScores;
          CurrentAdjustmentStorageByte = EEPROM_SCROLLING_SCORES_BYTE;
          break;
        case MACHINE_STATE_ADJUST_EXTRA_BALL_AWARD:
          AdjustmentType = ADJ_TYPE_SCORE_WITH_DEFAULT;
          CurrentAdjustmentUL = &ExtraBallValue;
          CurrentAdjustmentStorageByte = EEPROM_EXTRA_BALL_SCORE_UL;
          break;
        case MACHINE_STATE_ADJUST_SPECIAL_AWARD:
          AdjustmentType = ADJ_TYPE_SCORE_WITH_DEFAULT;
          CurrentAdjustmentUL = &SpecialValue;
          CurrentAdjustmentStorageByte = EEPROM_SPECIAL_SCORE_UL;
          break;
        case MACHINE_STATE_ADJUST_IDLE_MODE:
          CurrentAdjustmentByte = (byte *)&IdleModeEnabled;
          CurrentAdjustmentStorageByte = EEPROM_IDLE_MODE_BYTE;
          break;
        case MACHINE_STATE_ADJUST_ALLOW_RESET:
          CurrentAdjustmentByte = (byte *)&AllowResetAfterBallOne;
          CurrentAdjustmentStorageByte = EEPROM_ALLOW_RESET_BYTE;
          break;
        case MACHINE_STATE_ADJUST_DONE:
          returnState = MACHINE_STATE_ATTRACT;
          break;
      }
    }

    // Change value, if the switch is hit
    if (curSwitch == SW_CREDIT_RESET) {

      if (CurrentAdjustmentByte && (AdjustmentType == ADJ_TYPE_MIN_MAX || AdjustmentType == ADJ_TYPE_MIN_MAX_DEFAULT)) {
        byte curVal = *CurrentAdjustmentByte;

        if (RPU_GetUpDownSwitchState()) {
          curVal += 1;
          if (curVal > AdjustmentValues[1]) {
            if (AdjustmentType == ADJ_TYPE_MIN_MAX) curVal = AdjustmentValues[0];
            else {
              if (curVal > 99) curVal = AdjustmentValues[0];
              else curVal = 99;
            }
          }
        } else {
          if (curVal == AdjustmentValues[0]) {
            if (AdjustmentType == ADJ_TYPE_MIN_MAX_DEFAULT) curVal = 99;
            else curVal = AdjustmentValues[1];
          } else {
            curVal -= 1;
          }
        }

        *CurrentAdjustmentByte = curVal;
        if (CurrentAdjustmentStorageByte) EEPROM.write(CurrentAdjustmentStorageByte, curVal);

        if (curState == MACHINE_STATE_ADJUST_SOUND_SELECTOR) {
          Audio.StopAllAudio();
          Audio.PlaySound(SOUND_EFFECT_SELF_TEST_AUDIO_OPTIONS_START + curVal, AUDIO_PLAY_TYPE_WAV_TRIGGER, 10);
        } else if (curState == MACHINE_STATE_ADJUST_MUSIC_VOLUME) {
          if (SoundSettingTimeout) Audio.StopAllAudio();
          Audio.PlaySound(SOUND_EFFECT_BACKGROUND_SONG_1, AUDIO_PLAY_TYPE_WAV_TRIGGER, curVal);
          Audio.SetMusicVolume(curVal);
          SoundSettingTimeout = CurrentTime + 5000;
        } else if (curState == MACHINE_STATE_ADJUST_SFX_VOLUME) {
          if (SoundSettingTimeout) Audio.StopAllAudio();
          Audio.PlaySound(SOUND_EFFECT_COUNTDOWN_BONUS_START, AUDIO_PLAY_TYPE_WAV_TRIGGER, curVal);
          Audio.SetSoundFXVolume(curVal);
          SoundSettingTimeout = CurrentTime + 5000;
        } else if (curState == MACHINE_STATE_ADJUST_CALLOUTS_VOLUME) {
          if (SoundSettingTimeout) Audio.StopAllAudio();
          Audio.PlaySound(SOUND_EFFECT_VP_JACKPOT, AUDIO_PLAY_TYPE_WAV_TRIGGER, curVal);
          Audio.SetNotificationsVolume(curVal);
          SoundSettingTimeout = CurrentTime + 3000;
        }

      } else if (CurrentAdjustmentByte && AdjustmentType == ADJ_TYPE_LIST) {
        byte valCount = 0;
        byte curVal = *CurrentAdjustmentByte;
        byte newIndex = 0;
        boolean upDownState = RPU_GetUpDownSwitchState();
        for (valCount = 0; valCount < (NumAdjustmentValues); valCount++) {
          if (curVal == AdjustmentValues[valCount]) {
            if (upDownState) {
              if (valCount < (NumAdjustmentValues - 1)) newIndex = valCount + 1;
            } else {
              if (valCount > 0) newIndex = valCount - 1;
            }
          }
        }
        *CurrentAdjustmentByte = AdjustmentValues[newIndex];
        if (CurrentAdjustmentStorageByte) EEPROM.write(CurrentAdjustmentStorageByte, AdjustmentValues[newIndex]);
      } else if (CurrentAdjustmentUL && (AdjustmentType == ADJ_TYPE_SCORE_WITH_DEFAULT || AdjustmentType == ADJ_TYPE_SCORE_NO_DEFAULT)) {
        unsigned long curVal = *CurrentAdjustmentUL;
        if (RPU_GetUpDownSwitchState()) curVal += 5000;
        else if (curVal >= 5000) curVal -= 5000;
        if (curVal > 100000) curVal = 0;
        if (AdjustmentType == ADJ_TYPE_SCORE_NO_DEFAULT && curVal == 0) curVal = 5000;
        *CurrentAdjustmentUL = curVal;
        if (CurrentAdjustmentStorageByte) RPU_WriteULToEEProm(CurrentAdjustmentStorageByte, curVal);
      }

    }

    // Show current value
    if (CurrentAdjustmentByte != NULL) {
      RPU_SetDisplay(0, (unsigned long)(*CurrentAdjustmentByte), true);
    } else if (CurrentAdjustmentUL != NULL) {
      RPU_SetDisplay(0, (*CurrentAdjustmentUL), true);
    }

  }

  if (returnState == MACHINE_STATE_ATTRACT) {
    // If any variables have been set to non-override (99), return
    // them to dip switch settings
    // Balls Per Game, Player Loses On Ties, Novelty Scoring, Award Score
    //    DecodeDIPSwitchParameters();
    RPU_SetDisplayCredits(Credits, !FreePlayMode);
    ReadStoredParameters();
  }

  return returnState;
}




////////////////////////////////////////////////////////////////////////////
//
//  Audio Output functions
//
////////////////////////////////////////////////////////////////////////////
void PlayBackgroundSong(unsigned int songNum) {

  if (MusicVolume == 0) return;

  Audio.PlayBackgroundSong(songNum);
}


unsigned long NextSoundEffectTime = 0;

void PlaySoundEffect(unsigned int soundEffectNum) {

  if (MachineState == MACHINE_STATE_INIT_GAMEPLAY) return;
  Audio.PlaySound(soundEffectNum, AUDIO_PLAY_TYPE_WAV_TRIGGER);

  /*
    switch (soundEffectNum) {
      case SOUND_EFFECT_LEFT_SHOOTER_LANE:
        Audio.PlaySoundCardWhenPossible(12, CurrentTime, 0, 500, 7);
        break;
      case SOUND_EFFECT_RETURN_TO_SHOOTER_LANE:
        Audio.PlaySoundCardWhenPossible(22, CurrentTime, 0, 500, 8);
        break;
      case SOUND_EFFECT_SAUCER:
        Audio.PlaySoundCardWhenPossible(14, CurrentTime, 0, 500, 7);
        break;
      case SOUND_EFFECT_DROP_TARGET_HURRY:
        Audio.PlaySoundCardWhenPossible(2, CurrentTime, 0, 45, 3);
        break;
      case SOUND_EFFECT_DROP_TARGET_COMPLETE:
        Audio.PlaySoundCardWhenPossible(9, CurrentTime, 0, 1400, 4);
        Audio.PlaySoundCardWhenPossible(19, CurrentTime, 1500, 10, 4);
        break;
      case SOUND_EFFECT_HOOFBEATS:
        Audio.PlaySoundCardWhenPossible(12, CurrentTime, 0, 100, 10);
        break;
      case SOUND_EFFECT_STOP_BACKGROUND:
        Audio.PlaySoundCardWhenPossible(19, CurrentTime, 0, 10, 10);
        break;
      case SOUND_EFFECT_DROP_TARGET_HIT:
        Audio.PlaySoundCardWhenPossible(7, CurrentTime, 0, 150, 5);
        break;
      case SOUND_EFFECT_SPINNER:
        Audio.PlaySoundCardWhenPossible(6, CurrentTime, 0, 25, 2);
        break;
    }
  */
}


void QueueNotification(unsigned int soundEffectNum, byte priority) {
  if (CalloutsVolume == 0) return;
  if (SoundSelector < 3 || SoundSelector == 4 || SoundSelector == 7 || SoundSelector == 9) return;
  //if (soundEffectNum < SOUND_EFFECT_VP_VOICE_NOTIFICATIONS_START || soundEffectNum >= (SOUND_EFFECT_VP_VOICE_NOTIFICATIONS_START + NUM_VOICE_NOTIFICATIONS)) return;

  Audio.QueuePrioritizedNotification(soundEffectNum, 0, priority, CurrentTime);
}


void AlertPlayerUp(byte playerNum) {
  //  (void)playerNum;
  //  QueueNotification(SOUND_EFFECT_VP_PLAYER, 1);
  QueueNotification(SOUND_EFFECT_VP_PLAYER_ONE_UP + playerNum, 1);
  //  QueueNotification(SOUND_EFFECT_VP_LAUNCH_WHEN_READY, 1);
}





////////////////////////////////////////////////////////////////////////////
//
//  Diagnostics Mode
//
////////////////////////////////////////////////////////////////////////////

int RunDiagnosticsMode(int curState, boolean curStateChanged) {

  int returnState = curState;

  if (curStateChanged) {

    /*
        char buf[256];
        boolean errorSeen;

        Serial.write("Testing Volatile RAM at IC13 (0x0000 - 0x0080): writing & reading... ");
        Serial.write("3 ");
        delay(500);
        Serial.write("2 ");
        delay(500);
        Serial.write("1 \n");
        delay(500);
        errorSeen = false;
        for (byte valueCount=0; valueCount<0xFF; valueCount++) {
          for (unsigned short address=0x0000; address<0x0080; address++) {
            RPU_DataWrite(address, valueCount);
          }
          for (unsigned short address=0x0000; address<0x0080; address++) {
            byte readValue = RPU_DataRead(address);
            if (readValue!=valueCount) {
              sprintf(buf, "Write/Read failure at address=0x%04X (expected 0x%02X, read 0x%02X)\n", address, valueCount, readValue);
              Serial.write(buf);
              errorSeen = true;
            }
            if (errorSeen) break;
          }
          if (errorSeen) break;
        }
        if (errorSeen) {
          Serial.write("!!! Error in Volatile RAM\n");
        }

        Serial.write("Testing Volatile RAM at IC16 (0x0080 - 0x0100): writing & reading... ");
        Serial.write("3 ");
        delay(500);
        Serial.write("2 ");
        delay(500);
        Serial.write("1 \n");
        delay(500);
        errorSeen = false;
        for (byte valueCount=0; valueCount<0xFF; valueCount++) {
          for (unsigned short address=0x0080; address<0x0100; address++) {
            RPU_DataWrite(address, valueCount);
          }
          for (unsigned short address=0x0080; address<0x0100; address++) {
            byte readValue = RPU_DataRead(address);
            if (readValue!=valueCount) {
              sprintf(buf, "Write/Read failure at address=0x%04X (expected 0x%02X, read 0x%02X)\n", address, valueCount, readValue);
              Serial.write(buf);
              errorSeen = true;
            }
            if (errorSeen) break;
          }
          if (errorSeen) break;
        }
        if (errorSeen) {
          Serial.write("!!! Error in Volatile RAM\n");
        }

        // Check the CMOS RAM to see if it's operating correctly
        errorSeen = false;
        Serial.write("Testing CMOS RAM: writing & reading... ");
        Serial.write("3 ");
        delay(500);
        Serial.write("2 ");
        delay(500);
        Serial.write("1 \n");
        delay(500);
        for (byte valueCount=0; valueCount<0x10; valueCount++) {
          for (unsigned short address=0x0100; address<0x0200; address++) {
            RPU_DataWrite(address, valueCount);
          }
          for (unsigned short address=0x0100; address<0x0200; address++) {
            byte readValue = RPU_DataRead(address);
            if ((readValue&0x0F)!=valueCount) {
              sprintf(buf, "Write/Read failure at address=0x%04X (expected 0x%02X, read 0x%02X)\n", address, valueCount, (readValue&0x0F));
              Serial.write(buf);
              errorSeen = true;
            }
            if (errorSeen) break;
          }
          if (errorSeen) break;
        }

        if (errorSeen) {
          Serial.write("!!! Error in CMOS RAM\n");
        }


        // Check the ROMs
        Serial.write("CMOS RAM dump... ");
        Serial.write("3 ");
        delay(500);
        Serial.write("2 ");
        delay(500);
        Serial.write("1 \n");
        delay(500);
        for (unsigned short address=0x0100; address<0x0200; address++) {
          if ((address&0x000F)==0x0000) {
            sprintf(buf, "0x%04X:  ", address);
            Serial.write(buf);
          }
      //      RPU_DataWrite(address, address&0xFF);
          sprintf(buf, "0x%02X ", RPU_DataRead(address));
          Serial.write(buf);
          if ((address&0x000F)==0x000F) {
            Serial.write("\n");
          }
        }

    */

    //    RPU_EnableSolenoidStack();
    //    RPU_SetDisableFlippers(false);

  }

  return returnState;
}



////////////////////////////////////////////////////////////////////////////
//
//  Attract Mode
//
////////////////////////////////////////////////////////////////////////////

unsigned long AttractLastLadderTime = 0;
byte AttractLastLadderBonus = 0;
unsigned long AttractDisplayRampStart = 0;
byte AttractLastHeadMode = 255;
byte AttractLastPlayfieldMode = 255;
byte InAttractMode = false;


int RunAttractMode(int curState, boolean curStateChanged) {

  int returnState = curState;

  if (curStateChanged) {
    //PlaySoundEffect(SOUND_EFFECT_STOP_BACKGROUND);
    RPU_DisableSolenoidStack();
    RPU_TurnOffAllLamps();
    RPU_SetDisableFlippers(true);
    if (DEBUG_MESSAGES) {
      Serial.write("Entering Attract Mode\n\r");
    }
    AttractLastHeadMode = 0;
    AttractLastPlayfieldMode = 0;
    RPU_SetDisplayCredits(Credits, !FreePlayMode);
    for (byte count = 0; count < 4; count++) {
      //      RPU_SetLampState(LAMP_HEAD_PLAYER_1_UP + count, 0);
    }

    //    RPU_SetLampState(LAMP_HEAD_1_PLAYER, 0);
    //    RPU_SetLampState(LAMP_HEAD_2_PLAYERS, 0);
    //    RPU_SetLampState(LAMP_HEAD_3_PLAYERS, 0);
    //    RPU_SetLampState(LAMP_HEAD_4_PLAYERS, 0);

    // Update MachineLocks, and kick the ball from the saucer
    // (if it's there). MachineLocks doesn't include the saucer
    // unless we're playing a game.
    MachineLocks = InitializeMachineLocksBasedOnSwitches();
    if (RPU_ReadSingleSwitchState(SW_SAUCER)) {
      if (SaucerEjectTime == 0 || CurrentTime > (SaucerEjectTime + 2500)) {
        RPU_PushToSolenoidStack(SOL_SAUCER, 12, true);
        SaucerEjectTime = CurrentTime;
      }
    }

  }

  UpdateLockStatus();
  MoveBallFromOutholeToRamp();

  // Alternate displays between high score and blank
  if (CurrentTime < 16000) {
    if (AttractLastHeadMode != 1) {
      ShowPlayerScores(0xFF, false, false);
      RPU_SetDisplayCredits(Credits, !FreePlayMode);
      RPU_SetDisplayBallInPlay(0, true);
    }
  } else if ((CurrentTime / 8000) % 2 == 0) {

    if (AttractLastHeadMode != 2) {
      RPU_SetLampState(LAMP_HEAD_HIGH_SCORE_TO_DATE, 1, 0, 250);
      RPU_SetLampState(LAMP_HEAD_GAME_OVER, 0);
      SetLastTimeScoreChanged(CurrentTime);
    }
    AttractLastHeadMode = 2;
    ShowPlayerScores(0xFF, false, false, HighScore);
  } else {
    if (AttractLastHeadMode != 3) {
      if (CurrentTime < 32000) {
        for (int count = 0; count < 4; count++) {
          CurrentScores[count] = 0;
        }
        CurrentNumPlayers = 0;
      }
      RPU_SetLampState(LAMP_HEAD_HIGH_SCORE_TO_DATE, 0);
      RPU_SetLampState(LAMP_HEAD_GAME_OVER, 1);
      SetLastTimeScoreChanged(CurrentTime);
    }
    ShowPlayerScores(0xFF, false, false);

    AttractLastHeadMode = 3;
  }

  byte attractPlayfieldPhase = ((CurrentTime / 5000) % 6);

  if (attractPlayfieldPhase != AttractLastPlayfieldMode) {
    RPU_TurnOffAllLamps();
    AttractLastPlayfieldMode = attractPlayfieldPhase;
    if (attractPlayfieldPhase == 2) GameMode = GAME_MODE_SKILL_SHOT;
    else GameMode = GAME_MODE_UNSTRUCTURED_PLAY;
    AttractLastLadderBonus = 1;
    AttractLastLadderTime = CurrentTime;
  }

  ShowLampAnimation(attractPlayfieldPhase, 40, CurrentTime, 14, false, false);

  byte switchHit;
  while ( (switchHit = RPU_PullFirstFromSwitchStack()) != SWITCH_STACK_EMPTY ) {
    if (switchHit == SW_CREDIT_RESET) {
      if (AddPlayer(true)) returnState = MACHINE_STATE_INIT_GAMEPLAY;
    }
    if (switchHit == SW_COIN_1 || switchHit == SW_COIN_2 || switchHit == SW_COIN_3) {
      AddCoinToAudit(SwitchToChuteNum(switchHit));
      AddCoin(SwitchToChuteNum(switchHit));
    }
    if (switchHit == SW_SELF_TEST_SWITCH && (CurrentTime - GetLastSelfTestChangedTime()) > 250) {
      returnState = MACHINE_STATE_TEST_BOOT;
      SetLastSelfTestChangedTime(CurrentTime);
    }
  }

  return returnState;
}





////////////////////////////////////////////////////////////////////////////
//
//  Game Play functions
//
////////////////////////////////////////////////////////////////////////////
byte CountBits(unsigned short intToBeCounted) {
  byte numBits = 0;

  for (byte count = 0; count < 16; count++) {
    numBits += (intToBeCounted & 0x01);
    intToBeCounted = intToBeCounted >> 1;
  }

  return numBits;
}


void SetGameMode(byte newGameMode) {
  GameMode = newGameMode;
  GameModeStartTime = 0;
  GameModeEndTime = 0;
}

byte CountBallsInTrough() {
  // RPU_ReadSingleSwitchState(SW_OUTHOLE) +

  byte numBalls = RPU_ReadSingleSwitchState(SW_LEFT_BALL_RAMP) +
                  RPU_ReadSingleSwitchState(SW_CENTER_BALL_RAMP) +
                  RPU_ReadSingleSwitchState(SW_RIGHT_BALL_RAMP);

  return numBalls;
}



void AddToBonus(byte bonus) {
  Bonus[CurrentPlayer] += bonus;
  if (Bonus[CurrentPlayer] > MAX_DISPLAY_BONUS) {
    Bonus[CurrentPlayer] = MAX_DISPLAY_BONUS;
  } else {
    BonusAnimationStart = CurrentTime;
  }
}



void IncreaseBonusX() {

  if (BonusX[CurrentPlayer] < 10) {
    BonusX[CurrentPlayer] += 1;
    BonusXAnimationStart = CurrentTime;
    PlaySoundEffect(SOUND_EFFECT_BONUS_2X_AWARD + (BonusX[CurrentPlayer] - 2));
  }

}

unsigned long GameStartNotificationTime = 0;
boolean WaitForBallToReachOuthole = false;
unsigned long UpperBallEjectTime = 0;

int InitGamePlay(boolean curStateChanged) {
  RPU_TurnOffAllLamps();
  SetGeneralIlluminationOn(true);

  if (curStateChanged) {
    GameStartNotificationTime = CurrentTime;

    // Reset displays & game state variables
    for (int count = 0; count < 4; count++) {
      // Initialize game-specific variables
      BonusX[count] = 1;
      Bonus[count] = 1;
      SpinsTowardsNextGoal[count] = 0;
      SpinnerGoal[count] = 25;
      PlayerLockStatus[count] = 0;
      CombosAchieved[count] = 0;
      RPU_SetDisplayBlank(count, 0x00);
      CurrentScores[count] = 0;
      MagnaStatusLeft[count] = 0;
      MagnaStatusRight[count] = 0;
      SingleCombatLevelCompleted[count] = 0;
      DoubleCombatLevelCompleted[count] = 0;
      TripleCombatLevelCompleted[count] = 0;
      CombatJackpot[count] = 0; 
      DropTargetLevel[count] = 0;
      ExtraBallsOrSpecialAvailable[count] = 0;
      KingsChallengeStatus[count] = 0;

      LastChanceStatus[count] = 0;
      for (int i = 0; i < 4; i++) {
        NumDropTargetHits[count][i] = 0;
        NumDropTargetClears[count][i] = 0;
      }
    }

    SamePlayerShootsAgain = false;
    CurrentBallInPlay = 1;
    CurrentNumPlayers = 1;
    CurrentPlayer = 0;    
  }

  if (RPU_ReadSingleSwitchState(SW_SAUCER)) {
    if (SaucerEjectTime == 0 || CurrentTime > (SaucerEjectTime + 2500)) {
      RPU_PushToSolenoidStack(SOL_SAUCER, 12, true);
      SaucerEjectTime = CurrentTime;
    }
  }

  NumberOfBallsInPlay = 0;
  UpdateLockStatus();
  NumberOfBallsLocked = CountBits(MachineLocks & LOCKS_ENGAGED_MASK);

  if (NumberOfBallsLocked == TotalBallsLoaded) {
    // we have to kick a ball
    ReleaseLockedBall();
  }

  if (CountBallsInTrough() < (TotalBallsLoaded - NumberOfBallsLocked) /*|| RPU_ReadSingleSwitchState(SW_SHOOTER_LANE) */) {

    if (!RPU_ReadSingleSwitchState(SW_SHOOTER_LANE)) {
      MoveBallFromOutholeToRamp();
  
      if (CurrentTime > (GameStartNotificationTime + 5000)) {
        GameStartNotificationTime = CurrentTime;
        QueueNotification(SOUND_EFFECT_VP_BALL_MISSING, 10);
      }
  
      return MACHINE_STATE_INIT_GAMEPLAY;
    }
  }

  // The start button has been hit only once to get
  // us into this mode, so we assume a 1-player game
  // at the moment
  RPU_EnableSolenoidStack();
  RPU_SetCoinLockout((Credits >= MaximumCredits) ? true : false);

  return MACHINE_STATE_INIT_NEW_BALL;
}


int InitNewBall(bool curStateChanged) {

  // If we're coming into this mode for the first time
  // then we have to do everything to set up the new ball
  if (curStateChanged) {
    RPU_TurnOffAllLamps();
    BallFirstSwitchHitTime = 0;

    RPU_SetDisableFlippers(false);
    RPU_EnableSolenoidStack();
    RPU_SetDisplayCredits(Credits, !FreePlayMode);
    if (CurrentNumPlayers > 1 && (CurrentBallInPlay != 1 || CurrentPlayer != 0) && !SamePlayerShootsAgain) AlertPlayerUp(CurrentPlayer);
    SamePlayerShootsAgain = false;

    RPU_SetDisplayBallInPlay(CurrentBallInPlay);
    RPU_SetLampState(LAMP_HEAD_TILT, 0);

    if (BallSaveNumSeconds > 0) {
      RPU_SetLampState(LAMP_SHOOT_AGAIN, 1, 0, 500);
      RPU_SetLampState(LAMP_HEAD_SHOOT_AGAIN, 1, 0, 500);
    }

    BallSaveUsed = false;
    BallTimeInTrough = 0;
    NumTiltWarnings = 0;
    LastTiltWarningTime = 0;

    GIOverrideEndTime = 0;
    SetGeneralIlluminationOn();

    // Initialize game-specific start-of-ball lights & variables
    GameModeStartTime = 0;
    GameModeEndTime = 0;
    GameMode = GAME_MODE_SKILL_SHOT;

    for (byte count = 0; count < 4; count++) {
      DropTargetResetTime[count] = 0;
      DropTargetHurryTime[count] = 0;
      DropTargetHurryLamp[count] = false;
    }

    ExtraBallCollected = false;
    SpecialCollected = false;

    // If not held over, reset bonus & bonusx
    Bonus[CurrentPlayer] = 1;
    BonusX[CurrentPlayer] = 1;
    SpinnerGoal[CurrentPlayer] = 25;
    BonusXCollectAvailable = false;
    SpinsTowardsNextGoal[CurrentPlayer] = 0;
    BonusXCollectAvailableStart = 0;
    BonusXCollectReminder = 0;
    NumBonusXCollectReminders = 0;
    ExtraBallAwardStartTime = 0;
    KingsChallengeEndTime = 0;
    KingsChallengeRunning = 0;
    KingsChallengeKick = 0;
    LevitateMagnetOffTime = 0;
    LevitateMagnetOnTime = 0;
    LastTimeLeftMagnetOn = 0;
    LastTimeRightMagnetOn = 0;
    KingsChallengePerfectionBank = 0;

    PlayfieldMultiplier = 1;
    PlayfieldMultiplierExpiration = 0;
    ResetDisplayTrackingVariables();
    BonusXAnimationStart = 0;
    LastPopBumperHit = 0;
    LastSpinnerHit = 0;
    SpinnerLitUntil = 0;
    SpinnerStatus = 0;
    SpinnerPhase = 0;
    LoopLitToQualifyLock = false;
    BonusAnimationStart = 0;
    MagnaSaveAvailable = false;
    MagnaSoundOn = false;
    LockManagementInProgress = false;
    LastLoopHitTime = CurrentTime;
    TripleCombatJackpotsAvailable = 0;

    for (byte count = 0; count < 3; count++) {
      LockKickTime[count] = 0;
    }
    SaucerKickTime = 0;

    BallSaveEndTime = 0;
    IdleMode = IDLE_MODE_NONE;

    LastLeftInlane = 0;
    LastRightInlane = 0;

//    for (byte count = 0; count < NUM_BALL_SEARCH_SOLENOIDS; count++) {
//      BallSearchSolenoidFireTime[count] = 0;
//    }

    if (CurrentPlayer == 0) {
      // Only change skill shot on first ball of round.
      SkillShotTarget = CurrentTime % 3;
    }

    // Reset Drop Targets
    DropTargetsUL.ResetDropTargets(CurrentTime + 100, true);
    DropTargetsUR.ResetDropTargets(CurrentTime + 250, true);
    DropTargetsLL.ResetDropTargets(CurrentTime + 400, true);
    DropTargetsLR.ResetDropTargets(CurrentTime + 550, true);

    if (!RPU_ReadSingleSwitchState(SW_SHOOTER_LANE)) RPU_PushToTimedSolenoidStack(SOL_BALL_RAMP_THROWER, 16, CurrentTime + 1000);
    NumberOfBallsInPlay = 1;

    // Reset all combos
    for (byte count = 0; count < NumberOfComboDefinitions; count++) {
      BKCombos[count].currentStep = 0;
      BKCombos[count].currentTimeout = 0;
      if (BKCombos[count].trackingVariable != NULL) {
        *(BKCombos[count].trackingVariable) = false;
      }
    }

    // See if any locks have been stolen and move them from locked to availble
    UpdatePlayerLocks();
    byte rallyNum = CurrentBallInPlay - 1;
    if (BallsPerGame > 3) rallyNum = (CurrentBallInPlay > 1) ? ((CurrentBallInPlay == 5) ? 2 : 1) : 0;
    PlayBackgroundSong(SOUND_EFFECT_RALLY_SONG_1 + rallyNum);
  }

  //  if (CountBallsInTrough()==(TotalBallsLoaded-NumberOfBallsLocked)) {

  byte ballInShooter = RPU_ReadSingleSwitchState(SW_SHOOTER_LANE) ? 1 : 0;
  // We'll wait until we see the ball in the shooter lane
  if (!ballInShooter) {
    return MACHINE_STATE_INIT_NEW_BALL;
  } else {
    return MACHINE_STATE_NORMAL_GAMEPLAY;
  }

  LastTimeThroughLoop = CurrentTime;
}





void AnnounceStatus() {
  /*
        if (TicksCountedTowardsStatus > 68000) {
          IdleMode = IDLE_MODE_NONE;
          TicksCountedTowardsStatus = 0;
        } else if (TicksCountedTowardsStatus > 59000) {
          if (IdleMode != IDLE_MODE_BALL_SEARCH) {
            BallSearchSolenoidToTry = 0;
            BallSearchNextSolenoidTime = CurrentTime - 1;
          }
          if (CurrentTime > BallSearchNextSolenoidTime) {
            // Fire off a solenoid
            BallSearchSolenoidFireTime[BallSearchSolenoidToTry] = CurrentTime;
            RPU_PushToSolenoidStack(BallSearchSols[BallSearchSolenoidToTry], 10);
            BallSearchSolenoidToTry += 1;
            if (BallSearchSolenoidToTry >= NUM_BALL_SEARCH_SOLENOIDS) BallSearchSolenoidToTry = 0;
            BallSearchNextSolenoidTime = CurrentTime + 500;
          }
          IdleMode = IDLE_MODE_BALL_SEARCH;
        } else if (TicksCountedTowardsStatus > 52000) {
          if (WizardGoals[CurrentPlayer]&WIZARD_GOAL_SHIELD) {
            TicksCountedTowardsStatus = 59001;
          } else {
            if (IdleMode != IDLE_MODE_ADVERTISE_SHIELD) QueueNotification(SOUND_EFFECT_VP_ADVERTISE_SHIELD, 1);
            IdleMode = IDLE_MODE_ADVERTISE_SHIELD;
          }
        } else if (TicksCountedTowardsStatus > 45000) {
          if (WizardGoals[CurrentPlayer]&WIZARD_GOAL_SPINS) {
            TicksCountedTowardsStatus = 52001;
          } else {
            if (IdleMode != IDLE_MODE_ADVERTISE_SPINS) QueueNotification(SOUND_EFFECT_VP_ADVERTISE_SPINS, 1);
            IdleMode = IDLE_MODE_ADVERTISE_SPINS;
          }
        } else if (TicksCountedTowardsStatus > 38000) {
          if (WizardGoals[CurrentPlayer]&WIZARD_GOAL_7_NZ) {
            TicksCountedTowardsStatus = 45001;
          } else {
            if (IdleMode != IDLE_MODE_ADVERTISE_NZS) QueueNotification(SOUND_EFFECT_VP_ADVERTISE_NZS, 1);
            IdleMode = IDLE_MODE_ADVERTISE_NZS;
            ShowLampAnimation(0, 40, CurrentTime, 11, false, false);
            specialAnimationRunning = true;
          }
        } else if (TicksCountedTowardsStatus > 31000) {
          if (WizardGoals[CurrentPlayer]&WIZARD_GOAL_POP_BASES) {
            TicksCountedTowardsStatus = 38001;
          } else {
            if (IdleMode != IDLE_MODE_ADVERTISE_BASES) QueueNotification(SOUND_EFFECT_VP_ADVERTISE_BASES, 1);
            IdleMode = IDLE_MODE_ADVERTISE_BASES;
          }
        } else if (TicksCountedTowardsStatus > 24000) {
          if (WizardGoals[CurrentPlayer]&WIZARD_GOAL_COMBOS) {
            TicksCountedTowardsStatus = 31001;
          } else {
            if (IdleMode != IDLE_MODE_ADVERTISE_COMBOS) {
              byte countBits = CountBits(CombosAchieved[CurrentPlayer]);
              if (countBits==0) QueueNotification(SOUND_EFFECT_VP_ADVERTISE_COMBOS, 1);
              else if (countBits>0) QueueNotification(SOUND_EFFECT_VP_FIVE_COMBOS_LEFT+(countBits-1), 1);
            }
            IdleMode = IDLE_MODE_ADVERTISE_COMBOS;
          }
        } else if (TicksCountedTowardsStatus > 17000) {
          if (WizardGoals[CurrentPlayer]&WIZARD_GOAL_INVASION) {
            TicksCountedTowardsStatus = 24001;
          } else {
            if (IdleMode != IDLE_MODE_ADVERTISE_INVASION) QueueNotification(SOUND_EFFECT_VP_ADVERTISE_INVASION, 1);
            IdleMode = IDLE_MODE_ADVERTISE_INVASION;
          }
        } else if (TicksCountedTowardsStatus > 10000) {
          if (WizardGoals[CurrentPlayer]&WIZARD_GOAL_BATTLE) {
            TicksCountedTowardsStatus = 17001;
          } else {
            if (IdleMode != IDLE_MODE_ADVERTISE_BATTLE) QueueNotification(SOUND_EFFECT_VP_ADVERTISE_BATTLE, 1);
            IdleMode = IDLE_MODE_ADVERTISE_BATTLE;
          }
        } else if (TicksCountedTowardsStatus > 7000) {
          int goalCount = (int)(CountBits((WizardGoals[CurrentPlayer] & ~UsedWizardGoals[CurrentPlayer]))) + NumCarryWizardGoals[CurrentPlayer];
          if (GoalsUntilWizard==0) {
            TicksCountedTowardsStatus = 10001;
          } else {
            byte goalsRemaining = GoalsUntilWizard-(goalCount%GoalsUntilWizard);
            if (goalCount<0) goalsRemaining = (byte)(-1*goalCount);

            if (IdleMode != IDLE_MODE_ANNOUNCE_GOALS) {
              QueueNotification(SOUND_EFFECT_VP_ONE_GOAL_FOR_ENEMY-(goalsRemaining-1), 1);
              if (DEBUG_MESSAGES) {
                char buf[256];
                sprintf(buf, "Goals remaining = %d, Goals Until Wiz = %d, goalcount = %d, LO=%d, WizG=0x%04X\n", goalsRemaining, GoalsUntilWizard, goalCount, WizardGoals[CurrentPlayer], NumCarryWizardGoals[CurrentPlayer]);
                Serial.write(buf);
              }
            }
            IdleMode = IDLE_MODE_ANNOUNCE_GOALS;
            ShowLampAnimation(2, 40, CurrentTime, 11, false, false);
            specialAnimationRunning = true;
          }
        }
  */
}


boolean AddABall(boolean ballLocked = false, boolean ballSave = true) {
  if (NumberOfBallsInPlay >= TotalBallsLoaded) return false;

  if (ballLocked) {
    NumberOfBallsLocked += 1;
  } else {
    NumberOfBallsInPlay += 1;
    if (DEBUG_MESSAGES) {
      char buf[128];
      sprintf(buf, "Num BIP +1 to %d b/c AddABall\n", NumberOfBallsInPlay);
      Serial.write(buf);
    }
  }

  if (CountBallsInTrough()) {
    RPU_PushToTimedSolenoidStack(SOL_BALL_RAMP_THROWER, 16, CurrentTime + 100);
  } else {
    if (ballLocked) {
      if (NumberOfBallsInPlay) NumberOfBallsInPlay -= 1;
      if (DEBUG_MESSAGES) {
        char buf[128];
        sprintf(buf, "Num BIP minus 1 to %d b/c AddABall\n", NumberOfBallsInPlay);
        Serial.write(buf);
      }
    } else {
      return false;
    }
  }

  if (ballSave) {
    if (BallSaveEndTime) BallSaveEndTime += 10000;
    else BallSaveEndTime = CurrentTime + 20000;
  }

  return true;
}


void UpdateDropTargets() {
  DropTargetsLL.Update(CurrentTime);
  DropTargetsLR.Update(CurrentTime);
  DropTargetsUL.Update(CurrentTime);
  DropTargetsUR.Update(CurrentTime);

  for (byte count = 0; count < 4; count++) {

    DropTargetBank *curBank;
    if (count == 0) curBank = &DropTargetsUL;
    else if (count == 1) curBank = &DropTargetsUR;
    else if (count == 2) curBank = &DropTargetsLL;
    else if (count == 3) curBank = &DropTargetsLR;

    if (DropTargetResetTime[count] && curBank->GetStatus(false)) {
      if (CurrentTime > (DropTargetResetTime[count])) {
        DropTargetResetTime[count] = 0;
        curBank->ResetDropTargets(CurrentTime);
        if (count == 0) DropTargetsUL.ResetDropTargets(CurrentTime);
        if (count == 1) DropTargetsUR.ResetDropTargets(CurrentTime);
        if (count == 2) DropTargetsLL.ResetDropTargets(CurrentTime);
        if (count == 3) DropTargetsLR.ResetDropTargets(CurrentTime);
      } else {
        // Figure out if hurry lamp should be on (and if sound should play)
        if (CurrentTime > DropTargetHurryTime[count]) {
          unsigned long timeUntilNext = (DropTargetResetTime[count] - CurrentTime) / 32;
          if (timeUntilNext < 50) timeUntilNext = 50;
          DropTargetHurryTime[count] = CurrentTime + timeUntilNext;
          DropTargetHurryLamp[count] ^= 1;
          if (DropTargetHurryLamp[count]) PlaySoundEffect(SOUND_EFFECT_DROP_TARGET_HURRY);
        }
      }
    }
  }
}


void UpdateTimedKicks() {
  byte numTimedKicks = 0;
  boolean ballKicked = false;

  for (byte count = 0; count < 3; count++) {
    if (LockKickTime[count]) {
      if (CurrentTime > LockKickTime[count]) {
        LockKickTime[count] = 0;
        RPU_PushToSolenoidStack(SOL_UPPER_BALL_EJECT, 12, true);

        // Now we have to reset the upper lock switch debounce buffer
        // so that it registers if a new ball falls into it.
        for (int i = 2; i >= 0; i--) {
          if (UpperLockSwitchState[i]) {
            UpperLockSwitchDownTime[i] = 0;
            UpperLockSwitchState[i] = false;
            break;
          }
        }

        ballKicked = true;
        NumberOfBallsInPlay += 1;
        if (DEBUG_MESSAGES) {
          char buf[128];
          sprintf(buf, "Num BIP +1 to %d b/c UpdateTimedKicks\n", NumberOfBallsInPlay);
          Serial.write(buf);
        }
        //        if (NumberOfBallsLocked) NumberOfBallsLocked -= 1;
      }
      numTimedKicks += 1;
    }
  }

  if (numTimedKicks == 1 && ballKicked) {
    // We have kicked the last held ball, so the jackpot opportunity is gone
  }

  if (SaucerKickTime && CurrentTime > SaucerKickTime) {
    SaucerKickTime = 0;
    RPU_PushToSolenoidStack(SOL_SAUCER, 12, true);    
  }
}


byte ResetAllDropTargets(boolean onlyResetClearedBanks = false) {

  byte numBanksCleared = 0;

  if (DropTargetsUL.CheckIfBankCleared()) {
    if (onlyResetClearedBanks) DropTargetsUL.ResetDropTargets(CurrentTime + 200, true);
    numBanksCleared += 1;
  }
  if (DropTargetsUR.CheckIfBankCleared()) {
    if (onlyResetClearedBanks) DropTargetsUR.ResetDropTargets(CurrentTime + 400, true);
    numBanksCleared += 1;
  }
  if (DropTargetsLL.CheckIfBankCleared()) {
    if (onlyResetClearedBanks) DropTargetsLL.ResetDropTargets(CurrentTime + 600, true);
    numBanksCleared += 1;
  }
  if (DropTargetsLR.CheckIfBankCleared()) {
    if (onlyResetClearedBanks) DropTargetsLR.ResetDropTargets(CurrentTime + 800, true);
    numBanksCleared += 1;
  }


  if (!onlyResetClearedBanks) {
    if (DropTargetsUL.GetStatus(false)) DropTargetsUL.ResetDropTargets(CurrentTime + 200, true);
    if (DropTargetsUR.GetStatus(false)) DropTargetsUR.ResetDropTargets(CurrentTime + 400, true);
    if (DropTargetsLL.GetStatus(false)) DropTargetsLL.ResetDropTargets(CurrentTime + 600, true);
    if (DropTargetsLR.GetStatus(false)) DropTargetsLR.ResetDropTargets(CurrentTime + 800, true);
  }

  for (byte count = 0; count < 4; count++) {
    DropTargetResetTime[count] = 0;
  }

  return numBanksCleared;
}


void SetMagnetState(byte magnetNum, boolean magnetOn = true) {
  RPU_SetContinuousSolenoid(magnetOn, magnetNum ? SOL_RIGHT_MAGNA_SAVE : SOL_LEFT_MAGNA_SAVE);
}


byte GameModeStage;
boolean DisplaysNeedRefreshing = false;
boolean SawMagnetButonUp;
unsigned long LastTimePromptPlayed = 0;
unsigned short CurrentBattleLetterPosition = 0xFF;

#define NUM_GI_FLASH_SEQUENCE_ENTRIES 10
byte GIFlashIndex = 0;
unsigned long GIFlashTimer = 0;
unsigned int GIFlashChangeState[NUM_GI_FLASH_SEQUENCE_ENTRIES] = {1000, 1250, 2000, 2250, 3000, 3300, 3500, 3900, 5000, 6500};

// This function manages all timers, flags, and lights
int ManageGameMode() {
  int returnState = MACHINE_STATE_NORMAL_GAMEPLAY;

  boolean specialAnimationRunning = false;

  UpdateDropTargets();
  UpdateComboStatus();
  UpdateTimedKicks();

  if ((CurrentTime - LastSwitchHitTime) > 3000) TimersPaused = true;
  else TimersPaused = false;


  if (LastSpinnerHit && CurrentTime < (LastSpinnerHit + 3000)) {
    SpinnerPhase = 0;
  }
  if (CurrentTime > SpinnerLitUntil) {
    SpinnerLitUntil = 0;
    SpinnerStatus = 0;
  }

  if (LastPopBumperHit && CurrentTime > (LastPopBumperHit + 1000)) {
    LastPopBumperHit = 0;
  }

  if (GIOverrideEndTime && CurrentTime > GIOverrideEndTime) {
    GIOverrideEndTime = 0;
    SetGeneralIlluminationOn();
  }

  if (BonusXCollectReminder && CurrentTime > BonusXCollectReminder) {
    RemindBonusXCollect();
  }

  if (ExtraBallAwardStartTime && CurrentTime > (ExtraBallAwardStartTime + 5000)) {
    ExtraBallAwardStartTime = 0;
  }

  if (LevitateMagnetOnTime && (CurrentTime > LevitateMagnetOnTime)) {
    LevitateMagnetOnTime = 0;
    LevitateMagnetOffTime = CurrentTime + 1250;
    SetMagnetState(0, true);
    PlaySoundEffect(SOUND_EFFECT_LEVITATE);
  }

  if (LevitateMagnetOffTime && (CurrentTime > LevitateMagnetOffTime)) {
    if (KingsChallengeRunning & KINGS_CHALLENGE_LEVITATE) {
      LevitateMagnetOnTime = CurrentTime + 1000;
    }
    LevitateMagnetOffTime = 0;
    SetMagnetState(0, false);
    Audio.StopSound(SOUND_EFFECT_LEVITATE);
  }

  if (LastTimeLeftMagnetOn && CurrentTime>(LastTimeLeftMagnetOn+2000)) {
    LastTimeLeftMagnetOn = 0;
  }

  if (LastTimeRightMagnetOn && CurrentTime>(LastTimeRightMagnetOn+2000)) {
    LastTimeRightMagnetOn = 0;
  }

  boolean waitingForKick = false;
  if (LockKickTime[0] || LockKickTime[1] || LockKickTime[2]) waitingForKick = true;


  if (MagnaSaveAvailable) {
    boolean neitherMagnetOn = true;

    // manage magnets on/off
    unsigned long timeOnDelta = CurrentTime - LastTimeThroughLoop;
    if (RPU_ReadSingleSwitchState(SW_LEFT_MAGNET_BUTTON)) {
      if (MagnaStatusLeft[CurrentPlayer]) {
        if (timeOnDelta > MagnaStatusLeft[CurrentPlayer]) {
          MagnaStatusLeft[CurrentPlayer] = 0;
        } else {
          MagnaStatusLeft[CurrentPlayer] -= timeOnDelta;
        }
        LastTimeLeftMagnetOn = CurrentTime;
        SetMagnetState(0, true);
        neitherMagnetOn = false;
        if (!MagnaSoundOn) {
          PlaySoundEffect(SOUND_EFFECT_MAGNET);
          MagnaSoundOn = true;
        }
      } else {
        SetMagnetState(0, false);
      }
    } else {
      if (LevitateMagnetOffTime==0) SetMagnetState(0, false);
    }

    if (RPU_ReadSingleSwitchState(SW_RIGHT_MAGNET_BUTTON)) {
      if (MagnaStatusRight[CurrentPlayer]) {
        if (timeOnDelta > MagnaStatusRight[CurrentPlayer]) {
          MagnaStatusRight[CurrentPlayer] = 0;
        } else {
          MagnaStatusRight[CurrentPlayer] -= timeOnDelta;
        }
        LastTimeRightMagnetOn = CurrentTime;
        SetMagnetState(1, true);
        neitherMagnetOn = false;
        if (!MagnaSoundOn) {
          PlaySoundEffect(SOUND_EFFECT_MAGNET);
          MagnaSoundOn = true;
        }
      } else {
        SetMagnetState(1, false);
      }
    } else {
      SetMagnetState(1, false);
    }

    if (neitherMagnetOn && MagnaSoundOn) {
      MagnaSoundOn = false;
      Audio.StopSound(SOUND_EFFECT_MAGNET);
    }
  } else {
    SetMagnetState(0, false);
    SetMagnetState(1, false);
    if (MagnaSoundOn) {
      MagnaSoundOn = false;
      Audio.StopSound(SOUND_EFFECT_MAGNET);
    }
  }


  switch ( GameMode ) {
    case GAME_MODE_WAIT_FOR_BALL_TO_RETURN:
      if (GameModeStartTime == 0) {
        GameModeStartTime = CurrentTime;
        RPU_SetDisableFlippers();
        SetGeneralIlluminationOn(false);
        MagnaSaveAvailable = false;

        // Tell the machine that we haven't started the ball yet
        BallFirstSwitchHitTime = 0;
      }
      if (CountBallsInTrough()) {
        PutBallInPlay();
        SetGameMode(GAME_MODE_BALL_IN_SHOOTER_LANE);
      }
      break;
    case GAME_MODE_BALL_IN_SHOOTER_LANE:
      if (GameModeStartTime == 0) {
        GameModeStartTime = CurrentTime;
        SetGeneralIlluminationOn(true);
        RPU_SetDisableFlippers(false);
        MagnaSaveAvailable = false;
      }

      if (RPU_ReadSingleSwitchState(SW_SHOOTER_LANE)) {
        SetGameMode(WaitingForBallGameMode);
      }
      break;
    case GAME_MODE_SKILL_SHOT:
      if (GameModeStartTime == 0) {
        GameModeStartTime = CurrentTime;
        GameModeEndTime = 0;
        LastTimePromptPlayed = CurrentTime;
        GameModeStage = 0;
        SetGeneralIlluminationOn(true);
        MagnaSaveAvailable = false;
      }

      if (GameModeStage == 0 && RPU_ReadSingleSwitchState(SW_SHOOTER_LANE)) {
        GameModeStage = 1;
      } else if (GameModeStage == 1 && !RPU_ReadSingleSwitchState(SW_SHOOTER_LANE)) {
        //PlaySoundEffect(SOUND_EFFECT_LEFT_SHOOTER_LANE);
        Audio.PlayBackgroundSong(SOUND_EFFECT_RALLY_PLUNGE);
        GameModeStage = 2;
      } else if (GameModeStage == 2 && RPU_ReadSingleSwitchState(SW_SHOOTER_LANE)) {
        GameModeStage = 1;
        //PlaySoundEffect(SOUND_EFFECT_RETURN_TO_SHOOTER_LANE);
        byte rallyNum = CurrentBallInPlay - 1;
        if (BallsPerGame > 3) rallyNum = (CurrentBallInPlay > 1) ? ((CurrentBallInPlay == 5) ? 2 : 1) : 0;
        PlayBackgroundSong(SOUND_EFFECT_RALLY_SONG_1 + rallyNum);
      }

      if (BallFirstSwitchHitTime != 0) {
        SetGameMode(GAME_MODE_UNSTRUCTURED_PLAY);
        PlaySoundEffect(SOUND_EFFECT_HORSE_NEIGHING);
        byte songToPlay;
        if (BallsPerGame > 3) {
          songToPlay = (((CurrentBallInPlay - 1) * 2) + CurrentTime % 2) % NUM_BACKGROUND_SONGS;
        } else {
          songToPlay = (((CurrentBallInPlay - 1) * 3) + CurrentTime % 5) % NUM_BACKGROUND_SONGS;
        }
        PlayBackgroundSong(SOUND_EFFECT_BACKGROUND_SONG_1 + songToPlay);
      }
      break;

    case GAME_MODE_UNSTRUCTURED_PLAY:
      // If this is the first time in this mode
      if (GameModeStartTime == 0) {
        GameModeStartTime = CurrentTime;
        DisplaysNeedRefreshing = true;
        TicksCountedTowardsStatus = 0;
        IdleMode = IDLE_MODE_NONE;
        if (DEBUG_MESSAGES) {
          Serial.write("Entering unstructured play\n");
        }
        SetGeneralIlluminationOn(true);
        MagnaSaveAvailable = true;
        if (DEBUG_MESSAGES) {
          char buf[128];
          sprintf(buf, "Unstructured: Player lock = 0x%02X\n", PlayerLockStatus[CurrentPlayer]);
          Serial.write(buf);
        }
        GameModeStage = 0;
      }

      if (TimersPaused && IdleModeEnabled) {
        TicksCountedTowardsStatus += (CurrentTime - LastTimeThroughLoop);
        AnnounceStatus();
      } else {
        TicksCountedTowardsStatus = 0;
        IdleMode = IDLE_MODE_NONE;
      }
      
      if (PlayfieldMultiplierExpiration) {
        // Playfield X value is only reset during unstructured play
        if (CurrentTime > PlayfieldMultiplierExpiration) {
          PlayfieldMultiplierExpiration = 0;
          if (PlayfieldMultiplier > 1) QueueNotification(SOUND_EFFECT_VP_RETURN_TO_1X, 1);
          PlayfieldMultiplier = 1;
        } else {
          for (byte count = 0; count < 4; count++) {
            if (count != CurrentPlayer) OverrideScoreDisplay(count, PlayfieldMultiplier, DISPLAY_OVERRIDE_SYMMETRIC_BOUNCE);
          }
          DisplaysNeedRefreshing = true;
        }
      } else if (DisplaysNeedRefreshing) {
        DisplaysNeedRefreshing = false;
        ShowPlayerScores(0xFF, false, false);
      }

      if (KingsChallengeEndTime) {
        // King's Challenge only expires during unstructured play
        if (CurrentTime>KingsChallengeEndTime) {
          // Payoff Bonus
          StartScoreAnimation(KingsChallengeBonus * PlayfieldMultiplier);

          KingsChallengeStatus[CurrentPlayer] |= (KingsChallengeRunning * 0x10);
          KingsChallengeEndTime = 0;
          KingsChallengeRunning = 0;
          ShowPlayerScores(0xFF, false, false);
        } else {
          if ( (KingsChallengeEndTime - CurrentTime) < 10000 ) {
            if (GameModeStage==0) {
              GameModeStage = 1;
              PlaySoundEffect(SOUND_EFFECT_10_SECONDS_LEFT);
            }
            byte timeLeft = (KingsChallengeEndTime - CurrentTime) / 1000;
            OverrideScoreDisplay(CurrentPlayer, timeLeft, DISPLAY_OVERRIDE_ANIMATION_CENTER);
          }

          if (KingsChallengeBonusChangedTime) {
            if (CurrentTime>(KingsChallengeBonusChangedTime+3000)) {
              KingsChallengeBonusChangedTime = 0;
            } else {
              for (byte count = 0; count < 4; count++) {
                if (count != CurrentPlayer) OverrideScoreDisplay(count, KingsChallengeBonus, DISPLAY_OVERRIDE_ANIMATION_FLUTTER);
              }
              DisplaysNeedRefreshing = true;
            }
          }
        }
      }

      break;
    case GAME_MODE_OFFER_SINGLE_COMBAT:
      if (GameModeStartTime == 0) {
        GameModeStartTime = CurrentTime;
        GameModeEndTime = CurrentTime + 8000;
        if (SingleCombatLevelCompleted[CurrentPlayer] == 0) {
          QueueNotification(SOUND_EFFECT_VP_PRESS_FOR_SINGLE, 10);
        } else if (SingleCombatLevelCompleted[CurrentPlayer] == 1) {
          QueueNotification(SOUND_EFFECT_VP_PRESS_FOR_SINGLE_PART_2, 10);
        } else if (SingleCombatLevelCompleted[CurrentPlayer] == 2) {
          QueueNotification(SOUND_EFFECT_VP_PRESS_FOR_SINGLE_PART_3, 10);
        } else if (SingleCombatLevelCompleted[CurrentPlayer] == 3) {
          QueueNotification(SOUND_EFFECT_VP_PRESS_FOR_SINGLE, 10);
        }
        MagnaSaveAvailable = false;
        if (DEBUG_MESSAGES) Serial.write("Offer single combat\n");
      }

      for (byte count = 0; count < 4; count++) {
        if (count != CurrentPlayer) OverrideScoreDisplay(count, (GameModeEndTime - CurrentTime) / 1000, DISPLAY_OVERRIDE_ANIMATION_CENTER);
      }

      if (CurrentTime > GameModeEndTime) {
        ShowPlayerScores(0xFF, false, false);
        QueueNotification(SOUND_EFFECT_VP_BALL_LOCKED, 8);
        LockBall();
        LockManagementInProgress = false;
        if (PutBallInPlay()) {
          WaitingForBallGameMode = GAME_MODE_UNSTRUCTURED_PLAY;
          SetGameMode(GAME_MODE_BALL_IN_SHOOTER_LANE);
        } else {
          // Instruct player to wait for ball to come back
          WaitingForBallGameMode = GAME_MODE_UNSTRUCTURED_PLAY;
          SetGameMode(GAME_MODE_WAIT_FOR_BALL_TO_RETURN);
        }
      } else if ( RPU_ReadSingleSwitchState(SW_LEFT_MAGNET_BUTTON) && RPU_ReadSingleSwitchState(SW_RIGHT_MAGNET_BUTTON) ) {
        ShowPlayerScores(0xFF, false, false);
        SetGameMode(GAME_MODE_SINGLE_COMBAT_START);
        //        RemoveTopQualifiedFlag();
        PlayerLockStatus[CurrentPlayer] = (PlayerLockStatus[CurrentPlayer] & LOCKS_AVAILABLE_MASK) / 2;
      }

      break;

    case GAME_MODE_SINGLE_COMBAT_START:
      if (GameModeStartTime == 0) {
        Audio.StopAllMusic();
        MagnaSoundOn = false;
        GameModeStartTime = CurrentTime;
        SetGeneralIlluminationOn(true);
        RPU_SetDisableFlippers(false);
        GameModeStage = 0;
        CombatJackpotReady = false;
        CombatSuperJackpotReady = false;

        // Announce rules for single combat
        Audio.StopCurrentNotification();

        if (SingleCombatLevelCompleted[CurrentPlayer] == 0) {
          QueueNotification(SOUND_EFFECT_VP_SINGLE_HINT_0, 10);
        } else if (SingleCombatLevelCompleted[CurrentPlayer] == 1) {
          QueueNotification(SOUND_EFFECT_VP_SINGLE_HINT_PART_1, 10);
        } else {
          QueueNotification(SOUND_EFFECT_VP_SINGLE_HINT_PART_2, 10);
        }
        GameModeEndTime = CurrentTime + 4000;
        ResetAllDropTargets();
        MagnaSaveAvailable = false;
        SawMagnetButonUp = false;
      }

      if ( RPU_ReadSingleSwitchState(SW_LEFT_MAGNET_BUTTON) && RPU_ReadSingleSwitchState(SW_RIGHT_MAGNET_BUTTON) ) {
        if (SawMagnetButonUp) {
          Audio.StopAllNotifications();
          GameModeEndTime = CurrentTime - 1;
        }
      } else {
        SawMagnetButonUp = true;
      }

      if (CurrentTime > GameModeEndTime) {
        QueueNotification(SOUND_EFFECT_VP_SINGLE_COMBAT, 8);
        switch (SingleCombatNumSeconds) {
          case 30: QueueNotification(SOUND_EFFECT_VP_30_SECONDS, 8); break;
          case 45: QueueNotification(SOUND_EFFECT_VP_45_SECONDS, 8); break;
          case 60: QueueNotification(SOUND_EFFECT_VP_60_SECONDS, 8); break;
          case 75: QueueNotification(SOUND_EFFECT_VP_75_SECONDS, 8); break;
          case 90: QueueNotification(SOUND_EFFECT_VP_90_SECONDS, 8); break;
          case 120: QueueNotification(SOUND_EFFECT_VP_120_SECONDS, 8); break;
        }
        SetGameMode(GAME_MODE_SINGLE_COMBAT);
      }

      break;

    case GAME_MODE_SINGLE_COMBAT:
      if (GameModeStartTime == 0) {
        GameModeStartTime = CurrentTime;
        GameModeEndTime = CurrentTime + ((unsigned long)SingleCombatNumSeconds) * 1000;
        PlayBackgroundSong(SOUND_EFFECT_BATTLE_SONG_1);
        MagnaSaveAvailable = true;
        LockKickTime[0] = CurrentTime + 100;
        LockManagementInProgress = false;
        NumberOfBallsInPlay = 0;
        JackpotIncreasedTime = 0;
        if (DEBUG_MESSAGES) {
          char buf[128];
          sprintf(buf, "Single: Player lock = 0x%02X\n", PlayerLockStatus[CurrentPlayer]);
          Serial.write(buf);
        }
        GameModeStage = 0;
        if (BallSaveOnCombatModes) {
          BallSaveEndTime = CurrentTime + ((unsigned long)BallSaveOnCombatModes * 1000);
        }
      }

      if (JackpotIncreasedTime) {
        if (CurrentTime > (JackpotIncreasedTime + 2000)) {
          JackpotIncreasedTime = 0;
        } else {
          for (byte count = 0; count < 4; count++) {
            if (count != CurrentPlayer) OverrideScoreDisplay(count, CombatJackpot[CurrentPlayer], DISPLAY_OVERRIDE_ANIMATION_FLUTTER);
          }
        }
      } else {
        for (byte count = 0; count < 4; count++) {
          if (count != CurrentPlayer) OverrideScoreDisplay(count, (GameModeEndTime - CurrentTime) / 1000, DISPLAY_OVERRIDE_ANIMATION_CENTER);
        }
      }

      if ( (CurrentTime + 11000) > GameModeEndTime && GameModeStage == 0) {
        GameModeStage = 1;
        if (!CombatJackpotReady) {
          PlaySoundEffect(SOUND_EFFECT_10_SECONDS_LEFT);
        } else {
          QueueNotification(SOUND_EFFECT_VP_SINGLE_TEN_SECONDS_TO_HIT_SAUCER, 8);
        }
      }

      if (CurrentTime > GameModeEndTime) {
        SetGameMode(GAME_MODE_SINGLE_COMBAT_LOST);
      }
      break;

    case GAME_MODE_SINGLE_COMBAT_WON:
      if (GameModeStartTime == 0) {
        GameModeStartTime = CurrentTime;
        GameModeEndTime = CurrentTime + 3000;
        Audio.StopAllMusic();
        PlaySoundEffect(SOUND_EFFECT_CROWD_CHEERING);
        if (!SaucerLitFromCombo) {
          QueueNotification(SOUND_EFFECT_VP_JACKPOT, 8);
          CombatJackpot[CurrentPlayer] += COMBAT_JACKPOT_BASE_1;
          StartScoreAnimation(PlayfieldMultiplier * CombatJackpot[CurrentPlayer]);
        } else if (LeftInlaneLitFromLoop) {
          QueueNotification(SOUND_EFFECT_VP_MEGA_JACKPOT, 8);
          CombatJackpot[CurrentPlayer] += COMBAT_JACKPOT_BASE_1*10;
          StartScoreAnimation(PlayfieldMultiplier * CombatJackpot[CurrentPlayer]);
        } else {
          QueueNotification(SOUND_EFFECT_VP_DOUBLE_JACKPOT, 8);
          CombatJackpot[CurrentPlayer] += COMBAT_JACKPOT_BASE_1*2;
          StartScoreAnimation(PlayfieldMultiplier * CombatJackpot[CurrentPlayer]);
        } 

        ShowPlayerScores(0xFF, false, false);
        GameModeStage = SingleCombatLevelCompleted[CurrentPlayer];
        if (SingleCombatLevelCompleted[CurrentPlayer] < 3) {
          SingleCombatLevelCompleted[CurrentPlayer] += 1;
        }

      }

      if (CurrentTime > GameModeEndTime) {
        RPU_PushToSolenoidStack(SOL_SAUCER, 16, true);
        PlayBackgroundSong(SOUND_EFFECT_BACKGROUND_SONG_2 + SingleCombatLevelCompleted[CurrentPlayer]);
        if (SingleCombatLevelCompleted[CurrentPlayer] > GameModeStage) {
          if (SingleCombatLevelCompleted[CurrentPlayer] == 1) QueueNotification(SOUND_EFFECT_VP_SINGLE_COMBAT_PART_1_COMPLETE, 8);
          if (SingleCombatLevelCompleted[CurrentPlayer] == 2) QueueNotification(SOUND_EFFECT_VP_SINGLE_COMBAT_PART_2_COMPLETE, 8);
          if (SingleCombatLevelCompleted[CurrentPlayer] == 3) QueueNotification(SOUND_EFFECT_VP_SINGLE_COMBAT_PART_3_COMPLETE, 8);
        }
        ResetAllDropTargets();
        SetGameMode(GAME_MODE_UNSTRUCTURED_PLAY);
      }
      break;

    case GAME_MODE_SINGLE_COMBAT_LOST:
      if (GameModeStartTime == 0) {
        GameModeStartTime = CurrentTime;
        GameModeEndTime = CurrentTime + 3000;
        Audio.StopAllMusic();
        PlaySoundEffect(SOUND_EFFECT_BOOING_3);
        ShowPlayerScores(0xFF, false, false);
      }

      if (CurrentTime > GameModeEndTime) {
        ResetAllDropTargets();
        PlayBackgroundSong(SOUND_EFFECT_BACKGROUND_SONG_2);
        SetGameMode(GAME_MODE_UNSTRUCTURED_PLAY);
        QueueNotification(SOUND_EFFECT_VP_SINGLE_LOST, 8);
      }
      break;

    case GAME_MODE_OFFER_DOUBLE_COMBAT:
      if (GameModeStartTime == 0) {
        GameModeStartTime = CurrentTime;
        GameModeEndTime = CurrentTime + 8000;
        if (DoubleCombatLevelCompleted[CurrentPlayer] == 0) {
          QueueNotification(SOUND_EFFECT_VP_PRESS_FOR_DOUBLE, 10);
        } else if (DoubleCombatLevelCompleted[CurrentPlayer] == 1) {
          QueueNotification(SOUND_EFFECT_VP_PRESS_FOR_DOUBLE_PART_2, 10);
        } else if (DoubleCombatLevelCompleted[CurrentPlayer] == 2) {
          QueueNotification(SOUND_EFFECT_VP_PRESS_FOR_DOUBLE_PART_3, 10);
        } else {
          QueueNotification(SOUND_EFFECT_VP_PRESS_FOR_DOUBLE, 10);
        }
        MagnaSaveAvailable = false;
      }

      for (byte count = 0; count < 4; count++) {
        if (count != CurrentPlayer) OverrideScoreDisplay(count, (GameModeEndTime - CurrentTime) / 1000, DISPLAY_OVERRIDE_ANIMATION_CENTER);
      }

      if (CurrentTime > GameModeEndTime) {
        QueueNotification(SOUND_EFFECT_VP_BALL_LOCKED, 8);
        LockBall();
        LockManagementInProgress = false;
        if (PutBallInPlay()) {
          WaitingForBallGameMode = GAME_MODE_UNSTRUCTURED_PLAY;
          SetGameMode(GAME_MODE_BALL_IN_SHOOTER_LANE);
        } else {
          // Instruct player to wait for ball to come back
          WaitingForBallGameMode = GAME_MODE_UNSTRUCTURED_PLAY;
          SetGameMode(GAME_MODE_WAIT_FOR_BALL_TO_RETURN);
        }
      } else if ( RPU_ReadSingleSwitchState(SW_LEFT_MAGNET_BUTTON) && RPU_ReadSingleSwitchState(SW_RIGHT_MAGNET_BUTTON) ) {
        //        RemoveTopQualifiedFlag();
        //        RemoveTopQualifiedFlag();
        PlayerLockStatus[CurrentPlayer] = (PlayerLockStatus[CurrentPlayer] & LOCKS_AVAILABLE_MASK) / 4;
        SetGameMode(GAME_MODE_DOUBLE_COMBAT_START);
        ShowPlayerScores(0xFF, false, false);
      }

      break;

    case GAME_MODE_DOUBLE_COMBAT_START:
      if (GameModeStartTime == 0) {
        Audio.StopAllMusic();
        GameModeStartTime = CurrentTime;
        SetGeneralIlluminationOn(true);
        RPU_SetDisableFlippers(false);
        GameModeStage = 0;
        CombatBankFlags = 0;
        CombatJackpotReady = false;
        CombatSuperJackpotReady = false;
        CombatJackpot[CurrentPlayer] += COMBAT_JACKPOT_BASE_2;
        JackpotBeforeCombat = CombatJackpot[CurrentPlayer];

        // Announce rules for single combat
        Audio.StopCurrentNotification();
        if (DoubleCombatLevelCompleted[CurrentPlayer] == 0) {
          QueueNotification(SOUND_EFFECT_VP_DOUBLE_HINT_PART_1, 10);
        } else if (DoubleCombatLevelCompleted[CurrentPlayer] == 1) {
          QueueNotification(SOUND_EFFECT_VP_DOUBLE_HINT_2_OR_3, 10);
        } else if (DoubleCombatLevelCompleted[CurrentPlayer] == 2) {
          QueueNotification(SOUND_EFFECT_VP_DOUBLE_HINT_2_OR_3, 10);
        } else {
          QueueNotification(SOUND_EFFECT_VP_DOUBLE_HINT_PART_1, 10);
        }
        GameModeEndTime = CurrentTime + 4000;
        ResetAllDropTargets();
        MagnaSaveAvailable = false;
        SawMagnetButonUp = false;
      }

      if ( RPU_ReadSingleSwitchState(SW_LEFT_MAGNET_BUTTON) && RPU_ReadSingleSwitchState(SW_RIGHT_MAGNET_BUTTON) ) {
        if (SawMagnetButonUp) {
          Audio.StopAllNotifications();
          GameModeEndTime = CurrentTime;
        }
      } else {
        SawMagnetButonUp = true;
      }

      if (CurrentTime >= GameModeEndTime) {
        SetGameMode(GAME_MODE_DOUBLE_COMBAT);
      }

      break;

    case GAME_MODE_DOUBLE_COMBAT:
      if (GameModeStartTime == 0) {
        GameModeStartTime = CurrentTime;
        PlayBackgroundSong(SOUND_EFFECT_BATTLE_SONG_1 + 5);
        MagnaSaveAvailable = true;
        LockKickTime[0] = CurrentTime + 100;
        LockKickTime[1] = CurrentTime + 1100;
        if (NumberOfBallsLocked) NumberOfBallsLocked -= 1;
        LockManagementInProgress = false;
        waitingForKick = true;
        NumberOfBallsInPlay = 0;
        if (BallSaveOnCombatModes) {
          BallSaveEndTime = CurrentTime + ((unsigned long)BallSaveOnCombatModes * 1000);
        }
      }

      if (!waitingForKick && NumberOfBallsInPlay <= 1) {
        if (JackpotBeforeCombat != CombatJackpot[CurrentPlayer]) {
          if (DoubleCombatLevelCompleted[CurrentPlayer]==0) SetGameMode(GAME_MODE_DOUBLE_COMBAT_FIRST_WIN);
          else SetGameMode(GAME_MODE_DOUBLE_COMBAT_LEVEL_INCREASED);
        } else {
          // Double combat over but they have a jackpot, they either tied or increased
          if (DoubleCombatLevelCompleted[CurrentPlayer]==0) SetGameMode(GAME_MODE_DOUBLE_COMBAT_LOST);
          else SetGameMode(GAME_MODE_DOUBLE_COMBAT_LEVEL_SAME);
        }
      }
      break;

    case GAME_MODE_DOUBLE_COMBAT_LOST:
      if (GameModeStartTime == 0) {
        GameModeStartTime = CurrentTime;
        GameModeEndTime = CurrentTime + 3000;
        QueueNotification(SOUND_EFFECT_VP_DOUBLE_LOST, 7);
      }

      if (CurrentTime > GameModeEndTime) {
        ResetAllDropTargets();
        PlayBackgroundSong(SOUND_EFFECT_BACKGROUND_SONG_2);
        SetGameMode(GAME_MODE_UNSTRUCTURED_PLAY);
      }
      break;

    case GAME_MODE_DOUBLE_COMBAT_FIRST_WIN:
      if (GameModeStartTime == 0) {
        GameModeStartTime = CurrentTime;
        GameModeEndTime = CurrentTime + 3000;
        QueueNotification(SOUND_EFFECT_VP_DOUBLE_COMBAT_FIRST_VICTORY, 7);
        DoubleCombatLevelCompleted[CurrentPlayer] += 1;
      }
      if (CurrentTime > GameModeEndTime) {
        ResetAllDropTargets();
        PlayBackgroundSong(SOUND_EFFECT_BACKGROUND_SONG_2);
        SetGameMode(GAME_MODE_UNSTRUCTURED_PLAY);
      }
      break;

    case GAME_MODE_DOUBLE_COMBAT_LEVEL_INCREASED:
      if (GameModeStartTime == 0) {
        GameModeStartTime = CurrentTime;
        GameModeEndTime = CurrentTime + 3000;
        QueueNotification(SOUND_EFFECT_VP_DOUBLE_COMBAT_LEVEL_INCREASED, 7);
        DoubleCombatLevelCompleted[CurrentPlayer] += 1;
      }
      if (CurrentTime > GameModeEndTime) {
        ResetAllDropTargets();
        PlayBackgroundSong(SOUND_EFFECT_BACKGROUND_SONG_2);
        SetGameMode(GAME_MODE_UNSTRUCTURED_PLAY);
      }
      break;

    case GAME_MODE_DOUBLE_COMBAT_LEVEL_SAME:
      if (GameModeStartTime == 0) {
        GameModeStartTime = CurrentTime;
        GameModeEndTime = CurrentTime + 3000;
        QueueNotification(SOUND_EFFECT_VP_DOUBLE_COMBAT_LEVEL_SAME, 7);
      }
      if (CurrentTime > GameModeEndTime) {
        ResetAllDropTargets();
        PlayBackgroundSong(SOUND_EFFECT_BACKGROUND_SONG_2);
        SetGameMode(GAME_MODE_UNSTRUCTURED_PLAY);
      }
      break;

    case GAME_MODE_TRIPLE_COMBAT_START:
      if (GameModeStartTime == 0) {
        Audio.StopAllMusic();
        GameModeStartTime = CurrentTime;
        SetGeneralIlluminationOn(true);
        RPU_SetDisableFlippers(false);
        GameModeStage = 0;
        CombatBankFlags = 0;
        TripleCombatJackpotsAvailable = 0;
        CombatJackpot[CurrentPlayer] += COMBAT_JACKPOT_BASE_3;
        JackpotBeforeCombat = CombatJackpot[CurrentPlayer];

        // Announce rules for single combat
        Audio.StopCurrentNotification();
        if (TripleCombatLevelCompleted[CurrentPlayer] == 0) {
          QueueNotification(SOUND_EFFECT_VP_TRIPLE_HINT_0, 10);
        } else if (TripleCombatLevelCompleted[CurrentPlayer] == 1) {
          QueueNotification(SOUND_EFFECT_VP_TRIPLE_HINT_1, 10);
        } else if (TripleCombatLevelCompleted[CurrentPlayer] == 2) {
          QueueNotification(SOUND_EFFECT_VP_TRIPLE_HINT_2, 10);
        } else {
          QueueNotification(SOUND_EFFECT_VP_TRIPLE_HINT_3, 10);
        }
        GameModeEndTime = CurrentTime + 4000;
        ResetAllDropTargets();
        MagnaSaveAvailable = false;
        SawMagnetButonUp = false;
      }

      if ( RPU_ReadSingleSwitchState(SW_LEFT_MAGNET_BUTTON) && RPU_ReadSingleSwitchState(SW_RIGHT_MAGNET_BUTTON) ) {
        if (SawMagnetButonUp) {
          Audio.StopAllNotifications();
          GameModeEndTime = CurrentTime;
        }
      } else {
        SawMagnetButonUp = true;
      }

      if (CurrentTime >= GameModeEndTime) {
        SetGameMode(GAME_MODE_DOUBLE_COMBAT);
      }

      break;

    case GAME_MODE_TRIPLE_COMBAT:
      if (GameModeStartTime == 0) {
        GameModeStartTime = CurrentTime;
        PlayBackgroundSong(SOUND_EFFECT_BATTLE_SONG_1 + 2);
        MagnaSaveAvailable = true;
        LockKickTime[0] = CurrentTime + 100;
        LockKickTime[1] = CurrentTime + 1100;
        LockKickTime[2] = CurrentTime + 2100;
        LockManagementInProgress = false;
        PlayerLockStatus[CurrentPlayer] = 0;
        NumberOfBallsLocked = 0;
        NumberOfBallsInPlay = 0;
        waitingForKick = true;
        if (BallSaveOnCombatModes) {
          BallSaveEndTime = CurrentTime + ((unsigned long)BallSaveOnCombatModes * 1000);
        }
      }

      if (!waitingForKick && NumberOfBallsInPlay <= 1) {
        SetGameMode(GAME_MODE_TRIPLE_COMBAT_LOST);
      }
      break;

    case GAME_MODE_TRIPLE_COMBAT_LOST:
      if (GameModeStartTime == 0) {
        GameModeStartTime = CurrentTime;
        GameModeEndTime = CurrentTime + 3000;
        QueueNotification(SOUND_EFFECT_VP_TRIPLE_LOST, 7);
      }

      if (CurrentTime > GameModeEndTime) {
        ResetAllDropTargets();
        SetGameMode(GAME_MODE_UNSTRUCTURED_PLAY);
      }
      break;

    case GAME_MODE_TRIPLE_COMBAT_FIRST_WIN:
      if (GameModeStartTime == 0) {
        GameModeStartTime = CurrentTime;
        GameModeEndTime = CurrentTime + 3000;
        QueueNotification(SOUND_EFFECT_VP_TRIPLE_COMBAT_FIRST_VICTORY, 7);
        DoubleCombatLevelCompleted[CurrentPlayer] += 1;
      }
      if (CurrentTime > GameModeEndTime) {
        ResetAllDropTargets();
        PlayBackgroundSong(SOUND_EFFECT_BACKGROUND_SONG_3);
        SetGameMode(GAME_MODE_UNSTRUCTURED_PLAY);
      }
      break;

    case GAME_MODE_TRIPLE_COMBAT_LEVEL_INCREASED:
      if (GameModeStartTime == 0) {
        GameModeStartTime = CurrentTime;
        GameModeEndTime = CurrentTime + 3000;
        QueueNotification(SOUND_EFFECT_VP_TRIPLE_COMBAT_LEVEL_INCREASED, 7);
        DoubleCombatLevelCompleted[CurrentPlayer] += 1;
      }
      if (CurrentTime > GameModeEndTime) {
        ResetAllDropTargets();
        PlayBackgroundSong(SOUND_EFFECT_BACKGROUND_SONG_3);
        SetGameMode(GAME_MODE_UNSTRUCTURED_PLAY);
      }
      break;

    case GAME_MODE_TRIPLE_COMBAT_LEVEL_SAME:
      if (GameModeStartTime == 0) {
        GameModeStartTime = CurrentTime;
        GameModeEndTime = CurrentTime + 3000;
        QueueNotification(SOUND_EFFECT_VP_TRIPLE_COMBAT_LEVEL_SAME, 7);
      }
      if (CurrentTime > GameModeEndTime) {
        ResetAllDropTargets();
        PlayBackgroundSong(SOUND_EFFECT_BACKGROUND_SONG_3);
        SetGameMode(GAME_MODE_UNSTRUCTURED_PLAY);
      }
      break;
      
    case GAME_MODE_KINGS_CHALLENGE_START:
      if (GameModeStartTime == 0) {
        GameModeStartTime = CurrentTime;
        GameModeEndTime = CurrentTime + 5000;
        LevitateMagnetOnTime = CurrentTime + 4000;
        byte numChallengesRunning = 0;
        byte challengeFlag = KINGS_CHALLENGE_1_QUALIFIED;        
        for (byte count=0; count<4; count++) {
          if (KingsChallengeStatus[CurrentPlayer] & challengeFlag) {
            QueueNotification(SOUND_EFFECT_VP_KINGS_CHALLENGE_1 + numChallengesRunning, 10);
            numChallengesRunning += 1;
            QueueNotification(SOUND_EFFECT_VP_KINGS_CHALLENGE_JOUST + count, 10);
            IncreasePlayfieldMultiplier(25000);

            // decrease drop target progress 
            NumDropTargetClears[CurrentPlayer][count] = NumDropTargetClears[CurrentPlayer][count] % 3;
          }
          challengeFlag *= 2;
        }
        KingsChallengeRunning = KingsChallengeStatus[CurrentPlayer] & KINGS_CHALLENGE_AVAILABLE;
        KingsChallengeStatus[CurrentPlayer] &= ~KINGS_CHALLENGE_AVAILABLE;
        KingsChallengeEndTime = 5000 + CurrentTime + ((unsigned long)numChallengesRunning * 20000);        
        GameModeStage = 0;
        MagnaSaveAvailable = false;
        KingsChallengeBonus = 5000;
        KingsChallengeBonusChangedTime = 0;
        KingsChallengePerfectionBank = CurrentTime%4;       
      }

      if ( RPU_ReadSingleSwitchState(SW_LEFT_MAGNET_BUTTON) || RPU_ReadSingleSwitchState(SW_RIGHT_MAGNET_BUTTON) ) {
        Audio.StopAllNotifications();
        LockManagementInProgress = false;
        if (KingsChallengeKick==1) {
          RPU_PushToSolenoidStack(SOL_UPPER_BALL_EJECT, 12, true);
        } else if (KingsChallengeKick==2) {
          PlaySoundEffect(SOUND_EFFECT_HORSE_CHUFFING);
          RPU_PushToTimedSolenoidStack(SOL_SAUCER, 16, CurrentTime + 500, true);          
        }
        SetGameMode(GAME_MODE_UNSTRUCTURED_PLAY);
      }

      if (CurrentTime > GameModeEndTime) {
        // Eject ball and return to normal play
        if (KingsChallengeKick==1) {
          RPU_PushToSolenoidStack(SOL_UPPER_BALL_EJECT, 12, true);
        } else if (KingsChallengeKick==2) {
          PlaySoundEffect(SOUND_EFFECT_HORSE_CHUFFING);
          RPU_PushToTimedSolenoidStack(SOL_SAUCER, 16, CurrentTime + 500, true);          
        }
        KingsChallengeKick = 0;
        SetGameMode(GAME_MODE_UNSTRUCTURED_PLAY);
      }

      break;

  }

  if ( !specialAnimationRunning && NumTiltWarnings <= MaxTiltWarnings ) {
    ShowLockLamps();
    ShowBonusLamps();
    ShowBonusXLamps();
    ShowPlayfieldXAndMagnetLamps();
    ShowSpinnerAndPopBumperLamp();
    ShowHeadLamps();
    ShowLaneAndRolloverLamps();
    ShowDropTargetLamps();
    ShowShootAgainLamps();
  }

  // Show spinner progress (when applicable)
  if (LastSpinnerHit != 0 && SpinsTowardsNextGoal[CurrentPlayer] < SpinnerGoal[CurrentPlayer]) {
    OverrideScoreDisplay(CurrentPlayer, SpinnerGoal[CurrentPlayer] - SpinsTowardsNextGoal[CurrentPlayer], DISPLAY_OVERRIDE_ANIMATION_NONE);
    if (CurrentTime > (LastSpinnerHit + 3000)) {
      LastSpinnerHit = 0;
      ShowPlayerScores(0xFF, false, false);
    } else {
      ShowPlayerScores(CurrentPlayer, false, false);
    }
  }
    

  if (UpdateDisplays()) {
    PlaySoundEffect(SOUND_EFFECT_SCORE_TICK);
  }

  // Check to see if ball is in the outhole
  if (CountBallsInTrough() && (CountBallsInTrough() > (TotalBallsLoaded - (NumberOfBallsInPlay + NumberOfBallsLocked)))) {

    if (BallTimeInTrough == 0) {
      // If this is the first time we're seeing too many balls in the trough, we'll wait to make sure
      // everything is settled
      BallTimeInTrough = CurrentTime;
    } else {

      // Make sure the ball stays on the sensor for at least
      // 0.5 seconds to be sure that it's not bouncing or passing through
      if ((CurrentTime - BallTimeInTrough) > 750) {

        if (BallFirstSwitchHitTime == 0 && NumTiltWarnings <= MaxTiltWarnings) {
          // Nothing hit yet, so return the ball to the player
          if (DEBUG_MESSAGES) {
            char buf[128];
            sprintf(buf, "Unqualified and trough=%d, BIP=%d, Lock=%d\n", CountBallsInTrough(), NumberOfBallsInPlay, NumberOfBallsLocked);
            Serial.write(buf);
          }
          RPU_PushToTimedSolenoidStack(SOL_BALL_RAMP_THROWER, 16, CurrentTime);
          BallTimeInTrough = 0;
          returnState = MACHINE_STATE_NORMAL_GAMEPLAY;
        } else {
          // if we haven't used the ball save, and we're under the time limit, then save the ball
          if (BallSaveEndTime && CurrentTime < (BallSaveEndTime + BALL_SAVE_GRACE_PERIOD)) {
            if (DEBUG_MESSAGES) {
              char buf[128];
              sprintf(buf, "Ball save w/ trough=%d, BIP=%d, Lock=%d\n", CountBallsInTrough(), NumberOfBallsInPlay, NumberOfBallsLocked);
              Serial.write(buf);
            }
            RPU_PushToTimedSolenoidStack(SOL_BALL_RAMP_THROWER, 16, CurrentTime + 100);
            QueueNotification(SOUND_EFFECT_VP_SHOOT_AGAIN, 10);

            RPU_SetLampState(LAMP_SHOOT_AGAIN, 0);
            BallTimeInTrough = CurrentTime;
            returnState = MACHINE_STATE_NORMAL_GAMEPLAY;

            if (DEBUG_MESSAGES) {
              char buf[255];
              sprintf(buf, "Ball Save: BIT=%d, ML=0x%02X, LS=0x%02X, Numlocks=%d, NumBIP=%d\n", CountBallsInTrough(), MachineLocks, PlayerLockStatus[CurrentPlayer], NumberOfBallsLocked, NumberOfBallsInPlay);
              Serial.write(buf);
            }

            // Only 1 ball save if one ball in play
            if (NumberOfBallsInPlay == 1) {
              BallSaveEndTime = CurrentTime + 1000;
            } else {
              if (CurrentTime > BallSaveEndTime) BallSaveEndTime += 1000;
            }

          } else {

            if (DEBUG_MESSAGES) {
              char buf[128];
              sprintf(buf, "Drain: Kick b/c trough=%d, BIP=%d, Lock=%d\n", CountBallsInTrough(), NumberOfBallsInPlay, NumberOfBallsLocked);
              Serial.write(buf);
            }

            NumberOfBallsInPlay -= 1;
            if (DEBUG_MESSAGES) {
              char buf[128];
              sprintf(buf, "Num BIP minus 1 to %d b/c DRAIN\n", NumberOfBallsInPlay);
              Serial.write(buf);
            }
            if (NumberOfBallsInPlay == 0) {
              ShowPlayerScores(0xFF, false, false);
              Audio.StopAllAudio();
              MagnaSoundOn = false;
              SetMagnetState(0, false);
              SetMagnetState(1, false);
              returnState = MACHINE_STATE_COUNTDOWN_BONUS;
            }
          }
        }
      }
    }
  } else {
    BallTimeInTrough = 0;
  }

  LastTimeThroughLoop = CurrentTime;
  return returnState;
}



unsigned long CountdownStartTime = 0;
unsigned long LastCountdownReportTime = 0;
unsigned long BonusCountDownEndTime = 0;
byte DecrementingBonusCounter;
byte TotalBonus = 0;
byte TotalBonusX = 0;
byte BonusLadderPass = 0;
boolean CountdownBonusHurryUp = false;
boolean RestartCountdownSound = false;

int CountDownDelayTimes[] = {100, 80, 60, 50, 45, 42, 39, 36, 34, 32};

int CountdownBonus(boolean curStateChanged) {

  // If this is the first time through the countdown loop
  if (curStateChanged) {

    CountdownStartTime = CurrentTime;
    LastCountdownReportTime = CurrentTime;
    ShowBonusXLamps();
    ShowBonusLamps();
    BonusLadderPass = 0;
    RestartCountdownSound = false;
    DecrementingBonusCounter = Bonus[CurrentPlayer];
    TotalBonus = Bonus[CurrentPlayer];
    TotalBonusX = BonusX[CurrentPlayer];
    CountdownBonusHurryUp = false;

    BonusCountDownEndTime = 0xFFFFFFFF;
    if (NumTiltWarnings <= MaxTiltWarnings) PlaySoundEffect(SOUND_EFFECT_COUNTDOWN_BONUS_START);
  }

  unsigned long countdownDelayTime = (unsigned long)(CountDownDelayTimes[BonusLadderPass]);
  if (CountdownBonusHurryUp && countdownDelayTime > ((unsigned long)CountDownDelayTimes[9])) countdownDelayTime = CountDownDelayTimes[9];

  if ((CurrentTime - LastCountdownReportTime) > countdownDelayTime) {

    if (DecrementingBonusCounter) {

      // Only give sound & score if this isn't a tilt
      if (NumTiltWarnings <= MaxTiltWarnings) {
        CurrentScores[CurrentPlayer] += 1000;
        if (RestartCountdownSound) {
          PlaySoundEffect(SOUND_EFFECT_COUNTDOWN_BONUS_START + BonusLadderPass);
          RestartCountdownSound = false;
        }
      }

      DecrementingBonusCounter -= 1;
      Bonus[CurrentPlayer] = DecrementingBonusCounter;
      ShowBonusLamps();
      if ( (Bonus[CurrentPlayer] % 10) == 0 ) {
        if (NumTiltWarnings <= MaxTiltWarnings) {
          Audio.StopSound(SOUND_EFFECT_COUNTDOWN_BONUS_START + BonusLadderPass);
          PlaySoundEffect(SOUND_EFFECT_COUNTDOWN_BONUS_END);
          RestartCountdownSound = true;
        }
        BonusLadderPass += 1;
        if (BonusLadderPass >= 9) BonusLadderPass = 9;
      }

    } else if (BonusCountDownEndTime == 0xFFFFFFFF) {
      if (BonusX[CurrentPlayer] > 1) {
        DecrementingBonusCounter = TotalBonus;
        Bonus[CurrentPlayer] = TotalBonus;
        ShowBonusLamps();
        BonusX[CurrentPlayer] -= 1;
        ShowBonusXLamps();
      } else {
        BonusX[CurrentPlayer] = TotalBonusX;
        Bonus[CurrentPlayer] = TotalBonus;
        BonusCountDownEndTime = CurrentTime + 1000;
      }
    }
    LastCountdownReportTime = CurrentTime;
  }

  if (CurrentTime > BonusCountDownEndTime) {
    PlaySoundEffect(SOUND_EFFECT_COUNTDOWN_BONUS_END);

    // Reset any lights & variables of goals that weren't completed
    BonusCountDownEndTime = 0xFFFFFFFF;
    return MACHINE_STATE_BALL_OVER;
  }

  return MACHINE_STATE_COUNTDOWN_BONUS;
}



void CheckHighScores() {
  unsigned long highestScore = 0;
  int highScorePlayerNum = 0;
  for (int count = 0; count < CurrentNumPlayers; count++) {
    if (CurrentScores[count] > highestScore) highestScore = CurrentScores[count];
    highScorePlayerNum = count;
  }

  if (highestScore > HighScore) {
    HighScore = highestScore;
    if (HighScoreReplay) {
      AddCredit(false, 3);
      RPU_WriteULToEEProm(RPU_TOTAL_REPLAYS_EEPROM_START_BYTE, RPU_ReadULFromEEProm(RPU_TOTAL_REPLAYS_EEPROM_START_BYTE) + 3);
    }
    RPU_WriteULToEEProm(RPU_HIGHSCORE_EEPROM_START_BYTE, highestScore);
    RPU_WriteULToEEProm(RPU_TOTAL_HISCORE_BEATEN_START_BYTE, RPU_ReadULFromEEProm(RPU_TOTAL_HISCORE_BEATEN_START_BYTE) + 1);

    for (int count = 0; count < 4; count++) {
      if (count == highScorePlayerNum) {
        RPU_SetDisplay(count, CurrentScores[count], true, 2, true);
      } else {
        RPU_SetDisplayBlank(count, 0x00);
      }
    }

    RPU_PushToTimedSolenoidStack(SOL_BELL, 20, CurrentTime, true);
    RPU_PushToTimedSolenoidStack(SOL_BELL, 20, CurrentTime + 300, true);
    RPU_PushToTimedSolenoidStack(SOL_BELL, 20, CurrentTime + 600, true);
  }
}


unsigned long MatchSequenceStartTime = 0;
unsigned long MatchDelay = 150;
byte MatchDigit = 0;
byte NumMatchSpins = 0;
byte ScoreMatches = 0;

int ShowMatchSequence(boolean curStateChanged) {
  if (!MatchFeature) return MACHINE_STATE_ATTRACT;

  if (curStateChanged) {
    MatchSequenceStartTime = CurrentTime;
    MatchDelay = 1500;
    MatchDigit = CurrentTime % 10;
    NumMatchSpins = 0;
    RPU_SetLampState(LAMP_HEAD_MATCH, 1, 0);
    RPU_SetDisableFlippers();
    ScoreMatches = 0;
  }

  if (NumMatchSpins < 40) {
    if (CurrentTime > (MatchSequenceStartTime + MatchDelay)) {
      MatchDigit += 1;
      if (MatchDigit > 9) MatchDigit = 0;
      //PlaySoundEffect(10+(MatchDigit%2));
      PlaySoundEffect(SOUND_EFFECT_MATCH_SPIN);
      RPU_SetDisplayBallInPlay((int)MatchDigit * 10);
      MatchDelay += 50 + 4 * NumMatchSpins;
      NumMatchSpins += 1;
      RPU_SetLampState(LAMP_HEAD_MATCH, NumMatchSpins % 2, 0);

      if (NumMatchSpins == 40) {
        RPU_SetLampState(LAMP_HEAD_MATCH, 0);
        MatchDelay = CurrentTime - MatchSequenceStartTime;
      }
    }
  }

  if (NumMatchSpins >= 40 && NumMatchSpins <= 43) {
    if (CurrentTime > (MatchSequenceStartTime + MatchDelay)) {
      if ( (CurrentNumPlayers > (NumMatchSpins - 40)) && ((CurrentScores[NumMatchSpins - 40] / 10) % 10) == MatchDigit) {
        ScoreMatches |= (1 << (NumMatchSpins - 40));
        AddSpecialCredit();
        MatchDelay += 1000;
        NumMatchSpins += 1;
        RPU_SetLampState(LAMP_HEAD_MATCH, 1);
      } else {
        NumMatchSpins += 1;
      }
      if (NumMatchSpins == 44) {
        MatchDelay += 5000;
      }
    }
  }

  if (NumMatchSpins > 43) {
    if (CurrentTime > (MatchSequenceStartTime + MatchDelay)) {
      return MACHINE_STATE_ATTRACT;
    }
  }

  for (int count = 0; count < 4; count++) {
    if ((ScoreMatches >> count) & 0x01) {
      // If this score matches, we're going to flash the last two digits
      byte upperMask = 0x0F;
      byte lowerMask = 0x30;
      if (RPU_OS_NUM_DIGITS == 7) {
        upperMask = 0x1F;
        lowerMask = 0x60;
      }
      if ( (CurrentTime / 200) % 2 ) {
        RPU_SetDisplayBlank(count, RPU_GetDisplayBlank(count) & upperMask);
      } else {
        RPU_SetDisplayBlank(count, RPU_GetDisplayBlank(count) | lowerMask);
      }
    }
  }

  return MACHINE_STATE_MATCH_MODE;
}




////////////////////////////////////////////////////////////////////////////
//
//  Switch Handling functions
//
////////////////////////////////////////////////////////////////////////////
void HandleLockSwitch(byte lockIndex) {


  // This switch is only "new" if it's not reflected in MachineLocks
  if (MachineLocks & (LOCK_1_ENGAGED << lockIndex)) {
    if (DEBUG_MESSAGES) {
      char buf[128];
      sprintf(buf, "Ignoring lock switch %d because ML=0x%04X\n", lockIndex, MachineLocks);
      Serial.write(buf);
    }
    return;
  }

  if (MachineState == MACHINE_STATE_NORMAL_GAMEPLAY) {

    if (GameMode == GAME_MODE_SKILL_SHOT) {
      RPU_PushToSolenoidStack(SOL_UPPER_BALL_EJECT, 12, true);
      QueueNotification(SOUND_EFFECT_VP_SKILL_SHOT, 10);
      StartScoreAnimation(50000 * CurrentBallInPlay);
    } else if (GameMode == GAME_MODE_UNSTRUCTURED_PLAY) {
      CurrentScores[CurrentPlayer] += PlayfieldMultiplier * 5000;

      // These are the combat modes
      if (KingsChallengeStatus[CurrentPlayer]&KINGS_CHALLENGE_AVAILABLE) {
        KingsChallengeKick = 1;
        LockManagementInProgress = true;
        SetGameMode(GAME_MODE_KINGS_CHALLENGE_START);
      } else if ( (PlayerLockStatus[CurrentPlayer]&LOCKS_ENGAGED_MASK) == 0 && (PlayerLockStatus[CurrentPlayer]&LOCK_1_AVAILABLE) ) {
        // If there are no balls locked, but lock 1 is available, offer single cobat
        if (DEBUG_MESSAGES) {
          Serial.write("HLS: lock 1 avail, so offer single\n");
        }
        LockManagementInProgress = true;
        SetGameMode(GAME_MODE_OFFER_SINGLE_COMBAT);
      } else if ( (PlayerLockStatus[CurrentPlayer]&LOCKS_ENGAGED_MASK) == LOCK_1_ENGAGED && (PlayerLockStatus[CurrentPlayer]&LOCK_2_AVAILABLE) ) {
        // If there is 1 ball locked, and lock 2 is available, offer double combat
        if (DEBUG_MESSAGES) {
          Serial.write("HLS: lock 2 avail, so offer double\n");
        }
        LockManagementInProgress = true;
        SetGameMode(GAME_MODE_OFFER_DOUBLE_COMBAT);
      } else if ( (PlayerLockStatus[CurrentPlayer]&LOCKS_ENGAGED_MASK) == (LOCK_1_ENGAGED | LOCK_2_ENGAGED) && (PlayerLockStatus[CurrentPlayer]&LOCK_3_AVAILABLE) ) {
        // If the player has 2 balls locked and gets a third, go into triple combat
        if (DEBUG_MESSAGES) {
          Serial.write("HLS: lock 3 avail, so into triple\n");
        }
        LockManagementInProgress = true;
        SetGameMode(GAME_MODE_TRIPLE_COMBAT_START);
      } else {
        if (DEBUG_MESSAGES) {
          Serial.write("HLS: no locks avail, bump\n");
          char buf[128];
          sprintf(buf, "HLS: PlayerLockStatus = 0x%02X\n", PlayerLockStatus[CurrentPlayer]);
          Serial.write(buf);
        }
        PlaySoundEffect(SOUND_EFFECT_SWOOSH);
        RPU_PushToSolenoidStack(SOL_UPPER_BALL_EJECT, 12, true);
      }
    } else if (GameMode == GAME_MODE_SINGLE_COMBAT) {

      // If the player has cleared a bank, they can increase death blow
      byte numBanksCleared = 0;

      // Jackpots and such
      numBanksCleared = ResetAllDropTargets(true);

      if (numBanksCleared) {
        // mors auctus ictu (death blow increased)
        QueueNotification(SOUND_EFFECT_VP_DEATH_BLOW_INCREASED, 10);
        CombatJackpotReady = false;

        unsigned long halfwayTime = ((unsigned long)SingleCombatNumSeconds) * 500;
        if ( (CurrentTime + halfwayTime) > GameModeEndTime ) {
          GameModeEndTime = CurrentTime + halfwayTime;
        }
        LockKickTime[0] = CurrentTime + 2000;

        unsigned long jackpotAddition = (CombatJackpot[CurrentPlayer] / 4) * ((unsigned long)numBanksCleared);
        CombatJackpot[CurrentPlayer] += jackpotAddition;
        JackpotIncreasedTime = CurrentTime;
      } else {
        PlaySoundEffect(SOUND_EFFECT_BOOING_1);
        QueueNotification(SOUND_EFFECT_VP_RETURN_TO_FIGHT, 8);
        RPU_PushToSolenoidStack(SOL_UPPER_BALL_EJECT, 12, true);
      }


    } else if (GameMode == GAME_MODE_DOUBLE_COMBAT) {
      if (DEBUG_MESSAGES) {
        Serial.write("Double Combat: Handling a lock switch\n");
      }
      // Jackpots and such
      byte numTimedKicks = 0;
      for (byte count = 0; count < 3; count++) {
        if (LockKickTime[count] != 0) numTimedKicks += 1;
      }
      if (SaucerKickTime) numTimedKicks += 1;

      if (!CombatJackpotReady && numTimedKicks == 0) {
        LockKickTime[0] = CurrentTime;
        PlaySoundEffect(SOUND_EFFECT_BOOING_1);
        QueueNotification(SOUND_EFFECT_VP_RETURN_TO_FIGHT, 8);
      } else {
        if (CombatJackpotReady) {
          // Reset the banks for future jackpots
          ResetAllDropTargets(true);

          if (numTimedKicks == 0) {
            // Award a jackpot for hitting the lock
            QueueNotification(SOUND_EFFECT_VP_JACKPOT, 10);
            StartScoreAnimation(CombatJackpot[CurrentPlayer] * PlayfieldMultiplier);
            CombatBankFlags = 0;
            CombatJackpot[CurrentPlayer] += COMBAT_JACKPOT_STEP;
            for (byte count = 0; count < 3; count++) {
              if (LockKickTime[count] == 0) {
                // Tell this lock to kick in 8 seconds
                if (NumberOfBallsInPlay) {
                  NumberOfBallsInPlay -= 1;
                  if (DEBUG_MESSAGES) {
                    char buf[128];
                    sprintf(buf, "Num BIP minus 1 to %d b/c jackpot\n", NumberOfBallsInPlay);
                    Serial.write(buf);
                  }
                }
                LockKickTime[count] = CurrentTime + 8000;
                break;
              }
            }
          } else {
            // This is a rare case where they got a second jackpot ready
            // while a ball was still waiting to be kicked
            QueueNotification(SOUND_EFFECT_VP_MEGA_JACKPOT, 10);
            StartScoreAnimation(CombatJackpot[CurrentPlayer] * PlayfieldMultiplier * 10);
            CombatJackpot[CurrentPlayer] += COMBAT_JACKPOT_STEP*((unsigned long)10);
            CombatBankFlags = 0;
            boolean kickScheduled = false;
            for (byte count = 0; count < 3; count++) {
              if (LockKickTime[count] == 0 && !kickScheduled) {
                // Tell this lock to kick in 1 second
                LockKickTime[count] = CurrentTime + 1000;
                kickScheduled = true;
              } else {
                LockKickTime[count] = CurrentTime + 2000;
              }
            }
          }
          CombatJackpotReady = false;

        } else {
          // award double for second ball in lock
          if (CombatJackpotReady==false && CombatSuperJackpotReady==false) {
            RPU_PushToSolenoidStack(SOL_UPPER_BALL_EJECT, 12, true);
          } else if (SaucerKickTime == 0) {
            // Reset the banks for future jackpots
            ResetAllDropTargets(true);

            QueueNotification(SOUND_EFFECT_VP_DOUBLE_JACKPOT, 10);
            StartScoreAnimation(CombatJackpot[CurrentPlayer] * PlayfieldMultiplier * 2);
            CombatJackpot[CurrentPlayer] += COMBAT_JACKPOT_STEP*((unsigned long)2);
            CombatBankFlags = 0;
            boolean kickScheduled = false;
            for (byte count = 0; count < 3; count++) {
              if (LockKickTime[count] == 0 && !kickScheduled) {
                // Tell this lock to kick in 1 second
                LockKickTime[count] = CurrentTime + 1000;
                kickScheduled = true;
              } else {
                LockKickTime[count] = CurrentTime + 2000;
              }
            }
          } else {
            // Reset the banks for future jackpots
            ResetAllDropTargets(true);

            QueueNotification(SOUND_EFFECT_VP_SUPER_JACKPOT, 10);
            StartScoreAnimation(CombatJackpot[CurrentPlayer] * PlayfieldMultiplier * 5);
            CombatJackpot[CurrentPlayer] += COMBAT_JACKPOT_STEP*((unsigned long)5);
            CombatBankFlags = 0;
            for (byte count = 0; count < 3; count++) {
              if (LockKickTime[count] == 0) {
                // Tell this lock to kick in 1 second
                LockKickTime[count] = CurrentTime + 1000;
                break;
              }
            }
          }
          CombatJackpotReady = false;
        }
      }

    } else if (GameMode == GAME_MODE_TRIPLE_COMBAT) {
      // Jackpots and such
    } else {
      RPU_PushToSolenoidStack(SOL_UPPER_BALL_EJECT, 12, true);
    }


  } else {
    // If we're in attract, or some other non-gameplay mode,
    // we will register the change to MachineLocks, but not do anything
    // about it.
    if (UpperLockSwitchState[0]) MachineLocks |= LOCK_1_ENGAGED;
    else MachineLocks &= ~LOCK_1_ENGAGED;
    if (UpperLockSwitchState[1]) MachineLocks |= LOCK_2_ENGAGED;
    else MachineLocks &= ~LOCK_2_ENGAGED;
    if (UpperLockSwitchState[2]) MachineLocks |= LOCK_3_ENGAGED;
    else MachineLocks &= ~LOCK_3_ENGAGED;
  }
}



int HandleSystemSwitches(int curState, byte switchHit) {
  int returnState = curState;
  switch (switchHit) {
    case SW_SELF_TEST_SWITCH:
      returnState = MACHINE_STATE_TEST_BOOT;
      SetLastSelfTestChangedTime(CurrentTime);
      break;
    case SW_COIN_1:
    case SW_COIN_2:
    case SW_COIN_3:
      AddCoinToAudit(SwitchToChuteNum(switchHit));
      AddCoin(SwitchToChuteNum(switchHit));
      break;
    case SW_CREDIT_RESET:
      if (MachineState == MACHINE_STATE_MATCH_MODE) {
        // If the first ball is over, pressing start again resets the game
        if (Credits >= 1 || FreePlayMode) {
          if (!FreePlayMode) {
            Credits -= 1;
            RPU_WriteByteToEEProm(RPU_CREDITS_EEPROM_BYTE, Credits);
            RPU_SetDisplayCredits(Credits, !FreePlayMode);
          }
          returnState = MACHINE_STATE_INIT_GAMEPLAY;
        }
      } else {
        CreditResetPressStarted = CurrentTime;
      }
      break;
    case SW_OUTHOLE:
      MoveBallFromOutholeToRamp(true);
      break;
    case SW_PLUMB_TILT:
    case SW_ROLL_TILT:
    case SW_PLAYFIELD_TILT:
      // This should be debounced
      if (IdleMode != IDLE_MODE_BALL_SEARCH && (CurrentTime - LastTiltWarningTime) > TILT_WARNING_DEBOUNCE_TIME) {
        LastTiltWarningTime = CurrentTime;
        NumTiltWarnings += 1;
        if (NumTiltWarnings > MaxTiltWarnings) {
          RPU_DisableSolenoidStack();
          RPU_SetDisableFlippers(true);
          RPU_TurnOffAllLamps();
          RPU_SetLampState(LAMP_HEAD_TILT, 1);
          Audio.StopAllAudio();
          PlaySoundEffect(SOUND_EFFECT_TILT);
        } else {
          PlaySoundEffect(SOUND_EFFECT_TILT_WARNING);
        }
        OverrideGeneralIllumination(false, CurrentTime + 600);
      }
      break;
  }

  return returnState;
}


void QualifyKingsChallenge(byte dropTargetBank) {

  byte kingsChallengeFlag = (KINGS_CHALLENGE_1_QUALIFIED<<dropTargetBank);

  if ( (KingsChallengeStatus[CurrentPlayer] & kingsChallengeFlag)==0 ) {
    // Announce challenge
    QueueNotification(SOUND_EFFECT_VP_KINGS_CHALLENGE_AVAILABLE, 9);
    
    // Set flag
    KingsChallengeStatus[CurrentPlayer] |= kingsChallengeFlag;
  } else {
    // No need to announce anything because it's already qualified?
  }
  
}


boolean HandleDropTarget(byte bankNum, byte switchHit) {

  if (NumberOfBallsInPlay == 1 && RPU_ReadSingleSwitchState(SW_SHOOTER_LANE)) {
    // we haven't plunged yet, so ignore this hit
    if (DEBUG_MESSAGES) {
      Serial.write("Ball in lane - ignore drop target\n");
    }
    return false;
  }

  DropTargetBank *curBank;
  if (bankNum == 0) curBank = &DropTargetsUL;
  else if (bankNum == 1) curBank = &DropTargetsUR;
  else if (bankNum == 2) curBank = &DropTargetsLL;
  else if (bankNum == 3) curBank = &DropTargetsLR;

  byte result;
  unsigned long numTargetsDown = 0;
  result = curBank->HandleDropTargetHit(switchHit);
  numTargetsDown = (unsigned long)CountBits(result);
  NumDropTargetHits[CurrentPlayer][bankNum] += numTargetsDown;
  boolean joustHit = false;
  boolean perfectionMissed = false;
  CurrentScores[CurrentPlayer] += PlayfieldMultiplier * numTargetsDown * 1000;

  boolean cleared = curBank->CheckIfBankCleared();

  if (numTargetsDown) {

    if (DEBUG_MESSAGES) {
      if (bankNum == 1) {
        char buf[128];
        sprintf(buf, "UR Drops = 0x%02X\n", DropTargetsUR.GetStatus(false));
      }
    }

    if (KingsChallengeRunning&KINGS_CHALLENGE_JOUST) {
      // Check if middle target hit
      if (result & 0x02) {
        PlaySoundEffect(SOUND_EFFECT_SINGLE_ANVIL);
        if (KingsChallengeBonus<750000) {
          KingsChallengeBonus += 25000;
          KingsChallengeBonusChangedTime = CurrentTime;        
        }
        joustHit = true;                
      }
    }

    if (KingsChallengeRunning&KINGS_CHALLENGE_PERFECTION) {
      if (bankNum==KingsChallengePerfectionBank) {
        PlaySoundEffect(SOUND_EFFECT_SINGLE_ANVIL);
        if (KingsChallengeBonus<750000) {
          KingsChallengeBonus += 25000;
          KingsChallengeBonusChangedTime = CurrentTime;        
        }
      } else {
        perfectionMissed = true;
      }
    }

    if (GameMode == GAME_MODE_SKILL_SHOT) {
      if (bankNum == 0) {
        byte switchIndex = SW_UL_DROP_3 - switchHit;
        if (switchIndex == SkillShotTarget) {
          QueueNotification(SOUND_EFFECT_VP_SUPER_SKILL_SHOT, 10);
          StartScoreAnimation(100000 * CurrentBallInPlay);
        } else {
          QueueNotification(SOUND_EFFECT_VP_SKILL_SHOT_MISSED, 10);
        }
      } else {
        QueueNotification(SOUND_EFFECT_VP_SKILL_SHOT_MISSED, 10);
      }
      if (DropTargetResetTime[bankNum] == 0) DropTargetResetTime[bankNum] = CurrentTime + 1000;
    } else if (GameMode == GAME_MODE_SINGLE_COMBAT) {
      if (cleared) {

        CombatBankFlags |= (DROP_BANK_UL_FLAG << bankNum);
        boolean jackpotQualified = false;

        if (!CombatJackpotReady) {

          if (SingleCombatLevelCompleted[CurrentPlayer] == 0) {
            jackpotQualified = true;
          } else if (SingleCombatLevelCompleted[CurrentPlayer] == 1) {
            // Upper and lower
            if ( (CombatBankFlags & 0x03) && (CombatBankFlags & 0x0C)) jackpotQualified = true;        
          } else {
             if (CombatBankFlags==0x0F) jackpotQualified = true;
          }

          if (jackpotQualified) {
            QueueNotification(SOUND_EFFECT_VP_SAUCER_FOR_DEATHBLOW, 7);
            CombatJackpotReady = true;
            if (DEBUG_MESSAGES) {
              Serial.write("Single combat jackpot ready\n");
            }
          }
        }

        AddToBonus(4);
      }
      PlaySoundEffect(SOUND_EFFECT_SWORD_1 + (CurrentTime) % 7);
      CombatJackpot[CurrentPlayer] += 5000 * numTargetsDown * ((unsigned long)SingleCombatLevelCompleted[CurrentPlayer] + 1);
      JackpotIncreasedTime = CurrentTime;
    } else if (GameMode == GAME_MODE_DOUBLE_COMBAT) {
      if (cleared) {
        CombatBankFlags |= (DROP_BANK_UL_FLAG << bankNum);
        AddToBonus(5);

        if (DoubleCombatLevelCompleted[CurrentPlayer] == 0) {
          // If an upper and lower has been cleared, then jackpot is ready
          if ( (CombatBankFlags & 0x03) && (CombatBankFlags & 0x0C)) {
            if (!CombatJackpotReady) {
              QueueNotification(SOUND_EFFECT_VP_JACKPOT_READY, 7);
            }
            CombatJackpotReady = true;
          }
        } else if (DoubleCombatLevelCompleted[CurrentPlayer] == 1) {
          // If three banks have been cleared
          byte numTargetBanks = CountBits(CombatBankFlags);
          if ( numTargetBanks >= 3 ) {
            if (!CombatJackpotReady) {
              QueueNotification(SOUND_EFFECT_VP_JACKPOT_READY, 7);
            }
            CombatJackpotReady = true;
          }
        } else {
          // If an upper and lower has been cleared, then jackpot is ready
          if ( (CombatBankFlags & 0x03) && (CombatBankFlags & 0x0C)) {
            if (!CombatJackpotReady) {
              QueueNotification(SOUND_EFFECT_VP_JACKPOT_READY, 7);
            }
            CombatJackpotReady = true;
          }
        }
        
      } else {
        if (DoubleCombatLevelCompleted[CurrentPlayer] > 1) {
          // For level 2 & up, there's a hurry up
          if (DropTargetResetTime[bankNum] == 0) DropTargetResetTime[bankNum] = CurrentTime + 8000;
          DropTargetHurryTime[bankNum] = CurrentTime;
          DropTargetHurryLamp[bankNum] = false;
        }
      }

    } else if (GameMode == GAME_MODE_TRIPLE_COMBAT) {
      if (cleared) {
        CombatBankFlags |= (DROP_BANK_UL_FLAG << bankNum);
        AddToBonus(6);

        byte numTargets = CountBits(CombatBankFlags);
        if (TripleCombatJackpotsAvailable==0 && numTargets>=(TripleCombatLevelCompleted[CurrentPlayer]+2)) {
          TripleCombatJackpotsAvailable = TRIPLE_COMBAT_ALL_JACKPOTS;
          QueueNotification(SOUND_EFFECT_VP_TRIPLE_JACKPOTS_READY, 7);
        }
      }

      
      if ( (CombatBankFlags & 0x03) && (CombatBankFlags & 0x0C)) {
        CombatJackpotReady = true;
      }
    } else {

      // Step 3 - handle default behavior
      if (cleared) {
        NumDropTargetClears[CurrentPlayer][bankNum] += 1;

        // Every time we get to a 3rd clear, there are awards
        if (NumDropTargetClears[CurrentPlayer][bankNum]==3) {
          if (bankNum==2) LastChanceStatus[CurrentPlayer] |= LAST_CHANCE_LEFT_QUALIFIED;
          if (bankNum==3) LastChanceStatus[CurrentPlayer] |= LAST_CHANCE_RIGHT_QUALIFIED;
          if (bankNum<2 && NumDropTargetClears[CurrentPlayer][0]==3 && NumDropTargetClears[CurrentPlayer][1]==3) {
            ExtraBallsOrSpecialAvailable[CurrentPlayer] |= EBS_UPPER_EXTRA_BALL_AVAILABLE;            
          }

          // Qualify King's Challenges
          QualifyKingsChallenge(bankNum);
        }

        curBank->ResetDropTargets(CurrentTime + 500, true);
        DropTargetResetTime[bankNum] = 0;
        PlaySoundEffect(SOUND_EFFECT_DROP_TARGET_COMPLETE);
        LoopLitToQualifyLock = true;

        // update Magna status for this player
        if (MagnaStatusLeft[CurrentPlayer] < MagnaStatusRight[CurrentPlayer]) {
          MagnaStatusLeft[CurrentPlayer] += 1000;
          if (MagnaStatusLeft[CurrentPlayer] > 5000) MagnaStatusLeft[CurrentPlayer] = 5000;
        } else {
          MagnaStatusRight[CurrentPlayer] += 1000;
          if (MagnaStatusRight[CurrentPlayer] > 5000) MagnaStatusRight[CurrentPlayer] = 5000;
        }

        AddToBonus(3);
      } else {
        if (DropTargetHurryTime[bankNum]) {
          PlaySoundEffect(SOUND_EFFECT_DROP_TARGET_HIT_2 + CurrentTime % 2);
        } else {
          PlaySoundEffect(SOUND_EFFECT_DROP_TARGET_HIT_1);
        }
        if (DropTargetResetTime[bankNum] == 0) DropTargetResetTime[bankNum] = CurrentTime + 8000;
        DropTargetHurryTime[bankNum] = CurrentTime;
        DropTargetHurryLamp[bankNum] = false;
        if (joustHit) {
          DropTargetResetTime[bankNum] = CurrentTime + 2000;
        }
        if (perfectionMissed) {
          DropTargetResetTime[bankNum] = CurrentTime + 1000;
          DropTargetResetTime[KingsChallengePerfectionBank] = CurrentTime + 1000;
        }
      }
    }

    return true;
  }

  return false;
}


void UpdateComboStatus() {
  //  ComboSteps *curStep = NULL;

  // Check to see if any combos have timed out
  for (byte count = 0; count < NumberOfComboDefinitions; count++) {

    boolean trackingDone = false;

    // If the current step has expired,
    // we can turn off any variables and
    // reset the combo
    if (BKCombos[count].currentTimeout) {
      // This combo has timed out -- reset everything
      if (CurrentTime > BKCombos[count].currentTimeout) {
        if (BKCombos[count].currentStep > BKCombos[count].trackingStep) {
          trackingDone = true;
        }
        BKCombos[count].currentStep = 0;
        BKCombos[count].currentTimeout = 0;

        if (0 && DEBUG_MESSAGES) {
          char buf[128];
          sprintf(buf, "Timeout combo %d (0x%04lX)\n", count, BKCombos[count].achievementFlag);
          Serial.write(buf);
        }
      }
    } else if (BKCombos[count].currentStep == BKCombos[count].numSteps) {
      // This combo is done -- reset tracking (if necessary)
      BKCombos[count].currentStep = 0;
      BKCombos[count].currentTimeout = 0;
      trackingDone = true;
    }

    if (trackingDone) {
      // Turn off any scoring/other variables
      // that indicate this combo is active
      if (BKCombos[count].trackingVariable != NULL) {
        *(BKCombos[count].trackingVariable) = false;
      }
    }
  }
}

unsigned long LastLeftReturnRolloverHit = 0;

void AdvanceCombos(byte switchHit) {

  ComboSteps *curStep = NULL;

  // need to debounce lane rollovers a bit
  if (switchHit == SW_LEFT_INSIDE_ROLLOVER) {
    if (CurrentTime < (LastLeftReturnRolloverHit + 500)) return;
    LastLeftReturnRolloverHit = CurrentTime;
  }

  // Check to see if any comobos can be advanced
  for (byte count = 0; count < NumberOfComboDefinitions; count++) {
    boolean trackingDone = false;
    if (BKCombos[count].currentStep < BKCombos[count].numSteps) {
      curStep = &(BKCombos[count].steps[BKCombos[count].currentStep]);
    } else {
      // There's something wrong with this combo tracker, so we have to reset it
      BKCombos[count].currentStep = 0;
      BKCombos[count].currentTimeout = 0;
      continue;
    }

    if (switchHit == curStep->comboSwitch) {

      if (BKCombos[count].currentStep == BKCombos[count].trackingStep) {
        if (BKCombos[count].trackingVariable != NULL) *(BKCombos[count].trackingVariable) = true;
      }

      if (curStep->comboTimeConstraint) {
        BKCombos[count].currentTimeout = CurrentTime + curStep->comboTimeConstraint;
      } else {
        BKCombos[count].currentTimeout = 0;
      }

      // If this switch matches the next switch in a combo
      BKCombos[count].currentStep += 1;

      if (BKCombos[count].currentStep == BKCombos[count].numSteps) {
        // This combo is finished
        if ( (CombosAchieved[CurrentPlayer]&BKCombos[count].achievementFlag) == 0 ) {
          // This is the first time for this combo
          CombosAchieved[CurrentPlayer] |= BKCombos[count].achievementFlag;
          StartScoreAnimation(BKCombos[count].firstTimePoints);
          byte numCombosDone = CountBits(CombosAchieved[CurrentPlayer]);
          if (numCombosDone > 9) numCombosDone = 9;
          QueueNotification(SOUND_EFFECT_VP_RELIC_1 + (numCombosDone - 1), 6);
        } else {
          // This combo has been achieved before
          StartScoreAnimation(BKCombos[count].subsequentTimesPoints);
        }
        trackingDone = true;

        if (0 && DEBUG_MESSAGES) {
          char buf[128];
          sprintf(buf, "Combo %d (0x%04lX) complete\n", count, BKCombos[count].achievementFlag);
          Serial.write(buf);
        }

        // The combo status variables will get shut off by the update function
      } else {

        if (0 && DEBUG_MESSAGES) {
          char buf[128];
          sprintf(buf, "Combo %d (0x%04lX) advanced to %d\n", count, BKCombos[count].achievementFlag, BKCombos[count].currentStep);
          Serial.write(buf);
        }

        //curStep = &(BKCombos[count].steps[BKCombos[count].currentStep]);
      }
    } else if (BKCombos[count].currentStep) {
      // This switch doesn't match this combo -- we need to reset the strict combos
      if (curStep->comboStepType == COMBO_STEP_TYPE_STRICT_SWITCH || curStep->comboStepType == COMBO_STEP_TYPE_STRICT_SWITCH_AND_TIME) {
        BKCombos[count].currentStep = 0;
        BKCombos[count].currentTimeout = 0;
        trackingDone = true;
        if (0 && DEBUG_MESSAGES) {
          char buf[128];
          sprintf(buf, "Combo %d (0x%04lX) reset, wrong switch\n", count, BKCombos[count].achievementFlag);
          Serial.write(buf);
        }

      }
    }

    if (trackingDone) {
      // Turn off any scoring/other variables
      // that indicate this combo is active
      if (BKCombos[count].trackingVariable != NULL) {
        *(BKCombos[count].trackingVariable) = false;
      }
    }
  }
}


void QualifyNextPlayerLock() {

  byte playerLocks = PlayerLockStatus[CurrentPlayer];

  // if there are no more locks to qualify -- return
  if ( (playerLocks & LOCKS_ENGAGED_MASK) == LOCKS_ENGAGED_MASK ) return;

  LoopLitToQualifyLock = false;

  for (byte count = 0; count < 3; count++) {
    if ( (playerLocks & (LOCK_1_ENGAGED << count)) == 0 && (playerLocks & (LOCK_1_AVAILABLE << count)) == 0) {
      PlayerLockStatus[CurrentPlayer] |= (LOCK_1_AVAILABLE << count);
      PlaySoundEffect(SOUND_EFFECT_PORTCULLIS);
      return;
    }
  }

  PlaySoundEffect(SOUND_EFFECT_DOOR_SLAM);

}

void RemoveTopQualifiedFlag() {

  byte playerLocks = PlayerLockStatus[CurrentPlayer];

  // if there are no more locks to qualify -- return
  if ( (playerLocks & LOCKS_AVAILABLE_MASK) == 0 ) return;

  for (byte count = 0; count < 3; count++) {
    if ( (playerLocks & (LOCK_3_AVAILABLE >> count)) ) {
      PlayerLockStatus[CurrentPlayer] &= ~(playerLocks & (LOCK_3_AVAILABLE >> count));
      break;
    }
  }
}


unsigned long LastSaucerHit = 0;

void HandleSaucer() {

  if (CurrentTime < (LastSaucerHit + 500)) {
    return;
  }
  LastSaucerHit = CurrentTime;

  byte numTimedKicks = 0;
  for (byte count = 0; count < 3; count++) {
    if (LockKickTime[count] != 0) numTimedKicks += 1;
  }

  if (BonusXCollectAvailable) {
    BonusXCollectAvailable = false;
    BonusXCollectReminder = 0;
    NumBonusXCollectReminders = 0;
    IncreaseBonusX();
  }

  if (GameMode == GAME_MODE_SINGLE_COMBAT) {
    if (CombatJackpotReady) {
      SetGameMode(GAME_MODE_SINGLE_COMBAT_WON);
    } else {
      CurrentScores[CurrentPlayer] += PlayfieldMultiplier * 1000;
      QueueNotification(SOUND_EFFECT_VP_SINGLE_OPPONENT_RALLIES, 8);
      RPU_PushToTimedSolenoidStack(SOL_SAUCER, 16, CurrentTime + 1000, true);

      if (DropTargetsUL.GetStatus()) {
        DropTargetsUL.ResetDropTargets(CurrentTime + 200, true);
      } else if (DropTargetsUR.GetStatus()) {
        DropTargetsUR.ResetDropTargets(CurrentTime + 200, true);
      } else if (DropTargetsLL.GetStatus()) {
        DropTargetsLL.ResetDropTargets(CurrentTime + 200, true);
      } else if (DropTargetsLR.GetStatus()) {
        DropTargetsLR.ResetDropTargets(CurrentTime + 200, true);
      }
    }
  } else if (GameMode == GAME_MODE_DOUBLE_COMBAT) {

    if (CombatJackpotReady) {
      if (numTimedKicks == 0) {
        // Award a jackpot for hitting the lock
        QueueNotification(SOUND_EFFECT_VP_JACKPOT, 10);
        StartScoreAnimation(CombatJackpot[CurrentPlayer] * PlayfieldMultiplier);
        CombatJackpot[CurrentPlayer] += COMBAT_JACKPOT_STEP;
        CombatBankFlags = 0;
        SaucerKickTime = CurrentTime + 8000;
      } else {
        // This is a rare case where they got a second jackpot ready
        // while a ball was still waiting to be kicked
        QueueNotification(SOUND_EFFECT_VP_MEGA_JACKPOT, 10);
        ResetAllDropTargets(true);
        StartScoreAnimation(CombatJackpot[CurrentPlayer] * PlayfieldMultiplier * 10);
        CombatJackpot[CurrentPlayer] += COMBAT_JACKPOT_STEP*((unsigned long)10);
        CombatBankFlags = 0;
        for (byte count = 0; count < 3; count++) {
          if (LockKickTime[count] != 0) {
            // Tell this lock to kick in 1 second
            LockKickTime[count] = CurrentTime + 1000;
          }
        }
        SaucerKickTime = CurrentTime + 2000;
      }
      CombatJackpotReady = false;
    } else if (numTimedKicks) {
      if (CombatSuperJackpotReady) {
        // There's a ball waiting in the lock, so we can get a super jackpot
        QueueNotification(SOUND_EFFECT_VP_SUPER_JACKPOT, 10);
        StartScoreAnimation(CombatJackpot[CurrentPlayer] * 5 * PlayfieldMultiplier);
        CombatJackpot[CurrentPlayer] += COMBAT_JACKPOT_STEP*((unsigned long)5);
        SaucerKickTime = CurrentTime + 4000;
        CombatSuperJackpotReady = false;
      } else {
        // Double combat jackpot was used, so we'll just hold
        SaucerKickTime = CurrentTime + 8000;
      }
    } else {
      RPU_PushToTimedSolenoidStack(SOL_SAUCER, 16, CurrentTime + 1000, true);
    }
  } else if (GameMode == GAME_MODE_TRIPLE_COMBAT) {
    RPU_PushToTimedSolenoidStack(SOL_SAUCER, 16, CurrentTime + 1000, true);
  } else if (KingsChallengeStatus[CurrentPlayer]&KINGS_CHALLENGE_AVAILABLE) {
    PlaySoundEffect(SOUND_EFFECT_DOOR_SLAM);
    KingsChallengeKick = 2;
    SetGameMode(GAME_MODE_KINGS_CHALLENGE_START);
  } else {
    PlaySoundEffect(SOUND_EFFECT_HORSE_CHUFFING);
    RPU_PushToTimedSolenoidStack(SOL_SAUCER, 16, CurrentTime + 1000, true);
  }

  CurrentScores[CurrentPlayer] += PlayfieldMultiplier * 5000;
}


void AwardSpinnerGoal() {
  SpinsTowardsNextGoal[CurrentPlayer] = 0;
  SpinnerGoal[CurrentPlayer] += 10;
  BonusXCollectReminder = CurrentTime + 15000;
  BonusXCollectAvailable = true;
  BonusXCollectAvailableStart = CurrentTime;
  PlaySoundEffect(SOUND_EFFECT_THREE_DINGS);
  NumBonusXCollectReminders = 0;
}


void RemindBonusXCollect() {
  BonusXCollectReminder = CurrentTime + 10000;
  BonusXCollectAvailableStart = CurrentTime;
  PlaySoundEffect(SOUND_EFFECT_THREE_DINGS);
  NumBonusXCollectReminders += 1;
  if (NumBonusXCollectReminders==6) {
    NumBonusXCollectReminders = 0;
    QueueNotification(SOUND_EFFECT_VP_BONUS_X_COLLECT_INSTRUCTIONS, 7);
  }
}


void HandleGamePlaySwitches(byte switchHit) {


  AdvanceCombos(switchHit);

  switch (switchHit) {

    case SW_LEFT_SLING:
//      if (CurrentTime < (BallSearchSolenoidFireTime[BALL_SEARCH_LEFT_SLING_INDEX] + 150)) break;
      CurrentScores[CurrentPlayer] += PlayfieldMultiplier * 10;
      PlaySoundEffect(SOUND_EFFECT_SLING_SHOT);
      LastSwitchHitTime = CurrentTime;
      if (BallFirstSwitchHitTime == 0) BallFirstSwitchHitTime = CurrentTime;
      break;
      
    case SW_RIGHT_SLING:
//      if (CurrentTime < (BallSearchSolenoidFireTime[BALL_SEARCH_RIGHT_SLING_INDEX] + 150)) break;
      CurrentScores[CurrentPlayer] += PlayfieldMultiplier * 10;
      PlaySoundEffect(SOUND_EFFECT_SLING_SHOT);
      LastSwitchHitTime = CurrentTime;      
      if (BallFirstSwitchHitTime == 0) BallFirstSwitchHitTime = CurrentTime;      
      break;

    case SW_UL_DROP_1:
    case SW_UL_DROP_2:
    case SW_UL_DROP_3:
      if (HandleDropTarget(0, switchHit)) {
        LastSwitchHitTime = CurrentTime;
        if (BallFirstSwitchHitTime == 0) BallFirstSwitchHitTime = CurrentTime;
      }
      break;

    case SW_UR_DROP_1:
    case SW_UR_DROP_2:
    case SW_UR_DROP_3:
      if (HandleDropTarget(1, switchHit)) {
        LastSwitchHitTime = CurrentTime;
        if (BallFirstSwitchHitTime == 0) BallFirstSwitchHitTime = CurrentTime;
      }
      break;

    case SW_LL_DROP_1:
    case SW_LL_DROP_2:
    case SW_LL_DROP_3:
      if (HandleDropTarget(2, switchHit)) {
        LastSwitchHitTime = CurrentTime;
        if (BallFirstSwitchHitTime == 0) BallFirstSwitchHitTime = CurrentTime;
      }
      break;

    case SW_LR_DROP_1:
    case SW_LR_DROP_2:
    case SW_LR_DROP_3:
      if (HandleDropTarget(3, switchHit)) {
        LastSwitchHitTime = CurrentTime;
        if (BallFirstSwitchHitTime == 0) BallFirstSwitchHitTime = CurrentTime;
      }
      break;

    case SW_LOOP:
      if (LoopLitToQualifyLock) {
        QualifyNextPlayerLock();
        StartScoreAnimation(50000);
      } else {
        CurrentScores[CurrentPlayer] += PlayfieldMultiplier * 5000;
        PlaySoundEffect(SOUND_EFFECT_SWOOSH);
      }
      LastLoopHitTime = CurrentTime;
      LastSwitchHitTime = CurrentTime;
      if (BallFirstSwitchHitTime == 0) BallFirstSwitchHitTime = CurrentTime;
      break;

    case SW_POP_BUMPER:
      if (KingsChallengeRunning & KINGS_CHALLENGE_MELEE) {
        if (KingsChallengeBonus<500000) {
          PlaySoundEffect(SOUND_EFFECT_SINGLE_ANVIL);
          KingsChallengeBonus += KingsChallengeBonus/5;
          KingsChallengeBonus -= KingsChallengeBonus%50;
          KingsChallengeBonusChangedTime = CurrentTime;
        } else {
          PlaySoundEffect(SOUND_EFFECT_SWORD_1 + CurrentTime%7);
        }
      } else if (BallFirstSwitchHitTime) {
        LastPopBumperHit = CurrentTime;
        CurrentScores[CurrentPlayer] += 500 * PlayfieldMultiplier;
        PlaySoundEffect(SOUND_EFFECT_SLING_SHOT);
      }
      LastSwitchHitTime = CurrentTime;
      break;

    case SW_SPINNER:
      if (SpinnerStatus) {
        CurrentScores[CurrentPlayer] += 2500 * PlayfieldMultiplier;
        PlaySoundEffect(SOUND_EFFECT_SPINNER_LIT_2);
        SpinnerPhase += 1;
        if (SpinnerPhase >= NUM_LIT_SPINNER_SOUNDS) {
          SpinnerPhase = 0;
        }
        SpinnerLitUntil = CurrentTime + 1000;
        if (!BonusXCollectAvailable) SpinsTowardsNextGoal[CurrentPlayer] += 5;
      } else {
        CurrentScores[CurrentPlayer] += 100 * PlayfieldMultiplier;
        PlaySoundEffect(SOUND_EFFECT_SPINNER_UNLIT);
        if (!BonusXCollectAvailable) SpinsTowardsNextGoal[CurrentPlayer] += 1;
      }

      if (!BonusXCollectAvailable && (SpinsTowardsNextGoal[CurrentPlayer] > SpinnerGoal[CurrentPlayer])) {
        AwardSpinnerGoal();
      }

      LastSwitchHitTime = CurrentTime;
      LastSpinnerHit = CurrentTime;
      if (BallFirstSwitchHitTime == 0) BallFirstSwitchHitTime = CurrentTime;
      break;

    case SW_RIGHT_INSIDE_ROLLOVER:
      if (CurrentTime > (LastRightInlane + 250)) {
        SpinnerLitUntil = CurrentTime + 3000;
        SpinnerStatus = 1;
        LastSwitchHitTime = CurrentTime;
        if (BallFirstSwitchHitTime == 0) BallFirstSwitchHitTime = CurrentTime;
        if (LastTimeRightMagnetOn==0) {
          CurrentScores[CurrentPlayer] += PlayfieldMultiplier * 2000;
          AddToBonus(2);
        } else {
          CurrentScores[CurrentPlayer] += PlayfieldMultiplier * 10000;
          AddToBonus(5);
        }
        PlaySoundEffect(SOUND_EFFECT_CHURCH_BELL_1);
        LastRightInlane = CurrentTime;
      }
      LastSwitchHitTime = CurrentTime;
      break;

    case SW_LEFT_INSIDE_ROLLOVER:
      if (CurrentTime > (LastLeftInlane + 250)) {
        LastSwitchHitTime = CurrentTime;
        if (BallFirstSwitchHitTime == 0) BallFirstSwitchHitTime = CurrentTime;
        if (LastTimeLeftMagnetOn==0) {
          CurrentScores[CurrentPlayer] += PlayfieldMultiplier * 2000;
          AddToBonus(2);
        } else {
          CurrentScores[CurrentPlayer] += PlayfieldMultiplier * 10000;
          AddToBonus(5);
        }
        PlaySoundEffect(SOUND_EFFECT_CHURCH_BELL_1);
        LastLeftInlane = CurrentTime;
      }
      LastSwitchHitTime = CurrentTime;
      break;

    case SW_LOCK_3:
      LastSwitchHitTime = CurrentTime;
      if (BallFirstSwitchHitTime == 0) BallFirstSwitchHitTime = CurrentTime;
      CurrentScores[CurrentPlayer] += PlayfieldMultiplier * 1000;
      break;

    case SW_SAUCER:
      HandleSaucer();
      LastSwitchHitTime = CurrentTime;
      if (BallFirstSwitchHitTime == 0) BallFirstSwitchHitTime = CurrentTime;
      break;

    case SW_LEFT_OUTLANE:

      if ( (CurrentBallInPlay==BallsPerGame) && (LastChanceStatus[CurrentPlayer]&LAST_CHANCE_LEFT_QUALIFIED) && (PlayerLockStatus[CurrentPlayer]&LOCKS_ENGAGED_MASK)) {
        byte engaged = (PlayerLockStatus[CurrentPlayer] & LOCKS_ENGAGED_MASK) / 2;
        PlayerLockStatus[CurrentPlayer] = (PlayerLockStatus[CurrentPlayer] & (~LOCKS_ENGAGED_MASK)) | engaged;
        for (byte count = 0; count < 3; count++) {
          if (LockKickTime[count] == 0) {
            LockKickTime[count] = CurrentTime + 100;
          }
        }
        NumberOfBallsInPlay += 1;
        LastChanceStatus[CurrentPlayer] &= ~(LAST_CHANCE_LEFT_QUALIFIED);
        PlaySoundEffect(SOUND_EFFECT_FANFARE_2);

        if (DEBUG_MESSAGES) {
          char buf[255];
          sprintf(buf, "Last Chance: BIT=%d, ML=0x%02X, LS=0x%02X, Numlocks=%d, NumBIP=%d\n", CountBallsInTrough(), MachineLocks, PlayerLockStatus[CurrentPlayer], NumberOfBallsLocked, NumberOfBallsInPlay);
          Serial.write(buf);
        }

      } else {
        if (BallSaveEndTime) BallSaveEndTime += 3000;
        PlaySoundEffect(SOUND_EFFECT_OUTLANE_UNLIT);
        LastSwitchHitTime = CurrentTime;
        if (BallFirstSwitchHitTime == 0) BallFirstSwitchHitTime = CurrentTime;
      }
      AddToBonus(2);
      CurrentScores[CurrentPlayer] += PlayfieldMultiplier * 5000;
      break;

    case SW_RIGHT_OUTLANE:

      if ( (CurrentBallInPlay==BallsPerGame) && (LastChanceStatus[CurrentPlayer]&LAST_CHANCE_RIGHT_QUALIFIED) && (PlayerLockStatus[CurrentPlayer]&LOCKS_ENGAGED_MASK)) {
        byte engaged = (PlayerLockStatus[CurrentPlayer] & LOCKS_ENGAGED_MASK) / 2;
        PlayerLockStatus[CurrentPlayer] = (PlayerLockStatus[CurrentPlayer] & (~LOCKS_ENGAGED_MASK)) | engaged;
        for (byte count = 0; count < 3; count++) {
          if (LockKickTime[count] == 0) {
            LockKickTime[count] = CurrentTime + 100;
          }
        }
        NumberOfBallsInPlay += 1;
        LastChanceStatus[CurrentPlayer] &= ~(LAST_CHANCE_RIGHT_QUALIFIED);
        PlaySoundEffect(SOUND_EFFECT_FANFARE_2);
        if (DEBUG_MESSAGES) {
          char buf[255];
          sprintf(buf, "Last Chance: BIT=%d, ML=0x%02X, LS=0x%02X, Numlocks=%d, NumBIP=%d\n", CountBallsInTrough(), MachineLocks, PlayerLockStatus[CurrentPlayer], NumberOfBallsLocked, NumberOfBallsInPlay);
          Serial.write(buf);
        }
      } else {
        if (BallSaveEndTime) BallSaveEndTime += 3000;
        PlaySoundEffect(SOUND_EFFECT_OUTLANE_UNLIT);
        LastSwitchHitTime = CurrentTime;
        if (BallFirstSwitchHitTime == 0) BallFirstSwitchHitTime = CurrentTime;
      }
      AddToBonus(2);
      CurrentScores[CurrentPlayer] += PlayfieldMultiplier * 5000;      
      break;

    case SW_LEFT_RAMP_ROLLOVER:
      if (ExtraBallsOrSpecialAvailable[CurrentPlayer] & EBS_UPPER_EXTRA_BALL_AVAILABLE) {
        AwardExtraBall();
      }
      LastSwitchHitTime = CurrentTime;
      if (BallFirstSwitchHitTime == 0) BallFirstSwitchHitTime = CurrentTime;
      CurrentScores[CurrentPlayer] += PlayfieldMultiplier * 5000;
      break;
  }

}


int RunGamePlayMode(int curState, boolean curStateChanged) {
  int returnState = curState;
  unsigned long scoreAtTop = CurrentScores[CurrentPlayer];

  // Very first time into gameplay loop
  if (curState == MACHINE_STATE_INIT_GAMEPLAY) {
    returnState = InitGamePlay(curStateChanged);
  } else if (curState == MACHINE_STATE_INIT_NEW_BALL) {
    returnState = InitNewBall(curStateChanged);
  } else if (curState == MACHINE_STATE_NORMAL_GAMEPLAY) {
    returnState = ManageGameMode();
  } else if (curState == MACHINE_STATE_COUNTDOWN_BONUS) {
    returnState = CountdownBonus(curStateChanged);
    ShowPlayerScores(0xFF, false, false);
  } else if (curState == MACHINE_STATE_BALL_OVER) {
    RPU_SetDisplayCredits(Credits, !FreePlayMode);

    if (SamePlayerShootsAgain) {
      QueueNotification(SOUND_EFFECT_VP_SHOOT_AGAIN, 10);
      returnState = MACHINE_STATE_INIT_NEW_BALL;
    } else {

      CurrentPlayer += 1;
      if (CurrentPlayer >= CurrentNumPlayers) {
        CurrentPlayer = 0;
        CurrentBallInPlay += 1;
      }

      scoreAtTop = CurrentScores[CurrentPlayer];

      if (CurrentBallInPlay > BallsPerGame) {
        CheckHighScores();
        PlaySoundEffect(SOUND_EFFECT_GAME_OVER);
        for (int count = 0; count < CurrentNumPlayers; count++) {
          RPU_SetDisplay(count, CurrentScores[count], true, 2, true);
        }

        returnState = MACHINE_STATE_MATCH_MODE;
      }
      else returnState = MACHINE_STATE_INIT_NEW_BALL;
    }
  } else if (curState == MACHINE_STATE_MATCH_MODE) {
    returnState = ShowMatchSequence(curStateChanged);
  }

  UpdateLockStatus();
  MoveBallFromOutholeToRamp();

  byte switchHit;
  unsigned long lastBallFirstSwitchHitTime = BallFirstSwitchHitTime;

  while ( (switchHit = RPU_PullFirstFromSwitchStack()) != SWITCH_STACK_EMPTY ) {
    returnState = HandleSystemSwitches(curState, switchHit);
    if (NumTiltWarnings <= MaxTiltWarnings) HandleGamePlaySwitches(switchHit);
  }

  if (CreditResetPressStarted) {
    if (CurrentBallInPlay < 2) {
      // If we haven't finished the first ball, we can add players
      AddPlayer();
      if (DEBUG_MESSAGES) {
        Serial.write("Start game button pressed\n\r");
      }
      CreditResetPressStarted = 0;
    } else {
      if (RPU_ReadSingleSwitchState(SW_CREDIT_RESET)) {
        if (TimeRequiredToResetGame != 99 && (CurrentTime - CreditResetPressStarted) >= ((unsigned long)TimeRequiredToResetGame * 1000)) {
          // If the first ball is over, pressing start again resets the game
          if (Credits >= 1 || FreePlayMode) {
            if (!FreePlayMode) {
              Credits -= 1;
              RPU_WriteByteToEEProm(RPU_CREDITS_EEPROM_BYTE, Credits);
              RPU_SetDisplayCredits(Credits, !FreePlayMode);
            }
            returnState = MACHINE_STATE_INIT_GAMEPLAY;
            CreditResetPressStarted = 0;
          }
        }
      } else {
        CreditResetPressStarted = 0;
      }
    }

  }

  if (lastBallFirstSwitchHitTime == 0 && BallFirstSwitchHitTime != 0) {
    BallSaveEndTime = BallFirstSwitchHitTime + ((unsigned long)BallSaveNumSeconds) * 1000;
  }
  if (CurrentTime > (BallSaveEndTime + BALL_SAVE_GRACE_PERIOD)) {
    BallSaveEndTime = 0;
  }

  if (!ScrollingScores && CurrentScores[CurrentPlayer] > RPU_OS_MAX_DISPLAY_SCORE) {
    CurrentScores[CurrentPlayer] -= RPU_OS_MAX_DISPLAY_SCORE;
    if (!TournamentScoring) AddSpecialCredit();
  }

  if (scoreAtTop != CurrentScores[CurrentPlayer]) {
    SetLastTimeScoreChanged(CurrentTime);
    if (!TournamentScoring) {
      for (int awardCount = 0; awardCount < 3; awardCount++) {
        if (AwardScores[awardCount] != 0 && scoreAtTop < AwardScores[awardCount] && CurrentScores[CurrentPlayer] >= AwardScores[awardCount]) {
          // Player has just passed an award score, so we need to award it
          if (((ScoreAwardReplay >> awardCount) & 0x01)) {
            AddSpecialCredit();
          } else if (!ExtraBallCollected) {
            AwardExtraBall();
          }
        }
      }
    }

  }

  return returnState;
}


unsigned long LastLEDUpdateTime = 0;
byte LEDPhase = 0;
unsigned long NumLoops = 0;
unsigned long LastLoopReportTime = 0;

void loop() {

  /*
    if (DEBUG_MESSAGES) {
      NumLoops += 1;
      if (CurrentTime>(LastLoopReportTime+1000)) {
        LastLoopReportTime = CurrentTime;
        char buf[128];
        sprintf(buf, "Loop running at %lu Hz\n", NumLoops);
        Serial.write(buf);
        NumLoops = 0;
      }
    }
  */

  CurrentTime = millis();
  int newMachineState = MachineState;

  if (MachineState < 0) {
    newMachineState = RunSelfTest(MachineState, MachineStateChanged);
  } else if (MachineState == MACHINE_STATE_ATTRACT) {
    newMachineState = RunAttractMode(MachineState, MachineStateChanged);
  } else if (MachineState == MACHINE_STATE_DIAGNOSTICS) {
    newMachineState = RunDiagnosticsMode(MachineState, MachineStateChanged);
  } else {
    newMachineState = RunGamePlayMode(MachineState, MachineStateChanged);
  }

  if (newMachineState != MachineState) {
    MachineState = newMachineState;
    MachineStateChanged = true;
  } else {
    MachineStateChanged = false;
  }

  //  RPU_ApplyFlashToLamps(CurrentTime);
  //  RPU_UpdateTimedSolenoidStack(CurrentTime);
  //  RPU_UpdateTimedSoundStack(CurrentTime);
  RPU_Update(CurrentTime);
  Audio.Update(CurrentTime);

  /*
    if (LastLEDUpdateTime == 0 || (CurrentTime - LastLEDUpdateTime) > 250) {
      LastLEDUpdateTime = CurrentTime;
      RPU_SetBoardLEDs((LEDPhase % 8) == 1 || (LEDPhase % 8) == 3, (LEDPhase % 8) == 5 || (LEDPhase % 8) == 7);
      LEDPhase += 1;
    }
  */
}
