#ifndef _LIST_H_
#define _LIST_H_
#include <stddef.h>

struct TNode;
typedef struct TNode *PNode;

struct TData
{
  int type;
  char *name;
  char *comment;
  PNode pred;
  PNode succ;
  PNode link;
  int index_0;
  int index_1;
};

typedef TData *PData;

struct TNode
{
  PData data;
  PNode prev;
  PNode next;
};

extern PData DataCreate (int type = 0, char *name = NULL, char *comment = NULL, PNode pred = NULL, PNode succ = NULL, PNode link = NULL, int index_0 = 0, int index_1 = 0);
extern PData DataFill (PData data, int type = 0, char *name = NULL, char *comment = NULL, PNode pred = NULL, PNode succ = NULL, PNode link = NULL, int index_0 = 0, int index_1 = 0);
extern bool DataCmp (PData data, PData mask);
extern int DataFree (PData &data);

extern PNode NodeCreate (PData data);
extern int NodeAdd (PNode &head, PNode node);
extern int NodeDel (PNode &head, PNode &node, bool data_free = false);
extern PNode NodeFind (PNode head, PData mask);
extern PNode NodeLocate (PNode head, PData data);
extern int NodeCount (PNode head);
extern int NodeRevert (PNode &head);
extern int NodeSwap (PNode &head, PNode node_1, PNode node_2);
extern int NodeSort (PNode &head);
extern int NodeClear (PNode head);
extern int NodeCompare (PNode head1, PNode head2);
extern int NodeFree (PNode &head);

#endif
