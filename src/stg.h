#ifndef _STG_H_
#define _STG_H_

#include <stdio.h>
#include "list.h"

#define STG_EMPTY                 0x00000000
#define STG_NATIVE                0x00000000
#define STG_TRIGGER               0x40000000
#define STG_GENERATED             0x80000000
#define STG_PROCESSED             0x10000000
#define STG_TRACED                0x20000000
#define STG_ALL                   0xFFFFFFFF

#define STG_INPUT                 0x00000001
#define STG_OUTPUT                0x00000002
#define STG_INTERNAL              0x00000004
#define STG_DUMMY                 0x00000008
#define STG_SIGNAL                0x00000100
#define STG_SIGNAL_INPUT          (STG_SIGNAL | STG_INPUT)
#define STG_SIGNAL_OUTPUT         (STG_SIGNAL | STG_OUTPUT)
#define STG_SIGNAL_INTERNAL       (STG_SIGNAL | STG_INTERNAL)
#define STG_SIGNAL_DUMMY          (STG_SIGNAL | STG_DUMMY)
#define STG_TRANSITION            0x00000200
#define STG_TRANSITION_INPUT      (STG_TRANSITION | STG_INPUT)
#define STG_TRANSITION_OUTPUT     (STG_TRANSITION | STG_OUTPUT)
#define STG_TRANSITION_INTERNAL   (STG_TRANSITION | STG_INTERNAL)
#define STG_TRANSITION_DUMMY      (STG_TRANSITION | STG_DUMMY)
#define STG_MANDATORY             0x00000010
#define STG_REDUNDANT             0x00000020
#define STG_PLACE                 0x00000400
#define STG_PLACE_SEPARATOR       (STG_PLACE | STG_MANDATORY)
#define STG_PLACE_REDUNDANT       (STG_PLACE_SEPARATOR | STG_REDUNDANT)
#define STG_PLACE_INPUT           (STG_PLACE | STG_INPUT)
#define STG_PLACE_OUTPUT          (STG_PLACE | STG_OUTPUT)
#define STG_PLACE_INTERNA         (STG_PLACE | STG_INTERNAL)
#define STG_PLACE_DUMMY           (STG_PLACE | STG_DUMMY)
#define STG_ARC                   0x00000800
#define STG_ARC_CONSUMING         (STG_ARC | 0x00000001)
#define STG_ARC_READING           (STG_ARC | 0x00000002)
#define STG_MARKING               0x00001000
#define STG_INITIAL               0x00002000
#define STG_INITIAL_LOW           (STG_INITIAL | 0x00000001)
#define STG_INITIAL_HIGH          (STG_INITIAL | 0x00000002)

#define SIGN_POSITIVE                  '+'
#define SIGN_NEGATIVE                  '-'
#define SIGN_NO_CARE                   '*'
#define SIGN_INDEX                     '/'
#define SIGN_DELIMITERS                "+-/"
#define SIGN_LOW                       '0'
#define SIGN_HIGH                      '1'
#define SIGN_UNDERSCORE                '_'
#define SIGN_EQUAL                     '='

#define PATTERN_TRANSITION_NAME        "t%.2d"
#define PATTERN_PLACE_NAME             "p%.2d"
#define PATTERN_TRANSITION_NAME_       "t_%s_%s"
#define PATTERN_PLACE_NAME_            "p_%s_%s"
#define PATTERN_TRANSITION_COMMENT     "%s,%s"
#define PATTERN_PLACE_COMMENT          "%s,%s"
#define PATTERN_ARC_NAME               "a%.3d"
#define PATTERN_ARC_COMMENT            "%s,%s"
#define PATTERN_MARKING_NAME           "m%.2d"
#define PATTERN_INITIAL_NAME_LOW       "!%s"
#define PATTERN_INITIAL_NAME_HIGH      "%s"
#define PATTERN_INITIAL_COMMENT_LOW    "%s=0"
#define PATTERN_INITIAL_COMMENT_HIGH   "%s=1"

#define PATTERN_DUMMY_PLUS   "_%s_plus"
#define PATTERN_DUMMY_MINUS  "_%s_minus"
#define PATTERN_NAME_LOW     "%s_0"
#define PATTERN_COMMENT_LOW  "%s=0"
#define PATTERN_NAME_HIGH    "%s_1"
#define PATTERN_COMMENT_HIGH "%s=1"
#define PATTERN_TRANSITION   "%s%c/%.2d"
#define PATTERN_PLACE        "%s_%.2d"
#define PATTERN_REDUNDANT    "_%s"
#define PATTERN_SIGNAL_TST   "tst_%s_%s"

#define CNV_MINUS             "_minus"
#define CNV_PLUS              "_plus"

#define _ERROR               0xFFFFFFFF
#define _UNDEFINED           0x00000000
#define _COMMENT             0x00000010
#define _KEY                 0x00000020
#define KEY_MODEL            0x00000040
#define KEY_SIGNAL           0x00000100                // = STG_SIGNAL
#define KEY_INPUTS           (KEY_SIGNAL | 0x00000001) // = STG_SIGNAL_INPUT
#define KEY_OUTPUTS          (KEY_SIGNAL | 0x00000002) // = STG_SIGNAL_OUTPUT
#define KEY_INTERNAL         (KEY_SIGNAL | 0x00000004) // = STG_SIGNAL_INTERNAL
#define KEY_DUMMY            (KEY_SIGNAL | 0x00000008) // = STG_SIGNAL_DUMMY
#define KEY_INITIAL_         0x00000200
#define KEY_INITIAL_STATE    KEY_INITIAL_ | 0x00000001
#define KEY_GRAPH            0x00000400
#define _ARCS                0x00000800
#define KEY_MARKING          0x00001000
#define KEY_MARKING_IMPLICIT 0x00001001
#define KEY_END              0x00010000

#define WORD_MODEL      ".model"
#define WORD_INPUTS     ".inputs"
#define WORD_OUTPUTS    ".outputs"
#define WORD_INTERNAL   ".internal"
#define WORD_DUMMY      ".dummy"
#define WORD_INITIAL_   ".initial"
#define WORD_STATE      "state"
#define WORD_GRAPH      ".graph"
#define WORD_MARKING    ".marking"
#define WORD_CAPACITY   ".capacity"
#define WORD_END        ".end"

#define STG_BUF_INC         128
#define STG_BUF_MAX         512

struct stg_t
{
  // List of signals:
  // data->type    - type of a signal
  // data->name    - name of a signal
  // data->comment - comment for a signal
  // data->pred    - reserved (NULL)
  // data->succ    - reserved (NULL)
  // data->link    - pointer to a initial state (i) node for a signal
  // index_0       - max index of minus transition (or dummy)
  // index_1       - max index of plus transition
  PNode s;

  // List of transitions:
  // data->type    - type of a transition
  // data->name    - name of a transition
  // data->comment - comment for a transition
  // data->pred    - list of predecessor places for a transition
  // data->succ    - list of successor places for a transition
  // data->link    - pointer to a signal (s) node for a transition
  // index_0       - reserved
  // index_1       - cluster index
  PNode t;

  // List of places:
  // data->type    - type of a place
  // data->name    - name of a place
  // data->comment - comment for a place
  // data->pred    - list of predecessor transitions for a place
  // data->succ    - list of successor transitions for a place
  // data->link    - pointer to a marking (m) node for a place
  // index_0       - reserved
  // index_1       - cluster index
  PNode p;

  // List of arcs:
  // data->type    - type of an arc
  // data->name    - name of an arc
  // data->comment - comment for an arc
  // data->pred    - from node (place for reading arc)
  // data->succ    - to node (transition for reading arc)
  // data->link    - reserved (NULL)
  // index_0       - reserved
  // index_1       - reserved
  PNode a;

  // List of initial markings:
  // data->type    - type of a marking
  // data->name    - name of a marking
  // data->comment - comment for a marking
  // data->pred    - reserved (NULL)
  // data->succ    - reserved (NULL)
  // data->link    - pointer to a place (p) node for a marking
  // index_0       - reserved
  // index_1       - reserved
  PNode m;

  // List of initial states:
  // data->type    - type of a initial state
  // data->name    - name of a initial state [SIGNAL_NAME | !SIGNAL_NAME]
  // data->comment - comment for a initial state [SIGNAL_NAME=0 | SIGNAL_NAME=1]
  // data->pred    - reserved (NULL)
  // data->succ    - reserved (NULL)
  // data->link    - pointer to a signal (s) node for a initial state
  // index_0       - reserved
  // index_1       - reserved
  PNode i;

  // name of STG
  char *model_name;
  int t_num;
  int p_num;
  int a_num;
  int m_num;
  int i_num;
};

extern stg_t *stg_create ();
extern int stg_destroy (stg_t *stg);
extern int stg_revert (stg_t *stg);
extern int stg_sort (stg_t *stg);
extern int stg_pred_succ_sort (stg_t *stg);
extern int stg_signal_level (stg_t *stg, PNode node_signal);
extern int stg_cluster (stg_t *stg);
extern PNode stg_cluster_type (stg_t *stg, int cluster_index);

// input-output
extern int stg_read (FILE *infile, stg_t *stg);
extern int stg_write (FILE *outfile, stg_t *stg);
extern int stg_debug (FILE *outfile, stg_t *stg, int mask = STG_ALL);

#endif
