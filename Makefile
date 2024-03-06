ifdef AAA
OUT = aaa
endif

ifdef BBB
OUT = bbb
endif


CC = gcc
OUTFILE = tcptest
OBJS = tcpclient.o main.o pthreads.o
CFILE = $(wildcard *.c)
#OFILE = $(patsubst %.c,%.o,$(wildcard *.c))

#a = a.c b.c
#b = $(a:.c=.o) // 替换a变量中.c的字符为.o
#OFILE = $($CFILE:.c=.o)
OFILE = $(CFILE:%.c=%.o)

%.o:%.c
	$(CC) -c $< -o $@ -lpthread

$(OUTFILE):$(OFILE)
	$(CC) -o $(OUTFILE) $^ -lpthread
	@echo $(OUT)

clean:
	rm -fr $(OUTFILE) $(OFILE)
	



