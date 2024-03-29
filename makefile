# --------------------------------------------------------------------
# Student Names: Jonathan Stregger
# Assignment 3: Makefile
# Lab Section: X01L, Winter 2019
# Lab Instructor’s Name: Ron Meleshko
# CMPT 361, Class Section: AS01, Winter 2019
# Instructor's Name: Ron Meleshko
# --------------------------------------------------------------------

CC = gcc
CFLAGS = -Wall

all: calserv calclient

calserv: calserv.c calserv.h
	$(CC) $(CFLAGS) -o calserv calserv.c calserv.h -lm

calclient: calclient.c calclient.h
	$(CC) $(CFLAGS) -o calclient calclient.c calclient.h -lm 
