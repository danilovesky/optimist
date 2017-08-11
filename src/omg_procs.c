#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "debug.h"
#include "strutils.h"
#include "list.h"
#include "stg.h"
#include "stg_procs.h"
#include "omg_procs.h"


#define WORD_DIGRAPH          "digraph"
#define WORD_SUBGRAPH         "subgraph"
#define WORD_CLUSTER          "cluster"
#define WORD_STATEMENT_BEGIN  "{"
#define WORD_STATEMENT_END    "}"
#define WORD_ATTRIB_BEGIN     "["
#define WORD_ATTRIB_END       "]"
#define WORD_NODE             "node"
#define WORD_EDGE             "edge"
#define WORD_ARC              "->"
#define WORD_LABEL            "label"
#define WORD_COLOR            "color"
#define WORD_CENTER           "center"
#define WORD_SIZE             "size"
#define WORD_MARGIN           "margin"
#define WORD_RATIO            "ratio"

#define LABEL_NAME            "NAME: "
#define LABEL_INPUTS          "INPUTS: "
#define LABEL_OUTPUTS         "OUTPUTS: "
#define LABEL_INTERNAL        "INTERNAL: "
#define LABEL_DUMMY           "DUMMY: "
#define LABEL_DELIMITER       ", "
#define LABEL_EMPTY           "\"\""
#define COLOR_WHITE           "white"

#define ATTRIB_ARC_READING            "arrowhead=none, arrowtail=none, weight=1, color=gray, style=solid"
#define ATTRIB_ARC_READING_INPUT      "arrowhead=none, arrowtail=none, weight=1, color=gray, style=dashed"
#define ATTRIB_ARC_CONSUMING          "arrowhead=normal, arrowtail=none, weight=100, color=black, style=solid"
#define ATTRIB_ARC_CONSUMING_INPUT    "arrowhead=normal, arrowtail=none, weight=100, color=black, style=dashed"
#define ATTRIB_PLACE_EMPTY            "shape=circle, fontsize=11, fontcolor=black, height=0.4, width=0.4, fixedsize=true, color=black"
#define ATTRIB_PLACE_MARKED           "shape=circle, fontsize=11, fontcolor=red, height=0.3, width=0.3, fixedsize=true, color=black, peripheries=2, style=filled"
//#define ATTRIB_PLACE_EMPTY            "shape=circle, height=0.2, width=0.2, fixedsize=true, color=black"
//#define ATTRIB_PLACE_MARKED           "shape=circle, height=0.1, width=0.1, fixedsize=true, color=black, peripheries=2, style=filled"
#define ATTRIB_PLACE_REDUNDANT_EMPTY  "shape=circle, fontsize=11, fontcolor=black, height=0.4, width=0.4, fixedsize=true, color=gray"
#define ATTRIB_PLACE_REDUNDANT_MARKED "shape=circle, fontsize=11, fontcolor=white, height=0.3, width=0.3, fixedsize=true, color=gray, peripheries=2, style=filled"
//#define ATTRIB_TRANSITION             "shape=box, height=0.05, width=0.2, fixedsize=true, color=black, style=filled"
#define ATTRIB_PLACE_COPY             "shape=plaintext,    fontsize=11, fontcolor=black, height=0.2, width=0.4, fixedsize=true"
#define ATTRIB_PLACE_COPY_INPUT       "shape=plaintext,    fontsize=11, fontcolor=red, height=0.2, width=0.4, fixedsize=true"
#define ATTRIB_PLACE_COPY_OUTPUT      "shape=plaintext,    fontsize=11, fontcolor=blue, height=0.2, width=0.4, fixedsize=true"
#define ATTRIB_PLACE_TEST             "shape=circle, fontsize=11, fontcolor=black, height=0.4, width=0.4, fixedsize=true, color=red"
#define ATTRIB_TRANSITION             "shape=box,    fontsize=11, fontcolor=black, height=0.3, width=0.8, fixedsize=true, color=black"
#define ATTRIB_TRANSITION_INPUT       "shape=box,    fontsize=11, fontcolor=red,   height=0.3, width=0.8, fixedsize=true, color=gray, style=filled"
#define ATTRIB_TRANSITION_OUTPUT      "shape=box,    fontsize=11, fontcolor=blue,  height=0.3, width=0.8, fixedsize=true, color=black"

#define CNV_MINUS             "_minus"
#define CNV_PLUS              "_plus"
#define CNV_0                 "=0"
#define CNV_1                 "=1"

#define PATTERN_LABEL         "\"%s\""


/*===*/
char *dot_cnv_print (char *name)
{
  Verbose (DEBUG_STACK, "--> dot_cnv_print");
  int i = -1;
  char *s = NULL;
  char *s_ =NULL;
  bool dummy = false;

  s = (char *) malloc (strlen (name)+1);
  strcpy (s, name);
  
  while ((s_ = strstr (s, CNV_PLUS)) != NULL) 
  {
    s = (char *) realloc (s, strlen (s) - strlen (CNV_PLUS) + 2);
    i = s_ - s;
    s[i] = SIGN_POSITIVE;
    strcpy (&s[i+1], &s[i + strlen (CNV_PLUS)]);
    dummy = true;
  }
  
  while ((s_ = strstr (s, CNV_MINUS)) != NULL) 
  {
    s = (char *) realloc (s, strlen (s) - strlen (CNV_MINUS) + 2);
    i = s_ - s;
    s[i] = SIGN_NEGATIVE;
    strcpy (&s[i+1], &s[i + strlen (CNV_MINUS)]);
    dummy = true;
  }
  
  if ((s_ = strchr(s, '/')) != NULL)
    s_[0] = '\0';

  if (s[0] == SIGN_UNDERSCORE)
  {
    i = strlen (s);
    if (dummy)
    {
      s = (char *) realloc (s, i + 2);
      s[0] = '(';
      s[i] = ')';
      s[i+1] = '\0';
    }
    else
    {
      strcpy (s, &s[1]);
      s = (char *) realloc (s, i);
    }
  }

  i = strlen (s);
  if (i > 1)
    if (s[i-2] == SIGN_UNDERSCORE)
      if (s[i-1] == SIGN_LOW || s[i-1] == SIGN_HIGH)
        s[i-2] = SIGN_EQUAL;

  Verbose (DEBUG_LOCAL, "name = %s  s = %s", name, s);
  Verbose (DEBUG_STACK, "<-- dot_cnv_print");
  return s;
}


/*===*/
char *dot_cnv_label (char *name)
{
  Verbose (DEBUG_STACK, "--> dot_cnv_label");
  char *s = NULL;
  char *s_ =NULL;

  s_ = dot_cnv_print (name);
  s = (char *) realloc (s, strlen (s_) + strlen (PATTERN_LABEL));  
  sprintf (s, PATTERN_LABEL, s_);
  free (s_);
  Verbose (DEBUG_LOCAL, "name = %s  s = %s", name, s);
  Verbose (DEBUG_STACK, "<-- dot_cnv_label");
  return s;
}


/*===*/
char *dot_make_name (stg_t *stg)
{
  Verbose (DEBUG_STACK, "--> dot_make_name");
  char *name  = NULL;
  int i;

  name = (char *) malloc (strlen (stg->model_name) + 1);
  strcpy (name, stg->model_name);
  i = 0;
  while (name[i] != '\0')
  {
    if (name[i] == ' ' || name[i] == '\t' || name[i] == '\n')
      name[i] = SIGN_UNDERSCORE;
    i++;
  }  
  
  Verbose (DEBUG_STACK, "<-- dot_make_name");
  return name;
}


/*===*/
char *dot_make_label (stg_t *stg)
{
  Verbose (DEBUG_STACK, "--> dot_make_label");
  char *label  = NULL;
  char *label_line  = NULL;
  char *label_line_item  = NULL;
  bool label_printed;
  PNode node;
  
  label = (char *) malloc (strlen (LABEL_NAME) + strlen (stg->model_name) + 1);
  strcpy (label, LABEL_NAME);
  strcat (label, stg->model_name);

  label_printed = false;
  node = stg->s;
  while (node != NULL)
  {
    if (node->data->type == STG_SIGNAL_INPUT)
    {
      if (!label_printed) 
      {
        label_line = (char *) realloc (label_line, strlen (LABEL_INPUTS) + 1);
        strcpy (label_line, LABEL_INPUTS);
        label_printed = true;
      }
      else strcat (label_line, LABEL_DELIMITER);
      label_line_item = dot_cnv_print (node->data->name);
      label_line = (char *) realloc (label_line, strlen (label_line) + strlen (label_line_item) + strlen (LABEL_DELIMITER) + 1);
      strcat (label_line, label_line_item);
      free (label_line_item);
      label_line_item = NULL;
    }  
    node = node->next;
  }
  if (label_printed) 
  {
    label = (char *) realloc (label, strlen (label) + strlen (label_line) + 3);
    strcat (label, "\\n");
    strcat (label, label_line);
    free (label_line);
    label_line = NULL;
  }

  label_printed = false;
  node = stg->s;
  while (node != NULL)
  {
    if (node->data->type == STG_SIGNAL_OUTPUT)
    {
      if (!label_printed) 
      {
        label_line = (char *) realloc (label_line, strlen (LABEL_OUTPUTS) + 1);
        strcpy (label_line, LABEL_OUTPUTS);
        label_printed = true;
      }
      else strcat (label_line, LABEL_DELIMITER);
      label_line_item = dot_cnv_print (node->data->name);
      label_line = (char *) realloc (label_line, strlen (label_line) + strlen (label_line_item) + strlen (LABEL_DELIMITER) + 1);
      strcat (label_line, label_line_item);
      free (label_line_item); 
      label_line_item = NULL;
    }  
    node = node->next;
  }
  if (label_printed) 
  {
    label = (char *) realloc (label, strlen (label) + strlen (label_line) + 3);
    strcat (label, "\\n");
    strcat (label, label_line);
    free (label_line);
    label_line = NULL;
  }

  label_printed = false;
  node = stg->s;
  while (node != NULL)
  {
    if (node->data->type == STG_SIGNAL_INTERNAL)
    {
      if (!label_printed) 
      {
        label_line = (char *) realloc (label_line, strlen (LABEL_INTERNAL) + 1);
        strcpy (label_line, LABEL_INTERNAL);
        label_printed = true;
      }
      else strcat (label_line, LABEL_DELIMITER);
      label_line_item = dot_cnv_print (node->data->name);
      label_line = (char *) realloc (label_line, strlen (label_line) + strlen (label_line_item) + strlen (LABEL_DELIMITER) + 1);
      strcat (label_line, label_line_item);
      free (label_line_item);
      label_line_item = NULL;
    }  
    node = node->next;
  }
  if (label_printed) 
  {
    label = (char *) realloc (label, strlen (label) + strlen (label_line) + 3);
    strcat (label, "\\n");
    strcat (label, label_line);
    free (label_line);
    label_line = NULL;
  }

  label_printed = false;
  node = stg->s;
  while (node != NULL)
  {
    if (node->data->type == STG_SIGNAL_DUMMY)
    {
      if (!label_printed) 
      {
        label_line = (char *) realloc (label_line, strlen (LABEL_DUMMY) + 1);
        strcpy (label_line, LABEL_DUMMY);
        label_printed = true;
      }
      else strcat (label_line, LABEL_DELIMITER);
      label_line_item = dot_cnv_print (node->data->name);
      label_line = (char *) realloc (label_line, strlen (label_line) + strlen (label_line_item) + strlen (LABEL_DELIMITER) + 1);
      strcat (label_line, label_line_item);
      free (label_line_item);
      label_line_item = NULL;
    }
    node = node->next;
  }
  if (label_printed)
  {
    label = (char *) realloc (label, strlen (label) + strlen (label_line) + 3);
    strcat (label, "\\n");
    strcat (label, label_line);
    free (label_line);
    label_line = NULL;
  }

  Verbose (DEBUG_STACK, "<-- dot_make_label");
  return label;
}


/*===*/
int dot_write (FILE *outfile, stg_t *stg, int legend, int connect, int inputs)
{
  Verbose (DEBUG_STACK, "--> dot_write");

  PNode node, node_;
  char *indent_str  = NULL;
  int claster_num   = 0;
  int cluster_index = 0;
  int item_index    = 0;
  bool signal_input = false;
   
  indent_change (indent_str, 0);
 
  Verbose (DEBUG_PROC, "DOT WRITE DIGRAPH");
  fprintf (outfile, "%s%s %s\n", indent_str, WORD_DIGRAPH, dot_make_name (stg));
  Verbose (DEBUG_PROC, "DOT WRITE {");
  fprintf (outfile, "%s%s\n", indent_str, WORD_STATEMENT_BEGIN);
  indent_change (indent_str, +1);

  fprintf (outfile, "%s%s=%s;\n", indent_str, WORD_CENTER, "1");
  fprintf (outfile, "%s%s=%s;\n", indent_str, WORD_MARGIN, "0");
//  fprintf (outfile, "%s%s=%s;\n", indent_str, WORD_RATIO, "compressls");
  if (legend > 0)
  {
    fprintf (outfile, "%s%s=\"%s\";\n", indent_str, WORD_LABEL, dot_make_label (stg));
  }
  fprintf (outfile, "%s%s=%s;\n", indent_str, WORD_COLOR, COLOR_WHITE);
  fprintf (outfile, "%s\n", indent_str);
  
  claster_num = stg_cluster (stg);
  Verbose (DEBUG_PROC, "DOT CLASTERS WRITE BEGIN");
  for (cluster_index = 0; cluster_index < claster_num; cluster_index++)
  {
    signal_input = true;
    node = stg->t;
    while (node != NULL)
    {
      if (node->data->index_1 == cluster_index)
        if ((node->data->type & STG_TRANSITION_INPUT) != STG_TRANSITION_INPUT)
        {
          signal_input = false;
          break;
        }
      node = node->next;
    }  

    if (inputs > 0 || !signal_input)
    {
      Verbose (DEBUG_PROC, "DOT CLASTER %d BEGIN", cluster_index);
      Verbose (DEBUG_PROC, "DOT WRITE SUBGRAPH");
      fprintf (outfile, "%s%s %s_%d\n", indent_str, WORD_SUBGRAPH, WORD_CLUSTER, cluster_index);
      Verbose (DEBUG_PROC, "DOT WRITE {");
      fprintf (outfile, "%s%s\n", indent_str, WORD_STATEMENT_BEGIN);
      indent_change (indent_str, +1);
      fprintf (outfile, "%s%s=%s;\n", indent_str, WORD_LABEL, LABEL_EMPTY);
      fprintf (outfile, "%s%s=%s;\n", indent_str, WORD_COLOR, COLOR_WHITE);
      fprintf (outfile, "%s\n", indent_str);

      Verbose (DEBUG_PROC, "DOT CLASTERS CONSUMING ARCS BEGIN");
      if (signal_input)
        fprintf (outfile, "%s%s %s%s%s;\n", indent_str, WORD_EDGE, WORD_ATTRIB_BEGIN, ATTRIB_ARC_CONSUMING_INPUT, WORD_ATTRIB_END);
      else  
        fprintf (outfile, "%s%s %s%s%s;\n", indent_str, WORD_EDGE, WORD_ATTRIB_BEGIN, ATTRIB_ARC_CONSUMING, WORD_ATTRIB_END);
      
      node = stg->a;
      while (node != NULL)
      {
        if ((node->data->type & STG_ARC_CONSUMING) == STG_ARC_CONSUMING)
          if (node->data->pred->data->index_1 == cluster_index)
            fprintf (outfile, "%s%s %s %s;\n", indent_str, stg_cnv_var (node->data->pred->data->name), WORD_ARC, stg_cnv_var (node->data->succ->data->name));
        node = node->next;
      }
      fprintf (outfile, "%s\n", indent_str);
      Verbose (DEBUG_PROC, "DOT CLASTERS CONSUMING ARCS END");

      if (connect == 0)
      {
        Verbose (DEBUG_PROC, "DOT CLASTERS WRITE READING ARCS BEGIN");
        if (signal_input)
          fprintf (outfile, "%s%s %s%s%s;\n", indent_str, WORD_EDGE, WORD_ATTRIB_BEGIN, ATTRIB_ARC_READING_INPUT, WORD_ATTRIB_END);
        else
          fprintf (outfile, "%s%s %s%s%s;\n", indent_str, WORD_EDGE, WORD_ATTRIB_BEGIN, ATTRIB_ARC_READING, WORD_ATTRIB_END);

        item_index = 0;
        node = stg->a;
        while (node != NULL)
        {
          if ((node->data->type &STG_ARC_READING) == STG_ARC_READING)
            if (node->data->succ->data->index_1 == cluster_index)
            {
              fprintf (outfile, "%s%s_c%.2d_i%.2d %s %s;\n", indent_str, stg_cnv_var (node->data->pred->data->name), cluster_index, item_index, WORD_ARC, stg_cnv_var (node->data->succ->data->name));
              if ((node->data->pred->data->type & STG_PLACE_INPUT) == STG_PLACE_INPUT)
                fprintf (outfile, "%s%s_c%.2d_i%.2d %s%s=%s, %s%s;\n", indent_str, stg_cnv_var (node->data->pred->data->name), cluster_index, item_index, WORD_ATTRIB_BEGIN, WORD_LABEL, dot_cnv_label (node->data->pred->data->name), ATTRIB_PLACE_COPY_INPUT, WORD_ATTRIB_END);
              else
              if ((node->data->pred->data->type & STG_PLACE_OUTPUT) == STG_PLACE_OUTPUT)
                fprintf (outfile, "%s%s_c%.2d_i%.2d %s%s=%s, %s%s;\n", indent_str, stg_cnv_var (node->data->pred->data->name), cluster_index, item_index, WORD_ATTRIB_BEGIN, WORD_LABEL, dot_cnv_label (node->data->pred->data->name), ATTRIB_PLACE_COPY_OUTPUT, WORD_ATTRIB_END);
              else
                fprintf (outfile, "%s%s_c%.2d_i%.2d %s%s=%s, %s%s;\n", indent_str, stg_cnv_var (node->data->pred->data->name), cluster_index, item_index, WORD_ATTRIB_BEGIN, WORD_LABEL, dot_cnv_label (node->data->pred->data->name), ATTRIB_PLACE_COPY, WORD_ATTRIB_END);
              item_index++;
            }
          node = node->next;
        }
        fprintf (outfile, "%s\n", indent_str);
        Verbose (DEBUG_PROC, "DOT CLASTERS WRITE READING ARCS END");
      }
    
      Verbose (DEBUG_PROC, "  DOT CLASTERS PLACES & TRANSITIONS BEGIN");
      node = stg->p;
      while (node != NULL)
      {
        if (node->data->index_1 == cluster_index)
        {
          if (node->data->link == NULL)
          {
            if ((node->data->type & STG_REDUNDANT) == STG_REDUNDANT)
              fprintf (outfile, "%s%s %s%s=%s, %s%s;\n", indent_str, stg_cnv_var (node->data->name), WORD_ATTRIB_BEGIN, WORD_LABEL, dot_cnv_label (node->data->name), ATTRIB_PLACE_REDUNDANT_EMPTY, WORD_ATTRIB_END);
            else
              fprintf (outfile, "%s%s %s%s=%s, %s%s;\n", indent_str, stg_cnv_var (node->data->name), WORD_ATTRIB_BEGIN, WORD_LABEL, dot_cnv_label (node->data->name), ATTRIB_PLACE_EMPTY, WORD_ATTRIB_END);
          }
          else
          {
            if ((node->data->type & STG_REDUNDANT) == STG_REDUNDANT)
              fprintf (outfile, "%s%s %s%s=%s, %s%s;\n", indent_str, stg_cnv_var (node->data->name), WORD_ATTRIB_BEGIN, WORD_LABEL, dot_cnv_label (node->data->name), ATTRIB_PLACE_REDUNDANT_MARKED, WORD_ATTRIB_END);
            else
              fprintf (outfile, "%s%s %s%s=%s, %s%s;\n", indent_str, stg_cnv_var (node->data->name), WORD_ATTRIB_BEGIN, WORD_LABEL, dot_cnv_label (node->data->name), ATTRIB_PLACE_MARKED, WORD_ATTRIB_END);
          }
        }
        node = node->next;
      }
    
      node = stg->t;
      while (node != NULL)
      {
        if (node->data->index_1 == cluster_index)
        {
          if ((node->data->type & STG_TRANSITION_INPUT) == STG_TRANSITION_INPUT)
            fprintf (outfile, "%s%s %s%s=%s, %s%s;\n", indent_str, stg_cnv_var (node->data->name), WORD_ATTRIB_BEGIN, WORD_LABEL, dot_cnv_label (node->data->name), ATTRIB_TRANSITION_INPUT, WORD_ATTRIB_END);
          else  
          if ((node->data->type & STG_TRANSITION_OUTPUT) == STG_TRANSITION_OUTPUT)
            fprintf (outfile, "%s%s %s%s=%s, %s%s;\n", indent_str, stg_cnv_var (node->data->name), WORD_ATTRIB_BEGIN, WORD_LABEL, dot_cnv_label (node->data->name), ATTRIB_TRANSITION_OUTPUT, WORD_ATTRIB_END);
          else  
            fprintf (outfile, "%s%s %s%s=%s, %s%s;\n", indent_str, stg_cnv_var (node->data->name), WORD_ATTRIB_BEGIN, WORD_LABEL, dot_cnv_label (node->data->name), ATTRIB_TRANSITION, WORD_ATTRIB_END);
        }
        node = node->next;
      }
      Verbose (DEBUG_PROC, "  DOT CLASTERS PLACES & TRANSITIONS END");

      indent_change (indent_str, -1);
      Verbose (DEBUG_PROC, "DOT WRITE }");
      fprintf (outfile, "%s%s\n", indent_str, WORD_STATEMENT_END);
      fprintf (outfile, "%s\n", indent_str);
      Verbose (DEBUG_PROC, "  DOT CLASTER %d END", cluster_index);
    }
  }
  Verbose (DEBUG_PROC, "DOT CLASTERS WRITE END");

  if (connect > 0)
  {
    item_index = 0;
    Verbose (DEBUG_PROC, "DOT WRITE READING ARCS BEGIN");
    node = stg->a;
    while (node != NULL)
    {
      if ((node->data->type &STG_ARC_READING) == STG_ARC_READING)
      {
        signal_input = ((node->data->succ->data->type & STG_TRANSITION_INPUT) == STG_TRANSITION_INPUT);
        if (inputs > 0)
        {
          if (signal_input)
            fprintf (outfile, "%s%s %s %s %s%s%s\n", indent_str, stg_cnv_var (node->data->pred->data->name), WORD_ARC, stg_cnv_var (node->data->succ->data->name), WORD_ATTRIB_BEGIN, ATTRIB_ARC_READING_INPUT, WORD_ATTRIB_END);
          else
            fprintf (outfile, "%s%s %s %s %s%s%s\n", indent_str, stg_cnv_var (node->data->pred->data->name), WORD_ARC, stg_cnv_var (node->data->succ->data->name), WORD_ATTRIB_BEGIN, ATTRIB_ARC_READING, WORD_ATTRIB_END);
        }
        else
        {
          // if inputs are hide and there is need to build an arc from it...
          if (!signal_input)
          {
            cluster_index = node->data->pred->data->index_1;
            signal_input = true;
            node_ = stg->t;
            while (node_ != NULL)
            {
              if (node_->data->index_1 == cluster_index)
                if ((node_->data->type & STG_TRANSITION_INPUT) != STG_TRANSITION_INPUT)
                {
                  signal_input = false;
                  break;
                }
              node_ = node_->next;
            }
            
            if (signal_input)
            {
              fprintf (outfile, "%s%s_c%.2d_i%.2d %s %s %s%s%s;\n", indent_str, stg_cnv_var (node->data->pred->data->name), cluster_index, item_index, WORD_ARC, stg_cnv_var (node->data->succ->data->name), WORD_ATTRIB_BEGIN, ATTRIB_ARC_READING, WORD_ATTRIB_END);
              fprintf (outfile, "%s%s_c%.2d_i%.2d %s%s=%s, %s%s;\n", indent_str, stg_cnv_var (node->data->pred->data->name), cluster_index, item_index, WORD_ATTRIB_BEGIN, WORD_LABEL, dot_cnv_label (node->data->pred->data->name), ATTRIB_PLACE_COPY, WORD_ATTRIB_END);
              item_index++;
            }
            else
              fprintf (outfile, "%s%s %s %s %s%s%s\n", indent_str, stg_cnv_var (node->data->pred->data->name), WORD_ARC, stg_cnv_var (node->data->succ->data->name), WORD_ATTRIB_BEGIN, ATTRIB_ARC_READING, WORD_ATTRIB_END);
          }
        }
      }
      node = node->next;
    }
    Verbose (DEBUG_PROC, "DOT WRITE READING ARCS END");
  }

  indent_change (indent_str, -1);
  Verbose (DEBUG_PROC, "DOT WRITE }");
  fprintf (outfile, "%s%s\n", indent_str, WORD_STATEMENT_END);
  fprintf (outfile, "%s\n", indent_str);
  
  free (indent_str);
  Verbose (DEBUG_STACK, "<-- dot_write");
  return 1;
}


/*===*/
int dot_execute (stg_t *stg)
{
/*
  FILE *ftmp;
  char command[128], filename[128];
  int code;

  tmpnam (filename);
  if ((ftmp = fopen (filename, "w")) == NULL) 
  {
    Fatal("opening tmp file <%s>", filename);
  }
  
  int old_debug_level = _debug_level;
  _debug_level = 0;
  dot_write (ftmp, stg);
  _debug_level = old_debug_level;
  
  fclose (ftmp);
  sprintf (command, "dot %s -Tps -o a.ps|gv a.ps", filename);

  code = system (command);
  unlink (filename);
  if (code != 0) 
    Fatal("dot could not be executed succesfully");
*/
  return 1;
}
