# /* Version 3.3    Last modified: December 26, 1995 */

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
#CFLAGS = -Wall -O2

# Options for Sun C compiler
#CC= cc
#CFLAGS = -O2 -Xc -D__sun


# objects needed for decoder

OBJECTS = \
 basic_op.o\
 bits.o\
 decoder.o\
 de_acelp.o\
 dec_gain.o\
 dec_lag3.o\
 dec_ld8k.o\
 dspfunc.o\
 filter.o\
 gainpred.o\
 lpcfunc.o\
 lspdec.o\
 lspgetq.o\
 oper_32b.o\
 p_parity.o\
 post_pro.o\
 pred_lt3.o\
 pst.o\
 tab_ld8k.o\
 util.o

# linker
decoder : $(OBJECTS)
	$(CC) -g -o decoder $(OBJECTS)

# Dependencies for each routine

basic_op.o : basic_op.c typedef.h basic_op.h 
	$(CC) $(CFLAGS) -c  basic_op.c

bits.o : bits.c typedef.h ld8k.h tab_ld8k.h
	$(CC) $(CFLAGS) -c  bits.c

decoder.o : decoder.c typedef.h basic_op.h  ld8k.h
	$(CC) $(CFLAGS) -c decoder.c

de_acelp.o : de_acelp.c typedef.h basic_op.h  ld8k.h
	$(CC) $(CFLAGS) -c de_acelp.c

dec_gain.o : dec_gain.c typedef.h basic_op.h  ld8k.h tab_ld8k.h
	$(CC) $(CFLAGS) -c dec_gain.c

dec_lag3.o : dec_lag3.c typedef.h basic_op.h  ld8k.h
	$(CC) $(CFLAGS) -c dec_lag3.c

dec_ld8k.o : dec_ld8k.c typedef.h basic_op.h  ld8k.h 
	$(CC) $(CFLAGS) -c dec_ld8k.c

dspfunc.o : dspfunc.c typedef.h basic_op.h  ld8k.h tab_ld8k.h
	$(CC) $(CFLAGS) -c  dspfunc.c

filter.o : filter.c typedef.h basic_op.h  ld8k.h
	$(CC) $(CFLAGS) -c  filter.c

gainpred.o : gainpred.c typedef.h basic_op.h ld8k.h  tab_ld8k.h oper_32b.h
	$(CC) $(CFLAGS) -c  gainpred.c

lpcfunc.o : lpcfunc.c typedef.h basic_op.h oper_32b.h ld8k.h  tab_ld8k.h
	$(CC) $(CFLAGS) -c  lpcfunc.c

lspdec.o : lspdec.c typedef.h basic_op.h ld8k.h  tab_ld8k.h
	$(CC) $(CFLAGS) -c  lspdec.c

lspgetq.o : lspgetq.c typedef.h basic_op.h ld8k.h  
	$(CC) $(CFLAGS) -c  lspgetq.c

oper_32b.o : oper_32b.c typedef.h basic_op.h  oper_32b.h
	$(CC) $(CFLAGS) -c  oper_32b.c

p_parity.o : p_parity.c typedef.h basic_op.h  ld8k.h
	$(CC) $(CFLAGS) -c  p_parity.c

post_pro.o : post_pro.c typedef.h basic_op.h  ld8k.h tab_ld8k.h oper_32b.h
	$(CC) $(CFLAGS) -c post_pro.c

pred_lt3.o : pred_lt3.c typedef.h basic_op.h  ld8k.h tab_ld8k.h
	$(CC) $(CFLAGS) -c  pred_lt3.c

pst.o : pst.c typedef.h ld8k.h basic_op.h oper_32b.h 
	$(CC) $(CFLAGS) -c pst.c

tab_ld8k.o : tab_ld8k.c typedef.h ld8k.h tab_ld8k.h
	$(CC) $(CFLAGS) -c  tab_ld8k.c

util.o : util.c typedef.h ld8k.h  basic_op.h
	$(CC) $(CFLAGS) -c  util.c





