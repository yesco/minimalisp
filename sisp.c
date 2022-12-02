#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

typedef void* lisp;

#define L(a) ((long)a)

lisp mknum(long n) { return (void*)(n*2+1); }
long num(lisp n) { return L(n)/2; }

lisp tok() {
  int c= ' ';
  while(isspace(c)) c= getc(stdin);
  ungetc(c, stdin);
  long r= 0;
  while((c=fgetc(stdin))!=EOF) {
    if (isspace(c)||c=='('||c==')') break;
    r= r*(isdigit(c)?10:128) + (isdigit(c)?c-'0':c);
  }
  ungetc(c, stdin);
  printf("<%ld>", r);
  // map nil to 0
  return r==0x3769D9/2 ? 0 : mknum(r);
}

typedef struct cons {
  lisp car, cdr;
} *Cons;

lisp consp(lisp c) { return (void*)L(L(c && !(L(c)&7))); }
lisp car(Cons c) { return consp(c) ? c->car : 0; }
lisp cdr(Cons c) { return consp(c) ? c->cdr : 0; }
lisp cons(lisp a, lisp d) {
  return memcpy(malloc(sizeof(struct cons)), &(struct cons){a,d}, sizeof(struct cons));
  Cons c= malloc(sizeof(*c));
  c->car= a; c->cdr= d;
  return c;
}

lisp eq(lisp a, lisp b) { return (void*)(long)(a==b?1:0); }
       
lisp assoc(lisp v, lisp l) {
  while(consp(l) && !eq(v, car(car(l)))) l= cdr(l);
  return car(l);
}

lisp princ(lisp e) {
  if (!e) return printf("nil"),e;
  if (!consp(e)) return printf("%ld", num(e)),e;
  putchar('('); do {
    princ(car(e)); e= cdr(e); e && putchar(' ');
  } while (e);
  if (e) printf(" . "),princ(e);
  return putchar(')'),e;
}

lisp var(lisp v, lisp env, lisp def) {
  lisp e= assoc(v, env);
  return e ? cdr(e) : def;
}

lisp eval(lisp e, lisp env);

lisp evlist(lisp l, lisp env) {
  return !consp(l) ? l : cons(eval(car(l), env), evlist(cdr(l), env));
}

lisp eval(lisp e, lisp env) {
  if (!consp(e)) return var(e, env, e);
  lisp r= car(e)>0 ? evlist(cdr(e), env) : cdr(e); 
  switch((long)car(e)) {
  case 0x57: return mknum(num(car(r)) + num(car(cdr(r))));
  case 0x59: return mknum(num(car(r)) - num(car(cdr(r))));
  case 0x55: return mknum(num(car(r)) * num(car(cdr(r))));
  case 0x5f: return mknum(num(car(r)) / num(car(cdr(r))));
  case 0x31e1e5: return car(car(r));
  case 0x31e4e5: return cdr(car(r));
  case 0x18f7eee7: return cons(car(r), car(cdr(r)));
  case -0xe3d77f4cb: return car(r);
  case 0xcbc7ae1d9: // equal
  case 0x65e3: return eq(car(r), car(cdr(r)));
  case 0x36e1e1: // map
  case -0x69cd: // if
  default: printf("ERROR: "); princ(e); break;
  }
  return e;
}

lisp nil= NULL;

int main(int argc, char** argv) {
  lisp env= cons( cons( (void*)0xc3, mknum(999)),
	    cons( cons( (void*)0xc5, mknum(666)),
                  nil));

  lisp x= nil;
  x= mknum(42);
  x= cons( (void*)0x55, cons( mknum(111), cons(mknum(3), nil)));
  princ(x);
  putchar('\n');
  princ(eval(x, env));
  putchar('\n');

  fputc('>', stderr);
  lisp t;
  while((t=tok())) {
    printf("%ld 0x%lx\n", L(t), L(t));
    princ(eval(t, env));
    putchar('\n');
    fputc('>', stderr);
  }
}
