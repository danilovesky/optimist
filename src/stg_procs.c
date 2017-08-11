#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "debug.h"
#include "list.h"
#include "stg.h"
#include "stg_procs.h"

/* === */
int stg_fill_pred (stg_t *stg, PNode node)
{
  Verbose (DEBUG_STACK, "--> stg_fill_pred");
  PNode node_arc;
  PData mask;

  NodeFree (node->data->pred);
  mask = DataCreate (STG_ARC_CONSUMING, NULL, NULL, NULL, node);
  node_arc = stg->a;
  while ((node_arc = NodeFind (node_arc, mask)) != NULL)
  {
    NodeAdd (node->data->pred, NodeCreate (node_arc->data->pred->data));
    node_arc = node_arc->next;
  }
  DataFree (mask);
  Verbose (DEBUG_STACK, "<-- stg_fill_pred");
  return 1;
}


/* === */
int stg_fill_preds (stg_t *stg, PNode head)
{
  Verbose (DEBUG_STACK, "--> stg_fill_preds");
  PNode node;

  node = head;
  while (node != NULL)
  {
    stg_fill_pred (stg, node);
    node = node->next;
  }
  Verbose (DEBUG_STACK, "<-- stg_fill_preds");
  return 1;
}


/* === */
int stg_fill_succ (stg_t *stg, PNode node)
{
  Verbose (DEBUG_STACK, "--> stg_fill_succ");
  PNode node_arc;
  PData mask;

  NodeFree (node->data->succ);
  mask = DataCreate (STG_ARC_CONSUMING, NULL, NULL, node, NULL);
  node_arc = stg->a;
  while ((node_arc = NodeFind (node_arc, mask)) != NULL)
  {
    NodeAdd (node->data->succ, NodeCreate (node_arc->data->succ->data));
    node_arc = node_arc->next;
  }
  DataFree (mask);
  Verbose (DEBUG_STACK, "<-- stg_fill_succ");
  return 1;
}


/* === */
int stg_fill_succs (stg_t *stg, PNode head)
{
  Verbose (DEBUG_STACK, "--> stg_fill_succs");
  PNode node;

  node = head;
  while (node != NULL)
  {
    stg_fill_succ (stg, node);
    node = node->next;
  }
  Verbose (DEBUG_STACK, "<-- stg_fill_succs");
  return 1;
}


/* === */
int stg_fill_link_p (stg_t *stg, PNode node)
{
  Verbose (DEBUG_STACK, "--> stg_fill_link_p");
  PData mask;
  mask = DataCreate (STG_MARKING, NULL, NULL, NULL, NULL, node);
  node->data->link = NodeFind (stg->m, mask);
  DataFree (mask);
  Verbose (DEBUG_STACK, "<-- stg_fill_link_p");
  return 1;
}


/* === */
int stg_fill_link_t (stg_t *stg, PNode node)
{
  Verbose (DEBUG_STACK, "--> stg_fill_link_t");
  PData mask;
  mask = DataCreate (STG_SIGNAL, stg_cnv_signal (node->data->name), NULL);
  node->data->link = NodeFind (stg->s, mask);
  DataFree (mask);
  Verbose (DEBUG_STACK, "<-- stg_fill_link_t");
  return 1;
}


/* === */
int stg_fill_p (stg_t *stg)
{
  Verbose (DEBUG_STACK, "--> stg_fill_p");
  PNode node;
  stg_fill_preds (stg, stg->p);
  stg_fill_succs (stg, stg->p);
  node = stg->p;
  while (node != NULL)
  {
    stg_fill_link_p (stg, node);
    node = node->next;
  }
  Verbose (DEBUG_STACK, "<-- stg_fill_p");
  return 1;
}


/* === */
int stg_fill_t (stg_t *stg)
{
  Verbose (DEBUG_STACK, "--> stg_fill_t");
  PNode node;
  stg_fill_preds (stg, stg->t);
  stg_fill_succs (stg, stg->t);
  node = stg->t;
  while (node != NULL)
  {
    stg_fill_link_t (stg, node);
    node = node->next;
  }
  Verbose (DEBUG_STACK, "<-- stg_fill_t");
  return 1;
}


/* === */
int stg_fill_s (stg_t *stg)
{
  Verbose (DEBUG_STACK, "--> stg_fill_s");
  PNode node;
  node = stg->i;
  while (node != NULL)
  {
    if (node->data->link != NULL)
        node->data->link->data->link = node;
    node = node->next;
  }
  Verbose (DEBUG_STACK, "<-- stg_fill_s");
  return 1;
}


/* === */
int stg_fill_i (stg_t *stg)
{
  Verbose (DEBUG_STACK, "--> stg_fill_i");
  PNode node_signal = NULL;
  PNode node_init   = NULL;
  char *buf_name    = NULL;
  char *buf_comment = NULL;

  buf_name = (char *) malloc (STG_BUF_MAX);
  buf_comment = (char *) malloc (STG_BUF_MAX);

  node_signal = stg->s;
  while (node_signal != NULL)
  {
    if ((node_signal->data->type & STG_DUMMY) != STG_DUMMY)
      if (node_signal->data->link == NULL)
        switch (stg_signal_level (stg, node_signal))
        {
          case 0:
            sprintf (buf_name, PATTERN_INITIAL_NAME_LOW, node_signal->data->name);
            sprintf (buf_comment, PATTERN_INITIAL_COMMENT_LOW, node_signal->data->name);
            node_init = NodeCreate (DataCreate (STG_INITIAL_LOW, buf_name, buf_comment, NULL, NULL, node_signal));
            NodeAdd (stg->i, node_init);
            node_signal->data->link = node_init;
            break;
          case 1:
            sprintf (buf_name, PATTERN_INITIAL_NAME_HIGH, node_signal->data->name);
            sprintf (buf_comment, PATTERN_INITIAL_COMMENT_HIGH, node_signal->data->name);
            node_init = NodeCreate (DataCreate (STG_INITIAL_HIGH, buf_name, buf_comment, NULL, NULL, node_signal));
            NodeAdd (stg->i, node_init);
            node_signal->data->link = node_init;
            break;
        }
    node_signal = node_signal->next;
  }
  free (buf_name);
  free (buf_comment);
  Verbose (DEBUG_STACK, "<-- stg_fill_i");
  return 1;
}


/* === */
int stg_fill (stg_t *stg)
{
  Verbose (DEBUG_STACK, "--> stg_fill");
  int old_debug_level = _debug_level;
  _debug_level = 0;
  stg_fill_p (stg);
  stg_fill_t (stg);
  stg_fill_s (stg);
  _debug_level = old_debug_level;
  Verbose (DEBUG_STACK, "<-- stg_fill");
  return 1;
}


/* === */
int stg_type_and (PNode head, int mask)
{
  Verbose (DEBUG_STACK, "--> stg_type_and");
  PNode node;
  node = head;
  while (node != NULL)
  {
    node->data->type &= mask;
    node = node->next;
  }
  Verbose (DEBUG_STACK, "<-- stg_type_and");
  return 1;
}


/* === */
int stg_type_or (PNode head, int mask)
{
  Verbose (DEBUG_STACK, "--> stg_type_or");
  PNode node;
  node = head;
  while (node != NULL)
  {
    node->data->type |= mask;
    node = node->next;
  }
  Verbose (DEBUG_STACK, "<-- stg_type_or");
  return 1;
}
/* PROCS END */


/* CONVERT BEGIN */
/* === */
char *stg_cnv_signal (char *name)
{
//  Verbose (DEBUG_STACK, "--> stg_cnv_signal");
  int i;
  char *s;

  i = strcspn (name, SIGN_DELIMITERS);
  if (name[0] == '!')
  {
    i--;
    s = (char *) malloc (i+1);
    strncpy (s, &name[1], i);
    s[i] = '\0';
  }
  else
  {
    s = (char *) malloc (i+1);
    strncpy (s, &name[0], i);
    s[i] = '\0';
  }
//  Verbose (DEBUG_LOCAL, "name = %s  i = %d  s = %s", name, i, s);
//  Verbose (DEBUG_STACK, "<-- stg_cnv_signal");
  return s;
}


/*===*/
char *stg_cnv_dummy (char *name)
{
//  Verbose (DEBUG_STACK, "--> stg_cnv_dummy (name = %s)", name);
  int i, j, k;
  char *s = NULL;
  char *s_ = NULL;

  s = (char *) malloc (strlen (name) + 2);
  s[0] = SIGN_UNDERSCORE;
  strcpy (&s[1], name);

  while (strchr (s, SIGN_POSITIVE) != NULL)
  {
    s = (char *) realloc (s, strlen (s) + strlen (CNV_MINUS));
    s_ = strchr (s, SIGN_POSITIVE);
    i = s_ - s;
    k = strlen (s);
    for (j = k; j > i; j--)
    {
      s[j+strlen (CNV_PLUS)-1] = s[j];
      Verbose (DEBUG_STACK, "s = %s", s);
    }
    strncpy (&s[i], CNV_PLUS, strlen (CNV_PLUS));
  }

  while (strchr (s, SIGN_NEGATIVE) != NULL)
  {
    s = (char *) realloc (s, strlen (s) + strlen (CNV_MINUS));
    s_ = strchr (s, SIGN_NEGATIVE);
    i = s_ - s;
    strcpy (&s[i + strlen (CNV_MINUS)], &s[i+1]);
    strncpy (&s[i], CNV_MINUS, strlen (CNV_MINUS));
  }

//  Verbose (DEBUG_LOCAL, "name = %s  i = %d  s = %s", name, i, s);
//  Verbose (DEBUG_STACK, "<-- stg_cnv_dummy (result = %s)", s);
  return s;
}


/*===*/
char *stg_cnv_redundant (char *name)
{
//  Verbose (DEBUG_STACK, "--> stg_cnv_redundant");
  char *s = NULL;

  if (name[0] != SIGN_UNDERSCORE)
  {
    s = (char *) malloc (strlen (name) + 10);
    sprintf (s, PATTERN_REDUNDANT, name);
  }
  else
    s = name;

//  Verbose (DEBUG_STACK, "<-- stg_cnv_redundant");
  return s;
}


/*===*/
char *stg_cnv_mandatory (char *name)
{
//  Verbose (DEBUG_STACK, "--> stg_cnv_mandatory");
  char *s = NULL;

  if (name[0] == SIGN_UNDERSCORE)
  {
    s = (char *) malloc (strlen (name) - 1);
    sprintf (s, &name[1]);
  }
  else
    s = name;

//  Verbose (DEBUG_STACK, "<-- stg_cnv_mandatory");
  return s;
}


/*===*/
char *stg_cnv_var (char *name)
{
//  Verbose (DEBUG_STACK, "--> dot_cnv_name");
  int i = -1;
  char *s = NULL;
  char *s_ =NULL;

  s_ = strchr (name, '+');
  if (s_ != NULL)
  {
    s = (char *) realloc (s, strlen (name) + strlen (CNV_PLUS));
    i = s_ - name;
    strncpy (s, name, i);
    s[i] = '\0';
    strcat (s, CNV_PLUS);
    strcat (s, &name[i+1]);
  }

  s_ = strchr (name, '-');
  if (s_ != NULL)
  {
    s = (char *) realloc (s, strlen (name) + strlen (CNV_MINUS));
    i = s_ - name;
    strncpy (s, name, i);
    s[i] = '\0';
    strcat (s, CNV_MINUS);
    strcat (s, &name[i+1]);
  }

  if (s == NULL)
  {
    s = (char *) realloc (s, strlen (name) + 1);
    strcpy (s, name);
  }

  while ((s_ = strchr (s, '/')) != NULL)
  {
    s_[0] = SIGN_UNDERSCORE;
  }

//  Verbose (DEBUG_LOCAL, "name = %s  s = %s", name, s);
//  Verbose (DEBUG_STACK, "<-- dot_cnv_name");
  return s;
}


/* === */
char *stg_cnv_name (char *name, char sign, int index)
{
//  Verbose (DEBUG_STACK, "--> stg_cnv_name");
  char *s = NULL;

  s = (char *) malloc (strlen (name) + 10);
  if (sign == SIGN_POSITIVE)
    if (index > 0)
      sprintf (s, "%s%c/%.2d", name, SIGN_POSITIVE, index);
    else
      sprintf (s, "%s%c", name, SIGN_POSITIVE);
  else
  if (sign == SIGN_NEGATIVE)
    if (index > 0)
      sprintf (s, "%s%c/%.2d", name, SIGN_NEGATIVE, index);
    else
      sprintf (s, "%s%c", name, SIGN_NEGATIVE);
  else
    if (index > 0)
      sprintf (s, "%s/%.2d", name, index);
    else
      sprintf (s, "%s", name);
//  Verbose (DEBUG_STACK, "<-- stg_cnv_name");
  return s;
}


/* === */
char stg_cnv_polarity (char *name)
{
//  Verbose (DEBUG_STACK, "--> stg_cnv_polarity");
  char ch;
  if (strchr (name, SIGN_POSITIVE) != NULL)
    ch = SIGN_POSITIVE;
  else
  if (strchr (name, SIGN_NEGATIVE) != NULL)
    ch = SIGN_NEGATIVE;
  else
    ch = SIGN_NO_CARE;
//  Verbose (DEBUG_STACK, "<-- stg_cnv_polarity");
  return ch;
}


/* === */
int stg_cnv_index (char *name)
{
//  Verbose (DEBUG_STACK, "--> stg_cnv_index");

  char *s = NULL;
  int index = 0;
  s = strrchr (name, SIGN_INDEX);
  if (s != NULL)
  {
    index = atoi (&s[1]);
  }
//  Verbose (DEBUG_LOCAL, "name = %s  s = %s  index = %d", name, s, index);
//  Verbose (DEBUG_STACK, "<-- stg_cnv_index");
  return index;
}


/* === */
char *stg_cnv_invert (char *name)
{
//  Verbose (DEBUG_STACK, "--> stg_cnv_invert");
  char *s = NULL;
  int len = 0;

  if (name != NULL)
  {
    s = (char *) malloc (strlen (name) + 1);
    strcpy (s, name);
    len = strlen (s);
    switch (s[len-1])
    {
       case SIGN_LOW:
         s[len-1] = SIGN_HIGH;
         break;
       case SIGN_HIGH:
         s[len-1] = SIGN_LOW;
         break;
    }
  }
//  Verbose (DEBUG_STACK, "<-- stg_cnv_invert");
  return s;
}

/* === */
int stg_index_compress (stg_t *stg, PNode head)
{
  Verbose (DEBUG_STACK, "--> stg_index_compress");
  PNode node;
  int i, j, count, min_i;
  int min;
  int max;
  int *index = NULL;
  char *s = NULL;
  char sign = '*';

  count = NodeCount (head);
  index = (int *) malloc (count * sizeof (index));

  i = 0;
  node = head;
  while (node != NULL)
  {
    index[i++] = stg_cnv_index (node->data->name);
    node = node->next;
  }

  max = -1;
  for (j=0; j<count; j++)
  {
    min_i = -1;
    min = -1;
    for (i=0; i<count; i++)
    {
      if (index[i] > max)
        if (index[i] < min || min_i < 0)
        {
          min_i = i;
          min = index[min_i];
        }
    }
    if (min_i >= 0)
      index[min_i] = ++max;
  }

  i = 0;
  node = head;
  while (node != NULL)
  {
    sign = stg_cnv_polarity (node->data->name);
    s = node->data->name;
    node->data->name = stg_cnv_name (node->data->link->data->name, sign, index[i++]);
    free (s);
    node = node->next;
  }
  free (index);
  Verbose (DEBUG_STACK, "<-- stg_index_compress");
  return max;
}


/* === */
int stg_names_compress (stg_t *stg)
{
  Verbose (DEBUG_STACK, "--> stg_names_compress");
  PNode node_signal = NULL;
  PNode node_trans  = NULL;
  PNode node_sort_0 = NULL;
  PNode node_sort_1 = NULL;
  PNode node_place  = NULL;
  PNode node_place_ = NULL;
  PNode node_arc    = NULL;
  bool present      = false;
  PData mask        = NULL;
  char *buf_name    = NULL;
  char *buf_comment = NULL;
  int i             = 0;

  mask = DataCreate (STG_NATIVE, NULL, NULL, NULL, NULL);
  buf_name = (char *) malloc (STG_BUF_MAX);
  buf_comment = (char *) malloc (STG_BUF_MAX);

  // signals compress
  node_signal = stg->s;
  while (node_signal != NULL)
  {
    present = false;
    node_sort_0 = node_signal->next;
    node_trans = stg->t;
    while (node_trans != NULL)
    {
      if (node_trans->data->link == node_signal)
      {
        present = true;
        break;
      }
      node_trans = node_trans->next;
    }
    if (!present)
    {
      if (node_signal->data->link != NULL)
        NodeDel (stg->i, node_signal->data->link, true);
      NodeDel (stg->s, node_signal, true);
    }
    node_signal = node_sort_0;
  }

  // transition indexes compress
  node_signal = stg->s;
  while (node_signal != NULL)
  {
    DataFill (mask, STG_TRANSITION, NULL, NULL, NULL, NULL, node_signal);
    if ((node_signal->data->type & STG_SIGNAL_DUMMY) == STG_SIGNAL_DUMMY)
    {
      node_trans = stg->t;
      while ((node_trans = NodeFind (node_trans, mask)) != NULL)
      {
        NodeAdd (node_sort_0, NodeCreate (node_trans->data));
        node_trans = node_trans->next;
      }
      node_signal->data->index_0 = stg_index_compress (stg, node_sort_0);
      node_signal->data->index_1 = 0;
    }
    else
    {
      node_trans = stg->t;
      while ((node_trans = NodeFind (node_trans, mask)) != NULL)
      {
        if (stg_cnv_polarity (node_trans->data->name) == SIGN_NEGATIVE)
          NodeAdd (node_sort_0, NodeCreate (node_trans->data));
        else
        if (stg_cnv_polarity (node_trans->data->name) == SIGN_POSITIVE)
          NodeAdd (node_sort_1, NodeCreate (node_trans->data));
        node_trans = node_trans->next;
      }
      node_signal->data->index_0 = stg_index_compress (stg, node_sort_0);
      node_signal->data->index_1 = stg_index_compress (stg, node_sort_1);
    }
    NodeFree (node_sort_0);
    NodeFree (node_sort_1);
    node_signal = node_signal->next;
  }

  // place names compress
  i = 0;
  node_place = stg->p;
  while (node_place != NULL)
  {
    node_place_ = node_place->next;
    while (node_place_ != NULL)
    {
      if (strcmp (node_place->data->name, node_place_->data->name) == 0)
      {
        i++;
        sprintf (buf_name, PATTERN_PLACE, node_place->data->name, i);
        free (node_place_->data->name);
        node_place_->data->name = (char *) malloc (strlen (buf_name) + 1);
        strcpy (node_place_->data->name, buf_name);
      }
      node_place_ = node_place_->next;
    }
    if (i > 0)
    {
      i = 0;
      sprintf (buf_name, PATTERN_PLACE, node_place->data->name, i);
      free (node_place->data->name);
      node_place->data->name = (char *) malloc (strlen (buf_name) + 1);
      strcpy (node_place->data->name, buf_name);
    }
    node_place = node_place->next;
  }

  // arc names compress
  node_arc = stg->a;
  while (node_arc != NULL)
  {
    if (node_arc->data)
    if (node_arc->data->pred && node_arc->data->succ)
    if (node_arc->data->pred->data && node_arc->data->succ->data)
    if (node_arc->data->pred->data->name && node_arc->data->succ->data->name)
    {
      sprintf (buf_comment, PATTERN_ARC_COMMENT, node_arc->data->pred->data->name, node_arc->data->succ->data->name);
      free (node_arc->data->comment);
      node_arc->data->comment = (char *) malloc (strlen (buf_comment) + 1);
      strcpy (node_arc->data->comment, buf_comment);
    }
    node_arc = node_arc->next;
  }
  free (buf_name);
  free (buf_comment);
  DataFree (mask);
  Verbose (DEBUG_STACK, "<-- stg_names_compress");
  return 1;
}

/* ===*/
int stg_place_delete(stg_t *stg, PNode node_place_redundant, PNode node_place)
{
  Verbose (DEBUG_STACK, "--> stg_place_delete");
  PNode node_arc       = NULL;
  PNode node_arc_next  = NULL;
  PData mask            = NULL;

  mask = DataCreate (STG_NATIVE, NULL, NULL, NULL, NULL);
  // delete every redundant place pred consuming arc
  node_arc = stg->a;
  while ((node_arc = NodeFind (node_arc, DataFill (mask, STG_ARC_CONSUMING, NULL, NULL, NULL, node_place_redundant, NULL))) !=NULL)
  {
    node_arc_next = node_arc->next;
    Verbose (DEBUG_PROC, "  REDUNDANT PLACE PRED CONSUMING ARC DELETED (type =%#.8X  name = %s  comment = %s)", node_arc->data->type, node_arc->data->name, node_arc->data->comment);
    NodeDel (stg->a, node_arc, true);
    node_arc = node_arc_next;
  }
  // delete every redundant place succ consuming arc
  node_arc = stg->a;
  while ((node_arc = NodeFind (node_arc, DataFill (mask, STG_ARC_CONSUMING, NULL, NULL, node_place_redundant, NULL, NULL))) !=NULL)
  {
    node_arc_next = node_arc->next;
    Verbose (DEBUG_PROC, "  REDUNDANT PLACE SUCC CONSUMING ARC DELETED (type =%#.8X  name = %s  comment = %s)", node_arc->data->type, node_arc->data->name, node_arc->data->comment);
    NodeDel (stg->a, node_arc, true);
    node_arc = node_arc_next;
  }
  // move every redundant place reading arc
  node_arc = stg->a;
  while ((node_arc = NodeFind (node_arc, DataFill (mask, STG_ARC_READING, NULL, NULL, node_place_redundant, NULL, NULL))) !=NULL)
  {
    node_arc->data->pred = node_place;
    Verbose (DEBUG_PROC, "  REDUNDANT PLACE READING ARC MOVED (type =%#.8X  name = %s  comment = %s)", node_arc->data->type, node_arc->data->name, node_arc->data->comment);
    node_arc = node_arc->next;
  }
  Verbose (DEBUG_PROC, "  REDUNDANT PLACE DELETED (type =%#.8X  name = %s  comment = %s)", node_place_redundant->data->type, node_place_redundant->data->name, node_place_redundant->data->comment);
  NodeDel (stg->p, node_place_redundant, true);
  DataFree (mask);
  Verbose (DEBUG_STACK, "<-- stg_place_delete");
  return 1;
}

/* ===*/
int stg_compress_places (stg_t *stg)
{
  Verbose (DEBUG_STACK, "--> stg_compress_places");
  PNode node_place      = NULL;
  PNode node_place_     = NULL;
  PNode node_place__    = NULL;
  PNode node_place_next = NULL;
  PNode node_arc        = NULL;
  PNode node_arc_next   = NULL;
  int type_mask         = STG_PLACE | STG_MANDATORY | STG_REDUNDANT;
  bool compressed       = true;
  bool redundant        = true;

  compressed = true;
  node_place = stg->p;
  while (node_place)
  {
    Verbose (DEBUG_PROC, "CHECKING PLACE (type =%#.8X  name = %s  comment = %s)", node_place->data->type, node_place->data->name, node_place->data->comment);
    node_place_ = node_place->next;
    while (node_place_)
    {
      node_place_next = node_place_->next;
      redundant = true;

      if (redundant)
        if ((node_place->data->type & type_mask) != (node_place_->data->type & type_mask))
          redundant = false;

      if (redundant)
        if ((NodeCount(node_place->data->pred) != NodeCount(node_place_->data->pred)) ||
            (NodeCount(node_place->data->succ) != NodeCount(node_place_->data->succ)))
          redundant = false;

      if (redundant)
        redundant = NodeCompare(node_place->data->pred, node_place_->data->pred);

      if (redundant)
        redundant = NodeCompare(node_place->data->succ, node_place_->data->succ);

      if (redundant)
      {
        stg_place_delete(stg, node_place_, node_place);
        compressed = false;
      }
      node_place_ = node_place_next;
    }
    if (!compressed) stg_fill (stg);
    node_place = node_place->next;
  }
  if (!compressed) stg_names_compress (stg);
  Verbose (DEBUG_STACK, "<-- stg_compress_places");
  return compressed;
}

/* ===*/
PNode stg_place_transitive_ex_find(stg_t *stg, PNode node_place, PNode node_trans)
{
  PNode node_trans_           = NULL;
  PNode node_place_           = NULL;
  Verbose (DEBUG_STACK, "--> stg_place_transitive_ex_find");
  node_place_ = node_trans->data->succ;
  while ((node_place_ != NULL))
  {
    if (node_place_->data != node_place->data)
    if ((NodeCount(node_place_->data->pred) == 1) &&
        (NodeCount(node_place_->data->succ) == 1))
    {
      node_trans_ = node_place_->data->succ;
      if ((node_trans_->data->type & STG_PROCESSED) != STG_PROCESSED)
      {
        node_trans_->data->type |= STG_PROCESSED;
        if (node_trans_->data == node_place->data->succ->data)
          return node_place_;
        else
          return stg_place_transitive_ex_find(stg, node_place, node_trans_);
      }
    }
    node_place_ = node_place_->next;
  }
  Verbose (DEBUG_STACK, "<-- stg_place_transitive_ex_find");
  return NULL;
}

/* ===*/
PNode stg_place_transitive_ex(stg_t *stg, PNode node_place)
{
  PNode node_trans            = NULL;
  PNode node_place_transitive = NULL;
  Verbose (DEBUG_STACK, "--> stg_place_transitive_ex");
  if ((NodeCount(node_place->data->pred) == 1) &&
      (NodeCount(node_place->data->succ) == 1))
  if ((NodeCount(node_place->data->pred->data->succ) > 1) &&
      (NodeCount(node_place->data->succ->data->pred) > 1))
  {
    Verbose (DEBUG_PROC, "CHECKING PLACE (type =%#.8X  name = %s  comment = %s)", node_place->data->type, node_place->data->name, node_place->data->comment);
    stg_type_and (stg->p, ~STG_PROCESSED);
    stg_type_and (stg->t, ~STG_PROCESSED);
    node_trans = node_place->data->pred;
    node_trans->data->type |= STG_PROCESSED;
    node_place_transitive = stg_place_transitive_ex_find(stg, node_place, node_trans);
  }
  Verbose (DEBUG_STACK, "<-- stg_place_transitive_ex");
  return node_place_transitive;
}

/* ===*/
int stg_compress_places_ex (stg_t *stg, bool move_read_arcs)
{
  Verbose (DEBUG_STACK, "--> stg_compress_places_ex");
  PNode node_place      = NULL;
  PNode node_place_next = NULL;
  PNode node_place_     = NULL;
  bool compressed       = true;
  PData mask            = NULL;

  mask = DataCreate (STG_NATIVE, NULL, NULL, NULL, NULL);

  node_place = stg->p;
  while (node_place != NULL)
  {
    node_place_next = node_place->next;
    if (move_read_arcs || (NodeFind (stg->a, DataFill (mask, STG_ARC_READING, NULL, NULL, node_place, NULL, NULL)) ==NULL))
    {
      node_place_ = stg_place_transitive_ex(stg, node_place);
      if (node_place_)
      {
        compressed = false;
        stg_place_delete(stg, node_place, node_place_);
      }
      if (!compressed) stg_fill (stg);
    }
    node_place = node_place_next;
  }
  DataFree (mask);
  if (!compressed) stg_names_compress (stg);
  Verbose (DEBUG_STACK, "<-- stg_compress_places_ex");
  return 1;
}

/* ===*/
int stg_compress_transitions (stg_t *stg)
{
  Verbose (DEBUG_STACK, "--> stg_compress_transitions");
  PNode node_trans      = NULL;
  PNode node_trans_     = NULL;
  PNode node_trans__    = NULL;
  PNode node_trans_next = NULL;
  PNode node_arc        = NULL;
  PNode node_arc_next   = NULL;
  int type_mask         = STG_TRANSITION | STG_DUMMY;
  bool compressed       = true;
  bool redundant        = true;
  PData mask            = NULL;

  mask = DataCreate (STG_NATIVE, NULL, NULL, NULL, NULL);

  compressed = true;
  node_trans = stg->t;
  while (node_trans != NULL)
  {
    node_trans_ = node_trans->next;
    while (node_trans_ != NULL)
    {
      node_trans_next = node_trans_->next;
      redundant = true;

      if (redundant)
        if ((node_trans->data->type & type_mask) != (node_trans_->data->type & type_mask))
          redundant = false;

      if (redundant)
        if ((NodeCount(node_trans->data->pred) != NodeCount(node_trans_->data->pred)) ||
            (NodeCount(node_trans->data->succ) != NodeCount(node_trans_->data->succ)))
          redundant = false;

      if (redundant)
        redundant = NodeCompare(node_trans->data->pred, node_trans_->data->pred);

      if (redundant)
        redundant = NodeCompare(node_trans->data->succ, node_trans_->data->succ);

      node_arc = stg->a;
      while (redundant && ((node_arc = NodeFind (node_arc, DataFill (mask, STG_ARC_READING, NULL, NULL, NULL, node_trans_, NULL))) !=NULL))
      {
        if (NodeFind (stg->a, DataFill (mask, STG_ARC_READING, NULL, NULL, node_arc->data->pred, node_trans, NULL)) == NULL)
          redundant = false;
        node_arc = node_arc->next;
      }

      node_arc = stg->a;
      while (redundant && ((node_arc = NodeFind (node_arc, DataFill (mask, STG_ARC_READING, NULL, NULL, NULL, node_trans, NULL))) !=NULL))
      {
        if (NodeFind (stg->a, DataFill (mask, STG_ARC_READING, NULL, NULL, node_arc->data->pred, node_trans_, NULL)) == NULL)
          redundant = false;
        node_arc = node_arc->next;
      }

      if (redundant)
      {
          // delete every redundant transition pred consuming arc
          node_arc = stg->a;
          while ((node_arc = NodeFind (node_arc, DataFill (mask, STG_ARC_CONSUMING, NULL, NULL, NULL, node_trans_, NULL))) !=NULL)
          {
            node_arc_next = node_arc->next;
            Verbose (DEBUG_PROC, "  REDUNDANT TRANSITION PRED CONSUMING ARC DELETED (type =%#.8X  name = %s  comment = %s)", node_arc->data->type, node_arc->data->name, node_arc->data->comment);
            NodeDel (stg->a, node_arc, true);
            node_arc = node_arc_next;
          }
          // delete every redundant transition succ consuming arc
          node_arc = stg->a;
          while ((node_arc = NodeFind (node_arc, DataFill (mask, STG_ARC_CONSUMING, NULL, NULL, node_trans_, NULL, NULL))) !=NULL)
          {
            node_arc_next = node_arc->next;
            Verbose (DEBUG_PROC, "  REDUNDANT TRANSITION SUCC CONSUMING ARC DELETED (type =%#.8X  name = %s  comment = %s)", node_arc->data->type, node_arc->data->name, node_arc->data->comment);
            NodeDel (stg->a, node_arc, true);
            node_arc = node_arc_next;
          }
          // move every redundant transition reading arc
          node_arc = stg->a;
          while ((node_arc = NodeFind (node_arc, DataFill (mask, STG_ARC_READING, NULL, NULL, NULL, node_trans_, NULL))) !=NULL)
          {
            node_arc->data->succ = node_trans;
            Verbose (DEBUG_PROC, "  REDUNDANT TRANSITION READING ARC MOVED (type =%#.8X  name = %s  comment = %s)", node_arc->data->type, node_arc->data->name, node_arc->data->comment);
            node_arc = node_arc->next;
          }
          Verbose (DEBUG_PROC, "  REDUNDANT TRANSITION DELETED (type =%#.8X  name = %s  comment = %s)", node_trans_->data->type, node_trans_->data->name, node_trans_->data->comment);
          NodeDel (stg->t, node_trans_, true);
          compressed = false;
      }
      node_trans_ = node_trans_next;
    }
    if (!compressed) stg_fill (stg);
    node_trans = node_trans->next;
  }

  DataFree (mask);
  if (!compressed) stg_names_compress (stg);
  Verbose (DEBUG_STACK, "<-- stg_compress_transitions");
  return compressed;
}


/* ===*/
int stg_compress_transitions_ex (stg_t *stg)
{
  Verbose (DEBUG_STACK, "--> stg_compress_transitions_ex");
  PNode node_trans      = NULL;
  PNode node_trans_next = NULL;
  PNode node_arc        = NULL;
  PNode node_arc_next   = NULL;
  bool compressed       = true;
  bool redundant        = true;
  PData mask            = NULL;

  mask = DataCreate (STG_NATIVE, NULL, NULL, NULL, NULL);

  compressed = true;
  node_trans = stg->t;
  while (node_trans != NULL)
  {
    node_trans_next = node_trans->next;
    bool redundant        = true;

    if (redundant)
      if (NodeFind (stg->a, DataFill (mask, STG_ARC_READING, NULL, NULL, NULL, node_trans, NULL)) != NULL)
        redundant = false;

    if (redundant)
      if (NodeFind (stg->a, DataFill (mask, STG_ARC_CONSUMING, NULL, NULL, node_trans, NULL, NULL)) != NULL)
        redundant = false;

    if (redundant)
    {
      // delete every redundant-transition consuming-arc
      node_arc = stg->a;
      while ((node_arc = NodeFind (node_arc, DataFill (mask, STG_ARC_CONSUMING, NULL, NULL, NULL, node_trans, NULL))) !=NULL)
      {
        node_arc_next = node_arc->next;
        Verbose (DEBUG_PROC, "  REDUNDANT TRANSITION CONSUMING ARC DELETED (type =%#.8X  name = %s  comment = %s)", node_arc->data->type, node_arc->data->name, node_arc->data->comment);
        NodeDel (stg->a, node_arc, true);
        node_arc = node_arc_next;
      }
      // delete every redundant-transition producing-arc
      node_arc = stg->a;
      while ((node_arc = NodeFind (node_arc, DataFill (mask, STG_ARC_CONSUMING, NULL, NULL, node_trans, NULL, NULL))) !=NULL)
      {
        node_arc_next = node_arc->next;
        Verbose (DEBUG_PROC, "  REDUNDANT TRANSITION PRODCING ARC DELETED (type =%#.8X  name = %s  comment = %s)", node_arc->data->type, node_arc->data->name, node_arc->data->comment);
        NodeDel (stg->a, node_arc, true);
        node_arc = node_arc_next;
      }
      // delete every redundant-transition read-arc
      node_arc = stg->a;
      while ((node_arc = NodeFind (node_arc, DataFill (mask, STG_ARC_READING, NULL, NULL, NULL, node_trans, NULL))) !=NULL)
      {
        node_arc_next = node_arc->next;
        Verbose (DEBUG_PROC, "  REDUNDANT TRANSITION READ ARC DELETED (type =%#.8X  name = %s  comment = %s)", node_arc->data->type, node_arc->data->name, node_arc->data->comment);
        NodeDel (stg->a, node_arc, true);
        node_arc = node_arc_next;
      }

      Verbose (DEBUG_PROC, "  REDUNDANT TRANSITION DELETED (type =%#.8X  name = %s  comment = %s)", node_trans->data->type, node_trans->data->name, node_trans->data->comment);
      NodeDel (stg->t, node_trans, true);
      compressed = false;

      node_trans = node_trans_next;
    }
    if (!compressed) stg_fill (stg);
    node_trans = node_trans->next;
  }

  DataFree (mask);
  if (!compressed) stg_names_compress (stg);
  Verbose (DEBUG_STACK, "<-- stg_compress_transitions_ex");
  return compressed;
}

/* ===*/
int stg_compress_arcs (stg_t *stg)
{
  Verbose (DEBUG_STACK, "--> stg_compress_arcs");
  PNode node_arc        = NULL;
  PNode node_arc_       = NULL;
  PNode node_arc_next   = NULL;
  int type_mask         = STG_ARC_CONSUMING | STG_ARC_READING;
  bool compressed       = true;

  compressed = true;
  node_arc = stg->a;
  while (node_arc != NULL)
  {
    node_arc_ = node_arc->next;
    while (node_arc_ != NULL)
    {
      node_arc_next = node_arc_->next;
      if ((node_arc->data->type & type_mask) == (node_arc_->data->type & type_mask))
      {
        if (node_arc->data->pred->data == node_arc_->data->pred->data && node_arc->data->succ->data == node_arc_->data->succ->data)
        {
          Verbose (DEBUG_PROC, "  REDUNDANT ARC DELETED (type =%#.8X  name = %s  comment = %s)", node_arc_->data->type, node_arc_->data->name, node_arc_->data->comment);
          NodeDel (stg->a, node_arc_, true);
          compressed = false;
        }
      }
      node_arc_ = node_arc_next;
    }
    node_arc = node_arc->next;
  }

  Verbose (DEBUG_STACK, "<-- stg_compress_arcs");
  return compressed;
}

/* ===*/
int stg_compress (stg_t *stg)
{
  bool compressed = true;

  Verbose (DEBUG_STACK, "--> stg_compress");
  do
  {
    compressed = true;
    if (!stg_compress_places (stg)) compressed = false;
    if (!stg_compress_places_ex (stg, false)) compressed = false;
    if (!stg_compress_transitions (stg)) compressed = false;
    if (!stg_compress_transitions_ex (stg)) compressed = false;
    if (!stg_compress_arcs (stg)) compressed = false;
  }
  while (!compressed);

  Verbose (DEBUG_STACK, "<-- stg_compress");
  return compressed;
}

