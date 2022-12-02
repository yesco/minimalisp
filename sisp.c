#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

long tok() {
  int c= ' ';
  while(isspace(c)) c= getc(stdin);
  long r= 0;
  while((c=fgetc(stdin))!=EOF) {
    if (isspace(c)||c=='('||c==')') break;
    r= r*(isdigit(c)?10:128) + (isdigit(c)?c-'0':c);
  }
  ungetc(c, stdin);
  return r*2+1;
}

typedef void* lisp;

typedef struct cons {
  lisp car, cdr;
} *Cons;

lisp consp(lisp c) { return (void*)(long)((((long)c)&7)==0 && c); }
lisp car(Cons c) { return consp(c) ? c->car : 0; }
lisp cdr(Cons c) { return consp(c) ? c->cdr : 0; }
lisp cons(lisp a, lisp d) {
  Cons c= malloc(sizeof(*c));
  c->car= a; c->cdr= d;
  return c;
}

lisp eq(lisp a, lisp b) { return (void*)(long)(a==b?1:0); }
       
lisp assoc(lisp v, lisp l) {
  while(consp(l) && eq(v, car(car(l)))) l= cdr(l);
  return car(l);
}

lisp princ(lisp e) {
  if (!e) return printf("nil"),e;
  if (!consp(e)) return printf("%ld", (long)e),e;
  putchar('('); princ(car(e)); printf(" . "); princ(cdr(e)); putchar(')');
  return e;
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
  case 0x30f2: return car(car(r));
  case 0x3272: return cdr(car(r));
  case 0x1bf773: return cons(car(r), car(cdr(r)));
  case -0xebbfa65: return car(r);
  case 0x71: return eq(car(r), car(cdr(r)));
  case 0xe3d70ec: // equal
  case 0x30f0: // map
  case -0x66: // if
  case 0x34ec: // nil
  case 't': // t
  default: printf("ERROR: "); princ(e); break;
  }
  return e;
}

lisp nil= NULL;

int main(int argc, char** argv) {
  lisp x= nil;
  x= (void*)42;
  princ(x);
  putchar('\n');
  princ(eval(x, nil));
  putchar('\n');

  exit(3);

  long t;
  while((t=tok())) {
    printf("%ld 0x%lx\n", t, t);
  }
}
