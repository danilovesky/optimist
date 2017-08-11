#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "debug.h"
#include "strutils.h"
#include "element.h"
#include "verilog.h"
#include "oml_procs.h"


/* === */
int oml_check (char *name)
{
  Verbose (DEBUG_STACK, "--> oml_check");
  char *type      = NULL;
  char *req_set   = NULL;
  char *ack_reset = NULL;
  int result      = -1;

  if (el_chk (name))  
  {
    result = 0;
    type = el_get_logic (name, EL_TYPE);
    req_set = el_get_logic (name, EL_REQ_SET);
    ack_reset = el_get_logic (name, EL_ACK_RESET);
 
    if (!el_chk_type (type)) 
      result = 1;
    else
    if (!el_chk_logic (req_set))
      result = 2;
    else
    if (!el_chk_logic (ack_reset)) 
      result = 3;
 
    free (type);
    free (req_set);
    free (ack_reset);
  }
  Verbose (DEBUG_STACK, "<-- oml_check");
  return result;
}


/* === */
int oml_pin_write (FILE *outfile, char *name)
{
  Verbose (DEBUG_STACK, "--> oml_pin_write");
  int i, index_in;
  char *type      = NULL;
  char *req_set   = NULL;
  char *ack_reset = NULL;

  type = el_get_logic (name, EL_TYPE);
  req_set = el_get_logic (name, EL_REQ_SET);
  ack_reset = el_get_logic (name, EL_ACK_RESET);
 
  if (strcmp (type, NAME_DC) == 0)
  {
    i=0; index_in = 0;
    while (i < strlen (req_set))
    {
      if (req_set[i++] == NAME_INPUT)
        fprintf (outfile, "%s := %s, %s, %s%d\n", WORD_PIN_PLACEMENT, name, WORD_LEFT, NAME_REQ_IN, ++index_in);
    }
    fprintf (outfile, "%s := %s, %s, %s\n", WORD_PIN_PLACEMENT, name, WORD_LEFT, NAME_ACK_OUT);
    fprintf (outfile, "%s := %s, %s, %s\n", WORD_PIN_PLACEMENT, name, WORD_RIGHT, NAME_REQ_OUT);
    i=0; index_in = 0;
    while (i < strlen (ack_reset))
    {
      if (ack_reset[i++] == NAME_INPUT)
        fprintf (outfile, "%s := %s, %s, %s%d\n", WORD_PIN_PLACEMENT, name, WORD_RIGHT, NAME_ACK_IN, ++index_in);
    }
  }
  else
  if (strcmp (type, NAME_FF) == 0)
  {
    i=0; index_in = 0;
    while (i < strlen (req_set))
    {
      if (req_set[i++] == NAME_INPUT)
        fprintf (outfile, "%s := %s, %s, %s%d\n", WORD_PIN_PLACEMENT, name, WORD_LEFT, NAME_SET, ++index_in);
    }
    i=0; index_in = 0;
    while (i < strlen (ack_reset))
    {
      if (ack_reset[i++] == NAME_INPUT)
        fprintf (outfile, "%s := %s, %s, %s%d\n", WORD_PIN_PLACEMENT, name, WORD_LEFT, NAME_RESET, ++index_in);
    }
    fprintf (outfile, "%s := %s, %s, %s\n", WORD_PIN_PLACEMENT, name, WORD_RIGHT, NAME_OUT_1);
    fprintf (outfile, "%s := %s, %s, %s\n", WORD_PIN_PLACEMENT, name, WORD_RIGHT, NAME_OUT_0);
  }
  
  free (type);
  free (req_set);
  free (ack_reset);
  Verbose (DEBUG_STACK, "<-- oml_pin_write");
  return 0;
}


/* === */
int oml_gate_write (FILE *outfile, char *name)
{
  Verbose (DEBUG_STACK, "--> oml_gate_write");
  Message ("Generation of the gate-level netlist is not implemented yet.");
  Verbose (DEBUG_STACK, "<-- oml_gate_write");
  return 1;
}


/* === */
int oml_transistor_prototipes_write (FILE *outfile)
{
  Verbose (DEBUG_STACK, "--> oml_transistor_prototipes_write");

  //PMOS prototype
  fprintf (outfile, "%s\n", PROTOTYPE_PMOS);

  //NMOS prototype
  fprintf (outfile, "%s\n", PROTOTYPE_NMOS);

  //KEEPER prototype
  fprintf (outfile, "%s\n", PROTOTYPE_KEEPER);

  Verbose (DEBUG_STACK, "<-- oml_transistor_prototipes_write");
  return 1;
}


/* === */
int oml_transistor_logic_write (FILE *outfile, int type, char *logic, int &i, char *B, char *D, char *G, char *S, int &index, char *indent_str)
{
  Verbose (DEBUG_STACK, "--> oml_transistor_logic_write (logic =%s, i = %d, B = %s, D = %s, G = %s, S = %s, index = %d)", logic, i, B, D, G, S, index);
  int j;
  int fanin    = 0;
  char *sD     = NULL;
  char *sS     = NULL;
  
  j = strlen (PORT_MOS_BULK) + strlen (PORT_MOS_DRAIN) + strlen (PORT_MOS_GATE) + strlen (PORT_MOS_SOURCE) + 10;
  if (type == EL_REQ_SET)
    j += strlen (NICK_PMOS);
  else
    j += strlen (NICK_NMOS);
  sD = (char*) malloc (j);
  sS = (char*) malloc (j);
  
  if (logic[i] == NAME_INPUT)
  {
    index++;
    if (type == EL_REQ_SET)
      fprintf (outfile, "%s%s %s%d (%s, %s, %s%d, %s);\n", indent_str, ELEMENT_NMOS, NICK_NMOS, index, B, D, G, index, S);
    else
    if (type == EL_ACK_RESET)
      fprintf (outfile, "%s%s %s%d (%s, %s, %s%d, %s);\n", indent_str, ELEMENT_PMOS, NICK_PMOS, index, B, D, G, index, S);
    i++;
  }
  else
  // OR
  if (logic[i] == NAME_OR)
  {
    i++;
    fanin = 0;
    while (logic[i] >= '0' && logic[i] <= '9') 
    {
      fanin = fanin*10 + logic[i] - '0';
      i++;
    }
    j = 0;
    while (j < fanin)
    {
      oml_transistor_logic_write (outfile, type, logic, i, B, D, G, S, index, indent_str);
      j++;
    }
  }
  else
  // AND, C
  if (logic[i] == NAME_AND || logic[i] == NAME_C)
  {
    i++;
    fanin = 0;
    while (logic[i] >= '0' && logic[i] <= '9') 
    {
      fanin = fanin*10 + logic[i] - '0';
      i++;
    }
    j = 0;
    while (j < fanin)
    {
      j++;
      if (type == EL_REQ_SET)
      {
        if (j == 1)
        {
          sprintf (sS, "%s%d_", NICK_NMOS, index+1);
          oml_transistor_logic_write (outfile, type, logic, i, B, D, G, sS, index, indent_str);
        }
        else
        if (j == fanin)
        {
          sprintf (sD, "%s%d_", NICK_NMOS, index);
          oml_transistor_logic_write (outfile, type, logic, i, B, sD, G, S, index, indent_str);
        }
        else
        {
          sprintf (sD, "%s%d_", NICK_NMOS, index);
          sprintf (sS, "%s%d_", NICK_NMOS, index+1);
          oml_transistor_logic_write (outfile, type, logic, i, B, sD, G, sS, index, indent_str);
        }
      }
      else
      if (type == EL_ACK_RESET)
      {
        if (j == 1)
        {
          sprintf (sS, "%s%d_", NICK_PMOS, index+1);
          oml_transistor_logic_write (outfile, type, logic, i, B, D, G, sS, index, indent_str);
        }
        else
        if (j == fanin)
        {
          sprintf (sD, "%s%d_", NICK_PMOS, index);
          oml_transistor_logic_write (outfile, type, logic, i, B, sD, G, S, index, indent_str);
        }
        else
        {
          sprintf (sD, "%s%d_", NICK_PMOS, index);
          sprintf (sS, "%s%d_", NICK_PMOS, index+1);
          oml_transistor_logic_write (outfile, type, logic, i, B, sD, G, sS, index, indent_str);
        }
      }
    }
  }
  free (sD);
  free (sS);
  Verbose (DEBUG_STACK, "<-- oml_transistor_write");
  return 0;
}


/* === */
int oml_transistor_write (FILE *outfile, char *name)
{
  Verbose (DEBUG_STACK, "--> oml_transistor_write");
  int i, j, k;
  int index_in      = 0;
  char *indent_str  = NULL;
  char *type        = NULL;
  char *req_set     = NULL;
  char *ack_reset   = NULL;

  indent_change (indent_str, 0);

  type = el_get_logic (name, EL_TYPE);
  req_set = el_get_logic (name, EL_REQ_SET);
  ack_reset = el_get_logic (name, EL_ACK_RESET);

  Verbose (DEBUG_PROC, "VERILOG WRITE PROTOTYPES");
  fprintf (outfile, "%s%s\n", indent_str, COMMENT_PROTOTYPES);
  fprintf (outfile, "%s\n", indent_str);
  oml_transistor_prototipes_write (outfile);
  
  Verbose (DEBUG_PROC, "VERILOG WRITE MAIN");
  fprintf (outfile, "%s%s\n", indent_str, COMMENT_MAIN);
  fprintf (outfile, "%s\n", indent_str);
  // DC
  if (strcmp (type, NAME_DC) == 0)
  {
    fprintf (outfile, "%s%s %s (", indent_str, WORD_MODULE_BEGIN, name);
    // Declarations
    // input declarations
    i=0; index_in = 0;
    while (i < strlen (req_set))
    {
      if (req_set[i++] == NAME_INPUT)
        fprintf (outfile, "%s%d, ", NAME_REQ_IN, ++index_in);
    }
    i=0; index_in = 0;
    while (i < strlen (ack_reset))
    {
      if (ack_reset[i++] == NAME_INPUT)
        fprintf (outfile, "%s%d, ", NAME_ACK_IN, ++index_in);
    }
    // output declarations
    fprintf (outfile, "%s, %s);\n", NAME_REQ_OUT, NAME_ACK_OUT);
    indent_change(indent_str, 1);

    // Definitions
    // input definitions
    i=0; index_in = 0;
    while (i < strlen (req_set))
    {
      if (req_set[i++] == NAME_INPUT)
        fprintf (outfile, "%s%s %s%d;\n", indent_str, WORD_INPUT, NAME_REQ_IN, ++index_in);
    }
    i=0; index_in = 0;
    while (i < strlen (ack_reset))
    {
      if (ack_reset[i++] == NAME_INPUT)
        fprintf (outfile, "%s%s %s%d;\n", indent_str, WORD_INPUT, NAME_ACK_IN, ++index_in);
    }
    // output definitions
    fprintf (outfile, "%s%s %s;\n", indent_str, WORD_OUTPUT, NAME_REQ_OUT);
    fprintf (outfile, "%s%s %s;\n", indent_str, WORD_OUTPUT, NAME_ACK_OUT);

    // Wire definitions
    // req_set wire definitions
    i=0; index_in = 0;
    while (i < strlen (req_set))
    {
      if (req_set[i++] == NAME_INPUT)
      {
        if (index_in >=1)
          fprintf (outfile, "%s%s %s%d_;\n", indent_str, WORD_WIRE, NICK_NMOS, index_in);
        index_in++;
      }
    }
    // ack_reset wire definitions
    i=0; index_in = 0;
    while (i < strlen (ack_reset))
    {
      if (ack_reset[i++] == NAME_INPUT)
      {
        if (index_in >=1)
          fprintf (outfile, "%s%s %s%d_;\n", indent_str, WORD_WIRE, NICK_PMOS, index_in);
        index_in++;
      }
    }
    // vdd_gnd wire definitions
    fprintf (outfile, "%s\n", indent_str);

    // Element definitions
    // vdd, gnd, keeper
    fprintf (outfile, "%s%s %s (%s, %s);\n", indent_str, ELEMENT_KEEPER, NICK_KEEPER, NAME_REQ_OUT, NAME_ACK_OUT);
    // req_set logic
    i = 0;
    index_in = 0;
    oml_transistor_logic_write (outfile, EL_REQ_SET, req_set, i, WIRE_VDD, NAME_ACK_OUT, NAME_REQ_IN, WIRE_VDD, index_in, indent_str);
    // ack_reset logic
    i = 0;
    index_in = 0;
    oml_transistor_logic_write (outfile, EL_ACK_RESET, ack_reset, i, WIRE_GND, NAME_ACK_IN, NAME_ACK_OUT, WIRE_GND, index_in, indent_str);
    indent_change(indent_str, -1);
    fprintf (outfile, "%s%s\n", indent_str, WORD_MODULE_END);
  }
  else

  // FF
  if (strcmp (type, NAME_FF) == 0)
  {
    fprintf (outfile, "%s%s %s (", indent_str, WORD_MODULE_BEGIN, name);
    // Declarations
    // input declarations
    i=0; index_in = 0;
    while (i < strlen (req_set))
    {
      if (req_set[i++] == NAME_INPUT)
        fprintf (outfile, "%s%d, ", NAME_SET, ++index_in);
    }
    i=0; index_in = 0;
    while (i < strlen (ack_reset))
    {
      if (ack_reset[i++] == NAME_INPUT)
        fprintf (outfile, "%s%d, ", NAME_RESET, ++index_in);
    }
    // output declarations
    fprintf (outfile, "%s, %s);\n", NAME_OUT_1, NAME_OUT_0);
    indent_change(indent_str, 1);

    // Definitions
    // input definitions
    i=0; index_in = 0;
    while (i < strlen (req_set))
    {
      if (req_set[i++] == NAME_INPUT)
        fprintf (outfile, "%s%s %s%d;\n", indent_str, WORD_INPUT, NAME_SET, ++index_in);
    }
    i=0; index_in = 0;
    while (i < strlen (ack_reset))
    {
      if (ack_reset[i++] == NAME_INPUT)
        fprintf (outfile, "%s%s %s%d;\n", indent_str, WORD_INPUT, NAME_RESET, ++index_in);
    }
    // output definitions
    fprintf (outfile, "%s%s %s;\n", indent_str, WORD_OUTPUT, NAME_OUT_1);
    fprintf (outfile, "%s%s %s;\n", indent_str, WORD_OUTPUT, NAME_OUT_0);

    // Wire definitions
    // req_set wire definitions
    i=0; index_in = 0;
    while (i < strlen (req_set))
    {
      if (req_set[i++] == NAME_INPUT)
      {
        if (index_in >=1)
          fprintf (outfile, "%s%s %s%d_;\n", indent_str, WORD_WIRE, NICK_NMOS, index_in);
        index_in++;
      }
    }
    // ack_reset wire definitions
    i=0; index_in = 0;
    while (i < strlen (ack_reset))
    {
      if (ack_reset[i++] == NAME_INPUT)
      {
        if (index_in >=1)
          fprintf (outfile, "%s%s %s%d_;\n", indent_str, WORD_WIRE, NICK_PMOS, index_in);
        index_in++;
      }
    }
    // vdd_gnd wire definitions
    fprintf (outfile, "%s\n", indent_str);

    // Element definitions
    // vdd, gnd, keeper
    fprintf (outfile, "%s%s %s (%s, %s);\n", indent_str, ELEMENT_KEEPER, NICK_KEEPER, NAME_OUT_1, NAME_OUT_0);
    // req_set logic
    i = 0;
    index_in = 0;
    oml_transistor_logic_write (outfile, EL_REQ_SET, req_set, i, WIRE_VDD, NAME_OUT_0, NAME_SET, WIRE_VDD, index_in, indent_str);
    // ack_reset logic
    i = 0;
    index_in = 0;
    oml_transistor_logic_write (outfile, EL_ACK_RESET, ack_reset, i, WIRE_GND, NAME_OUT_0, NAME_RESET, WIRE_GND, index_in, indent_str);
    indent_change(indent_str, -1);
    fprintf (outfile, "%s%s\n", indent_str, WORD_MODULE_END);
  }
  free (type);
  free (req_set);
  free (ack_reset);
  Verbose (DEBUG_STACK, "<-- oml_transistor_write");
  return 1;
}
