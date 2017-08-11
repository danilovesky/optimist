#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "debug.h"
#include "list.h"
#include "stg.h"
#include "stg_procs.h"
#include "omt_procs.h"


/* === -f1 option */
int omt_fanin (stg_t *stg)
{
  Verbose (DEBUG_STACK, "--> omt_fanin");
  PNode node_trans      = NULL;
  PNode node_place      = NULL;
  bool bl_mandatory_place = false;

  // for every transition check...
  node_trans = stg->t;
  while (node_trans != NULL)
  {
    bl_mandatory_place = false;
    // if at least one succ place is mandatory
    node_place = node_trans->data->succ;
    while (node_place != NULL)
    {
      if ((node_place->data->type & STG_REDUNDANT) != STG_REDUNDANT)
      {
        bl_mandatory_place = true;
	Message("place %s was mandatory", node_place->data->name);
        node_place = NULL;
      }
      else	
        node_place = node_place->next;
    }
    // then tag all succ places as mandatory
    if (bl_mandatory_place)
    {
      node_place = node_trans->data->succ;
      while (node_place != NULL)
      {
        node_place->data->type &= ~STG_REDUNDANT;
        node_place->data->type |= STG_MANDATORY;
	Message("place %s became mandatory", node_place->data->name);        
	node_place->data->name = stg_cnv_mandatory(node_place->data->name);
	node_place = node_place->next;
      }
    }
    node_trans = node_trans->next;
  }
  Verbose (DEBUG_STACK, "<-- omt_fanin");
  return 1;
}

/* === -r1 option */
int omt_marking (stg_t *stg)
{
  Verbose (DEBUG_STACK, "--> omt_marking");
  PNode node_place      = NULL;
  PNode node_place_cur  = NULL;
  PNode node_place_pred = NULL;
  PNode node_place_succ = NULL;
  PNode node_trans      = NULL;
  PNode node_trans_pred = NULL;
  PNode node_mark       = NULL;
  PData mask            = NULL;
  char *buf_name        = NULL;
  char *buf_comment     = NULL;
  int dif               = 0;
  int dif_max           = 0;
  bool ok               = false;

  mask = DataCreate (STG_NATIVE, NULL, NULL, NULL, NULL);
  buf_name = (char *) malloc (STG_BUF_MAX);
  buf_comment = (char *) malloc (STG_BUF_MAX);

  // for every redundant marked place
  node_place_cur = stg->p;
  while (node_place_cur != NULL)
  {
    Verbose (DEBUG_PROC, "PLACE FOUND (type =%#.8X  name = %s  comment = %s  link = %p)", node_place_cur->data->type, node_place_cur->data->name, node_place_cur->data->comment, node_place_cur->data->link);
    node_mark = node_place_cur->data->link;
    if ((node_place_cur->data->type & STG_REDUNDANT) == STG_REDUNDANT && node_mark != NULL)
    {
      Verbose (DEBUG_PROC, "REDUNDANT MARKED PLACE FOUND (type =%#.8X  name = %s  comment = %s)", node_place_cur->data->type, node_place_cur->data->name, node_place_cur->data->comment);
      // find OPTIMAL (max difference |trans->succ| - |trans->pred|) redundant marked place pred transition
      node_trans = NULL;
      node_trans_pred = node_place_cur->data->pred;
      while (node_trans_pred != NULL)
      {
        ok = true;
        node_place_succ = node_trans_pred->data->succ;
        while (ok && node_place_succ != NULL)
        {
          if (node_place_succ->data->link == NULL)
            ok = false;
          node_place_succ = node_place_succ->next;
        }
        if (ok)
        {
          dif = NodeCount (node_trans_pred->data->succ) - NodeCount (node_trans_pred->data->pred);
          if (dif > dif_max || node_trans == NULL)
          {
            dif_max = dif;
            node_trans = NodeLocate (stg->t, node_trans_pred->data);
          }
        }
        Verbose (DEBUG_PROC, "  REDUNDANT MARKED PLACE OPTIMAL PRED TRANSITION TESTED (type =%#.8X  name = %s  comment = %s)", node_trans_pred->data->type, node_trans_pred->data->name, node_trans_pred->data->comment);
        node_trans_pred = node_trans_pred->next;
      }

      // if it is possible then roll back marking
      if (node_trans != NULL)
      {
        Verbose (DEBUG_PROC, "  REDUNDANT MARKED PLACE OPTIMAL PRED TRANSITION FOUND (type =%#.8X  name = %s  comment = %s)", node_trans->data->type, node_trans->data->name, node_trans->data->comment);
        // delete every redundant place pred transition post place marking
        node_place_succ = node_trans->data->succ;
        while (node_place_succ != NULL)
        {
          node_mark = NodeLocate (stg->m, node_place_succ->data->link->data);
          if (node_mark != NULL)
          {
            Verbose (DEBUG_PROC, "    REDUNDANT MARKED PLACE OPTIMAL PRED TRANSITION SUCC PLACE MARKING DELETED (type =%#.8X  name = %s  comment = %s)", node_mark->data->type, node_mark->data->name, node_mark->data->comment);
            NodeDel (stg->m, node_mark, true);
          }
          node_place_succ = node_place_succ->next;
        }
        // for every not state separation marked place OPTIMAL pred transition pred place create marking (if it does not exist)
        node_place_pred = node_trans->data->pred;
        while (node_place_pred != NULL)
        {
          node_place = NodeLocate (stg->p, node_place_pred->data);
          if (node_place->data->link == NULL)
          {
            sprintf (buf_name, PATTERN_MARKING_NAME, stg->m_num++);
            node_mark = NodeCreate (DataCreate (STG_MARKING | STG_GENERATED, buf_name, node_place->data->name, NULL, NULL, node_place));
            NodeAdd (stg->m, node_mark);
            Verbose (DEBUG_PROC, "    REDUNDANT MARKED PLACE OPTIMAL PRED TRANSITION PRED PLACE MARKING CREATED (type =%#.8X  name = %s  comment = %s)", node_mark->data->type, node_mark->data->name, node_mark->data->comment);
          }
          node_place_pred = node_place_pred->next;
        }
        stg_fill (stg);
        node_place_cur = stg->p;
      }
      else
      {
        node_place_cur = node_place_cur->next;
      }
    }
    else
      node_place_cur = node_place_cur->next;
  }
  // if it is imposible to roll back the marking, then make all marked places mandatory
  node_place_cur = stg->p;
  while (node_place_cur != NULL)
  {
    node_mark = node_place_cur->data->link;
    if ((node_place_cur->data->type & STG_REDUNDANT) == STG_REDUNDANT && node_mark != NULL)
    {
      node_place_cur->data->type &= ~STG_REDUNDANT;
      node_place_cur->data->type |= STG_MANDATORY;
    }
    node_place_cur = node_place_cur->next;
  }

  free (buf_name);
  free (buf_comment);
  DataFree (mask);
  stg_names_compress (stg);

  // if there is any redundant place with markin, make it state separation
  node_place_cur = stg->p;
  while (node_place_cur != NULL)
  {
    if (node_place_cur->data->link != NULL)
    {
      node_place_cur->data->type &= ~STG_REDUNDANT;
      Verbose (DEBUG_PROC, "REDUNDANT MARKED PLACE FOUND AND CHANGED TO STATE SEPARATION (type =%#.8X  name = %s  comment = %s)", node_place_cur->data->type, node_place_cur->data->name, node_place_cur->data->comment);
    }
    node_place_cur = node_place_cur->next;
  }

  Verbose (DEBUG_STACK, "<-- omt_marking");
  return 1;
}


/* === -r2 option */
int omt_context (stg_t *stg)
{
  Verbose (DEBUG_STACK, "--> omt_context");
  PNode node_dum        = NULL;
  PNode node_dum_dup    = NULL;
  PNode node_arc        = NULL;
  PNode node_arc_dup    = NULL;
  PNode node_arc_r      = NULL;
  PNode node_place_cur  = NULL;
  PNode node_place_dup  = NULL;
  PNode node_place_pred = NULL;
  PNode node_trans      = NULL;
  PNode node_trans_dup  = NULL;
  PNode node_trans_pred = NULL;
  PNode node_trans_dum  = NULL;
  PData mask            = NULL;
  char *buf_name        = NULL;
  char *buf_comment     = NULL;
  char polarity         = SIGN_NO_CARE;
  bool bl_control          = false;
  bool bl_node_removed     = false;
  bool bl_trigger          = false;

  mask = DataCreate (STG_NATIVE, NULL, NULL, NULL, NULL);
  buf_name = (char *) malloc (STG_BUF_MAX);
  buf_comment = (char *) malloc (STG_BUF_MAX);
  
  node_place_cur = stg->p;
  while (node_place_cur != NULL)
  {
    node_place_cur->data->type &= ~STG_PROCESSED;
    node_place_cur = node_place_cur->next;
  }

  node_place_cur = stg->p;
  while (node_place_cur != NULL)
  {
  if ((node_place_cur->data->type & STG_PROCESSED) != STG_PROCESSED)
  {
    node_place_cur->data->type |= STG_PROCESSED;
    bl_node_removed = false;
    bl_control = false;
    // for every redundant-place
    if ((node_place_cur->data->type & STG_REDUNDANT) == STG_REDUNDANT)
    {
      Verbose (DEBUG_PROC, "REDUNDANT-PLACE FOUND (type =%#.8X  name = %s  comment = %s)", node_place_cur->data->type, node_place_cur->data->name, node_place_cur->data->comment);
      // for every redundant-place read-arc create a read-arc from each place which controls pred-transition by read-arc
      node_arc_r = stg->a;
      while ((node_arc_r = NodeFind (node_arc_r, DataFill (mask, STG_ARC_READING, NULL, NULL, node_place_cur, NULL, NULL))) != NULL)
      {
        Verbose (DEBUG_PROC, "  REDUNDANT-PLACE READ-ARC FOUND (type =%#.8X  name = %s  comment = %s)", node_arc_r->data->type, node_arc_r->data->name, node_arc_r->data->comment);
        bl_control = true;
        node_trans = node_arc_r->data->succ;
        // modification of elementary cycle
        node_trans_pred = node_place_cur->data->pred;
        while (node_trans_pred != NULL)
        {
          bl_trigger = false;
          Verbose (DEBUG_PROC, "    REDUNDANT-PLACE PRED-TRANSITION FOUND (type =%#.8X  name = %s  comment = %s)", node_trans_pred->data->type, node_trans_pred->data->name, node_trans_pred->data->comment);
          // duplicate transition and its arcs
          if ((polarity = stg_cnv_polarity (node_trans->data->name)) == SIGN_NEGATIVE)
            node_trans_dup = NodeCreate (DataCreate (node_trans->data->type | STG_GENERATED, stg_cnv_name (node_trans->data->link->data->name, polarity, node_trans->data->link->data->index_0++), NULL, NULL, NULL, node_trans->data->link));
          else
            node_trans_dup = NodeCreate (DataCreate (node_trans->data->type | STG_GENERATED, stg_cnv_name (node_trans->data->link->data->name, polarity, node_trans->data->link->data->index_1++), NULL, NULL, NULL, node_trans->data->link));
	    
          NodeAdd (stg->t, node_trans_dup);
          Verbose (DEBUG_PROC, "    REDUNDANT-PLACE CONTROLED OUTPUT-TRANSITION DUPLICATED (type =%#.8X  name = %s  comment = %s)", node_trans_dup->data->type, node_trans_dup->data->name, node_trans_dup->data->comment);
          // duplicate every transition consuming-arc
          node_arc = stg->a;
          while ((node_arc = NodeFind (node_arc, DataFill (mask, STG_ARC_CONSUMING, NULL, NULL, NULL, node_trans, NULL))) != NULL)
          {
            sprintf (buf_name, PATTERN_ARC_NAME, stg->a_num++);
            sprintf (buf_comment, PATTERN_ARC_COMMENT, node_arc->data->pred->data->name, node_trans_dup->data->name);
            node_arc_dup = NodeCreate (DataCreate (node_arc->data->type | STG_GENERATED, buf_name, buf_comment, node_arc->data->pred, node_trans_dup, NULL));
            NodeAdd (stg->a, node_arc_dup);
            Verbose (DEBUG_PROC, "      REDUNDANT-PLACE CONTROLED OUTPUT-TRANSITION CONSUMING-ARC DUPLICATED (type =%#.8X  name = %s  comment = %s)", node_arc_dup->data->type, node_arc_dup->data->name, node_arc_dup->data->comment);
            node_arc = node_arc->next;
          }
          // duplicate every transition producing-arc
          node_arc = stg->a;
          while ((node_arc = NodeFind (node_arc, DataFill (mask, STG_ARC_CONSUMING, NULL, NULL, node_trans, NULL, NULL))) != NULL)
          {
            sprintf (buf_name, PATTERN_ARC_NAME, stg->a_num++);
            sprintf (buf_comment, PATTERN_ARC_COMMENT, node_trans_dup->data->name, node_arc->data->succ->data->name);
            node_arc_dup = NodeCreate (DataCreate (node_arc->data->type | STG_GENERATED, buf_name, buf_comment, node_trans_dup, node_arc->data->succ, NULL));
            NodeAdd (stg->a, node_arc_dup);
            Verbose (DEBUG_PROC, "      REDUNDANT-PLACE CONTROLED OUTPUT-TRANSITION PRODUCING-ARC DUPLICATED (type =%#.8X  name = %s  comment = %s)", node_arc_dup->data->type, node_arc_dup->data->name, node_arc_dup->data->comment);
            node_arc = node_arc->next;
          }
          // duplicate every transition read-arc except the one we are going to delete
          node_arc = stg->a;
          while ((node_arc = NodeFind (node_arc, DataFill (mask, STG_ARC_READING, NULL, NULL, NULL, node_trans, NULL))) != NULL)
          {
            if (node_arc->data->pred->data != node_place_cur->data)
            {
              sprintf (buf_name, PATTERN_ARC_NAME, stg->a_num++);
              sprintf (buf_comment, PATTERN_ARC_COMMENT, node_arc->data->pred->data->name, node_trans_dup->data->name);
              node_arc_dup = NodeCreate (DataCreate (node_arc->data->type | STG_GENERATED, buf_name, buf_comment, node_arc->data->pred, node_trans_dup, NULL));
              NodeAdd (stg->a, node_arc_dup);
              Verbose (DEBUG_PROC, "      REDUNDANT-PLACE CONTROLLED OUTPUT-TRANSITION READ-ARC DUPLICATED (type =%#.8X  name = %s  comment = %s)", node_arc_dup->data->type, node_arc_dup->data->name, node_arc_dup->data->comment);
            }
            node_arc = node_arc->next;
          }
	  // if the output transition does not have a trigger signal yet, then add it
          if ((node_arc_r->data->type & STG_TRIGGER) != STG_TRIGGER)
          {
            // create read-arc from each redundant-place pred-transition control-place (by reading arc) to redundant-place controled elementry-cycle
            node_trans_dum = NodeLocate (stg->t, node_trans_pred->data);
            node_arc = stg->a;
            while ((node_arc = NodeFind (node_arc, DataFill (mask, STG_ARC_READING, NULL, NULL, NULL, node_trans_dum, NULL))) != NULL)
            {
              sprintf (buf_name, PATTERN_ARC_NAME, stg->a_num++);
              sprintf (buf_comment, PATTERN_ARC_COMMENT, node_arc->data->pred->data->name, node_trans_dup->data->name);
	      bl_trigger = true;
              node_arc_dup = NodeCreate (DataCreate (node_arc->data->type | STG_TRIGGER | STG_GENERATED, buf_name, buf_comment, node_arc->data->pred, node_trans_dup, NULL));
              NodeAdd (stg->a, node_arc_dup);
              Verbose (DEBUG_PROC, "      REDUNDANT-PLACE CONTROLLED OUTPUT-TRANSITION TRIGGER READ-ARC CREATEDED (type =%#.8X  name = %s  comment = %s)", node_arc_dup->data->type, node_arc_dup->data->name, node_arc_dup->data->comment);
              node_arc = node_arc->next;
            }
	  }
          else
          {
            Verbose (DEBUG_PROC, "      REDUNDANT-PLACE CONTROLLED OUTPUT-TRANSITION ALREADY HAS TRIGGER READ-ARC (type =%#.8X  name = %s  comment = %s)", node_arc_r->data->type, node_arc_r->data->name, node_arc_r->data->comment);
          }
	  
	  // MOVE READ ARCS FROM REDUNDANT PLACE TO THE PRED PLACES
	  if (NodeCount(node_trans_pred->data->succ) > 1)
	  {
  	    // duplicate redundant-place pred-trasition
            PNode node_trans_pred_dup = NodeCreate (DataCreate (node_trans_pred->data->type | STG_GENERATED, stg_cnv_name (node_trans_pred->data->link->data->name, stg_cnv_polarity (node_trans_pred->data->name), ++node_trans_pred->data->link->data->index_0), NULL, NULL, NULL, node_trans_pred->data->link));
            NodeAdd (stg->t, node_trans_pred_dup);
            Verbose (DEBUG_PROC, "  REDUNDANT-PLACE PRED-TRANSITION DUPLICATED (type =%#.8X  name = %s  comment = %s)", node_trans_pred_dup->data->type, node_trans_pred_dup->data->name, node_trans_pred_dup->data->comment);
            // duplicate each resundant-place pred-transition pred-place
            node_place_pred = node_trans_pred->data->pred;
            while (node_place_pred != NULL)
            {
  	      PNode node_place_pred_real = NodeLocate (stg->p, node_place_pred->data);
              PNode node_place_pred_dup = NodeCreate (DataCreate (node_place_pred_real->data->type | STG_GENERATED, node_place_pred_real->data->name, node_place_pred_real->data->comment, NULL, NULL, NULL));
              NodeAdd (stg->p, node_place_pred_dup);
              Verbose (DEBUG_PROC, "    REDUNDANT-PLACE PRED-TRANSITION PRED-PLACE DUPLICATED (type =%#.8X  name = %s  comment = %s)", node_place_pred_dup->data->type, node_place_pred_dup->data->name, node_place_pred_dup->data->comment);
              // duplicate every not-state-separation-place pred-transition pred-place marking (if it exists)
              PNode node_mark = node_place_pred->data->link;
	      if (node_mark)
              {
                sprintf (buf_name, PATTERN_MARKING_NAME, stg->m_num++);
                PNode node_mark_dup = NodeCreate (DataCreate (node_mark->data->type | STG_GENERATED, buf_name, node_place_pred_dup->data->name, NULL, NULL, node_place_pred_dup));
                NodeAdd (stg->m, node_mark_dup);
                Verbose (DEBUG_PROC, "      REDUNDANT-PLACE PRED-TRANSITION PRED-PLACE MARKING DUPLICATED (type =%#.8X  name = %s  comment = %s)", node_mark_dup->data->type, node_mark_dup->data->name, node_mark_dup->data->comment);
              }
  	      // duplicate every redundant-place pred-transition pred-place producing-arc
              node_arc = stg->a;
              while ((node_arc = NodeFind (node_arc, DataFill (mask, STG_ARC_CONSUMING, NULL, NULL, NULL, node_place_pred_real, NULL))) != NULL)
              {
                sprintf (buf_name, PATTERN_ARC_NAME, stg->a_num++);
                sprintf (buf_comment, PATTERN_ARC_COMMENT, node_arc->data->pred->data->name, node_place_pred_dup->data->name);
                node_arc_dup = NodeCreate (DataCreate (STG_ARC_CONSUMING | STG_GENERATED, buf_name, buf_comment, node_arc->data->pred, node_place_pred_dup, NULL));
                NodeAdd (stg->a, node_arc_dup);
                Verbose (DEBUG_PROC, "      REDUNDANT-PLACE PRED-TRANSITION PRED-PLACE PRODUCING-ARC DUPLICATED (type =%#.8X  name = %s  comment = %s)", node_arc_dup->data->type, node_arc_dup->data->name, node_arc_dup->data->comment);
                node_arc = node_arc->next;
	      }
	      // duplicate every redundant-place pred-transition pred-place consuming-arc
              // (except of redundant-place pred-transition pred-place to redundant-place pred-transition one)
              node_arc = stg->a;
              while ((node_arc = NodeFind (node_arc, DataFill (mask, STG_ARC_CONSUMING, NULL, NULL, node_place_pred_real, NULL, NULL))) != NULL)
              {
                if (node_arc->data->succ->data != node_trans_pred->data)
                {
                  sprintf (buf_name, PATTERN_ARC_NAME, stg->a_num++);
                  sprintf (buf_comment, PATTERN_ARC_COMMENT, node_place_pred_dup->data->name, node_arc->data->succ->data->name);
                  node_arc_dup = NodeCreate (DataCreate (STG_ARC_CONSUMING | STG_GENERATED, buf_name, buf_comment, node_place_pred_dup, node_arc->data->succ, NULL));
                  NodeAdd (stg->a, node_arc_dup);
                  Verbose (DEBUG_PROC, "      REDUNDANT-PLACE PRED-TRANSITION PRED-PLACE CONSUMING-ARC DUPLICATED (type =%#.8X  name = %s  comment = %s)", node_arc_dup->data->type, node_arc_dup->data->name, node_arc_dup->data->comment);
	        }
                node_arc = node_arc->next;
	      }
	      // crate a consuming arc from redundant-place pred-transition pred-place duplication to redundant-place pred-transition duplication
              sprintf (buf_name, PATTERN_ARC_NAME, stg->a_num++);
              sprintf (buf_comment, PATTERN_ARC_COMMENT, node_place_pred_dup->data->name, node_trans_pred_dup->data->name);
              node_arc_dup = NodeCreate (DataCreate (STG_ARC_CONSUMING | STG_GENERATED, buf_name, buf_comment, node_place_pred_dup, node_trans_pred_dup, NULL));
              NodeAdd (stg->a, node_arc_dup);
              Verbose (DEBUG_PROC, "    PLACE-DUP TO TRANSITION-DUP CONSUMING-ARC CREATED (type =%#.8X  name = %s  comment = %s)", node_arc_dup->data->type, node_arc_dup->data->name, node_arc_dup->data->comment);
              // create read-arc from redundant-place pred-transition pred-place duplication to redundant-place controled elementry-cycle
              sprintf (buf_name, PATTERN_ARC_NAME, stg->a_num++);
              sprintf (buf_comment, PATTERN_ARC_COMMENT, node_place_pred_dup->data->name, node_trans_dup->data->name);
	      if (bl_trigger)
                node_arc_dup = NodeCreate (DataCreate (STG_ARC_READING | STG_TRIGGER | STG_GENERATED, buf_name, buf_comment, node_place_pred_dup, node_trans_dup, NULL));
	      else
                node_arc_dup = NodeCreate (DataCreate (STG_ARC_READING | STG_GENERATED, buf_name, buf_comment, node_place_pred_dup, node_trans_dup, NULL));
              NodeAdd (stg->a, node_arc_dup);
              Verbose (DEBUG_PROC, "    REDUNDANT-PLACE CONTROLED OUTPUT-TRANSITION READ-ARC CREATED (type =%#.8X  name = %s  comment = %s)", node_arc_dup->data->type, node_arc_dup->data->name, node_arc_dup->data->comment);
 	      node_place_pred = node_place_pred->next;
            }
            // create producing-arc from redundant-place pred-transition duplication to redundant-place
            sprintf (buf_name, PATTERN_ARC_NAME, stg->a_num++);
            sprintf (buf_comment, PATTERN_ARC_COMMENT, node_trans_pred_dup->data->name, node_place_cur->data->name);
            node_arc_dup = NodeCreate (DataCreate (STG_ARC_CONSUMING  | STG_GENERATED, buf_name, buf_comment, node_trans_pred_dup, node_place_cur, NULL));
            NodeAdd (stg->a, node_arc_dup);
            Verbose (DEBUG_PROC, "  REDUNDANT-PLACE PRED-TRANSITION-DUP TO REDUNDANT-PLACE PRODUCING-ARC CREATED (type =%#.8X  name = %s  comment = %s)", node_arc_dup->data->type, node_arc_dup->data->name, node_arc_dup->data->comment);
	    // DELETE 
            // delete redundant-place pred-transtion to redundant-place producing arc
	    node_trans_dum = NodeLocate(stg->t, node_trans_pred->data);
	    node_arc = stg->a;
            if ((node_arc = NodeFind (node_arc, DataFill (mask, STG_ARC_CONSUMING, NULL, NULL, node_trans_dum, node_place_cur, NULL))) !=NULL)
            {
              Verbose (DEBUG_PROC, "  REDUNDANT-PLACE PRODUCING-ARC DELETED (type =%#.8X  name = %s  comment = %s)", node_arc->data->type, node_arc->data->name, node_arc->data->comment);
              NodeDel (stg->a, node_arc, true);
            }
	    bl_node_removed = true;
	  }
	  else
	  {
            // create read-arc from each redundant-place pred-transition pred-place to redundant-place controled elementry-cycle
            node_place_pred = node_trans_pred->data->pred;
            while (node_place_pred != NULL)
            {
	      PNode node_place = NodeLocate(stg->p, node_place_pred->data);
              sprintf (buf_name, PATTERN_ARC_NAME, stg->a_num++);
              sprintf (buf_comment, PATTERN_ARC_COMMENT, node_place->data->name, node_trans_dup->data->name);
	      if (bl_trigger)
                node_arc_dup = NodeCreate (DataCreate (STG_ARC_READING | STG_TRIGGER |  STG_GENERATED, buf_name, buf_comment, node_place, node_trans_dup, NULL));
	      else
                node_arc_dup = NodeCreate (DataCreate (STG_ARC_READING |  STG_GENERATED, buf_name, buf_comment, node_place, node_trans_dup, NULL));
              NodeAdd (stg->a, node_arc_dup);
              Verbose (DEBUG_PROC, "    REDUNDANT-PLACE CONTROLED OUTPUT-TRANSITION READ-ARC CREATED (type =%#.8X  name = %s  comment = %s)", node_arc_dup->data->type, node_arc_dup->data->name, node_arc_dup->data->comment);
 	      node_place_pred = node_place_pred->next;
	    }
          }
          //
	  node_trans_pred = node_trans_pred->next;
        }

        // DELETE OUTPUT TRANSITION AND ITS CONSUMING AND READ-ARCS
        // delete every output transition pred consuming arc
        node_arc = stg->a;
        while ((node_arc = NodeFind (node_arc, DataFill (mask, STG_ARC_CONSUMING, NULL, NULL, NULL, node_trans, NULL))) !=NULL)
        {
          node_arc_dup = node_arc->next;
          Verbose (DEBUG_PROC, "      REDUNDANT-PLACE CONTROLED OUTPUT-TRANSITION CONSUMING-ARC DELETED (type =%#.8X  name = %s  comment = %s)", node_arc->data->type, node_arc->data->name, node_arc->data->comment);
          NodeDel (stg->a, node_arc, true);
          node_arc = node_arc_dup;
        }
        // delete every output-transition producing-arc
        node_arc = stg->a;
        while ((node_arc = NodeFind (node_arc, DataFill (mask, STG_ARC_CONSUMING, NULL, NULL, node_trans, NULL, NULL))) !=NULL)
        {
          node_arc_dup = node_arc->next;
          Verbose (DEBUG_PROC, "      REDUNDANT-PLACE CONTROLED OUTPUT-TRANSITION PRODUCING-ARC DELETED (type =%#.8X  name = %s  comment = %s)", node_arc->data->type, node_arc->data->name, node_arc->data->comment);
          NodeDel (stg->a, node_arc, true);
          node_arc = node_arc_dup;
        }
        // delete every output-transition read-arc
        node_arc = stg->a;
        while ((node_arc = NodeFind (node_arc, DataFill (mask, STG_ARC_READING, NULL, NULL, NULL, node_trans, NULL))) !=NULL)
        {
          node_arc_dup = node_arc->next;
          Verbose (DEBUG_PROC, "      REDUNDANT-PLACE CONTROLED OUTPUT-TRANSITION READ-ARC DELETED (type =%#.8X  name = %s  comment = %s)", node_arc->data->type, node_arc->data->name, node_arc->data->comment);
          NodeDel (stg->a, node_arc, true);
          node_arc = node_arc_dup;
        }
        Verbose (DEBUG_PROC, "    REDUNDANT-PLACE CONTROLED OUTPUT-TRANSITION DELETED (type =%#.8X  name = %s  comment = %s)", node_trans->data->type, node_trans->data->name, node_trans->data->comment);
        NodeDel (stg->t, node_trans, true);

        node_arc_r = stg->a;
      }
      // if redundant-place does not control any transition, then create read-arcs from control-places of its pred-transitions to its succ-transitions
      if (!bl_control)
      {
        Verbose (DEBUG_PROC, "  REDUNDANT PLACE DOES NOT CONTROL ANY TRANSITION (type =%#.8X  name = %s  comment = %s)", node_place_cur->data->type, node_place_cur->data->name, node_place_cur->data->comment);
        node_trans_pred = node_place_cur->data->pred;
        while (node_trans_pred != NULL)
        {
          Verbose (DEBUG_PROC, "    DUPLICATE READ-ARCS TO REDUNDANT PLACE PRED TRANSITION (type =%#.8X  name = %s  comment = %s)", node_trans_pred->data->type, node_trans_pred->data->name, node_trans_pred->data->comment);
          // duplicate every redundant place pred transition read-arc
          node_arc = stg->a;
          while ((node_arc = NodeFind (node_arc, DataFill (mask, STG_ARC_READING, NULL, NULL, NULL, NodeLocate(stg->t, node_trans_pred->data), NULL))) != NULL)
          {
            node_trans_dum = node_place_cur->data->succ;
            while (node_trans_dum != NULL)
            {
              sprintf (buf_name, PATTERN_ARC_NAME, stg->a_num++);
              sprintf (buf_comment, PATTERN_ARC_COMMENT, node_arc->data->pred->data->name, node_trans_dum->data->name);
              node_arc_dup = NodeCreate (DataCreate (node_arc->data->type | STG_GENERATED, buf_name, buf_comment, node_arc->data->pred, NodeLocate(stg->t, node_trans_dum->data), NULL));
              NodeAdd (stg->a, node_arc_dup);
              Verbose (DEBUG_PROC, "      CREATE READ-ARC (type =%#.8X  name = %s  comment = %s)", node_arc_dup->data->type, node_arc_dup->data->name, node_arc_dup->data->comment);
              node_trans_dum = node_trans_dum->next;
	    }
            node_arc = node_arc->next;
          }
          node_trans_pred = node_trans_pred->next;
        }
      }
    }
    //	
    if (bl_node_removed)
    {
      stg_fill(stg);
      node_place_cur = stg->p;
    }
    else
      node_place_cur = node_place_cur->next;
  }
  else
    node_place_cur = node_place_cur->next;
  }

  // for every DUMMY TRANSITION which is a predicessor only of REDUNDANT places delete its reading arcs
  node_dum = stg->t;
  while ((node_dum = NodeFind (node_dum, DataFill (mask, STG_TRANSITION_DUMMY))) != NULL)
  {
    node_place_cur = node_dum->data->succ;
    while (node_place_cur != NULL && (node_place_cur->data->type & STG_REDUNDANT) == STG_REDUNDANT)
      node_place_cur = node_place_cur->next;
    if (node_place_cur == NULL)
    {
      DataFill (mask, STG_ARC_READING, NULL, NULL, NULL, node_dum, NULL);
      node_arc_r = stg->a;
      while ((node_arc_r = NodeFind (node_arc_r, mask)) != NULL)
      {
        node_dum_dup = node_arc_r->next;
        DataFree (node_arc_r->data);
        NodeDel (stg->a, node_arc_r);
        node_arc_r = node_dum_dup;
      }
    }
    node_dum = node_dum->next;
  }

  free (buf_name);
  free (buf_comment);
  DataFree (mask);
  stg_names_compress (stg);
  stg_type_and (stg->a, ~STG_PROCESSED);
  Verbose (DEBUG_STACK, "<-- omt_context");
  return 1;
}


/* === -r3 option */
int omt_redundant (stg_t *stg)
{
  Verbose (DEBUG_STACK, "--> omt_redundant");
  PNode node_place          = NULL;
  PNode node_place_cur      = NULL;
  PNode node_place_pred     = NULL;
  PNode node_place_dup      = NULL;
  PNode node_trans          = NULL;
  PNode node_trans_pred     = NULL;
  PNode node_trans_succ     = NULL;
  PNode node_trans_dup      = NULL;
  PNode node_trans_dup_list = NULL;
  PNode node_arc            = NULL;
  PNode node_arc_dup        = NULL;
  PNode node_mark           = NULL;
  PNode node_mark_dup       = NULL;
  PData mask                = NULL;
  char *buf_name            = NULL;
  char *buf_comment         = NULL;

  mask = DataCreate (STG_NATIVE, NULL, NULL, NULL, NULL);
  buf_name = (char *) malloc (STG_BUF_MAX);
  buf_comment = (char *) malloc (STG_BUF_MAX);
  // for every not-state-separation-place
  node_place_cur = stg->p;
  while ((node_place_cur = NodeFind (node_place_cur, DataFill (mask, STG_PLACE))) != NULL)
  {
    Verbose (DEBUG_PROC, "PLACE FOUND (type =%#.8X  name = %s  comment = %s)", node_place_cur->data->type, node_place_cur->data->name, node_place_cur->data->comment);
    if ((node_place_cur->data->type & STG_REDUNDANT) == STG_REDUNDANT)
    {
      Verbose (DEBUG_PROC, "REDUNDANT PLACE FOUND (type =%#.8X  name = %s  comment = %s)", node_place_cur->data->type, node_place_cur->data->name, node_place_cur->data->comment);
      // ADD CONSUMIN ARCS from every not-state-separation-place pred-transition pred-place duplication to not-state-separation-place succ-transition duplication
      // for every not-state-separation-place pred-transition...
      node_trans_pred = node_place_cur->data->pred;
      while (node_trans_pred != NULL)
      {
        // duplicate every not-state-separation-place succ-transition
        node_trans_succ = node_place_cur->data->succ;
        while (node_trans_succ != NULL)
        {
          node_trans = NodeLocate (stg->t, node_trans_succ->data);
          node_trans_dup = NodeCreate (DataCreate (node_trans->data->type | STG_GENERATED, stg_cnv_name (node_trans->data->link->data->name, stg_cnv_polarity (node_trans->data->name), node_trans->data->link->data->index_0++), NULL, NULL, NULL, node_trans->data->link));
          NodeAdd (stg->t, node_trans_dup);
          Verbose (DEBUG_PROC, "  REDUNDANT PLACE SUCC TRANSITION DUPLICATED (type =%#.8X  name = %s  comment = %s)", node_trans_dup->data->type, node_trans_dup->data->name, node_trans_dup->data->comment);
          NodeAdd (node_trans_dup_list, NodeCreate (node_trans_dup->data));
          // duplicate every not-state-separation-place succ-transition consuming-arc
          // (except of not-state-separation-place to not-state-separation-place succ-transition one)
          node_place = NodeLocate (stg->p, node_place_cur->data);
          node_arc = stg->a;
          while ((node_arc = NodeFind (node_arc, DataFill (mask, STG_ARC_CONSUMING, NULL, NULL, NULL, node_trans, NULL))) != NULL)
          {
            if (node_arc->data->pred->data != node_place->data)
            {
              sprintf (buf_name, PATTERN_ARC_NAME, stg->a_num++);
              sprintf (buf_comment, PATTERN_ARC_COMMENT, node_arc->data->pred->data->name, node_trans_dup->data->name);
              node_arc_dup = NodeCreate (DataCreate (node_arc->data->type | STG_GENERATED, buf_name, buf_comment, node_arc->data->pred, node_trans_dup, NULL));
              NodeAdd (stg->a, node_arc_dup);
              Verbose (DEBUG_PROC, "    REDUNDANT PLACE SUCC TRANSITION PRED CONSUMING ARC DUPLICATED (type =%#.8X  name = %s  comment = %s)", node_arc_dup->data->type, node_arc_dup->data->name, node_arc_dup->data->comment);
            }
            node_arc = node_arc->next;
          }
          // duplicate every not-state-separation-place succ-transition producing-arc
          node_arc = stg->a;
          while ((node_arc = NodeFind (node_arc, DataFill (mask, STG_ARC_CONSUMING, NULL, NULL, node_trans, NULL, NULL))) != NULL)
          {
            if (node_arc->data->succ->data != node_place->data)
            {
              sprintf (buf_name, PATTERN_ARC_NAME, stg->a_num++);
              sprintf (buf_comment, PATTERN_ARC_COMMENT, node_trans_dup->data->name, node_arc->data->succ->data->name);
              node_arc_dup = NodeCreate (DataCreate (node_arc->data->type | STG_GENERATED, buf_name, buf_comment, node_trans_dup, node_arc->data->succ, NULL));
              NodeAdd (stg->a, node_arc_dup);
              Verbose (DEBUG_PROC, "    REDUNDANT PLACE SUCC TRANSITION SUCC CONSUMING ARC DUPLICATED (type =%#.8X  name = %s  comment = %s)", node_arc_dup->data->type, node_arc_dup->data->name, node_arc_dup->data->comment);
            }
            node_arc = node_arc->next;
          }
          // duplicate every not-state-separation-place succ-transition reading-arc
          node_arc = stg->a;
          while ((node_arc = NodeFind (node_arc, DataFill (mask, STG_ARC_READING, NULL, NULL, NULL, node_trans, NULL))) != NULL)
          {
            if (node_arc->data->pred->data != node_place->data)
            {
              sprintf (buf_name, PATTERN_ARC_NAME, stg->a_num++);
              sprintf (buf_comment, PATTERN_ARC_COMMENT, node_arc->data->pred->data->name, node_trans_dup->data->name);
              node_arc_dup = NodeCreate (DataCreate (node_arc->data->type | STG_GENERATED, buf_name, buf_comment, node_arc->data->pred, node_trans_dup, NULL));
              NodeAdd (stg->a, node_arc_dup);
              Verbose (DEBUG_PROC, "    REDUNDANT PLACE SUCC TRANSITION READING ARC DUPLICATED (type =%#.8X  name = %s  comment = %s)", node_arc_dup->data->type, node_arc_dup->data->name, node_arc_dup->data->comment);
            }
            node_arc = node_arc->next;
          }
          node_trans_succ = node_trans_succ->next;
        }

        // duplicate every not-state-separation-place pred-transition pred-place
        node_place_pred = node_trans_pred->data->pred;
        while (node_place_pred != NULL)
        {
          node_place = NodeLocate (stg->p, node_place_pred->data);
          node_place_dup = NodeCreate (DataCreate (node_place->data->type | STG_GENERATED, node_place->data->name, node_place->data->comment, NULL, NULL, NULL));
          NodeAdd (stg->p, node_place_dup);
          Verbose (DEBUG_PROC, "  REDUNDANT PLACE PRED TRANSITION PRED PLACE DUPLICATED (type =%#.8X  name = %s  comment = %s)", node_place_dup->data->type, node_place_dup->data->name, node_place_dup->data->comment);
          // duplicate every not-state-separation-place pred-transition pred-place marking (if it exists)
          node_mark = node_place->data->link;
          if (node_mark != NULL)
          {
            sprintf (buf_name, PATTERN_MARKING_NAME, stg->m_num++);
            node_mark_dup = NodeCreate (DataCreate (node_mark->data->type | STG_GENERATED, buf_name, node_place_dup->data->name, NULL, NULL, node_place_dup));
            NodeAdd (stg->m, node_mark_dup);
            Verbose (DEBUG_PROC, "    REDUNDANT PLACE PRED TRANSITION PRED PLACE MARKING DUPLICATED (type =%#.8X  name = %s  comment = %s)", node_mark_dup->data->type, node_mark_dup->data->name, node_mark_dup->data->comment);
          }
          // duplicate every not-state-separation-place pred-transition pred-place producing-arc
          node_arc = stg->a;
          while ((node_arc = NodeFind (node_arc, DataFill (mask, STG_ARC_CONSUMING, NULL, NULL, NULL, node_place, NULL))) != NULL)
          {
            sprintf (buf_name, PATTERN_ARC_NAME, stg->a_num++);
            sprintf (buf_comment, PATTERN_ARC_COMMENT, node_arc->data->pred->data->name, node_place_dup->data->name);
            node_arc_dup = NodeCreate (DataCreate (node_arc->data->type | STG_GENERATED, buf_name, buf_comment, node_arc->data->pred, node_place_dup, NULL));
            NodeAdd (stg->a, node_arc_dup);
            Verbose (DEBUG_PROC, "    REDUNDANT PLACE PRED TRANSITION PRED PLACE PRED CONSUMING ARC DUPLICATED (type =%#.8X  name = %s  comment = %s)", node_arc_dup->data->type, node_arc_dup->data->name, node_arc_dup->data->comment);
            node_arc = node_arc->next;
          }
          // duplicate every not-state-separation-place pred-transition pred-place consuming-arc
          // (except from not-state-separation-place pred-transition pred-place to not-state-separation-place pred-transition one)
          node_trans = NodeLocate (stg->t, node_trans_pred->data);
          node_arc = stg->a;
          while ((node_arc = NodeFind (node_arc, DataFill (mask, STG_ARC_CONSUMING, NULL, NULL, node_place, NULL, NULL))) != NULL)
          {
            if (node_arc->data->succ->data != node_trans->data)
            {
              sprintf (buf_name, PATTERN_ARC_NAME, stg->a_num++);
              sprintf (buf_comment, PATTERN_ARC_COMMENT, node_place_dup->data->name, node_arc->data->succ->data->name);
              node_arc_dup = NodeCreate (DataCreate (node_arc->data->type | STG_GENERATED, buf_name, buf_comment, node_place_dup, node_arc->data->succ, NULL));
              NodeAdd (stg->a, node_arc_dup);
              Verbose (DEBUG_PROC, "    REDUNDANT PLACE PRED TRANSITION PRED PLACE SUCC CONSUMING ARC DUPLICATED (type =%#.8X  name = %s  comment = %s)", node_arc_dup->data->type, node_arc_dup->data->name, node_arc_dup->data->comment);
            }
            node_arc = node_arc->next;
          }
          // duplicate every not-state-separation-place pred-transition pred-place reading-arc
          node_arc = stg->a;
          while ((node_arc = NodeFind (node_arc, DataFill (mask, STG_ARC_READING, NULL, NULL, node_place, NULL, NULL))) != NULL)
          {
            sprintf (buf_name, PATTERN_ARC_NAME, stg->a_num++);
            sprintf (buf_comment, PATTERN_ARC_COMMENT, node_place_dup->data->name, node_arc->data->succ->data->name);
            node_arc_dup = NodeCreate (DataCreate (node_arc->data->type | STG_GENERATED, buf_name, buf_comment, node_place_dup, node_arc->data->succ, NULL));
            NodeAdd (stg->a, node_arc_dup);
            Verbose (DEBUG_PROC, "    REDUNDANT PLACE PRED TRANSITION PRED PLACE READING ARC DUPLICATED (type =%#.8X  name = %s  comment = %s)", node_arc_dup->data->type, node_arc_dup->data->name, node_arc_dup->data->comment);
            node_arc = node_arc->next;
          }
          // duplicate every not-state-separation-place reading-arc
          node_arc = stg->a;
          while ((node_arc = NodeFind (node_arc, DataFill (mask, STG_ARC_READING, NULL, NULL, node_place_cur, NULL, NULL))) != NULL)
          {
            sprintf (buf_name, PATTERN_ARC_NAME, stg->a_num++);
            sprintf (buf_comment, PATTERN_ARC_COMMENT, node_place_dup->data->name, node_arc->data->succ->data->name);
            node_arc_dup = NodeCreate (DataCreate (node_arc->data->type | STG_GENERATED, buf_name, buf_comment, node_place_dup, node_arc->data->succ, NULL));
            NodeAdd (stg->a, node_arc_dup);
            Verbose (DEBUG_PROC, "    REDUNDANT PLACE READING ARC DUPLICATED (type =%#.8X  name = %s  comment = %s)", node_arc_dup->data->type, node_arc_dup->data->name, node_arc_dup->data->comment);
            node_arc = node_arc->next;
          }

          // create consuming-arc from duplicated not-state-separation-place pred-transition pred-place to every duplicated not-state-separation-place succ-transition
          node_trans_succ = node_trans_dup_list;
          while (node_trans_succ != NULL)
          {
            node_trans_dup = NodeLocate (stg->t, node_trans_succ->data);
            sprintf (buf_name, PATTERN_ARC_NAME, stg->a_num++);
            sprintf (buf_comment, PATTERN_ARC_COMMENT, node_place_dup->data->name, node_trans_dup->data->name);
            node_arc = NodeCreate (DataCreate (STG_ARC_CONSUMING | STG_GENERATED, buf_name, buf_comment, node_place_dup, node_trans_dup, NULL));
            NodeAdd (stg->a, node_arc);
            Verbose (DEBUG_PROC, "  REDUNDANT PLACE PRED TRANSITION PRED PLACE DUP - SUCC TRANSITION DUP CONSUMING ARC CREATED (type =%#.8X  name = %s  comment = %s)", node_arc->data->type, node_arc->data->name, node_arc->data->comment);
            node_trans_succ = node_trans_succ->next;
          }

          node_place_pred = node_place_pred->next;
        }
        NodeFree (node_trans_dup_list);
        node_trans_pred = node_trans_pred->next;
      }
      //---
      // DELETE REDUNDANT PLACE, SUCC TRANSITIONS, PRED TRANSITIONS, PRED TRANSITION PRED PLACES
      node_place = NodeLocate (stg->p, node_place_cur->data);
      // delete every not-state-separation-place producing-arc
      node_arc = stg->a;
      while ((node_arc = NodeFind (node_arc, DataFill (mask, STG_ARC_CONSUMING, NULL, NULL, NULL, node_place, NULL))) !=NULL)
      {
        node_arc_dup = node_arc->next;
        Verbose (DEBUG_PROC, "  REDUNDANT PLACE PRED CONSUMING ARC DELETED (type =%#.8X  name = %s  comment = %s)", node_arc->data->type, node_arc->data->name, node_arc->data->comment);
        NodeDel (stg->a, node_arc, true);
        node_arc = node_arc_dup;
      }
      // delete every not-state-separation-place consuming-arc
      node_arc = stg->a;
      while ((node_arc = NodeFind (node_arc, DataFill (mask, STG_ARC_CONSUMING, NULL, NULL, node_place, NULL, NULL))) !=NULL)
      {
        node_arc_dup = node_arc->next;
        Verbose (DEBUG_PROC, "  REDUNDANT PLACE SUCC CONSUMING ARC DELETED (type =%#.8X  name = %s  comment = %s)", node_arc->data->type, node_arc->data->name, node_arc->data->comment);
        NodeDel (stg->a, node_arc, true);
        node_arc = node_arc_dup;
      }
      // delete every not-state-separation-place reading-arc
      node_arc = stg->a;
      while ((node_arc = NodeFind (node_arc, DataFill (mask, STG_ARC_READING, NULL, NULL, node_place, NULL, NULL))) !=NULL)
      {
        node_arc_dup = node_arc->next;
        Verbose (DEBUG_PROC, "  REDUNDANT PLACE READING ARC DELETED (type =%#.8X  name = %s  comment = %s)", node_arc->data->type, node_arc->data->name, node_arc->data->comment);
        NodeDel (stg->a, node_arc, true);
        node_arc = node_arc_dup;
      }

      //---
      // delete every not-state-separation-place succ-transition
      node_trans_succ = node_place_cur->data->succ;
      while (node_trans_succ != NULL)
      {
        node_trans_dup = node_trans_succ->next;
        node_trans = NodeLocate (stg->t, node_trans_succ->data);
        // delete every not-state-separation-place succ-transition consuming-arc
        node_arc = stg->a;
        while ((node_arc = NodeFind (node_arc, DataFill (mask, STG_ARC_CONSUMING, NULL, NULL, NULL, node_trans, NULL))) !=NULL)
        {
          node_arc_dup = node_arc->next;
          Verbose (DEBUG_PROC, "  REDUNDANT PLACE SUCC TRANSITION PRED CONSUMING ARC DELETED (type =%#.8X  name = %s  comment = %s)", node_arc->data->type, node_arc->data->name, node_arc->data->comment);
          NodeDel (stg->a, node_arc, true);
          node_arc = node_arc_dup;
        }
        // delete every not-state-separation-place succ-transition producing-arc
        node_arc = stg->a;
        while ((node_arc = NodeFind (node_arc, DataFill (mask, STG_ARC_CONSUMING, NULL, NULL, node_trans, NULL, NULL))) !=NULL)
        {
          node_arc_dup = node_arc->next;
          Verbose (DEBUG_PROC, "  REDUNDANT PLACE SUCC TRANSITION SUCC CONSUMING ARC DELETED (type =%#.8X  name = %s  comment = %s)", node_arc->data->type, node_arc->data->name, node_arc->data->comment);
          NodeDel (stg->a, node_arc, true);
          node_arc = node_arc_dup;
        }
        // delete every not-state-separation-place succ-transition reading-arc
        node_arc = stg->a;
        while ((node_arc = NodeFind (node_arc, DataFill (mask, STG_ARC_READING, NULL, NULL, NULL, node_trans, NULL))) !=NULL)
        {
          node_arc_dup = node_arc->next;
          Verbose (DEBUG_PROC, "  REDUNDANT PLACE SUCC TRANSITION READING ARC DELETED (type =%#.8X  name = %s  comment = %s)", node_arc->data->type, node_arc->data->name, node_arc->data->comment);
          NodeDel (stg->a, node_arc, true);
          node_arc = node_arc_dup;
        }
        // delete not state separation place succ transition
        Verbose (DEBUG_PROC, "  REDUNDANT PLACE SUCC TRANSITION DELETED (type =%#.8X  name = %s  comment = %s)", node_trans->data->type, node_trans->data->name, node_trans->data->comment);
        NodeDel (stg->t, node_trans, true);
        node_trans_succ = node_trans_dup;
      }
      //---
      // delete every not-state-separation-place pred-transition if ...
      node_trans_pred = node_place_cur->data->pred;
      while (node_trans_pred != NULL)
      {
        node_trans_dup = node_trans_pred->next;
        node_trans = NodeLocate (stg->t, node_trans_pred->data);
        Verbose (DEBUG_PROC, "*** type =%#.8X  name = %s  comment = %s", node_trans->data->type, node_trans->data->name, node_trans->data->comment);
        // ... there are no producing-arc from it
        if (NodeFind (stg->a, DataFill (mask, STG_ARC_CONSUMING, NULL, NULL, node_trans, NULL, NULL)) == NULL)
        {
          // delete every not-state-separation-place pred-transition consuming-arc
          node_arc = stg->a;
          while ((node_arc = NodeFind (node_arc, DataFill (mask, STG_ARC_CONSUMING, NULL, NULL, NULL, node_trans, NULL))) !=NULL)
          {
            node_arc_dup = node_arc->next;
            Verbose (DEBUG_PROC, "  REDUNDANT PLACE PRED TRANSITION PRED CONSUMING ARC DELETED (type =%#.8X  name = %s  comment = %s)", node_arc->data->type, node_arc->data->name, node_arc->data->comment);
            NodeDel (stg->a, node_arc, true);
            node_arc = node_arc_dup;
          }
          // delete every not-state-separation-place pred-transition reading-arc
          node_arc = stg->a;
          while ((node_arc = NodeFind (node_arc, DataFill (mask, STG_ARC_READING, NULL, NULL, NULL, node_trans, NULL))) !=NULL)
          {
            node_arc_dup = node_arc->next;
            Verbose (DEBUG_PROC, "  REDUNDANT PLACE PRED TRANSITION READING ARC DELETED (type =%#.8X  name = %s  comment = %s)", node_arc->data->type, node_arc->data->name, node_arc->data->comment);
            NodeDel (stg->a, node_arc, true);
            node_arc = node_arc_dup;
          }

          // delete every not state separation place pred transition pred place
          node_place_pred = node_trans->data->pred;
          while (node_place_pred != NULL)
          {
            node_place_dup = node_place_pred->next;
            node_place = NodeLocate (stg->p, node_place_pred->data);
            if (node_place)
            {
              // delete every not state separation place pred transition pred place pred consuming arc
              node_arc = stg->a;
              while ((node_arc = NodeFind (node_arc, DataFill (mask, STG_ARC_CONSUMING, NULL, NULL, NULL, node_place, NULL))) !=NULL)
              {
                node_arc_dup = node_arc->next;
                Verbose (DEBUG_PROC, "  REDUNDANT PLACE PRED TRANSITION PRED PLACE PRED CONSUMING ARC DELETED (type =%#.8X  name = %s  comment = %s)", node_arc->data->type, node_arc->data->name, node_arc->data->comment);
                NodeDel (stg->a, node_arc, true);
                node_arc = node_arc_dup;
              }
              // delete every not state separation place pred transition pred place succ consuming arc
              node_arc = stg->a;
              while ((node_arc = NodeFind (node_arc, DataFill (mask, STG_ARC_CONSUMING, NULL, NULL, node_place, NULL, NULL))) !=NULL)
              {
                node_arc_dup = node_arc->next;
                Verbose (DEBUG_PROC, "  REDUNDANT PLACE PRED TRANSITION PRED PLACE SUCC CONSUMING ARC DELETED (type =%#.8X  name = %s  comment = %s)", node_arc->data->type, node_arc->data->name, node_arc->data->comment);
                NodeDel (stg->a, node_arc, true);
                node_arc = node_arc_dup;
              }
              // delete every not state separation place pred transition pred place reading arc
              node_arc = stg->a;
              while ((node_arc = NodeFind (node_arc, DataFill (mask, STG_ARC_READING, NULL, NULL, node_place, NULL, NULL))) !=NULL)
              {
                node_arc_dup = node_arc->next;
                Verbose (DEBUG_PROC, "  REDUNDANT PLACE PRED TRANSITION PRED PLACE READING ARC DELETED (type =%#.8X  name = %s  comment = %s)", node_arc->data->type, node_arc->data->name, node_arc->data->comment);
                NodeDel (stg->a, node_arc, true);
                node_arc = node_arc_dup;
              }
              // delete not state separation place pred transition pred place marking
              node_mark = node_place->data->link;
              if (node_mark != NULL)
              {
                Verbose (DEBUG_PROC, "    REDUNDANT PLACE PRED TRANSITION PRED PLACE MARKING DELETED (type =%#.8X  name = %s  comment = %s)", node_mark->data->type, node_mark->data->name, node_mark->data->comment);
                NodeDel (stg->m, node_mark, true);
              }
              // delete not state separation place pred transition pred place
              Verbose (DEBUG_PROC, "  REDUNDANT PLACE PRED TRANSITION PRED PLACE DELETED (type =%#.8X  name = %s  comment = %s)", node_place->data->type, node_place->data->name, node_place->data->comment);
              NodeDel (stg->p, node_place, true);

              node_place_pred = node_place_dup;
            }
            else
              node_place_pred = NULL;
          }
          // delete not state separation place pred transition
          Verbose (DEBUG_PROC, "  REDUNDANT PLACE PRED TRANSITION DELETED (type =%#.8X  name = %s  comment = %s)", node_trans->data->type, node_trans->data->name, node_trans->data->comment);
          NodeDel (stg->t, node_trans, true);
        }
        node_trans_pred = node_trans_dup;
      }
      //---
      // delete not state separation place marking (usualy there shoud not be any)
      node_mark = node_place_cur->data->link;
      if (node_mark != NULL)
      {
        Verbose (DEBUG_PROC, "    REDUNDANT PLACE MARKING DELETED (type =%#.8X  name = %s  comment = %s)", node_mark->data->type, node_mark->data->name, node_mark->data->comment);
        NodeDel (stg->m, node_mark, true);
      }
      // delete not state separation place
      node_place = NodeLocate (stg->p, node_place_cur->data);
      Verbose (DEBUG_PROC, "  REDUNDANT PLACE DELETED (type =%#.8X  name = %s  comment = %s)", node_place->data->type, node_place->data->name, node_place->data->comment);
      NodeDel (stg->p, node_place, true);
      stg_fill (stg);
      node_place_cur = stg->p;
    }
    else
      node_place_cur = node_place_cur->next;
  }
  free (buf_name);
  free (buf_comment);
  DataFree (mask);
  stg_names_compress (stg);
  Verbose (DEBUG_STACK, "<-- omt_redundant");
  return 1;
}

/* === -r4 option */
int omt_compress (stg_t *stg)
{
  return stg_compress(stg);
}

/* === -r5 option */
void omt_simplify (stg_t *stg, int rate)
{
  Verbose (DEBUG_STACK, "--> omt_simplify");
  PNode node_place_cur      = NULL;
  PNode node_place_cur_     = NULL;
  PNode node_trans_cur      = NULL;
  PNode node_place_new      = NULL;
  PNode node_trans_new      = NULL;
  PNode node_trans_new_     = NULL;
  PNode node_signal_new     = NULL;
  PNode node_arc            = NULL;
  PNode node_arc_dup        = NULL;
  PNode list_trans          = NULL;
  bool flag                 = false;
  PData mask                = NULL;
  char *buf_name            = NULL;
  char *buf_comment         = NULL;
  if (rate < 1) return;
  mask = DataCreate (STG_NATIVE, NULL, NULL, NULL, NULL);
  buf_name = (char *) malloc (STG_BUF_MAX);
  buf_comment = (char *) malloc (STG_BUF_MAX);

  // for every dummy transition which has more then _rate_ incomming consuming arcs and is controled by a read-arc
  node_trans_cur = stg->t;
  while ((node_trans_cur = NodeFind (node_trans_cur, DataFill (mask, STG_TRANSITION_DUMMY))) != NULL)
  {
    Verbose (DEBUG_PROC, "DUMMY TRANSITION FOUND (type =%#.8X  name = %s  comment = %s)", node_trans_cur->data->type, node_trans_cur->data->name, node_trans_cur->data->comment);
    if ((node_arc = NodeFind (stg->a, DataFill (mask, STG_ARC_READING, NULL, NULL, NULL, node_trans_cur, NULL))) != NULL)
    {
      Verbose (DEBUG_PROC, "  IT IS CONTROLED BY READ-ARC (type =%#.8X  name = %s  comment = %s)", node_arc->data->type, node_arc->data->name, node_arc->data->comment);
      if (NodeCount (node_trans_cur->data->pred) > rate)
      {
        Verbose (DEBUG_PROC, "  ...AND HAS MORE THEN %d INCOMMING CONSUMING ARCS", rate);
        // Probably, all the elementry cycles, which control this dummy transition use
        // all pred places of the transition as context signals. If it is the case, then
        // we insert dummy transition and place before found transition and use newly
        // inserted place as context signal decreasing the fanin of elementary cycles.
        flag = false;
        NodeFree (list_trans);
        node_place_cur_ = node_trans_cur->data->pred;
        if (node_place_cur_ != NULL)
        {
          flag = true;
          node_place_cur = NodeLocate (stg->p, node_place_cur_->data);
          Verbose (DEBUG_PROC, "  BUILD LIST OF CONTROLED BY TRANSITION PRED PLACE TRANSITIONS (type =%#.8X  name = %s  comment = %s)", node_place_cur->data->type, node_place_cur->data->name, node_place_cur->data->comment);
          node_arc = stg->a;
          while ((node_arc = NodeFind (node_arc, DataFill (mask, STG_ARC_READING, NULL, NULL, node_place_cur, NULL, NULL))) !=NULL)
          {
            if ((node_arc->data->succ->data->type & STG_TRANSITION_OUTPUT) == STG_TRANSITION_OUTPUT)
            {
              node_trans_new = NodeCreate (node_arc->data->succ->data);
              NodeAdd (list_trans, node_trans_new);
              Verbose (DEBUG_PROC, "    ADD CONTROLED BY TRANSITION PRED PLACE TRANSITION TO LIST (type =%#.8X  name = %s  comment = %s)", node_trans_new->data->type, node_trans_new->data->name, node_trans_new->data->comment);
            }
            node_arc = node_arc->next;
          }
          //
          node_place_cur_ = node_trans_cur->data->pred->next;
          while (flag && node_place_cur_ != NULL)
          {
            node_place_cur = NodeLocate (stg->p, node_place_cur_->data);
            Verbose (DEBUG_PROC, "  COMPARE LIST WITH CONTROLED BY TRANSITION PRED PLACE TRANSITIONS (type =%#.8X  name = %s  comment = %s)", node_place_cur->data->type, node_place_cur->data->name, node_place_cur->data->comment);
            node_arc = stg->a;
            while (flag && ((node_arc = NodeFind (node_arc, DataFill (mask, STG_ARC_READING, NULL, NULL, node_place_cur, NULL, NULL))) !=NULL))
            {
              if ((node_arc->data->succ->data->type & STG_TRANSITION_OUTPUT) == STG_TRANSITION_OUTPUT)
              {
                node_trans_new = node_arc->data->succ;
                if (NodeFind (list_trans, node_trans_new->data) == NULL)
                {
                  flag = false;
                  Verbose (DEBUG_PROC, "    ARC IS NOT FOUND IN LIST (type =%#.8X  name = %s  comment = %s)", node_arc->data->type, node_arc->data->name, node_arc->data->comment);
                }
                else
                  Verbose (DEBUG_PROC, "    ARC IS FOUND IN LIST (type =%#.8X  name = %s  comment = %s)", node_arc->data->type, node_arc->data->name, node_arc->data->comment);
              }
              node_arc = node_arc->next;
            }
            node_place_cur_ = node_place_cur_->next;
          }
        }
        // OptiMise!
        if (flag)
        {
          // delete every transition pred consuming arc
          node_arc = stg->a;
          while ((node_arc = NodeFind (node_arc, DataFill (mask, STG_ARC_CONSUMING, NULL, NULL, NULL, node_trans_cur, NULL))) !=NULL)
          {
            node_arc_dup = node_arc->next;
            Verbose (DEBUG_PROC, "    TRANSITION PRED CONSUMING ARC DELETED (type =%#.8X  name = %s  comment = %s)", node_arc->data->type, node_arc->data->name, node_arc->data->comment);
            NodeDel (stg->a, node_arc, true);
            node_arc = node_arc_dup;
          }
          // create intermediate transition (and signal for it)
          sprintf (buf_name, PATTERN_TRANSITION_NAME, stg->t_num++);
          node_signal_new = NodeCreate (DataCreate (STG_SIGNAL_DUMMY, buf_name, NULL, NULL, NULL, NULL));
          NodeAdd (stg->s, node_signal_new);
          Verbose (DEBUG_PROC, "  SIGNAL FOR INTERMEDIATE TRANSITION CREATED (type =%#.8X  name = %s  comment = %s)", node_signal_new->data->type, node_signal_new->data->name, node_signal_new->data->comment);
          node_trans_new = NodeCreate (DataCreate (STG_TRANSITION_DUMMY | STG_GENERATED, buf_name, NULL, NULL, NULL, node_signal_new));
          NodeAdd (stg->t, node_trans_new);
          Verbose (DEBUG_PROC, "  INTERMEDIATE TRANSITION CREATED (type =%#.8X  name = %s  comment = %s)", node_trans_new->data->type, node_trans_new->data->name, node_trans_new->data->comment);
          // create intermediate place
          sprintf (buf_name, PATTERN_PLACE_NAME, stg->p_num++);
          node_place_new = NodeCreate (DataCreate (STG_PLACE_SEPARATOR | STG_GENERATED, buf_name, NULL, NULL, NULL, NULL));
          NodeAdd (stg->p, node_place_new);
          Verbose (DEBUG_PROC, "  INTERMEDIATE PLACE CREATED (type =%#.8X  name = %s  comment = %s)", node_place_new->data->type, node_place_new->data->name, node_place_new->data->comment);
          // create consuming arc from intermediate transition to intermediate place
          sprintf (buf_name, PATTERN_ARC_NAME, stg->a_num++);
          sprintf (buf_comment, PATTERN_ARC_COMMENT, node_trans_new->data->name, node_place_new->data->name);
          node_arc = NodeCreate (DataCreate (STG_ARC_CONSUMING | STG_GENERATED, buf_name, buf_comment, node_trans_new, node_place_new, NULL));
          NodeAdd (stg->a, node_arc);
          Verbose (DEBUG_PROC, "  CONSUMING ARC FROM INTERMEDIATE TRANSITION TO INTERMEDIATE PLACE CREATED (type =%#.8X  name = %s  comment = %s)", node_arc->data->type, node_arc->data->name, node_arc->data->comment);
          // create consuming arc from intermediate place to transition
          sprintf (buf_name, PATTERN_ARC_NAME, stg->a_num++);
          sprintf (buf_comment, PATTERN_ARC_COMMENT, node_place_new->data->name, node_trans_cur->data->name);
          node_arc = NodeCreate (DataCreate (STG_ARC_CONSUMING | STG_GENERATED, buf_name, buf_comment, node_place_new, node_trans_cur, NULL));
          NodeAdd (stg->a, node_arc);
          Verbose (DEBUG_PROC, "  CONSUMING ARC FROM INTERMEDIATE PLACE TO TRANSITION CREATED (type =%#.8X  name = %s  comment = %s)", node_arc->data->type, node_arc->data->name, node_arc->data->comment);
          // create consuming arcs from each transition pred place to intermediate transition
          node_place_cur_ = node_trans_cur->data->pred;
          while (node_place_cur_ != NULL)
          {
            node_place_cur = NodeLocate (stg->p, node_place_cur_->data);
            sprintf (buf_name, PATTERN_ARC_NAME, stg->a_num++);
            sprintf (buf_comment, PATTERN_ARC_COMMENT, node_place_cur->data->name, node_trans_new->data->name);
            node_arc = NodeCreate (DataCreate (STG_ARC_CONSUMING | STG_GENERATED, buf_name, buf_comment, node_place_cur, node_trans_new, NULL));
            NodeAdd (stg->a, node_arc);
            Verbose (DEBUG_PROC, "  CONSUMING ARC FROM TANSITION PRED PLACE TO INTERMEDIATE TRANSITION CREATED (type =%#.8X  name = %s  comment = %s)", node_arc->data->type, node_arc->data->name, node_arc->data->comment);
            node_place_cur_ = node_place_cur_->next;
          }
          // delete read arcs from each transition pred place to each transition in the list_trans
          node_place_cur_ = node_trans_cur->data->pred;
          while (node_place_cur_ != NULL)
          {
            node_place_cur = NodeLocate (stg->p, node_place_cur_->data);
            node_trans_new_ = list_trans;
            while (node_trans_new_ != NULL)
            {
              node_trans_new = NodeLocate (stg->t, node_trans_new_->data);
              node_arc = NodeFind (stg->a, DataFill (mask, STG_ARC_READING, NULL, NULL, node_place_cur, node_trans_new, NULL));
              if (node_arc != NULL)
              {
                Verbose (DEBUG_PROC, "  TRANSITION PRED PLACE READ-ARC DELETED (type =%#.8X  name = %s  comment = %s)", node_arc->data->type, node_arc->data->name, node_arc->data->comment);
                NodeDel (stg->a, node_arc, true);
              }
              node_trans_new_ = node_trans_new_->next;
            }
            node_place_cur_ = node_place_cur_->next;
          }
          // create read arcs from intermediate place to each transition in the list_trans
          node_trans_new_ = list_trans;
          while (node_trans_new_ != NULL)
          {
            node_trans_new = NodeLocate (stg->t, node_trans_new_->data);
            sprintf (buf_name, PATTERN_ARC_NAME, stg->a_num++);
            sprintf (buf_comment, PATTERN_ARC_COMMENT, node_place_new->data->name, node_trans_new->data->name);
            node_arc = NodeCreate (DataCreate (STG_ARC_READING | STG_GENERATED, buf_name, buf_comment, node_place_new, node_trans_new, NULL));
            NodeAdd (stg->a, node_arc);
            Verbose (DEBUG_PROC, "  READI-ARC FROM INTERMEDIATE PLACE TO TRANSITION IN LIST CREATED (type =%#.8X  name = %s  comment = %s)", node_arc->data->type, node_arc->data->name, node_arc->data->comment);
            node_trans_new_ = node_trans_new_->next;
          }
          stg_fill (stg);
        }
      }
    }
    node_trans_cur = node_trans_cur->next;
  }
  free (buf_name);
  free (buf_comment);
  DataFree (mask);
  NodeFree (list_trans);
  stg_fill (stg);
  stg_names_compress (stg);
  Verbose (DEBUG_STACK, "<-- omt_simplify");
}
