#include <stdlib.h>
#include <string.h>
#include "debug.h"
#include "element.h"


/* === */
bool el_chk (char *name)
{
  Verbose (DEBUG_STACK, "--> el_chk");
  int i       = 0;
  int sep_num = 0;
  int space_num = 0;
  if (name != NULL)
  {
    for (i = 0; i < strlen (name); i++)
    {
      if (name[i] == PATTERN_SEP)
        sep_num++;
      if (name[i] == ' ')
        space_num++;
    }
  }
  Verbose (DEBUG_STACK, "<-- el_chk");
  return (sep_num == 2 && space_num == 0);
}


/* === */
bool el_chk_type (char *name)
{
  Verbose (DEBUG_STACK, "--> el_chk_type");
  bool result = false;
  if (name != NULL)
  {
    if (strcmp (name, NAME_DC) == 0) 
      result = true;
    else
    if (strcmp (name, NAME_FF) == 0) 
      result = true;
  }
  Verbose (DEBUG_STACK, "<-- el_chk_type");
  return result;
}


/* === */
bool el_chk_term (char *name, int &i)
{
  Verbose (DEBUG_STACK, "--> el_chk_term");
  int len;
  int fanin;
  bool result = true;

  len = strlen (name);
  if (i >= len) result = false;
  fanin = 0;
  switch (name[i])
  {
    case NAME_OR:
    case NAME_AND:
    case NAME_C:
      i++;
      while (name[i] >= '0' && name[i] <= '9') 
      {
        fanin = fanin*10 + name[i] - '0';
        i++;
      }
      if (fanin <= 0) result = false;

      while (result && name[i] == NAME_INPUT)
      {
        fanin--;
        if (fanin < 0) result = false;
        i++;
      }
          
      while (result && fanin > 0) 
      {
        fanin--;
        result = el_chk_term (name, i);
      }
      break;

    case NAME_INPUT:
      i++;
      if (i < len) result = false;
      break;
     
    default:
      result = false;
      break;
  }
  Verbose (DEBUG_STACK, "<-- el_chk_term");
  return result;
}


/* === */
bool el_chk_logic (char *name)
{
  Verbose (DEBUG_STACK, "--> el_chk_logic");
  int i = 0;
  bool result = false;
  if (name != NULL)
  {
    result = el_chk_term (name, i);
    if (result)
      result = (i == strlen (name));
  };
  Verbose (DEBUG_STACK, "<-- el_chk_logic");
  return result;
}


/* === */
char *el_get_logic (char *name, int input_type)
{
  Verbose (DEBUG_STACK, "--> el_get_logic (name = %s, input_type = %d)", name, input_type);
  char *s      = NULL;
  char *start  = NULL;
  int len      = 0;
  
  if (input_type == EL_REQ_SET)
    start = strchr (name, PATTERN_SEP) + 1;
  else
  if (input_type == EL_ACK_RESET)
    start = strrchr (name, PATTERN_SEP) + 1;
  else
    start = name;
  
  if (start != NULL)
  {
    len = 0;
    while (start[len] != PATTERN_SEP && start[len] != '\0') 
      len++;
    s = (char *) malloc (len + 1);
    strncpy (s, start, len);
    s[len] = '\0';
  }
  
  Verbose (DEBUG_STACK, "<-- el_get_logic (result = %s)", s);
  return s;
}


/* === */
int el_count_trans (char *name, int input_type)
{
  Verbose (DEBUG_STACK, "--> el_count_trans");
  char *logic  = NULL;
  int count    = 0;
  int i        = 0;
  int len;
  
  if ((logic = el_get_logic (name, input_type)) != NULL)
  {
    len = strlen (logic);
    for (i = 0; i < len; i++)
      if (logic[i] == NAME_INPUT)
        count++;
  
    free (logic);
  }
  Verbose (DEBUG_STACK, "<-- el_count_trans");
  return count;
}


/* === */
int el_count_fanin (char *name, int input_type)
{
  Verbose (DEBUG_STACK, "--> el_count_fanin");
  char *logic   = NULL;
  int fanin     = 0;
  int fanin_max = 1;
  int i, dum, len;
  
  if ((logic = el_get_logic (name, input_type)) != NULL)
  {
    len = strlen (logic);
    i = 0;
    while (i < len)
    {
      fanin = 0;
    switch (logic[i])
    {
      case NAME_AND:
        fanin = 0;
        i++;
        dum = atoi (&logic[i]);
        while (logic[i] >= '0' && logic[i] <= '9') i++;
        while (logic[i] == NAME_INPUT)
        {
          fanin++;
          dum--;
          i++;
        }
            
        while (dum > 0)
        {
          if (logic[i] == NAME_C)
          {
            dum--;
            i++;
            fanin += atoi (&logic[i]);
            while (logic[i] >= '0' && logic[i] <= '9') i++;
          }
          else
            i++;
        }
        break;

      case NAME_C:
        i++;
        dum = atoi (&logic[i]);
        while (logic[i] >= '0' && logic[i] <= '9') i++;
        while (logic[i] == NAME_INPUT)
        {
          fanin++;
          dum--;
          i++;
        }
            
        while (dum > 0)
        {
          if (logic[i] == NAME_AND)
          {
            dum--;
            i++;
            fanin += atoi (&logic[i]);
            while (logic[i] >= '0' && logic[i] <= '9') i++;
          }
          else
            i++;
        }
        break;
      }
      if (fanin > fanin_max)
        fanin_max = fanin;
      i++;
    }
    free (logic);
  }
  Verbose (DEBUG_STACK, "<-- el_count_fanin");
  return fanin_max;
}
