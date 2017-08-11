#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "debug.h"
#include "list.h"
#include "stg.h"
#include "stg_procs.h"
#include "omd_procs.h"


/* === */
bool omd_triple_circle (PNode &stack, int num_loop)
{
  Verbose (DEBUG_STACK, "--> omd_triple_circle");
  PNode node                   = NULL;
  int count                    = 0;
  bool result                  = true;
  bool cutoff                  = false;

  node = stack->next;
  while (result && !cutoff && node != NULL)
  {
    if (stack->data == node->data)
    {
      cutoff = true;
      if (node->next == NULL && count < num_loop)
       result = false;
    }
    else
    if ((node->data->type & STG_PLACE) == STG_PLACE)
      if ((node->data->type & STG_REDUNDANT) != STG_REDUNDANT)
      {
        count++;
        if (count >= num_loop)
          cutoff = true;
      }
    node = node->next;
  }

  if (!cutoff)
  {
    node = stack->data->succ;
    while (result && node != NULL)
    {
      NodeAdd (stack, NodeCreate (node->data));
      result = omd_triple_circle (stack, num_loop);
      NodeDel (stack, stack);
      node = node->next;
    }
  }
  Verbose (DEBUG_STACK, "<-- omd_triple_circle");
  return result;
}


/* === */
int omd_region (PNode &stack, PNode &list, int direction)
{
  Verbose (DEBUG_STACK, "--> omd_region");
  PNode node   = NULL;
  PNode node_s = NULL;
  bool b       = true;

  if ((stack->data->type & STG_PLACE) == STG_PLACE)
    if ((stack->data->type & STG_REDUNDANT) != STG_REDUNDANT)
      b = false;

  if (b)
  {
    node = stack->next;
    while (b && node != NULL)
    {
      if (stack->data == node->data)
        b = false;
      node = node->next;
    }
  }

  if (b)
  {
    if (direction >= 0)
      node = stack->data->succ;
    else
      node = stack->data->pred;
    while (node != NULL)
    {
      if ((node->data->type & STG_TRANSITION) == STG_TRANSITION)
      {
        b = true;
        node_s = list;
        while (node_s != NULL && b)
        {
          if (node->data->link->data == node_s->data)
            b = false;
          node_s = node_s->next;
        }
        if (b) NodeAdd (list, NodeCreate (node->data->link->data));
      }
      NodeAdd (stack, NodeCreate (node->data));
      omd_region (stack, list, direction);
      NodeDel (stack, stack);
      node = node->next;
    }
  }
  Verbose (DEBUG_STACK, "<-- omd_region");
  return 1;
}


/* === */
bool omd_redundant (stg_t *stg, PNode node, int num_loop)
{
  Verbose (DEBUG_STACK, "--> omd_redundant");
  PNode list_pred = NULL;
  PNode list_succ = NULL;
  PNode node_pred = NULL;
  PNode node_succ = NULL;
  PNode stack     = NULL;
  int type        = 0;
  bool result     = true;

  type = node->data->type;
  node->data->type |= STG_REDUNDANT;

  if (result)
    result = ((node->data->type & STG_MANDATORY) != STG_MANDATORY);

  if (result)
  {
    NodeAdd (stack, NodeCreate (node->data));
    result = omd_triple_circle (stack, num_loop);
    NodeDel (stack, stack);
  }

  if (result)
  {
    NodeAdd (stack, NodeCreate (node->data));
    omd_region (stack, list_pred, -1);
    omd_region (stack, list_succ, +1);
    NodeDel (stack, stack);
    Verbose (DEBUG_LOCAL, "list_pred = %p  list_succ = %p", list_pred, list_succ);

    Verbose (DEBUG_LOCAL, "LIST_PRED:");
    node_pred = list_pred;
    while (node_pred != NULL)
    {
      Verbose (DEBUG_LOCAL, "  type = %#.8X  name = %s  comment = %s  pred = %p  succ = %p  link = %p", node_pred->data->type, node_pred->data->name, node_pred->data->comment, node_pred->data->pred, node_pred->data->succ, node_pred->data->link);
      node_pred = node_pred->next;
    }

    Verbose (DEBUG_LOCAL, "LIST_SUCC:");
    node_succ = list_succ;
    while (node_succ != NULL)
    {
      Verbose (DEBUG_LOCAL, "  type = %#.8X  name = %s  comment = %s  pred = %p  succ = %p  link = %p", node_succ->data->type, node_succ->data->name, node_succ->data->comment, node_succ->data->pred, node_succ->data->succ, node_succ->data->link);
      node_succ = node_succ->next;
    }
    node_pred = list_pred;
    while (node_pred != NULL && result)
    {
      node_succ = list_succ;
      while (node_succ != NULL && result)
      {
        if (node_pred->data == node_succ->data)
          result = false;
        node_succ = node_succ->next;
      }
      node_pred = node_pred->next;
    }
    NodeFree (list_pred);
    NodeFree (list_succ);
  }

  node->data->type = type;
  Verbose (DEBUG_STACK, "<-- omd_redundant");
  return result;
}


/* === */
void omd_trace (PNode head)
{
  Verbose (DEBUG_STACK, "--> omd_trace");
  PNode node                   = NULL;

  if ((head->data->type & STG_TRACED) != STG_TRACED)
  {
    head->data->type |= STG_TRACED;
    node = head->data->succ;
    while (node != NULL)
    {
      omd_trace (node);
      node = node->next;
    }
  }
  Verbose (DEBUG_STACK, "<-- omd_trace");
}


/* === */
bool omd_mandatory_after_fork (stg_t *stg, PNode node)
{
  Verbose (DEBUG_STACK, "--> omd_mandatory_after_fork");
  bool result                  = false;
  PNode node_pred              = NULL;
  PNode node_start             = NULL;
  PNode node_concurrent        = NULL;

  Verbose (DEBUG_PROC, "node->data->name = %s", node->data->name);
  node_pred = node->data->pred;
  while (node_pred != NULL)
  {
    Verbose (DEBUG_PROC, "  node_pred->data->name = %s", node_pred->data->name);
    if (NodeCount (node_pred->data->succ) > 1)
    {
      Verbose (DEBUG_PROC, "    fork transition");
      stg_type_and (stg->p, ~STG_TRACED);
      stg_type_and (stg->t, ~STG_TRACED);
      // Tag all transitions preceding the tested place
      node_pred->data->type |= STG_TRACED;
      // Trace from starting places
      node_start = stg->p;
      while (node_start != NULL)
      {
        if (node_start->data->pred == NULL)
          omd_trace (node_start);
        node_start = node_start->next;
      }
      // Trace from starting transitions
      node_start = stg->t;
      while (node_start != NULL)
      {
        if (node_start->data->pred == NULL)
          omd_trace (node_start);
        node_start = node_start->next;
      }
      // Trace from tested place
      omd_trace (node);
      // Test all concurrent (that were not traced) if any of them is of the same
      // signal as the preceding fork trnasition
      node_concurrent = stg->t;
      while (node_concurrent != NULL)
      {
        if ((node_concurrent->data->type & STG_TRACED) != STG_TRACED)
        {
          Verbose (DEBUG_PROC, "    node_concurrent->data->name = %s", node_concurrent->data->name);
          if (node_concurrent->data->link->data == node_pred->data->link->data)
          {
            Verbose (DEBUG_PROC, "    node_concurrent->data->name = %s (transition of the fork signal)", node_concurrent->data->name);
            result = true;
          }
        }
        node_concurrent = node_concurrent->next;
      }
    }
    node_pred = node_pred->next;
  }
  Verbose (DEBUG_STACK, "<-- omd_mandatory_after_fork");
  return result;
}


/* === -s0 option*/
int omd_mandatory (stg_t *stg)
{
  Verbose (DEBUG_STACK, "--> omd_mandatory");
  PNode node        = NULL;
  PNode node_pred   = NULL;
  PNode node_succ   = NULL;
  PNode node_succ_p = NULL;

  node = stg->p;
  while (node != NULL)
  {
    if ((node->data->type & STG_REDUNDANT) != STG_REDUNDANT)
    if ((node->data->type & STG_MANDATORY) != STG_MANDATORY)
    {
      if (omd_mandatory_after_fork (stg, node))
      {
        node_pred = node->data->pred;
        while (node_pred != NULL)
        {
          node_succ = node_pred->data->succ;
          while (node_succ != NULL)
          {
            if ((node_succ->data->type & STG_PLACE) == STG_PLACE)
            {
              node_succ->data->type |= STG_MANDATORY;
            }
            node_succ = node_succ->next;
          }
          node_pred = node_pred->next;
        }
      }
    }
    node = node->next;
  }

  node = stg->p;
  while (node != NULL)
  {
    if ((node->data->type & STG_REDUNDANT) != STG_REDUNDANT)
    {
      if (NodeCount (node->data->succ) > 1)
      {
        node_succ = node->data->succ;
        while (node_succ != NULL)
        {
          node_succ_p = node_succ->data->succ;
          while (node_succ_p != NULL)
          {
            node_succ_p->data->type |= STG_MANDATORY;
            node_succ_p = node_succ_p->next;
          }
          node_succ = node_succ->next;
        }
      }
    }
    node = node->next;
  }

  Verbose (DEBUG_STACK, "<-- omd_mandatory");
  return 1;
}


/* === -s0 option*/
int omd_force_class (stg_t *stg, int start, int end, int fork, int join, int choice, int merge, int mark)
{
  Verbose (DEBUG_STACK, "--> omd_force_class");
  PNode node_p = NULL;
  PNode node_t = NULL;
  bool done    = false;

  node_p = stg->p;
  while (node_p != NULL)
  {
    if ((node_p->data->type & STG_MANDATORY) != STG_MANDATORY)
    {
      done = false;

      // start
      if (!done)
        if (start)
          if (node_p->data->pred == NULL)
            done = true;


      // end
      if (!done)
        if (end)
          if (node_p->data->succ == NULL)
            done = true;

      // fork
      if (!done)
        if (fork)
        {
          node_t = node_p->data->pred;
          while (node_t != NULL && !done)
          {
            if (node_t->data->succ != NULL)
              if (node_t->data->succ->next != NULL)
                done = true;
            node_t = node_t->next;
          }
        }

      // join
      if (!done)
        if (join)
        {
          node_t = node_p->data->pred;
          while (node_t != NULL && !done)
          {
            if (node_t->data->pred != NULL)
              if (node_t->data->pred->next != NULL)
                done = true;
            node_t = node_t->next;
          }
        }

      // choice
      if (!done)
        if (choice)
          if (node_p->data->succ != NULL)
            if (node_p->data->succ->next != NULL)
              done = true;

      // merge
      if (!done)
        if (merge)
          if (node_p->data->pred != NULL)
            if (node_p->data->pred->next != NULL)
              done = true;

      // mark
      if (!done)
        if (mark)
          if (node_p->data->link != NULL)
              done = true;

      // action
      if (done)
      {
        node_p->data->type &= ~STG_REDUNDANT;
        node_p->data->type |= STG_MANDATORY;
      }
    }
    node_p = node_p->next;
  }

  Verbose (DEBUG_STACK, "<-- omd_force_class");
  return 1;
}


/* === -s0 option*/
int omd_force_list (stg_t *stg, char *redundant_names, char *separator_names, int num_loop)
{
  Verbose (DEBUG_STACK, "--> omd_force_list");
  PNode node_place = NULL;
  char *s          = NULL;
  char *name       = NULL;

  // separator
  if (separator_names != NULL)
  {
    name = NULL;
    s = (char *) realloc (s, strlen (separator_names) + 1);
    strcpy (s, separator_names);
    while (name != s)
    {
      name = strrchr (s, OMD_NAME_SEP);
      if (name == NULL)
        name = s;
      else
        name++;

      node_place = NodeFind (stg->p, DataCreate (STG_PLACE, name, NULL));
      if (node_place != NULL)
      {
        node_place->data->type |= STG_MANDATORY;
        node_place->data->type &= ~STG_REDUNDANT;
      }
      else
        Warning ("Place %s does not exist!", name);

      if (name != s)
        name[-1] = '\0';
    }
  }

  // redundant
  if (redundant_names != NULL)
  {
    name = NULL;
    s = (char *) realloc (s, strlen (redundant_names) + 1);
    strcpy (s, redundant_names);
    while (name != s)
    {
      name = strrchr (s, OMD_NAME_SEP);
      if (name == NULL)
        name = s;
      else
        name++;

      node_place = NodeFind (stg->p, DataCreate (STG_PLACE, name, NULL));
      if (node_place != NULL)
      {

//        if (omd_redundant (stg, node_place, num_loop))
        {
          node_place->data->type |= STG_REDUNDANT;
          node_place->data->type &= ~STG_MANDATORY;
        }
//        else
//          Fatal ("Place %s cannot be redundant!", name);
      }
      else
        Warning ("Place %s does not exist!", name);

      if (name != s)
        name[-1] = '\0';
    }
  }

  free (s);
  Verbose (DEBUG_STACK, "<-- omd_force_list");
  return 1;
}


/* === -s1 option*/
int omd_eliminate_input_output (stg_t *stg, int num_loop)
{
  Verbose (DEBUG_STACK, "--> omd_eliminate_input_output");
  PNode node_p   = NULL;
  PNode node_t   = NULL;
  bool eleminate = false;

  node_p = stg->p;
  while (node_p != NULL)
  {
    eleminate = ((node_p->data->type & STG_MANDATORY) != STG_MANDATORY);

    if (eleminate)
    {
      // all pred are inputs
      node_t = node_p->data->pred;
      while (node_t != NULL && eleminate)
      {
        if ((node_t->data->type & STG_TRANSITION_INPUT) != STG_TRANSITION_INPUT)
          eleminate = false;
        node_t = node_t->next;
      }

      // all succ are not inputs
      node_t = node_p->data->succ;
      while (node_t != NULL && eleminate)
      {
//        if ((node_t->data->type & STG_TRANSITION_OUTPUT) != STG_TRANSITION_OUTPUT)
        if ((node_t->data->type & STG_TRANSITION_INPUT) == STG_TRANSITION_INPUT)
          eleminate = false;
        node_t = node_t->next;
      }
    }

    // action
    if (eleminate)
      if (omd_redundant (stg, node_p, num_loop))
        node_p->data->type |= STG_REDUNDANT;

    node_p = node_p->next;
  }
  Verbose (DEBUG_STACK, "<-- omd_eliminate_input_output");
  return 1;
}


/* === */
bool omd_eliminate_chain_cutoff (stg_t *stg, PNode stack)
{
  Verbose (DEBUG_STACK, "--> omd_eliminate_chain_cutoff");
  PNode node  = NULL;
  bool result = false;

  if ((stack->data->type & STG_PLACE) == STG_PLACE)
  {
    node = stack->next;
    while (node != NULL && !result)
    {
      if (node->data == stack->data)
        result = true;
      node = node->next;
    }
  }
  Verbose (DEBUG_STACK, "<-- omd_eliminate_chain_cutoff");
  return result;
}


/* === */
bool omd_eliminate_chain_test (stg_t *stg, PNode stack)
{
  Verbose (DEBUG_STACK, "--> omd_eliminate_chain_test");
  PNode node  = NULL;
  bool result = true;

  if ((stack->data->type & STG_PLACE) == STG_PLACE)
  {
    result = ((stack->data->type & STG_MANDATORY) != STG_MANDATORY);

    //--- processed state not redundant place
    if (result)
      if ((stack->data->type & STG_PROCESSED) == STG_PROCESSED && (stack->data->type & STG_REDUNDANT) != STG_REDUNDANT)
        result = false;
  }
  else
  if ((stack->data->type & STG_TRANSITION) == STG_TRANSITION)
  {
    //--- transition after processed not redundant place
    if (result)
      if ((stack->next->data->type & STG_PROCESSED) == STG_PROCESSED && (stack->next->data->type & STG_REDUNDANT) != STG_REDUNDANT)
        result = false;

    //--- transition after the last not redundant plase in a chain
    if (result)
    {
      node = stack->data->succ;
      while (result && node != NULL)
      {
        if ((node->data->type & STG_REDUNDANT) == STG_REDUNDANT)
          result = false;
        node = node->next;
      }
    }

    //--- transition after place that separates it from privious transition of the same signal
    if (result)
    {
      Verbose (DEBUG_LOCAL, "TEST (type = %#.8X  name = %s  comment = %s  pred = %p  succ = %p  link = %p)", stack->data->type, stack->data->name, stack->data->comment, stack->data->pred, stack->data->succ, stack->data->link);
      node = stack->next;
      while (result && node != NULL)
      {
        Verbose (DEBUG_LOCAL, "  WITH (type = %#.8X  name = %s  comment = %s  pred = %p  succ = %p  link = %p)", node->data->type, node->data->name, node->data->comment, node->data->pred, node->data->succ, node->data->link);
        if ((node->data->type & STG_PROCESSED) == STG_PROCESSED)
          break;
        if ((node->data->type & STG_TRANSITION) == STG_TRANSITION)
        {
          if (node->data->link == stack->data->link)
            result = false;
        }
        node = node->next;
      }
    }
  }
  Verbose (DEBUG_STACK, "<-- omd_eliminate_chain_test");
  return result;
}


/* === */
int omd_eliminate_chain_find (stg_t *stg, PNode &stack, int num_loop)
{
  Verbose (DEBUG_STACK, "--> omd_eliminate_chain_find");
  PNode node = NULL;

  if (!omd_eliminate_chain_cutoff (stg, stack))
  {
    node = stack->data->succ;
    while (node != NULL)
    {
      NodeAdd (stack, NodeCreate (node->data));
      Verbose (DEBUG_LOCAL, "  NODE PUSHED TO STACK (type = %#.8X  name = %s  comment = %s  pred = %p  succ = %p  link = %p)", stack->data->type, stack->data->name, stack->data->comment, stack->data->pred, stack->data->succ, stack->data->link);
      if (omd_eliminate_chain_test (stg, stack))
      {
        // make plase preciding current transition redundant
        if ((stack->data->type & STG_TRANSITION) == STG_TRANSITION)
          if (stack->next != NULL)
          {
            if (omd_redundant (stg, stack->next, num_loop))
            {
              stack->next->data->type |= STG_REDUNDANT;
              stack->next->data->type &= ~STG_MANDATORY;
            }
            Verbose (DEBUG_LOCAL, "  REDUNDANT PLACE (type = %#.8X  name = %s  comment = %s  pred = %p  succ = %p  link = %p)", stack->next->data->type, stack->next->data->name, stack->next->data->comment, stack->next->data->pred, stack->next->data->succ, stack->next->data->link);
          }
      }
      else
      {
        // set processed flag to the current place (or place preceding current transition)
        if ((stack->data->type & STG_TRANSITION) == STG_TRANSITION)
        {
          if (stack->next != NULL)
            stack->next->data->type |= STG_PROCESSED;
        }
        else
          stack->data->type |= STG_PROCESSED;
      }
      omd_eliminate_chain_find (stg, stack, num_loop);
      Verbose (DEBUG_LOCAL, "  NODE POPED FROM STACK (type = %#.8X  name = %s  comment = %s  pred = %p  succ = %p  link = %p)", stack->data->type, stack->data->name, stack->data->comment, stack->data->pred, stack->data->succ, stack->data->link);
      NodeDel (stack, stack);
      node = node->next;
    }
  }
  Verbose (DEBUG_STACK, "<-- omd_eliminate_chain_find");
  return 1;
}


/* === -s2 option */
int omd_eliminate_chain (stg_t *stg, int num_loop)
{
  Verbose (DEBUG_STACK, "--> omd_eliminate_chain");
  PNode node_p     = NULL;
  PNode node_start = NULL;
  PNode node       = NULL;
  PNode stack      = NULL;

  stg_type_and (stg->p, ~STG_PROCESSED);
  stg_type_and (stg->t, ~STG_PROCESSED);

  node_p = stg->p;
  while (node_p != NULL)
  {
    // while there is any not processed plase...
    if ((node_p->data->type & STG_PROCESSED) != STG_PROCESSED)// && (node_p->data->type & STG_REDUNDANT) != STG_REDUNDANT)
    {
      // select starting place (which is state separation almost for sure) - !!! the priority shoud be studied !!!
      node_start = NULL;

      // FORSE STATE SEPARATION
      if (node_start == NULL)
      {
        Verbose (DEBUG_PROC, "--> place->data->type == STG_PLACE_SEPARATOR");
        node = stg->p;
        while (node != NULL && node_start == NULL)
        {
          if ((node->data->type & STG_PROCESSED) != STG_PROCESSED && (node->data->type & STG_MANDATORY) == STG_MANDATORY)
          {
            node_start = node;
            Verbose (DEBUG_PROC, "place->data->type == STG_PLACE_SEPARATOR (type = %#.8X  name = %s  comment = %s  pred = %p  succ = %p  link = %p)\n", node->data->type, node->data->name, node->data->comment, node->data->pred, node->data->succ, node->data->link);
          }
          node = node->next;
        }
        Verbose (DEBUG_PROC, "<-- place->data->type == STG_PLACE_SEPARATOR");
      }

      // place->data->pred == NULL // FIRST PLACE
      if (node_start == NULL)
      {
        Verbose (DEBUG_PROC, "--> place->data->pred == NULL");
        node = stg->p;
        while (node != NULL && node_start == NULL)
        {
          if ((node->data->type & STG_PROCESSED) != STG_PROCESSED && (node->data->type & STG_REDUNDANT) != STG_REDUNDANT)
            if (node->data->pred == NULL)
            {
              node_start = node;
              Verbose (DEBUG_PROC, "place->data->pred == NULL (type = %#.8X  name = %s  comment = %s  pred = %p  succ = %p  link = %p)\n", node->data->type, node->data->name, node->data->comment, node->data->pred, node->data->succ, node->data->link);
            }
          node = node->next;
        }
        Verbose (DEBUG_PROC, "<-- place->data->pred == NULL");
      }

      // place->data->succ == NULL // LAST PLACE
      if (node_start == NULL)
      {
        Verbose (DEBUG_PROC, "--> place->data->succ == NULL");
        node = stg->p;
        while (node != NULL && node_start == NULL)
        {
          if ((node->data->type & STG_PROCESSED) != STG_PROCESSED && (node->data->type & STG_REDUNDANT) != STG_REDUNDANT)
            if (node->data->succ == NULL)
            {
              node_start = node;
              Verbose (DEBUG_PROC, "place->data->succ == NULL (type = %#.8X  name = %s  comment = %s  pred = %p  succ = %p  link = %p)\n", node->data->type, node->data->name, node->data->comment, node->data->pred, node->data->succ, node->data->link);
            }
          node = node->next;
        }
        Verbose (DEBUG_PROC, "<-- place->data->succ == NULL");
      }

      // |place->data->pred->succ| > 1 // AFTER FORK PLACE
      if (node_start == NULL)
      {
        Verbose (DEBUG_PROC, "--> |place->data->pred->succ| > 1");
        node = stg->p;
        while (node != NULL && node_start == NULL)
        {
          if ((node->data->type & STG_PROCESSED) != STG_PROCESSED && (node->data->type & STG_REDUNDANT) != STG_REDUNDANT)
            if (node->data->pred != NULL)
              if (node->data->pred->data->succ->next != NULL)
              {
                node_start = node;
                Verbose (DEBUG_PROC, "|place->data->pred->succ| > 1 (type = %#.8X  name = %s  comment = %s  pred = %p  succ = %p  link = %p)", node->data->type, node->data->name, node->data->comment, node->data->pred, node->data->succ, node->data->link);
              }
          node = node->next;
        }
        Verbose (DEBUG_PROC, "<-- |place->data->pred->succ| > 1");
      }

      // |place->data->succ| > 1 // MERGE PLACE
      if (node_start == NULL)
      {
        Verbose (DEBUG_PROC, "--> |place->data->pred| > 1");
        node = stg->p;
        while (node != NULL && node_start == NULL)
        {
          if ((node->data->type & STG_PROCESSED) != STG_PROCESSED && (node->data->type & STG_REDUNDANT) != STG_REDUNDANT)
            if (node->data->pred != NULL)
              if (node->data->pred->next != NULL)
              {
                node_start = node;
                Verbose (DEBUG_PROC, "|place->data->pred| > 1 (type = %#.8X  name = %s  comment = %s  pred = %p  succ = %p  link = %p)", node->data->type, node->data->name, node->data->comment, node->data->pred, node->data->succ, node->data->link);
              }
          node = node->next;
        }
        Verbose (DEBUG_PROC, "<-- |place->data->pred| > 1");
      }

      // place => marked // MARKED PLACE
      if (node_start == NULL)
      {
        Verbose (DEBUG_PROC, "--> place => marked");
        node = stg->m;
        while (node != NULL && node_start == NULL)
        {
          if (node->data->link != NULL)
            if ((node->data->link->data->type & STG_PROCESSED) != STG_PROCESSED && (node->data->link->data->type & STG_REDUNDANT) != STG_REDUNDANT)
            {
              node_start = node->data->link;
              Verbose (DEBUG_PROC, "place => marked (type = %#.8X  name = %s  comment = %s  pred = %p  succ = %p  link = %p)", node->data->type, node->data->name, node->data->comment, node->data->pred, node->data->succ, node->data->link);
            }
          node = node->next;
        }
        Verbose (DEBUG_PROC, "<-- place => marked");
      }

      // |place->data->pred| > 1 // AFTER JOIN PLACE
      if (node_start == NULL)
      {
        Verbose (DEBUG_PROC, "--> |place->data->pred| > 1");
        node = stg->p;
        while (node != NULL && node_start == NULL)
        {
          if ((node->data->type & STG_PROCESSED) != STG_PROCESSED && (node->data->type & STG_REDUNDANT) != STG_REDUNDANT)
            if (node->data->pred != NULL)
              if (node->data->pred->next != NULL)
              {
                node_start = node;
                Verbose (DEBUG_PROC, "|place->data->pred| > 1 (type = %#.8X  name = %s  comment = %s  pred = %p  succ = %p  link = %p)", node->data->type, node->data->name, node->data->comment, node->data->pred, node->data->succ, node->data->link);
              }
          node = node->next;
        }
        Verbose (DEBUG_PROC, "<-- |place->data->pred| > 1");
      }

      // |place->data->succ| > 1 // CHOICE PLACE
      if (node_start == NULL)
      {
        Verbose (DEBUG_PROC, "--> |place->data->succ| > 1");
        node = stg->p;
        while (node != NULL && node_start == NULL)
        {
          if ((node->data->type & STG_PROCESSED) != STG_PROCESSED && (node->data->type & STG_REDUNDANT) != STG_REDUNDANT)
            if (node->data->succ != NULL)
              if (node->data->succ->next != NULL)
              {
                node_start = node;
                Verbose (DEBUG_PROC, "|place->data->succ| > 1 (type = %#.8X  name = %s  comment = %s  pred = %p  succ = %p  link = %p)", node->data->type, node->data->name, node->data->comment, node->data->pred, node->data->succ, node->data->link);
              }
          node = node->next;
        }
        Verbose (DEBUG_PROC, "<-- |place->data->succ| > 1");
      }

      // any place
      if (node_start == NULL)
      {
        Verbose (DEBUG_PROC, "--> any");
        node_start = node_p;
        Verbose (DEBUG_PROC, "<-- any");
      }

      // try to eliminate state separations from places after node_start
      if (node_start != NULL)
      {
        node_start->data->type |= STG_PROCESSED;
        NodeAdd (stack, NodeCreate (node_start->data));
        Verbose (DEBUG_LOCAL, "START NODE PUSHED TO STACK (type = %#.8X  name = %s  comment = %s  pred = %p  succ = %p  link = %p)", stack->data->type, stack->data->name, stack->data->comment, stack->data->pred, stack->data->succ, stack->data->link);
        omd_eliminate_chain_find (stg, stack, num_loop);
        Verbose (DEBUG_LOCAL, "START NODE POPED FROM STACK (type = %#.8X  name = %s  comment = %s  pred = %p  succ = %p  link = %p)", stack->data->type, stack->data->name, stack->data->comment, stack->data->pred, stack->data->succ, stack->data->link);
        NodeDel (stack, stack);
      }
      node_p = stg->p;
    }
    else
      node_p = node_p->next;
  }

  Verbose (DEBUG_STACK, "<-- omd_eliminate_chain");
  return 1;
}


/* === -s3 option */
int omd_eliminate_one (stg_t *stg, int num_loop)
{
  Verbose (DEBUG_STACK, "--> omd_eliminate_one");
  PNode node_p     = NULL;
  PNode node_start = NULL;
  PNode node       = NULL;

  node_p = stg->p;
  while (node_p != NULL)
  {
    if ((node_p->data->type & STG_MANDATORY) != STG_MANDATORY)
      if ((node_p->data->type & STG_REDUNDANT) != STG_REDUNDANT)
      {
        Verbose (DEBUG_LOCAL, "TRYING PLACE (type = %#.8X  name = %s  comment = %s  pred = %p  succ = %p  link = %p)", node_p->data->type, node_p->data->name, node_p->data->comment, node_p->data->pred, node_p->data->succ, node_p->data->link);
        if (omd_redundant (stg, node_p, num_loop))
          node_p->data->type |= STG_REDUNDANT;
      }
    node_p = node_p->next;
  }
  Verbose (DEBUG_STACK, "<-- omd_eliminate_one");
  return 1;
}
