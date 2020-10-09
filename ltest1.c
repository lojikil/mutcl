/* a test of the mutcl lexer */
#include <stdio.h>
#include <stdlib.h>
#include "mutcl.h"

extern int lex(char *,int *,char **);

int
main()
{
	char *buf = "set x 1;if {$x = 1} {echo \"yes\"}";
	char *ret = nil;
	int idx = 0, iter = 0, rc = 0;
	if((ret = malloc(sizeof(char) * 16)) == nil)
	{
		printf("cannot malloc ret!\n");
		return 1;
	}
	do
	{
		rc = lex(buf,&idx,&ret);
		printf("%d: \"%s\"\n",rc,ret);
	}while(!(rc == EOB || rc == EOL));
	free(ret);
	printf("Done\n");
	return 1;
}
