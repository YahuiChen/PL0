// pl/0 compiler with code generation
#include <stdlib.h>
#include <string.h>
#include "pl0.h"

void error(long n) {
    long i;

    printf(" ****");
    for (i = 1; i <= cc - 1; i++) {
        printf(" ");
    }
    printf("^%2d\n", n);
    //printf("|%s(%d)\n", err_msg[n], n);
    err++;
}

void getch() {
    if (cc == ll) {
        if (feof(infile)) {
            printf("************************************\n");
            printf("      program incomplete\n");
            printf("************************************\n");
            exit(1);
        }
        ll = 0; cc = 0;
        printf("%5d ", cx);
        while ((!feof(infile)) && ((ch = getc(infile)) != '\n')) {
            printf("%c", ch);
            ll = ll + 1; line[ll] = ch;
        }
        printf("\n");
        ll = ll + 1; line[ll] = ' ';
    }
    cc = cc + 1; ch = line[cc];
}

void getsym()
{
    long i, j, k;

    while (ch == ' ' || ch == '\t')
    {
        getch();
    }
    if (isalpha(ch)) { 	// identified or reserved
        k = 0;
        do {
            if (k < al) {
                a[k] = ch; k = k + 1;
            }
            getch();
        } while (isalpha(ch) || isdigit(ch));
        if (k >= kk) {
            kk = k;
        }
        else {
            do {
                kk = kk - 1; a[kk] = ' ';
            } while (k < kk);
        }
        strcpy(id, a); i = 0; j = norw - 1;
        do {								//�۰����
            k = (i + j) / 2;
            if (strcmp(id, word[k]) <= 0) {
                j = k - 1;
            }
            if (strcmp(id, word[k]) >= 0) {
                i = k + 1;
            }
        } while (i <= j);
        if (i - 1 > j) {
            sym = wsym[k];
        }
        else {
            sym = ident;
        }
    }
    else if (isdigit(ch)) { // ����
        k = 0; num = 0; sym = number;
        do {
            num = num * 10 + (ch - '0');
            k = k + 1;
            getch();
        } while (isdigit(ch));
        if (k > nmax) {
            error(31);
        }
    }
    else if (ch == ':') {
        getch();
        if (ch == '=') {
            sym = becomes;
            getch();
        }
        else {
            sym = nul;
        }
    }
    else if (ch == '<') {
        getch();
        if (ch == '=') {
            sym = leq; getch();
        }
        else if (ch == '>') {
            sym = neq; getch();
        }
        else {
            sym = lss;
        }
    }
    else if (ch == '>') {
        getch();
        if (ch == '=') {
            sym = geq; getch();
        }
        else {
            sym = gtr;
        }
    }
    else if (ch == '/')		// to handle the notes beginning with "/*"
    {
        getch();
        if (ch == '*') {
            do {
                getch();
            } while (ch != '*');
            while (1) {
                getch();
                if (ch == '/') {
                    getch();
                    getsym();
                    break;
                }
                else {
                    do {
                        getch();
                    } while (ch != '*');
                }
            }
        }
        else if (ch == '/') {    // to handle the notes beginning with "//"
            cc = ll;
            getch();
            getsym();
        }
        else {
            // error(0);
           // sym = ssym['/'];
        }
    }

    else {
        sym = ssym[(unsigned char)ch]; getch();
    }
}

void gen(enum fct x, long y, long z) {
    if (cx > cxmax) {
        printf("program too long\n");
        exit(1);
    }
    code[cx].f = x; code[cx].l = y; code[cx].a = z;
    cx = cx + 1;
}

void test(unsigned long s1, unsigned long s2, long n) {
    if (!(sym & s1)) {
        error(n);
        s1 = s1 | s2;
        while (!(sym & s1)) {
            getsym();
        }
    }
}

void enter(enum object k) {		// enter object into table
    tx = tx + 1;
    strcpy(table[tx].name, id);
    table[tx].kind = k;
    switch (k)
    {
    case constant:
        if (num > amax) {
            error(31);
            num = 0;
        }
        table[tx].val = num;
        break;
    case variable:
        table[tx].level = lev; table[tx].addr = dx; dx = dx + 1;
        break;
    case proc:
        table[tx].level = lev;
        break;
    }
}

/*
* start:   ���鿪ʼλ��
* end:     �������Ϊֹ
*
* ��������ͬenter����
*/
void enterArray(int start, int end, char* id) {

    tx++;//�������ֱ�����
    strcpy(table[tx].name, id);
    table[tx].kind = array;//��������
    table[tx].level = lev;//���
    table[tx].addr = dx;//�����׵�ַ
    table[tx].low = start;
    dx += (end - start + 1);//�����ڴ��ַ�����������
}

long position(char* id, int tx) {	// find identifier id in table
    long i;

    strcpy(table[0].name, id);
    i = tx;
    while (strcmp(table[i].name, id) != 0) {
        i = i - 1;
    }
    return i;
}

void constdeclaration() {
    if (sym == ident) {
        getsym();
        if (sym == eql || sym == becomes) {
            if (sym == becomes) {
                error(1);
            }
            getsym();
            if (sym == number) {
                enter(constant);
                getsym();
            }
            else {
                error(2);
            }
        }
        else {
            error(3);
        }
    }
    else {
        error(4);
    }
}

//void vardeclaration() {
//    if (sym == ident) {
//        enter(variable);
//        getsym();
//    }
//    else {
//        error(4);
//    }
//}

void vardeclaration()
{
    if (sym == ident)
    {
        int n1 = 0, n2 = 0;
        bool e = false;
        getsym();
        if (sym == lparen)
        {
            char mid[11];//�洢�������֣�������д���ֱ�
            memcpy(mid, id, 11);
            //printf("find (");
            getsym();
            if (sym == number || sym == ident)
            {

                if (sym == number)
                {
                    n1 = num;
                }
                else
                {
                    n1 = isV(position(id, tx));
                    if (n1 == -1)
                    {
                        e = true;
                        error(31);//�����Ǳ���
                    }
                }
                if (!e)
                {
                    getsym();
                    if (sym == colon)
                    {
                        getsym();
                        if (sym == number || sym == ident)
                        {
                            if (sym == number)
                            {
                                n2 = num;
                            }
                            else
                            {
                                n2 = isV(position(id, tx));
                                if (n2 == -1)
                                {
                                    e = true;
                                    error(31);//�����Ǳ���
                                }
                            }
                            if (!e)
                            {
                                if (n1 <= n2)
                                {
                                    getsym();
                                    if (sym == rparen)
                                    {
                                        enterArray(n1, n2, mid);
                                    }
                                    else
                                    {
                                        error(31);//ȱ��������
                                    }
                                }
                                else
                                {
                                    error(31);//�½�����Ͻ�
                                }
                            }
                        }
                    }
                    else
                    {
                        error(31);//ȱ��ð��
                    }
                }
            }
            else
            {
                error(31);//ֻ�������ֻ��߱�ʶ��
            }
            getsym();
        }
        else
        {
            //int��
            enter(variable);
            // ��д���ֱ�
        }

    }
    else
    {
        error(4);   /* var��Ӧ�Ǳ�ʶ */
    }
    return 0;
}

int isV(int index)
{
    if (index == 0 || table[index].kind != constant)
    {
        error(31);//����δ�ҵ�
        return -1;
    }
    else
    {
        return table[index].val;
    }
}

void listcode(long cx0) {	// list code generated for this block
    long i;

    for (i = cx0; i <= cx - 1; i++) {
        printf("%10d%5s%3d%5d\n", i, mnemonic[code[i].f], code[i].l, code[i].a);
    }

}

void expression(unsigned long, bool nowArray, int index);
void factor(unsigned long fsys)
{
    long i;

    test(facbegsys, fsys, 24);

    if (sym & facbegsys)
    {
        if (sym == ident)
        {
            bool ar = false;
            i = position(id, tx);
            if (i == 0)
            {
                error(11);
            }
            else
            {
                switch (table[i].kind)
                {
                case constant:
                    ar = false;
                    gen(lit, 0, table[i].val);
                    break;
                case variable:
                    ar = false;
                    gen(lod, lev - table[i].level, table[i].addr);
                    break;
                case proc:
                    error(21);
                    break;
                case array:
                    ar = true;
                    getsym();
                    getsym();
                    expression(fsys, true, i);//�±��ֵ��ջ��
                    getsym();
                    gen(lit, 0, table[i].addr);
                    gen(opr, 0, 2);//��ǰջ������ʵ��ַ
                    gen(lod2, lev - table[i].level, 0);
                    break;
                }
            }
            if (!ar)
            {
                getsym();
            }
        }
        else
        {
            if (sym == number)
            {
                if (num > amax)
                {
                    error(31); num = 0;
                }
                gen(lit, 0, num);
                getsym();
            }
            else
            {
                if (sym == lparen)
                {
                    getsym();
                    expression(rparen | fsys, false, 0);
                    if (sym == rparen)
                    {
                        getsym();
                    }
                    else
                    {
                        error(22);
                    }
                }
                test(fsys, lparen, 23);
            }
        }
    }
}



void term(unsigned long fsys) {
    unsigned long mulop;

    factor(fsys | times | slash);
    while (sym == times || sym == slash) {
        mulop = sym;
        getsym();
        factor(fsys | times | slash);
        if (mulop == times) {
            gen(opr, 0, 4);
        }
        else {
            gen(opr, 0, 5);
        }
    }
}

void expression(unsigned long fsys, bool nowArray, int index) {
    unsigned long addop;

    if (sym == plus || sym == minus) {
        addop = sym; getsym();
        term(fsys | plus | minus);
        if (addop == minus) {
            gen(opr, 0, 1);
        }
    }
    else {
        term(fsys | plus | minus);
    }
    while (sym == plus || sym == minus) {
        addop = sym; getsym();
        term(fsys | plus | minus);
        if (addop == plus) {
            gen(opr, 0, 2);
        }
        else {
            gen(opr, 0, 3);
        }
    }
    //��������飬�±�Ӧ�ü�ȥ�½�ֵ���ܵõ���ʵ�ĵ�ֵַ
    if (nowArray == true) {
        gen(lit, 0, table[index].low);
        gen(opr, 0, 3);
    }
}

void condition(unsigned long fsys) {
    unsigned long relop;

    if (sym == oddsym) {
        getsym();
        expression(fsys, false, 0);
        gen(opr, 0, 6);
    }
    else {
        expression(fsys | eql | neq | lss | gtr | leq | geq, false, 0);
        if (!(sym&(eql | neq | lss | gtr | leq | geq))) {
            error(20);
        }
        else {
            relop = sym;
            getsym();
            expression(fsys, false, 0);
            switch (relop) {
            case eql:
                gen(opr, 0, 8);
                break;
            case neq:
                gen(opr, 0, 9);
                break;
            case lss:
                gen(opr, 0, 10);
                break;
            case geq:
                gen(opr, 0, 11);
                break;
            case gtr:
                gen(opr, 0, 12);
                break;
            case leq:
                gen(opr, 0, 13);
                break;
            }
        }
    }
}

/*
* ��䴦��
*/
void statement(unsigned long fsys)
{
    long i, cx1, cx2;

    if (sym == ident)
    {
        i = position(id, tx);
        if (i == 0) {
            error(11);
        }
        else
        {
            if (table[i].kind != variable && table[i].kind != array)
            {	// assignment to non-variable
                error(12); i = 0;
            }
            else
            {
                if (table[i].kind == variable)
                {
                    getsym();
                    if (sym == becomes)
                    {
                        getsym();
                    }
                    else
                    {
                        error(13);
                    }
                    expression(fsys, false, i);
                    if (i != 0)
                    {
                        /* expression��ִ��һϵ��ָ������ս�����ᱣ����ջ����ִ��sto������ɸ�ֵ */
                        gen(sto, lev - table[i].level, table[i].addr);
                    }
                }
                else
                {
                    getsym();

                    expression(fsys, true, i);//�±�ı��ʽ����ƫ�������Ѿ������½�ֵ���ŵ�ջ��

                                                            //gendo(lit, 0, table[i].low);
                                                            //gendo(opr, 0, 3);
                    gen(lit, 0, table[i].addr);//������ַ�ŵ�ջ��
                    gen(opr, 0, 2);//��ǰջ������ʵ��ַ
                    if (sym == becomes) {
                        getsym();
                    }
                    else {
                        error(13);
                    }
                    //memcpy(nxtlev, fsys, sizeof(bool)*symnum);
                    expression(fsys, false, i); /* ����ֵ�����Ҳ���ʽ */
                    if (i != 0)
                    {
                        /* expression��ִ��һϵ��ָ������ս�����ᱣ����ջ����ִ��sto������ɸ�ֵ */
                        gen(sto2, lev - table[i].level, 0);
                    }
                }
            }
        }
    }


    /*getsym();
    if (sym == becomes) {
        getsym();
    }
    else {
        error(13);
    }
    expression(fsys);
    if (i != 0) {
        gen(sto, lev - table[i].level, table[i].addr);
    }*/

    else
    {
        if (sym == readsym) /* ׼������read��䴦�� */
        {
            getsym();
            if (sym != lparen)
            {
                error(34);  /* ��ʽ����Ӧ�������� */
            }
            else
            {

                do {
                    getsym();
                    if (sym == ident)
                    {
                        i = position(id, tx); /* ����Ҫ���ı��� */
                    }
                    else
                    {
                        i = 0;
                    }

                    if (i == 0)
                    {
                        error(35);  /* read()��Ӧ���������ı����� */
                    }
                    else if (table[i].kind != variable && table[i].kind != array)
                    {
                        error(32);	/* read()������ı�ʶ�����Ǳ���, thanks to amd */
                    }
                    else
                    {

                        if (table[i].kind == variable) {//read(a);
                            gen(opr, 0, 16);  /* ��������ָ���ȡֵ��ջ�� */
                            gen(sto, lev - table[i].level, table[i].addr);   /* ���浽���� */
                            getsym();
                        }
                        else
                        {

                            getsym();

                            expression(fsys, true, i);//�����ڵı��ʽ����ƫ�����ŵ�ջ��
                                                                    //gendo(lit, 0, table[i].low);
                                                                    //gendo(opr, 0, 3);
                            gen(lit, 0, table[i].addr);//����ַ
                            gen(opr, 0, 2);//��ǰջ������ʵ��ַ
                            gen(opr, 0, 16);  /* ��������ָ���ȡֵ��ջ�� */
                            gen(sto2, lev - table[i].level, 0);
                            //gendo(sto, lev - table[i].level, table[i].adr);
                            //int ad = *(int*)(table[i].adr);
                            //gendo(sto, lev - table[i].level, table[i].adr);


                        }

                    }

                } while (sym == comma); /* һ��read���ɶ�������� */
            }
            if (sym != rparen)
            {
                error(33);  /* ��ʽ����Ӧ�������� */
                while (!(sym & fsys))   /* �����ȣ�ֱ���յ��ϲ㺯���ĺ������ */
                {
                    getsym();
                }
            }
            else
            {
                getsym();
            }
        }

        else {
            if (sym == writesym)    /* ׼������write��䴦����read���� */
            {
                getsym();
                if (sym == lparen)
                {
                    do {
                        getsym();
                        // memcpy(nxtlev, fsys, sizeof(bool)*symnum);
        /*                  nxtlev[rparen] = true;
                          nxtlev[comma] = true;       */    /* write�ĺ������Ϊ) or , */
                        expression(fsys, false, 0); /* ���ñ��ʽ�����˴���read��ͬ��readΪ��������ֵ */
                        gen(opr, 0, 14);  /* �������ָ����ջ����ֵ */
                    } while (sym == comma);
                    if (sym != rparen)
                    {
                        error(33);  /* write()��ӦΪ�������ʽ */
                    }
                    else
                    {
                        getsym();
                    }
                }
                gen(opr, 0, 15);  /* ������� */
            }
            else {
                if (sym == callsym)
                {
                    getsym();
                    if (sym != ident)
                    {
                        error(14);
                    }
                    else
                    {
                        i = position(id, tx);
                        if (i == 0) {
                            error(11);
                        }
                        else if (table[i].kind == proc)
                        {
                            gen(cal, lev - table[i].level, table[i].addr);
                        }
                        else
                        {
                            error(15);
                        }
                        getsym();
                    }
                }

                else {
                    if (sym == ifsym)
                    {
                        getsym();
                        condition(fsys | thensym | dosym);
                        if (sym == thensym)
                        {
                            getsym();
                        }
                        else {
                            error(16);
                        }
                        cx1 = cx;
                        gen(jpc, 0, 0);
                        statement(fsys);
                        code[cx1].a = cx;
                    }


                    else
                    {
                        if (sym == beginsym)
                        {
                            getsym();
                            statement(fsys | semicolon | endsym);

                            while (sym == semicolon || (sym&statbegsys))
                            {
                                if (sym == semicolon)
                                {
                                    getsym();
                                }
                                else
                                {
                                    error(10);
                                }
                                statement(fsys | semicolon | endsym);
                            }
                            //     sym = endsym;
                            if (sym == endsym)
                            {
                                getsym();
                            }
                            else
                            {
                                error(17);
                            }
                        }


                        else {
                            if (sym == whilesym)
                            {
                                cx1 = cx; getsym();
                                condition(fsys | dosym);
                                cx2 = cx;
                                gen(jpc, 0, 0);
                                if (sym == dosym)
                                {
                                    getsym();
                                }
                                else
                                {
                                    error(18);
                                }
                                statement(fsys);
                                gen(jmp, 0, cx1);
                                code[cx2].a = cx;
                            }

                            test(fsys, 0, 19);

                        }
                    }
                }
            }
        }
    }
    //test(fsys, 0, 19);
}



void block(unsigned long fsys) {
    long tx0;		// initial table index
    long cx0; 		// initial code index
    long tx1;		// save current table index before processing nested procedures
    long dx1;		// save data allocation index

    dx = 3; tx0 = tx; table[tx].addr = cx; gen(jmp, 0, 0);
    if (lev > levmax) {
        error(32);
    }
    do
    {
        if (sym == constsym)    /* �յ������������ţ���ʼ���������� */
        {
            getsym();
            do
            {
                constdeclaration();
                while (sym == comma) {
                    getsym();
                    constdeclaration();
                }
                if (sym == semicolon) {
                    getsym();
                }
                else {
                    error(5);   /*©���˶��Ż��߷ֺ�*/
                }
            } while (sym == ident);
        }

        if (sym == varsym) {    /* �յ������������ţ���ʼ����������� */
            getsym();
            do {
                vardeclaration();
                while (sym == comma) {
                    getsym();
                    vardeclaration();
                }
                if (sym == semicolon) {
                    getsym();
                }
                else {
                    error(5);
                }
            } while (sym == ident);
        }

        while (sym == procsym) {    /* �յ������������ţ���ʼ����������� */
            getsym();
            if (sym == ident) {
                enter(proc);
                getsym();
            }
            else {
                error(4);
            }
            if (sym == semicolon) {
                getsym();
            }
            else {
                error(5);
            }

            lev = lev + 1; tx1 = tx; dx1 = dx;
            block(fsys | semicolon);
            lev = lev - 1; tx = tx1; dx = dx1;

            if (sym == semicolon) {
                getsym();
                test(statbegsys | ident | procsym, fsys, 6);
            }
            else {
                error(5);
            }
        }
        test(statbegsys | ident, declbegsys, 7);
    } while (sym&declbegsys);

    code[table[tx0].addr].a = cx;
    table[tx0].addr = cx;		// start addr of code
    cx0 = cx;
    gen(Int, 0, dx);

    statement(fsys | semicolon | endsym);
    gen(opr, 0, 0); // return
    test(fsys, 0, 8);
    listcode(cx0);
}

long base(long b, long l) {
    long b1;

    b1 = b;
    while (l > 0) {	// find base l levels down
        b1 = s[b1]; l = l - 1;
    }
    return b1;
}

void interpret() {
    long p, b, t;		// program-, base-, topstack-registers
    instruction i;	// instruction register

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
            switch (i.a) { 	// operator
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
            }
            break;
        case lod:
            t = t + 1; s[t] = s[base(b, i.l) + i.a];
            break;
        case lod2:
            // s[t - 1] = s[base(i.l, s, b) + s[t - 1]];//Ӧ�����ǻ�ַ��ƫ��������λ�ã��Ա�֤���Խ��������ж�
            s[t] = s[base(b, i.l) + s[t]];
            break;
        case sto:
            s[base(b, i.l) + i.a] = s[t]; printf("%10d\n", s[t]); t = t - 1;
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

main() {
    long i;
    for (i = 0; i < 256; i++) {
        ssym[i] = nul;
    }
    strcpy(word[0], "begin     ");
    strcpy(word[1], "call      ");
    strcpy(word[2], "const     ");
    strcpy(word[3], "do        ");
    strcpy(word[4], "end       ");
    strcpy(word[5], "if        ");
    strcpy(word[6], "odd       ");
    strcpy(word[7], "procedure ");
    strcpy(word[8], "then      ");
    strcpy(word[9], "var       ");
    strcpy(word[10], "while     ");
    strcpy(word[11], "read");
    strcpy(word[12], "write");
    //strcpy(word[13], "");

    wsym[0] = beginsym;
    wsym[1] = callsym;
    wsym[2] = constsym;
    wsym[3] = dosym;
    wsym[4] = endsym;
    wsym[5] = ifsym;
    wsym[6] = oddsym;
    wsym[7] = procsym;
    wsym[8] = thensym;
    wsym[9] = varsym;
    wsym[10] = whilesym;
    wsym[11] = readsym;
    wsym[12] = writesym;
    ssym['+'] = plus;
    ssym['-'] = minus;
    ssym['*'] = times;
    ssym['/'] = slash;
    ssym['('] = lparen;
    ssym[')'] = rparen;
    ssym['='] = eql;
    ssym[','] = comma;
    ssym['.'] = period;
    ssym[';'] = semicolon;
    ssym[':'] = colon;

    strcpy(mnemonic[lit], "lit");
    strcpy(mnemonic[opr], "opr");
    strcpy(mnemonic[lod], "lod");
    strcpy(mnemonic[sto], "sto");
    strcpy(mnemonic[cal], "cal");
    strcpy(mnemonic[Int], "int");
    strcpy(mnemonic[jmp], "jmp");
    strcpy(mnemonic[jpc], "jpc");
    strcpy(mnemonic[sto2], "sto2");
    strcpy(mnemonic[lod2], "lod2");
    declbegsys = constsym | varsym | procsym;
    statbegsys = beginsym | callsym | ifsym | whilesym;
    facbegsys = ident | number | lparen;

    printf("please input source program file name: ");
    scanf("%s", infilename);
    printf("\n");
    if ((infile = fopen(infilename, "r")) == NULL) {
        printf("File %s can't be opened.\n", infilename);
        exit(1);
    }

    err = 0;
    cc = 0; cx = 0; ll = 0; ch = ' '; kk = al; getsym();
    lev = 0; tx = 0;
    block(declbegsys | statbegsys | period);
    if (sym != period) {
        error(9);
    }
    // err = 0;
    if (err == 0) {
        interpret();
    }
    else {
        printf("errors in PL/0 program\n");
        // printf("%d", err);
    }
    fclose(infile);
}
