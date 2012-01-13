# G.729 with ANNEX B   Version 1.5   Last modified: October 2006 

# makefile for ANSI-C version of G.729 Annex B.
# NOTE: Edit these options to reflect your particular system

# Options for GCC C compiler
CC= gcc
CFLAGS = -O2 -Wall

# macro noms des objets
OBJETS = \
 basic_op.o bits.o decoder.o de_acelp.o dec_gain.o\
 dec_lag3.o dec_ld8k.o dspfunc.o filter.o gainpred.o\
 lpcfunc.o lspdec.o lspgetq.o oper_32b.o p_parity.o\
 post_pro.o pred_lt3.o pst.o tab_ld8k.o util.o\
 taming.o dec_sid.o qsidgain.o calcexc.o tab_dtx.o

# edition de liens
decoder : $(OBJETS)
	$(CC) -o decoder $(OBJETS) -lm

# Compilations si changements

basic_op.o : basic_op.c typedef.h basic_op.h
	$(CC) $(CFLAGS) -c basic_op.c

bits.o : bits.c typedef.h ld8k.h tab_ld8k.h\
    vad.h dtx.h tab_dtx.h octet.h
	$(CC) $(CFLAGS) -c bits.c

decoder.o : decoder.c typedef.h basic_op.h ld8k.h\
    dtx.h octet.h
	$(CC) $(CFLAGS) -c decoder.c

de_acelp.o : de_acelp.c typedef.h basic_op.h ld8k.h
	$(CC) $(CFLAGS) -c de_acelp.c

dec_gain.o : dec_gain.c typedef.h basic_op.h ld8k.h\
    tab_ld8k.h
	$(CC) $(CFLAGS) -c dec_gain.c

dec_lag3.o : dec_lag3.c typedef.h basic_op.h ld8k.h
	$(CC) $(CFLAGS) -c dec_lag3.c

dec_ld8k.o : dec_ld8k.c typedef.h basic_op.h ld8k.h\
    dtx.h sid.h
	$(CC) $(CFLAGS) -c dec_ld8k.c

dspfunc.o : dspfunc.c typedef.h basic_op.h ld8k.h\
    tab_ld8k.h
	$(CC) $(CFLAGS) -c dspfunc.c

filter.o : filter.c typedef.h basic_op.h ld8k.h
	$(CC) $(CFLAGS) -c filter.c

gainpred.o : gainpred.c typedef.h basic_op.h ld8k.h\
    tab_ld8k.h oper_32b.h
	$(CC) $(CFLAGS) -c gainpred.c

lpcfunc.o : lpcfunc.c typedef.h basic_op.h oper_32b.h\
    ld8k.h tab_ld8k.h
	$(CC) $(CFLAGS) -c lpcfunc.c

lspdec.o : lspdec.c typedef.h ld8k.h basic_op.h\
    tab_ld8k.h
	$(CC) $(CFLAGS) -c lspdec.c

lspgetq.o : lspgetq.c typedef.h basic_op.h ld8k.h
	$(CC) $(CFLAGS) -c lspgetq.c

oper_32b.o : oper_32b.c typedef.h basic_op.h oper_32b.h
	$(CC) $(CFLAGS) -c oper_32b.c

p_parity.o : p_parity.c typedef.h basic_op.h ld8k.h
	$(CC) $(CFLAGS) -c p_parity.c

post_pro.o : post_pro.c typedef.h basic_op.h oper_32b.h\
    ld8k.h tab_ld8k.h
	$(CC) $(CFLAGS) -c post_pro.c

pred_lt3.o : pred_lt3.c typedef.h basic_op.h ld8k.h\
    tab_ld8k.h
	$(CC) $(CFLAGS) -c pred_lt3.c

pst.o : pst.c typedef.h ld8k.h basic_op.h\
    oper_32b.h
	$(CC) $(CFLAGS) -c pst.c

tab_ld8k.o : tab_ld8k.c typedef.h ld8k.h tab_ld8k.h
	$(CC) $(CFLAGS) -c tab_ld8k.c

util.o : util.c typedef.h basic_op.h ld8k.h
	$(CC) $(CFLAGS) -c util.c

taming.o : taming.c typedef.h basic_op.h oper_32b.h\
    ld8k.h tab_ld8k.h
	$(CC) $(CFLAGS) -c taming.c

dec_sid.o : dec_sid.c typedef.h ld8k.h tab_ld8k.h\
    basic_op.h vad.h dtx.h sid.h tab_dtx.h
	$(CC) $(CFLAGS) -c dec_sid.c

qsidgain.o : qsidgain.c typedef.h basic_op.h oper_32b.h\
    ld8k.h vad.h dtx.h sid.h tab_dtx.h
	$(CC) $(CFLAGS) -c qsidgain.c

calcexc.o : calcexc.c typedef.h ld8k.h dtx.h\
    basic_op.h oper_32b.h
	$(CC) $(CFLAGS) -c calcexc.c

tab_dtx.o : tab_dtx.c typedef.h ld8k.h vad.h\
    dtx.h tab_dtx.h
	$(CC) $(CFLAGS) -c tab_dtx.c
