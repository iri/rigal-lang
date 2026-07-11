#include "globrig.h"
#include "define.h"
#include "defpage.h"
#include "ley.h"
#include "nef2.h"

/* lexic analysis for rigal language
   input : text file with name first_file
           and if 'NOT_INCLUDE' is false
             then all included files
   output :s-object code and added a-objects

   may be called from editor environment through checker
   or from usepas for user's call
   #call_pas(14 'A.RIG') or #call_pas(15 'A.RIG') .

   token's COoRDinates returned as LINE_NUMBER*80+SYMBOL_NUMBER
   included file names returned as tatom descriptors;
     exception: first_file never returned as first lexem.
   added letter to filename  i if %included file
                             c if returned from %include
*/

#define filemax 4           /* ~islo wlovenij dlq include */
#define bufmaxlen 10        /* dlina malogo bufera , kak minimum - 8 */
#define two_char_sym_max 50 /* maks. massiwa */

typedef union bufrectype
{
  Char lexbuf[bufmaxlen];
  Char b1;
  Char b2[2];
  Char b3[3];
  Char b4[4];
  Char b5[5];
  Char b6[6];
  Char b7[7];
  Char b8[8];
  Char b9[9];
  Char b10[10];
} bufrectype;

FILE *infile[filemax]; /* fajly ish. teksta */
bufrectype bufrec;     /* sohranenie dlq leksera !!!! */
c2 twochar_symbols[two_char_sym_max];
char twochar_symbols_num;

typedef Char a146[146]; /* source string type */

typedef Char bigstr_type[146];

typedef struct _REC_fistack
{
  long curline;        /* current line of this file */
  filespecification f; /* file name */
} _REC_fistack;

/* Local variables for ley: */
struct LOC_ley
{
  error_rec_type *error_rec;
  a satomadr;
  long i; /* current byte */
  boolean errflag;
  a146 s;

  _REC_fistack fistack[filemax + 1];
  long fistacklen;
};

Local Void newlist(pp, LINK)
ptr_ *pp;
struct LOC_ley *LINK;
{
  /* nowyj ukazatelx spiska */
  /* sozdaet nowyj spisok */
  mpd x;
  a a1;

  gets5(&a1, &x.sa);
  points(a1, &x.sa);
  x.smld->dtype = listmain;
  x.smld->lastfragm = a1;
  pp->ptrtype = ptrlist;
  pp->cel = 0;
  pp->nel = 0;
  pp->UU.U1.curfragment = a1;
  pp->UU.U1.mainadr = a1;
}

Local Void push(pp, adr, LINK)
ptr_ *pp;
long adr;
struct LOC_ley *LINK;
{
  mpd x, x1;
  a a1;

  /* dobawlqet po pojnteru spiska nowyj |lement k spisku*/
  /* i sdwigaet pojnter pp */
  if (pp->ptrtype != ptrlist)
  {
    printf("Rigal internal error Push-102\n");
    return;
  } /* if/then */
  points(pp->UU.U1.mainadr, &x.sa);
  if (x.smld->dtype == listmain)
    x.smld->totalelnum++;
  else
    printf("Rigal internal error Push-101\n");
  points(pp->UU.U1.curfragment, &x.sa);
  if (x.smld->dtype == listmain && pp->nel == mainlistelnum ||
      x.sfld->dtype == listfragm && pp->nel == fragmlistelnum)
  {
    /* w slu~ae dostiveniq konca fragmenta spiska */
    gets5(&a1, &x1.sa);
    if (x.smld->dtype == listmain) /* podceplenie */
      x.smld->next = a1;
    else
      x.sfld->next = a1;
    /* obrazuem i zapolnqem nowyj  */
    x1.sfld->dtype = listfragm;
    x1.sfld->elnum = 1;
    x1.sfld->elt[0] = adr;
    points(pp->UU.U1.mainadr, &x.sa);
    x.smld->lastfragm = a1;
    /* sdwig pojntera */
    pp->nel = 1;
    pp->cel = adr;
    pp->UU.U1.curfragment = a1;
    return;
  } /* then */
  /* ob{ij clu~aj dobawleniq |lementa wnutri fragmenta */
  switch (x.smld->dtype)
  {

  case listmain:
    x.smld->elnum++;
    x.smld->elt[pp->nel] = adr;
    break;

  case listfragm:
    x.sfld->elnum++;
    x.sfld->elt[pp->nel] = adr;
    break;
  } /* case */
  pp->nel++;
  pp->cel = adr;
  /* else */
} /* push */

Local Void mistake(mistake_num, LINK)
long mistake_num;
struct LOC_ley *LINK;
{
  string80 com;

  printf("Error...%12ld M=%s\n", mistake_num, LINK->error_rec->message);
  switch (mistake_num)
  {

  case 1:
    strcpy(com, "MAIN PROGRAM FILE  IS NOT FOUND ");
    break;

  case 2:
    strcpy(com, "MORE THAN 2 NESTED %INCLUDE FILES");
    break;

  case 3:
    strcpy(com, "THIS %INCLUDE FILE  IS NOT FOUND ");
    break;

  case 4:
    strcpy(com, "TOO LONG (>80 BYTES) TOKEN");
    break;

  case 5:
    strcpy(com, "WRONG CHARACTER AFTER NUMBER");
    break;

  case 6:
    strcpy(com, "TOO BIG NUMBER (> 2.**31) ");
    break;

  case 8:
    strcpy(com, "ENDING APOSTROPHE NOT FOUND IN THIS LINE");
    break;

  case 11:
    strcpy(com, "THIS CHARACTER NOT ALLOWED ");
    break;

  case 12:
    strcpy(com, "NUMBER AFTER \"A'\" NOT FOUND ");
    break;

  case 13:
    strcpy(com, "ZERO LENGTH STRING NOT ALLOWED");
    break;

  case 14:
    strcpy(com, "RULE NAME AFTER \"#\" NOT FOUND ");
    break;

  case 17:
    strcpy(com, "NUMBER AFTER \"A'\" MUST BE N*512");
    break;

  case 18:
    strcpy(com, "WRONG DIGIT (8 or 9) IN OCTAL NUMBER ");
    break;

  default:
    strcpy(com, "UNKNOWN LEXICAL ERROR");
    break;
  }
  printf("...\n");
  LINK->errflag = true;
  strcpy(LINK->error_rec->message, com);
  LINK->error_rec->address =
      LINK->fistack[LINK->fistacklen - 1].curline * 80 + LINK->i;
  strcpy(LINK->error_rec->filename, LINK->fistack[LINK->fistacklen - 1].f);
  printf(" LEXICAL ERROR : %s\n", com);
  printf(" LINE=%12ld  SYMBOL=%12ld\n",
         LINK->fistack[LINK->fistacklen - 1].curline, LINK->i);
}

Local Void makeatom(ik, jk, desk, LINK)
long ik, jk;
char desk;
struct LOC_ley *LINK;
{
  /* makes s-atom
     from array s, starting ik, length jk
   */
  mpd x;
  a a1m;
  atomdescriptor *WITH;

  putatm(&LINK->s[ik - 1], jk, &a1m);
  gets1(&LINK->satomadr, &x.sa);
  WITH = x.sad;
  WITH->cord = LINK->fistack[LINK->fistacklen - 1].curline * 80 + LINK->i;
  /*!!*/
  WITH->dtype = desk;
  WITH->name = a1m;
}

Void ley(first_file, lesrez, not_include, error_rec_)
Char *first_file;
long *lesrez;
boolean not_include;
error_rec_type *error_rec_;
{
  /* added lastfragm 12-jul-91 */
  struct LOC_ley V;
  /*string80 first_file;*/
  char dt;
  longint ilong;
  a adr;
  char jcase;
  long j, nn, jj, ii;
  mpd x;
  long len; /* current line length */
  ptr_ p;
  mpd y;
  boolean is_ident, x_lists;
  /*  srb,srl,slb,sll: string; */
  /*  Char table[256]; */
  boolean maybe_octal;
  bigstr_type a_long; /*varying[145] of char;*/
  a146 s1;
  filespecification ff1;
  string80 ssint;

  Char twochar_string[161];
  long rline;
  Char c;
  long FORLIM;
  int fff;
  numberdescriptor *WITH;
  ruledescriptor *WITH1;
  vardescriptor *WITH2;
  specdescriptor *WITH3;

  /* start of main program */

  /*  strcpy(first_file, first_file_); */
  V.error_rec = error_rec_;
  strcpy(twochar_string,
         ":= :: >= <> (. .) <= -> (* *) (+ +) ## ;; !! ++ !. <. .> <* *> << >> IF FI IN DO OD OR $$ <] S' V'");
  /* file stack initialization */
  V.fistacklen = 1;
  V.fistack[0].curline = 0;
  strcpy(V.fistack[0].f, first_file);

  V.errflag = false;
  x_lists = false;
  for (V.i = 1; V.i <= 33; V.i++)
  {
    twochar_symbols[V.i - 1][0] = twochar_string[V.i * 3 - 3];
    twochar_symbols[V.i - 1][1] = twochar_string[V.i * 3 - 2];
  }
  twochar_symbols_num = 33;
  if (!existfile(first_file))
  {
    strcpy(V.error_rec->message, first_file);
    mistake(1L, &V);
    goto _L199;
  }
  infile[0] = fopen(first_file, "r");
  if (infile[0] == NULL)
    _EscIO(FileNotFound);
  newlist(&p, &V); /* create list descriptor */
  *lesrez = p.UU.U1.mainadr;
  /* for (ii = 0; ii <= 255; ii++)
     table[(Char)ii] = (Char)ii;*/

  ii = 0; /* token number */

  /* file stack initialization */

  V.i = 1;
  len = 0;

_L1:

  /* len - current line length
     i - byte number in this line where first letter of
         token stays */

  if (V.i == len + 1)
  {
    /* go to next line of source text */
    if (feof(infile[V.fistacklen - 1]))
    {
      if (V.fistacklen == 1)
        goto _L99; /* exit from lexer */
      else
      {
        if (infile[V.fistacklen - 1] != NULL)
          fclose(infile[V.fistacklen - 1]);
        infile[V.fistacklen - 1] = NULL;
        V.fistacklen--;
        /* adding letter 'C'=continuation flag */
        sprintf(ff1, "%sC", V.fistack[V.fistacklen - 1].f);
        FORLIM = strlen(ff1);
        for (j = 0; j < FORLIM; j++)
          V.s[j] = ff1[j];
        makeatom(1L, (long)strlen(ff1), tatom, &V);
        goto _L33;
      }
    }

    /* next line take */

    /*readln(infile[fistacklen],a_long);*/

#ifdef xxx
    *a_long = '\0';
    while (true)
    {
      if (feoln(infile[V.fistacklen - 1]))
      {
        c = getc(infile[V.fistacklen - 1]);
        if (c == '\n')
          c = ' ';
        rline = 4;
        goto _L95;
      }
      c = getc(infile[V.fistacklen - 1]);
      if (c == '\n')
        c = ' ';
      sprintf(a_long + strlen(a_long), "%c", c);
      if (feof(infile[V.fistacklen - 1]))
      {
        rline = 1;
        goto _L95;
      }
      if (!feoln(infile[V.fistacklen - 1]))
        continue;
      c = getc(infile[V.fistacklen - 1]);
      if (c == '\n')
        c = ' ';
      if (feof(infile[V.fistacklen - 1]))
        rline = 2;
      else
        rline = 3;
      goto _L95;
    }
  _L95:
#endif

    fgets(a_long, 145, infile[V.fistacklen - 1]);
    if (a_long[strlen(a_long) - 1] == '\n')
    {
      a_long[strlen(a_long) - 1] = 0;
      fff = fgetc(infile[V.fistacklen - 1]);
      if (fff != 10)
      {
        ungetc(fff, infile[V.fistacklen - 1]);
      }
      else
        V.fistack[V.fistacklen - 1].curline++;
    }

    if (*a_long != '\0' && a_long[strlen(a_long) - 1] == '\r')
      a_long[strlen(a_long) - 1] = 0;

    V.fistack[V.fistacklen - 1].curline++;
    /* line counter */
    len = strlen(a_long);
    for (V.i = 1; V.i <= len; V.i++)
      V.s[V.i - 1] = a_long[V.i - 1];
    V.s[len] = ' ';
    V.i = 1;
  }

  for (j = 1; j <= 10; j++)
    bufrec.b10[j - 1] = V.s[V.i + j - 2];
  if (!strncmp(bufrec.b2, "--", 2))
  { /* koa_longmentarii */
    V.i = len + 1;
    goto _L1;
  }
  switch (bufrec.b1)
  {

  case ' ':
  case '\t': /*tabulator*/
    while ((V.s[V.i - 1] == ' ' || V.s[V.i - 1] == '\t') && V.i <= len)
      V.i++;
    /* when exits  i=len+1 or s[i]<>' ' */
    goto _L1;
    break;

  case '\'':
    memcpy(s1, V.s, sizeof(a146));
    /* saving line to s1, analise, write to s
              and give s to  makeatom */
    j = 1;
    jj = 1;
    while (V.i + j <= len &&
           (s1[V.i + j - 1] != '\'' ||
            s1[V.i + j - 1] == '\'' && s1[V.i + j] == '\''))
    {
      if (s1[V.i + j - 1] == '\'' && s1[V.i + j] == '\'')
        j++;
      /* if two apostrophes then we move to second
         and write only one */
      V.s[V.i + jj - 1] = s1[V.i + j - 1];
      /* s filled not from [1] for right diagnostics*/
      jj++;
      j++;
    }

    is_ident = is_rig_letter(V.s[V.i]);
    for (nn = 1; nn <= jj - 2; nn++)
    {
      if (!is_rig_symbol(V.s[V.i + nn]))
        is_ident = false;
    }
    if (is_ident)
      dt = idatom;
    else
      dt = atom;

    if (jj == 1)
    {
      mistake(13L, &V);
      goto _L199;
    }
    if (s1[V.i + j - 1] != '\'')
    {
      mistake(8L, &V);
      goto _L199;
    }
    makeatom(V.i + 1, jj - 1, dt, &V);
    V.i += j + 1;
    memcpy(V.s, s1, sizeof(a146)); /* return saved line */
    goto _L33;
    break;

  case '%':
    if (!strncmp(bufrec.b6, "%INCLU", 6))
    {
      for (j = -1; j <= 6; j++)
        V.s[V.i + j] = ' ';
      *ff1 = '\0';
      for (j = 7; j <= len - 2; j++)
      {
        /* file name we take till the end of line */
        if (V.s[V.i + j] != ' ')
          sprintf(ff1 + strlen(ff1), "%c", V.s[V.i + j]);
      }
      V.i = len + 1;
      if (!not_include)
      {
        if (V.fistacklen == filemax)
        {
          mistake(2L, &V);
          goto _L199;
        }

        V.fistacklen++;

        if (!existfile(ff1))
        {
          V.fistacklen--;
          strcpy(V.error_rec->message, ff1);
          mistake(3L, &V);
          goto _L199;
        }

        infile[V.fistacklen - 1] = fopen(ff1, "r");

        if (infile[V.fistacklen - 1] == NULL)
          _EscIO(FileNotFound);
        printf("reading %s\n", ff1);

        V.fistack[V.fistacklen - 1].curline = 0;
        strcpy(V.fistack[V.fistacklen - 1].f, ff1);
        /* establish %include flag='I' */
        strcat(ff1, "I");
        FORLIM = strlen(ff1);
        for (j = 0; j < FORLIM; j++)
          V.s[j] = ff1[j];
        makeatom(1L, (long)strlen(ff1), tatom, &V);
        V.i = 1;
        len = 0;
        goto _L33;
      }
      V.i = 1;
      len = 0;
      goto _L1;
    }
    else
    {
      mistake(11L, &V);
      goto _L199;
    }
    break;

  case '#':
    j = 1;
    if (!strncmp(bufrec.b2, "##", 2))
    {
      makeatom(V.i, 2L, keyword, &V);
      V.i += 2;
      goto _L33;
    }

    while (is_rig_symbol(V.s[V.i + j - 1]))
      j++;
    if (j == 1)
    {
      mistake(14L, &V);
      goto _L199;
    }
    j--;
    putatm(&V.s[V.i], j, &adr);
    gets2(&V.satomadr, &y.sa);
    WITH1 = y.srd;
    WITH1->dtype = rulename;
    WITH1->cord = V.fistack[V.fistacklen - 1].curline * 80 + V.i;
    WITH1->fragmadr = 0;
    WITH1->nomintab = 0;
    WITH1->name = adr;
    V.i += j + 1;
    goto _L33;
    break;

  case '$':
    j = 1;
    if (V.s[V.i] == '$')
    {
      j = 2;
      putatm(&V.s[V.i - 1], j, &adr);
      makeatom(V.i, 2L, keyword, &V);
      V.i += 2;
      goto _L33;
    }
    while (is_rig_symbol(V.s[V.i + j - 1]))
      j++;
    j--;
    if (j > 0)
      putatm(&V.s[V.i], j, &adr);
    j++;
    if (j == 1)
    {
      V.s[V.i - 1] = '_';
      j = 1;
      putatm(&V.s[V.i - 1], j, &adr);
    }
    gets1(&V.satomadr, &x.sa);
    WITH2 = x.svd; /* with */
    switch (V.s[V.i])
    {

    case 'N':
      WITH2->dtype = nvariable;
      break;

    case 'I':
      WITH2->dtype = idvariable;
      break;

    default:
      WITH2->dtype = variable;
      break;
    }
    WITH2->location = 0;
    WITH2->name = adr;
    WITH2->guard = false;
    V.i += j;
    goto _L33;
    break;

  case '(':
  case ':':
  case '*':
  case '<':
  case '>':
  case '.':
  case '-':
  case '+':
  case ';':
  case '!':
    if (bufrec.b2[1] == ']' || bufrec.b2[1] == '<' || bufrec.b2[1] == '!' ||
        bufrec.b2[1] == ';' || bufrec.b2[1] == '+' || bufrec.b2[1] == ':' ||
        bufrec.b2[1] == '*' || bufrec.b2[1] == ')' || bufrec.b2[1] == '>' ||
        bufrec.b2[1] == '.' || bufrec.b2[1] == '=')
    {
      FORLIM = twochar_symbols_num;
      for (nn = 0; nn < FORLIM; nn++)
      {
        if (bufrec.b2[0] == twochar_symbols[nn][0] &&
            bufrec.b2[1] == twochar_symbols[nn][1])
        {
          makeatom(V.i, 2L, keyword, &V);
          V.i += 2;
          goto _L33;
        }
      }
    }
    makeatom(V.i, 1L, keyword, &V);
    V.i++;
    goto _L33;
    break;

  case ')':
  case '=':
  case ',':
  case '/':
  case '^':
  case '@':
  case ']':
  case '[':
    makeatom(V.i, 1L, keyword, &V);
    V.i++;
    goto _L33;
    break;

  default:
    if (isdigit(bufrec.b1))
    {
      *ssint = '\0';
      jj = 0;
      j = 0;
      ilong = 0;
      maybe_octal = true;
      while (isdigit(V.s[V.i + j - 1]))
      {
        if (V.s[V.i + j - 1] == '8' || V.s[V.i + j - 1] == '9')
          maybe_octal = false;
        ilong = ilong * 8 + V.s[V.i + j - 1] - '0';
        sprintf(ssint + strlen(ssint), "%c", V.s[V.i + j - 1]);
        j++;
      }
      if (V.s[V.i + j - 1] == 'B' || V.s[V.i + j - 1] == 'b')
      {
        if (!maybe_octal)
        {
          mistake(18L, &V);
          goto _L199;
        }
        j++;
      }
      else if (is_rig_symbol(V.s[V.i + j - 1]))
      {
        mistake(5L, &V);
        goto _L199;
      }
      else
        val(ssint, &ilong, &jj);
      if (jj == 0 && ilong < 2147483647)
      {
        gets1(&V.satomadr, &x.sa);
        WITH = x.snd; /* with */
        WITH->dtype = number;
        WITH->cord = V.fistack[V.fistacklen - 1].curline * 80 + V.i; /*!!*/
        WITH->val = ilong;
        V.i += j;
        goto _L33;
      }
      else
      {
        mistake(6L, &V);
        goto _L199;
      }
    }
    else
    {

      if (is_rig_letter(bufrec.b1))
      {
        j = 1;
        while (is_rig_symbol(V.s[V.i + j - 1]))
          j++;
        dt = idatom;
        jcase = j;
        switch (jcase)
        {

        case 1:
          if (!strncmp(bufrec.b2, "S'", 2) || !strncmp(bufrec.b2, "V'", 2))
          {
            j = 2;
            dt = keyword;
          }
          break;

        case 2:
          if (!strncmp(bufrec.b2, "OD", 2) || !strncmp(bufrec.b2, "IF", 2) ||
              !strncmp(bufrec.b2, "FI", 2) || !strncmp(bufrec.b2, "IN", 2) ||
              !strncmp(bufrec.b2, "DO", 2) || !strncmp(bufrec.b2, "OR", 2))
            dt = keyword;
          break;

        case 3:
          if (!strncmp(bufrec.b3, "AND", 3) ||
              !strncmp(bufrec.b3, "MOD", 3) ||
              !strncmp(bufrec.b3, "DIV", 3) ||
              !strncmp(bufrec.b3, "END", 3) || !strncmp(bufrec.b3, "NOT", 3))
            dt = keyword;
          break;

        case 4:
          if (!strncmp(bufrec.b4, "NULL", 4))
          {
            V.i += 4;
            gets1(&V.satomadr, &x.sa);
            WITH3 = x.sspec;
            WITH3->dtype = spec;
            WITH3->val = 0;
            goto _L33;
          }

          if (!strncmp(bufrec.b4, "LAST", 4) ||
              !strncmp(bufrec.b4, "LOOP", 4) ||
              !strncmp(bufrec.b4, "OPEN", 4) ||
              !strncmp(bufrec.b4, "SAVE", 4) ||
              !strncmp(bufrec.b4, "FAIL", 4) ||
              !strncmp(bufrec.b4, "COPY", 4) ||
              !strncmp(bufrec.b4, "LOAD", 4))
            dt = keyword;
          break;

        case 5:
          if (!strncmp(bufrec.b5, "ELSIF", 5) ||
              !strncmp(bufrec.b5, "CLOSE", 5) ||
              !strncmp(bufrec.b5, "BREAK", 5) ||
              !strncmp(bufrec.b5, "PRINT", 5))
            dt = keyword;
          break;

        case 6:
          if (!strncmp(bufrec.b6, "ONFAIL", 6) ||
              !strncmp(bufrec.b6, "RETURN", 6) ||
              !strncmp(bufrec.b6, "FORALL", 6))
            dt = keyword;
          break;

        case 8:
          if (!strncmp(bufrec.b8, "BRANCHES", 8))
            dt = keyword;
          break;

        case 9:
          if (!strncmp(bufrec.b9, "SELECTORS", 9))
            dt = keyword;
          break;

        } /*case*/

        makeatom(V.i, j, dt, &V);
        V.i += j;
        goto _L33;
      }
      else
      {
        mistake(11L, &V);
        goto _L199;
      }
    }
    break;
  } /* case */

_L33:
  ii++;
  push(&p, V.satomadr, &V); /* adding to list */
  goto _L1;                 /*with*/
_L99:
  if (V.errflag)
    printf("... RIGAL  lexic errors found\n");
  /*    writeln('Tokens count=',ii,'                       ');*/
  if (infile[0] != NULL)
    fclose(infile[0]);
  infile[0] = NULL;
  /* printf(" TOTAL RESULT=\n");
   pscr(*lesrez);
   printf("\n"); */

_L199:;

  /* prints current line counter */
  /*write(fistack[fistacklen].curline,' ');*/
  /* go to more common file */

  /* no mistake */
}

/* End. */
