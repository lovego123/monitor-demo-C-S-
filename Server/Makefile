CC:=g++
CFLAGS:= -Wall -g
out:=myserver
obj:=V4l2.o Server.o Main.o

.PHONY : all clean cleanall
all:$(obj)
	$(CC) -o $(out) $(obj) -lpthread
%.o:%.c
	$(CC) $(CFLAGS) -c $^ -o $@ 

clean:
	rm -f $(obj)
cleanall:
	rm -f $(obj) $(out)

