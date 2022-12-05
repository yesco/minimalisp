// lisp - A minimal "compact" lisp
// ===============================
// (>) 2022 jsk@yesco.org
// 
// + - * / < = > % & |
// car cdr cons assoc
// eq equal consp symbolp and or not
// princ print terpri quote lambda read
// if eval quit
//
// 30 ops!
// TODO: define let setq cond map apply leta reduce
// No Garbage Collection (yet...)

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

typedef void* lisp;
#define tag(c)(L(c)&3)
// cons 00
//  num 01
// ???? 10 free to use, string?
// atom 11
lisp nil= NULL, t= (lisp)5L; // numeric: 1
typedef struct cons { lisp car, cdr; } *Cons;
#define L(a) ((long)((long)(a)))
#define D(SIG, RET) lisp SIG { return (lisp)L(RET); }

D(mknum(long n), n*4+1)                long num(lisp n) { return (L(n)-1)/4; }
D(consp(lisp c), c&&!tag(c)?t:nil)     D(car(Cons c), consp(c) ? c->car : 0)  
D(symp(lisp s), tag(s)==3?t:nil)       D(cdr(Cons c), consp(c) ? c->cdr : 0)  
D(cons(lisp a, lisp d), ({Cons c= malloc(sizeof(*c));c->car=a;c->cdr=d; c;}))

D(eq(lisp a, lisp b), a==b?t:0)
D(equ(lisp a,lisp b), eq(a,b)||equ(car(a),car(b))&&equ(cdr(a),cdr(b)))
D(assoc(lisp v, lisp l), ({while(consp(l) && !eq(v,car(car(l)))) l= cdr(l); car(l);}))

lisp rd(); D(rdl(), ({lisp x=rd(); x? cons(x, rdl()) : x;}))
lisp rd() {
  long c= ' ', r= 0, a= 0;
  while(isspace(c)) c= getc(stdin);
  if (c==')') return nil;
  if (c=='(' || c=='.') return rdl();
  do {r= r*(isdigit(c)?10:128) + (isdigit(c)?c-'0':c); a= a||!isdigit(c);
    // TODO: isnotstop?
  } while(isalnum((c=getc(stdin))));
  ungetc(c, stdin);
  // map "nil" to (lisp)0
  return r==0x3769D9/2 ? 0 : mknum(r)+2*a;
}

int pratom(unsigned long a){char s[9]={},i=8;do s[--i]=a&127;while(a>>=7);return printf("%s",s+i);}
lisp princ(lisp e) { lisp x= e;
  if (!e) return printf("nil"),e;
  if (!consp(e)) return symp(e)?pratom(L(e)/4):printf("%ld", num(e)), e;
  putchar('(');do{princ(car(x)); x=cdr(x); x && putchar(' ');} while(consp(x));
  if (x) printf(". "),princ(x);
  return putchar(')'),e;
}

D(var(lisp v, lisp env, lisp def), ({lisp e=assoc(v,env); e? cdr(e): def;}))

lisp eval(lisp e, lisp env);
D(evlist(lisp l,lisp env),!consp(l)?l:cons(eval
(car(l),env),evlist(cdr(l),env)))
D(bnd(lisp f,lisp a),({f&&a?cons(cons(car(f),car(a)),bnd(cdr(f),cdr(a))):nil;}))
lisp eval(lisp e, lisp env) {
  if (!consp(e)) return symp(e)? var(e, env, e): e;
  lisp r= car(e)>0 ? evlist(cdr(e), env) : cdr(e); 
  switch(L(car(e))/2) {

  #define M(CD,OP) case CD: return mknum(num(car(r)) OP num(car(cdr(r))))
  M(0x57, +);M(0x5b, -);M(0x55, *);M(0x5f, /);M(0x4b, %);M(0x4d, &);M(0xf9, |);M(0x30eec9, &&);M(0x6fe5, ||);

  #define C(CD,OP) case CD: return (num(car(r)) OP num(car(cdr(r))))?t:0
  C(0x79, <);C(0x7b, ==);C(0x7d, >);

  #define S(CD,F) case CD: return F(car(r))
  S(0x31e1e5, car);S(0x31e4e5, cdr);S(0x7bf773e1, consp);S(0x1cb4eec7, princ);S(0x39f9db8b7ece1, symp);
// D(nump(lisp n), tag(s)==2)
  case 0x1bbaecd9: case 0x376fe9: return (lisp)(car(r)?0L:t); // not == null
  case 0x1cb4eee9: princ(car(r)); // print
  case 0xbcb872d3: return putchar('\n'),nil; // terpri
  case 0x1cb2e1c9: return rd(); // read

#define B(CD,F) case CD: return F(car(r), car(cdr(r)))
  B(0x18f7eee7, cons); B(0x65e3, eq); B(0xcbc7ae1d9, equ);
  B(0x3cf9efc7, assoc); B(0x197b61d9, eval);   //B(0x3c386cf3, apply);

  case 0x36e1e1: return nil; // map

  // TODO: no-eval
  case 0x6cc3b7164c3: return e; // lambda
  case 0xe3d77f4cb: return car(r); // quote

  // TODO: tail recursion
  case 0x69cd: return eval(eval(car(r), env)? car(cdr(r)): car(cdr(cdr(r))), env);
    
  default: if (!consp(car(e))) return princ(e);;
    return e=cdr(car(e)),eval(car(cdr(e)), bnd(car(e), r));
  }
  
  return e;
}



// ENDWCOUNT

#include <assert.h>

int main(int argc, char** argv) {
  assert(sizeof(long)==8); // require 64-bit

  // ( (x , 999) (y . 666) )
  lisp env= cons( cons( (lisp)0x1e3, mknum(999)),
	    cons( cons( (lisp)0x1e7, mknum(666)),
                  nil));

  //   ((lambda (x) (+ x x)) (* 3 4))     

  lisp x= nil;
  x= mknum(42);
  x= cons( (lisp)0xab, cons( mknum(111), cons(mknum(3), nil)));
  princ(x);
  putchar('\n');
  princ(eval(x, env));
  putchar('\n');

  fputc('>', stderr);
  lisp t;
  while(L((t=rd()))!=0x1c7ae9e9) { // quit
    printf("  [ %ld 0x%16lx ]\n", L(t), L(t)/2);
    princ(t); putchar('\n');
    princ(eval(t, env));
    putchar('\n');
    putchar('\n');
    fputc('>', stderr);
  }
}
