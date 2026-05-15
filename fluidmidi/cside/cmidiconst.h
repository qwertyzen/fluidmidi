#ifndef __CMIDICONST_H__
#define __CMIDICONST_H__

// musiclab definitions
#define _DEFAULT_TICKS_PER_BEAT                  480
#define _CHAN_L                                 0x80
#define _CHAN_H                                 0xEF
#define _DEFAULT_NOTEVEL                          80
#define _DEFAULT_TSIG_CLOCKS_PER_CLICK            24
#define _DEFAULT_TSIG_32nd_NOTES_PER_BEAT          8
#define _DEFAULT_TEMPO_MAX                  16777216
#define _DEFAULT_NOTE_OFF_VEL                     64

// MIDI standard definitions
#define _FILE_HEADER          "MThd"
#define _TRACK_HEADER         "MTrk"

#define _NOTE_OFF             0x80
#define _NOTE_ON              0x90
#define _CONTROL_CHANGE       0xb0
#define _PROGRAM_CHANGE       0xc0
#define _PITCH_BEND           0xe0
#define _POLY_TOUCH           0xa0
#define _AFTER_TOUCH          0xd0

// Realtime messages
#define _RT_TIMING_CLOCK      0xF8
#define _RT_UNDEFINED         0xF8
#define _RT_START             0xFA
#define _RT_CONTINUE          0xFB
#define _RT_STOP              0xFC
#define _RT_UNDEFINED2        0xFD
#define _RT_ACTIVE_SENSING    0xFE
#define _RT_SYSTEM_RESET      0xFF

// Meta messages
#define _META_EVENT           0xFF

#define _SYSTEM_EXCLUSIVE     0xF0
#define _END_OF_EXCLUSIVE     0xF7

#define _ESCAPE_SEQUENCE      0xF7

// Meta message status codes
#define _SEQUENCE_NUMBER      0x00
#define _TEXT                 0x01
#define _COPYRIGHT            0x02
#define _TRACK_NAME           0x03
#define _INSTRUMENT_NAME      0x04
#define _LYRICS               0x05
#define _MARKER               0x06
#define _CUE_MARKER           0x07
#define _DEVICE_NAME          0x09
#define _CHANNEL_PREFIX       0x20
#define _MIDI_PORT            0x21
#define _END_OF_TRACK         0x2F
#define _TEMPO_SET            0x51
#define _SMPTE_OFFSET         0x54
#define _TIME_SIGNATURE       0x58
#define _KEY_SIGNATURE        0x59
#define _SEQUENCER_SPECIFIC   0x7F

// Other MIDI specific constants
#define _MIN_PITCHWHEEL       (-8192)
#define _MAX_PITCHWHEEL       (8191)

// Program change and Control names
extern const char* program_changes[128];
extern const char* control_changes[128];

// Control change specification
#define _HIRES_COMBINED_MAX                 (16384)
#define _HIRES_MSB_LSB_DIFF                 (32)

#define _CC_BANK_SELECT                     (0)
#define _CC_MODULATION_WHEEL                (1)
#define _CC_BREATH_CONTROLLER               (2)
#define _CC_FOOT_CONTROLLER                 (4)
#define _CC_PORTAMENTO_TIME                 (5)
#define _CC_DATA_ENTRY                      (6)
#define _CC_CHANNEL_VOLUME                  (7)
#define _CC_BALANCE                         (8)
#define _CC_PAN                             (10)
#define _CC_EXPRESSION_CONTROLLER           (11)
#define _CC_EFFECT_CONTROL_1                (12)
#define _CC_EFFECT_CONTROL_2                (13)
#define _CC_GEN_CONTROLLER_1                (16)
#define _CC_GEN_CONTROLLER_2                (17)
#define _CC_GEN_CONTROLLER_3                (18)
#define _CC_GEN_CONTROLLER_4                (19)
#define _CC_LSB_BANK_SELECT                 (32)
#define _CC_LSB_MODULATION_WHEEL            (33)
#define _CC_LSB_BREATH_CONTROLLER           (34)
#define _CC_LSB_FOOT_CONTROLLER             (36)
#define _CC_LSB_PORTAMENTO_TIME             (37)
#define _CC_LSB_DATA_ENTRY                  (38)
#define _CC_LSB_CHANNEL_VOLUME              (39)
#define _CC_LSB_BALANCE                     (40)
#define _CC_LSB_PAN                         (42)
#define _CC_LSB_EXPRESSION_CONTROLLER       (43)
#define _CC_LSB_EFFECT_CONTROL_1            (44)
#define _CC_LSB_EFFECT_CONTROL_2            (45)
#define _CC_LSB_GEN_CONTROLLER_1            (48)
#define _CC_LSB_GEN_CONTROLLER_2            (49)
#define _CC_LSB_GEN_CONTROLLER_3            (50)
#define _CC_LSB_GEN_CONTROLLER_4            (51)
#define _CC_SWITCH_SUSTAIN                  (64)
#define _CC_SWITCH_PORTAMENTO               (65)
#define _CC_SWITCH_SOSTENUTO                (66)
#define _CC_SWITCH_SOFT_PEDAL               (67)
#define _CC_SWITCH_LEGATO                   (68)
#define _CC_SWITCH_HOLD_2                   (69)
#define _CC_SOUND_CONTROLLER_1              (70)
#define _CC_SOUND_CONTROLLER_2              (71)
#define _CC_SOUND_CONTROLLER_3              (72)
#define _CC_SOUND_CONTROLLER_4              (73)
#define _CC_SOUND_CONTROLLER_5              (74)
#define _CC_SOUND_CONTROLLER_6              (75)
#define _CC_SOUND_CONTROLLER_7              (76)
#define _CC_SOUND_CONTROLLER_8              (77)
#define _CC_SOUND_CONTROLLER_9              (78)
#define _CC_SOUND_CONTROLLER_10             (79)
#define _CC_GEN_CONTROLLER_5                (80)
#define _CC_GEN_CONTROLLER_6                (81)
#define _CC_GEN_CONTROLLER_7                (82)
#define _CC_GEN_CONTROLLER_8                (83)
#define _CC_PORTAMENTO_CONTROL              (84)
#define _CC_HIGH_RES_VELOCITY_PREFIX        (88)
#define _CC_EFFECTS_1_DEPTH                 (91)
#define _CC_EFFECTS_2_DEPTH                 (92)
#define _CC_EFFECTS_3_DEPTH                 (93)
#define _CC_EFFECTS_4_DEPTH                 (94)
#define _CC_EFFECTS_5_DEPTH                 (95)
#define _CC_DATA_INCREMENT                  (96)
#define _CC_DATA_DECREMENT                  (97)
#define _CC_NRPN_LSB                        (98)
#define _CC_NRPN_MSB                        (99)
#define _CC_RPN_LSB                         (100)
#define _CC_RPN_MSB                         (101)
#define _CC_CH_ALL_SOUND_OFF                (120)
#define _CC_CH_RESET_ALL_CONTROLLERS        (121)
#define _CC_CH_LOCAL_CONTROL                (122)
#define _CC_CH_ALL_NOTES_OFF                (123)
#define _CC_CH_OMNI_OFF                     (124)
#define _CC_CH_OMNI_ON                      (125)
#define _CC_CH_MONO_ON                      (126)
#define _CC_CH_POLY_ON                      (127)
#define _CC_UNDEFINED_1                     (3)
#define _CC_UNDEFINED_2                     (9)
#define _CC_UNDEFINED_3                     (14)
#define _CC_UNDEFINED_4                     (15)
#define _CC_UNDEFINED_5                     (20)
#define _CC_UNDEFINED_6                     (21)
#define _CC_UNDEFINED_7                     (22)
#define _CC_UNDEFINED_8                     (23)
#define _CC_UNDEFINED_9                     (24)
#define _CC_UNDEFINED_10                    (25)
#define _CC_UNDEFINED_11                    (26)
#define _CC_UNDEFINED_12                    (27)
#define _CC_UNDEFINED_13                    (28)
#define _CC_UNDEFINED_14                    (29)
#define _CC_UNDEFINED_15                    (30)
#define _CC_UNDEFINED_16                    (31)
#define _CC_UNDEFINED_17                    (35)
#define _CC_UNDEFINED_18                    (90)
#define _CC_UNDEFINED_19                    (41)
#define _CC_UNDEFINED_20                    (46)
#define _CC_UNDEFINED_21                    (47)
#define _CC_UNDEFINED_22                    (52)
#define _CC_UNDEFINED_23                    (53)
#define _CC_UNDEFINED_24                    (54)
#define _CC_UNDEFINED_25                    (55)
#define _CC_UNDEFINED_26                    (56)
#define _CC_UNDEFINED_27                    (57)
#define _CC_UNDEFINED_28                    (58)
#define _CC_UNDEFINED_29                    (59)
#define _CC_UNDEFINED_30                    (60)
#define _CC_UNDEFINED_31                    (61)
#define _CC_UNDEFINED_32                    (62)
#define _CC_UNDEFINED_33                    (63)
#define _CC_UNDEFINED_34                    (85)
#define _CC_UNDEFINED_35                    (86)
#define _CC_UNDEFINED_36                    (86)
#define _CC_UNDEFINED_37                    (89)
#define _CC_UNDEFINED_38                    (90)
#define _CC_UNDEFINED_39                    (102)
#define _CC_UNDEFINED_40                    (103)
#define _CC_UNDEFINED_41                    (104)
#define _CC_UNDEFINED_42                    (105)
#define _CC_UNDEFINED_43                    (106)
#define _CC_UNDEFINED_44                    (107)
#define _CC_UNDEFINED_45                    (108)
#define _CC_UNDEFINED_46                    (109)
#define _CC_UNDEFINED_47                    (110)
#define _CC_UNDEFINED_48                    (111)
#define _CC_UNDEFINED_49                    (112)
#define _CC_UNDEFINED_50                    (113)
#define _CC_UNDEFINED_51                    (114)
#define _CC_UNDEFINED_52                    (115)
#define _CC_UNDEFINED_53                    (116)
#define _CC_UNDEFINED_54                    (117)
#define _CC_UNDEFINED_55                    (118)

#endif  // __CMIDICONST_H__
