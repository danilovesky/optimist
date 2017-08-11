#ifndef _STG_PROCS_H_
#define _STG_PROCS_H_

#include "stg.h"
#include "list.h"

extern int stg_fill_pred (stg_t *stg, PNode node);
extern int stg_fill_preds (stg_t *stg, PNode head);
extern int stg_fill_succ (stg_t *stg, PNode node);
extern int stg_fill_succs (stg_t *stg, PNode head);
extern int stg_fill_link_t (stg_t *stg, PNode node);
extern int stg_fill_link_p (stg_t *stg, PNode node);
extern int stg_fill_s (stg_t *stg);
extern int stg_fill_t (stg_t *stg);
extern int stg_fill_p (stg_t *stg);
extern int stg_fill_i (stg_t *stg);
extern int stg_fill (stg_t *stg);
extern int stg_type_and (PNode head, int mask);
extern int stg_type_or (PNode head, int mask);
extern char *stg_cnv_signal (char *name);
extern char *stg_cnv_dummy (char *name);
extern char *stg_cnv_redundant (char *name);
extern char *stg_cnv_mandatory (char *name);
extern char *stg_cnv_var (char *name);
extern char *stg_cnv_name (char *name, char sign = '*', int index = 0);
extern char stg_cnv_polarity (char *name);
extern int stg_cnv_index (char *name);
extern char *stg_cnv_invert (char *name);
extern int stg_index_compress (stg_t *stg, PNode head);
extern int stg_names_compress (stg_t *stg);
extern int stg_compress_places(stg_t *stg);
extern int stg_compress_transitions(stg_t *stg);
extern int stg_compress_arcs(stg_t *stg);
extern int stg_compress(stg_t *stg);

#endif
