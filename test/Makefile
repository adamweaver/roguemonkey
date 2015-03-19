# -*- Mode: Makefile -*-
# RogueMonkey copyright 2007 Adam White spudboy@iinet.net.au
# Released under the GPL version 2 - refer to included file LICENCE.txt

############################################################################
# Compiler Configuration
CXX = g++
LD = g++
OSTYPE = BUILD_LINUX

INCLUDE = -I..
FLAGS = -W -Wall -ansi -pedantic -ggdb3 -O0 -fno-inline -D$(OSTYPE) 
#FLAGS = -W -Wall -ansi -pedantic -O3 -D$(OSTYPE)
LIBS =


BOOST = $(FLAGS) $(INCLUDE) -lboost_test_exec_monitor -lboost_thread


.PHONY : test
test:	netstring dictionary tcp_srv
	@echo "netstring" && ./netstring
	@echo "dictionary" && ./dictionary
	@echo "server" && ./tcp_srv



tcpip.o : ../util/tcpip.h ../util/tcpip_unx.cc 
	$(CXX) ../util/tcpip_unx.cc $(INCLUDE) -I ../ $(FLAGS) -c -o tcpip.o

tcp_srv : tcp_srv.cc ../util/tcpip.h ../util/tcpip_unx.cc tcpip.o
	$(CXX) tcp_srv.cc tcpip.o $(BOOST) -o tcp_srv

dictionary : dictionary.cc ../util/dictionary.h
	$(CXX) dictionary.cc $(BOOST) -o dictionary

netstring : netstring.cc ../util/netstring.h
	$(CXX) netstring.cc $(BOOST) -o netstring

display : display.cc
	$(CXX) display.cc $(BOOST) -o display

clean :
	-@rm dictionary netstring tcp_srv tcpip.o


