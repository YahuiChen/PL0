﻿#include <stdio.h>

#define norw       15             // no. of reserved words
#define txmax      100            // length of identifier table
#define nmax       14             // max. no. of digits in numbers
#define al         10             // length of identifiers
#define amax       2047           // maximum address
#define levmax     3              // maximum depth of block nesting
#define cxmax      2000           // size of code array
#define elsize      2000           // while中的exit表的长度
//#define etlsize     2000           // 变量声明中的enterlist表的长度
#define MAXDIM 10	      // maximum dimensions of array

#define nul	   0x1
#define ident      0x2
#define number     0x4
#define plus       0x8
#define minus      0x10
#define times      0x20
#define slash      0x40
#define oddsym     0x80
#define eql        0x100
#define neq        0x200
#define lss        0x400
#define leq        0x800
#define gtr        0x1000
#define geq        0x2000
#define lparen     0x4000
#define rparen     0x8000
#define comma      0x10000
#define semicolon  0x20000
#define period     0x40000
#define becomes    0x80000
#define beginsym   0x100000
#define endsym     0x200000
#define ifsym      0x400000
#define thensym    0x800000
#define whilesym   0x1000000
#define writesym   0x2000000
#define readsym    0x4000000
#define dosym      0x8000000
#define callsym    0x10000000
#define constsym   0x20000000
#define varsym     0x40000000
#define procsym    0x80000000
#define elsesym    0x91000000
#define colon      0x90000000
#define exitsym    0x92000000
#define arraysym   0x81000000
#define integersym 0x21000000
#define rangesym   0x82000000
#define lbracket 0x2000000000000
#define rbracket 0x4000000000000
#define ofsym      0x100000000

//#define writesym   0x2000000

enum object {
    constant, variable, proc, array
};

typedef enum {
    false,
    true
} bool;

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

char ch;               // last character read
unsigned long long sym;     // last symbol read
char id[al + 1];         // last identifier read
long num;              // last number read
long cc;               // character count
long ll;               // line length
long kk, err;
long cx;               // code allocation index
long cx3;
int  level = 0;
//int  tx = 0;		//符号表索引
//int  ax = 0;		//数组符号表索引

char line[81];
char a[al + 1];
instruction code[cxmax + 1];
char word[norw][al + 1];
unsigned long wsym[norw];
unsigned long ssym[256];

char mnemonic[10][5];
unsigned long declbegsys, statbegsys, facbegsys;
long exitlist[elsize];    // while中的exit地址表
long elx;                 // exitlist指针

struct {
    char name[al + 1];
    enum object kind;
    long val;
    long level;
    long addr;
    //int low;      /* 下界：仅数组需要使用*/
    //int sum;
    //int  n;						//数组总维数
    //int  dim[MAXDIM];			//数组对应维数的存储空间
    //int  size[MAXDIM];			//数组对应维数的地址偏移量大小
}table[txmax + 1];

char infilename[80];
FILE* infile;

// the following variables for block
long dx;		// data allocation index
long lev;		// current depth of block nesting
long tx;		// current table index
//long enterlist[etlsize];  // 变量声明中的enterlist表
//long etlx;                // enterlist指针

// the following array space for interpreter
#define stacksize 50000
long s[stacksize];	// datastore
