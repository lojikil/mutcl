/* a test of the mutcl lexer */
#include <stdio.h>
#include <stdlib.h>
#include "mutcl.h"

extern int lex(char *,int *,char **);

int
main()
{
	char *buf = "this $is {a} \"test of mutcl\" 45 [times]";
	char *ret = nil;
	int idx = 0, iter = 0, rc = 0;
	if((ret = malloc(sizeof(char) * 16)) == nil)
	{
		printf("cannot malloc ret!\n");
		return 1;
	}
	while(rc != EOB)
	{
		rc = lex(buf,&idx,&ret);
		printf("%d: \"%s\"\n",rc,ret);
	}
	free(ret);
	printf("Done\n");
	return 1;
}
