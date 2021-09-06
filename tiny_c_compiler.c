/*
  Obfuscated Tiny C Compiler with ELF output

  Copyright (C) 2001-2003 Fabrice Bellard

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product and its documentation 
     *is* required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/
#ifndef TINY
#include <stdarg.h>
#endif
#include <stdio.h>

/* vars: value of variables 
   loc : local variable index
   glo : global variable ptr
   data: base of data segment
   ind : output code ptr
   prog: output code
   rsym: return symbol
   sym_stk: symbol stack
   dstk: symbol stack pointer
   dptr, dch: macro state

   * 'vars' format: 
   For each character TAG_TOK at offset 'i' before a
    symbol in sym_stk, we have:
    v = (int *)(vars + 8 * i + TOK_IDENT)[0] 
    p = (int *)(vars + 8 * i + TOK_IDENT)[0] 
    
    v = 0    : undefined symbol, p = list of use points.
    v = 1    : define symbol, p = pointer to define text.
    v < LOCAL: offset on stack, p = 0.
    otherwise: symbol with value 'v', p = list of use points.

   * 'sym_stk' format:
   TAG_TOK sym1 TAG_TOK sym2 .... symN '\0'
   'dstk' points to the last '\0'.
*/
int tok, tokc, tokl, ch, vars, rsym, prog, ind, loc, glo, file, sym_stk, dstk, dptr, dch, last_id, data, text, data_offset;

#define ALLOC_SIZE 99999

#define ELFOUT

/* depends on the init string */
#define TOK_STR_SIZE 48
#define TOK_IDENT    0x100
#define TOK_INT      0x100
#define TOK_IF       0x120
#define TOK_ELSE     0x138
#define TOK_WHILE    0x160
#define TOK_BREAK    0x190
#define TOK_RETURN   0x1c0
#define TOK_FOR      0x1f8
#define TOK_DEFINE   0x218
#define TOK_MAIN     0x250

#define TOK_DUMMY   1
#define TOK_NUM     2

#define LOCAL   0x200

#define SYM_FORWARD 0
#define SYM_DEFINE  1

/* tokens in string heap */
#define TAG_TOK    ' '
#define TAG_MACRO  2

/* additionnal elf output defines */
#ifdef ELFOUT

#define ELF_BASE      0x08048000
#define PHDR_OFFSET   0x30

#define INTERP_OFFSET 0x90
#define INTERP_SIZE   0x13

#ifndef TINY
#define DYNAMIC_OFFSET (INTERP_OFFSET + INTERP_SIZE + 1)
#define DYNAMIC_SIZE   (11*8)

#define ELFSTART_SIZE  (DYNAMIC_OFFSET + DYNAMIC_SIZE)
#else
#define DYNAMIC_OFFSET 0xa4
#define DYNAMIC_SIZE   0x58

#define ELFSTART_SIZE  0xfc
#endif

/* size of startup code */
#define STARTUP_SIZE   17

/* size of library names at the start of the .dynstr section */
#define DYNSTR_BASE      22

#endif

pdef(t)
{
    *(char *)dstk++ = t;
}

inp()
{
    if (dptr) {
        ch = *(char *)dptr++;
        if (ch == TAG_MACRO) {
            dptr = 0;
            ch = dch;
        }
    } else
        ch = fgetc(file);
    /*    printf("ch=%c 0x%x\n", ch, ch); */
}

isid()
{
    return isalnum(ch) | ch == '_';
}

/* read a character constant */
getq()
{
    if (ch == '\\') {
        inp();
        if (ch == 'n')
            ch = '\n';
    }
}

next()
{
    int t, l, a;

    while (isspace(ch) | ch == '#') {
        if (ch == '#') {
            inp();
            next();
            if (tok == TOK_DEFINE) {
                next();
                pdef(TAG_TOK); /* fill last ident tag */
                *(int *)tok = SYM_DEFINE;
                *(int *)(tok + 4) = dstk; /* define stack */
            }
            /* well we always save the values ! */
            while (ch != '\n') {
                pdef(ch);
                inp();
            }
            pdef(ch);
            pdef(TAG_MACRO);
        }
        inp();
    }
    tokl = 0;
    tok = ch;
    /* encode identifiers & numbers */
    if (isid()) {
        pdef(TAG_TOK);
        last_id = dstk;
        while (isid()) {
            pdef(ch);
            inp();
        }
        if (isdigit(tok)) {
            tokc = strtol(last_id, 0, 0);
            tok = TOK_NUM;
        } else {
            *(char *)dstk = TAG_TOK; /* no need to mark end of string (we
                                        suppose data is initied to zero */
            tok = strstr(sym_stk, last_id - 1) - sym_stk;
            *(char *)dstk = 0;   /* mark real end of ident for dlsym() */
            tok = tok * 8 + TOK_IDENT;
            if (tok > TOK_DEFINE) {
                tok = vars + tok;
                /*        printf("tok=%s %x\n", last_id, tok); */
                /* define handling */
                if (*(int *)tok == SYM_DEFINE) {
                    dptr = *(int *)(tok + 4);
                    dch = ch;
                    inp();
                    next();
                }
            }
        }
    } else {
        inp();
        if (tok == '\'') {
            tok = TOK_NUM;
            getq();
            tokc = ch;
            inp();
            inp();
        } else if (tok == '/' & ch == '*') {
            inp();
            while (ch) {
                while (ch != '*')
                    inp();
                inp();
                if (ch == '/')
                    ch = 0;
            }
            inp();
            next();
        } else
        {
            t = "++#m--%am*@R<^1c/@%[_[H3c%@%[_[H3c+@.B#d-@%:_^BKd<<Z/03e>>`/03e<=0f>=/f<@.f>@1f==&g!=\'g&&k||#l&@.BCh^@.BSi|@.B+j~@/%Yd!@&d*@b";
            while (l = *(char *)t++) {
                a = *(char *)t++;
                tokc = 0;
                while ((tokl = *(char *)t++ - 'b') < 0)
                    tokc = tokc * 64 + tokl + 64;
                if (l == tok & (a == ch | a == '@')) {
#if 0
                    printf("%c%c -> tokl=%d tokc=0x%x\n", 
                           l, a, tokl, tokc);
#endif
                    if (a == ch) {
                        inp();
                        tok = TOK_DUMMY; /* dummy token for double tokens */
                    }
                    break;
                }
            }
        }
    }
#if 0
    {
        int p;

        printf("tok=0x%x ", tok);
        if (tok >= TOK_IDENT) {
            printf("'");
            if (tok > TOK_DEFINE) 
                p = sym_stk + 1 + (tok - vars - TOK_IDENT) / 8;
            else
                p = sym_stk + 1 + (tok - TOK_IDENT) / 8;
            while (*(char *)p != TAG_TOK && *(char *)p)
                printf("%c", *(char *)p++);
            printf("'\n");
        } else if (tok == TOK_NUM) {
            printf("%d\n", tokc);
        } else {
            printf("'%c'\n", tok);
        }
    }
#endif
}

#ifdef TINY
#define skip(c) next()
#else

void error(char *fmt,...)
{
    va_list ap;

    va_start(ap, fmt);
    fprintf(stderr, "%d: ", ftell((FILE *)file));
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
    va_end(ap);
}

void skip(c)
{
    if (tok != c) {
        error("'%c' expected", c);
    }
    next();
}

#endif

/* from 0 to 4 bytes */
o(n)
{
    /* cannot use unsigned, so we must do a hack */
    while (n && n != -1) {
        *(char *)ind++ = n;
        n = n >> 8;
    }
}

#ifdef ELFOUT

/* put a 32 bit little endian word 'n' at unaligned address 't' */
put32(t, n)
{
    *(char *)t++ = n;
    *(char *)t++ = n >> 8;
    *(char *)t++ = n >> 16;
    *(char *)t++ = n >> 24;
}

/* get a 32 bit little endian word at unaligned address 't' */
get32(t)
{
    int n;
    return  (*(char *)t & 0xff) |
        (*(char *)(t + 1) & 0xff) << 8 |
        (*(char *)(t + 2) & 0xff) << 16 |
        (*(char *)(t + 3) & 0xff) << 24;
}

#else

#define put32(t, n) *(int *)t = n
#define get32(t) *(int *)t

#endif

/* output a symbol and patch all references to it */
gsym1(t, b)
{
    int n;
    while (t) {
        n = get32(t); /* next value */
        /* patch absolute reference (always mov/lea before) */
        if (*(char *)(t - 1) == 0x05) {
            /* XXX: incorrect if data < 0 */
            if (b >= data && b < glo)
                put32(t, b + data_offset);
            else
                put32(t, b - prog + text + data_offset);
        } else {
            put32(t, b - t - 4);
        }
        t = n;
    }
}

gsym(t)
{
    gsym1(t, ind);
}

/* psym is used to put an instruction with a data field which is a
   reference to a symbol. It is in fact the same as oad ! */
#define psym oad

/* instruction + address */
oad(n, t)
{
    o(n);
    put32(ind, t);
    t = ind;
    ind = ind + 4;
    return t;
}

/* load immediate value */
li(t)
{
    oad(0xb8, t); /* mov $xx, %eax */
}

gjmp(t)
{
    return psym(0xe9, t);
}

/* l = 0: je, l == 1: jne */
gtst(l, t)
{
    o(0x0fc085); /* test %eax, %eax, je/jne xxx */
    return psym(0x84 + l, t);
}

gcmp(t)
{
    o(0xc139); /* cmp %eax,%ecx */
    li(0);
    o(0x0f); /* setxx %al */
    o(t + 0x90);
    o(0xc0);
}

gmov(l, t)
{
    int n;
    o(l + 0x83);
    n = *(int *)t;
    if (n && n < LOCAL)
        oad(0x85, n);
    else {
        t = t + 4;
        *(int *)t = psym(0x05, *(int *)t);
    }
}

/* l is one if '=' parsing wanted (quick hack) */
unary(l)
{
    int n, t, a, c;

    n = 1; /* type of expression 0 = forward, 1 = value, other =
              lvalue */
    if (tok == '\"') {
        li(glo + data_offset);
        while (ch != '\"') {
            getq();
            *(char *)glo++ = ch;
            inp();
        }
        *(char *)glo = 0;
        glo = glo + 4 & -4; /* align heap */
        inp();
        next();
    } else {
        c = tokl;
        a = tokc;
        t = tok;
        next();
        if (t == TOK_NUM) {
            li(a);
        } else if (c == 2) {
            /* -, +, !, ~ */
            unary(0);
            oad(0xb9, 0); /* movl $0, %ecx */
            if (t == '!')
                gcmp(a);
            else
                o(a);
        } else if (t == '(') {
            expr();
            skip(')');
        } else if (t == '*') {
            /* parse cast */
            skip('(');
            t = tok; /* get type */
            next(); /* skip int/char/void */
            next(); /* skip '*' or '(' */
            if (tok == '*') {
                /* function type */
                skip('*');
                skip(')');
                skip('(');
                skip(')');
                t = 0;
            }
            skip(')');
            unary(0);
            if (tok == '=') {
                next();
                o(0x50); /* push %eax */
                expr();
                o(0x59); /* pop %ecx */
                o(0x0188 + (t == TOK_INT)); /* movl %eax/%al, (%ecx) */
            } else if (t) {
                if (t == TOK_INT)
                    o(0x8b); /* mov (%eax), %eax */
                else 
                    o(0xbe0f); /* movsbl (%eax), %eax */
                ind++; /* add zero in code */
            }
        } else if (t == '&') {
            gmov(10, tok); /* leal EA, %eax */
            next();
        } else {
            n = 0;
            if (tok == '=' & l) {
                /* assignment */
                next();
                expr();
                gmov(6, t); /* mov %eax, EA */
            } else if (tok != '(') {
                /* variable */
                gmov(8, t); /* mov EA, %eax */
                if (tokl == 11) {
                    gmov(0, t);
                    o(tokc);
                    next();
                }
            }
        }
    }

    /* function call */
    if (tok == '(') {
        if (n)
            o(0x50); /* push %eax */

        /* push args and invert order */
        a = oad(0xec81, 0); /* sub $xxx, %esp */
        next();
        l = 0;
        while(tok != ')') {
            expr();
            oad(0x248489, l); /* movl %eax, xxx(%esp) */
            if (tok == ',')
                next();
            l = l + 4;
        }
        put32(a, l);
        next();
        if (n) {
            oad(0x2494ff, l); /* call *xxx(%esp) */
            l = l + 4;
        } else {
            /* forward reference */
            t = t + 4;
            *(int *)t = psym(0xe8, *(int *)t);
        }
        if (l)
            oad(0xc481, l); /* add $xxx, %esp */
    }
}

sum(l)
{
    int t, n, a;

    if (l-- == 1)
        unary(1);
    else {
        sum(l);
        a = 0;
        while (l == tokl) {
            n = tok;
            t = tokc;
            next();

            if (l > 8) {
                a = gtst(t, a); /* && and || output code generation */
                sum(l);
            } else {
                o(0x50); /* push %eax */
                sum(l);
                o(0x59); /* pop %ecx */
                
                if (l == 4 | l == 5) {
                    gcmp(t);
                } else {
                    o(t);
                    if (n == '%')
                        o(0x92); /* xchg %edx, %eax */
                }
            }
        }
        /* && and || output code generation */
        if (a && l > 8) {
            a = gtst(t, a);
            li(t ^ 1);
            gjmp(5); /* jmp $ + 5 */
            gsym(a);
            li(t);
        }
    }
}

expr()
{
    sum(11);
}


test_expr()
{
    expr();
    return gtst(0, 0);
}

block(l)
{
    int a, n, t;

    if (tok == TOK_IF) {
        next();
        skip('(');
        a = test_expr();
        skip(')');
        block(l);
        if (tok == TOK_ELSE) {
            next();
            n = gjmp(0); /* jmp */
            gsym(a);
            block(l);
            gsym(n); /* patch else jmp */
        } else {
            gsym(a); /* patch if test */
        }
    } else if (tok == TOK_WHILE | tok == TOK_FOR) {
        t = tok;
        next();
        skip('(');
        if (t == TOK_WHILE) {
            n = ind;
            a = test_expr();
        } else {
            if (tok != ';')
                expr();
            skip(';');
            n = ind;
            a = 0;
            if (tok != ';')
                a = test_expr();
            skip(';');
            if (tok != ')') {
                t = gjmp(0);
                expr();
                gjmp(n - ind - 5);
                gsym(t);
                n = t + 4;
            }
        }
        skip(')');
        block(&a);
        gjmp(n - ind - 5); /* jmp */
        gsym(a);
    } else if (tok == '{') {
        next();
        /* declarations */
        decl(1);
        while(tok != '}')
            block(l);
        next();
    } else {
        if (tok == TOK_RETURN) {
            next();
            if (tok != ';')
                expr();
            rsym = gjmp(rsym); /* jmp */
        } else if (tok == TOK_BREAK) {
            next();
            *(int *)l = gjmp(*(int *)l);
        } else if (tok != ';')
            expr();
        skip(';');
    }
}

/* 'l' is true if local declarations */
decl(l)
{
    int a;

    while (tok == TOK_INT | tok != -1 & !l) {
        if (tok == TOK_INT) {
            next();
            while (tok != ';') {
                if (l) {
                    loc = loc + 4;
                    *(int *)tok = -loc;
                } else {
                    *(int *)tok = glo;
                    glo = glo + 4;
                }
                next();
                if (tok == ',') 
                    next();
            }
            skip(';');
        } else {
            /* put function address */
            *(int *)tok = ind;
            next();
            skip('(');
            a = 8;
            while (tok != ')') {
                /* read param name and compute offset */
                *(int *)tok = a;
                a = a + 4;
                next();
                if (tok == ',')
                    next();
            }
            next(); /* skip ')' */
            rsym = loc = 0;
            o(0xe58955); /* push   %ebp, mov %esp, %ebp */
            a = oad(0xec81, 0); /* sub $xxx, %esp */
            block(0);
            gsym(rsym);
            o(0xc3c9); /* leave, ret */
            put32(a, loc); /* save local variables */
        }
    }
}

#ifdef ELFOUT

gle32(n)
{
    put32(glo, n);
    glo = glo + 4;
}

/* used to generate a program header at offset 't' of size 's' */
gphdr1(n, t)
{
    gle32(n);
    n = n + ELF_BASE;
    gle32(n);
    gle32(n);
    gle32(t);
    gle32(t);
}

elf_reloc(l)
{
    int t, a, n, p, b, c;

    p = 0;
    t = sym_stk;
    while (1) {
        /* extract symbol name */
        t++;
        a = t;
        while (*(char *)t != TAG_TOK && t < dstk)
            t++;
        if (t == dstk)
            break;
        /* now see if it is forward defined */
        tok = vars + (a - sym_stk) * 8 + TOK_IDENT - 8;
        b = *(int *)tok;
        n = *(int *)(tok + 4);
        if (n && b != 1) {
#if 0
            {
                char buf[100];
                memcpy(buf, a, t - a);
                buf[t - a] = '\0';
                printf("extern ref='%s' val=%x\n", buf, b);
            }
#endif
            if (!b) {
                if (!l) {
                    /* symbol string */
                    memcpy(glo, a, t - a);
                    glo = glo + t - a + 1; /* add a zero */
                } else if (l == 1) {
                    /* symbol table */
                    gle32(p + DYNSTR_BASE);
                    gle32(0);
                    gle32(0);
                    gle32(0x10); /* STB_GLOBAL, STT_NOTYPE */
                    p = p + t - a + 1; /* add a zero */
                } else {
                    p++;
                    /* generate relocation patches */
                    while (n) {
                        a = get32(n);
                        /* c = 0: R_386_32, c = 1: R_386_PC32 */
                        c = *(char *)(n - 1) != 0x05;
                        put32(n, -c * 4);
                        gle32(n - prog + text + data_offset);
                        gle32(p * 256 + c + 1);
                        n = a;
                    }
                }
            } else if (!l) {
                /* generate standard relocation */
                gsym1(n, b);
            }
        }
    }
}

elf_out(c)
{
    int glo_saved, dynstr, dynstr_size, dynsym, hash, rel, n, t, text_size;

    /*****************************/
    /* add text segment (but copy it later to handle relocations) */
    text = glo;
    text_size = ind - prog;

    /* add the startup code */
    ind = prog;
    o(0x505458); /* pop %eax, push %esp, push %eax */
    t = *(int *)(vars + TOK_MAIN);
    oad(0xe8, t - ind - 5);
    o(0xc389);  /* movl %eax, %ebx */
    li(1);      /* mov $1, %eax */
    o(0x80cd);  /* int $0x80 */
    glo = glo + text_size;

    /*****************************/
    /* add symbol strings */
    dynstr = glo;
    /* libc name for dynamic table */
    glo++;
    glo = strcpy(glo, "libc.so.6") + 10;
    glo = strcpy(glo, "libdl.so.2") + 11;
    
    /* export all forward referenced functions */
    elf_reloc(0);
    dynstr_size = glo - dynstr;

    /*****************************/
    /* add symbol table */
    glo = (glo + 3) & -4;
    dynsym = glo;
    gle32(0);
    gle32(0);
    gle32(0);
    gle32(0);
    elf_reloc(1);

    /*****************************/
    /* add symbol hash table */
    hash = glo;
    n = (glo - dynsym) / 16;
    gle32(1); /* one bucket (simpler!) */
    gle32(n);
    gle32(1);
    gle32(0); /* dummy first symbol */
    t = 2;
    while (t < n)
        gle32(t++);
    gle32(0);
    
    /*****************************/
    /* relocation table */
    rel = glo;
    elf_reloc(2);

    /* copy code AFTER relocation is done */
    memcpy(text, prog, text_size);

    glo_saved = glo;
    glo = data;

    /* elf header */
    gle32(0x464c457f);
    gle32(0x00010101);
    gle32(0);
    gle32(0);
    gle32(0x00030002);
    gle32(1);
    gle32(text + data_offset); /* address of _start */
    gle32(PHDR_OFFSET); /* offset of phdr */
    gle32(0);
    gle32(0);
    gle32(0x00200034);
    gle32(3); /* phdr entry count */

    /* program headers */
    gle32(3); /* PT_INTERP */
    gphdr1(INTERP_OFFSET, INTERP_SIZE);
    gle32(4); /* PF_R */
    gle32(1); /* align */
    
    gle32(1); /* PT_LOAD */
    gphdr1(0, glo_saved - data);
    gle32(7); /* PF_R | PF_X | PF_W */
    gle32(0x1000); /* align */
    
    gle32(2); /* PT_DYNAMIC */
    gphdr1(DYNAMIC_OFFSET, DYNAMIC_SIZE);
    gle32(6); /* PF_R | PF_W */
    gle32(0x4); /* align */

    /* now the interpreter name */
    glo = strcpy(glo, "/lib/ld-linux.so.2") + 0x14;

    /* now the dynamic section */
    gle32(1); /* DT_NEEDED */
    gle32(1); /* libc name */
    gle32(1); /* DT_NEEDED */
    gle32(11); /* libdl name */
    gle32(4); /* DT_HASH */
    gle32(hash + data_offset);
    gle32(6); /* DT_SYMTAB */
    gle32(dynsym + data_offset);
    gle32(5); /* DT_STRTAB */
    gle32(dynstr + data_offset);
    gle32(10); /* DT_STRSZ */
    gle32(dynstr_size);
    gle32(11); /* DT_SYMENT */
    gle32(16);
    gle32(17); /* DT_REL */
    gle32(rel + data_offset);
    gle32(18); /* DT_RELSZ */
    gle32(glo_saved - rel);
    gle32(19); /* DT_RELENT */
    gle32(8);
    gle32(0);  /* DT_NULL */
    gle32(0);

    t = fopen(c, "w");
    fwrite(data, 1, glo_saved - data, t);
    fclose(t);
}
#endif

main(int n, char** t)
{
    if (n < 3) {
        printf("usage: otccelf file.c outfile\n");
        return 0;
    }
    dstk = strcpy(sym_stk = calloc(1, ALLOC_SIZE), 
                  " int if else while break return for define main ") + TOK_STR_SIZE;
    glo = data = calloc(1, ALLOC_SIZE);
    ind = prog = calloc(1, ALLOC_SIZE);
    vars = calloc(1, ALLOC_SIZE);

    t = t + 4;
    file = fopen(*(int *)t, "r");

    data_offset = ELF_BASE - data; 
    glo = glo + ELFSTART_SIZE;
    ind = ind + STARTUP_SIZE;

    inp();
    next();
    decl(0);
    t = t + 4;
    elf_out(*(int *)t);
    return 0;
}
