#ifndef _ELEMENT_H_
#define _ELEMENT_H_

#define NAME_DC              "dc"
#define NAME_FF              "ff"
#define NAME_INPUT           'i'
#define NAME_NOT             'n'
#define NAME_OR              'o'
#define NAME_AND             'a'
#define NAME_C               'c'
#define NAME_SEP             ','

#define NAME_REQ_IN          "r"
#define NAME_REQ_OUT         "r"
#define NAME_ACK_IN          "a"
#define NAME_ACK_OUT         "a"
#define NAME_SET             "s"
#define NAME_RESET           "r"
#define NAME_IN              "i"
#define NAME_OUT_0           "n"
#define NAME_OUT_1           "p"
#define NAME_REQ_WIRE        "req"
#define NAME_ACK_WIRE        "ack"
//#define PATTERN_REQ_IN       "r%d"
//#define PATTERN_ACK_IN       "a%d"
//#define PATTERN_SET_IN       "s%d"
//#define PATTERN_RESET_IN     "r%d"

#define PATTERN_ELEMENT      "%s_%s_%s"
#define PATTERN_DC           "dc_%s_%s"
#define PATTERN_FF           "ff_%s_%s"
#define PATTERN_INPUT        "i"
#define PATTERN_NOT          "n"
#define PATTERN_OR           "o%d"
#define PATTERN_AND          "a%d"
#define PATTERN_C            "c%d"
#define PATTERN_SEP          '_'
#define PATTERN_INVERTOR     "inv_%s"
#define PATTERN_WIRE         "%s_%s"
#define PATTERN_AND_2        "a2ii"

#define EL_TYPE              0
#define EL_REQ_SET           1
#define EL_ACK_RESET        -1


extern bool el_chk (char *name);
extern bool el_chk_type (char *name);
extern bool el_chk_logic (char *name);
extern char *el_get_logic (char *name, int input_type);
extern int el_count_trans (char *name, int input_type);
extern int el_count_fanin (char *name, int input_type);

#endif
