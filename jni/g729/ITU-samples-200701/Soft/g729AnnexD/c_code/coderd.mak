# G.729 with ANNEX D   Version 1.3     Last modified: 17 Oct 2000 

# makefile for ANSI-C version of G.729 Annex D 6.4 kbps Speech Coder Extension
# options for ? C compiler
# NOTE: Edit these options to reflect your particular system

#CC= cc
#CFLAGS= -w2 -std

#options for HP C compiler
#CC= c89
#CFLAGS= -O -Aa

# options for SGI C compiler
#CC=cc
#CFLAGS= -O2 -mips2 -float -fullwarn -ansi
#CFLAGS= -g -mips2 -float -fullwarn

# Options for GCC C compiler
CC= gcc
CFLAGS = -O2 -Wall
# CFLAGS = -O2 -Wall -DSYNC

# Options for Sun C compiler
#CC= cc
#CFLAGS = -O2 -Xc -D__sun

# objects needed for encoder

OBJECTS= \
 acelpcod.o\
 basic_op.o\
 bitsd.o\
 codld8kd.o\
 coderd.o\
 dspfunc.o\
 filterd.o\
 gainpred.o\
 lpc.o\
 lpcfunc.o\
 lspgetq.o\
 oper_32b.o\
 p_parity.o\
 pitchd.o\
 pre_proc.o\
 pred_lt3.o\
 pwf.o\
 qua_g6k.o\
 qua_g8k.o\
 qua_lsp.o\
 tab_ld8k.o\
 tabld8kd.o\
 util.o

coderd : $(OBJECTS)
	$(CC) -g -o coderd $(OBJECTS)

# Dependencies for each file

acelpcod.o : acelpcod.c typedef.h basic_op.h ld8k.h ld8kd.h
	$(CC) $(CFLAGS) -c  acelpcod.c

basic_op.o : basic_op.c typedef.h basic_op.h 
	$(CC) $(CFLAGS) -c  basic_op.c

bitsd.o : bitsd.c typedef.h ld8k.h ld8kd.h tab_ld8k.h tabld8kd.h 
	$(CC) $(CFLAGS) -c  bitsd.c

codld8kd.o : codld8kd.c typedef.h basic_op.h  ld8k.h ld8kd.h
	$(CC) $(CFLAGS) -c  codld8kd.c

coderd.o : coderd.c typedef.h basic_op.h  ld8k.h ld8kd.h
	$(CC) $(CFLAGS) -c  coderd.c

dspfunc.o : dspfunc.c typedef.h basic_op.h  ld8k.h tab_ld8k.h
	$(CC) $(CFLAGS) -c  dspfunc.c

filterd.o : filterd.c typedef.h basic_op.h  ld8k.h ld8kd.h\
            tab_ld8k.h tabld8kd.h
	$(CC) $(CFLAGS) -c  filterd.c

gainpred.o : gainpred.c typedef.h basic_op.h ld8k.h  tab_ld8k.h oper_32b.h
	$(CC) $(CFLAGS) -c  gainpred.c

lpc.o : lpc.c typedef.h basic_op.h oper_32b.h ld8k.h  tab_ld8k.h
	$(CC) $(CFLAGS) -c  lpc.c

lpcfunc.o : lpcfunc.c typedef.h basic_op.h oper_32b.h ld8k.h  tab_ld8k.h
	$(CC) $(CFLAGS) -c  lpcfunc.c

lspgetq.o : lspgetq.c typedef.h basic_op.h ld8k.h  
	$(CC) $(CFLAGS) -c  lspgetq.c

oper_32b.o : oper_32b.c typedef.h basic_op.h  oper_32b.h
	$(CC) $(CFLAGS) -c  oper_32b.c

p_parity.o : p_parity.c typedef.h basic_op.h  ld8k.h
	$(CC) $(CFLAGS) -c  p_parity.c

pitchd.o : pitchd.c typedef.h basic_op.h ld8k.h ld8kd.h\
           tab_ld8k.h tabld8kd.h oper_32b.h 
	$(CC) $(CFLAGS) -c  pitchd.c

pre_proc.o : pre_proc.c typedef.h basic_op.h oper_32b.h ld8k.h\
             tab_ld8k.h
	$(CC) $(CFLAGS) -c  pre_proc.c

pred_lt3.o : pred_lt3.c typedef.h basic_op.h  ld8k.h tab_ld8k.h
	$(CC) $(CFLAGS) -c  pred_lt3.c

pwf.o : pwf.c typedef.h basic_op.h  ld8k.h
	$(CC) $(CFLAGS) -c  pwf.c

qua_g6k.o : qua_g6k.c typedef.h basic_op.h oper_32b.h  ld8k.h ld8kd.h\
             tab_ld8k.h tabld8kd.h
	$(CC) $(CFLAGS) -c  qua_g6k.c

qua_g8k.o : qua_g8k.c typedef.h basic_op.h oper_32b.h  ld8k.h\
             tab_ld8k.h
	$(CC) $(CFLAGS) -c  qua_g8k.c

qua_lsp.o : qua_lsp.c typedef.h basic_op.h  ld8k.h tab_ld8k.h
	$(CC) $(CFLAGS) -c  qua_lsp.c

tab_ld8k.o : tab_ld8k.c typedef.h ld8k.h tab_ld8k.h
	$(CC) $(CFLAGS) -c  tab_ld8k.c

tabld8kd.o : tabld8kd.c typedef.h ld8k.h ld8kd.h tab_ld8k.h \
              tabld8kd.h
	$(CC) $(CFLAGS) -c  tabld8kd.c

util.o : util.c typedef.h ld8k.h  basic_op.h 
	$(CC) $(CFLAGS) -c  util.c

