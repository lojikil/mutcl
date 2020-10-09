/* ltest2: a simple TCL-clone, that originally was supposed to *only* 
 * be a test, but has grown organically to be a nice system for scripting
 * and a half-way decent shell. It's real name is MikroCL.
 * Ideas:
 *    - no expr; use +,-,/,%,^,&c as in lisp set x [+ 1 2 3 4 5]; set y [+ 1 [* 2 3]]
 * History:
 *  start - sometime in December of 07
 *  alpha - made ltest2 the final product, not muTCL.
 * To Do:
 *  0 - make eval support [somecommand] nicely
 *  1 - port to Plan9, possibly Inferno.
 *  2 - optimize eval to not call it self recursively, but use a stack.
 *  3 - proc, while, for, foreach, if, cond, &c. 
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/param.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include "mutcl.h"

#define MAXHEAP sizeof(char) * 4096

#define TCL_OK 0
#define TCL_ER 1
#define TCL_EX 2 // TCL_EX == exit

extern int lex(char *,int *,char **);

uint8_t eval(char *); /* returns 0 for Ok, & > 0 for ERR */
uint8_t _set(int,char **);
uint8_t _gets(int,char **);
uint8_t _mexit(int,char **);
uint8_t _echo(int,char **);
uint8_t _expr(int,char **);
uint8_t _walk(int,char **);
uint8_t _chdir(int,char **);
uint8_t _pwd(int,char **);
uint8_t _lindex(int,char **);
uint8_t _llength(int,char **);
uint8_t _lf(int,char **);
uint8_t _incr(int, char **);
uint8_t _add(int,char **);
uint8_t _car(int,char **);
uint8_t _cdr(int,char **);
uint8_t _sub(int,char **);
uint8_t _mul(int, char **);
uint8_t _div(int, char **);
void *hmalloc(int);
int addsym(char *,char *);
char *getsym(char *);
int fndproc(char *); /* returns the index to the proc, or -1 */
char *_strdup(char *); /* uses hmalloc */
void cleansymbols(void); /* clean up symbol list */

typedef struct
{
	char *name;
	uint8_t numargs;
	uint8_t (*fn)(int,char **);
} FunTab;

typedef struct _SYMTAB
{
	uint8_t type; /* string, CURLY, &c. */
	char *name;
	char *data;
	struct _SYMTAB *next;
	//struct _SYMTAB *prev;
} Symtab;

FunTab fntbl[] = {
	{"set",2,_set},
	{"echo",-1,_echo},
	{"gets",1,_gets},
	{"exit",0,_mexit},
	{"quit",0,_mexit},
	{"walk",0,_walk}, // dump a list of all variables
	{"chdir",1,_chdir},
	{"cd",1,_chdir},
	{"pwd",0,_pwd},
	{"lindex",2,_lindex},
	{"llength",2,_llength},
	{"expr",-1,_expr},
	{"lf",1,_lf},
	{"incr",-1,_incr},
	{"+",-1,_add},
	{"*",-1,_mul},
	{"/",-1,_div},
	{"-",-1,_sub},
	{nil,0,nil}	
};

static char *heap = nil; /* heap for all internal mallocs */
Symtab *head = nil,*tail = nil;

int
main()
{
	char *buf = nil;
	if((heap = malloc(MAXHEAP)) == nil)
	{
		printf("cannot malloc heap!\n");
		return 1;
	}
	if((buf = hmalloc(sizeof(char) * 256)) == nil)
	{
		printf("cannot malloc buf!\n");
		return 2;
	}
	fputs("ltest2 (mutcl alpha)\n",stdout);
	while(1)
	{
		fputs("; ",stdout);
		fgets(buf,256,stdin);
		if(eval(buf))
			break;
	}
#ifdef DEBUG
	printf("%%DEBUG-main: exiting; going to free heap\n");
#endif
	cleansymbols();
	free(heap);
	return 0;
}
int
fndproc(char *name)
{
	int iter = 0;
	while(fntbl[iter].name != nil)
	{
		if(!strcmp(fntbl[iter].name,name))
			return iter;
		iter++;
	}
	return -1;
}
char *
getsym(char *name)
{
	Symtab *cur = head;
	if(cur == nil)
		return nil;
	do
	{
		if(!strcmp(cur->name,name))
			return cur->data;
		cur = cur->next;
	}while(cur != nil);
	return nil;
}
uint8_t
eval(char *buf)
{
	int iter = 0, ac = 0, rc = 0, lt = 0, fidx = 0,aiter = 0;
	static char *cmd = nil, *tmp = nil;
	char *argv[32], *vartmp = nil; /* max 32 args */
	if(cmd == nil)
	{
		if((cmd = hmalloc(sizeof(char) * 128)) == nil)
		{
			printf("cannot malloc cmd!\n");
			return -1;
		}
	}
	if(tmp == nil)
	{
		if((tmp = hmalloc(sizeof(char) * 128)) == nil)
		{
			printf("cannot malloc tmp!\n");
			return -1;
		}
	}
	lex(buf,&iter,&cmd);
	if((fidx = fndproc(cmd)) >= 0)
	{
		// process arguments
		while((lt = lex(buf,&iter,&tmp)) != EOL)
		{
#ifdef DEBUG
			printf("%%DEBUG-eval(%d): %d \"%s\"\n",ac,lt,tmp);
#endif
			if(lt == VAR)
			{
				if((vartmp = getsym(tmp)) == nil)
				{
					printf("no such variable: '%s'\n",tmp);
					break;
				}
				argv[ac] = _strdup(vartmp);
			}
			else
				argv[ac] = _strdup(tmp);
			ac++;	
		}
#ifdef DEBUG
		printf("%%DEBUG-dump-argv: %d\n",ac);
		for(aiter = 0;aiter < ac;aiter++)
			printf("Argv[%d] = %s\n",aiter,argv[aiter]);
#endif
		// run function
		switch(fntbl[fidx].fn(ac,argv))
		{
			case TCL_EX: // exit
				rc = 1;
				break;
			case TCL_ER:
				rc = 2;
				break;
			case TCL_OK:
			default:
				rc = 0;
				break;
		}
	}
	else
		printf("no such command: \"%s\"\n",cmd);
	return rc;
}
void *
hmalloc(int sze)
{
	static int barrier = 0;
	char *rc = nil;
#ifdef DEBUG
	printf("%%DEBUG-hmalloc: malloc'ing %d w/ barrier @ %d\n",sze,barrier);
#endif
	if((barrier + sze) < MAXHEAP)
	{
		rc = &heap[barrier];
		barrier += sze;
	}
	else
		rc = nil;
	return rc;
}
uint8_t
_expr(int ac, char **al)
{
	return TCL_OK;
}
uint8_t
_mexit(int ac, char **al)
{
	#ifdef DEBUG
	printf("%%DEBUG-_mexit: exiting\n");
	#endif
	return TCL_EX;
}
uint8_t
_echo(int ac, char **al)
{
	int iter = 0;
#ifdef DEBUG
	printf("%%DEBUG-echo\n");
#endif
	for(;iter < ac;iter++)
		printf("%s ", al[iter]);
	printf("\n");
	return TCL_OK;
}
uint8_t
_gets(int ac, char **al)
{
	char *buf = nil;
	if((buf = hmalloc(sizeof(char) * 64)) == nil)
		return TCL_ER;
	fgets(buf,64,stdin);
	addsym(al[0],buf);
	return TCL_OK;
}
uint8_t
_set(int ac, char **al)
{
	return (uint8_t)addsym(al[0],al[1]);
}
char *
_strdup(char *i)
{
	int len = strlen(i) + 1, iter = 0;
	char *rc = nil;
	if((rc = hmalloc(sizeof(char) * len)) == nil)
		return nil;
	while(iter < len)
	{
		rc[iter] = i[iter];
		iter++;
	}
	rc[iter] = nul;
#ifdef DEBUG
	printf("%%DEBUG-_strdup: \"%s\"\n",rc);
#endif
	return rc;
}
int
addsym(char *name,char *data)
{
	Symtab *cur = head, *tmp = nil;
	if(cur == nil)
	{
		if((cur = malloc(sizeof(Symtab))) == nil)
			return TCL_ER;
		cur->next = nil;
		cur->data = data;
		cur->name = name;
		cur->type = STR;
		head = cur;
	}
	else
	{
		/* need to search list in case var is already defined.. */
		while(cur->next != nil && strcmp(cur->name,name)) cur = cur->next;
		/* create a new node as above & populate it */
		if(cur->next == nil) // end of the list
		{
			if((cur->next = malloc(sizeof(Symtab))) == nil)
				return TCL_ER;
			cur = cur->next;
			cur->next = nil;
			cur->data = data;
			cur->name = name;
			cur->type = STR;
		}
		else
			cur->data = data;
	}
	return TCL_OK;
}
void
cleansymbols()
/* if I made the allocator work properly (actually, the deallocator),
 * I wouldn't need to keep symbols in the real heap. A to do...
 */
{
	Symtab *car = head, *cdr = head;
	while(cdr != nil)
	{
		cdr = cdr->next;
		free(car);
		car = cdr;
	}
	free(car);
}
uint8_t
_walk(int ac, char **al)
{
	Symtab *cur = head;
	while(cur != nil)
	{
		printf("\"%s\"::\"%s\"\n",cur->name,cur->data);
		cur = cur->next;
	}
	return TCL_OK;
}
uint8_t
_chdir(int ac, char **al)
{
	if(chdir(al[0]))
		return TCL_ER;
	return TCL_OK;
}
uint8_t
_pwd(int ac, char **al)
{
	static char *pathbuf = nil; // let heap take care of cleanup
	if(!pathbuf)
		if((pathbuf = hmalloc(sizeof(char) * MAXPATHLEN)) == nil)
			return TCL_ER;
	pathbuf = getcwd(pathbuf,MAXPATHLEN);
	printf("%s\n",pathbuf);
	return TCL_OK;
}
uint8_t
_llength(int ac, char **al)
{
	return TCL_OK;
}
uint8_t
_lindex(int ac, char **al)
{
	int index = 0, len = 0, siter = 0, liter = 0; 
	char *lst = nil;
	if(ac != 2)
	{
		printf("usage: lindex list index\n");
		return TCL_ER;
	}
	if(al[1][0] >= '0' && al[1][0] <= '9')
		index = atoi(al[1]);
	else
	{
		printf("index must be an integer!\n");
		return TCL_ER;
	}
	lst = al[0];
	len = strlen(al[0]);
#ifdef DEBUG
	printf("%%DEBUG-lindex: %s %d\n",al[0],index);
#endif
	for(;siter < len;siter++)
	{
		if(lst[siter] == ' ')
			liter++;
		if(liter == index)
		{
			if(siter != 0) siter++;	
			while(lst[siter] != ' ' && lst[siter] != nul)
			{
				printf("%c",lst[siter]);
				siter++;
			}
			printf("\n");
			liter++;
		}
	}
	return TCL_OK;
}
uint8_t
_lf(int ac, char **al)
/* an extremely naive 'ls' implementation; 'lf' stands
 * for 'list files'. I was going to call this the 
 * Multics/PR1MOS 'ld', but I didn't want to miss
 * being able to call the linker later =)
 */
{
	struct dirent *ent;
	DIR *dp = nil;
	dp = opendir(".");
	while((ent = readdir(dp)) != nil)
		printf("%s\n",ent->d_name);
	closedir(dp);
	return TCL_OK;
}
uint8_t
_incr(int ac, char **al)
{
	int val = 0, inc = 0;
	if(ac == 2)
	{
		val = atoi(al[0]);
		inc = atoi(al[1]);
		printf("%d\n",val + inc);
	}
	else if (ac == 1)
	{
		val = atoi(al[0]);
		printf("%d\n",val + 1);
	}
	return TCL_OK;
}
uint8_t
_add(int ac, char **al)
{
	float acc = 0.0f, op = 0.0f;
	int iter = 0;
	for(;iter < ac;iter++)
	{
		op = strtof(al[iter],nil);
		acc += op;
	}
	printf("%f\n",acc);
	return TCL_OK;
}
uint8_t
_sub(int ac, char **al)
{
	return TCL_OK;
}
uint8_t
_div(int ac, char **al)
{
	return TCL_OK;
}
uint8_t
_mul(int ac, char **al)
{
	return TCL_OK;
}
uint8_t
_car(int ac, char **al)
{
	return TCL_OK;
}
uint8_t
_cdr(int ac, char **al)
{
	return TCL_OK;
}
