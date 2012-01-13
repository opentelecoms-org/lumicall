#/* Version 3.3    Last modified: December 26, 1995 */

#makefile for ANSI-C version of G.729
#options for ? C compiler
# NOTE: Edit these options to reflect your particular system

#CC= cc
#CFLAGS= -w2 -std

#options for HP C compiler
#CC= c89
#CFLAGS= -O -Aa

# options for SGI C compiler
CC=cc
CFLAGS= -O2 -mips2 -float -fullwarn -ansi
#CFLAGS= -g -mips2 -float -fullwarn

# Options for GCC C compiler
#CC= gcc
#CFLAGS = -O2 -Wall

# Options for Sun C compiler
#CC= cc
#CFLAGS = -O2 -Xc -D__sun

# objects needed for encoder

OBJECTS= \
 acelp_co.o\
 basic_op.o\
 bits.o\
 cod_ld8k.o\
 coder.o\
 dspfunc.o\
 filter.o\
 gainpred.o\
 lpc.o\
 lpcfunc.o\
 lspgetq.o\
 oper_32b.o\
 p_parity.o\
 pitch.o\
 pre_proc.o\
 pred_lt3.o\
 pwf.o\
 qua_gain.o\
 qua_lsp.o\
 tab_ld8k.o\
 util.o

coder :	$(OBJECTS)
	$(CC) -g -o coder $(OBJECTS)

# Dependencies for each file

acelp_co.o : acelp_co.c typedef.h basic_op.h  ld8k.h
	$(CC) $(CFLAGS) -c  acelp_co.c

basic_op.o : basic_op.c typedef.h basic_op.h 
	$(CC) $(CFLAGS) -c  basic_op.c

bits.o : bits.c typedef.h ld8k.h tab_ld8k.h
	$(CC) $(CFLAGS) -c  bits.c

cod_ld8k.o : cod_ld8k.c typedef.h basic_op.h  ld8k.h
	$(CC) $(CFLAGS) -c  cod_ld8k.c

coder.o : coder.c typedef.h basic_op.h  ld8k.h
	$(CC) $(CFLAGS) -c  coder.c

dspfunc.o : dspfunc.c typedef.h basic_op.h  ld8k.h tab_ld8k.h
	$(CC) $(CFLAGS) -c  dspfunc.c

filter.o : filter.c typedef.h basic_op.h  ld8k.h
	$(CC) $(CFLAGS) -c  filter.c

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

pitch.o : pitch.c typedef.h basic_op.h ld8k.h   tab_ld8k.h oper_32b.h
	$(CC) $(CFLAGS) -c  pitch.c

pre_proc.o : pre_proc.c typedef.h basic_op.h oper_32b.h  ld8k.h\
             tab_ld8k.h
	$(CC) $(CFLAGS) -c  pre_proc.c

pred_lt3.o : pred_lt3.c typedef.h basic_op.h  ld8k.h tab_ld8k.h
	$(CC) $(CFLAGS) -c  pred_lt3.c

pwf.o : pwf.c typedef.h basic_op.h  ld8k.h
	$(CC) $(CFLAGS) -c  pwf.c

qua_gain.o : qua_gain.c typedef.h basic_op.h oper_32b.h  ld8k.h\
             tab_ld8k.h
	$(CC) $(CFLAGS) -c  qua_gain.c

qua_lsp.o : qua_lsp.c typedef.h basic_op.h  ld8k.h tab_ld8k.h
	$(CC) $(CFLAGS) -c  qua_lsp.c

tab_ld8k.o : tab_ld8k.c typedef.h ld8k.h tab_ld8k.h
	$(CC) $(CFLAGS) -c  tab_ld8k.c

util.o : util.c typedef.h ld8k.h  basic_op.h
	$(CC) $(CFLAGS) -c  util.c











