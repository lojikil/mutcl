/* a naieve lexer for mutcl; doesn't double buffer, doesn't work with
 * stdin directly, or many other useful things. It's a PoC lexer, and 
 * can easily be extended for the Real Thing (TM).
 */
#include <stdio.h>
#include "mutcl.h"

int symbol(char c)
{
	if((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
		return 1;
	if(c == '!' || (c >= '%' && c <= '/') || (c >= ':' && c <= '@') || c == '`' || c == '|' || c == '^' || c == '_' || c == '~' || c == '\\')
		return 1;
	if((c >= '0' && c <= '9') || c == '.')
		return 1;
	return 0;
} 
int
lex(char *buf, int *sidx, char **ret)
{
	int iter = *sidx, tmp = *sidx, riter = 0, rc = 0;
	/* riter is the index for ret */
	char *rbuf = *ret;
	if(rbuf == nil)
		return ERR;
	if(buf[iter] == ' ' || buf[iter] == '\t' || buf[iter] == '\n')
		while(buf[iter] == ' ' || buf[iter] == '\t' || buf[iter]== '\n')
			iter++;
	tmp = iter;
	if(buf[iter] == ';')
	{
		iter++;
		rc = SEMI;
	}
	else if(buf[iter] == '$') /* variable */
	{
		++iter;
		tmp = iter;
		while(symbol(buf[iter])) iter++;
		rc = VAR;
	}
	else if(symbol(buf[iter]))
	{
		while(symbol(buf[iter])) iter++;
		rc = SYM;
	}
	else if(buf[iter] == '[')
	{
		rc = SBOPN;
		rc = STR;
		iter++;
		tmp = iter;
		while(buf[iter] != ']') iter++;
	}
	else if(buf[iter] == ']')
	{
		rc = SBCLS;
		iter++;
	}
	else if(buf[iter] == '{')
	{
		rc = CURLY;
		iter++;
		tmp = iter;
		while(buf[iter] != '}') iter++;
		//iter++; // consume '}'
	}
	else if(buf[iter] == '"')
	{
		rc = STR;
		iter++;
		tmp = iter;
		while(buf[iter] != '"') iter++;
	}
	else if(buf[iter] == nul)
	{
		rc = EOL;
		iter++;
	}
	else
	{
		iter++;
		rc = ERR;
	}
	while(tmp < iter)
	{
		rbuf[riter] = buf[tmp];
		tmp++; riter++;
	}
	rbuf[riter] = nul;
	*sidx = iter + 1;
	return rc;
}
