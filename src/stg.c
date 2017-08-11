#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "debug.h"
#include "stg.h"
#include "stg_procs.h"
#include "list.h"

/* === */
stg_t *stg_create ()
{
  Verbose (DEBUG_STACK, "--> stg_create");

  stg_t *stg;

  stg = (stg_t *) malloc (sizeof (stg_t));
  stg->s = NULL;
  stg->t = NULL;
  stg->p = NULL;
  stg->a = NULL;
  stg->m = NULL;
  stg->i = NULL;
  stg->model_name = NULL;
  stg->p_num = 0;
  stg->t_num = 0;
  stg->a_num = 0;
  stg->m_num = 0;
  stg->i_num = 0;

  Verbose (DEBUG_STACK, "<-- stg_create");
  return stg;
}

/* === */
int stg_destroy (stg_t *stg)
{
  Verbose (DEBUG_STACK, "--> stg_destroy");
  NodeClear (stg->i);  NodeFree (stg->i);
  NodeClear (stg->m);  NodeFree (stg->m);
  NodeClear (stg->a);  NodeFree (stg->a);
  NodeClear (stg->p);  NodeFree (stg->p);
  NodeClear (stg->t);  NodeFree (stg->t);
  NodeClear (stg->s);  NodeFree (stg->s);
  free (stg->model_name);
  free (stg);
  Verbose (DEBUG_STACK, "<-- stg_destroy");
  return 1;
}




/* === */
int stg_revert (stg_t *stg)
{
  Verbose (DEBUG_STACK, "--> stg_revert");
  NodeRevert (stg->s);
  NodeRevert (stg->t);
  NodeRevert (stg->p);
  NodeRevert (stg->a);
  NodeRevert (stg->m);
  NodeRevert (stg->i);
  Verbose (DEBUG_STACK, "<-- stg_revert");
  return 1;
}


/* === */
int stg_sort (stg_t *stg)
{
  Verbose (DEBUG_STACK, "--> stg_sort");
  NodeSort (stg->s);
  NodeSort (stg->t);
  NodeSort (stg->p);
  NodeSort (stg->a);
  NodeSort (stg->m);
  NodeSort (stg->i);
  Verbose (DEBUG_STACK, "<-- stg_sort");
  return 1;
}


/* === */
// Priorety:
// 1 - number of reading arcs
// 2 - number of consuming arcs
int stg_complexity_sort (stg_t *stg, PNode &head, int direction)
{
  Verbose (DEBUG_STACK, "--> stg_complexity_sort");
  PNode node_trans     = NULL;
  PNode node_trans_dum = NULL;
  int num_arc_c_dum    = 0;
  int num_arc_r_dum    = 0;
  PNode node_trans_min = NULL;
  int num_arc_c_min    = 0;
  int num_arc_r_min    = 0;
  PNode node_arc       = NULL;
  PData mask           = NULL;

  mask = DataCreate (STG_NATIVE, NULL, NULL, NULL, NULL);
  
  node_trans = head;
  while (node_trans != NULL)
  {
    node_trans_min = NULL;

    node_trans_dum = node_trans;
    while (node_trans_dum != NULL)
    {
      num_arc_r_dum = 0;
      DataFill (mask, STG_ARC_READING, NULL, NULL, NULL, node_trans_dum, NULL);
      node_arc = stg->a;
      while ((node_arc = NodeFind (node_arc, mask)) != NULL)
      {
        num_arc_r_dum++;
        node_arc = node_arc->next;
      }

      if (direction == -1)
        num_arc_c_dum = NodeCount (node_trans_dum->data->pred);
      else
        num_arc_c_dum = NodeCount (node_trans_dum->data->succ);

      if ((node_trans_min == NULL) ||
         (num_arc_r_dum < num_arc_r_min) ||
         ((num_arc_r_dum == num_arc_r_min) && (num_arc_c_dum < num_arc_c_min)))
      {
        node_trans_min = node_trans_dum;
        num_arc_r_min = num_arc_r_dum;
        num_arc_c_min = num_arc_c_dum;
        Verbose (DEBUG_PROC, "  MIN FOUND: node->name = %s  r_num = %d  c_num = %d", node_trans_min->data->name, num_arc_r_min, num_arc_c_min);
      }
      node_trans_dum = node_trans_dum->next;
    }
    NodeSwap (head, node_trans, node_trans_min);
    node_trans = node_trans->next;
  }
  
  DataFree (mask);
  Verbose (DEBUG_STACK, "<-- stg_complexity_sort");
  return 1;
}


/* === */
int stg_pred_succ_sort (stg_t *stg)
{
  Verbose (DEBUG_STACK, "--> stg_pred_succ_sort");
  PNode node_place  = NULL;
  PNode node        = NULL;

  // sort pred
  node_place = stg->p;
  while (node_place != NULL)
  {
    stg_complexity_sort (stg, node_place->data->pred, -1);
    node_place = node_place->next;
  }
  
  // sort succ
  node_place = stg->p;
  while (node_place != NULL)
  {
    stg_complexity_sort (stg, node_place->data->succ, +1);
    node_place = node_place->next;
  }
  
  Verbose (DEBUG_STACK, "<-- stg_pred_succ_sort");
  return 1;
}
/* CONVERT END */


/* INPUT-OUTPUT BEGIN */
/* === */
int stg_read (FILE *infile, stg_t *stg)
{
  Verbose (DEBUG_STACK, "--> stg_read");

  char ch;
  int s_len         = 0;
  int s_max         = STG_BUF_MAX;
  char *s           = NULL;
  char *s_          = NULL;
  int pos           = _UNDEFINED;
  int line          = 1;
  PNode node        = NULL;
  PNode node_from   = NULL;
  PNode node_mid    = NULL;
  PNode node_to     = NULL;
  PData mask        = NULL;
  char *buf_name    = NULL;
  char *buf_comment = NULL;

  NodeClear (stg->i);  NodeFree (stg->i);
  NodeClear (stg->m);  NodeFree (stg->m);
  NodeClear (stg->a);  NodeFree (stg->a);
  NodeClear (stg->p);  NodeFree (stg->p);
  NodeClear (stg->t);  NodeFree (stg->t);
  NodeClear (stg->s);  NodeFree (stg->s);
  free (stg->model_name);
  stg->t_num = 0;
  stg->p_num = 0;
  stg->a_num = 0;
  stg->m_num = 0;
  stg->i_num = 0;

  mask = DataCreate (STG_NATIVE, NULL, NULL, NULL, NULL);
  buf_name = (char *) malloc (STG_BUF_MAX);
  buf_comment = (char *) malloc (STG_BUF_MAX);
  s = (char *) malloc (s_max);
  s[0] = '\0';
  s_len = 0;

  while ( (ch = getc (infile)) != EOF)
  {
    switch (pos)
    {
      case _ERROR:
        Fatal("reading input can't recognize word %s in line %d", s, line);
        break;

      /* ' ', '#', '.', 'a'-'z' */
      case _UNDEFINED:
        switch (ch)
        {
          case ' ':
          case '\t':
          case '\n':
            break;
          case '#':
            Verbose (DEBUG_PROC, "BEGIN COMMENT");
            pos = _COMMENT;
            break;
          case '.':
            Verbose (DEBUG_PROC, "BEGIN KEY");
            pos = _KEY;
            break;
          default:
            Verbose (DEBUG_PROC, "BEGIN ARC(S)");
            pos = _ARCS;
            node_from = NULL;
            node_to = NULL;
        }
        break;

      /* '#' */
      case _COMMENT:
        switch (ch)
        {
          case '\n':
            Verbose (DEBUG_PROC, "END COMMENT (%s)", s);
            pos = _UNDEFINED;
            s[0] = '\0';
            s_len = 0;
            break;
        }
        break;

      /* '.' */
      case _KEY:
        switch (ch)
        {
          case ' ':
          case '\t':
          case '\n':
            Verbose (DEBUG_PROC, "END KEY (%s)", s);
            if (ch == '\n')
            {
              pos = _UNDEFINED;
            }
            else
            if (strcmp (s, WORD_MODEL) == 0)
            {
              Verbose (DEBUG_PROC, "BEGIN MODEL");
              pos = KEY_MODEL;
            }
            else
            if (strcmp (s, WORD_INPUTS) == 0)
            {
              Verbose (DEBUG_PROC, "BEGIN INPUT SIGNAL(S)");
              pos = KEY_INPUTS;
            }
            else
            if (strcmp (s, WORD_OUTPUTS) == 0)
            {
              Verbose (DEBUG_PROC, "BEGIN OUTPUT SIGNAL(S)");
              pos = KEY_OUTPUTS;
            }
            else
            if (strcmp (s, WORD_INTERNAL) == 0)
            {
              Verbose (DEBUG_PROC, "BEGIN INTERNAL SIGNAL(S)");
              pos = KEY_INTERNAL;
            }
            else
            if (strcmp (s, WORD_DUMMY) == 0)
            {
              Verbose (DEBUG_PROC, "BEGIN DUMMY SIGNAL(S)");
              pos = KEY_DUMMY;
            }
            else
            if (strcmp (s, WORD_INITIAL_) == 0)
            {
              Verbose (DEBUG_PROC, "BEGIN INITIAL...");
              pos = KEY_INITIAL_;
            }
            else
            if (strcmp (s, WORD_GRAPH) == 0)
            {
              Verbose (DEBUG_PROC, "BEGIN GRAPH");
              pos = KEY_GRAPH;
            }
            else
            if (strcmp (s, WORD_MARKING) == 0)
            {
              Verbose (DEBUG_PROC, "BEGIN MARKING");
              pos = KEY_MARKING;
            }
            else
            if (strcmp (s, WORD_END) == 0)
            {
              Verbose (DEBUG_PROC, "BEGIN END");
              pos = KEY_END;
            }
            else
            {
              Verbose (DEBUG_PROC, "ERROR (line %d)", line);
              pos = _ERROR;
            }
            s[0] = '\0';
            s_len = 0;
            break;
        }
        break;


      /* .model */
      case KEY_MODEL:
        switch (ch)
        {
          case '#':
          case '\n':
            Verbose (DEBUG_PROC, "END MODEL (%s)", s);
            stg->model_name = (char *) realloc (stg->model_name, strlen (s)+1);
            strcpy (stg->model_name, s);
            pos = _UNDEFINED;
            if (ch == '#')
            {
              Verbose (DEBUG_PROC, "BEGIN COMMENT");
              pos = _COMMENT;
            }
            s[0] = '\0';
            s_len = 0;
            break;
        }
        break;

      /* .inputs | .outputs | .internal | .dummy */
      case KEY_INPUTS:
      case KEY_OUTPUTS:
      case KEY_INTERNAL:
      case KEY_DUMMY:
        switch (ch)
        {
          case '\n':
          case '#':
          case ' ':
          case '\t':
            if (strlen (s) > 0)
            {
              node = NodeFind (stg->t, DataFill (mask, STG_SIGNAL, s, NULL));
              switch (pos)
              {
                case KEY_INPUTS:
                  if (node != NULL)
                  {
                    if (node->data->type != STG_SIGNAL_INPUT)
                      Fatal("reading input line %d signal %s repeats in different sections", line, s);
                  }
                  else
                  {
                    Verbose (DEBUG_PROC, "INPUT SIGNAL (name = %s  comment = %s)", s, NULL);
                    node = NodeCreate (DataCreate (STG_SIGNAL_INPUT | STG_NATIVE, s, NULL));
                  }
                  break;

                case KEY_OUTPUTS:
                  if (node != NULL)
                  {
                    if (node->data->type != STG_SIGNAL_OUTPUT)
                      Fatal("reading input line %d signal %s repeats in different sections", line, s);
                  }
                  else
                  {
                    Verbose (DEBUG_PROC, "OUTPUT SIGNAL (name = %s  comment = %s)", s, NULL);
                    node = NodeCreate (DataCreate (STG_SIGNAL_OUTPUT | STG_NATIVE, s, NULL));
                  }
                  break;

                case KEY_INTERNAL:
                  if (node != NULL)
                  {
                    if (node->data->type != STG_SIGNAL_INTERNAL)
                      Fatal("reading input line %d signal %s repeats in different sections", line, s);
                  }
                  else
                  {
                    Verbose (DEBUG_PROC, "INTERNAL SIGNAL (name = %s  comment = %s)", s, NULL);
                    node = NodeCreate (DataCreate (STG_SIGNAL_INTERNAL | STG_NATIVE, s, NULL));
                  }
                  break;

                case KEY_DUMMY:
                  if (node != NULL)
                  {
                    if (node->data->type != STG_SIGNAL_DUMMY)
                      Fatal("reading input line %d signal %s repeats in different sections", line, s);
                  }
                  else
                  {
                    Verbose (DEBUG_PROC, "DUMMY SIGNAL (name = %s  comment = %s)", s, NULL);
                    node = NodeCreate (DataCreate (STG_SIGNAL_DUMMY | STG_NATIVE, s, NULL));
                  }
                  break;
              }
              NodeAdd (stg->s, node);
            }

            if (ch == '#' || ch == '\n')
            {
              switch (pos)
              {
                case KEY_INPUTS:
                  Verbose (DEBUG_PROC, "END INTPUT SIGNAL(S)");
                  break;
                case KEY_OUTPUTS:
                  Verbose (DEBUG_PROC, "END OUTPUT SIGNAL(S)");
                  break;
                case KEY_INTERNAL:
                  Verbose (DEBUG_PROC, "END INTERNAL SIGNAL(S)");
                  break;
                case KEY_DUMMY:
                  Verbose (DEBUG_PROC, "END DUMMY SIGNAL(S)");
                  break;
              }
              switch (ch)
              {
                case '#':
                  Verbose (DEBUG_PROC, "BEGIN COMMENT");
                  pos = _COMMENT;
                  break;
                case '\n':
                  pos = _UNDEFINED;
                  break;
              }
            }
            s[0] = '\0';
            s_len = 0;
            break;
        }
        break;

      /* .initial */
      case KEY_INITIAL_:
        switch (ch)
        {
          case '\n':
          case '#':
          case ' ':
          case '\t':
            if (strlen (s) > 0)
            {
              if (strcmp (s, WORD_STATE) == 0)
              {
                Verbose (DEBUG_PROC, "BEGIN INITIAL STATE");
                pos = KEY_INITIAL_STATE;
              }
              else
              {
                Verbose (DEBUG_PROC, "ERROR (line %d)", line);
                pos = _ERROR;
              }  
            }
            s[0] = '\0';
            s_len = 0;
            break;
        }
        break;
        
      /* .initial stat */
      case KEY_INITIAL_STATE:
        switch (ch)
        {
          case '\n':
          case '#':
          case ' ':
          case '\t':
            if (strlen (s) > 0)
            {
              s_ = (char *) realloc (s_, strlen (s)+1);
              s_ = stg_cnv_signal (s);
              node_mid = NodeFind (stg->s, DataFill (mask, STG_SIGNAL, s_, NULL, NULL, NULL, NULL));
              if (node_mid != NULL && node_mid->data->link == NULL)
              {
                if (s[0] == '!')
                {
                  sprintf (buf_name, PATTERN_INITIAL_NAME_LOW, node_mid->data->name);
                  sprintf (buf_comment, PATTERN_INITIAL_COMMENT_LOW, node_mid->data->name);
                  node = NodeCreate (DataCreate (STG_INITIAL_LOW, buf_name, buf_comment, NULL, NULL, node_mid));
                }
                else
                {
                  sprintf (buf_name, PATTERN_INITIAL_NAME_HIGH, node_mid->data->name);
                  sprintf (buf_comment, PATTERN_INITIAL_COMMENT_HIGH, node_mid->data->name);
                  node = NodeCreate (DataCreate (STG_INITIAL_HIGH, buf_name, buf_comment, NULL, NULL, node_mid));
                }
                NodeAdd (stg->i, node);
                node_mid->data->link = node;
              }
              else
              {
                Fatal("reading input line %d signal %s does not exist or initial state conflict", line, s_);
              }
            }
            switch (ch)
            {
              case '#':
                Verbose (DEBUG_PROC, "END INITIAL STATE");
                Verbose (DEBUG_PROC, "BEGIN COMMENT");
                pos = _COMMENT;
                break;
              case '\n':
                Verbose (DEBUG_PROC, "END INITIAL STATE");
                pos = _UNDEFINED;
                break;
            }
            s[0] = '\0';
            s_len = 0;
            break;
        }
        break;

      /* 'a'-'z', '_' */
      case _ARCS:
        switch (ch)
        {
          case '#':
          case ' ':
          case '\t':
          case '\n':
            if (strlen (s) > 0)
            {
              node_to = NULL;
              if (node_to == NULL)
                node_to = NodeFind (stg->t, DataFill (mask, STG_TRANSITION, s, NULL));
              if (node_to == NULL)
                node_to = NodeFind (stg->p, DataFill (mask, STG_PLACE, s, NULL));
              if (node_to == NULL)
              {
                node = NodeFind (stg->s, DataFill (mask, STG_SIGNAL, stg_cnv_signal (s)));
                if (node == NULL)
                {
                  // force state separation place
                  if (s[0] == SIGN_UNDERSCORE)
                  {
                    node_to = NodeCreate (DataCreate (STG_PLACE_REDUNDANT | STG_NATIVE, s, s));
                    NodeAdd (stg->p, node_to);
                    Verbose (DEBUG_PROC, "PLACE NATIVE REDUNDANT (type =%#.8X  name = %s  comment = %s)", node_to->data->type, node_to->data->name, node_to->data->comment);
                  }
                  else
                  // usual place
                  {
                    node_to = NodeCreate (DataCreate (STG_PLACE | STG_NATIVE, s, s));
                    NodeAdd (stg->p, node_to);
                    Verbose (DEBUG_PROC, "PLACE NATIVE (type =%#.8X  name = %s  comment = %s)", node_to->data->type, node_to->data->name, node_to->data->comment);
                  }
                }
                else
                {
                  node_to = NodeCreate (DataCreate (STG_TRANSITION | STG_NATIVE | (node->data->type ^ STG_SIGNAL), s, NULL));
                  NodeAdd (stg->t, node_to);
                  Verbose (DEBUG_PROC, "TRANSITION NATIVE (type =%#.8X  name = %s  comment = %s)", node_to->data->type, node_to->data->name, node_to->data->comment);
                }
              }

              if (node_from == NULL)
              {
                node_from = node_to;
              }
              else
              {
                if (node_from->data->type & node_to->data->type & (STG_PLACE | STG_TRANSITION))
                {
                  if ((node_from->data->type & STG_TRANSITION) == STG_TRANSITION)
                  {
                    sprintf (buf_name, PATTERN_PLACE_NAME, stg->p_num++);
//                    sprintf (buf_name, PATTERN_PLACE_NAME_, stg_cnv_var (node_from->data->name), stg_cnv_var (node_to->data->name));
                    sprintf (buf_comment, PATTERN_PLACE_COMMENT, node_from->data->name, node_to->data->name);
                    node_mid = NodeCreate (DataCreate (STG_PLACE | STG_GENERATED, buf_name, buf_comment));
                    NodeAdd (stg->p, node_mid);
                    Verbose (DEBUG_PROC, "PLACE GENERATED (type =%#.8X  name = %s  comment = %s)", node_mid->data->type, node_mid->data->name, node_mid->data->comment);
                  }
                  else
                  {
                    sprintf (buf_name, PATTERN_TRANSITION_NAME, stg->t_num++);
//                    sprintf (buf_name, PATTERN_TRANSITION_NAME_, stg_cnv_var (node_from->data->name), stg_cnv_var (node_to->data->name));
                    node_mid = NodeCreate (DataCreate (STG_SIGNAL_DUMMY | STG_GENERATED , buf_name, NULL));
                    NodeAdd (stg->s, node_mid);
                    sprintf (buf_comment, PATTERN_TRANSITION_COMMENT, node_from->data->name, node_to->data->name);
                    Verbose (DEBUG_PROC, "DUMMY GENERATED (type =%#.8X  name = %s  comment = %s)", node_mid->data->type, node_mid->data->name, node_mid->data->comment);
                    node_mid = NodeCreate (DataCreate (STG_TRANSITION_DUMMY | STG_GENERATED , buf_name, buf_comment));
                    NodeAdd (stg->t, node_mid);
                    Verbose (DEBUG_PROC, "TRANSITION GENERATED (type =%#.8X  name = %s  comment = %s)", node_mid->data->type, node_mid->data->name, node_mid->data->comment);
                  }

                  sprintf (buf_name, PATTERN_ARC_NAME, stg->a_num++);
                  sprintf (buf_comment, PATTERN_ARC_COMMENT, node_from->data->name, node_mid->data->name);
                  node = NodeCreate (DataCreate (STG_ARC_CONSUMING | STG_GENERATED, buf_name, buf_comment, node_from, node_mid));
                  NodeAdd (stg->a, node);
                  Verbose (DEBUG_PROC, "ARC GENERATED (type =%#.8X  name = %s  comment = %s)", node->data->type, node->data->name, node->data->comment);

                  sprintf (buf_name, PATTERN_ARC_NAME, stg->a_num++);
                  sprintf (buf_comment, PATTERN_ARC_COMMENT, node_mid->data->name, node_to->data->name);
                  node = NodeCreate (DataCreate (STG_ARC_CONSUMING | STG_GENERATED, buf_name, buf_comment, node_mid, node_to));
                  NodeAdd (stg->a, node);
                  Verbose (DEBUG_PROC, "ARC GENERATED (type =%#.8X  name = %s  comment = %s)", node->data->type, node->data->name, node->data->comment);
                }
                else
                {
                  node = NodeFind (stg->a, DataFill (mask, STG_ARC, NULL, NULL, node_to, node_from));
                  if (node != NULL)
                  {
                    node->data->type = STG_ARC_READING;
                    // pred = place, succ = transition  => swap if not
                    if ((node_from->data->type & STG_PLACE) == STG_PLACE)
                    {
                      node->data->pred = node_from;
                      node->data->succ = node_to;
                    }
                    Verbose (DEBUG_PROC, "ARC READING DETECTED (name = %s  comment = %s)", node->data->name, node->data->comment);
                  }
                  else
                  {
                    sprintf (buf_name, PATTERN_ARC_NAME, stg->a_num++);
                    sprintf (buf_comment, PATTERN_ARC_COMMENT, node_from->data->name, node_to->data->name);
                    node = NodeCreate (DataCreate (STG_ARC_CONSUMING | STG_NATIVE, buf_name, buf_comment, node_from, node_to));
                    NodeAdd (stg->a, node);
                    Verbose (DEBUG_PROC, "ARC NATIVE (name = %s  comment = %s)", node->data->name, node->data->comment);
                  }
                }
              }
            }
            switch (ch)
            {
              case '#':
                Verbose (DEBUG_PROC, "END ARC(S)");
                Verbose (DEBUG_PROC, "BEGIN COMMENT");
                node_from = NULL;
                pos = _COMMENT;
                break;
              case '\n':
                Verbose (DEBUG_PROC, "END ARC(S)");
                node_from = NULL;
                pos = _UNDEFINED;
                break;
            }
            node_to = NULL;
            s[0] = '\0';
            s_len = 0;
            break;
        }
        break;

      /* .marking */
      case KEY_MARKING:
      case KEY_MARKING_IMPLICIT:
        switch (ch)
        {
          case '\n':
          case '#':
          case ' ':
          case '\t':
          case '{':
          case '}':
          case '<':
          case '>':
            if (!(pos == KEY_MARKING_IMPLICIT && (ch == ' ' || ch == '\t')))
            {
              if (strlen (s) > 0)
              {
                node_mid = NodeFind (stg->p, DataFill (mask, STG_PLACE, NULL, s));
                if (node_mid == NULL)
                {
                  Fatal("reading input line %d marking %s does not exist", line, s);
                }
                else
                {
                  sprintf (buf_name, PATTERN_MARKING_NAME, stg->m_num++);
                  node = NodeCreate (DataCreate (STG_MARKING | STG_NATIVE, buf_name, node_mid->data->name, NULL, NULL, node_mid));
                  NodeAdd (stg->m, node);
                  Verbose (DEBUG_PROC, "MARKING (name = %s  comment = %s)", node->data->name, node->data->comment);
                }
              }
              switch (ch)
              {
                case '<':
                  pos = KEY_MARKING_IMPLICIT;
                  break;
                case '>':
                  pos = KEY_MARKING;
                  break;
                case '#':
                  Verbose (DEBUG_PROC, "END MARKING");
                  Verbose (DEBUG_PROC, "BEGIN COMMENT");
                  pos = _COMMENT;
                  break;
                case '\n':
                  Verbose (DEBUG_PROC, "END MARKING");
                  pos = _UNDEFINED;
                  break;
              }
              s[0] = '\0';
              s_len = 0;
            }
            break;
        }
        break;

      /* ??? */
      default:
        switch (ch)
        {
          case '\n':
            Verbose (DEBUG_PROC, "END ???");
            pos = _UNDEFINED;
            s[0] = '\0';
            s_len = 0;
            break;
        }
        break;

    }

    if (pos == _UNDEFINED || pos == KEY_END ||
       (pos == KEY_INPUTS || pos == KEY_OUTPUTS || pos == KEY_INTERNAL || pos == KEY_DUMMY) && (ch == ' ' || ch == '\t' || ch == '\n') ||
       (pos == _ARCS) &&  (ch == ' ' || ch == '\t' || ch == '\n') ||
       (pos == KEY_MARKING || pos == KEY_MARKING_IMPLICIT) &&  (ch == ' ' || ch == '\t' || ch == '\n' || ch =='{' || ch =='}' || ch =='<' || ch =='>') ||
       (pos == KEY_INITIAL_) &&  (ch == ' ' || ch == '\t' || ch == '\n') ||
       (pos == KEY_INITIAL_STATE) &&  (ch == ' ' || ch == '\t' || ch == '\n'))
       ;
    else
    {
      s_len = strlen (s);
      if (s_len+2 >= s_max)
      {
        s_max = s_max + STG_BUF_INC;
        s = (char *) realloc (s, s_max);
      }
      s[s_len] = ch;
      s[s_len+1] = '\0';
      Verbose (DEBUG_LOCAL, "s = %s", s);
    }
    if (ch == '\n')
    {
      Verbose (DEBUG_PROC, "LINE %d PROCESSED", line);
      line++;
    }
  }
  
  if (stg->model_name == NULL)
  {
    stg->model_name = (char *) malloc (2);
    stg->model_name[0] = SIGN_UNDERSCORE;
    stg->model_name[1] = '\0';
  }

  free (s);
  free (s_);
  free (buf_name);
  free (buf_comment);
  DataFree (mask);
  stg_fill (stg);
  stg_names_compress (stg);

  stg_fill_i (stg);

  Verbose (DEBUG_STACK, "<-- stg_read");
  return 1;
}


/* === */
int stg_write (FILE *outfile, stg_t *stg)
{
  Verbose (DEBUG_STACK, "--> stg_write");
  int i;
  bool label_printed;
  PNode node = NULL;
  PData mask = NULL;

  mask = DataCreate (STG_NATIVE, NULL, NULL, NULL, NULL);

  if (stg->model_name != NULL)
    fprintf (outfile, "%s %s\n", WORD_MODEL, stg->model_name);

  label_printed = false;
  node = stg->s;
  while (node != NULL)
  {
    if (node->data->type == STG_SIGNAL_INPUT)
    {
      if (!label_printed)
      {
        fprintf (outfile, "%s", WORD_INPUTS);
        label_printed = true;
      }
      fprintf (outfile, " %s", node->data->name);
    }
    node = node->next;
  }
  if (label_printed)
    fprintf (outfile, "\n");

  label_printed = false;
  node = stg->s;
  while (node != NULL)
  {
    if (node->data->type == STG_SIGNAL_OUTPUT)
    {
      if (!label_printed)
      {
        fprintf (outfile, "%s", WORD_OUTPUTS);
        label_printed = true;
      }
      fprintf (outfile, " %s", node->data->name);
    }
    node = node->next;
  }
  if (label_printed)
    fprintf (outfile, "\n");

  label_printed = false;
  node = stg->s;
  while (node != NULL)
  {
    if (node->data->type == STG_SIGNAL_INTERNAL)
    {
      if (!label_printed)
      {
        fprintf (outfile, "%s", WORD_INTERNAL);
        label_printed = true;
      }
      fprintf (outfile, " %s", node->data->name);
    }
    node = node->next;
  }
  if (label_printed)
    fprintf (outfile, "\n");

  label_printed = false;
  node = stg->s;
  while (node != NULL)
  {
    if (node->data->type == STG_SIGNAL_DUMMY)
    {
      if (!label_printed)
      {
        fprintf (outfile, "%s", WORD_DUMMY);
        label_printed = true;
      }
      fprintf (outfile, " %s", node->data->name);
    }
    node = node->next;
  }
  if (label_printed)
    fprintf (outfile, "\n");

  label_printed = false;
  node = stg->i;
  while (node != NULL)
  {
    if (node->data->link != NULL)
    {
      if (!label_printed)
      {
        fprintf (outfile, "%s %s", WORD_INITIAL_, WORD_STATE);
        label_printed = true;
      }
      fprintf (outfile, " %s", node->data->name);
    }
    node = node->next;
  }
  if (label_printed)
    fprintf (outfile, "\n");

  fprintf (outfile, "%s\n", WORD_GRAPH);
  node = stg->a;
  while (node != NULL)
  {
    if ((node->data->type & STG_ARC_READING)  == STG_ARC_READING)
    {
      if ((node->data->pred->data->type & STG_REDUNDANT) == STG_REDUNDANT)
      {
        fprintf (outfile, "%s %s \t# %s\n", stg_cnv_redundant (node->data->pred->data->name), node->data->succ->data->name, node->data->name);
        fprintf (outfile, "%s %s \t# %s\n", node->data->succ->data->name, stg_cnv_redundant (node->data->pred->data->name), node->data->name);
      }
      else
      {
        fprintf (outfile, "%s %s \t# %s\n", node->data->pred->data->name, node->data->succ->data->name, node->data->name);
        fprintf (outfile, "%s %s \t# %s\n", node->data->succ->data->name, node->data->pred->data->name, node->data->name);
      }
    }
    else
    {
      if ((node->data->pred->data->type & STG_PLACE) == STG_PLACE)
        if ((node->data->pred->data->type & STG_REDUNDANT) == STG_REDUNDANT)
          fprintf (outfile, "%s %s \t# %s\n", stg_cnv_redundant (node->data->pred->data->name), node->data->succ->data->name, node->data->name);
        else
          fprintf (outfile, "%s %s \t# %s\n", node->data->pred->data->name, node->data->succ->data->name, node->data->name);
      else
        if ((node->data->succ->data->type & STG_REDUNDANT) == STG_REDUNDANT)
          fprintf (outfile, "%s %s \t# %s\n", node->data->pred->data->name, stg_cnv_redundant (node->data->succ->data->name), node->data->name);
        else
          fprintf (outfile, "%s %s \t# %s\n", node->data->pred->data->name, node->data->succ->data->name, node->data->name);
    }
    node = node->next;
  }

  fprintf (outfile, "%s {", WORD_MARKING);
  node = stg->m;
  while (node != NULL)
  {
    if (NodeFind (stg->p, DataFill (mask, STG_PLACE, NULL, NULL, NULL, NULL, node)) != NULL)
    {
      if ((node->data->link->data->type & STG_REDUNDANT) == STG_REDUNDANT)
        fprintf (outfile, "%s", stg_cnv_redundant (node->data->link->data->name));
      else
        fprintf (outfile, "%s", node->data->link->data->name);
      if (node->next != NULL)
        fprintf (outfile, " ");
    }
    node = node->next;
  }
  fprintf (outfile, "}\n");

  fprintf (outfile, "%s\n", WORD_END);

  DataFree (mask);
  Verbose (DEBUG_STACK, "<-- stg_write");
  return 1;
}


/* === */
int stg_debug (FILE *outfile, stg_t *stg, int mask)
{
  Verbose (DEBUG_STACK, "--> stg_write_debug");
  int i                   = 0;
  const int width_name    = 8;
  const int width_comment = 12;

  PNode node;
  if ((mask & STG_SIGNAL) == STG_SIGNAL)
  {
    fprintf (outfile, "Signals:\n");
    node = stg->s;
    while (node != NULL)
    {
      fprintf (outfile, "  & = %p  data = %p  .type = %#.8X  .name = %-*s  .comment = %-*s  .pred = %p  .succ = %p  .link = %p\n", node, node->data, node->data->type, width_name, node->data->name, width_comment, node->data->comment, node->data->pred, node->data->succ, node->data->link);
      node = node->next;
    }
  }

  if ((mask & STG_TRANSITION) == STG_TRANSITION)
  {
    fprintf (outfile, "Transitions:\n");
    node = stg->t;
    while (node != NULL)
    {
      fprintf (outfile, "  & = %p  data = %p  .type = %#.8X  .name = %-*s  .comment = %-*s  .pred = %p  .succ = %p  .link = %p\n", node, node->data, node->data->type, width_name, node->data->name, width_comment, node->data->comment, node->data->pred, node->data->succ, node->data->link);
      node = node->next;
    }
  }

  if ((mask & STG_PLACE) == STG_PLACE)
  {
    fprintf (outfile, "Places:\n");
    node = stg->p;
    while (node != NULL)
    {
      fprintf (outfile, "  & = %p  data = %p  .type = %#.8X  .name = %-*s  .comment = %-*s  .pred = %p  .succ = %p  .link = %p\n", node, node->data, node->data->type, width_name, node->data->name, width_comment, node->data->comment, node->data->pred, node->data->succ, node->data->link);
      node = node->next;
   }
 }

  if ((mask & STG_ARC) == STG_ARC)
  {
    fprintf (outfile, "Arcs:\n");
    node = stg->a;
    while (node != NULL)
    {
      fprintf (outfile, "  & = %p  data = %p  .type = %#.8X  .name = %-*s  .comment = %-*s  .pred = %p  .succ = %p  .link = %p\n", node, node->data, node->data->type, width_name, node->data->name, width_comment, node->data->comment, node->data->pred, node->data->succ, node->data->link);
      node = node->next;
    }
  }

  if ((mask & STG_MARKING) == STG_MARKING)
  {
    fprintf (outfile, "Marking:\n");
    node = stg->m;
    while (node != NULL)
    {
      fprintf (outfile, "  & = %p  data = %p  .type = %#.8X  .name = %-*s  .comment = %-*s  .pred = %p  .succ = %p  .link = %p\n", node, node->data, node->data->type, width_name, node->data->name, width_comment, node->data->comment, node->data->pred, node->data->succ, node->data->link);
      node = node->next;
    }
  }

  if ((mask & STG_INITIAL) == STG_INITIAL)
  {
    fprintf (outfile, "Initial state:\n");
    node = stg->i;
    while (node != NULL)
    {
      fprintf (outfile, "  & = %p  data = %p  .type = %#.8X  .name = %-*s  .comment = %-*s  .pred = %p  .succ = %p  .link = %p\n", node, node->data, node->data->type, width_name, node->data->name, width_comment, node->data->comment, node->data->pred, node->data->succ, node->data->link);
      node = node->next;
    }
  }

  Verbose (DEBUG_STACK, "<-- stg_write_debug");
  return 1;
}
/* INPUT-OUTPUT END*/


/* SIGNAL LEVEL BEGIN */
/* === */
int stg_signal_level_find (stg_t *stg, PNode node_signal, PNode node_place_start)
{
  Verbose (DEBUG_STACK, "--> stg_signal_level_find");
  PNode node_place = NULL;
  PNode node_trans = NULL;
  int level        = -1;

  node_trans = node_place_start->data->succ;
  while (level == -1 && node_trans != NULL)
  {
    if (node_trans->data->link == node_signal)
    {
      if (stg_cnv_polarity (node_trans->data->name) == SIGN_POSITIVE)
        level = 0;
      else
      if (stg_cnv_polarity (node_trans->data->name) == SIGN_NEGATIVE)
        level = 1;
    }
    else
    if (node_trans->data->pred->next == NULL)
    {
      node_place = node_trans->data->succ;
      while (level == -1 && node_place != NULL)
      {
        if ((node_place->data->type & STG_PROCESSED) != STG_PROCESSED)
        {
          node_place->data->type |= STG_PROCESSED;
          level = stg_signal_level_find (stg, node_signal, node_place);
          node_place->data->type &= ~STG_PROCESSED;
        }
        node_place = node_place->next;
      }
    }
    node_trans = node_trans->next;
  }

  Verbose (DEBUG_STACK, "<-- stg_signal_level_find");
  return level;
}


/* === */
int stg_signal_level (stg_t *stg, PNode node_signal)
{
  Verbose (DEBUG_STACK, "--> stg_signal_level");
  PNode node_place = NULL;
  PNode node_mark  = NULL;
  int level        = -1;
  
  stg_type_and (stg->p, ~STG_PROCESSED);
  stg_type_and (stg->t, ~STG_PROCESSED);

  node_mark = stg->m;
  while (level == -1 && node_mark != NULL)
  {
    node_place = node_mark->data->link;
    node_place->data->type |= STG_PROCESSED;
    level = stg_signal_level_find (stg, node_signal, node_place);
    node_place->data->type &= ~STG_PROCESSED;
    node_mark = node_mark->next;
  }

  Verbose (DEBUG_STACK, "<-- stg_signal_level");
  return level;
}
/* SIGNAL LEVEL END */


/* CLUSTER BEGIN */
/*===*/
int stg_cluster_find (stg_t *stg, PNode head, int cluster_index)
{
  Verbose (DEBUG_STACK, "--> stg_cluster_find");
  PNode node;
  
  head->data->index_1 = cluster_index;

  node = head->data->pred;
  while (node != NULL)
  {
    if (node->data->index_1 != cluster_index)
      stg_cluster_find (stg, node, cluster_index);
    node = node->next;
  }

  node = head->data->succ;
  while (node != NULL)
  {
    if (node->data->index_1 != cluster_index)
      stg_cluster_find (stg, node, cluster_index);
    node = node->next;
  }
  
  Verbose (DEBUG_STACK, "<-- stg_cluster_find");
}


/*===*/
int stg_cluster (stg_t *stg)
{
  Verbose (DEBUG_STACK, "--> stg_cluster");
  PNode node;
  int cluster_index = -1;
  int cluster_num = 0;
  bool done = false;
  
  node = stg->t;
  while (node != NULL)
  {
    node->data->index_1 = cluster_index;
    node = node->next;
  }
  node = stg->p;
  while (node != NULL)
  {
    node->data->index_1 = cluster_index;
    node = node->next;
  }
  
  node = stg->p;
  while (node != NULL)
  {
    if (node->data->index_1 < 0)
      stg_cluster_find (stg, node, cluster_index++);
    node = node->next;
  }
  
  cluster_num = cluster_index+1;
  for (cluster_index = 0; cluster_index < cluster_num; cluster_index++)                                                                               
  {                                                                                                                                                   
    PNode signal_cur = stg_cluster_type (stg, cluster_index);
    if (signal_cur != NULL)
    {
      node = stg->p;
      while (node != NULL)                                                                                                                              
      {                                                                                                                                                 
        if (node->data->index_1 == cluster_index)
	{
          if ((signal_cur->data->type & STG_SIGNAL_INPUT) == STG_SIGNAL_INPUT)
            node->data->type |= STG_PLACE_INPUT;
          else
          if ((signal_cur->data->type & STG_SIGNAL_OUTPUT) == STG_SIGNAL_OUTPUT)
            node->data->type |= STG_PLACE_OUTPUT;
	}
        node = node->next;                                                                                                                              
      }
    }
  }
  
  Verbose (DEBUG_STACK, "<-- stg_cluster");
  return cluster_index;
}


/* === */
PNode stg_cluster_type (stg_t *stg, int cluster_index)
{
  Verbose (DEBUG_STACK, "--> stg_cluster_type");
  PNode node_signal       = NULL;
  PNode node_trans        = NULL;
  const int type_mask     = STG_INPUT | STG_OUTPUT | STG_INTERNAL | STG_DUMMY;
  int type                = STG_EMPTY;

  node_signal = NULL;
  type = STG_EMPTY;
  node_trans = stg->t;
  while (node_trans != NULL)
  {
    if (node_trans->data->index_1 == cluster_index)
    {
      if (type == STG_EMPTY)
      {
        type = node_trans->data->type & type_mask;
        node_signal = node_trans->data->link;
      }  
      else
      if (type != (node_trans->data->type & type_mask))
      {
        node_signal = NULL;
        break;
      }
    }
    node_trans = node_trans->next;
  }
  if (type == STG_DUMMY)
    node_signal = NULL;
  Verbose (DEBUG_STACK, "<-- stg_cluster_type");
  return node_signal;
}
/* CLUSTER END */
