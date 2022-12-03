// lisp - A minimal "compact" lisp
// ===============================
// (>) 2022 jsk@yesco.org
// 
// + - * / car cdr cons eq equal assoc eval consp princ print terpri quote read
// quit not
// TODO: define/setq/lambda/if/cond/map

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>


typedef void* lisp;
lisp nil= NULL;


#define L(a) ((long)((long)(a)))
#define D(SIG, RET) lisp SIG { return (lisp)L(RET); }


D(mknum(long n), n*2+1)
long num(lisp n) { return (L(n)-1)/2; }


typedef struct cons { lisp car, cdr; } *Cons;
D(consp(lisp c), c && !(L(c)&7))
D(car(Cons c), consp(c) ? c->car : 0)
D(cdr(Cons c), consp(c) ? c->cdr : 0)
D(cons(lisp a, lisp d), ({Cons c= malloc(sizeof(*c));c->car=a;c->cdr=d; c;}))


lisp rd();
D(rdl(), ({lisp x=rd(); x? cons(x, rdl()) : x;}))
lisp rd() {
  int c= ' ', r= 0;
  while(isspace(c)) c= getc(stdin);
  if (c==')') return nil;
  if (c=='(' || c=='.') return rdl();
  do {r= r*(isdigit(c)?10:128) + (isdigit(c)?c-'0':c);
  } while(isalnum((c=getc(stdin))));
  ungetc(c, stdin);
  // map nil to 0
  return r==0x3769D9/2 ? 0 : mknum(r);
}

D(eq(lisp a, lisp b), a==b?3:0)
D(equ(lisp a,lisp b), eq(a,b)||equ(car(a),car(b))&&equ(cdr(a),cdr(b)))

D(assoc(lisp v, lisp l), ({while(consp(l) && !eq(v,car(car(l)))) l= cdr(l); car(l);}))

lisp princ(lisp e) { lisp x= e;
  if (!e) return printf("nil"),e;
  if (!consp(e)) return printf("%ld", num(e)),e;
  putchar('('); do {
    princ(car(x)); x= cdr(x); x && putchar(' ');
  } while (consp(x));
  if (x) printf(". "),princ(x);
  return putchar(')'),e;
}


D(var(lisp v, lisp env, lisp def), ({lisp e=assoc(v,env);e ? cdr(e) : def;}))

lisp eval(lisp e, lisp env);
D(evlist(lisp l,lisp env),!consp(l)?l:cons(eval(car(l),env),evlist(cdr(l),env)))

lisp eval(lisp e, lisp env) {
  if (!consp(e)) return var(e, env, e);
  lisp r= car(e)>0 ? evlist(cdr(e), env) : cdr(e); 
  switch((long)car(e)) {

#define M(CD,OP) case CD: return mknum(num(car(r)) OP num(car(cdr(r))))
  M(0x57, +); M(0x5b, -); M(0x55, *); M(0x5f, /);

#define S(CD,F) case CD: return F(car(r))
  S(0x31e1e5, car);S(0x31e4e5, cdr);S(0x7bf773e1, consp);S(0x1cb4eec7, princ);

  case 0x376fe9: return (lisp)(car(r)?0L:3L); // not

  case 0xe3d77f4cb: return car(r); // quote

  case 0x1cb4eee9: princ(car(r)); // print
  case 0xbcb872d3: return putchar('\n'),nil;

  case 0x1cb2e1c9: return rd();

#define B(CD,F) case CD: return F(car(r), car(cdr(r)))
  B(0x18f7eee7, cons); B(0x65e3, eq); B(0xcbc7ae1d9, equ);
  B(0x3cf9efc7, assoc); B(0x197b61d9, eval);   //B(0x3c386cf3, apply);

  case 0x36e1e1: // map

  case -0x69cd: // if

  // TODO: let int lt or and cond if leta lambda define reduce gc
 
  default: printf("ERROR: "); princ(e); break;
  }
  return e;
}



// ENDWCOUNT

#include <assert.h>

int main(int argc, char** argv) {
  assert(sizeof(long)==8); // require 64-bit

  lisp env= cons( cons( (lisp)0xc3, mknum(999)),
	    cons( cons( (lisp)0xc5, mknum(666)),
                  nil));

  lisp x= nil;
  x= mknum(42);
  x= cons( (lisp)0x55, cons( mknum(111), cons(mknum(3), nil)));
  princ(x);
  putchar('\n');
  princ(eval(x, env));
  putchar('\n');

  fputc('>', stderr);
  lisp t;
  while(L((t=rd()))!=0x1c7ae9e9) { // quit
    printf("  [ %ld 0x%lx ]\n", L(t), L(t));
    princ(t); putchar('\n');
    princ(eval(t, env));
    putchar('\n');
    putchar('\n');
    fputc('>', stderr);
  }
}
