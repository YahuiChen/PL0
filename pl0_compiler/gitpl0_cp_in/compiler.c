// pl/0 compiler with code generation
#include <stdlib.h>
#include <string.h>
#include "compiler.h"

void error(long n) {
    char space[81];
    memset(space, 32, 81);

    space[cc - 1] = 0; //����ʱ��ǰ�����Ѿ����꣬����cc-1

    printf("****%s!%d\n", space, n);
    err++;
}

void getch()
{
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

        if (k >= kk)
        {
            kk = k;
        }
        else
        {
            do
            {
                kk = kk - 1; a[kk] = ' ';
            } while (k < kk);
        }

        strcpy(id, a);
        i = 0;
        j = norw - 1;

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
    else if (ch == ':')
    {
        getch();
        if (ch == '=') {
            sym = becomes;
            getch();
        }
        else
        {
            sym = colon;
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
    else
    {
        if (ch == '/')		// to handle the notes beginning with "/*"
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
            else if (ch == '/') {
                cc = ll;
                getch();
                getsym();
            }
            else {
                //error(0);
                sym = ssym['/'];
            }
        }

        else {
            sym = ssym[(unsigned char)ch];
            if (sym != period)
            {
                getch();
            }
        }
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

int isV(long index)
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
                error(11);  /* ��ʶ��δ���� */
            }
            else
            {
                switch (table[i].kind)
                {
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
                //test(fsys, lparen, 23);
                //test(fsys, facbegsys, 23);
            }
        }
    }
}

void term(unsigned long fsys)
{
    unsigned long mulop;
    /*long elx1;
    elx1 = elx;
    */
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

    //if (elx > elx1)                // ����д���and��·��ת
    //{
    //    gen(jmp, 0, cx + 2);         // ��������ִ����������һ��ָ��
    //    while (elx > elx1)          // ����֮ǰ��jpc��ַ
    //    {
    //        elx--;
    //        code[exitlist[elx]].a = cx;
    //    }
    //    gen(lit, 0, 0);            // ��·��ת�ѽ��ǿ����Ϊ0
    //}
}

void expression(unsigned long fsys, bool nowArray, int index) {
    unsigned long addop;

    if (sym == plus || sym == minus)
    {
        addop = sym;
        getsym();
        term(fsys | plus | minus);
        if (addop == minus)
        {
            gen(opr, 0, 1);
        }
    }
    else {
        term(fsys | plus | minus);
    }
    while (sym == plus || sym == minus)
    {
        addop = sym;
        getsym();
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
        else
        {
            relop = sym;
            getsym();
            expression(fsys, false, 0);
            switch (relop)
            {
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
        if (i == 0)
        {
            error(11);   /* ����δ�ҵ� */
        }
        else
        {
            if (table[i].kind != variable && table[i].kind != array)
            {	// assignment to non-variable
                error(12);
                i = 0;
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
                do
                {
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
                            //getsym();
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
        else
        {
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
                        expression(fsys | rparen | comma, false, 0); /* ���ñ��ʽ�����˴���read��ͬ��readΪ��������ֵ */
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
            else
            {
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

                else
                {
                    if (sym == ifsym)
                    {
                        getsym();
                        condition(fsys | thensym | dosym | elsesym);
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
                        // code[cx1].a = cx;
                        cx3 = cx;
                        gen(jmp, 0, 0);//������ֱ����ת��else������

                        code[cx1].a = cx;   /* ��statement�����cxΪthen�����ִ�����λ�ã�������ǰ��δ������ת��ַ */
                        if (sym == elsesym)
                        {
                            getsym();
                            statement(fsys);
                            code[cx3].a = cx;//��ǰ��else�����������λ�ã�if���ִ�к�Ӧ����ת����
                        }
                    }
                    else
                    {
                        if (sym == beginsym)
                        {
                            getsym();
                            statement(fsys | semicolon | endsym);

                            while ((sym&statbegsys) || sym == semicolon)
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
                                //statement(fsys);
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

                        else
                        {
                            if (sym == whilesym)
                            {
                                cx1 = cx;
                                getsym();
                                condition(fsys | dosym | exitsym);
                                //expression(fsys | dosym, false, 0);

                                // cx2 = cx;
                                cx2 = elx;                 // ��¼�µ�ǰ�㿪ʼ��elx
                                exitlist[elx] = cx;        // ��jpc��λ�ü�¼��exitlist
                                elx++;

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
                                //statement(fsys | exitsym);
                                gen(jmp, 0, cx1);

                                //code[cx2].a = cx;
                                while (elx > cx2)           // ��exitlist�м�¼�µ���ת���ĵ�ַ��������
                                {
                                    elx--;
                                    code[exitlist[elx]].a = cx;
                                }
                            }
                            else
                            {
                                if (sym == exitsym)
                                {
                                    test(fsys, 0, 34);     // exit��䱨����

                                    exitlist[elx] = cx;    // ��jpc��λ�ü�¼��exitlist
                                    elx++;
                                    gen(jmp, 0, 0);

                                    getsym();
                                }

                                else
                                {
                                    test(fsys, 0, 19);
                                }
                            }
                        }
                    }
                }
            }
        }
        //test(fsys, 0, 19);
    }
}

//int block(int lev, int  tx, unsigned long fsys)
void block(unsigned long fsys)
{
    long i;

    long tx0;		// initial table index
    long cx0; 		// initial code index
    long tx1;		// save current table index before processing nested procedures
    long dx1;		// save data allocation index
                    /*bool nxtlev[33]; *//* ���¼������Ĳ����У����ż��Ͼ�Ϊֵ�Σ�������ʹ������ʵ�֣�
                                         ���ݽ�������ָ�룬Ϊ��ֹ�¼������ı��ϼ������ļ��ϣ������µĿ�?
                                         ���ݸ��¼�����*/

    dx = 3; tx0 = tx; table[tx].addr = cx; gen(jmp, 0, 0);
    if (lev > levmax)
    {
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
            //memcpy(nxtlev, fsys, sizeof(bool) * 33);
            //nxtlev[semicolon] = true;
            //if (-1 == block(lev + 1, tx, nxtlev))
            //{
            //    return -1;  /* �ݹ���� */
            //}

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

void list_code(FILE *fout, instruction const *const first, instruction const *const last)
{
    instruction const *ite = first;
    for (; ite != last; ++ite)
    {
        fprintf(fout, "%5s%3d%15d\n", mnemonic[ite->f], ite->l, ite->a);
        /*fwrite(mnemonic[ite->f], sizeof(enum fct), 1, fout);
        fwrite(ite->l, sizeof(long), 1, fout);
        fwrite(ite->a, sizeof(long), 1, fout);*/
    }
}
main()
{
    long i;
    for (i = 0; i < 256; i++)
    {
        ssym[i] = nul;
    }
    strcpy(word[0], "begin     ");
    strcpy(word[1], "call      ");
    strcpy(word[2], "const     ");
    strcpy(word[3], "do        ");
    strcpy(word[4], "else      ");
    strcpy(word[5], "end       ");
    strcpy(word[6], "exit      ");
    strcpy(word[7], "if        ");
    strcpy(word[8], "odd       ");
    strcpy(word[9], "procedure ");
    strcpy(word[10], "read      ");
    strcpy(word[11], "then      ");
    strcpy(word[12], "var       ");
    strcpy(word[13], "while     ");
    strcpy(word[14], "write     ");

    /*strcpy(word[0], "begin     ");
    strcpy(word[1], "call      ");
    strcpy(word[2], "const     ");
    strcpy(word[3], "do        ");
    strcpy(word[4], "else      ");
    strcpy(word[5], "end       ");
    strcpy(word[6], "if        ");
    strcpy(word[7], "odd       ");
    strcpy(word[8], "procedure ");
    strcpy(word[9], "read      ");
    strcpy(word[10], "then      ");
    strcpy(word[11], "var       ");
    strcpy(word[12], "while     ");
    strcpy(word[13], "write     ");*/

    wsym[0] = beginsym;
    wsym[1] = callsym;
    wsym[2] = constsym;
    wsym[3] = dosym;
    wsym[4] = elsesym;
    wsym[5] = endsym;
    wsym[6] = exitsym;
    wsym[7] = ifsym;
    wsym[8] = oddsym;
    wsym[9] = procsym;
    wsym[10] = readsym;
    wsym[11] = thensym;
    wsym[12] = varsym;
    wsym[13] = whilesym;
    //wsym[11] = readsym;
    wsym[14] = writesym;

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
    statbegsys = beginsym | callsym | ifsym | whilesym | readsym | writesym;
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
    elx = 0;                       // exitlist����
                                   //etlx = 0;                      // enterlist����

    block(declbegsys | statbegsys | period);
    if (sym != period) {
        error(9);
    }
    // err = 0;
    if (err == 0) {
        //  interpret();
        FILE *outfile = NULL;
        char outfilename[80];
        sprintf(outfilename, "%s%c", infilename, 'c'); // IR ��չ��Ϊ pl0c
        outfile = fopen(outfilename, "w");
        list_code(outfile, code, code + cx);
        printf("Output IR to file %s.\n", outfilename);
        fclose(outfile);
    }
    else {
        printf("errors in PL/0 program\n");
        // printf("%d", err);
    }
    fclose(infile);
}