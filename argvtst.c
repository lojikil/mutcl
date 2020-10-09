/* so the error, finally?
 * look at hmalloc, as it originally was:
void *
hmalloc(int sze)
{
	static int barrier = 0;
	if((barrier + sze) < MAXHEAP)
	{
		barrier += sze;
		return &heap[barrier];
	}
	return nil;
}
 * spot the error? Uh, we're returning the section
 * AFTER the one we wanted here. the way argvtst's 
 * hmalloc is written now is correct. D'oh.
 * So, I only created something like 2k of extra 
 * text debugging a stupid issue.
 */
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>

#define nil NULL
#define nul '\0'

#define MAXHEAP sizeof(char) * 4096

static char *heap = nil;

void *
hmalloc(int sze)
{
	static int barrier = 0;
	char *rc = nil;
#ifdef DEBUG
	printf("%%DEBUG-hmalloc: malloc'ing %d w/ barrier @ %x\n",sze,(int)&heap[barrier]);
#endif
	if((barrier + sze) < MAXHEAP)
	{
		rc = &heap[barrier];
		barrier += sze + 1;
#ifdef DEBUG
		printf("%%DEBUG-hmalloc: malloc'd %d & barrier now resides @ %x\n",sze,(int)&heap[barrier]);
#endif
	}
	else
		rc = nil;
	return (void *)rc;
}

char *
_strdup(const char *i)
{
	int len = strlen(i), iter = 0;
	char *rc = nil;
	if((rc = hmalloc(sizeof(char) * len)) == nil)
		return nil;
	while(iter < len)
	{
		rc[iter] = i[iter];
		iter++;
	}
#ifdef DEBUG
	printf("len = %d, iter = %d\n",len,iter);
#endif
	rc[iter] = 0;
#ifdef DEBUG
	printf("%%DEBUG-_strdup: \"%s\"\n",rc);
#endif
	return rc;
}

int
main()
{
	int ac = 0, iter = 0;
	char *argv[32] = {0}, *buf = nil, *tmp = nil;
	if((heap = malloc(MAXHEAP)) == nil)
	{
		printf("suicide: cannot malloc heap.\n");
		return 1;
	}
	if((buf = hmalloc(sizeof(char) * 128)) == nil)
	{
		printf("suicide: cannot hmalloc buf.\n");
		free(heap);
		return 2;
	}
	while(fgets(buf,128,stdin))
	{
		if(!strncmp(buf,"exit",4))
			break;
		tmp = _strdup(buf);
		printf("%x\n",(int) &tmp[0]);
		iter = strlen(tmp);
		printf("%x\n",(int) &tmp[iter]);
		argv[ac] = tmp;
		ac++;
	}
	for(iter = 0;iter < ac;iter++)
	{
		write(1,argv[iter],strlen(argv[iter]));
		printf("\n");
		//free(argv[iter]);
	}
	//free(buf);
	free(heap);
	return 0;
}