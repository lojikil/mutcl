#ifndef __MUTCL_H
#define __MUTCL_H

#define nil NULL
#define nul '\0'

#define START 0
#define VAR 1
#define SYM 2
#define SBOPN 3
#define SBCLS 4
#define CBOPN 5 // CBOPN & CBCLS are not used
#define CBCLS 6
#define STR 7
#define NUM 8
#define CURLY 9
#define SEMI 10
#define EOL 97  // \n (maybe \r\n too, for network support)
#define EOB 98
#define ERR 99

#endif 
