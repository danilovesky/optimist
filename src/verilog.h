#ifndef _VERILOG_H_
#define _VERILOG_H_

#define COMMENT_PROTOTYPES   "// Prototypes"
#define COMMENT_MAIN         "// Main part"

#define WORD_MODULE_BEGIN    "module"
#define WORD_MODULE_END      "endmodule"
#define WORD_INPUT           "input"
#define WORD_OUTPUT          "output"
#define WORD_INOUT           "inout"
#define WORD_WIRE            "wire"
#define WORD_PIN_PLACEMENT   "pin_placement"
#define WORD_LEFT            "left"
#define WORD_RIGHT           "right"
#define WORD_TOP             "top"
#define WORD_BOTTOM          "bottom"

#define NICK_INVERTOR        "INV"
#define ELEMENT_INVERTOR     "IN2"
#define PROTOTYPE_INVERTOR   "module IN2 (A, Q);\n  input A;\n  output Q;\nendmodule\n"

#define NICK_VDD             "vdd"
#define ELEMENT_VDD          "vdd"
#define PROTOTYPE_VDD        "module vdd (\\vdd! );\n  inout \\vdd! ;\nendmodule\n"
#define WIRE_VDD             "vdd"

#define NICK_GND             "gnd"
#define ELEMENT_GND          "gnd"
#define PROTOTYPE_GND        "module gnd (\\gnd! );\n  inout \\gnd! ;\nendmodule\n"
#define WIRE_GND             "gnd"

#define NICK_PMOS            "p"
#define ELEMENT_PMOS         "pmos4"
#define PROTOTYPE_PMOS       "module pmos4 (B, D, G, S);\n  inout B;\n  inout D;\n  inout G;\n  inout S;\nendmodule\n"

#define NICK_NMOS            "n"
#define ELEMENT_NMOS         "nmos4"
#define PROTOTYPE_NMOS       "module nmos4 (B, D, G, S);\n  inout B;\n  inout D;\n  inout G;\n  inout S;\nendmodule\n"

#define NICK_KEEPER          "keeper"
#define ELEMENT_KEEPER       "keeper_2"
#define PROTOTYPE_KEEPER     "module keeper_2 (Q, SIG);\n  output Q;\n  inout SIG;\nendmodule\n"

#define PORT_VDD_VDD         "vdd"
#define PORT_GND_GND         "gnd"
#define PORT_MOS_BULK        "B"   // podlogka
#define PORT_MOS_DRAIN       "D"   // stok
#define PORT_MOS_GATE        "G"   // zatvor
#define PORT_MOS_SOURCE      "S"   // stok
#define PORT_KEEPER_Q        "Q"
#define PORT_KEEPER_SIG      "SIG"

#endif
