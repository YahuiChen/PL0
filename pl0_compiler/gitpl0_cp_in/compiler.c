// pl/0 compiler with code generation
#include <stdlib.h>
#include <string.h>
#include "compiler.h"

void error(long n) {
    char space[81];
    memset(space, 32, 81);

    space[cc - 1] = 0; //出错时当前符号已经读完，所以cc-1

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

        do {								//折半查找
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
    else if (isdigit(ch)) { // 数字
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
* start:   数组开始位置
* end:     数组结束为止
*
* 其他变量同enter方法
*/
void enterArray(int start, int end, char* id) {
    tx++;//增加名字表内容
    strcpy(table[tx].name, id);
    table[tx].kind = array;//数组类型
    table[tx].level = lev;//层次
    table[tx].addr = dx;//数组首地址
    table[tx].low = start;
    dx += (end - start + 1);//连续内存地址分配给该数组
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
            char mid[11];//存储变量名字，用于填写名字表
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
                        error(31);//不能是变量
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
                                    error(31);//不能是变量
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
                                        error(31);//缺少右括号
                                    }
                                }
                                else
                                {
                                    error(31);//下界大于上界
                                }
                            }
                        }
                    }
                    else
                    {
                        error(31);//缺少冒号
                    }
                }
            }
            else
            {
                error(31);//只能是数字或者标识符
            }
            getsym();
        }
        else
        {
            //int型
            enter(variable);
            // 填写名字表
        }
    }
    else
    {
        error(4);   /* var后应是标识 */
    }
    return 0;
}

int isV(long index)
{
    if (index == 0 || table[index].kind != constant)
    {
        error(31);//常量未找到
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
                error(11);  /* 标识符未声明 */
            }
            else
            {
                switch (table[i].kind)
                {
                case array:
                    ar = true;
                    getsym();
                    getsym();
                    expression(fsys, true, i);//下标的值在栈顶
                    getsym();
                    gen(lit, 0, table[i].addr);
                    gen(opr, 0, 2);//当前栈顶是真实地址
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

    //if (elx > elx1)                // 语句中存在and短路跳转
    //{
    //    gen(jmp, 0, cx + 2);         // 若是正常执行则跳过下一句指令
    //    while (elx > elx1)          // 补上之前的jpc地址
    //    {
    //        elx--;
    //        code[exitlist[elx]].a = cx;
    //    }
    //    gen(lit, 0, 0);            // 短路跳转把结果强制置为0
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
    //如果是数组，下标应该减去下界值才能得到真实的地址值
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
* 语句处理
*/
void statement(unsigned long fsys)
{
    long i, cx1, cx2;

    if (sym == ident)
    {
        i = position(id, tx);
        if (i == 0)
        {
            error(11);   /* 变量未找到 */
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
                        /* expression将执行一系列指令，但最终结果将会保存在栈顶，执行sto命令完成赋值 */
                        gen(sto, lev - table[i].level, table[i].addr);
                    }
                }
                else
                {
                    getsym();

                    expression(fsys, true, i);//下标的表达式，将偏移量（已经减掉下界值）放到栈顶

                                              //gendo(lit, 0, table[i].low);
                                              //gendo(opr, 0, 3);
                    gen(lit, 0, table[i].addr);//将基地址放到栈顶
                    gen(opr, 0, 2);//当前栈顶是真实地址
                    if (sym == becomes) {
                        getsym();
                    }
                    else {
                        error(13);
                    }
                    //memcpy(nxtlev, fsys, sizeof(bool)*symnum);
                    expression(fsys, false, i); /* 处理赋值符号右侧表达式 */
                    if (i != 0)
                    {
                        /* expression将执行一系列指令，但最终结果将会保存在栈顶，执行sto命令完成赋值 */
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
        if (sym == readsym) /* 准备按照read语句处理 */
        {
            getsym();
            if (sym != lparen)
            {
                error(34);  /* 格式错误，应是左括号 */
            }
            else
            {
                do
                {
                    getsym();
                    if (sym == ident)
                    {
                        i = position(id, tx); /* 查找要读的变量 */
                    }
                    else
                    {
                        i = 0;
                    }

                    if (i == 0)
                    {
                        error(35);  /* read()中应是声明过的变量名 */
                    }
                    else if (table[i].kind != variable && table[i].kind != array)
                    {
                        error(32);	/* read()参数表的标识符不是变量, thanks to amd */
                    }
                    else
                    {
                        if (table[i].kind == variable) {//read(a);
                            gen(opr, 0, 16);  /* 生成输入指令，读取值到栈顶 */
                            gen(sto, lev - table[i].level, table[i].addr);   /* 储存到变量 */
                            getsym();
                        }
                        else
                        {
                            getsym();
                            expression(fsys, true, i);//括号内的表达式，将偏移量放到栈顶
                                                      //gendo(lit, 0, table[i].low);
                                                      //gendo(opr, 0, 3);
                            gen(lit, 0, table[i].addr);//基地址
                            gen(opr, 0, 2);//当前栈顶是真实地址
                            gen(opr, 0, 16);  /* 生成输入指令，读取值到栈顶 */
                            gen(sto2, lev - table[i].level, 0);
                            //gendo(sto, lev - table[i].level, table[i].adr);
                            //int ad = *(int*)(table[i].adr);
                            //gendo(sto, lev - table[i].level, table[i].adr);
                            //getsym();
                        }
                    }
                } while (sym == comma); /* 一条read语句可读多个变量 */
            }
            if (sym != rparen)
            {
                error(33);  /* 格式错误，应是右括号 */
                while (!(sym & fsys))   /* 出错补救，直到收到上层函数的后跟符号 */
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
            if (sym == writesym)    /* 准备按照write语句处理，与read类似 */
            {
                getsym();
                if (sym == lparen)
                {
                    do {
                        getsym();
                        // memcpy(nxtlev, fsys, sizeof(bool)*symnum);
                        /*                  nxtlev[rparen] = true;
                        nxtlev[comma] = true;       */    /* write的后跟符号为) or , */
                        expression(fsys | rparen | comma, false, 0); /* 调用表达式处理，此处与read不同，read为给变量赋值 */
                        gen(opr, 0, 14);  /* 生成输出指令，输出栈顶的值 */
                    } while (sym == comma);

                    if (sym != rparen)
                    {
                        error(33);  /* write()中应为完整表达式 */
                    }
                    else
                    {
                        getsym();
                    }
                }
                gen(opr, 0, 15);  /* 输出换行 */
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
                        gen(jmp, 0, 0);//将来会直接跳转到else语句后面

                        code[cx1].a = cx;   /* 经statement处理后，cx为then后语句执行完的位置，它正是前面未定的跳转地址 */
                        if (sym == elsesym)
                        {
                            getsym();
                            statement(fsys);
                            code[cx3].a = cx;//当前是else后面的语句结束位置，if语句执行后应当跳转至此
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
                                cx2 = elx;                 // 记录下当前层开始的elx
                                exitlist[elx] = cx;        // 将jpc的位置记录进exitlist
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
                                while (elx > cx2)           // 将exitlist中记录下的跳转语句的地址补充完整
                                {
                                    elx--;
                                    code[exitlist[elx]].a = cx;
                                }
                            }
                            else
                            {
                                if (sym == exitsym)
                                {
                                    test(fsys, 0, 34);     // exit语句报错处理

                                    exitlist[elx] = cx;    // 将jpc的位置记录进exitlist
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
                    /*bool nxtlev[33]; *//* 在下级函数的参数中，符号集合均为值参，但由于使用数组实现，
                                         传递进来的是指针，为防止下级函数改变上级函数的集合，开辟新的空?
                                         传递给下级函数*/

    dx = 3; tx0 = tx; table[tx].addr = cx; gen(jmp, 0, 0);
    if (lev > levmax)
    {
        error(32);
    }
    do
    {
        if (sym == constsym)    /* 收到常量声明符号，开始处理常量声明 */
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
                    error(5);   /*漏掉了逗号或者分号*/
                }
            } while (sym == ident);
        }

        if (sym == varsym) {    /* 收到变量声明符号，开始处理变量声明 */
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

        while (sym == procsym) {    /* 收到过程声明符号，开始处理过程声明 */
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
            //    return -1;  /* 递归调用 */
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
    elx = 0;                       // exitlist清零
                                   //etlx = 0;                      // enterlist清零

    block(declbegsys | statbegsys | period);
    if (sym != period) {
        error(9);
    }
    // err = 0;
    if (err == 0) {
        //  interpret();
        FILE *outfile = NULL;
        char outfilename[80];
        sprintf(outfilename, "%s%c", infilename, 'c'); // IR 扩展名为 pl0c
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