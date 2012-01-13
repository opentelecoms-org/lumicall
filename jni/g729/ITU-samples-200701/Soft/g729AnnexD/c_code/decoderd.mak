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
CFLAGS = -Wall -O2

# Options for Sun C compiler
#CC= cc
#CFLAGS = -O2 -Xc -D__sun


# objects needed for decoder

OBJECTS = \
 basic_op.o\
 bitsd.o\
 decoderd.o\
 de_acelp.o\
 deacelpd.o\
 dec_g6k.o\
 dec_g8k.o\
 declag3d.o\
 decld8kd.o\
 dspfunc.o\
 filterd.o\
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
 tabld8kd.o\
 util.o

# linker
decoderd : $(OBJECTS)
	$(CC) -g -o decoderd $(OBJECTS)

# Dependencies for each routine

basic_op.o : basic_op.c typedef.h basic_op.h 
	$(CC) $(CFLAGS) -c  basic_op.c

bitsd.o : bitsd.c typedef.h ld8k.h ld8kd.h tab_ld8k.h tabld8kd.h
	$(CC) $(CFLAGS) -c  bitsd.c

decoderd.o : decoderd.c typedef.h basic_op.h ld8k.h ld8kd.h
	$(CC) $(CFLAGS) -c decoderd.c

de_acelp.o : de_acelp.c typedef.h basic_op.h ld8k.h
	$(CC) $(CFLAGS) -c de_acelp.c

deacelpd.o : deacelpd.c typedef.h basic_op.h ld8k.h ld8kd.h
	$(CC) $(CFLAGS) -c deacelpd.c

dec_g6k.o : dec_g6k.c typedef.h basic_op.h ld8k.h ld8kd.h\
	    tab_ld8k.h tabld8kd.h
	$(CC) $(CFLAGS) -c dec_g6k.c

dec_g8k.o : dec_g8k.c typedef.h basic_op.h ld8k.h tab_ld8k.h
	$(CC) $(CFLAGS) -c dec_g8k.c

declag3d.o : declag3d.c typedef.h basic_op.h ld8k.h ld8kd.h
	$(CC) $(CFLAGS) -c declag3d.c

decld8kd.o : decld8kd.c typedef.h basic_op.h ld8k.h ld8kd.h
	$(CC) $(CFLAGS) -c decld8kd.c

dspfunc.o : dspfunc.c typedef.h basic_op.h ld8k.h tab_ld8k.h
	$(CC) $(CFLAGS) -c  dspfunc.c

filterd.o : filterd.c typedef.h basic_op.h ld8k.h ld8kd.h\
            tab_ld8k.h tabld8kd.h  
	$(CC) $(CFLAGS) -c  filterd.c

gainpred.o : gainpred.c typedef.h basic_op.h ld8k.h tab_ld8k.h oper_32b.h
	$(CC) $(CFLAGS) -c  gainpred.c

lpcfunc.o : lpcfunc.c typedef.h basic_op.h oper_32b.h ld8k.h tab_ld8k.h
	$(CC) $(CFLAGS) -c  lpcfunc.c

lspdec.o : lspdec.c typedef.h basic_op.h ld8k.h tab_ld8k.h
	$(CC) $(CFLAGS) -c  lspdec.c

lspgetq.o : lspgetq.c typedef.h basic_op.h ld8k.h  
	$(CC) $(CFLAGS) -c  lspgetq.c

oper_32b.o : oper_32b.c typedef.h basic_op.h oper_32b.h
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

tabld8kd.o : tabld8kd.c typedef.h ld8k.h ld8kd.h\
              tab_ld8k.h tabld8kd.h
	$(CC) $(CFLAGS) -c  tabld8kd.c

util.o : util.c typedef.h ld8k.h  basic_op.h
	$(CC) $(CFLAGS) -c  util.c

