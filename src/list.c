#include <stdlib.h>
#include <string.h>
#include "debug.h"
#include "list.h"


/* === */
PData DataCreate (int type, char *name, char *comment, PNode pred, PNode succ, PNode link, int index_0, int index_1)
{
//  Verbose (DEBUG_STACK, "--> DataCreate");
  PData data;
  data = (PData) malloc (sizeof (TData));
  data->name = NULL;
  data->comment = NULL;
  DataFill (data, type, name, comment, pred, succ, link, index_0, index_1);
//  Verbose (DEBUG_STACK, "<-- DataCreate");
  return data;
}


/* === */
PData DataFill (PData data, int type, char *name, char *comment, PNode pred, PNode succ, PNode link, int index_0, int index_1)
{
//  Verbose (DEBUG_STACK, "--> DataFill");
  data->type = type;
  free (data->name);
  if (name == NULL)
  {
    data->name = NULL;
  }
  else
  {
    data->name = (char *) malloc (strlen (name) + 10);
    strcpy (data->name, name);
  }

  free (data->comment);
  if (comment == NULL)
  {
    data->comment = NULL;
  }
  else
  {
    data->comment = (char *) malloc (strlen (comment) + 1);
    strcpy (data->comment, comment);
  }
  data->pred = pred;
  data->succ = succ;
  data->link = link;
  data->index_0 = index_0;
  data->index_1 = index_1;
//  Verbose (DEBUG_STACK, "<-- DataFill");
  return data;
}


/* === */
bool DataCmp (PData data, PData mask)
{
//  Verbose (DEBUG_STACK, "--> DataCmp");
  bool result = false;
  if ((mask->type & data->type) == mask->type)
    if (mask->name == NULL || strcmp (data->name, mask->name) == 0)
      if (mask->comment == NULL || strcmp (data->comment, mask->comment) == 0)
        if (mask->pred == NULL || data->pred == mask->pred)
          if (mask->succ == NULL || data->succ == mask->succ)
            if (mask->link == NULL || data->link == mask->link)
              result = true;
//  Verbose (DEBUG_STACK, "<-- DataCmp");
  return result;
}


/* === */
int DataFree (PData &data)
{
//  Verbose (DEBUG_STACK, "--> DataFree");
//  Verbose (DEBUG_STACK, "data = %p  data->name = %p (%s)  data->comment = %p (%s)", data, data->name, data->name, data->comment, data->comment);
  if (data)
  {
    if (data->name) free (data->name);
    if (data->comment) free (data->comment);
    free (data);
    data = NULL;
  }
//  Verbose (DEBUG_STACK, "<-- DataFree");
  return 1;
}


/* === */
PNode NodeCreate (PData data)
{
//  Verbose (DEBUG_STACK, "--> NodeCreate");
  PNode node;
  node = (PNode) malloc (sizeof (TNode));
  node->data = data;
//  Verbose (DEBUG_STACK, "<-- NodeCreate");
  return node;
}


/* === */
int NodeAdd (PNode &head, PNode node)
{
//  Verbose (DEBUG_STACK, "--> NodeAdd");
  if (node != NULL)
  {
    node->prev = NULL;
    node->next = head;
    if (head != NULL)
      head->prev = node;
    head = node;
  }
//  Verbose (DEBUG_STACK, "<-- NodeAdd");
  return 1;
}


/* === */
int NodeDel (PNode &head, PNode &node, bool data_free)
{
//  Verbose (DEBUG_STACK, "--> NodeDel");
  PNode head_next;
//  Verbose (DEBUG_LOCAL, "head = %p  node = %p", head, node);
  if (head != NULL)
  {
    if (node != NULL)
    {
      if (node == head)
      {
        head_next = head->next;
        if (data_free) DataFree (node->data);
        free (node);
        head = head_next;
        if (head != NULL)
          head->prev = NULL;
      }
      else
      {
        node->prev->next = node->next;
        if (node->next != NULL)
          node->next->prev = node->prev;
        if (data_free) DataFree (node->data);
        free (node);
      }
    }
  }
//  Verbose (DEBUG_STACK, "<-- NodeDel");
  return 1;
}


/* === */
PNode NodeFind (PNode head, PData mask)
{
//  Verbose (DEBUG_STACK, "--> NodeFind");
  PNode node;
//  Verbose (DEBUG_LOCAL, "mask: type = %#.8X  name = %s  comment = %s  pred = %p  succ = %p  link = %p", mask->type, mask->name, mask->comment, mask->pred, mask->succ, mask->link);
  node = head;
  while (node != NULL)
  {
//    Verbose (DEBUG_LOCAL, "node = %p  node->data = %p  node->next = %p  node->prev = %p", node, node->data, node->next, node->prev);
//    Verbose (DEBUG_LOCAL, "node->data: type = %#.8X  name = %s  comment = %s  pred = %p  succ = %p  link = %p", node->data->type, node->data->name, node->data->comment, node->data->pred, node->data->succ, node->data->link);
    if (DataCmp (node->data, mask))
      break;
    node = node->next;
  }
//  Verbose (DEBUG_STACK, "<-- NodeFind");
  return node;
}


/* === */
PNode NodeLocate (PNode head, PData data)
{
//  Verbose (DEBUG_STACK, "--> NodeLocate");
  PNode node;
  node = head;
  while (node != NULL)
  {
    if (node->data == data)
      break;
    node = node->next;
  }
//  Verbose (DEBUG_STACK, "<-- NodeLocate");
  return node;
}


/* === */
int NodeCount (PNode head)
{
//  Verbose (DEBUG_STACK, "--> NodeCount");
  PNode node;
  int count = 0;
  node = head;
  while (node != NULL) 
  {
    count++;
    node = node->next;
  }  
//  Verbose (DEBUG_STACK, "<-- NodeCount");
  return count;
}


/* === */
int NodeRevert (PNode &head)
{
//  Verbose (DEBUG_STACK, "--> NodeRevert");
  PNode node;
  PNode node_prev;
  PNode node_next;
  node = head;
  while (node != NULL) 
  {
    node_prev = node->prev;
    node_next = node->next;
    node->prev = node_next;
    node->next = node_prev;
    head = node;
    node = node_next;
  }
//  Verbose (DEBUG_STACK, "<-- NodeRevert");
  return 1;
}


/* === */
int NodeSwap (PNode &head, PNode node_1, PNode node_2)
{
//  Verbose (DEBUG_STACK, "--> NodeSwap");
  PNode node_dum;
  if (node_1 != node_2)
  {
    node_dum = node_1->prev;
    if (node_2->prev == node_1)
      node_1->prev = node_2;
    else
      node_1->prev = node_2->prev;
    if (node_dum == node_2)
      node_2->prev = node_1;
    else
      node_2->prev = node_dum;

    node_dum = node_1->next;
    if (node_2->next == node_1)
      node_1->next = node_2;
    else
      node_1->next = node_2->next;
    if (node_dum == node_2)
      node_2->next = node_1;
    else
      node_2->next = node_dum;
    
    if (node_1->prev != NULL)
      node_1->prev->next = node_1;
    else
      head = node_1;
    
    if (node_2->prev != NULL)
      node_2->prev->next = node_2;
    else
      head = node_2;
    
    if (node_1->next != NULL)
      node_1->next->prev = node_1;
    
    if (node_2->next != NULL)
      node_2->next->prev = node_2;
  }
//  Verbose (DEBUG_STACK, "<-- NodeSwap");
  return 1;
}

/* === */
int NodeSort (PNode &head)
{
//  Verbose (DEBUG_STACK, "--> NodeSort");
  PNode node;
  PNode node_;
  PNode node_min;

  node = head;
  while (node != NULL)
  {
    node_min = NULL;
    node_ = node;
    while (node_ != NULL)
    {
      if (node_min == NULL)
        node_min = node_;
      else
      if (strcmp (node_min->data->name, node_->data->name) > 0)
        node_min = node_;
      node_ = node_->next;
    }
    NodeSwap (head, node, node_min);
    node = node_min->next;
  }
//  Verbose (DEBUG_STACK, "<-- NodeSort");
  return 1;
}


/* === */
int NodeClear (PNode head)
{
//  Verbose (DEBUG_STACK, "--> NodeClear");
  PNode node;
  node = head;
  while (node != NULL)
  {
    DataFree (node->data);
    node = node->next;
  }
//  Verbose (DEBUG_STACK, "<-- NodeClear");
  return 1;
}

/* === */
int NodeCompare (PNode head1, PNode head2)
{
  PNode node;
  bool equal = false;
  if ((NodeCount(head1) > 0) && (NodeCount(head2) > 0))
  {
    equal = true;
    node = head1;
    while (equal && node)
    {
      if (NodeLocate(head2, node->data) == NULL)
        equal = false;
      node = node->next;
    }
    node = head2;
    while (equal && node)
    {
      if (NodeLocate(head1, node->data) == NULL)
        equal = false;
      node = node->next;
    }
  }
  return equal;
}


/* === */
int NodeFree (PNode &head)
{
//  Verbose (DEBUG_STACK, "--> NodeFree");
  PNode node;
  while (head != NULL)
  {
    node = head;
    head = head->next;
    free (node);
  }
//  Verbose (DEBUG_STACK, "<-- NodeFree");
  return 1;
}
