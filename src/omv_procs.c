#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "debug.h"
#include "list.h"
#include "stg.h"
#include "stg_procs.h"
#include "strutils.h"
#include "element.h"
#include "verilog.h"
#include "omv_procs.h"


/* === */
omv_t *omv_create ()
{
  Verbose (DEBUG_STACK, "--> omv_create");

  omv_t *omv;

  omv = (omv_t *) malloc (sizeof (omv_t));
  omv->dc = NULL;
  omv->ff = NULL;

  Verbose (DEBUG_STACK, "<-- omv_create");
  return omv;
}


/* === */
int omv_destroy (omv_t *omv)
{
  Verbose (DEBUG_STACK, "--> omv_destroy");
  NodeClear (omv->dc);  NodeFree (omv->dc);
  NodeClear (omv->ff);  NodeFree (omv->ff);
  NodeClear (omv->tst);  NodeFree (omv->tst);
  free (omv);
  Verbose (DEBUG_STACK, "<-- omv_destroy");
  return 1;
}


/* === */
int omv_sort (omv_t *omv)
{
  Verbose (DEBUG_STACK, "--> omv_sort");
  NodeSort (omv->dc);
  NodeSort (omv->ff);
  Verbose (DEBUG_STACK, "<-- omv_sort");
  return 1;
}


/* === */
int omv_calculate (stg_t *stg, omv_t *omv, int flag)
{
  Verbose (DEBUG_STACK, "--> omv_david_cell");
  PNode node_signal = NULL;
  PNode node_dc     = NULL;
  PNode node_ff     = NULL;
  PNode node_trans  = NULL;
  PNode node_place  = NULL;
  PNode node_arc    = NULL;
  int cluster_num   = 0;
  int cluster_index = 0;
  char *s1          = NULL;
  int s1_len        = 0;
  char *s2          = NULL;
  int s2_len        = 0;
  char *s           = NULL;
  int fanin_or      = 0;
  int fanin_c       = 0;
  int fanin_and     = 0;
  int i, dum;
  PNode node_pred   = NULL; // set or request
  PNode node_succ   = NULL; // reset or acknowledgement
  PData mask        = NULL;

  mask = DataCreate (STG_NATIVE, NULL, NULL, NULL, NULL);
  stg_pred_succ_sort (stg);
  cluster_num = stg_cluster (stg);
  
  NodeClear (omv->dc);  NodeFree (omv->dc);
  NodeClear (omv->ff);  NodeFree (omv->ff);
  
  omv->model_name = (char *) realloc (omv->model_name, strlen (stg->model_name) + 1);
  strcpy (omv->model_name, stg->model_name);
  
  for (cluster_index = 0; cluster_index < cluster_num; cluster_index++)
  {
    Verbose (DEBUG_PROC, "CLUSTER %d(of %d)", cluster_index, cluster_num);
    if ((node_signal = stg_cluster_type (stg, cluster_index)) != NULL)
    {
      //=== create exposed FF
      Verbose (DEBUG_PROC, "  FF %s", node_signal->data->name);
      node_pred = NULL;
      node_succ = NULL;

      node_place = stg->p;
      while (node_place != NULL)
      {
        Verbose (DEBUG_PROC, "  PLACE FOUND %s", node_place->data->name);
        if (node_place->data->index_1 == cluster_index)
        {
          i = strlen (node_place->data->name) - 1;
          switch (node_place->data->name[i])
          {
            case SIGN_HIGH:
              //--- set
              Verbose (DEBUG_PROC, "    SET LOGIC");
              fanin_or = NodeCount (node_place->data->pred);
              s1_len = 0;
              s1 = (char *) realloc (s1, s1_len + 1);
              s1[s1_len] = '\0';
              if (fanin_or > 1)
              {
                s1_len += strlen (PATTERN_OR);
                s1 = (char *) realloc (s1, s1_len + 1);
                sprintf (s1, PATTERN_OR, fanin_or);
              }
              Verbose (DEBUG_PROC, "      OR%d GATE FOUND (s1 = %s)", fanin_or, s1);

              node_trans = node_place->data->pred;
              while (node_trans != NULL)
              {
                // AND-element
                mask = DataFill (mask, STG_ARC_READING, NULL, NULL, NULL, NodeLocate (stg->t, node_trans->data));
                fanin_and = 0;
                node_arc = stg->a;
                while ((node_arc = NodeFind (node_arc, mask)) != NULL)
                {
                  NodeAdd (node_pred, NodeCreate (node_arc->data->pred->data));
                  fanin_and++;
                  node_arc = node_arc->next;
                }
                Verbose (DEBUG_PROC, "      AND%d GATE FOUND", fanin_and);

                if (fanin_and > 1)
                {
                  s1_len += strlen (PATTERN_AND) + fanin_and * strlen (PATTERN_INPUT);
                  s1 = (char *) realloc (s1, s1_len + 1);
                  strcat (s1, PATTERN_AND);
                  sprintf (s1, s1, fanin_and);
                  for (i = 0; i < fanin_and; i++)
                    strcat (s1, PATTERN_INPUT);
                }
                else
                {
                  s1_len += strlen (PATTERN_INPUT);
                  s1 = (char *) realloc (s1, s1_len + 1);
                  strcat (s1, PATTERN_INPUT);
                }
                node_trans = node_trans->next;
              }
              Verbose (DEBUG_PROC, "    SET LOGIC FOR FF %s COMPOSED (%s)", node_signal->data->name, s1);
              break;

            case SIGN_LOW:
              //--- reset
              Verbose (DEBUG_PROC, "    RESET LOGIC");
              fanin_or = NodeCount (node_place->data->pred);
              s2_len = 0;
              s2 = (char *) realloc (s2, s2_len + 1);
              s2[s2_len] = '\0';
              if (fanin_or > 1)
              {
                s2_len += strlen (PATTERN_OR);
                s2 = (char *) realloc (s2, s2_len + 1);
                sprintf (s2, PATTERN_OR, fanin_or);
              }
              Verbose (DEBUG_PROC, "      OR%d GATE FOUND (s2 = %s)", fanin_or, s2);
              
              node_trans = node_place->data->pred;
              while (node_trans != NULL)
              {
                // AND-element
                mask = DataFill (mask, STG_ARC_READING, NULL, NULL, NULL, NodeLocate (stg->t, node_trans->data));
                fanin_and = 0;
                node_arc = stg->a;
                while ((node_arc = NodeFind (node_arc, mask)) != NULL)
                {
                  NodeAdd (node_succ, NodeCreate (node_arc->data->pred->data));
                  fanin_and++;
                  node_arc = node_arc->next;
                }
                Verbose (DEBUG_PROC, "      AND%d GATE FOUND", fanin_and);

                if (fanin_and > 1)
                {
                  s2_len += strlen (PATTERN_AND) + fanin_and * strlen (PATTERN_INPUT);
                  s2 = (char *) realloc (s2, s2_len + 1);
                  strcat (s2, PATTERN_AND);
                  sprintf (s2, s2, fanin_and);
                  for (i = 0; i < fanin_and; i++)
                    strcat (s2, PATTERN_INPUT);
                }
                else
                {
                  s2_len += strlen (PATTERN_INPUT);
                  s2 = (char *) realloc (s2, s2_len + 1);
                  strcat (s2, PATTERN_INPUT);
                }

                node_trans = node_trans->next;
              }
              Verbose (DEBUG_PROC, "    RESET LOGIC FOR FF %s COMPOSED (%s)", node_signal->data->name, s2);
              break;
          }
        }
        node_place = node_place->next;
      }

      NodeRevert (node_pred);
      NodeRevert (node_succ);
      s = (char *) realloc (s, strlen (PATTERN_FF) + strlen (s1) +strlen (s2) + 1);
      sprintf (s, PATTERN_FF, s1, s2);
      node_ff = NodeCreate (DataCreate (OMV_FF | (node_signal->data->type & (STG_INPUT | STG_OUTPUT | STG_INTERNAL)), node_signal->data->name, s, node_pred, node_succ, NULL));
      NodeAdd(omv->ff, node_ff);
      Verbose (DEBUG_PROC, "    FF WAS ADDED (type =%#.8X  name = %s  comment = %s)", node_ff->data->type, node_ff->data->name, node_ff->data->comment);
    }
    else
    {
      //=== create DC
      Verbose (DEBUG_PROC, "  DCs");
      node_place = stg->p;
      while (node_place != NULL)
      {
        node_pred = NULL;
        node_succ = NULL;
        Verbose (DEBUG_PROC, "  PLACE FOUND %s", node_place->data->name);
        if (node_place->data->index_1 == cluster_index)
        {
          //--- request
          Verbose (DEBUG_PROC, "    REQUEST LOGIC");
          fanin_or = NodeCount (node_place->data->pred);
          s1_len = 0;
          s1 = (char *) realloc (s1, s1_len + 1);
          s1[s1_len] = '\0';
          if (fanin_or > 1)
          {
            s1_len += strlen (PATTERN_OR);
            s1 = (char *) realloc (s1, s1_len + 1);
            sprintf (s1, PATTERN_OR, fanin_or);
          }
          Verbose (DEBUG_PROC, "      OR%d GATE FOUND (s1 = %s)", fanin_or, s1);

          node_trans = node_place->data->pred;
          while (node_trans != NULL)
          {
            // AND-element
            mask = DataFill (mask, STG_ARC_READING, NULL, NULL, NULL, NodeLocate (stg->t, node_trans->data));
            fanin_and = 0;
            node_arc = stg->a;
            while ((node_arc = NodeFind (node_arc, mask)) != NULL)
            {
              NodeAdd (node_pred, NodeCreate (node_arc->data->pred->data));
              fanin_and++;
              node_arc = node_arc->next;
            }
            Verbose (DEBUG_PROC, "      AND%d GATE FOUND", fanin_and);

            // C-element
            mask = DataFill (mask, STG_ARC_CONSUMING, NULL, NULL, NULL, NodeLocate (stg->t, node_trans->data));
            fanin_c = 0;
            node_arc = stg->a;
            while ((node_arc = NodeFind (node_arc, mask)) != NULL)
            {
              NodeAdd (node_pred, NodeCreate (node_arc->data->pred->data));
              fanin_c++;
              node_arc = node_arc->next;
            }
            Verbose (DEBUG_PROC, "      C%d GATE FOUND", fanin_c);

            if (fanin_c > 0) fanin_and++;
            if (fanin_c == 1) fanin_c = 0;

            if (fanin_and > 1)
            {
              dum = fanin_and;
              if (fanin_c > 0) dum--;
              s1_len += strlen (PATTERN_AND) + dum * strlen (PATTERN_INPUT);
              s1 = (char *) realloc (s1, s1_len + 1);
              strcat (s1, PATTERN_AND);
              sprintf (s1, s1, fanin_and);
              for (i = 0; i < dum; i++)
                strcat (s1, PATTERN_INPUT);
            }

            if (fanin_c > 1)
            {
              s1_len += strlen (PATTERN_C) + fanin_c * strlen (PATTERN_INPUT);
              s1 = (char *) realloc (s1, s1_len + 1);
              strcat (s1, PATTERN_C);
              sprintf (s1, s1, fanin_c);
              for (i = 0; i < fanin_c; i++)
                strcat (s1, PATTERN_INPUT);
            }

            if (fanin_and + fanin_c == 1)
            {
              s1_len += strlen (PATTERN_INPUT);
              s1 = (char *) realloc (s1, s1_len + 1);
              strcat (s1, PATTERN_INPUT);
            }

            node_trans = node_trans->next;
          }
          Verbose (DEBUG_PROC, "    REQUEST LOGIC FOR PLACE %s COMPOSED %s", node_place->data->name, s1);

          //--- acknowledgement
          Verbose (DEBUG_PROC, "    ACKNOWLEDGEMENT LOGIC");
          fanin_or = NodeCount (node_place->data->succ);
          s2_len = 0;
          s2 = (char *) realloc (s2, s2_len + 1);
          s2[s2_len] = '\0';
          if (fanin_or > 1)
          {
            s2_len += strlen (PATTERN_OR);
            s2 = (char *) realloc (s2, s2_len + 1);
            sprintf (s2, PATTERN_OR, fanin_or);
          }
          Verbose (DEBUG_PROC, "      OR%d GATE FOUND (s2 = %s)", fanin_or, s2);
          node_trans = node_place->data->succ;
          while (node_trans != NULL)
          {
            // C-element
            mask = DataFill (mask, STG_ARC_CONSUMING, NULL, NULL, NodeLocate (stg->t, node_trans->data), NULL);
            fanin_c = 0;
            node_arc = stg->a;
            while ((node_arc = NodeFind (node_arc, mask)) != NULL)
            {
              NodeAdd (node_succ, NodeCreate (node_arc->data->succ->data));
              if ((flag & OMV_TEST) == OMV_TEST)
              {
                char* buf_name = new char [strlen(node_place->data->name) + strlen(node_arc->data->succ->data->name) + strlen(PATTERN_SIGNAL_TST)];
                sprintf (buf_name, PATTERN_SIGNAL_TST, node_place->data->name, node_arc->data->succ->data->name);
                PData signal_tst = DataCreate (STG_SIGNAL_INPUT | STG_GENERATED, buf_name, NULL);
                NodeAdd (omv->tst, NodeCreate (signal_tst));
                NodeAdd (node_succ, NodeCreate (signal_tst));
              }
              fanin_c++;
              node_arc = node_arc->next;
            }
            Verbose (DEBUG_PROC, "      C%d GATE FOUND", fanin_c);

            if (fanin_c > 1)
            {
              s2_len += strlen (PATTERN_C) + fanin_c * strlen (PATTERN_INPUT) + strlen (PATTERN_AND_2);
              s2 = (char *) realloc (s2, s2_len + 1);
              strcat (s2, PATTERN_C);
              sprintf (s2, s2, fanin_c);
              for (i = 0; i < fanin_c; i++)
              {
                if ((flag & OMV_TEST) == OMV_TEST)
                  strcat (s2, PATTERN_AND_2);
                else
                  strcat (s2, PATTERN_INPUT);
              }
            }
            else
            {
              s2_len += strlen (PATTERN_INPUT) + strlen (PATTERN_AND_2);
              s2 = (char *) realloc (s2, s2_len + 1);
              if ((flag & OMV_TEST) == OMV_TEST)
                strcat (s2, PATTERN_AND_2);
              else
                strcat (s2, PATTERN_INPUT);
            }
            node_trans = node_trans->next;
          }
          Verbose (DEBUG_PROC, "    ACKNOWLEDGEMENT LOGIC FOR PLACE %s COMPOSED %s", node_place->data->name, s2);

          NodeRevert (node_pred);
          NodeRevert (node_succ);
          s = (char *) realloc (s, strlen (PATTERN_DC) + strlen (s1) +strlen (s2) + 1);
          sprintf (s, PATTERN_DC, s1, s2);
          node_dc = NodeCreate (DataCreate (OMV_DC, node_place->data->name, s, node_pred, node_succ, NULL));
          NodeAdd(omv->dc, node_dc);
          Verbose (DEBUG_PROC, "    DC WAS ADDED (type =%#.8X  name = %s  comment = %s  prev = %p  next = %p)", node_dc->data->type, node_dc->data->name, node_dc->data->comment, node_dc->prev, node_dc->next);
        }
        node_place = node_place->next;
      }
    }
  }

  free (s1);
  free (s2);
  free (s);
  DataFree (mask);
  Verbose (DEBUG_STACK, "<-- omv_david_cell");
  return 1;
}


/* === */
// output_type = REQ_SET || ACK_RESET
int omv_count_dc_fanout (omv_t *omv, PNode node_gate, int output_type)
{
  Verbose (DEBUG_STACK, "--> omv_count_fanout");
  PNode node_dc    = NULL;
  PNode node_ff    = NULL;
  PNode node_place = NULL;
  int fanout       = 0;

  fanout = 0;
    
  Verbose (DEBUG_PROC, "DCs vs node_gate = %s", node_gate->data->name);
  node_dc = omv->dc;
  while (node_dc != NULL)
  {
    Verbose (DEBUG_PROC, "  node_dc = %s", node_dc->data->name);
    node_place = NULL;
    if (output_type == EL_REQ_SET)
      node_place = node_dc->data->pred;
    if (output_type == EL_ACK_RESET)
      node_place = node_dc->data->succ;
    while (node_place != NULL)
    {
      Verbose (DEBUG_PROC, "    node_place = %s", node_place->data->name);
      if (strcmp (node_place->data->name, node_gate->data->name) == 0)
        fanout++;
      node_place = node_place->next;
    }
    node_dc = node_dc->next;
  }

  if (output_type == EL_REQ_SET)
  {
    Verbose (DEBUG_PROC, "FFs vs node_gate = %s", node_gate->data->name);
    node_ff = omv->ff;
    while (node_ff != NULL)
    {
      Verbose (DEBUG_PROC, "  node_ff = %s", node_ff->data->name);
      if ((node_ff->data->type & STG_INPUT) != STG_INPUT)
      {
        node_place = node_ff->data->succ;
        while (node_place != NULL)
        {
          Verbose (DEBUG_PROC, "    node_place = %s", node_place->data->name);
          if (strcmp (node_place->data->name, node_gate->data->name) == 0)
            fanout++;
          node_place = node_place->next;
        }
      }
      node_ff = node_ff->next;
    }
  }

  if (output_type == EL_ACK_RESET)
  {
    Verbose (DEBUG_PROC, "FFs vs node_gate = %s", node_gate->data->name);
    node_ff = omv->ff;
    while (node_ff != NULL)
    {
      Verbose (DEBUG_PROC, "  node_ff = %s", node_ff->data->name);
      if ((node_ff->data->type & STG_INPUT) != STG_INPUT)
      {
        node_place = node_ff->data->pred;
        while (node_place != NULL)
        {
          Verbose (DEBUG_PROC, "    node_place = %s", node_place->data->name);
          if (strcmp (node_place->data->name, node_gate->data->name) == 0)
            fanout++;
          node_place = node_place->next;
        }
      }
      node_ff = node_ff->next;
    }
  }

  Verbose (DEBUG_STACK, "<-- omv_count_fanout (result = %d)", fanout);
  return fanout;
}


/* === */
// output_type = OMV_REQ_SET || OMV_ACK_RESET
int omv_count_ff_fanout (omv_t *omv, PNode node_gate, int output_type)
{
  Verbose (DEBUG_STACK, "--> omv_count_fanout");
  PNode node_dc    = NULL;
  PNode node_ff    = NULL;
  PNode node_place = NULL;
  char *s          = NULL;
  int fanout       = 0;

  fanout = 0;
  if (output_type == EL_REQ_SET)
  {
    s = (char *) malloc (strlen (node_gate->data->name) + strlen (PATTERN_NAME_HIGH) + 1);
    sprintf (s, PATTERN_NAME_HIGH, node_gate->data->name);
  }
  if (output_type == EL_ACK_RESET)
  {
    s = (char *) malloc (strlen (node_gate->data->name) + strlen (PATTERN_NAME_LOW) + 1);
    sprintf (s, PATTERN_NAME_LOW, node_gate->data->name);
  }
    
  Verbose (DEBUG_PROC, "DCs vs node_gate = %s  s = %s", node_gate->data->name, s);
  node_dc = omv->dc;
  while (node_dc != NULL)
  {
    Verbose (DEBUG_PROC, "  node_dc = %s", node_dc->data->name);
    node_place = node_dc->data->pred;
    while (node_place != NULL)
    {
      Verbose (DEBUG_PROC, "    node_place = %s", node_place->data->name);
      if (strcmp (node_place->data->name, s) == 0)
        fanout++;
      node_place = node_place->next;
    }
    node_dc = node_dc->next;
  }

  Verbose (DEBUG_PROC, "FFs vs node_gate = %s", node_gate->data->name);
  node_ff = omv->ff;
  while (node_ff != NULL)
  {
    Verbose (DEBUG_PROC, "  node_ff = %s", node_ff->data->name);
    node_place = node_ff->data->pred;
    while (node_place != NULL)
    {
      Verbose (DEBUG_PROC, "    stg_cnv_invert (node_place) = %s", stg_cnv_invert (node_place->data->name));
      if (strcmp (stg_cnv_invert (node_place->data->name), s) == 0)
        fanout++;
      node_place = node_place->next;
    }
    node_place = node_ff->data->succ;
    while (node_place != NULL)
    {
      Verbose (DEBUG_PROC, "    node_place = %s", node_place->data->name);
      if (strcmp (node_place->data->name, s) == 0)
        fanout++;
      node_place = node_place->next;
    }
    node_ff = node_ff->next;
  }

  free (s);
  Verbose (DEBUG_STACK, "<-- omv_count_fanout (result = %d)", fanout);
  return fanout;
}


/* === */
int omv_info_write (FILE *outfile, omv_t *omv, int flag)
{
  Verbose (DEBUG_STACK, "--> omv_write_info");
  PNode node_dc        = NULL;
  PNode node_ff        = NULL;
  PNode node           = NULL;
  int dc_trans_n_num, dc_trans_p_num, dc_fanin_req, dc_fanin_ack, dc_fanout_req, dc_fanout_ack , dc_num;
  int ff_trans_n_num, ff_trans_p_num, ff_fanin_set, ff_fanin_reset, ff_fanout_set, ff_fanout_reset, ff_num;
  int trans_n_num, trans_p_num;
  int fanin_req_set, fanin_ack_reset, fanout_req_set, fanout_ack_reset;
  int dum;
  const int name_width = 20;
  const int comment_width = 30;
  const int num_width  = 4;

  dc_trans_n_num = 0;
  dc_trans_p_num = 0;
  dc_fanin_req = 0;
  dc_fanin_ack = 0;
  dc_fanout_req = 0;
  dc_fanout_ack = 0;
  dc_num = 0;

  ff_trans_n_num = 0;
  ff_trans_p_num = 0;
  ff_fanin_set = 0;
  ff_fanin_reset = 0;
  ff_fanout_set = 0;
  ff_fanout_reset = 0;
  ff_num = 0;

  if (flag == OMV_EMPTY)
  {
    node_dc = omv->dc;
    while (node_dc != NULL)
    {
      trans_p_num = 2 + el_count_trans (node_dc->data->comment, EL_REQ_SET);
      trans_n_num = 2 + el_count_trans (node_dc->data->comment, EL_ACK_RESET);
      fanin_req_set = el_count_fanin (node_dc->data->comment, EL_REQ_SET);
      fanin_ack_reset = el_count_fanin (node_dc->data->comment, EL_ACK_RESET);
      fanout_req_set = omv_count_dc_fanout (omv, node_dc, EL_REQ_SET);
      fanout_ack_reset = omv_count_dc_fanout (omv, node_dc, EL_ACK_RESET);
      dc_trans_p_num += trans_p_num;
      dc_trans_n_num += trans_n_num;
      if (dc_fanin_req < fanin_req_set) dc_fanin_req = fanin_req_set;
      if (dc_fanin_ack < fanin_ack_reset) dc_fanin_ack = fanin_ack_reset;
      if (dc_fanout_req < fanout_req_set) dc_fanout_req = fanout_req_set;
      if (dc_fanout_ack < fanout_ack_reset) dc_fanout_ack = fanout_ack_reset;
      dc_num++;
      node_dc = node_dc->next;
    }
    node_ff = omv->ff;
    while (node_ff != NULL)
    {
      if ((node_ff->data->type & OMV_INPUT) != OMV_INPUT)
      {
        trans_p_num = 2 + el_count_trans (node_ff->data->comment, EL_REQ_SET);
        trans_n_num = 2 + el_count_trans (node_ff->data->comment, EL_ACK_RESET);
        fanin_req_set = el_count_fanin (node_ff->data->comment, EL_REQ_SET);
        fanin_ack_reset = el_count_fanin (node_ff->data->comment, EL_ACK_RESET);
        fanout_req_set = omv_count_ff_fanout (omv, node_ff, EL_REQ_SET);
        fanout_ack_reset = omv_count_ff_fanout (omv, node_ff, EL_ACK_RESET);
        ff_trans_p_num += trans_p_num;
        ff_trans_n_num += trans_n_num;
        if (ff_fanin_set < fanin_req_set) ff_fanin_set = fanin_req_set;
        if (ff_fanin_reset < fanin_ack_reset) ff_fanin_reset = fanin_ack_reset;
        if (ff_fanout_set < fanout_req_set) ff_fanout_set = fanout_req_set;
        if (ff_fanout_reset < fanout_ack_reset) ff_fanout_reset = fanout_ack_reset;
        ff_num++;
      }
      node_ff = node_ff->next;
    }
  }

  fprintf (outfile, "/*\n");
  node_dc = omv->dc;
  if (node_dc != NULL && (flag & OMV_DC) == OMV_DC)
  {
    fprintf (outfile, "  \n");
    fprintf (outfile, "  DC list:\n");
    fprintf (outfile, "  %-*s %-*s %*s %*s %*s %*s %*s %*s %s\n", name_width, "name", comment_width, "type", num_width, "p", num_width, "n", num_width, "r-fi", num_width, "a-fi", num_width, "r-fo", num_width, "a-fo", "list (req | ack)");
    fprintf (outfile, "  \n");
    while (node_dc != NULL)
    {
      trans_p_num = 2 + el_count_trans (node_dc->data->comment, EL_REQ_SET);
      trans_n_num = 2 + el_count_trans (node_dc->data->comment, EL_ACK_RESET);
      fanin_req_set = el_count_fanin (node_dc->data->comment, EL_REQ_SET);
      fanin_ack_reset = el_count_fanin (node_dc->data->comment, EL_ACK_RESET);
      fanout_req_set = omv_count_dc_fanout (omv, node_dc, EL_REQ_SET);
      fanout_ack_reset = omv_count_dc_fanout (omv, node_dc, EL_ACK_RESET);
      fprintf (outfile, "  %-*s %-*s %*d %*d %*d %*d %*d %*d", name_width, node_dc->data->name, comment_width, node_dc->data->comment, num_width, trans_p_num, num_width, trans_n_num, num_width, fanin_req_set, num_width, fanin_ack_reset, num_width, fanout_req_set, num_width, fanout_ack_reset);
      // request
      node = node_dc->data->pred;
      while (node != NULL)
      {
        fprintf (outfile, " %s", node->data->name);
        node = node->next;
      }
      fprintf (outfile, " |");
      // acknowledgement
      node = node_dc->data->succ;
      while (node != NULL)
      {
        fprintf (outfile, " %s", node->data->name);
        node = node->next;
      }
      fprintf (outfile, "\n");
      dc_trans_p_num += trans_p_num;
      dc_trans_n_num += trans_n_num;
      if (dc_fanin_req < fanin_req_set) dc_fanin_req = fanin_req_set;
      if (dc_fanin_ack < fanin_ack_reset) dc_fanin_ack = fanin_ack_reset;
      if (dc_fanout_req < fanout_req_set) dc_fanout_req = fanout_req_set;
      if (dc_fanout_ack < fanout_ack_reset) dc_fanout_ack = fanout_ack_reset;
      dc_num++;
      node_dc = node_dc->next;
    }
    fprintf (outfile, "  --------------------\n");
    fprintf (outfile, "  %*d %-*s %*d %*d %*d %*d %*d %*d\n", name_width, dc_num, comment_width, "", num_width, dc_trans_p_num, num_width, dc_trans_n_num, num_width, dc_fanin_req, num_width, dc_fanin_ack, num_width, dc_fanout_req, num_width, dc_fanout_ack);
  }

  node_ff = omv->ff;
  if (node_ff != NULL && (flag & OMV_FF) == OMV_FF)
  {
    fprintf (outfile, "  \n");
    fprintf (outfile, "  FF list\n");
    fprintf (outfile, "  %-*s %-*s %*s %*s %*s %*s %*s %*s %s\n", name_width, "name", comment_width, "type", num_width, "p", num_width, "n", num_width, "s-fi", num_width, "r-fi", num_width, "1-fo", num_width, "0-fo", "list (set | reset)");
    fprintf (outfile, "  \n");
    while (node_ff != NULL)
    {
      if ((node_ff->data->type & OMV_INPUT) != OMV_INPUT || (flag & OMV_INPUT) == OMV_INPUT)
      {
        trans_p_num = 2 + el_count_trans (node_ff->data->comment, EL_REQ_SET);
        trans_n_num = 2 + el_count_trans (node_ff->data->comment, EL_ACK_RESET);
        fanin_req_set = el_count_fanin (node_ff->data->comment, EL_REQ_SET);
        fanin_ack_reset = el_count_fanin (node_ff->data->comment, EL_ACK_RESET);
        fanout_req_set = omv_count_ff_fanout (omv, node_ff, EL_REQ_SET);
        fanout_ack_reset = omv_count_ff_fanout (omv, node_ff, EL_ACK_RESET);
          if ((flag & OMV_FF_INPUT) == OMV_FF_INPUT)
          {
            if ((node_ff->data->type & OMV_INPUT) == OMV_INPUT)
              fprintf (outfile, "  i: ");
            else
              fprintf (outfile, "  o: ");
            fprintf (outfile, "%-*s %-*s %*d %*d %*d %*d%*d+%*d+", name_width-3, node_ff->data->name, comment_width, node_ff->data->comment, num_width, trans_p_num, num_width, trans_n_num, num_width, fanin_req_set, num_width, fanin_ack_reset, num_width, fanout_req_set, num_width, fanout_ack_reset);
          }
          else
            fprintf (outfile, "  %-*s %-*s %*d %*d %*d %*d%*d+%*d+", name_width, node_ff->data->name, comment_width, node_ff->data->comment, num_width, trans_p_num, num_width, trans_n_num, num_width, fanin_req_set, num_width, fanin_ack_reset, num_width, fanout_req_set, num_width, fanout_ack_reset);
        node = node_ff->data->pred;
        while (node != NULL)
        {
          fprintf (outfile, " %s", node->data->name);
          node = node->next;
        }
        fprintf (outfile, " |");
        node = node_ff->data->succ;
        while (node != NULL)
        {
          fprintf (outfile, " %s", node->data->name);
          node = node->next;
        }
        fprintf (outfile, "\n");
        ff_trans_p_num += trans_p_num;
        ff_trans_n_num += trans_n_num;
        if (ff_fanin_set < fanin_req_set) ff_fanin_set = fanin_req_set;
        if (ff_fanin_reset < fanin_ack_reset) ff_fanin_reset = fanin_ack_reset;
        if (ff_fanout_set < fanout_req_set) ff_fanout_set = fanout_req_set;
        if (ff_fanout_reset < fanout_ack_reset) ff_fanout_reset = fanout_ack_reset;
        ff_num++;
      }
      node_ff = node_ff->next;
    }
    fprintf (outfile, "  --------------------\n");
    fprintf (outfile, "  %*d %-*s %*d %*d %*d %*d%*d+%*d+\n", name_width, ff_num, comment_width, "", num_width, ff_trans_p_num, num_width, ff_trans_n_num, num_width, ff_fanin_set, num_width, ff_fanin_reset, num_width, ff_fanout_set, num_width, ff_fanout_reset);
  }

  if (dc_fanin_req > ff_fanin_set)
    fanin_req_set = dc_fanin_req;
  else
    fanin_req_set = ff_fanin_set;
  if (dc_fanin_ack > ff_fanin_reset)
    fanin_ack_reset = dc_fanin_ack;
  else
    fanin_ack_reset = ff_fanin_reset;

  if (dc_fanout_req > ff_fanout_set)
    fanout_req_set = dc_fanout_req;
  else
    fanout_req_set = ff_fanout_set;
  if (dc_fanout_ack > ff_fanout_reset)
    fanout_ack_reset = dc_fanout_ack;
  else
    fanout_ack_reset = ff_fanout_reset;

  if (flag == OMV_EMPTY)
  {
    fprintf (outfile, "  DC number:              %4d\n", dc_num);
    dum = dc_fanin_req;
    if (dum < dc_fanin_ack) dum = dc_fanin_ack;
    fprintf (outfile, "  DC max fanin:           %4d\n", dum);
    dum = dc_fanout_req;
    if (dum < dc_fanout_ack) dum = dc_fanout_ack;
    fprintf (outfile, "  DC max fanout:          %4d\n", dum);
    fprintf (outfile, "  DC p-transistor number: %4d\n", dc_trans_p_num);
    fprintf (outfile, "  DC n-transistor number: %4d\n", dc_trans_n_num);
    fprintf (outfile, "  ----------------------------\n");
    fprintf (outfile, "  FF number:              %4d\n", ff_num);
    dum = ff_fanin_set;
    if (dum < ff_fanin_reset) dum = ff_fanin_reset;
    fprintf (outfile, "  FF max fanin:           %4d\n", dum);
    dum = ff_fanout_set;
    if (dum < ff_fanout_reset) dum = ff_fanout_reset;
    fprintf (outfile, "  FF max fanout:          %4d\n", dum);
    fprintf (outfile, "  FF p-transistor number: %4d\n", ff_trans_p_num);
    fprintf (outfile, "  FF n-transistor number: %4d\n", ff_trans_n_num);
    fprintf (outfile, "  ============================\n");
    fprintf (outfile, "  element number:         %4d\n", dc_num + ff_num);
    dum = fanin_req_set;
    if (dum < fanin_ack_reset) dum = fanin_ack_reset;
    fprintf (outfile, "  max fanin:              %4d\n", dum);
    dum = fanout_req_set;
    if (dum < fanout_ack_reset) dum = fanout_ack_reset;
    fprintf (outfile, "  max fanout:             %4d\n", dum);
    fprintf (outfile, "  transistor number:      %4d\n", dc_trans_p_num + ff_trans_p_num + dc_trans_n_num + ff_trans_n_num);
  }
  else
  {
    fprintf (outfile, "  ====================\n");
    fprintf (outfile, "  %*d %-*s %*d %*d %*d %*d%*d+%*d+\n", name_width, dc_num + ff_num, comment_width, "", num_width, dc_trans_p_num + ff_trans_p_num, num_width, dc_trans_n_num + ff_trans_n_num, num_width, fanin_req_set, num_width, fanin_ack_reset, num_width, fanout_req_set, num_width, fanout_ack_reset);
  }
  fprintf (outfile, "*/\n");

  Verbose (DEBUG_STACK, "<-- omv_write_info");
  return 1;
}


/* === */
int omv_verilog_write (FILE *outfile, stg_t *stg, omv_t *omv, int flag)
{
  Verbose (DEBUG_STACK, "--> omv_write_verilog");
  PNode node_dc        = NULL;
  PNode node_ff        = NULL;
  PNode node_tst       = NULL;
  PNode node_dum       = NULL;
  PNode node_in        = NULL;
  int index_in         = 0;
  char *indent_str     = NULL;
  bool comma           = false;

  indent_change (indent_str, 0);

  Verbose (DEBUG_PROC, "VERILOG WRITE PROTOTYPES");
  fprintf (outfile, "%s%s\n", indent_str, COMMENT_PROTOTYPES);
  fprintf (outfile, "%s\n", indent_str);

  //IN prototype
  if ((flag & OMV_FF_INPUT) != OMV_FF_INPUT)
    fprintf (outfile, "%s\n", PROTOTYPE_INVERTOR);

  // DC prototypes
  node_dc = omv->dc;
  while (node_dc != NULL)
  {
    node_dum = omv->dc;
    while (strcmp (node_dum->data->comment, node_dc->data->comment) != 0)
      node_dum = node_dum->next;
    if (node_dum == node_dc)
    {
      fprintf (outfile, "%s%s %s (", indent_str, WORD_MODULE_BEGIN, node_dc->data->comment);
      index_in = 0;
      node_in = node_dc->data->pred;
      while (node_in != NULL)
      {
        index_in++;
        fprintf (outfile, "%s%d, ", NAME_REQ_IN, index_in);
        node_in = node_in->next;
      }
 
      index_in = 0;
      node_in = node_dc->data->succ;
      while (node_in != NULL)
      {
        index_in++;
        fprintf (outfile, "%s%d, ", NAME_ACK_IN, index_in);
        node_in = node_in->next;
      }
 
      fprintf (outfile, "%s, %s);\n", NAME_REQ_OUT, NAME_ACK_OUT);
      indent_change(indent_str, +1);
    
      index_in = 0;
      node_in = node_dc->data->pred;
      while (node_in != NULL)
      {
        index_in++;
        fprintf (outfile, "%s%s %s%d;\n", indent_str, WORD_INPUT, NAME_REQ_IN, index_in);
        node_in = node_in->next;
      }

      index_in = 0;
      node_in = node_dc->data->succ;
      while (node_in != NULL)
      {
        index_in++;
        fprintf (outfile, "%s%s %s%d;\n", indent_str, WORD_INPUT, NAME_ACK_IN, index_in);
        node_in = node_in->next;
      }

      fprintf (outfile, "%s%s %s;\n", indent_str, WORD_OUTPUT, NAME_REQ_OUT);
      fprintf (outfile, "%s%s %s;\n", indent_str, WORD_OUTPUT, NAME_ACK_OUT);
    
      indent_change(indent_str, -1);
      fprintf (outfile, "%s%s\n", indent_str, WORD_MODULE_END);
      fprintf (outfile, "%s\n", indent_str);
    }
    node_dc = node_dc->next;
  }

  // FF prototypes
  node_ff = omv->ff;
  while (node_ff != NULL)
  {
    if ((node_ff->data->type & OMV_INPUT) != OMV_INPUT)    
    {
      node_dum = omv->ff;
      while (strcmp (node_dum->data->comment, node_ff->data->comment) != 0)
        node_dum = node_dum->next;
      if (node_dum == node_ff)
      {
        fprintf (outfile, "%s%s %s (", indent_str, WORD_MODULE_BEGIN, node_ff->data->comment);
        index_in = 0;
        node_in = node_ff->data->pred;
        while (node_in != NULL)
        {
          index_in++;
          fprintf (outfile, "%s%d, ", NAME_SET, index_in);
          node_in = node_in->next;
        }

        index_in = 0;
        node_in = node_ff->data->succ;
        while (node_in != NULL)
        {
          index_in++;
          fprintf (outfile, "%s%d, ", NAME_RESET, index_in);
          node_in = node_in->next;
        }
    
        fprintf (outfile, "%s, %s);\n", NAME_OUT_1, NAME_OUT_0);
        indent_change(indent_str, +1);
    
        index_in = 0;
        node_in = node_ff->data->pred;
        while (node_in != NULL)
        {
          index_in++;
          fprintf (outfile, "%s%s %s%d;\n", indent_str, WORD_INPUT, NAME_SET, index_in);
          node_in = node_in->next;
        }

        index_in = 0;
        node_in = node_ff->data->succ;
        while (node_in != NULL)
        {
          index_in++;
          fprintf (outfile, "%s%s %s%d;\n", indent_str, WORD_INPUT, NAME_RESET, index_in);
          node_in = node_in->next;
        }

        fprintf (outfile, "%s%s %s;\n", indent_str, WORD_OUTPUT, NAME_OUT_1);
        fprintf (outfile, "%s%s %s;\n", indent_str, WORD_OUTPUT, NAME_OUT_0);

        indent_change(indent_str, -1);
        fprintf (outfile, "%s%s\n", indent_str, WORD_MODULE_END);
        fprintf (outfile, "%s\n", indent_str);
      }
    }  
    node_ff = node_ff->next;
  }
  
  Verbose (DEBUG_PROC, "VERILOG WRITE MAIN");
  fprintf (outfile, "%s%s\n", indent_str, COMMENT_MAIN);
  fprintf (outfile, "%s\n", indent_str);
  fprintf (outfile, "%s%s %s (", indent_str, WORD_MODULE_BEGIN, omv->model_name);

  comma = false;

  // Main test inputs declaration
  if ((flag & OMV_TEST) == OMV_TEST)
  {
    node_tst = omv->tst;
    while (node_tst != NULL)
    {
      if (comma) fprintf (outfile, ", ");
      else comma = true;
      fprintf (outfile, node_tst->data->name);
      node_tst = node_tst->next;
    }
  }

  // Main inputs declaration
  node_ff = omv->ff;
  while (node_ff != NULL)
  {
    if ((node_ff->data->type & OMV_INPUT) == OMV_INPUT)
    {
      if (comma) fprintf (outfile, ", ");
      else comma = true;
      if ((flag & OMV_FF_INPUT) == OMV_FF_INPUT)
        fprintf (outfile, PATTERN_NAME_LOW ", ", node_ff->data->name);
      fprintf (outfile, PATTERN_NAME_HIGH, node_ff->data->name);
    }
    node_ff = node_ff->next;
  }

  // Main outputs declaration
  node_ff = omv->ff;
  while (node_ff != NULL)
  {
    if ((node_ff->data->type & OMV_INPUT) != OMV_INPUT)
    {
      if (comma) fprintf (outfile, ", ");
      else comma = true;
      if ((flag & OMV_FF_INPUT) == OMV_FF_INPUT)
        fprintf (outfile, PATTERN_NAME_LOW ", ", node_ff->data->name);
      fprintf (outfile, PATTERN_NAME_HIGH, node_ff->data->name);
    }
    node_ff = node_ff->next;
  }
  fprintf (outfile, ");\n");
  indent_change(indent_str, +1);

  // Main test inputs declaration
  if ((flag & OMV_TEST) == OMV_TEST)
  {
    node_tst = omv->tst;
    while (node_tst != NULL)
    {
      fprintf (outfile, "%s%s %s;\n", indent_str, WORD_INPUT, node_tst->data->name);
      node_tst = node_tst->next;
    }
  }

  // Main inputs definition
  node_ff = omv->ff;
  while (node_ff != NULL)
  {
    if ((node_ff->data->type & OMV_INPUT) == OMV_INPUT)
    {
      if ((flag & OMV_FF_INPUT) == OMV_FF_INPUT)
        fprintf (outfile, "%s%s " PATTERN_NAME_LOW ";\n", indent_str, WORD_INPUT, node_ff->data->name);
      fprintf (outfile, "%s%s " PATTERN_NAME_HIGH ";\n", indent_str, WORD_INPUT, node_ff->data->name);
    }
    node_ff = node_ff->next;
  }

  // Main outputs definition
  node_ff = omv->ff;
  while (node_ff != NULL)
  {
    if ((node_ff->data->type & OMV_INPUT) != OMV_INPUT)
    {
      if ((flag & OMV_FF_INPUT) == OMV_FF_INPUT)
        fprintf (outfile, "%s%s " PATTERN_NAME_LOW ";\n", indent_str, WORD_OUTPUT, node_ff->data->name);
      fprintf (outfile, "%s%s " PATTERN_NAME_HIGH ";\n", indent_str, WORD_OUTPUT, node_ff->data->name);
    }
    node_ff = node_ff->next;
  }

  // Main wires definition
  if ((flag & OMV_FF_INPUT) != OMV_FF_INPUT)
  {
    node_ff = omv->ff;
    while (node_ff != NULL)
    {
      fprintf (outfile, "%s%s " PATTERN_NAME_LOW ";\n", indent_str, WORD_WIRE, node_ff->data->name);
      node_ff = node_ff->next;
    }
  }
  node_dc = omv->dc;
  while (node_dc != NULL)
  {
    fprintf (outfile, "%s%s %s_%s;\n", indent_str, WORD_WIRE, node_dc->data->name, NAME_REQ_WIRE);
    fprintf (outfile, "%s%s %s_%s;\n", indent_str, WORD_WIRE, node_dc->data->name, NAME_ACK_WIRE);
    node_dc = node_dc->next;
  }
  fprintf (outfile, "%s\n", indent_str);

  // Invertors structure
  if ((flag & OMV_FF_INPUT) != OMV_FF_INPUT)
  {
    node_ff = omv->ff;
    while (node_ff != NULL)
    {
      if ((node_ff->data->type & OMV_INPUT) == OMV_INPUT)
        fprintf (outfile, "%s%s " PATTERN_INVERTOR " (" PATTERN_NAME_HIGH ", " PATTERN_NAME_LOW ");\n", indent_str, ELEMENT_INVERTOR, node_ff->data->name, node_ff->data->name, node_ff->data->name);
      node_ff = node_ff->next;
    }
  }
  fprintf (outfile, "%s\n", indent_str);


  // Main DCs structure
  node_dc = omv->dc;
  while (node_dc != NULL)
  {
    fprintf (outfile, "%s%s %s (", indent_str, node_dc->data->comment, node_dc->data->name);
    node_in = node_dc->data->pred;
    while (node_in != NULL)
    {
      if (NodeLocate(omv->tst, node_in->data) == NULL)
        if (stg_cluster_type (stg, node_in->data->index_1) == NULL)
          fprintf (outfile, PATTERN_WIRE ", ", node_in->data->name, NAME_REQ_WIRE);
        else
          fprintf (outfile, "%s, ", node_in->data->name);
        else
        fprintf (outfile, "%s, ", node_in->data->name);
      node_in = node_in->next;
    }
    node_in = node_dc->data->succ;
    while (node_in != NULL)
    {
      if (NodeLocate(omv->tst, node_in->data) == NULL)
        if (stg_cluster_type (stg, node_in->data->index_1) == NULL)
          fprintf (outfile, PATTERN_WIRE ", ", node_in->data->name, NAME_ACK_WIRE);
        else
          fprintf (outfile, "%s, ", node_in->data->name);
      else
        fprintf (outfile, "%s, ", node_in->data->name);
      node_in = node_in->next;
    }
    fprintf (outfile, PATTERN_WIRE ", " PATTERN_WIRE ");\n", node_dc->data->name, NAME_REQ_WIRE, node_dc->data->name, NAME_ACK_WIRE);
    node_dc = node_dc->next;
  }
  fprintf (outfile, "%s\n", indent_str);

  // Main FFs structure
  node_ff = omv->ff;
  while (node_ff != NULL)
  {
    if ((node_ff->data->type & OMV_INPUT) != OMV_INPUT)
    {
      fprintf (outfile, "%s%s %s (", indent_str, node_ff->data->comment, node_ff->data->name);
      node_in = node_ff->data->pred;
      while (node_in != NULL)
      {
        if (stg_cluster_type (stg, node_in->data->index_1) == NULL)
          fprintf (outfile, PATTERN_WIRE ", ", node_in->data->name, NAME_ACK_WIRE);
        else
          fprintf (outfile, "%s, ", stg_cnv_invert (node_in->data->name));
        node_in = node_in->next;
      }
      node_in = node_ff->data->succ;
      while (node_in != NULL)
      {
        if (stg_cluster_type (stg, node_in->data->index_1) == NULL)
          fprintf (outfile, PATTERN_WIRE ", ", node_in->data->name, NAME_REQ_WIRE);
        else
          fprintf (outfile, "%s, ", node_in->data->name);
        node_in = node_in->next;
      }
      fprintf (outfile, PATTERN_NAME_HIGH ", " PATTERN_NAME_LOW ");\n", node_ff->data->name, node_ff->data->name);
    }
    node_ff = node_ff->next;
  }
  fprintf (outfile, "%s\n", indent_str);

  indent_change(indent_str, -1);
  fprintf (outfile, "%s%s\n", indent_str, WORD_MODULE_END);


  free (indent_str);
  Verbose (DEBUG_STACK, "<-- omv_write_verilog");
  return 1;
}


/* === */
int omv_debug (FILE *outfile, omv_t *omv, int flag)
{
  Verbose (DEBUG_STACK, "--> omv_debug");
  PNode node        = NULL;
  int width_name    = 8;
  int width_comment = 12;
  
  if ((flag & OMV_DC) == OMV_DC)
  {
    fprintf (outfile, "DSs:\n");
    node = omv->dc;
    while (node != NULL)
    {
      fprintf (outfile, "  & = %p  data = %p  .type = %#.8X  .name = %-*s  .comment = %-*s  .pred = %p  .succ = %p  .link = %p\n", node, node->data, node->data->type, width_name, node->data->name, width_comment, node->data->comment, node->data->pred, node->data->succ, node->data->link);
      node = node->next;
    }
  }
  
  if ((flag & OMV_FF) == OMV_FF)
  {
    fprintf (outfile, "FFs:\n");
    node = omv->ff;
    while (node != NULL)
    {
      fprintf (outfile, "  & = %p  data = %p  .type = %#.8X  .name = %-*s  .comment = %-*s  .pred = %p  .succ = %p  .link = %p\n", node, node->data, node->data->type, width_name, node->data->name, width_comment, node->data->comment, node->data->pred, node->data->succ, node->data->link);
      node = node->next;
    }
  }
  Verbose (DEBUG_STACK, "<-- omv_debug");
  return 1;
}
