CC=gcc
LTEST2OBJS=ltest2.o nlex.o
DLTEST2OBJS=dltest2.o nlex.o
LTEST1OBJS=ltest1.o nlex.o
LTEST0OBJS=ltest0.o nlex.o

dummy: 
	@echo "This is a dummy rule; you should really attempt to"
	@echo "build ltest2, dltest2, argvtst, ltest1, ltest0"
ltest2: $(LTEST2OBJS)
	$(CC) -o ltest2 $(LTEST2OBJS) 

dltest2: $(DLTEST2OBJS)
	$(CC) -o dltest2 $(DLTEST2OBJS)

dltest2.o: ltest2.c
	$(CC) -c -o dltest2.o ltest2.c -DDEBUG -Wall

argvtst: 
	$(CC) -o argvtst argvtst.c -Wall -DDEBUG
ltest1: $(LTEST1OBJS)
	$(CC) -o $@ $(LTEST1OBJS)
ltest0: $(LTEST0OBJS)
	$(CC) -o $@ $(LTEST0OBJS)
%.o: %.c
	$(CC) -c $<
clean: 
	rm -f *.o ltest2 dltest2 ltest1 core argvtst
