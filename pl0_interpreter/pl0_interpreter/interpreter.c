#define _CRT_SECURE_NO_WARNINGS
#define NDEBUG
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include "interpreter.h"

long base(long b, long l)
{
    long b1;

    b1 = b;
    while (l > 0)
    {	// find base l levels down
        b1 = s[b1]; l = l - 1;
    }
    return b1;
}

typedef struct Pair {
    char *key;
    enum fct value;
} Pair;

Pair fct_arr[] = { "cal", cal,  "int", Int, "jmp", jmp,
 "jpc", jpc,  "lit", lit, "lod", lod,"lod2", lod2,
"opr", opr,   "sto", sto,"sto2", sto2 };

static int cmp(const void *key, const void *pair) {
    return strcmp((char *)key, ((Pair *)pair)->key);
}

/*
** s 是 fct 的字符串形式
*/
enum fct str2fct(char *s) {
    Pair *p = (Pair *)bsearch(s, fct_arr, sizeof fct_arr / sizeof *fct_arr, sizeof *fct_arr, cmp);
    assert(NULL != p);
    return p->value;
}

void interpret()
{
    // long pre_p = 0;   // 记录 pc
    long p = 0;       // program count
    long b = 1;       // base register
    long t = 0;       // topstack register(指向栈顶，非下一位置，s[0]不用)
    instruction i;    // instruction register	float fa = 0;	// 为操作浮点数

    printf("start PL/0\n");
    t = 0; b = 1; p = 0;
    s[1] = 0; s[2] = 0; s[3] = 0;
    do {
        i = code[p]; p = p + 1;
        switch (i.f) {
        case lit:
            t = t + 1; s[t] = i.a;
            break;
        case opr:
            switch (i.a)
            { 	// operator
            case 0:	// return
                t = b - 1; p = s[t + 3]; b = s[t + 2];
                break;
            case 1:
                s[t] = -s[t];
                break;
            case 2:
                t = t - 1; s[t] = s[t] + s[t + 1];
                break;
            case 3:
                t = t - 1; s[t] = s[t] - s[t + 1];
                break;
            case 4:
                t = t - 1; s[t] = s[t] * s[t + 1];
                break;
            case 5:
                t = t - 1; s[t] = s[t] / s[t + 1];
                break;
            case 6:
                s[t] = s[t] % 2;
                break;
            case 8:
                t = t - 1; s[t] = (s[t] == s[t + 1]);
                break;
            case 9:
                t = t - 1; s[t] = (s[t] != s[t + 1]);
                break;
            case 10:
                t = t - 1; s[t] = (s[t] < s[t + 1]);
                break;
            case 11:
                t = t - 1; s[t] = (s[t] >= s[t + 1]);
                break;
            case 12:
                t = t - 1; s[t] = (s[t] > s[t + 1]);
                break;
            case 13:
                t = t - 1; s[t] = (s[t] <= s[t + 1]);
            case 14:
                printf("%d", s[t]);
                t--;
                break;
            case 15:
                printf("\n");
                break;
            case 16:
                printf("?");
                scanf("%d", &(s[t + 1]));
                t++;
                break;
            }
            break;
        case lod:
            t = t + 1; s[t] = s[base(b, i.l) + i.a];
            break;
        case lod2:
            // s[t - 1] = s[base(i.l, s, b) + s[t - 1]];//应当覆盖基址和偏移量所在位置，以保证可以进行条件判断
            s[t] = s[base(b, i.l) + s[t]];
            break;
        case sto:
            s[base(b, i.l) + i.a] = s[t]; //printf("%10d\n", s[t]);
            t = t - 1;
            break;
        case sto2:
            s[base(b, i.l) + s[t - 1]] = s[t];
            t--;
            break;
        case cal:		// generate new block mark
            s[t + 1] = base(b, i.l);
            s[t + 2] = b;
            s[t + 3] = p;
            b = t + 1; p = i.a;
            break;
        case Int:
            t = t + i.a;
            break;
        case jmp:
            p = i.a;
            break;
        case jpc:
            if (s[t] == 0) {
                p = i.a;
            }
            t = t - 1;
        }
    } while (p != 0);
    printf("end PL/0\n");
}

int main()
{
    long i = 0;
    char s_fct[4];
    infile = NULL;

    printf("Please input intermediate representation file name:\n");
    scanf("%s", infilename);
    infile = fopen(infilename, "r");
    if (infile == NULL)
    {
        printf("Can't open file %s.\n", infilename);
        system("pause");
        exit(1);
    }

    for (i = 0; i <= cxmax && EOF != fscanf(infile, "%s", s_fct); ++i)
    {
        code[i].f = str2fct(s_fct);
        fscanf(infile, "%ld%ld", &(code[i].l), &(code[i].a));
        // printf("%5s%3d%15d\n", code[i].f, code[i].l, code[i].a);
    }

    if (i > cxmax)
    {
        printf("program too long\n");
        system("pause");
        exit(1);
    }

    interpret();

    fclose(infile);
    system("pause");
    return 0;
}