#ifndef PL0_H
#define PL0_H





#include <stdio.h>

#define NRW        20     // number of reserved words
#define TXMAX      2000    // length of identifier table
#define MAXNUMLEN  14     // maximum number of digits in numbers
#define NSYM       16     // maximum number of symbols in array ssym and csym
#define MAXIDLEN   10     // length of identifiers

#define MAXADDRESS 32767  // maximum address
#define MAXLEVEL   32     // maximum depth of nesting block
#define CXMAX      2000    // size of code array

#define MAXSYM     43     // maximum number of symbols  

#define STACKSIZE  4000   // maximum storage

#define MAXDIM 10	      // maximum dimensions of array

enum symtype
{
	SYM_NULL,
	SYM_IDENTIFIER,
	SYM_NUMBER,
	SYM_PLUS,
	SYM_MINUS,
	SYM_TIMES,
	SYM_SLASH,
	SYM_ODD,
	SYM_EQU,
	SYM_NEQ,
	SYM_LES,
	SYM_LEQ,
	SYM_GTR,
	SYM_GEQ,
	SYM_LPAREN,
	SYM_RPAREN,
	SYM_COMMA,
	SYM_SEMICOLON,
	SYM_PERIOD,
	SYM_BECOMES,
    SYM_BEGIN,
	SYM_END,
	SYM_IF,
	SYM_THEN,
	SYM_WHILE,
	SYM_DO,
	SYM_CALL,
	SYM_CONST,
	SYM_VAR,
	SYM_PROCEDURE,	//����Ϊ��ӵ�symtype,��12��
	SYM_AND,		
	SYM_OR,
	SYM_NOT,
	SYM_ELSE,
	SYM_FOR,
	SYM_BREAK,
	SYM_EXIT,
	SYM_LSPAREN,
	SYM_RSPAREN,
	SYM_ARRAY,
	SYM_READ,
	SYM_WRITE,
	SYM_MOD,
	SYM_ADDPLUS,
	SYM_SUBPLUS
};

enum idtype			//�����ID_ARRAY�ķ��ű�����
{
	ID_CONSTANT, ID_VARIABLE, ID_PROCEDURE, ID_ARRAY
};

/*
 *opcode��ԭ��8��ָ��Ļ����ϣ����������7����
 *JPNC:������������ת��
 *LDA :��������ֵ��ջ����
 *STA :�洢����ֵ��ƫ�Ƶ�ַ�ϣ�
 *RDA :��ȡ����ָ�
 *WTA :��ӡ����ָ�
 *READ:��ȡ����ֵ��
 *WRITE:��ӡ����ֵ��
 */
enum opcode			
{
	LIT, OPR, LOD, STO, CAL, INT, JMP, JPC, JPNC, LDA, STA, RDA, WTA, READ, WRITE
};

/*
 *oprcode��ԭ��13��ָ��Ļ����ϣ����������5����
 *OPR_NOT:��ջ��ֵȡ�ǣ�
 *OPR_WEN:��ӡ�ո�ָ�
 *OPR_WRITE:��ӡջ������ָ�
 *OPR_ADDPLUS: ����+=���㣻
 *OPR_SUBPLUS: ����-=���㡣
 */
enum oprcode
{
	OPR_RET, OPR_NEG, OPR_ADD, OPR_MIN,
	OPR_MUL, OPR_DIV, OPR_ODD, OPR_EQU,
	OPR_NEQ, OPR_LES, OPR_LEQ, OPR_GTR,
	OPR_GEQ, OPR_NOT, OPR_WEN, OPR_WRITE,
	OPR_MOD, OPR_ADDPLUS,OPR_SUBPLUS
};


typedef struct
{
	int f; // function code
	int l; // level
	int a; // displacement address
} instruction;

//////////////////////////////////////////////////////////////////////
char* err_msg[] =
{
/*  0 */    "",
/*  1 */    "Found ':=' when expecting '='.",
/*  2 */    "There must be a number to follow '='.",
/*  3 */    "There must be an '=' to follow the identifier.",
/*  4 */    "There must be an identifier to follow 'const', 'var', or 'procedure'.",
/*  5 */    "Missing ',' or ';'.",
/*  6 */    "Incorrect procedure name.",
/*  7 */    "Statement expected.",
/*  8 */    "Follow the statement is an incorrect symbol.",
/*  9 */    "'.' expected.",
/* 10 */    "';' expected.",
/* 11 */    "Undeclared identifier.",
/* 12 */    "Illegal assignment.",
/* 13 */    "':=' expected.",
/* 14 */    "There must be an identifier to follow the 'call'.",
/* 15 */    "A constant or variable can not be called.",
/* 16 */    "'then' expected.",
/* 17 */    "';' or 'end' expected.",
/* 18 */    "'do' expected.",
/* 19 */    "Incorrect symbol.",
/* 20 */    "Relative operators expected.",
/* 21 */    "Procedure identifier can not be in an expression.",
/* 22 */    "Missing ')'.",
/* 23 */    "The symbol can not be followed by a factor.",
/* 24 */    "The symbol can not be as the beginning of an expression.",
/* 25 */    "The number is too great.",
/* 26 */    "expected ')' or '('",
/* 27 */    "can't break",
/* 28 */    "Missing ']'.",
/* 29 */    "",
/* 30 */    "",
/* 31 */    "",
/* 32 */    "There are too many levels."
};

//////////////////////////////////////////////////////////////////////
char ch;         // last character read
int  sym;        // last symbol read
char id[MAXIDLEN + 1]; // last identifier read
int  num;        // last number read
int  cc;         // character count
int  ll;         // line length
int  kk;
int  err;
int  cx = 0;         // index of current instruction to be generated.
int  level = 0;
int  tx = 0;		//���ű�����
int  ax = 0;		//������ű�����
//add
/*
	ע����һ��������Ƕ�׶��������� ���� for�������� begin if����then statement end��
	����for�����������ܻ���if���������غ� ���桢��ֵ���ö�ά���飬�Ա������ʱ��������ָ��
	�� condition_level ��¼��ǰ����Ƕ�ײ���
	true_count[]��false_count[]��ʾ��ǰ��Ҫ�����桢�ٳ��ڵ�ָ����Ŀ
*/
int true_out[4][10] = { 0 }; //��·���� ��ֵ�� 
int false_out[4][10] = { 0 };//��·���� ��ֵ��
int true_count[4] = { 0 };
int false_count[4] = { 0 };
int condition_level = 0;
int mid_cx;
/*
	����������������if ��䣬���������else,��if�н���ȥ��ȡ������ķֺź����ֵ��
	���磺 if a > 1 then 
			statement ; 
		  j := 1;
		��ִ����if���󣬵�ǰ�ķ��ű�ɱ�ʶ������Ϊ'j'��
		�����Ҫ��'j'������������sym ��ֵΪ';',Ȼ����һ������ȡ'j'����'j'ΪԤ֪����
*/
int sym_count = 0;  //Ԥ֪�ķ�����Ŀ
int sym_stack[10] = { 0 };//Ԥ֪�ķ���
//int break_stack[10] = { 0 };

/*
	���·���Ϊbreak������õ��ģ�break_count ��ʾbreak�Ĳ�����
	break_cx[]�洢��ǰ��break��Ӧ�õ�ת����ָ��
*/
int break_count = 0;
int break_cx[4] = { 0 };


char line[80];

instruction code[CXMAX];

/*
 *������and,or,not,else,for,break,exit,read,print��9���ؼ���
 */
char* word[NRW + 1] =
{
	"", /* place holder */
	"begin", "call", "const", "do", "end","if",
	"odd", "procedure", "then", "var", "while","and","or","not","else","for",
	"break","exit","read","print"
};

/*
 *wsym�����������Ϲؼ��ֶ�Ӧ��symtype����
 */
int wsym[NRW + 1] =
{
	SYM_NULL, SYM_BEGIN, SYM_CALL, SYM_CONST, SYM_DO, SYM_END,
	SYM_IF, SYM_ODD, SYM_PROCEDURE, SYM_THEN, SYM_VAR, SYM_WHILE,SYM_AND,SYM_OR,SYM_NOT,
	SYM_ELSE,SYM_FOR,SYM_BREAK,SYM_EXIT, SYM_READ, SYM_WRITE
};

/*
*ssym����������SYM_AND,SYM_OR,SYM_NOT,
*SYM_LSPAREN,SYM_RSPAREN,SYM_MOD��6��symtype����
*/
int ssym[NSYM + 1] =
{
	SYM_NULL, SYM_PLUS, SYM_MINUS, SYM_TIMES, SYM_SLASH,
	SYM_LPAREN, SYM_RPAREN, SYM_EQU, SYM_COMMA, SYM_PERIOD, SYM_SEMICOLON,
	SYM_AND,SYM_OR,SYM_NOT, SYM_LSPAREN, SYM_RSPAREN,SYM_MOD
};

/*
*csym������������ssym�����Ӧsymtype���͵ķ���
*/
char csym[NSYM + 1] =
{
	' ', '+', '-', '*', '/', '(', ')', '=', ',', '.', ';','&','|','!', '[', ']','%'
};

/*
*mnemonic��������7��ָ��
*/
#define MAXINS   15
char* mnemonic[MAXINS] =
{
	"LIT", "OPR", "LOD", "STO", "CAL", "INT", "JMP", "JPC", "JPNC",
	"LDA", "STA", "RDA", "WTA", "READ", "WRITE"
};

typedef struct
{
	char name[MAXIDLEN + 1];
	int  kind;
	int  value;
} comtab;

//comtab table[TXMAX];


/*
 *mask�ṹ���value��������ԭcomtab table[TXMAX]ע�͵���
 *ֱ����mask�ṹ���������ű�
*/
typedef struct
{
	char  name[MAXIDLEN + 1];
	int   kind;
	int   value;
	short level;
	short address;
} mask;
mask table[TXMAX];	


/*
 *�������ṹ
*/
typedef struct
{
	char name[MAXIDLEN + 1];	//�����������
	int  sum;					//����
	int  n;						//������ά��
	int  dim[MAXDIM];			//�����Ӧά���Ĵ洢�ռ�
	int  size[MAXDIM];			//�����Ӧά���ĵ�ַƫ������С
	int  addr;					//�����׵�ַ
}arr;

arr array_t, array_table[TXMAX];	//��������ṹ����������ű�


FILE* infile;
#endif // 
// EOF PL0.h
