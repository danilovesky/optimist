#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "debug.h"
#include "list.h"
#include "stg.h"
#include "stg_procs.h"
#include "ome_procs.h"

/* === */
int ome_output_exposition (stg_t *stg)
{
  Verbose (DEBUG_STACK, "--> ome_output_exposition");
  PNode node             = NULL;  // first level loop
  PNode node_            = NULL;  // second level loop
  PNode node__           = NULL;  // third level loop
  PNode node_dum_plus    = NULL;  // dummy transition +
  PNode node_dum_minus   = NULL;  // dummy transition -
  PNode node_low         = NULL;  // output exposition SIGNAL=0 place
  PNode node_high        = NULL;  // output exposition SIGNAL=1 place
  PNode node_minus       = NULL;  // output exposition SIGNAL- transition
  PNode node_plus        = NULL;  // output exposition SIGNAL+ transition
  PNode node_arc         = NULL;  // arc
  PNode node_mark        = NULL;  // marking
  char *s                = NULL;
  PData mask             = NULL;
  char *buf_name         = NULL;
  char *buf_comment      = NULL;

  bool b =false;

  mask = DataCreate (STG_NATIVE, NULL, NULL, NULL, NULL);
  buf_name = (char *) malloc (STG_BUF_MAX);
  buf_comment = (char *) malloc (STG_BUF_MAX);

  node = stg->s;
  while (node != NULL)
  {
    Verbose (DEBUG_PROC, "TRANSITION FOUND (type =%#.8X  name = %s  comment = %s)", node->data->type, node->data->name, node->data->comment);
    //--- if signal is INPUT or OUTPUT ...
    if ((node->data->type & STG_SIGNAL_INPUT) == STG_SIGNAL_INPUT ||
        (node->data->type & STG_SIGNAL_OUTPUT) == STG_SIGNAL_OUTPUT ||
        (node->data->type & STG_SIGNAL_INTERNAL) == STG_SIGNAL_INTERNAL)
    {
      Verbose (DEBUG_PROC, "TRANSITION (INPUT|OUTPUT) FOUND (type =%#.8X  name = %s  comment = %s)", node->data->type, node->data->name, node->data->comment);
      Verbose (DEBUG_PROC, "  CREATE DUMMIES");
      //--- create DUMMY+
      sprintf (buf_name, PATTERN_DUMMY_PLUS, node->data->name);
      node_dum_plus = NodeCreate (DataCreate (STG_SIGNAL_DUMMY, buf_name, NULL));
      NodeAdd (stg->s, node_dum_plus);
      Verbose (DEBUG_PROC, "    DUMMY_PLUS CREATED (type =%#.8X  name = %s  comment = %s)", node_dum_plus->data->type, node_dum_plus->data->name, node_dum_plus->data->comment);
      //--- create DUMMY-
      sprintf (buf_name, PATTERN_DUMMY_MINUS, node->data->name);
      node_dum_minus = NodeCreate (DataCreate (STG_SIGNAL_DUMMY, buf_name, NULL));
      NodeAdd (stg->s, node_dum_minus);
      Verbose (DEBUG_PROC, "    DUMMY_MINUS CREATED (type =%#.8X  name = %s  comment = %s)", node_dum_minus->data->type, node_dum_minus->data->name, node_dum_minus->data->comment);

      //--- change all corresponding TRANSITIONS to DUMMY
      Verbose (DEBUG_PROC, "  CHANGE TRANSITIONS TO DUMMIES");
      node_ = stg->t;
      while (node_ != NULL)
      {
        if (node_->data->link != NULL)
          if (node->data == node_->data->link->data)
          {
            s = stg_cnv_dummy (node_->data->name);
            free (node_->data->name);
            node_->data->name = s;
            node_->data->type = STG_TRANSITION_DUMMY;
            Verbose (DEBUG_PROC, "    TRANSITION TO DUMMY CHANGED (type =%#.8X  name = %s  comment = %s)", node_->data->type, node_->data->name, node_->data->comment);
          }
        node_ = node_->next;
      }

      Verbose (DEBUG_PROC, "  CREATE OUTPUT EXPOSITION");
    //=== create OUTPUT EXPOSITION (begin)
      //--- create SIGNAL=0
      sprintf (buf_name, PATTERN_NAME_LOW, node->data->name);
      sprintf (buf_comment, PATTERN_COMMENT_LOW, node->data->name);
      node_low = NodeCreate (DataCreate (STG_PLACE | STG_GENERATED | STG_MANDATORY, buf_name, buf_comment));
      NodeAdd (stg->p, node_low);
      Verbose (DEBUG_PROC, "    LOW LEVEL PLACE CREATED (type =%#.8X  name = %s  comment = %s)", node_low->data->type, node_low->data->name, node_low->data->comment);
      //--- create SIGNAL=1
      sprintf (buf_name, PATTERN_NAME_HIGH, node->data->name);
      sprintf (buf_comment, PATTERN_COMMENT_HIGH, node->data->name);
      node_high = NodeCreate (DataCreate (STG_PLACE | STG_GENERATED | STG_MANDATORY, buf_name, buf_comment));
      NodeAdd (stg->p, node_high);
      Verbose (DEBUG_PROC, "    HIGH LEVEL PLACE CREATED (type =%#.8X  name = %s  comment = %s)", node_high->data->type, node_high->data->name, node_high->data->comment);
    //+++ create OUTPUT EXPOSITION (end)

      Verbose (DEBUG_PROC, "  CREATE READING ARCS");
    //=== create READING ARCS (begin)
      node_ = stg->t;
      while (node_ != NULL)
      {
        if (strcmp (node_dum_minus->data->name, stg_cnv_signal (node_->data->name)) == 0)
        {
          //--- create SIGNAL-
          s = stg_cnv_name (node->data->name, SIGN_NEGATIVE, node->data->index_0++);
          node_minus = NodeCreate (DataCreate (STG_TRANSITION | STG_GENERATED | node->data->type ^ STG_SIGNAL, s, NULL));
          NodeAdd (stg->t, node_minus);
          Verbose (DEBUG_PROC, "    MINUS TRANSITION CREATED (type =%#.8X  name = %s  comment = %s)", node_minus->data->type, node_minus->data->name, node_minus->data->comment);
          //--- create ARC [SIGNAL=1 -> SIGNAL-]
          sprintf (buf_name, PATTERN_ARC_NAME, stg->a_num++);
          sprintf (buf_comment, PATTERN_ARC_COMMENT, node_high->data->name, node_minus->data->name);
          node_arc = NodeCreate (DataCreate (STG_ARC_CONSUMING | STG_GENERATED, buf_name, buf_comment, node_high, node_minus));
          NodeAdd (stg->a, node_arc);
          Verbose (DEBUG_PROC, "    READING ARC CREATED (type =%#.8X  name = %s  comment = %s)", node_arc->data->type, node_arc->data->name, node_arc->data->comment);
          //--- create ARC [SIGNAL- -> SIGNAL=0]
          sprintf (buf_name, PATTERN_ARC_NAME, stg->a_num++);
          sprintf (buf_comment, PATTERN_ARC_COMMENT, node_minus->data->name, node_low->data->name);
          node_arc = NodeCreate (DataCreate (STG_ARC_CONSUMING | STG_GENERATED, buf_name, buf_comment, node_minus, node_low));
          NodeAdd (stg->a, node_arc);
          Verbose (DEBUG_PROC, "    READING ARC CREATED (type =%#.8X  name = %s  comment = %s)", node_arc->data->type, node_arc->data->name, node_arc->data->comment);
          //--- create READING ARC [DUM- -- SIGNAL=0]
          sprintf (buf_name, PATTERN_ARC_NAME, stg->a_num++);
          sprintf (buf_comment, PATTERN_ARC_COMMENT, node_low->data->name, node_->data->name);
          node_arc = NodeCreate (DataCreate (STG_ARC_READING | STG_GENERATED, buf_name, buf_comment, node_low, node_));
          NodeAdd (stg->a, node_arc);
          Verbose (DEBUG_PROC, "    READING ARC CREATED (type =%#.8X  name = %s  comment = %s)", node_arc->data->type, node_arc->data->name, node_arc->data->comment);
          //--- create READING ARCS [<DUM- -- SIGNAL-]
          node__ = stg->a;
          while ((node__ = NodeFind (node__, DataFill (mask, STG_ARC_CONSUMING, NULL, NULL, NULL, node_))) != NULL)
          {
            sprintf (buf_name, PATTERN_ARC_NAME, stg->a_num++);
            sprintf (buf_comment, PATTERN_ARC_COMMENT, node__->data->pred->data->name, node_minus->data->name);
            node_arc = NodeCreate (DataCreate (STG_ARC_READING | STG_GENERATED, buf_name, buf_comment, node__->data->pred, node_minus));
            NodeAdd (stg->a, node_arc);
            Verbose (DEBUG_PROC, "      READING ARC CREATED (type =%#.8X  name = %s  comment = %s)", node_arc->data->type, node_arc->data->name, node_arc->data->comment);
            node__ = node__->next;
          }
        }
        else
        if (strcmp (node_dum_plus->data->name, stg_cnv_signal (node_->data->name)) == 0)
        {
          //--- create SIGNAL+
          s = stg_cnv_name (node->data->name, SIGN_POSITIVE, node->data->index_1++);
          node_plus = NodeCreate (DataCreate (STG_TRANSITION | STG_GENERATED | (node->data->type ^ STG_SIGNAL), s, NULL));
          NodeAdd (stg->t, node_plus);
          Verbose (DEBUG_PROC, "    PLUS TRANSITION CREATED (type =%#.8X  name = %s  comment = %s)", node_plus->data->type, node_plus->data->name, node_plus->data->comment);
          //--- create ARC [SIGNAL=0 -> SIGNAL+]
          sprintf (buf_name, PATTERN_ARC_NAME, stg->a_num++);
          sprintf (buf_comment, PATTERN_ARC_COMMENT, node_low->data->name, node_plus->data->name);
          node_arc = NodeCreate (DataCreate (STG_ARC_CONSUMING | STG_GENERATED, buf_name, buf_comment, node_low, node_plus));
          NodeAdd (stg->a, node_arc);
          Verbose (DEBUG_PROC, "    READING ARC CREATED (type =%#.8X  name = %s  comment = %s)", node_arc->data->type, node_arc->data->name, node_arc->data->comment);
          //--- create ARC [SIGNAL+ -> SIGNAL=1]
          sprintf (buf_name, PATTERN_ARC_NAME, stg->a_num++);
          sprintf (buf_comment, PATTERN_ARC_COMMENT, node_plus->data->name, node_high->data->name);
          node_arc = NodeCreate (DataCreate (STG_ARC_CONSUMING | STG_GENERATED, buf_name, buf_comment, node_plus, node_high));
          NodeAdd (stg->a, node_arc);
          Verbose (DEBUG_PROC, "    READING ARC CREATED (type =%#.8X  name = %s  comment = %s)", node_arc->data->type, node_arc->data->name, node_arc->data->comment);
          //--- create READING ARC [DUM+ -- SIGNAL=1]
          sprintf (buf_name, PATTERN_ARC_NAME, stg->a_num++);
          sprintf (buf_comment, PATTERN_ARC_COMMENT, node_high->data->name, node_->data->name);
          node_arc = NodeCreate (DataCreate (STG_ARC_READING | STG_GENERATED, buf_name, buf_comment, node_high, node_));
          NodeAdd (stg->a, node_arc);
          Verbose (DEBUG_PROC, "    READING ARC CREATED (type =%#.8X  name = %s  comment = %s)", node_arc->data->type, node_arc->data->name, node_arc->data->comment);
          //--- create READING ARCS [<DUM+ -- SIGNAL+]
          node__ = stg->a;
          while ((node__ = NodeFind (node__, DataFill (mask, STG_ARC_CONSUMING, NULL, NULL, NULL, node_))) != NULL)
          {
            sprintf (buf_name, PATTERN_ARC_NAME, stg->a_num++);
            sprintf (buf_comment, PATTERN_ARC_COMMENT, node__->data->pred->data->name, node_plus->data->name);
            node_arc = NodeCreate (DataCreate (STG_ARC_READING | STG_GENERATED, buf_name, buf_comment, node__->data->pred, node_plus));
            NodeAdd (stg->a, node_arc);
            Verbose (DEBUG_PROC, "      READING ARC CREATED (type =%#.8X  name = %s  comment = %s)", node_arc->data->type, node_arc->data->name, node_arc->data->comment);
            node__ = node__->next;
          }
        }
        node_ = node_->next;
      }
    //+++ create READING ARCS (end)
     

      Verbose (DEBUG_PROC, "  CREATE MARKING");
      //--- create MARKING
      if (node->data->link != NULL)
      {
        if ((node->data->link->data->type & STG_INITIAL_LOW) == STG_INITIAL_LOW)
        {
          sprintf (buf_name, PATTERN_MARKING_NAME, stg->m_num++);
          node_mark = NodeCreate (DataCreate (STG_MARKING | STG_GENERATED, buf_name, node_low->data->name, NULL, NULL, node_low));
          NodeAdd (stg->m, node_mark);
          Verbose (DEBUG_PROC, "    MARKING CREATED (type =%#.8X  name = %s  comment = %s)", node_mark->data->type, node_mark->data->name, node_mark->data->comment);
        }
        else
        if ((node->data->link->data->type & STG_INITIAL_HIGH) == STG_INITIAL_HIGH)
        {
          sprintf (buf_name, PATTERN_MARKING_NAME, stg->m_num++);
          node_mark = NodeCreate (DataCreate (STG_MARKING | STG_GENERATED, buf_name, node_high->data->name, NULL, NULL, node_high));
          NodeAdd (stg->m, node_mark);
          Verbose (DEBUG_PROC, "    MARKING CREATED (type =%#.8X  name = %s  comment = %s)", node_mark->data->type, node_mark->data->name, node_mark->data->comment);
        }
      }
    }
    node = node->next;
  }

  // choice mutual exclusion
  stg_fill (stg);
  stg_names_compress (stg);
  Verbose (DEBUG_PROC, "CREATE CHOICE MUTUAL EXCLUSION READING ARCS");
  node = stg->p;
  while (node != NULL)
  {
    PNode node_trans = node->data->succ;
    if (node_trans != NULL)
    // choice
    if (node_trans->next != NULL)
    {
      // free choice
      if (node_trans->data->pred->next == NULL)
      {
        if ((node_trans->data->type & STG_TRANSITION_DUMMY) == STG_TRANSITION_DUMMY)
        {
          Verbose (DEBUG_PROC, "  FREE CHOICE DETECTED (type = %#.8X  name = %s  comment = %s  pred = %p  succ = %p  link = %p)", node->data->type, node->data->name, node->data->comment, node->data->pred, node->data->succ, node->data->link);
          node_ = stg->a;
          while ((node_ = NodeFind (node_, DataFill (mask, STG_ARC_READING, NULL, NULL, node, NULL))) != NULL)
          {
            // exposed place that is marked before choice
            node_mark = NodeLocate (stg->p, node_->data->succ->data->pred->data);
            Verbose (DEBUG_PROC, "    EXCLUSION PLACE  (type = %#.8X  name = %s  comment = %s  pred = %p  succ = %p  link = %p)", node_mark->data->type, node_mark->data->name, node_mark->data->comment, node_mark->data->pred, node_mark->data->succ, node_mark->data->link);
            node__ = stg->a;
            while ((node__ = NodeFind (node__, DataFill (mask, STG_ARC_READING, NULL, NULL, node, NULL))) != NULL)
            {
              if (node__ != node_)
              {
                sprintf (buf_name, PATTERN_ARC_NAME, stg->a_num++);
                sprintf (buf_comment, PATTERN_ARC_COMMENT, node_mark->data->name, node__->data->succ->data->name);
                node_arc = NodeCreate (DataCreate (STG_ARC_READING | STG_GENERATED, buf_name, buf_comment, node_mark, node__->data->succ));
                NodeAdd (stg->a, node_arc);
                Verbose (DEBUG_PROC, "      READING ARC GENERATED (type =%#.8X  name = %s  comment = %s)", node_arc->data->type, node_arc->data->name, node_arc->data->comment);
              }
              node__ = node__->next;
            }
            node_ = node_->next;
          }
        }
      }
    }
    node = node->next;
  }

  free (buf_name);
  free (buf_comment);
  DataFree (mask);
  stg_fill (stg);
  stg_names_compress (stg);
  Verbose (DEBUG_STACK, "<-- ome_output_exposition");
  return 1;
}
