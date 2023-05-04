CC=gcc
CFLAGS=-I. -lstdc++ -ggdb -DMSGDIR_EXTENDED
DEPS = cmss.h linktime.h log.h msgdir.h 
OBJ = cirfile.cc circfile.c  circfint.c  logc.c  log_maketime.cc tokens.c tracec.c alarmc.c	msgdir_getfname.cc \
	dispdir.cc  find_logmsg.cc  getcommstime.cc  logfind.cc  ReadMsgText.cc ReadTextFile.cc \
	RWaddresses.cc ReadMessage.cc cmssparam.cc cmmktime.cc searchin.cc msgdir_getfname.cc getgroup.cc \
	makestring.cc gtime.cc

%.o: %.c %.cc $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

searchmsg: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) 

clean:
	rm -f searchmsg
