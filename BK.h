 
#define LAMP_HEAD_SHOOT_AGAIN       0
#define LAMP_HEAD_BALL_IN_PLAY      1
#define LAMP_HEAD_TILT              2
#define LAMP_HEAD_GAME_OVER         3
#define LAMP_HEAD_MATCH             4
#define LAMP_HEAD_HIGH_SCORE_TO_DATE  5
//#define LAMP_LEFT_SAUCER            6
#define LAMP_HEAD_TIMER_BONUS_BALL  7
#define LAMP_RIGHT_MAGNASAVE        8
#define LAMP_LEFT_MAGNASAVE         9
#define LAMP_LEFT_OUTLANE           10
#define LAMP_RIGHT_OUTLANE          11
#define LAMP_SPINNER                12
#define LAMP_MIDDLE_RAMP            13
#define LAMP_RIGHT_INLANE           14
#define LAMP_LEFT_INLANE            15
#define LAMP_LOWER_LEFT_DROPS       16
#define LAMP_LOWER_RIGHT_DROPS      17
#define LAMP_UPPER_LEFT_DROPS       18
#define LAMP_UPPER_RIGHT_DROPS      19
#define LAMP_LOCK_2                 20
#define LAMP_LOOP_SPECIAL           21
#define LAMP_LOOP_EXTRA_BALL        22
#define LAMP_SAUCER                 23
#define LAMP_LLDROPS_1              24
#define LAMP_LLDROPS_2              25
#define LAMP_LLDROPS_3              26
#define LAMP_DOUBLE_SCORING         27
#define LAMP_LRDROPS_3              28
#define LAMP_LRDROPS_2              29
#define LAMP_LRDROPS_1              30
#define LAMP_TRIPLE_SCORING         31
#define LAMP_ULDROPS_3              32
#define LAMP_ULDROPS_2              33
#define LAMP_ULDROPS_1              34
#define LAMP_POP_BUMPER             35
#define LAMP_URDROPS_3              36
#define LAMP_URDROPS_2              37
#define LAMP_URDROPS_1              38
#define LAMP_LOCK_3                 39
#define LAMP_UPPER_EXTRA_BALL       40
#define LAMP_LOCK_1                 41

#define LAMP_SHOOT_AGAIN            46
#define LAMP_BONUS_1                47
#define LAMP_BONUS_2                48
#define LAMP_BONUS_3                49
#define LAMP_BONUS_4                50
#define LAMP_BONUS_5                51
#define LAMP_BONUS_6                52
#define LAMP_BONUS_7                53
#define LAMP_BONUS_8                54
#define LAMP_BONUS_9                55
#define LAMP_BONUS_10               56
#define LAMP_BONUS_20               57
#define LAMP_BONUS_30               58
#define LAMP_BONUS_40               59
#define LAMP_2X                     60
#define LAMP_3X                     61
#define LAMP_4X                     62
#define LAMP_5X                     63
#define LAMP_APRON_CREDITS                  55


#define SW_PLUMB_TILT             0
#define SW_ROLL_TILT              1
#define SW_CREDIT_RESET           2
#define SW_COIN_1                 3
#define SW_COIN_2                 4
#define SW_COIN_3                 5
#define SW_SLAM                   6
#define SW_HIGH_SCORE_RESET       7 
#define SW_RIGHT_MAGNET_BUTTON    8
#define SW_LEFT_MAGNET_BUTTON     9
#define SW_LEFT_OUTLANE           10
#define SW_RIGHT_OUTLANE          11
#define SW_SPINNER                12
#define SW_RIGHT_RAMP_ROLLUNDER   13
#define SW_RIGHT_INSIDE_ROLLOVER  14
#define SW_LEFT_INSIDE_ROLLOVER   15
#define SW_RIGHT_BALL_RAMP        16
#define SW_CENTER_BALL_RAMP       17
#define SW_LEFT_BALL_RAMP         18
#define SW_OUTHOLE                19
#define SW_LEFT_SLING             20
#define SW_RIGHT_SLING            21
#define SW_LOOP                   22
#define SW_SAUCER                 23
#define SW_LL_DROP_1              24
#define SW_LL_DROP_2              25
#define SW_LL_DROP_3              26
#define NO_SW_LL_DROP_STANDUP     27
#define SW_LR_DROP_3              28
#define SW_LR_DROP_2              29
#define SW_LR_DROP_1              30
#define NO_SW_LR_DROP_STANDUP     31
#define SW_UL_DROP_1              32
#define SW_UL_DROP_2              33
#define SW_UL_DROP_3              34
#define SW_POP_BUMPER             35
#define SW_UR_DROP_1              36
#define SW_UR_DROP_2              37
#define SW_UR_DROP_3              38
#define NO_SW_UR_DROP_STANDUP     39
#define SW_LOCK_1                 40
#define SW_LOCK_2                 41
#define SW_LOCK_3                 42
#define SW_LEFT_RAMP_ROLLOVER     43
#define SW_SHOOTER_LANE           44
#define SW_PLAYFIELD_TILT         45


#define SOL_OUTHOLE                 0
#define SOL_LL_DROP_RESET           1
#define SOL_LR_DROP_RESET           2
#define SOL_UL_DROP_RESET           3
#define SOL_UR_DROP_RESET           4
#define SOL_BALL_RAMP_THROWER       5
#define SOL_UPPER_BALL_EJECT        6
#define SOL_SAUCER                  7
#define SOL_RIGHT_MAGNA_SAVE        8
#define SOL_LEFT_MAGNA_SAVE         9
#define SOL_GI_RELAY                10
#define SOL_BELL                    14
#define SOLCONT_COIN_LOCKOUT        15

#define SOL_LEFT_SLING              16
#define SOL_RIGHT_SLING             17 
#define SOL_POP_BUMPER              18


#define MAX_COMBO_STEPS 4

#define COMBO_STEP_TYPE_STRICT_SWITCH             0   /* Only one switch can be hit, or combo is broken */
#define COMBO_STEP_TYPE_LIBERAL_SWITCH            1   /* The given switch extends the combo, but others don't break it */
#define COMBO_STEP_TYPE_STRICT_SWITCH_AND_TIME    2   /* Only one switch can be hit, and it has to be hit in given amount of time */
#define COMBO_STEP_TYPE_LIBERAL_SWITCH_AND_TIME   3   /* The given switch extends the combo for the given amount of time */

struct ComboSteps {
  byte comboStepType;
  byte comboSwitch;
  unsigned long comboTimeConstraint;
};

struct ComboShot {  

  // These variables are initialized to define the combo
  ComboSteps                *steps = NULL;
  byte                      numSteps;
  unsigned long             firstTimePoints;
  unsigned long             subsequentTimesPoints;
  unsigned long             achievementFlag;
  byte                      trackingStep = 0;
  boolean                   *trackingVariable = NULL;

  // These variables track the current progress of the combo
  byte                      currentStep;
  unsigned long             currentTimeout;  
};

#define COMBO_RIGHT_INLANE_SPINNER                    0x0001
#define COMBO_RIGHT_INLANE_SPINNER_LEFT_RAMP          0x0002
#define COMBO_RIGHT_INLANE_SPINNER_LEFT_RAMP_LOCK     0x0004
#define COMBO_RIGHT_INLANE_LOOP                       0x0008
#define COMBO_LEFT_INLANE_RAMP                        0x0010
#define COMBO_LEFT_INLANE_SAUCER                      0x0020
#define COMBO_SAUCER_RAMP                             0x0040
#define COMBO_SAUCER_SAUCER                           0x0080
#define COMBO_LOOP_LEFT_INLANE                        0x0100
ComboSteps BKComboSteps[24]; // All the steps for all the combos
ComboShot BKCombos[10];

// Variables tracking the status of 
// combos are used for lamps & scoring
boolean SpinnerLitFromCombo = false;
boolean RightRampLitFromCombo = false;
boolean SaucerLitFromCombo = false;
boolean LockLitFromCombo = false;
boolean LoopLitFromCombo = false;
boolean LeftInlaneLitFromLoop = false;


byte InitBKCombosArray() {
  byte stepIndex = 0;
  byte comboIndex = 0;

  // Combo = right lane & spinner
  // COMBO 0
  BKCombos[comboIndex].steps = &BKComboSteps[stepIndex];
  BKCombos[comboIndex].numSteps = 2;
  BKCombos[comboIndex].currentStep = 0;
  BKCombos[comboIndex].currentTimeout = 0;
  BKCombos[comboIndex].firstTimePoints = 50000;
  BKCombos[comboIndex].subsequentTimesPoints = 5000;
  BKCombos[comboIndex].achievementFlag = COMBO_RIGHT_INLANE_SPINNER;
  BKCombos[comboIndex].trackingStep = 0; 
  BKCombos[comboIndex].trackingVariable = &SpinnerLitFromCombo;
  BKComboSteps[stepIndex].comboStepType = COMBO_STEP_TYPE_STRICT_SWITCH_AND_TIME;
  BKComboSteps[stepIndex].comboSwitch = SW_RIGHT_INSIDE_ROLLOVER;
  BKComboSteps[stepIndex].comboTimeConstraint = 3000;
  stepIndex += 1;
  BKComboSteps[stepIndex].comboStepType = COMBO_STEP_TYPE_LIBERAL_SWITCH_AND_TIME;
  BKComboSteps[stepIndex].comboSwitch = SW_SPINNER;
  BKComboSteps[stepIndex].comboTimeConstraint = 1000;
  stepIndex += 1;
  comboIndex += 1;

  // Combo = right lane, spinner, left ramp
  // COMBO 1
  BKCombos[comboIndex].steps = &BKComboSteps[stepIndex];
  BKCombos[comboIndex].numSteps = 3;
  BKCombos[comboIndex].currentStep = 0;
  BKCombos[comboIndex].currentTimeout = 0;
  BKCombos[comboIndex].firstTimePoints = 100000;
  BKCombos[comboIndex].subsequentTimesPoints = 25000;
  BKCombos[comboIndex].achievementFlag = COMBO_RIGHT_INLANE_SPINNER_LEFT_RAMP;
  BKCombos[comboIndex].trackingStep = 0; 
  BKCombos[comboIndex].trackingVariable = NULL;
  BKComboSteps[stepIndex].comboStepType = COMBO_STEP_TYPE_STRICT_SWITCH_AND_TIME;
  BKComboSteps[stepIndex].comboSwitch = SW_RIGHT_INSIDE_ROLLOVER;
  BKComboSteps[stepIndex].comboTimeConstraint = 3000;
  stepIndex += 1;
  BKComboSteps[stepIndex].comboStepType = COMBO_STEP_TYPE_LIBERAL_SWITCH_AND_TIME;
  BKComboSteps[stepIndex].comboSwitch = SW_SPINNER;
  BKComboSteps[stepIndex].comboTimeConstraint = 1000;
  stepIndex += 1;
  BKComboSteps[stepIndex].comboStepType = COMBO_STEP_TYPE_LIBERAL_SWITCH_AND_TIME;
  BKComboSteps[stepIndex].comboSwitch = SW_LEFT_RAMP_ROLLOVER;
  BKComboSteps[stepIndex].comboTimeConstraint = 1000;
  stepIndex += 1;
  comboIndex += 1;

  // Combo = right lane, spinner, left ramp, lock
  // COMBO 2
  BKCombos[comboIndex].steps = &BKComboSteps[stepIndex];
  BKCombos[comboIndex].numSteps = 4;
  BKCombos[comboIndex].currentStep = 0;
  BKCombos[comboIndex].currentTimeout = 0;
  BKCombos[comboIndex].firstTimePoints = 500000;
  BKCombos[comboIndex].subsequentTimesPoints = 100000;
  BKCombos[comboIndex].achievementFlag = COMBO_RIGHT_INLANE_SPINNER_LEFT_RAMP_LOCK;
  BKCombos[comboIndex].trackingStep = 2; 
  BKCombos[comboIndex].trackingVariable = &LockLitFromCombo;
  BKComboSteps[stepIndex].comboStepType = COMBO_STEP_TYPE_STRICT_SWITCH_AND_TIME;
  BKComboSteps[stepIndex].comboSwitch = SW_RIGHT_INSIDE_ROLLOVER;
  BKComboSteps[stepIndex].comboTimeConstraint = 3000;
  stepIndex += 1;
  BKComboSteps[stepIndex].comboStepType = COMBO_STEP_TYPE_LIBERAL_SWITCH_AND_TIME;
  BKComboSteps[stepIndex].comboSwitch = SW_SPINNER;
  BKComboSteps[stepIndex].comboTimeConstraint = 1000;
  stepIndex += 1;
  BKComboSteps[stepIndex].comboStepType = COMBO_STEP_TYPE_LIBERAL_SWITCH_AND_TIME;
  BKComboSteps[stepIndex].comboSwitch = SW_LEFT_RAMP_ROLLOVER;
  BKComboSteps[stepIndex].comboTimeConstraint = 2000;
  stepIndex += 1;
  BKComboSteps[stepIndex].comboStepType = COMBO_STEP_TYPE_LIBERAL_SWITCH_AND_TIME;
  BKComboSteps[stepIndex].comboSwitch = SW_LOCK_3;
  BKComboSteps[stepIndex].comboTimeConstraint = 2000;
  stepIndex += 1;
  comboIndex += 1;
  
  // Combo = right lane, loop
  // COMBO 3
  BKCombos[comboIndex].steps = &BKComboSteps[stepIndex];
  BKCombos[comboIndex].numSteps = 2;
  BKCombos[comboIndex].currentStep = 0;
  BKCombos[comboIndex].currentTimeout = 0;
  BKCombos[comboIndex].firstTimePoints = 35000;
  BKCombos[comboIndex].subsequentTimesPoints = 15000;
  BKCombos[comboIndex].achievementFlag = COMBO_RIGHT_INLANE_LOOP;
  BKCombos[comboIndex].trackingStep = 0; 
  BKCombos[comboIndex].trackingVariable = &LoopLitFromCombo;
  BKComboSteps[stepIndex].comboStepType = COMBO_STEP_TYPE_STRICT_SWITCH_AND_TIME;
  BKComboSteps[stepIndex].comboSwitch = SW_RIGHT_INSIDE_ROLLOVER;
  BKComboSteps[stepIndex].comboTimeConstraint = 4000;
  stepIndex += 1;
  BKComboSteps[stepIndex].comboStepType = COMBO_STEP_TYPE_LIBERAL_SWITCH;
  BKComboSteps[stepIndex].comboSwitch = SW_LOOP;
  BKComboSteps[stepIndex].comboTimeConstraint = 0;
  stepIndex += 1;
  comboIndex += 1;

  // Combo = left lane & ramp
  // COMBO 4
  BKCombos[comboIndex].steps = &BKComboSteps[stepIndex];
  BKCombos[comboIndex].numSteps = 2;
  BKCombos[comboIndex].currentStep = 0;
  BKCombos[comboIndex].currentTimeout = 0;
  BKCombos[comboIndex].firstTimePoints = 35000;
  BKCombos[comboIndex].subsequentTimesPoints = 50000;
  BKCombos[comboIndex].achievementFlag = COMBO_LEFT_INLANE_RAMP;
  BKCombos[comboIndex].trackingStep = 0; 
  BKCombos[comboIndex].trackingVariable = &RightRampLitFromCombo;
  BKComboSteps[stepIndex].comboStepType = COMBO_STEP_TYPE_STRICT_SWITCH_AND_TIME;
  BKComboSteps[stepIndex].comboSwitch = SW_LEFT_INSIDE_ROLLOVER;
  BKComboSteps[stepIndex].comboTimeConstraint = 1500;
  stepIndex += 1;
  BKComboSteps[stepIndex].comboStepType = COMBO_STEP_TYPE_STRICT_SWITCH_AND_TIME;
  BKComboSteps[stepIndex].comboSwitch = SW_RIGHT_RAMP_ROLLUNDER;
  BKComboSteps[stepIndex].comboTimeConstraint = 250;
  stepIndex += 1;
  comboIndex += 1;

  // Combo = left lane & saucer
  // COMBO 5
  BKCombos[comboIndex].steps = &BKComboSteps[stepIndex];
  BKCombos[comboIndex].numSteps = 2;
  BKCombos[comboIndex].currentStep = 0;
  BKCombos[comboIndex].currentTimeout = 0;
  BKCombos[comboIndex].firstTimePoints = 20000;
  BKCombos[comboIndex].subsequentTimesPoints = 30000;
  BKCombos[comboIndex].achievementFlag = COMBO_LEFT_INLANE_SAUCER;
  BKCombos[comboIndex].trackingStep = 0; 
  BKCombos[comboIndex].trackingVariable = &SaucerLitFromCombo;
  BKComboSteps[stepIndex].comboStepType = COMBO_STEP_TYPE_STRICT_SWITCH_AND_TIME;
  BKComboSteps[stepIndex].comboSwitch = SW_LEFT_INSIDE_ROLLOVER;
  BKComboSteps[stepIndex].comboTimeConstraint = 1500;
  stepIndex += 1;
  BKComboSteps[stepIndex].comboStepType = COMBO_STEP_TYPE_STRICT_SWITCH_AND_TIME;
  BKComboSteps[stepIndex].comboSwitch = SW_SAUCER;
  BKComboSteps[stepIndex].comboTimeConstraint = 250;
  stepIndex += 1;
  comboIndex += 1;
  
  // Combo = saucer & saucer again
  // COMBO 6
  BKCombos[comboIndex].steps = &BKComboSteps[stepIndex];
  BKCombos[comboIndex].numSteps = 2;
  BKCombos[comboIndex].currentStep = 0;
  BKCombos[comboIndex].currentTimeout = 0;
  BKCombos[comboIndex].firstTimePoints = 30000;
  BKCombos[comboIndex].subsequentTimesPoints = 40000;
  BKCombos[comboIndex].achievementFlag = COMBO_SAUCER_SAUCER;
  BKCombos[comboIndex].trackingStep = 0; 
  BKCombos[comboIndex].trackingVariable = &SaucerLitFromCombo;
  BKComboSteps[stepIndex].comboStepType = COMBO_STEP_TYPE_STRICT_SWITCH_AND_TIME;
  BKComboSteps[stepIndex].comboSwitch = SW_SAUCER;
  BKComboSteps[stepIndex].comboTimeConstraint = 1500;
  stepIndex += 1;
  BKComboSteps[stepIndex].comboStepType = COMBO_STEP_TYPE_STRICT_SWITCH_AND_TIME;
  BKComboSteps[stepIndex].comboSwitch = SW_SAUCER;
  BKComboSteps[stepIndex].comboTimeConstraint = 250;
  stepIndex += 1;
  comboIndex += 1;

    // Combo = LOOP to left inlane
  // COMBO 7
  BKCombos[comboIndex].steps = &BKComboSteps[stepIndex];
  BKCombos[comboIndex].numSteps = 2;
  BKCombos[comboIndex].currentStep = 0;
  BKCombos[comboIndex].currentTimeout = 0;
  BKCombos[comboIndex].firstTimePoints = 50000;
  BKCombos[comboIndex].subsequentTimesPoints = 20000;
  BKCombos[comboIndex].achievementFlag = COMBO_LOOP_LEFT_INLANE;
  BKCombos[comboIndex].trackingStep = 0; 
  BKCombos[comboIndex].trackingVariable = &LeftInlaneLitFromLoop;
  BKComboSteps[stepIndex].comboStepType = COMBO_STEP_TYPE_STRICT_SWITCH_AND_TIME;
  BKComboSteps[stepIndex].comboSwitch = SW_LOOP;
  BKComboSteps[stepIndex].comboTimeConstraint = 1500;
  stepIndex += 1;
  BKComboSteps[stepIndex].comboStepType = COMBO_STEP_TYPE_STRICT_SWITCH_AND_TIME;
  BKComboSteps[stepIndex].comboSwitch = SW_LEFT_INSIDE_ROLLOVER;
  BKComboSteps[stepIndex].comboTimeConstraint = 250;
  stepIndex += 1;
  comboIndex += 1;
  
  return comboIndex;
}
