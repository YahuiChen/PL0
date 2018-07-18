#include <stdio.h>
#define cxmax      2000           // size of code array

enum fct {
    lit, opr, lod, sto, cal, Int, jmp, jpc, sto2, lod2         // functions
};
typedef struct {
    enum fct f;		// function code
    long l; 		// level
    long a; 		// displacement address
} instruction;
/*  lit 0, a : load constant a
opr 0, a : execute operation a
lod l, a : load variable l, a
sto l, a : store variable l, a
cal l, a : call procedure a at level l
Int 0, a : increment t-register by a
jmp 0, a : jump to a
jpc 0, a : jump conditional to a       */

char infilename[80];
FILE* infile;

instruction code[cxmax + 1];

#define stacksize 50000
long s[stacksize];	// datastore