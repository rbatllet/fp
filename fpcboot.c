#define STANDALONE
#define DEFAULT_HEAP_SIZE 100000000
/* fp.h */


#ifndef FP_H
#define FP_H


#define GENERATED

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <sys/stat.h>
#include <setjmp.h>
#include <string.h>
#include <time.h>

#ifndef __MINGW32__
# define PRECISE_CLOCK
# include <sys/time.h>
# include <sys/resource.h>
#endif


#if defined(STRESS_GC) || defined(EXTRA_CAREFUL)
# ifndef CAREFUL
#  define CAREFUL
# endif
#endif

#define ERROR_FILE          "stderr.txt"

#ifndef DEFAULT_HEAP_SIZE
# define DEFAULT_HEAP_SIZE  2000000
#endif

// percentage when debug-mod will trigger warnings
#define HEAP_CRITICAL_SIZE  25

#define DEFAULT_ATOM_TABLE_SIZE 1000

#define WORD_SIZE           (sizeof(void *) * 8)

#define NUMBER_BIT          1
#define ATOM_BIT            2
#define NONSEQ_MASK         (NUMBER_BIT | ATOM_BIT)
#define GCMARK_BIT          (1UL << (WORD_SIZE - 1))

#define NUMBER_TYPE         NUMBER_BIT
#define ATOM_TYPE           ATOM_BIT
#define SEQUENCE_TYPE       0

#define SAVED_AREA_SIZE     10000
#define ROOT_SLOTS          16
#define DUMP_ELEMENT_LIMIT  100
#define DEFAULT_CALL_TRACE_LENGTH 16
#define DEFAULT_HANDY_BUFFER_SIZE 10000
#define MESSAGE_BUFFER_SIZE 1024
#define MAX_HASH_COUNT      64

#define LF_ROOT_SLOT        0

#ifdef CAREFUL
# define CHECK_FPTR(x)      check_fptr(x)
#else
# define CHECK_FPTR(x)
#endif

#ifdef CAREFUL
# define INLINE             static
#else
# define INLINE             static inline
#endif

#ifdef STACK_LIMIT
// assumes stack grows downward
# define CHECK_STACK  \
  char unused;					\
  if(&unused < stack_base - STACK_LIMIT)	\
    bomb("stack limit reached - aborting")
#else
# define CHECK_STACK
#endif

#define DEFINE(name)        static X name(X x)
#define XDEFINE(name)       X name(X x)
#define OP2DEFINE(name)     static X name ## _5fop2(X x, X y)

#ifdef CAREFUL
# define ENTRY              X *sp0 = savedp; CHECK_STACK; CONSULTCACHE(x)
# define RETURN(x)          {UPDATECACHE(x); check_saved(sp0); return check_fptr(x);}
#else
# define ENTRY              CONSULTCACHE(x)
# define RETURN(x)          {UPDATECACHE(x); return(x);}
#endif

#ifdef RCACHE
# define CONSULTCACHE(x)    static int g; static X r, a, a0; a0 = x; \
                            if(g == gc_count && x == a) { ++rcache_hits; return r; }
# define UPDATECACHE(x)     g = gc_count; a = a0; r = x
#else
# define CONSULTCACHE(x)
# define UPDATECACHE(x)
#endif


typedef void *X;

typedef struct S {
  unsigned long length;
  X data[ 1 ];
} S;

typedef char *A;

struct bucket {
  char *str;
  struct bucket *next;
};


#define IS_N(x)             ((long)(x) & NUMBER_BIT)
#define IS_A(x)             (((long)(x) & NONSEQ_MASK) == ATOM_BIT)
#define IS_S(x)             (((long)(x) & NONSEQ_MASK) == 0)
#define N_VALUE(x)          ((long)(x) >> 1)

// #define TO_N(n)             ((X)(long)(((n) << 1) | NUMBER_BIT))
#define TO_N(n) ((X)(long)((((long)(n)) * 2) | NUMBER_BIT))

#define A_STRING(x)         ((char *)((unsigned long)(x) & ~ATOM_BIT))
#define TO_A(ptr)           ((X)((unsigned long)(ptr) | ATOM_BIT))
#define S_LENGTH(x)         (((S *)(x))->length)
#define S_DATA(x)           (((S *)(x))->data)
#define TO_B(x)             ((x) ? T : F)


#define error_output        ensure_error_output()

static int debug_flag = 0;
static int no_strings_flag = 0;
static X *fromspace_start, *fromspace_end, *tospace_start, *tospace_end;
static X *fromspace_top, *tospace_top;
static int gc_count = 0;
static int rcache_hits = 0;
static X saved[ SAVED_AREA_SIZE ];
static X *savedp;
static char *buffer;
static int buffer_size;
static int lf_created = 0;
static int log_errors = 0;
static char message_buffer[ MESSAGE_BUFFER_SIZE ];

static struct bucket **atom_table;
static int atom_table_size;
static int atom_table_count;
static X T, F, ERROR, UNDEFINED, EMPTY, FPTR, IO;

static char **calltrace, **calltraceend, **calltracetop;
static jmp_buf *catcher;
static char *stack_base;
static char *max_stack_top;

static void dribble(char *msg, ...);
static X bomb(char *msg, ...);	/* doesn't throw */
static X fail(char *msg, ...);	/* throws */
static X failx(X x, char *msg, ...);	/* throws */
static X uncaught(X x);
static int init(int argc, char *argv[], X (*main)(X), void (*conslf)());
static X string(char *str);
static void dump(X x, FILE *fp, int limited);
static int is_string(X x, int nl);
static FILE *ensure_error_output();

#ifndef NDEBUG
static void dumptrace(FILE *fp);
static void tracecall(char *loc);
#else
# define tracecall(loc)
# define dumptrace(fp)
#endif

#ifdef CAREFUL
static void check_saved(X *sp0);
#endif

static void init_heap(unsigned int size);
static unsigned int shrink_heap(unsigned int size);
static unsigned int reclaim();
static X allocate(unsigned long length);
static X sequence(unsigned long length, ...);
static X fpointer(void *fptr);

static void init_atoms(unsigned int size);
static X intern(char *str);

#ifdef UNSAFE
# define check_N(x, loc)   N_VALUE(x)
# define check_A(x, loc)   A_STRING(x)
# define check_S(x, loc)   S_DATA(x)
# define check_Smin(x, minlen, loc)  S_DATA(x)
#else
static long check_N(X x, char *loc);
static char *check_A(X x, char *loc);
static X *check_S(X x, char *loc);
static X *check_Sn(X x, unsigned long len, char *loc);
static X *check_Smin(X x, unsigned long minlen, char *loc);
#endif

static char *check_string(X x, char *loc, int nl);
static X check_fptr(X x);

static int eq1(X x, X y);
static X select1(X x, X y);

DEFINE(___add);
DEFINE(___sub);
DEFINE(___mul);
DEFINE(___div);
DEFINE(___mod);
DEFINE(___band);
DEFINE(___bor);
DEFINE(___bxor);
OP2DEFINE(___add);
OP2DEFINE(___sub);
OP2DEFINE(___mul);
OP2DEFINE(___div);
OP2DEFINE(___mod);
OP2DEFINE(___band);
OP2DEFINE(___bor);
OP2DEFINE(___bxor);
DEFINE(___id);
DEFINE(___eq);
DEFINE(____5f);
DEFINE(___s1);
DEFINE(___s2);
DEFINE(___s3);
DEFINE(___s4);
DEFINE(____25trace);
DEFINE(___al);
DEFINE(___ar);
DEFINE(___tl);
DEFINE(___ton);
DEFINE(___toa);
DEFINE(___tos);
DEFINE(___cat);
DEFINE(___atom);
DEFINE(___len);
DEFINE(___num);
DEFINE(___select);
DEFINE(____5fin);
DEFINE(____5fout);
DEFINE(____5femit);
DEFINE(____5fget);
DEFINE(____5fshow);
DEFINE(____5fsystem);
DEFINE(____5fenv);
DEFINE(____5fthrow);
DEFINE(____5frnd);
DEFINE(___rev);
DEFINE(___cmp);
OP2DEFINE(___cmp);
DEFINE(___string);
DEFINE(____5fgc);
DEFINE(___app);
DEFINE(___subseq);
DEFINE(____5fiostep);
DEFINE(___make);
DEFINE(___iota);
DEFINE(___dl);
DEFINE(___dr);


#if defined(GENERATED) && !defined(STANDALONE)
# include <fp/gc.c>
# include <fp/ht.c>
# include <fp/rt.c>
# include <fp/lib.c>
#endif


#endif
/* gc.c - simple cheney-style collector */


#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>


#ifdef CAREFUL
INLINE void
check_saved(X *p)
{
  if(p != savedp)
    bomb("save/restore mismatch");
}
#endif


INLINE void
save(X x)
{
#ifdef CAREFUL
  if(savedp < saved + ROOT_SLOTS) 
    bomb("saved area underflow");
#endif

  *(savedp++) = x;
}


INLINE X
restore()
{
#ifdef CAREFUL
  if(savedp <= saved + ROOT_SLOTS) 
    bomb("saved area underflow");
#endif

  return *(--savedp);
}


INLINE X 
top()
{
#ifdef CAREFUL
  if(savedp <= saved + ROOT_SLOTS) 
    bomb("saved area underflow");
#endif

  return *(savedp - 1);
}


INLINE X
pick(int n)			/* 1-based! */
{
#ifdef CAREFUL
  if(savedp - n < saved + ROOT_SLOTS)
    bomb("saved area underflow");
#endif

  return *(savedp - n);
}


X
check_fptr(X x)
{
  if(IS_S(x)) {
    if((S_LENGTH(x) & GCMARK_BIT) != 0)
      bomb("forwarding pointer in sequence header");
#ifdef EXTRA_CAREFUL
    /* check all elements for being forwarding pointers */
    else {
      int i, len = S_LENGTH(x);
      X *a = S_DATA(x);
      
      for(i = 0; i < len; ++i) {
	if(IS_S(x) && (S_LENGTH(x) & GCMARK_BIT) != 0)
	  bomb("forwarding pointer in sequence header (item #%d of sequence %p)", i, x);
      }
    }
#endif
  }

  return x;
}


void
init_heap(unsigned int size)
{
  int i;

  dribble("[allocating heap of size %u]\n", size);
  size /= 2;
  fromspace_start = (X *)malloc(size);

  if(fromspace_start == NULL)
    bomb("out of memory - can not allocate fromspace of size %u", size);

  fromspace_end = (X *)((char *)fromspace_start + size);
  tospace_start = (X *)malloc(size);

  if(tospace_start == NULL)
    bomb("out of memory - can not allocate tospace of size %u", size);

  tospace_end = (X *)((char *)tospace_start + size);
  fromspace_top = fromspace_start;
  dribble("[  fromspace: %p - %p]\n", fromspace_start, fromspace_end);
  dribble("[    tospace: %p - %p]\n", tospace_start, tospace_end);
  savedp = saved + ROOT_SLOTS;

  for(i = 0; i < ROOT_SLOTS; ++i)
    saved[ i ] = F;
}


unsigned int
shrink_heap(unsigned int size)
{
  unsigned int lsize = (fromspace_end - fromspace_top) * sizeof(X);

  dribble("[shrinking fromspace of size %u to %u]\n", size / 2, lsize);

  fromspace_start = fromspace_top;
  tospace_start = (X *)realloc(tospace_start, lsize);
  
  if(tospace_start == NULL)
    bomb("out of memory - can not shrink tospace to size %u", lsize);

  tospace_end = (X *)((char *)tospace_start + lsize);
  dribble("[  fromspace: %p - %p]\n", fromspace_start, fromspace_end);
  dribble("[    tospace: %p - %p]\n", tospace_start, tospace_end);
  return lsize;
}


static void
mark(X *xp)
{
  X x = *xp;

  if(((long)x & NONSEQ_MASK) == 0) { /* if not a sequence, ignore */
    if(x >= (X)fromspace_start && x < (X)fromspace_end) { /* ignore pointers outside of heap */
      unsigned long len = S_LENGTH(x);

      if((len & GCMARK_BIT) != 0) /* if already marked, use forwarding pointer */
	*xp = (X)((len & ~GCMARK_BIT) << 1);
      else {			/* otherwise copy to tospace */
	int bytes = len * sizeof(X) + sizeof(unsigned long);
	memcpy(tospace_top, x, bytes);
	S_LENGTH(x) = GCMARK_BIT | ((unsigned long)tospace_top >> 1);
	*xp = (X)tospace_top;
	tospace_top += len + 1;
      }
    }
  }
}


unsigned int
reclaim()
{
  X *scan;
  unsigned int r;

  if(!lf_created)
    bomb("GC while creating literal frame - out of space");

  ++gc_count;
  //dribble("[GC #%d: starting, saved: %d]\n", gc_count, savedp - saved + ROOT_SLOTS);

  tospace_top = tospace_start;

  for(scan = saved; scan < savedp; ++scan)
    mark(scan);

  scan = tospace_start;

  while(scan < tospace_top) {
    unsigned long len = S_LENGTH((X)scan);

    while(len--)
      mark(++scan);

    ++scan;
  }

  scan = fromspace_start;
  fromspace_start = tospace_start;
  tospace_start = scan;
  scan = fromspace_end;
  fromspace_end = tospace_end;
  tospace_end = scan;
  fromspace_top = tospace_top;
  r = (fromspace_end - fromspace_top) * sizeof(X);

  if(debug_flag) {
    unsigned int p = (r * 100) / (fromspace_end - fromspace_start) * sizeof(X);
    
    if(p <= HEAP_CRITICAL_SIZE)
      dribble("[GC: heap low, remaining: %d%%]\n", p);
  }

  return r;
}


X
allocate(unsigned long length)
{
  S *s;
  X *p;
  unsigned long size;

#ifndef STRESS_GC
  if(fromspace_top + length + 1 > fromspace_end) {
#else 
  if(lf_created) {
#endif
    unsigned long space = reclaim();

    if(length + 1 > space / sizeof(X))
      bomb("heap size exceeded - can not allocate sequence of size %u", length);
  }

  s = (S *)fromspace_top;
  fromspace_top += length + 1;
  s->length = length;
  
  for(p = s->data; length--; ++p)
    *p = F;

  return (X)s;
}


X
sequence(unsigned long length, ...)
{
  va_list va;
  int i;
  X s;

  va_start(va, length);

  for(i = 0; i < length; ++i) 
    save(va_arg(va, X));

  s = allocate(length);

  for(i = length - 1; i >= 0; --i)
    S_DATA(s)[ i ] = restore();

  va_end(va);
  return s;
}
/* rt.c */


FILE *
ensure_error_output()
{
  static FILE *err = NULL;
  
  if(log_errors) {
    if(err == NULL)
      err = fopen(ERROR_FILE, "w");

    return err;
  }

  return stderr;
}


void
dribble(char *msg, ...)
{
  va_list va;

  if(debug_flag) {
    va_start(va, msg);
    vfprintf(error_output, msg, va);
    fflush(error_output);
    va_end(va);
  }
}


X
bomb(char *msg, ...)
{
  va_list va;

  va_start(va, msg);
  fputs("\n_|_\n", error_output);
  vfprintf(error_output, msg, va);
  fputc('\n', error_output);
  va_end(va);
  exit(EXIT_FAILURE);
}


static int
argument_value(char *arg)
{
  int len = strlen(arg);

  if(len > 0) {
    switch(arg[ len - 1 ]) {
    case 'k':
    case 'K':
      return atoi(arg) * 1024;

    case 'm':
    case 'M':
      return atoi(arg) * 1024 * 1024;

    case 'g':
    case 'G':
      return atoi(arg) * 1024 * 1024 * 1024;

    default:
      return atoi(arg);
    }
  }

  return 0;
}


int
init(int argc, char *argv[], X (*main)(X), void (*conslf)())
{
  unsigned int atoms = DEFAULT_ATOM_TABLE_SIZE;
  int i, c = argc - 1, j, ct = DEFAULT_CALL_TRACE_LENGTH;
  X args, x;
  jmp_buf exit_jmp;
  unsigned int heap = DEFAULT_HEAP_SIZE;
#ifdef PRECISE_CLOCK
  struct rusage startup_time;
  getrusage(RUSAGE_SELF, &startup_time);
#else
  clock_t startup_time = clock();
#endif
  int status = 0;

  catcher = &exit_jmp;
  srand(time(NULL));

  if(setjmp(*catcher)) {
    x = restore();

    if(IS_S(x) && S_LENGTH(x) == 2 && S_DATA(x)[ 0 ] == intern("EXIT")) {
      status = N_VALUE(S_DATA(x)[ 1 ]);
      goto finished;
    }
    else uncaught(x);
  }

  for(i = 1; i < argc; ++i) {
    if(*argv[ i ] == '-' && argv[ i ][ 1 ] == ':') {
      --c;

      switch(argv[ i ][ 2 ]) {
      case 'h':
	heap = argument_value(argv[ i ] + 3);
	break;

      case 'a':
	atoms = argument_value(argv[ i ] + 3);
	break;

      case 'd':
	debug_flag = 1;
	break;

      case 't':
	ct = atoi(argv[ i ] + 3);
	break;

      case '?':
        printf("%s [-:?] [-:hSIZE] [-:aSIZE] [-:d] [-:s] [-:e] [-:tLENGTH] "
	       "ARGUMENT ...\n", argv[ 0 ]);
	exit(0);

      case 's':
	no_strings_flag = 1;
	break;

      case 'e':
	log_errors = 1;
	break;

      default:
	bomb("invalid runtime option");
      }
    }
  }

  buffer = (char *)malloc(DEFAULT_HANDY_BUFFER_SIZE);

  if(buffer == NULL)
    fail("can not allocate temporary buffer of size %d", DEFAULT_HANDY_BUFFER_SIZE);

  buffer_size = DEFAULT_HANDY_BUFFER_SIZE;
  init_heap(heap);
  init_atoms(atoms);
  EMPTY = sequence(0);
  calltrace = (char **)malloc(sizeof(char *) * ct);

  if(calltrace == NULL)
    fail("can not allocate trace buffer");

  memset(calltrace, 0, sizeof(char *) * ct);
  calltracetop = calltrace;
  calltraceend = calltrace + ct;
  dribble("[creating literals]\n");
  conslf();
  lf_created = 1;
  heap = shrink_heap(heap);
 
  if(main != NULL) {
    args = allocate(c);
    j = 0;

    for(i = 1; i < argc; ++i) {
      if(*argv[ i ] != '-' || argv[ i ][ 1 ] != ':') {
	save(args);
	x = intern(argv[ i ]);
	args = restore();
	S_DATA(args)[ j++ ] = x;
      }
    }

    dribble("[start]\n");
    char unused;
    stack_base = &unused;
    max_stack_top = stack_base;
    x = main(args);

    if(savedp != saved + ROOT_SLOTS)
      fail("(internal) %d items on temporary stack left", savedp - saved);

  finished:    
    dribble("[%d atoms, %dk of %dk heap used, %d collections]\n", 
	    atom_table_count,
	    (sizeof(X) * (fromspace_top - fromspace_start)) / 1000,
	    heap / 2000, gc_count);
#ifndef NDEBUG
    dribble("[max stack depth: %d]\n", (char *)(&heap) - max_stack_top);
#endif
#ifdef RCACHE
    dribble("[%d r-cache hit%s]\n", rcache_hits, rcache_hits != 1 ? "s" : "");
#endif
#ifdef PRECISE_CLOCK
    struct rusage t2;
    getrusage(RUSAGE_SELF, &t2);
    dribble("[execution took %g seconds CPU time]\n", 
	    (double)(t2.ru_utime.tv_sec - startup_time.ru_utime.tv_sec) +
	    (double)(t2.ru_utime.tv_usec - startup_time.ru_utime.tv_usec) / 1000000);
#else
    dribble("[execution took %g seconds wall-clock time]\n", 
	    (double)(clock() - startup_time) / CLOCKS_PER_SEC);
#endif
    dribble("[exit status is %d]\n", status);
  }

  return status;
}


X
string(char *str)
{
  int i, len = strlen(str);
  X s = allocate(len);

  for(i = 0; *str; ++i)
    S_DATA(s)[ i ] = TO_N(*(str++));

  return s;
}


static int
printable_char(int c, int nl)
{
  return (c < 0x100 && isgraph(c)) 
    || c == ' ' || (nl && (c == '\n' || c == '\r' || c == '\t'));
}


int
is_string(X x, int nl)
{
  int i, len, c;
  X e;
  
  if(!IS_S(x)) return 0;

  len = S_LENGTH(x);

  for(i = 0; i < len; ++i) {
    e = S_DATA(x)[ i ];
    
    if(!IS_N(e)) return 0;
    
    c = N_VALUE(e);

    if(!printable_char(c, nl)) return 0;
  }

  return 1;
}


static int dump_count;

static void
dump1(X x, int ind, int indf, FILE *fp, int limit)
{
  int len, i;
  int s = 1, p = 0, comma = 0, ind2, ind2f;
  X e;

  if(indf) {
    fputc('\n', fp);

    for(i = 0; i < ind; ++i) fputc(' ', fp);
  }

  if(IS_N(x)) fprintf(fp, "%ld", N_VALUE(x));
  else if(IS_A(x)) fprintf(fp, "%s", A_STRING(x));
  else {
    len = S_LENGTH(x);

    if(!no_strings_flag) {
      for(i = 0; i < len; ++i) {
	int c;
	e = S_DATA(x)[ i ];
	
	if(IS_N(e)) {
	  c = N_VALUE(e);
	  
	  if(!printable_char(c, 0)) s = 0;
	}
	else if(IS_A(e)) s = 0;
	else if(is_string(e, 0)) s = 0;
	else {
	  p = 1;
	  s = 0;
	}
      }
    }
    else s = 0;

    if(s) s = len > 0;

    if(!s) fputc('<', fp);
    else fputc('\"', fp);

    ind2 = ind + 1;

    for(i = 0; i < len && dump_count++ < limit; ++i) {
      ind2f = p && i > 0;

      if(comma && !s) fputs(", ", fp);
      
      comma = 1;
      e = S_DATA(x)[ i ];

      if(IS_N(e)) {
	if(s) fputc(N_VALUE(e), fp);
	else dump1(e, ind2, ind2f, fp, limit);
      }
      else if(IS_A(e)) dump1(e, ind2, ind2f, fp, limit);
      else dump1(e, ind2, ind2f, fp, limit);
    }

    if(i < len) fputs(", ...", fp);

    if(!s) fputc('>', fp);
    else fputc('\"', fp);
  }
}


void
dump(X x, FILE *fp, int limited)
{
  dump_count = 0;
  dump1(x, 0, 0, fp, !limited ? 0x0fffffff : DUMP_ELEMENT_LIMIT);
  fputc('\n', fp);
  fflush(fp);
}


#ifndef NDEBUG
void
tracecall(char *loc)
{
  char dummy;

  if(calltracetop >= calltraceend) calltracetop = calltrace;

  *(calltracetop++) = loc;

  if(&dummy < max_stack_top)
    max_stack_top = &dummy;
}


void
dumptrace(FILE *fp)
{
  char **ptr = calltracetop + 1; 

  while(ptr != calltracetop) {
    if(ptr >= calltraceend) ptr = calltrace;
    else {
      if(*ptr != NULL) 
	fprintf(fp, "\n  %s", *ptr);

      ++ptr;
    }
  }

  fputs("\t\t<--\n", fp);
}
#endif
/* ht.c - atom table */


#include <string.h>
#include <stdlib.h>


static unsigned int
hash(char *str)
{
  unsigned int key = 0;
  int len = strlen(str);

  if(len > MAX_HASH_COUNT)
    len = MAX_HASH_COUNT;

  while(len--) key += (key << 5) + *(str++);

  return key;
}


void
init_atoms(unsigned int size)
{
  int i;

  atom_table = (struct bucket **)malloc(sizeof(struct bucket *) * size);

  if(atom_table == NULL)
    bomb("out of memory - can not allocate atom table of size %l", size);

  for(i = 0; i < size; ++i)
    atom_table[ i ] = NULL;

  atom_table_size = size;
  atom_table_count = 0;
  T = intern("T");
  F = intern("F");
  ERROR = intern("ERROR");
  UNDEFINED = intern("UNDEFINED");
  FPTR = intern("FPTR");
  IO = intern("IO");
}


X
intern(char *str)
{
  unsigned int key = hash(str) % atom_table_size;
  struct bucket *b;

  for(b = atom_table[ key ]; b != NULL; b = b->next)
    if(!strcmp(b->str, str)) return TO_A(b->str);

  b = (struct bucket *)malloc(sizeof(struct bucket));

  if(b == NULL) 
    bomb("out of memory - can not allocate bucket");

  b->str = strdup(str);
  b->next = atom_table[ key ];
  atom_table[ key ] = b;
  ++atom_table_count;
  return TO_A(b->str);
}
/* lib.c */


static X
fail(char *msg, ...)
{
  va_list va;

  va_start(va, msg);
  vsnprintf(message_buffer, MESSAGE_BUFFER_SIZE, msg, va);
  va_end(va);
  ____5fthrow(sequence(2, ERROR, string(message_buffer)));
  return F;
}


static X
failx(X x, char *msg, ...)
{
  va_list va;
  X s;

  save(x);
  va_start(va, msg);
  vsnprintf(message_buffer, MESSAGE_BUFFER_SIZE, msg, va);
  va_end(va);
  s = string(message_buffer);
  ____5fthrow(sequence(3, ERROR, s, restore()));
  return F;
}


static X
uncaught(X x)
{
  fputs("\nUncaught:\n", error_output);
  dump(x, error_output, 1);
  dumptrace(error_output);
  exit(EXIT_FAILURE);
  return F;
}


#ifndef UNSAFE
static long
check_N(X x, char *loc)
{
  CHECK_FPTR(x);

  if(!IS_N(x))
    failx(x, "bad argument type in `%s' - not a number", loc);

  return N_VALUE(x);
}


static char *
check_A(X x, char *loc)
{
  CHECK_FPTR(x);

  if(!IS_A(x))
    failx(x, "bad argument type in `%s' - not an atom", loc);

  return A_STRING(x);
}


static X *
check_S(X x, char *loc)
{
  CHECK_FPTR(x);

  if(!IS_S(x))
    failx(x, "bad argument type in `%s' - not a sequence", loc);

  return S_DATA(x);
}


static X *
check_Sn(X x, unsigned long len, char *loc)
{
  CHECK_FPTR(x);

  if(!IS_S(x))
    failx(x, "bad argument type in `%s' - not a sequence", loc);

  if(S_LENGTH(x) != len)
    failx(x, "bad argument type in `%s' - not a sequence of length %d", loc, len);

  return S_DATA(x);
}


static X *
check_Smin(X x, unsigned long minlen, char *loc)
{
  CHECK_FPTR(x);

  if(!IS_S(x))
    failx(x, "bad argument type in `%s' - not a sequence", loc);

  if(S_LENGTH(x) < minlen)
    failx(x, "bad argument type in `%s' - not a sequence of length %d or greater", 
	 loc, minlen);

  return S_DATA(x);
}
#endif


static char *
resize_buffer(int len)
{
  if(len > buffer_size) {
    buffer = (char *)realloc(buffer, len);

    if(buffer == NULL)
      fail("can not re-allocate temporary buffer from %d to %d bytes",
	   buffer_size, len);

    buffer_size = len;
  }

  return buffer;
}


static char *
check_string(X x, char *loc, int nl)
{
  CHECK_FPTR(x);

  if(IS_A(x)) return A_STRING(x);
  else if(is_string(x, nl)) {
    int i, len = S_LENGTH(x);
    char *ptr = resize_buffer(len);

    for(i = 0; i < len; ++i) {
      X e = S_DATA(x)[ i ];
      *(ptr++) = N_VALUE(e);
    }

    *ptr = '\0';
    return buffer;
  }
  else {
    failx(x, "bad argument type in `%s' - not a string or atom", loc);
    return NULL;
  }
}


X
fpointer(void *fptr)
{
  unsigned long addr = (unsigned long)fptr;

  return sequence(3, FPTR, TO_N(addr >> 16), TO_N(addr & 0xffff));
}


#define op2(name, op) \
  OP2DEFINE(___ ## name) { \
    return TO_N(check_N(x, #name) op check_N(y, #name)); \
  } \
  DEFINE(___ ## name) { \
    X *a = check_Sn(x, 2, #name); \
    return ___ ## name ## _5fop2(a[ 0 ], a[ 1 ]); \
  }

op2(add, +)
op2(sub, -)
op2(mul, *)
op2(band, &)
op2(bor, |)
op2(bxor, ^)
op2(bshl, <<)
op2(bshr, >>)


OP2DEFINE(___div)
{
  long q = check_N(y, "div");

  if(N_VALUE(q) == 0)
    failx(y, "division by zero", "div");

  return TO_N(check_N(x, "div") / q);
}

DEFINE(___div) {
  X *a = check_Sn(x, 2, "div");
  return ___div_5fop2(a[ 0 ], a[ 1 ]);
}  


OP2DEFINE(___mod)
{
  long q = check_N(y, "mod");

  if(N_VALUE(q) == 0)
    failx(y, "division by zero", "mod");

  return TO_N(check_N(x, "mod") % q);
}

DEFINE(___mod) {
  X *a = check_Sn(x, 2, "mod");
  return ___mod_5fop2(a[ 0 ], a[ 1 ]);
}  


DEFINE(___bnot) { return TO_N(~check_N(x, "bnot")); }
DEFINE(___id) { return x; }


static int 
eq1(X x, X y) 
{
 loop:
  if(x == y) return 1;

  if(IS_S(x) && IS_S(y)) {
    int i, len = S_LENGTH(x);

    if(S_LENGTH(y) != len) return 0;
    else if(len == 0) return 1;

    --len;

    for(i = 0; i < len; ++i) {
      if(!eq1(S_DATA(x)[ i ] , S_DATA(y)[ i ])) return 0;
    }

    x = S_DATA(x)[ i ];
    y = S_DATA(y)[ i ];
    goto loop;
  }

  return 0;
}


DEFINE(___eq)
{ 
  X *s = check_Sn(x, 2, "eq"); 
  return TO_B(eq1(s[ 0 ], s[ 1 ]));
}


static int
compare(X x, X y)
{
  if(IS_N(x)) {
    if(IS_N(y)) {
      if(x == y) return 0;	/* N1 <=> N2 */
      else if(N_VALUE(x) > N_VALUE(y)) return 1;
      else return -1;
    }
    else return -1;
  }
  else if(IS_A(x)) {
    if(IS_N(y)) return 1;	/* A <=> N */
    else if(IS_A(y)) return strcmp(A_STRING(x), A_STRING(y)); /* A1 <=> A1 */
    else return -1;					      /* A <=> S */
  }
  else if(!IS_S(y)) return 1;	/* S <=> A/N */
  else {
    int i, r;			/* S1 <=> S2 */

    for(i = 0; i < S_LENGTH(x); ++i) {
      if(i >= S_LENGTH(y)) return 1;

      r = compare(S_DATA(x)[ i ], S_DATA(y)[ i ]);

      if(r != 0) return r;
    }

    return 0;
  }
}


OP2DEFINE(___cmp)
{
  return TO_N(compare(x, y));
}

DEFINE(___cmp)
{
  X *s = check_Sn(x, 2, "cmp");
  return TO_N(compare(s[ 0 ], s[ 1 ]));
}


DEFINE(____5f)
{ 
  fputs("\n_|_:\n", error_output);
  dump(x, error_output, 1);
  dumptrace(error_output);
  exit(EXIT_FAILURE);
  return F;
}


DEFINE(____25trace)
{
  X *s = check_Sn(x, 2, "%trace");
  fprintf(error_output, "| %s: ", check_A(s[ 0 ], "%trace"));

  if(IS_S(s[ 1 ])) fputc('\n', error_output);

  dump(s[ 1 ], error_output, 1);
  return s[ 1 ];
}


DEFINE(____5fin)
{
  char *fn = check_string(x, "_in", 0);
  struct stat sb;
  int len, i;
  X buf, *ptr;
  FILE *fp;

  if(stat(fn, &sb) != 0) 
    fail("can not read file `%s': %s", fn, strerror(errno));

  if((sb.st_mode & S_IFMT) == S_IFDIR) 
    fail("can not read from directory `%s'", fn);

  len = sb.st_size;
  buf = allocate(len);
  ptr = S_DATA(buf);
  
  if((fp = fopen(fn, "rb")) == NULL)
    fail("can not open file `%s': %s", fn, strerror(errno));

  while(len > 0) {
    int n = len < buffer_size ? len : buffer_size;

    if(fread(buffer, n, 1, fp) != 1)
      fail("error while reading `%s': %s", fn, strerror(errno));

    len -= n;
    
    for(i = 0; i < n; ++i)
      *(ptr++) = TO_N(buffer[ i ]);
  }

  fclose(fp);
  return buf;
}


DEFINE(___al)
{
  X *a = check_Sn(x, 2, "al");
  X *b = check_S(a[ 1 ],  "al");
  int len = S_LENGTH(a[ 1 ]);
  X s;
  int i;

  save(a[ 0 ]);
  save(a[ 1 ]);
  s = allocate(len + 1);
  b = S_DATA(restore());
  
  for(i = 0; i < len; ++i)
    S_DATA(s)[ i + 1 ] = b[ i ];

  S_DATA(s)[ 0 ] = restore();
  return s;
}
  

DEFINE(___ar)
{
  X *a = check_Sn(x, 2, "ar");
  X *b = check_S(a[ 0 ],  "ar");
  int len = S_LENGTH(a[ 0 ]);
  X s;
  int i;

  save(a[ 1 ]);
  save(a[ 0 ]);
  s = allocate(len + 1);
  b = S_DATA(restore());
  
  for(i = 0; i < len; ++i)
    S_DATA(s)[ i ] = b[ i ];

  S_DATA(s)[ i ] = restore();
  return s;
}


DEFINE(___tl)
{
  X *a = check_Smin(x, 1, "tl");
  X s;
  int len = S_LENGTH(x);
  int i;

  save(x);
  s = allocate(len - 1);
  x = restore();
  
  for(i = 1; i < len; ++i)
    S_DATA(s)[ i - 1 ] = S_DATA(x)[ i ];

  return s;
}


DEFINE(___cat)
{
  X *a = check_S(x, "cat");
  X s, s2;
  int len = S_LENGTH(x);
  int i, j, len2, slen = 0;

  for(i = 0; i < len; ++i) {
    check_S(a[ i ], "cat");
    slen += S_LENGTH(a[ i ]);
  }

  save(x);
  s = allocate(slen);
  x = restore();
  slen = 0;

  for(i = 0; i < len; ++i) {
    s2 = S_DATA(x)[ i ];
    len2 = S_LENGTH(s2);

    for(j = 0; j < len2; ++j)
      S_DATA(s)[ slen++ ] = S_DATA(s2)[ j ];
  }

  return s;
}


DEFINE(___len) { check_S(x, "len"); return TO_N(S_LENGTH(x)); }
DEFINE(___atom) { return TO_B(!IS_S(x)); }
DEFINE(___num) { return TO_B(IS_N(x)); }


DEFINE(___tos)
{
  X s;
  char *ptr;
  int i, len;

  if(IS_S(x)) return x;

  if(IS_A(x)) ptr = A_STRING(x);
  else {
    snprintf(buffer, buffer_size, "%ld", N_VALUE(x));
    ptr = buffer;
  }

  len = strlen(ptr);
  s = allocate(len);
  
  for(i = 0; i < len; ++i)
    S_DATA(s)[ i ] = TO_N(*(ptr++));
  
  return s;
}


DEFINE(___ton)
{
  int i, len;
  X e;
  char *ptr;

  if(IS_N(x)) return x;

  if(IS_A(x)) return TO_N(atol(A_STRING(x)));

  len = S_LENGTH(x);
  ptr = buffer;

  if(len > buffer_size) len = buffer_size - 1;

  for(i = 0; i < len; ++i) {
    e = S_DATA(x)[ i ];

    if(!IS_N(e) || N_VALUE(e) >= 0x100 || !isdigit(N_VALUE(e))) break;
    else *(ptr++) = N_VALUE(e);
  }

  *ptr = '\0';
  return TO_N(atol(buffer));
}


DEFINE(___toa)
{
  int i, len;
  char *ptr;
  X e;

  if(IS_A(x)) return x;

  if(IS_N(x)) 
    snprintf(buffer, buffer_size, "%ld", N_VALUE(x));
  else {
    len = S_LENGTH(x);
    ptr = resize_buffer(len);

    for(i = 0; i < len; ++i) {
      e = S_DATA(x)[ i ];
      
      if(!IS_N(e))
	failx(x, "bad argument type in `toa' - not a string");
      else *(ptr++) = N_VALUE(e);
    }

    *ptr = '\0';
  }

  return intern(buffer);
}


DEFINE(____5femit)
{
  if(IS_N(x)) putchar(N_VALUE(x));
  else if(IS_A(x)) fputs(A_STRING(x), stdout);
  else {
    int i, len = S_LENGTH(x);

    for(i = 0; i < len; ++i) 
      ____5femit(S_DATA(x)[ i ]);
  }

  fflush(stdout);
  return x;
}


DEFINE(____5fget)
{
  int i, n = check_N(x, "_get");
  char *ptr = buffer;

  if(n >= buffer_size)
    failx(x, "can not `get' more than %d bytes at once", buffer_size);

  for(i = 0; i < n; ++i) {
    int c = getchar();

    if(c == EOF) { 
      n = i;
      break;
    }
    else *(ptr++) = getchar();  
  }

  x = allocate(n);

  for(i = 0; i < n; ++i)
    S_DATA(x)[ i ] = TO_N(buffer[ i ]);

  return x;
}


DEFINE(____5fout)
{
  X *s = check_Sn(x, 2, "_out");
  FILE *fp;
  char *name = check_string(S_DATA(x)[ 0 ], "_out", 0);
  X *data = check_S(S_DATA(x)[ 1 ], "_out");
  int len;

  check_S(data, "_out");
  len = S_LENGTH(S_DATA(x)[ 1 ]);

  if((fp = fopen(name, "wb")) == NULL)
    fail("can not open file `%s' for writing in `_out'", name);

  while(len--) {
    X x = *(data++);

    if(IS_N(x)) fputc(N_VALUE(x), fp);
    else failx(x, "data contains non-numeric value in `_out'");
  }

  fclose(fp);
  return x;
}


DEFINE(____5fshow)
{
  dump(x, stdout, 0);
  return x;
}


DEFINE(____5fsystem)
{
  return TO_N(system(check_string(x, "_system", 1)));
}


static X
select1(X index, X seq)
{
  int n = check_N(index, "select");
  X *b = check_S(seq, "select");

  if(n < 1 || n > S_LENGTH(seq))
    failx(seq, "index of %d out of bounds", n);

  return b[ n - 1 ];
}


DEFINE(___select)
{
  X *a = check_Sn(x, 2, "select");
  return select1(a[ 0 ], a[ 1 ]);
}


DEFINE(____5fenv)
{
  char *v = check_A(x, "_env");
  v = getenv(v);
  return v == NULL ? F : intern(v);
}


DEFINE(____5fthrow)
{
  save(x);
  longjmp(*catcher, 1);
}


DEFINE(____5frnd)
{
  return TO_N(rand() % check_N(x, "_rnd"));
}


DEFINE(___rev)
{
  X *s = check_S(x, "rev");
  int len;
  X s2;

  save(x);
  s2 = allocate(len = S_LENGTH(x));
  x = restore();
  s = S_DATA(x);
  
  while(--len >= 0)
    S_DATA(s2)[ len ] = *(s++);

  return s2;
}


DEFINE(___string)
{
  return TO_B(is_string(x, 1));
}


DEFINE(____5fgc)
{
  reclaim();
  return x;
}


DEFINE(___app)
{
  X *a = check_Sn(x, 2, "app");
  X *fptr = check_Sn(a[ 0 ], 3, "app");
  X (*func)(X);
  unsigned long addr;

  if(fptr[ 0 ] != FPTR)
    failx(fptr[ 0 ], "bad argument type in `app' - not a function pointer");

  addr = ((unsigned long)N_VALUE(fptr[ 1 ]) << 16) | (unsigned long)N_VALUE(fptr[ 2 ]);
  func = (X (*)(X))((void *)addr);
  return func(a[ 1 ]);
}


DEFINE(___subseq)
{
  X *a = check_Sn(x, 3, "subseq");
  int i = check_N(a[ 0 ], "subseq") - 1;
  int j = check_N(a[ 1 ], "subseq");
  X *s = check_S(a[ 2 ], "subseq");
  int len = S_LENGTH(a[ 2 ]);
  X s2;
  int k;

  if(len == 0 || i >= j || i >= len) 
    return EMPTY;

  if(i < 0) i = 0;

  if(i >= len) i = len - 1;

  if(j < 0) j = 0;

  if(j > len) j = len;

  save(a[ 2 ]);
  s2 = allocate(j - i);
  s = S_DATA(restore());

  for(k = 0; i < j; ++k, ++i)
    S_DATA(s2)[ k ] = s[ i ];

  return s2;
}


DEFINE(____5fiostep)
{
  static int iostep = 0;
  X *a = check_Sn(x, 2, "_iostep");
  X *b = check_Sn(a[ 0 ], 2, "_iostep");
  char *loc = check_A(a[ 1 ], "_iostep");
  
  if(b[ 0 ] != IO || !IS_N(b[ 1 ]))
    failx(b[ 0 ], "bad argument type in `%s' - not a valid I/O tag", loc);

  if(N_VALUE(b[ 1 ]) != iostep)
    failx(a[ 0 ], "I/O operation in `%s' out of order", loc);

  return sequence(2, IO, TO_N(++iostep));
}


DEFINE(___make)
{
  X *a = check_Sn(x, 2, "make");
  int i, len;
  X s;
  
  len = check_N(a[ 0 ], "make");

  if(len < 0)
    failx(a[ 0 ], "invalid length given to `make'");

  save(a[ 1 ]);
  s = allocate(len);
  x = restore();

  for(i = 0; i < len; ++i)
    S_DATA(s)[ i ] = x;

  return s;
}


DEFINE(___iota)
{
  int n = check_N(x, "iota");
  int i;
  X s;

  if(n < 0)
    failx(x, "invalid length given to `iota'");

  s = allocate(n);

  for(i = 1; i <= n; ++i)
    S_DATA(s)[ i - 1 ] = TO_N(i);

  return s;
}


DEFINE(___dl)
{
  X *a = check_Sn(x, 2, "dl");
  X r, p;
  int i, len;

  check_S(a[ 1 ], "dl");
  len = S_LENGTH(a[ 1 ]);
  save(a[ 0 ]);
  save(a[ 1 ]);
  r = allocate(len);
  save(r);

  for(i = 0; i < len; ++i) {
    p = sequence(2, pick(3), S_DATA(pick(2))[ i ]);
    S_DATA(top())[ i ] = p;
  }

  r = restore();
  restore();
  restore();
  return r;
}


DEFINE(___dr)
{
  X *a = check_Sn(x, 2, "dr");
  X s;
  X r, p;
  int i, len;

  check_S(a[ 0 ], "dr");
  len = S_LENGTH(a[ 0 ]);
  save(a[ 1 ]);
  save(a[ 0 ]);
  r = allocate(len);
  save(r);

  for(i = 0; i < len; ++i) {
    p = sequence(2, S_DATA(pick(2))[ i ], pick(3));
    S_DATA(top())[ i ] = p;
  }

  r = restore();
  restore();
  restore();
  return r;
}
/* GENERATED BY fpc */
#define GENERATED
#ifndef STANDALONE
#include <fp/fp.h>
#endif
static X lf[388];
static void gen_lf(){
lf[0]=intern("exit");
lf[1]=intern("EXIT");
lf[2]=sequence(42,TO_N(99),TO_N(111),TO_N(109),TO_N(109),TO_N(97),TO_N(110),TO_N(100),TO_N(32),TO_N(102),TO_N(97),TO_N(105),TO_N(108),TO_N(101),TO_N(100),TO_N(32),TO_N(119),TO_N(105),TO_N(116),TO_N(104),TO_N(32),TO_N(110),TO_N(111),TO_N(110),TO_N(45),TO_N(122),TO_N(101),TO_N(114),TO_N(111),TO_N(32),TO_N(101),TO_N(120),TO_N(105),TO_N(116),TO_N(32),TO_N(115),TO_N(116),TO_N(97),TO_N(116),TO_N(117),TO_N(115),TO_N(58),TO_N(32));
lf[3]=sequence(1,TO_N(32));
lf[4]=sequence(3,TO_N(103),TO_N(99),TO_N(99));
lf[5]=intern("CC");
lf[6]=sequence(3,TO_N(45),TO_N(73),TO_N(46));
lf[7]=sequence(11,TO_N(116),TO_N(114),TO_N(97),TO_N(110),TO_N(115),TO_N(108),TO_N(97),TO_N(116),TO_N(105),TO_N(110),TO_N(103));
lf[8]=sequence(9,sequence(22,TO_N(47),TO_N(42),TO_N(32),TO_N(71),TO_N(69),TO_N(78),TO_N(69),TO_N(82),TO_N(65),TO_N(84),TO_N(69),TO_N(68),TO_N(32),TO_N(66),TO_N(89),TO_N(32),TO_N(102),TO_N(112),TO_N(99),TO_N(32),TO_N(42),TO_N(47)),sequence(1,TO_N(10)),sequence(17,TO_N(35),TO_N(100),TO_N(101),TO_N(102),TO_N(105),TO_N(110),TO_N(101),TO_N(32),TO_N(71),TO_N(69),TO_N(78),TO_N(69),TO_N(82),TO_N(65),TO_N(84),TO_N(69),TO_N(68)),sequence(1,TO_N(10)),sequence(18,TO_N(35),TO_N(105),TO_N(102),TO_N(110),TO_N(100),TO_N(101),TO_N(102),TO_N(32),TO_N(83),TO_N(84),TO_N(65),TO_N(78),TO_N(68),TO_N(65),TO_N(76),TO_N(79),TO_N(78),TO_N(69)),sequence(1,TO_N(10)),sequence(18,TO_N(35),TO_N(105),TO_N(110),TO_N(99),TO_N(108),TO_N(117),TO_N(100),TO_N(101),TO_N(32),TO_N(60),TO_N(102),TO_N(112),TO_N(47),TO_N(102),TO_N(112),TO_N(46),TO_N(104),TO_N(62)),sequence(1,TO_N(10)),sequence(6,TO_N(35),TO_N(101),TO_N(110),TO_N(100),TO_N(105),TO_N(102)));
lf[9]=sequence(12,TO_N(115),TO_N(116),TO_N(97),TO_N(116),TO_N(105),TO_N(99),TO_N(32),TO_N(88),TO_N(32),TO_N(108),TO_N(102),TO_N(91));
lf[10]=sequence(21,TO_N(115),TO_N(116),TO_N(97),TO_N(116),TO_N(105),TO_N(99),TO_N(32),TO_N(118),TO_N(111),TO_N(105),TO_N(100),TO_N(32),TO_N(103),TO_N(101),TO_N(110),TO_N(95),TO_N(108),TO_N(102),TO_N(40),TO_N(41),TO_N(123));
lf[11]=sequence(1,TO_N(125));
lf[12]=sequence(43,TO_N(47),TO_N(47),TO_N(45),TO_N(45),TO_N(45),TO_N(45),TO_N(45),TO_N(45),TO_N(45),TO_N(45),TO_N(45),TO_N(45),TO_N(45),TO_N(45),TO_N(45),TO_N(45),TO_N(45),TO_N(45),TO_N(45),TO_N(45),TO_N(45),TO_N(45),TO_N(45),TO_N(45),TO_N(45),TO_N(45),TO_N(45),TO_N(45),TO_N(45),TO_N(45),TO_N(45),TO_N(45),TO_N(45),TO_N(45),TO_N(45),TO_N(45),TO_N(45),TO_N(45),TO_N(45),TO_N(45),TO_N(45),TO_N(45),TO_N(32));
lf[13]=sequence(7,TO_N(68),TO_N(69),TO_N(70),TO_N(73),TO_N(78),TO_N(69),TO_N(40));
lf[14]=sequence(6,TO_N(69),TO_N(78),TO_N(84),TO_N(82),TO_N(89),TO_N(59));
lf[15]=sequence(6,TO_N(108),TO_N(111),TO_N(111),TO_N(112),TO_N(58),TO_N(59));
lf[16]=sequence(10,TO_N(116),TO_N(114),TO_N(97),TO_N(99),TO_N(101),TO_N(99),TO_N(97),TO_N(108),TO_N(108),TO_N(40));
lf[17]=sequence(3,TO_N(58),TO_N(32),TO_N(32));
lf[18]=sequence(7,TO_N(82),TO_N(69),TO_N(84),TO_N(85),TO_N(82),TO_N(78),TO_N(40));
lf[19]=sequence(3,TO_N(41),TO_N(59),TO_N(125));
lf[20]=sequence(1,TO_N(120));
lf[21]=sequence(10,TO_N(35),TO_N(105),TO_N(110),TO_N(99),TO_N(108),TO_N(117),TO_N(100),TO_N(101),TO_N(32),TO_N(34));
lf[22]=sequence(32,TO_N(105),TO_N(110),TO_N(116),TO_N(32),TO_N(109),TO_N(97),TO_N(105),TO_N(110),TO_N(40),TO_N(105),TO_N(110),TO_N(116),TO_N(32),TO_N(97),TO_N(114),TO_N(103),TO_N(99),TO_N(44),TO_N(99),TO_N(104),TO_N(97),TO_N(114),TO_N(32),TO_N(42),TO_N(97),TO_N(114),TO_N(103),TO_N(118),TO_N(91),TO_N(93),TO_N(41),TO_N(123));
lf[23]=sequence(22,TO_N(114),TO_N(101),TO_N(116),TO_N(117),TO_N(114),TO_N(110),TO_N(32),TO_N(105),TO_N(110),TO_N(105),TO_N(116),TO_N(40),TO_N(97),TO_N(114),TO_N(103),TO_N(99),TO_N(44),TO_N(97),TO_N(114),TO_N(103),TO_N(118),TO_N(44));
lf[24]=sequence(10,TO_N(44),TO_N(103),TO_N(101),TO_N(110),TO_N(95),TO_N(108),TO_N(102),TO_N(41),TO_N(59),TO_N(125));
lf[25]=sequence(17,TO_N(47),TO_N(42),TO_N(32),TO_N(69),TO_N(78),TO_N(68),TO_N(32),TO_N(79),TO_N(70),TO_N(32),TO_N(70),TO_N(73),TO_N(76),TO_N(69),TO_N(32),TO_N(42),TO_N(47));
lf[26]=sequence(13,TO_N(61),TO_N(61),TO_N(61),TO_N(32),TO_N(108),TO_N(105),TO_N(116),TO_N(101),TO_N(114),TO_N(97),TO_N(108),TO_N(115),TO_N(58));
lf[27]=sequence(2,sequence(1,TO_N(99)),sequence(1,TO_N(104)));
lf[28]=sequence(11,TO_N(61),TO_N(61),TO_N(61),TO_N(32),TO_N(116),TO_N(111),TO_N(107),TO_N(101),TO_N(110),TO_N(115),TO_N(58));
lf[29]=sequence(10,TO_N(112),TO_N(114),TO_N(101),TO_N(108),TO_N(117),TO_N(100),TO_N(101),TO_N(46),TO_N(102),TO_N(112));
lf[30]=sequence(4,intern("F"),intern("F"),sequence(0),sequence(0));
lf[31]=sequence(2,intern("EXIT"),TO_N(70));
lf[32]=TO_N(35);
lf[33]=TO_N(33);
lf[34]=intern("-");
lf[35]=TO_N(95);
lf[36]=sequence(42,TO_N(97),TO_N(98),TO_N(99),TO_N(100),TO_N(101),TO_N(102),TO_N(103),TO_N(104),TO_N(105),TO_N(106),TO_N(107),TO_N(108),TO_N(109),TO_N(110),TO_N(111),TO_N(112),TO_N(113),TO_N(114),TO_N(115),TO_N(116),TO_N(117),TO_N(118),TO_N(119),TO_N(120),TO_N(121),TO_N(122),TO_N(48),TO_N(49),TO_N(50),TO_N(51),TO_N(52),TO_N(53),TO_N(54),TO_N(55),TO_N(56),TO_N(57),TO_N(95),TO_N(37),TO_N(36),TO_N(39),TO_N(63),TO_N(33));
lf[37]=sequence(10,TO_N(48),TO_N(49),TO_N(50),TO_N(51),TO_N(52),TO_N(53),TO_N(54),TO_N(55),TO_N(56),TO_N(57));
lf[38]=sequence(42,TO_N(65),TO_N(66),TO_N(67),TO_N(68),TO_N(69),TO_N(70),TO_N(71),TO_N(72),TO_N(73),TO_N(74),TO_N(75),TO_N(76),TO_N(77),TO_N(78),TO_N(79),TO_N(80),TO_N(81),TO_N(82),TO_N(83),TO_N(84),TO_N(85),TO_N(86),TO_N(87),TO_N(88),TO_N(89),TO_N(90),TO_N(48),TO_N(49),TO_N(50),TO_N(51),TO_N(52),TO_N(53),TO_N(54),TO_N(55),TO_N(56),TO_N(57),TO_N(95),TO_N(37),TO_N(36),TO_N(39),TO_N(63),TO_N(33));
lf[39]=sequence(26,TO_N(64),TO_N(47),TO_N(92),TO_N(126),TO_N(61),TO_N(58),TO_N(59),TO_N(40),TO_N(41),TO_N(123),TO_N(125),TO_N(91),TO_N(93),TO_N(60),TO_N(62),TO_N(46),TO_N(44),TO_N(45),TO_N(38),TO_N(124),TO_N(43),TO_N(63),TO_N(33),TO_N(35),TO_N(94),TO_N(42));
lf[40]=sequence(17,TO_N(105),TO_N(108),TO_N(108),TO_N(101),TO_N(103),TO_N(97),TO_N(108),TO_N(32),TO_N(115),TO_N(121),TO_N(110),TO_N(116),TO_N(97),TO_N(120),TO_N(58),TO_N(32),TO_N(96));
lf[41]=sequence(25,TO_N(117),TO_N(110),TO_N(101),TO_N(120),TO_N(112),TO_N(101),TO_N(99),TO_N(116),TO_N(101),TO_N(100),TO_N(32),TO_N(101),TO_N(110),TO_N(100),TO_N(32),TO_N(111),TO_N(102),TO_N(32),TO_N(108),TO_N(105),TO_N(116),TO_N(101),TO_N(114),TO_N(97),TO_N(108));
lf[42]=TO_N(110);
lf[43]=TO_N(114);
lf[44]=TO_N(116);
lf[45]=sequence(9,TO_N(58),TO_N(32),TO_N(101),TO_N(114),TO_N(114),TO_N(111),TO_N(114),TO_N(58),TO_N(32));
lf[46]=intern("_");
lf[47]=intern("{");
lf[48]=intern("}");
lf[49]=intern("=");
lf[50]=intern("+");
lf[51]=intern("->");
lf[52]=intern(";");
lf[53]=intern("&");
lf[54]=intern("|");
lf[55]=intern("^");
lf[56]=intern("(");
lf[57]=intern(")");
lf[58]=intern("[");
lf[59]=intern("[|");
lf[60]=intern("(|");
lf[61]=intern("|)");
lf[62]=intern("~");
lf[63]=sequence(2,intern("T"),intern("F"));
lf[64]=intern("@");
lf[65]=intern("/");
lf[66]=intern("\\");
lf[67]=intern("trace");
lf[68]=intern("%trace");
lf[69]=intern("#");
lf[70]=intern("?");
lf[71]=intern("!");
lf[72]=sequence(24,TO_N(105),TO_N(110),TO_N(118),TO_N(97),TO_N(108),TO_N(105),TO_N(100),TO_N(32),TO_N(105),TO_N(110),TO_N(100),TO_N(101),TO_N(120),TO_N(32),TO_N(102),TO_N(111),TO_N(114),TO_N(32),TO_N(83),TO_N(69),TO_N(76),TO_N(69),TO_N(67),TO_N(84));
lf[73]=intern("*");
lf[74]=intern("while");
lf[75]=intern("catch");
lf[76]=intern(".");
lf[77]=intern("unit");
lf[78]=intern("extern");
lf[79]=intern(":");
lf[80]=intern("ID");
lf[81]=intern("]");
lf[82]=sequence(19,TO_N(101),TO_N(120),TO_N(112),TO_N(101),TO_N(99),TO_N(116),TO_N(101),TO_N(100),TO_N(32),TO_N(96),TO_N(44),TO_N(39),TO_N(32),TO_N(111),TO_N(114),TO_N(32),TO_N(96),TO_N(93),TO_N(39));
lf[83]=intern("..");
lf[84]=intern("|]");
lf[85]=sequence(20,TO_N(101),TO_N(120),TO_N(112),TO_N(101),TO_N(99),TO_N(116),TO_N(101),TO_N(100),TO_N(32),TO_N(96),TO_N(44),TO_N(39),TO_N(32),TO_N(111),TO_N(114),TO_N(32),TO_N(96),TO_N(124),TO_N(93),TO_N(39));
lf[86]=intern("NUM");
lf[87]=intern("ATOM");
lf[88]=intern("<");
lf[89]=intern(">");
lf[90]=intern("SYM");
lf[91]=intern(",");
lf[92]=intern("STRING");
lf[93]=sequence(16,TO_N(105),TO_N(110),TO_N(118),TO_N(97),TO_N(108),TO_N(105),TO_N(100),TO_N(32),TO_N(99),TO_N(111),TO_N(110),TO_N(115),TO_N(116),TO_N(97),TO_N(110),TO_N(116));
lf[94]=sequence(11,TO_N(32),TO_N(98),TO_N(117),TO_N(116),TO_N(32),TO_N(102),TO_N(111),TO_N(117),TO_N(110),TO_N(100),TO_N(32));
lf[95]=sequence(9,TO_N(101),TO_N(120),TO_N(112),TO_N(101),TO_N(99),TO_N(116),TO_N(101),TO_N(100),TO_N(32));
lf[96]=sequence(5,sequence(2,intern("ID"),sequence(10,TO_N(105),TO_N(100),TO_N(101),TO_N(110),TO_N(116),TO_N(105),TO_N(102),TO_N(105),TO_N(101),TO_N(114))),sequence(2,intern("ATOM"),sequence(4,TO_N(97),TO_N(116),TO_N(111),TO_N(109))),sequence(2,intern("SYM"),sequence(6,TO_N(115),TO_N(121),TO_N(109),TO_N(98),TO_N(111),TO_N(108))),sequence(2,intern("STRING"),sequence(6,TO_N(115),TO_N(116),TO_N(114),TO_N(105),TO_N(110),TO_N(103))),sequence(2,intern("NUM"),sequence(6,TO_N(110),TO_N(117),TO_N(109),TO_N(98),TO_N(101),TO_N(114))));
lf[97]=sequence(23,TO_N(32),TO_N(98),TO_N(117),TO_N(116),TO_N(32),TO_N(102),TO_N(111),TO_N(117),TO_N(110),TO_N(100),TO_N(32),TO_N(101),TO_N(110),TO_N(100),TO_N(32),TO_N(111),TO_N(102),TO_N(32),TO_N(105),TO_N(110),TO_N(112),TO_N(117),TO_N(116));
lf[98]=sequence(10,TO_N(101),TO_N(120),TO_N(112),TO_N(101),TO_N(99),TO_N(116),TO_N(101),TO_N(100),TO_N(32),TO_N(96));
lf[99]=sequence(13,TO_N(39),TO_N(32),TO_N(98),TO_N(117),TO_N(116),TO_N(32),TO_N(102),TO_N(111),TO_N(117),TO_N(110),TO_N(100),TO_N(32),TO_N(96));
lf[100]=sequence(23,TO_N(117),TO_N(110),TO_N(101),TO_N(120),TO_N(112),TO_N(101),TO_N(99),TO_N(116),TO_N(101),TO_N(100),TO_N(32),TO_N(101),TO_N(110),TO_N(100),TO_N(32),TO_N(111),TO_N(102),TO_N(32),TO_N(105),TO_N(110),TO_N(112),TO_N(117),TO_N(116));
lf[101]=sequence(5,TO_N(69),TO_N(114),TO_N(114),TO_N(111),TO_N(114));
lf[102]=sequence(5,intern("ICONST"),intern("CONST"),intern("EXTERN"),intern("REF"),intern("POINTER2"));
lf[103]=sequence(16,TO_N(32),TO_N(32),TO_N(112),TO_N(114),TO_N(111),TO_N(103),TO_N(114),TO_N(97),TO_N(109),TO_N(32),TO_N(115),TO_N(105),TO_N(122),TO_N(101),TO_N(58),TO_N(32));
lf[104]=sequence(26,TO_N(32),TO_N(100),TO_N(101),TO_N(102),TO_N(105),TO_N(110),TO_N(105),TO_N(116),TO_N(105),TO_N(111),TO_N(110),TO_N(115),TO_N(44),TO_N(32),TO_N(116),TO_N(111),TO_N(116),TO_N(97),TO_N(108),TO_N(32),TO_N(115),TO_N(105),TO_N(122),TO_N(101),TO_N(58),TO_N(32));
lf[105]=sequence(36,TO_N(32),TO_N(32),TO_N(100),TO_N(114),TO_N(111),TO_N(112),TO_N(112),TO_N(105),TO_N(110),TO_N(103),TO_N(32),TO_N(117),TO_N(110),TO_N(117),TO_N(115),TO_N(101),TO_N(100),TO_N(32),TO_N(100),TO_N(101),TO_N(102),TO_N(105),TO_N(110),TO_N(105),TO_N(116),TO_N(105),TO_N(111),TO_N(110),TO_N(115),TO_N(32),TO_N(40),TO_N(112),TO_N(97),TO_N(115),TO_N(115),TO_N(32));
lf[106]=sequence(2,TO_N(41),TO_N(58));
lf[107]=sequence(4,TO_N(32),TO_N(32),TO_N(32),TO_N(32));
lf[108]=sequence(3,TO_N(58),TO_N(32),TO_N(40));
lf[109]=sequence(3,TO_N(41),TO_N(58),TO_N(32));
lf[110]=sequence(12,TO_N(99),TO_N(111),TO_N(110),TO_N(116),TO_N(114),TO_N(97),TO_N(99),TO_N(116),TO_N(97),TO_N(98),TO_N(108),TO_N(101));
lf[111]=sequence(5,TO_N(115),TO_N(105),TO_N(122),TO_N(101),TO_N(32));
lf[112]=intern("INVALID");
lf[113]=sequence(22,TO_N(97),TO_N(112),TO_N(112),TO_N(108),TO_N(121),TO_N(105),TO_N(110),TO_N(103),TO_N(32),TO_N(114),TO_N(101),TO_N(119),TO_N(114),TO_N(105),TO_N(116),TO_N(101),TO_N(32),TO_N(114),TO_N(117),TO_N(108),TO_N(101),TO_N(115));
lf[114]=sequence(2,sequence(0),intern("F"));
lf[115]=sequence(2,TO_N(32),TO_N(40));
lf[116]=intern("converting recursive definition into loop");
lf[117]=sequence(4,TO_N(116),TO_N(109),TO_N(112),TO_N(37));
lf[118]=sequence(1,TO_N(37));
lf[119]=intern("T");
lf[120]=sequence(17,TO_N(58),TO_N(32),TO_N(99),TO_N(111),TO_N(110),TO_N(115),TO_N(116),TO_N(97),TO_N(110),TO_N(116),TO_N(32),TO_N(105),TO_N(110),TO_N(100),TO_N(101),TO_N(120),TO_N(32));
lf[121]=sequence(13,TO_N(32),TO_N(111),TO_N(117),TO_N(116),TO_N(32),TO_N(111),TO_N(102),TO_N(32),TO_N(114),TO_N(97),TO_N(110),TO_N(103),TO_N(101));
lf[122]=intern("COMP/id");
lf[123]=intern("normalized composition");
lf[124]=sequence(12,TO_N(112),TO_N(114),TO_N(105),TO_N(109),TO_N(105),TO_N(116),TO_N(105),TO_N(118),TO_N(101),TO_N(32),TO_N(69),TO_N(81));
lf[125]=intern("eq");
lf[126]=sequence(24,TO_N(112),TO_N(114),TO_N(105),TO_N(109),TO_N(105),TO_N(116),TO_N(105),TO_N(118),TO_N(101),TO_N(32),TO_N(69),TO_N(81),TO_N(32),TO_N(40),TO_N(114),TO_N(101),TO_N(103),TO_N(114),TO_N(111),TO_N(117),TO_N(112),TO_N(101),TO_N(100),TO_N(41));
lf[127]=sequence(16,TO_N(112),TO_N(114),TO_N(105),TO_N(109),TO_N(105),TO_N(116),TO_N(105),TO_N(118),TO_N(101),TO_N(32),TO_N(83),TO_N(69),TO_N(76),TO_N(69),TO_N(67),TO_N(84));
lf[128]=intern("select");
lf[129]=sequence(28,TO_N(112),TO_N(114),TO_N(105),TO_N(109),TO_N(105),TO_N(116),TO_N(105),TO_N(118),TO_N(101),TO_N(32),TO_N(83),TO_N(69),TO_N(76),TO_N(69),TO_N(67),TO_N(84),TO_N(32),TO_N(40),TO_N(114),TO_N(101),TO_N(103),TO_N(114),TO_N(111),TO_N(117),TO_N(112),TO_N(101),TO_N(100),TO_N(41));
lf[130]=intern("ALPHA/ALPHA fusion");
lf[131]=intern("ALPHA/ALPHA fusion (regrouped)");
lf[132]=intern("constant conditional");
lf[133]=intern("always equal");
lf[134]=intern("constant equal");
lf[135]=intern("introducing f-pointer temporary");
lf[136]=intern("direct function application");
lf[137]=intern("direct function application (regrouped)");
lf[138]=intern("cat/ALPHA promotion");
lf[139]=intern("cat");
lf[140]=intern("cat/ALPHA promotion (regrouped)");
lf[141]=intern("ALPHA/id elimination");
lf[142]=intern("catamorphism/ALPHA fusion");
lf[143]=intern("catamorphism/ALPHA fusion (regrouped)");
lf[144]=intern("ALPHA/CONS fusion");
lf[145]=intern("ALPHA/CONS fusion (regrouped)");
lf[146]=intern("banana split");
lf[147]=intern("merged SELECT/composition");
lf[148]=intern("merged EQ/composition");
lf[149]=intern("constant construction");
lf[150]=sequence(7,TO_N(69),TO_N(114),TO_N(114),TO_N(111),TO_N(114),TO_N(58),TO_N(32));
lf[151]=sequence(33,TO_N(58),TO_N(32),TO_N(99),TO_N(111),TO_N(110),TO_N(115),TO_N(116),TO_N(97),TO_N(110),TO_N(116),TO_N(32),TO_N(83),TO_N(69),TO_N(76),TO_N(69),TO_N(67),TO_N(84),TO_N(32),TO_N(111),TO_N(102),TO_N(32),TO_N(110),TO_N(111),TO_N(110),TO_N(45),TO_N(115),TO_N(101),TO_N(113),TO_N(117),TO_N(101),TO_N(110),TO_N(99),TO_N(101));
lf[152]=intern("constant SELECT");
lf[153]=intern("construction cancellation");
lf[154]=sequence(3,intern("POINTER2"),intern("ICONST"),intern("CONST"));
lf[155]=intern("composition with constant");
lf[156]=intern("constant OR");
lf[157]=intern("composition hoisting (OR)");
lf[158]=intern("composition hoisting (conditional)");
lf[159]=intern("reverse composition hoisting (conditional)");
lf[160]=sequence(4,intern("REF"),intern("POINTER2"),intern("EXTERN"),intern("ICONST"));
lf[161]=sequence(7,TO_N(105),TO_N(110),TO_N(116),TO_N(101),TO_N(114),TO_N(110),TO_N(40));
lf[162]=sequence(2,TO_N(34),TO_N(92));
lf[163]=sequence(9,TO_N(115),TO_N(101),TO_N(113),TO_N(117),TO_N(101),TO_N(110),TO_N(99),TO_N(101),TO_N(40));
lf[164]=sequence(11,intern("add"),intern("sub"),intern("mul"),intern("div"),intern("mod"),intern("band"),intern("bor"),intern("bxor"),intern("bshl"),intern("bshr"),intern("cmp"));
lf[165]=sequence(4,TO_N(61),TO_N(95),TO_N(95),TO_N(95));
lf[166]=sequence(7,TO_N(95),TO_N(53),TO_N(102),TO_N(111),TO_N(112),TO_N(50),TO_N(40));
lf[167]=sequence(11,TO_N(41),TO_N(59),TO_N(32),TO_N(47),TO_N(47),TO_N(32),TO_N(79),TO_N(80),TO_N(50),TO_N(58),TO_N(32));
lf[168]=sequence(6,TO_N(125),TO_N(101),TO_N(108),TO_N(115),TO_N(101),TO_N(123));
lf[169]=sequence(5,TO_N(33),TO_N(61),TO_N(70),TO_N(41),TO_N(123));
lf[170]=sequence(10,TO_N(41),TO_N(59),TO_N(32),TO_N(47),TO_N(47),TO_N(32),TO_N(67),TO_N(79),TO_N(78),TO_N(68));
lf[171]=sequence(7,TO_N(61),TO_N(116),TO_N(111),TO_N(112),TO_N(40),TO_N(41),TO_N(59));
lf[172]=sequence(7,TO_N(83),TO_N(95),TO_N(68),TO_N(65),TO_N(84),TO_N(65),TO_N(40));
lf[173]=sequence(9,TO_N(61),TO_N(112),TO_N(105),TO_N(99),TO_N(107),TO_N(40),TO_N(50),TO_N(41),TO_N(59));
lf[174]=sequence(10,TO_N(41),TO_N(59),TO_N(32),TO_N(47),TO_N(47),TO_N(32),TO_N(67),TO_N(79),TO_N(78),TO_N(83));
lf[175]=sequence(3,TO_N(108),TO_N(102),TO_N(91));
lf[176]=sequence(10,TO_N(59),TO_N(32),TO_N(47),TO_N(47),TO_N(32),TO_N(67),TO_N(79),TO_N(78),TO_N(83),TO_N(84));
lf[177]=sequence(5,TO_N(84),TO_N(79),TO_N(95),TO_N(78),TO_N(40));
lf[178]=sequence(5,TO_N(69),TO_N(77),TO_N(80),TO_N(84),TO_N(89));
lf[179]=sequence(1,TO_N(84));
lf[180]=sequence(1,TO_N(70));
lf[181]=sequence(11,TO_N(59),TO_N(32),TO_N(47),TO_N(47),TO_N(32),TO_N(73),TO_N(67),TO_N(79),TO_N(78),TO_N(83),TO_N(84));
lf[182]=sequence(14,TO_N(83),TO_N(95),TO_N(68),TO_N(65),TO_N(84),TO_N(65),TO_N(40),TO_N(116),TO_N(111),TO_N(112),TO_N(40),TO_N(41),TO_N(41),TO_N(91));
lf[183]=sequence(2,TO_N(93),TO_N(61));
lf[184]=sequence(10,TO_N(59),TO_N(32),TO_N(47),TO_N(47),TO_N(32),TO_N(65),TO_N(76),TO_N(80),TO_N(72),TO_N(65));
lf[185]=sequence(10,TO_N(61),TO_N(97),TO_N(108),TO_N(108),TO_N(111),TO_N(99),TO_N(97),TO_N(116),TO_N(101),TO_N(40));
lf[186]=sequence(3,TO_N(61),TO_N(48),TO_N(59));
lf[187]=sequence(17,TO_N(61),TO_N(83),TO_N(95),TO_N(68),TO_N(65),TO_N(84),TO_N(65),TO_N(40),TO_N(112),TO_N(105),TO_N(99),TO_N(107),TO_N(40),TO_N(50),TO_N(41),TO_N(41),TO_N(91));
lf[188]=sequence(12,TO_N(59),TO_N(32),TO_N(47),TO_N(47),TO_N(32),TO_N(73),TO_N(78),TO_N(83),TO_N(69),TO_N(82),TO_N(84),TO_N(82));
lf[189]=sequence(2,TO_N(41),TO_N(91));
lf[190]=sequence(4,TO_N(45),TO_N(49),TO_N(93),TO_N(59));
lf[191]=sequence(3,TO_N(45),TO_N(50),TO_N(59));
lf[192]=sequence(28,TO_N(61),TO_N(115),TO_N(101),TO_N(113),TO_N(117),TO_N(101),TO_N(110),TO_N(99),TO_N(101),TO_N(40),TO_N(50),TO_N(44),TO_N(83),TO_N(95),TO_N(68),TO_N(65),TO_N(84),TO_N(65),TO_N(40),TO_N(112),TO_N(105),TO_N(99),TO_N(107),TO_N(40),TO_N(49),TO_N(41),TO_N(41),TO_N(91));
lf[193]=sequence(12,TO_N(59),TO_N(32),TO_N(47),TO_N(47),TO_N(32),TO_N(73),TO_N(78),TO_N(83),TO_N(69),TO_N(82),TO_N(84),TO_N(76));
lf[194]=sequence(8,TO_N(99),TO_N(104),TO_N(101),TO_N(99),TO_N(107),TO_N(95),TO_N(83),TO_N(40));
lf[195]=sequence(10,TO_N(61),TO_N(83),TO_N(95),TO_N(76),TO_N(69),TO_N(78),TO_N(71),TO_N(84),TO_N(72),TO_N(40));
lf[196]=sequence(4,TO_N(61),TO_N(61),TO_N(48),TO_N(63));
lf[197]=sequence(8,TO_N(58),TO_N(83),TO_N(95),TO_N(68),TO_N(65),TO_N(84),TO_N(65),TO_N(40));
lf[198]=sequence(5,TO_N(41),TO_N(91),TO_N(48),TO_N(93),TO_N(59));
lf[199]=sequence(3,TO_N(61),TO_N(49),TO_N(59));
lf[200]=sequence(3,TO_N(59),TO_N(43),TO_N(43));
lf[201]=sequence(12,TO_N(61),TO_N(115),TO_N(101),TO_N(113),TO_N(117),TO_N(101),TO_N(110),TO_N(99),TO_N(101),TO_N(40),TO_N(50),TO_N(44));
lf[202]=sequence(17,TO_N(44),TO_N(83),TO_N(95),TO_N(68),TO_N(65),TO_N(84),TO_N(65),TO_N(40),TO_N(112),TO_N(105),TO_N(99),TO_N(107),TO_N(40),TO_N(49),TO_N(41),TO_N(41),TO_N(91));
lf[203]=sequence(3,TO_N(93),TO_N(41),TO_N(59));
lf[204]=sequence(14,TO_N(99),TO_N(104),TO_N(101),TO_N(99),TO_N(107),TO_N(95),TO_N(83),TO_N(40),TO_N(116),TO_N(111),TO_N(112),TO_N(40),TO_N(41),TO_N(44));
lf[205]=sequence(7,TO_N(40),TO_N(124),TO_N(46),TO_N(46),TO_N(46),TO_N(124),TO_N(41));
lf[206]=sequence(1,TO_N(34));
lf[207]=sequence(4,TO_N(105),TO_N(110),TO_N(116),TO_N(32));
lf[208]=sequence(17,TO_N(61),TO_N(83),TO_N(95),TO_N(76),TO_N(69),TO_N(78),TO_N(71),TO_N(84),TO_N(72),TO_N(40),TO_N(116),TO_N(111),TO_N(112),TO_N(40),TO_N(41),TO_N(41),TO_N(59));
lf[209]=sequence(4,TO_N(102),TO_N(111),TO_N(114),TO_N(40));
lf[210]=sequence(3,TO_N(45),TO_N(49),TO_N(59));
lf[211]=sequence(6,TO_N(62),TO_N(61),TO_N(48),TO_N(59),TO_N(45),TO_N(45));
lf[212]=sequence(2,TO_N(41),TO_N(123));
lf[213]=sequence(26,TO_N(61),TO_N(115),TO_N(101),TO_N(113),TO_N(117),TO_N(101),TO_N(110),TO_N(99),TO_N(101),TO_N(40),TO_N(50),TO_N(44),TO_N(83),TO_N(95),TO_N(68),TO_N(65),TO_N(84),TO_N(65),TO_N(40),TO_N(116),TO_N(111),TO_N(112),TO_N(40),TO_N(41),TO_N(41),TO_N(91));
lf[214]=sequence(2,TO_N(93),TO_N(44));
lf[215]=sequence(10,TO_N(41),TO_N(59),TO_N(32),TO_N(47),TO_N(47),TO_N(32),TO_N(75),TO_N(65),TO_N(84),TO_N(65));
lf[216]=sequence(2,TO_N(120),TO_N(61));
lf[217]=sequence(10,TO_N(59),TO_N(32),TO_N(47),TO_N(47),TO_N(32),TO_N(82),TO_N(69),TO_N(70),TO_N(58),TO_N(32));
lf[218]=sequence(25,TO_N(103),TO_N(111),TO_N(116),TO_N(111),TO_N(32),TO_N(108),TO_N(111),TO_N(111),TO_N(112),TO_N(59),TO_N(32),TO_N(47),TO_N(47),TO_N(32),TO_N(116),TO_N(97),TO_N(105),TO_N(108),TO_N(32),TO_N(99),TO_N(97),TO_N(108),TO_N(108),TO_N(58),TO_N(32));
lf[219]=sequence(11,TO_N(41),TO_N(59),TO_N(32),TO_N(47),TO_N(47),TO_N(32),TO_N(82),TO_N(69),TO_N(70),TO_N(58),TO_N(32));
lf[220]=sequence(4,TO_N(40),TO_N(88),TO_N(41),TO_N(59));
lf[221]=sequence(17,TO_N(61),TO_N(102),TO_N(112),TO_N(111),TO_N(105),TO_N(110),TO_N(116),TO_N(101),TO_N(114),TO_N(40),TO_N(40),TO_N(118),TO_N(111),TO_N(105),TO_N(100),TO_N(42),TO_N(41));
lf[222]=sequence(16,TO_N(41),TO_N(59),TO_N(32),TO_N(47),TO_N(47),TO_N(32),TO_N(80),TO_N(79),TO_N(73),TO_N(78),TO_N(84),TO_N(69),TO_N(82),TO_N(50),TO_N(58),TO_N(32));
lf[223]=sequence(9,TO_N(41),TO_N(59),TO_N(32),TO_N(47),TO_N(47),TO_N(32),TO_N(83),TO_N(69),TO_N(81));
lf[224]=sequence(5,TO_N(61),TO_N(61),TO_N(70),TO_N(41),TO_N(123));
lf[225]=sequence(8,TO_N(41),TO_N(59),TO_N(32),TO_N(47),TO_N(47),TO_N(32),TO_N(79),TO_N(82));
lf[226]=sequence(11,TO_N(61),TO_N(61),TO_N(70),TO_N(41),TO_N(32),TO_N(98),TO_N(114),TO_N(101),TO_N(97),TO_N(107),TO_N(59));
lf[227]=sequence(10,TO_N(59),TO_N(32),TO_N(47),TO_N(47),TO_N(32),TO_N(87),TO_N(72),TO_N(73),TO_N(76),TO_N(69));
lf[228]=sequence(8,TO_N(102),TO_N(111),TO_N(114),TO_N(40),TO_N(59),TO_N(59),TO_N(41),TO_N(123));
lf[229]=sequence(2,TO_N(59),TO_N(125));
lf[230]=sequence(5,TO_N(101),TO_N(108),TO_N(115),TO_N(101),TO_N(123));
lf[231]=sequence(3,TO_N(88),TO_N(32),TO_N(42));
lf[232]=sequence(8,TO_N(61),TO_N(115),TO_N(97),TO_N(118),TO_N(101),TO_N(100),TO_N(112),TO_N(44));
lf[233]=sequence(10,TO_N(59),TO_N(32),TO_N(47),TO_N(47),TO_N(32),TO_N(67),TO_N(65),TO_N(84),TO_N(67),TO_N(72));
lf[234]=sequence(9,TO_N(106),TO_N(109),TO_N(112),TO_N(95),TO_N(98),TO_N(117),TO_N(102),TO_N(32),TO_N(42));
lf[235]=sequence(9,TO_N(61),TO_N(99),TO_N(97),TO_N(116),TO_N(99),TO_N(104),TO_N(101),TO_N(114),TO_N(44));
lf[236]=sequence(9,TO_N(99),TO_N(97),TO_N(116),TO_N(99),TO_N(104),TO_N(101),TO_N(114),TO_N(61),TO_N(38));
lf[237]=sequence(10,TO_N(105),TO_N(102),TO_N(40),TO_N(115),TO_N(101),TO_N(116),TO_N(106),TO_N(109),TO_N(112),TO_N(40));
lf[238]=sequence(3,TO_N(41),TO_N(41),TO_N(123));
lf[239]=sequence(7,TO_N(115),TO_N(97),TO_N(118),TO_N(101),TO_N(100),TO_N(112),TO_N(61));
lf[240]=sequence(8,TO_N(99),TO_N(97),TO_N(116),TO_N(99),TO_N(104),TO_N(101),TO_N(114),TO_N(61));
lf[241]=sequence(9,TO_N(101),TO_N(120),TO_N(116),TO_N(101),TO_N(114),TO_N(110),TO_N(32),TO_N(88),TO_N(32));
lf[242]=sequence(16,TO_N(40),TO_N(88),TO_N(32),TO_N(120),TO_N(41),TO_N(59),TO_N(32),TO_N(47),TO_N(47),TO_N(32),TO_N(69),TO_N(88),TO_N(84),TO_N(69),TO_N(82),TO_N(78));
lf[243]=sequence(1,TO_N(61));
lf[244]=sequence(2,TO_N(61),TO_N(40));
lf[245]=sequence(2,TO_N(61),TO_N(61));
lf[246]=sequence(7,TO_N(41),TO_N(124),TO_N(124),TO_N(101),TO_N(113),TO_N(49),TO_N(40));
lf[247]=sequence(12,TO_N(41),TO_N(63),TO_N(84),TO_N(58),TO_N(70),TO_N(59),TO_N(32),TO_N(47),TO_N(47),TO_N(32),TO_N(69),TO_N(81));
lf[248]=sequence(14,TO_N(61),TO_N(115),TO_N(101),TO_N(108),TO_N(101),TO_N(99),TO_N(116),TO_N(49),TO_N(40),TO_N(84),TO_N(79),TO_N(95),TO_N(78),TO_N(40));
lf[249]=sequence(2,TO_N(41),TO_N(44));
lf[250]=sequence(21,TO_N(41),TO_N(59),TO_N(32),TO_N(47),TO_N(47),TO_N(32),TO_N(83),TO_N(69),TO_N(76),TO_N(69),TO_N(67),TO_N(84),TO_N(32),TO_N(40),TO_N(115),TO_N(105),TO_N(109),TO_N(112),TO_N(108),TO_N(101),TO_N(41));
lf[251]=sequence(9,TO_N(61),TO_N(115),TO_N(101),TO_N(108),TO_N(101),TO_N(99),TO_N(116),TO_N(49),TO_N(40));
lf[252]=sequence(1,TO_N(44));
lf[253]=sequence(12,TO_N(41),TO_N(59),TO_N(32),TO_N(47),TO_N(47),TO_N(32),TO_N(83),TO_N(69),TO_N(76),TO_N(69),TO_N(67),TO_N(84));
lf[254]=sequence(3,TO_N(61),TO_N(84),TO_N(59));
lf[255]=sequence(1,TO_N(58));
lf[256]=sequence(10,TO_N(114),TO_N(101),TO_N(115),TO_N(116),TO_N(111),TO_N(114),TO_N(101),TO_N(40),TO_N(41),TO_N(59));
lf[257]=sequence(2,sequence(1,TO_N(10)),sequence(10,TO_N(47),TO_N(47),TO_N(32),TO_N(115),TO_N(107),TO_N(105),TO_N(112),TO_N(112),TO_N(101),TO_N(100)));
lf[258]=sequence(3,TO_N(105),TO_N(102),TO_N(40));
lf[259]=sequence(10,TO_N(61),TO_N(61),TO_N(70),TO_N(41),TO_N(32),TO_N(103),TO_N(111),TO_N(116),TO_N(111),TO_N(32));
lf[260]=sequence(15,TO_N(61),TO_N(83),TO_N(95),TO_N(68),TO_N(65),TO_N(84),TO_N(65),TO_N(40),TO_N(116),TO_N(111),TO_N(112),TO_N(40),TO_N(41),TO_N(41),TO_N(91));
lf[261]=sequence(2,TO_N(93),TO_N(59));
lf[262]=sequence(2,TO_N(88),TO_N(32));
lf[263]=sequence(3,TO_N(61),TO_N(70),TO_N(59));
lf[264]=sequence(6,TO_N(41),TO_N(59),TO_N(32),TO_N(47),TO_N(47),TO_N(32));
lf[265]=sequence(9,TO_N(105),TO_N(102),TO_N(40),TO_N(33),TO_N(73),TO_N(83),TO_N(95),TO_N(83),TO_N(40));
lf[266]=sequence(12,TO_N(41),TO_N(124),TO_N(124),TO_N(83),TO_N(95),TO_N(76),TO_N(69),TO_N(78),TO_N(71),TO_N(84),TO_N(72),TO_N(40));
lf[267]=sequence(2,TO_N(33),TO_N(61));
lf[268]=sequence(7,TO_N(41),TO_N(32),TO_N(103),TO_N(111),TO_N(116),TO_N(111),TO_N(32));
lf[269]=sequence(7,sequence(2,intern("add"),sequence(7,TO_N(84),TO_N(79),TO_N(95),TO_N(78),TO_N(40),TO_N(48),TO_N(41))),sequence(2,intern("mul"),sequence(7,TO_N(84),TO_N(79),TO_N(95),TO_N(78),TO_N(40),TO_N(49),TO_N(41))),sequence(2,intern("sub"),sequence(7,TO_N(84),TO_N(79),TO_N(95),TO_N(78),TO_N(40),TO_N(48),TO_N(41))),sequence(2,intern("div"),sequence(7,TO_N(84),TO_N(79),TO_N(95),TO_N(78),TO_N(40),TO_N(49),TO_N(41))),sequence(2,intern("band"),sequence(7,TO_N(84),TO_N(79),TO_N(95),TO_N(78),TO_N(40),TO_N(48),TO_N(41))),sequence(2,intern("bor"),sequence(7,TO_N(84),TO_N(79),TO_N(95),TO_N(78),TO_N(40),TO_N(48),TO_N(41))),sequence(2,intern("bxor"),sequence(7,TO_N(84),TO_N(79),TO_N(95),TO_N(78),TO_N(40),TO_N(48),TO_N(41))));
lf[270]=sequence(5,sequence(5,TO_N(102),TO_N(97),TO_N(105),TO_N(108),TO_N(40)),sequence(1,TO_N(34)),sequence(13,TO_N(110),TO_N(111),TO_N(32),TO_N(117),TO_N(110),TO_N(105),TO_N(116),TO_N(32),TO_N(118),TO_N(97),TO_N(108),TO_N(117),TO_N(101)),sequence(1,TO_N(34)),sequence(1,TO_N(41)));
lf[271]=sequence(5,TO_N(115),TO_N(97),TO_N(118),TO_N(101),TO_N(40));
lf[272]=sequence(2,TO_N(41),TO_N(59));
lf[273]=sequence(11,TO_N(61),TO_N(114),TO_N(101),TO_N(115),TO_N(116),TO_N(111),TO_N(114),TO_N(101),TO_N(40),TO_N(41),TO_N(59));
lf[274]=sequence(3,TO_N(95),TO_N(95),TO_N(95));
lf[275]=sequence(62,TO_N(97),TO_N(98),TO_N(99),TO_N(100),TO_N(101),TO_N(102),TO_N(103),TO_N(104),TO_N(105),TO_N(106),TO_N(107),TO_N(108),TO_N(109),TO_N(110),TO_N(111),TO_N(112),TO_N(113),TO_N(114),TO_N(115),TO_N(116),TO_N(117),TO_N(118),TO_N(119),TO_N(120),TO_N(121),TO_N(122),TO_N(65),TO_N(66),TO_N(67),TO_N(68),TO_N(69),TO_N(70),TO_N(71),TO_N(72),TO_N(73),TO_N(74),TO_N(75),TO_N(76),TO_N(77),TO_N(78),TO_N(79),TO_N(80),TO_N(81),TO_N(82),TO_N(83),TO_N(84),TO_N(85),TO_N(86),TO_N(87),TO_N(88),TO_N(89),TO_N(90),TO_N(48),TO_N(49),TO_N(50),TO_N(51),TO_N(52),TO_N(53),TO_N(54),TO_N(55),TO_N(56),TO_N(57));
lf[276]=sequence(2,TO_N(95),TO_N(48));
lf[277]=sequence(2,TO_N(97),TO_N(10));
lf[278]=sequence(7,TO_N(32),TO_N(61),TO_N(9),TO_N(9),TO_N(9),TO_N(37),TO_N(32));
lf[279]=sequence(3,TO_N(1),sequence(0),sequence(2,TO_N(32),TO_N(32)));
lf[280]=sequence(1,TO_N(10));
lf[281]=sequence(1,TO_N(63));
lf[282]=intern("id");
lf[283]=sequence(1,TO_N(33));
lf[284]=sequence(1,TO_N(41));
lf[285]=sequence(1,TO_N(40));
lf[286]=sequence(3,TO_N(32),TO_N(38),TO_N(32));
lf[287]=intern("COND");
lf[288]=sequence(1,TO_N(59));
lf[289]=sequence(3,TO_N(32),TO_N(45),TO_N(62));
lf[290]=intern("CONST");
lf[291]=intern("ICONST");
lf[292]=sequence(1,TO_N(126));
lf[293]=intern("REF");
lf[294]=intern("app");
lf[295]=sequence(1,TO_N(94));
lf[296]=intern("ALPHA");
lf[297]=sequence(1,TO_N(64));
lf[298]=intern("INSERTR");
lf[299]=sequence(1,TO_N(47));
lf[300]=intern("INSERTL");
lf[301]=sequence(1,TO_N(92));
lf[302]=intern("KATA");
lf[303]=sequence(2,TO_N(124),TO_N(41));
lf[304]=sequence(2,TO_N(40),TO_N(124));
lf[305]=intern("WHILE");
lf[306]=sequence(6,TO_N(119),TO_N(104),TO_N(105),TO_N(108),TO_N(101),TO_N(32));
lf[307]=intern("CATCH");
lf[308]=sequence(6,TO_N(99),TO_N(97),TO_N(116),TO_N(99),TO_N(104),TO_N(32));
lf[309]=intern("SEQ");
lf[310]=sequence(2,TO_N(59),TO_N(32));
lf[311]=intern("EXTERN");
lf[312]=sequence(7,TO_N(101),TO_N(120),TO_N(116),TO_N(101),TO_N(114),TO_N(110),TO_N(32));
lf[313]=intern("UNIT");
lf[314]=sequence(5,TO_N(117),TO_N(110),TO_N(105),TO_N(116),TO_N(32));
lf[315]=intern("OR");
lf[316]=sequence(3,TO_N(32),TO_N(124),TO_N(32));
lf[317]=intern("EQ");
lf[318]=sequence(3,intern("REF"),intern("F"),intern("eq"));
lf[319]=intern("SELECT");
lf[320]=intern("COMP");
lf[321]=sequence(3,intern("REF"),intern("F"),intern("select"));
lf[322]=sequence(2,intern("CONS"),intern("T"));
lf[323]=intern("POINTER1");
lf[324]=intern("POINTER2");
lf[325]=sequence(1,TO_N(42));
lf[326]=intern("CONS");
lf[327]=sequence(1,TO_N(93));
lf[328]=sequence(1,TO_N(91));
lf[329]=intern("PCONS");
lf[330]=sequence(2,TO_N(124),TO_N(93));
lf[331]=intern("PCONSL");
lf[332]=sequence(6,TO_N(44),TO_N(32),TO_N(46),TO_N(46),TO_N(124),TO_N(93));
lf[333]=sequence(2,TO_N(91),TO_N(124));
lf[334]=sequence(1,TO_N(96));
lf[335]=sequence(2,TO_N(60),TO_N(62));
lf[336]=sequence(1,TO_N(62));
lf[337]=sequence(1,TO_N(60));
lf[338]=sequence(2,TO_N(44),TO_N(32));
lf[339]=sequence(6,sequence(2,intern("SEQ"),TO_N(1)),sequence(2,intern("COND"),TO_N(2)),sequence(2,intern("OR"),TO_N(3)),sequence(2,intern("WHILE"),TO_N(4)),sequence(2,intern("CATCH"),TO_N(4)),sequence(2,intern("COMP"),TO_N(6)));
lf[340]=sequence(18,TO_N(111),TO_N(112),TO_N(116),TO_N(105),TO_N(109),TO_N(105),TO_N(122),TO_N(97),TO_N(116),TO_N(105),TO_N(111),TO_N(110),TO_N(32),TO_N(112),TO_N(97),TO_N(115),TO_N(115),TO_N(32));
lf[341]=sequence(10,TO_N(61),TO_N(61),TO_N(61),TO_N(32),TO_N(110),TO_N(111),TO_N(100),TO_N(101),TO_N(115),TO_N(58));
lf[342]=sequence(8,TO_N(105),TO_N(110),TO_N(108),TO_N(105),TO_N(110),TO_N(105),TO_N(110),TO_N(103));
lf[343]=sequence(2,TO_N(32),TO_N(32));
lf[344]=sequence(2,TO_N(58),TO_N(32));
lf[345]=sequence(16,TO_N(32),TO_N(105),TO_N(115),TO_N(32),TO_N(99),TO_N(111),TO_N(110),TO_N(116),TO_N(114),TO_N(97),TO_N(99),TO_N(116),TO_N(97),TO_N(98),TO_N(108),TO_N(101));
lf[346]=intern("_start");
lf[347]=sequence(34,TO_N(58),TO_N(32),TO_N(101),TO_N(114),TO_N(114),TO_N(111),TO_N(114),TO_N(58),TO_N(32),TO_N(109),TO_N(117),TO_N(108),TO_N(116),TO_N(105),TO_N(112),TO_N(108),TO_N(101),TO_N(32),TO_N(100),TO_N(101),TO_N(102),TO_N(105),TO_N(110),TO_N(105),TO_N(116),TO_N(105),TO_N(111),TO_N(110),TO_N(115),TO_N(32),TO_N(111),TO_N(102),TO_N(32),TO_N(96));
lf[348]=sequence(36,TO_N(58),TO_N(32),TO_N(101),TO_N(114),TO_N(114),TO_N(111),TO_N(114),TO_N(58),TO_N(32),TO_N(114),TO_N(101),TO_N(100),TO_N(101),TO_N(102),TO_N(105),TO_N(110),TO_N(105),TO_N(116),TO_N(105),TO_N(111),TO_N(110),TO_N(32),TO_N(111),TO_N(102),TO_N(32),TO_N(105),TO_N(110),TO_N(116),TO_N(114),TO_N(105),TO_N(110),TO_N(115),TO_N(105),TO_N(99),TO_N(32),TO_N(96));
lf[349]=sequence(46,intern("add"),intern("sub"),intern("mul"),intern("div"),intern("mod"),intern("band"),intern("bor"),intern("bxor"),intern("bnot"),intern("bshl"),intern("bshr"),intern("id"),intern("eq"),intern("select"),intern("_in"),intern("_out"),intern("_get"),intern("_emit"),intern("_show"),intern("_system"),intern("al"),intern("ar"),intern("tl"),intern("cat"),intern("len"),intern("num"),intern("rev"),intern("subseq"),intern("atom"),intern("tos"),intern("toa"),intern("ton"),intern("_throw"),intern("%trace"),intern("_"),intern("_rnd"),intern("_env"),intern("cmp"),intern("string"),intern("_gc"),intern("app"),intern("_iostep"),intern("make"),intern("iota"),intern("dl"),intern("dr"));
lf[350]=sequence(44,TO_N(58),TO_N(32),TO_N(101),TO_N(114),TO_N(114),TO_N(111),TO_N(114),TO_N(58),TO_N(32),TO_N(114),TO_N(101),TO_N(102),TO_N(101),TO_N(114),TO_N(101),TO_N(110),TO_N(99),TO_N(101),TO_N(32),TO_N(116),TO_N(111),TO_N(32),TO_N(117),TO_N(110),TO_N(100),TO_N(101),TO_N(102),TO_N(105),TO_N(110),TO_N(101),TO_N(100),TO_N(32),TO_N(105),TO_N(100),TO_N(101),TO_N(110),TO_N(116),TO_N(105),TO_N(102),TO_N(105),TO_N(101),TO_N(114),TO_N(32),TO_N(96));
lf[351]=sequence(1,TO_N(39));
lf[352]=sequence(10,TO_N(117),TO_N(110),TO_N(114),TO_N(101),TO_N(115),TO_N(111),TO_N(108),TO_N(118),TO_N(101),TO_N(100));
lf[353]=sequence(2,TO_N(99),TO_N(46));
lf[354]=sequence(1,TO_N(99));
lf[355]=sequence(5,TO_N(45),TO_N(104),TO_N(101),TO_N(108),TO_N(112));
lf[356]=sequence(8,TO_N(45),TO_N(118),TO_N(101),TO_N(114),TO_N(115),TO_N(105),TO_N(111),TO_N(110));
lf[357]=sequence(2,sequence(8,TO_N(50),TO_N(48),TO_N(48),TO_N(57),TO_N(48),TO_N(53),TO_N(49),TO_N(49)),sequence(1,TO_N(10)));
lf[358]=sequence(2,intern("EXIT"),TO_N(0));
lf[359]=sequence(6,TO_N(45),TO_N(100),TO_N(101),TO_N(98),TO_N(117),TO_N(103));
lf[360]=sequence(5,TO_N(45),TO_N(100),TO_N(117),TO_N(109),TO_N(112));
lf[361]=intern("DEBUG");
lf[362]=intern("DEBUGDUMP");
lf[363]=sequence(2,TO_N(45),TO_N(99));
lf[364]=intern("COMPILE");
lf[365]=sequence(6,TO_N(45),TO_N(108),TO_N(105),TO_N(109),TO_N(105),TO_N(116));
lf[366]=intern("LIMIT");
lf[367]=sequence(5,TO_N(45),TO_N(104),TO_N(101),TO_N(97),TO_N(112));
lf[368]=sequence(20,TO_N(45),TO_N(68),TO_N(68),TO_N(69),TO_N(70),TO_N(65),TO_N(85),TO_N(76),TO_N(84),TO_N(95),TO_N(72),TO_N(69),TO_N(65),TO_N(80),TO_N(95),TO_N(83),TO_N(73),TO_N(90),TO_N(69),TO_N(61));
lf[369]=sequence(2,TO_N(45),TO_N(111));
lf[370]=sequence(8,TO_N(45),TO_N(112),TO_N(114),TO_N(101),TO_N(108),TO_N(117),TO_N(100),TO_N(101));
lf[371]=intern("PRELUDE");
lf[372]=sequence(8,TO_N(45),TO_N(105),TO_N(110),TO_N(99),TO_N(108),TO_N(117),TO_N(100),TO_N(101));
lf[373]=intern("INCLUDE");
lf[374]=sequence(6,TO_N(45),TO_N(110),TO_N(111),TO_N(100),TO_N(101),TO_N(115));
lf[375]=intern("NODES");
lf[376]=sequence(8,TO_N(45),TO_N(117),TO_N(110),TO_N(112),TO_N(97),TO_N(114),TO_N(115),TO_N(101));
lf[377]=intern("UNPARSE");
lf[378]=sequence(9,TO_N(45),TO_N(110),TO_N(111),TO_N(45),TO_N(99),TO_N(104),TO_N(101),TO_N(99),TO_N(107));
lf[379]=intern("NO_CHECK");
lf[380]=sequence(12,TO_N(45),TO_N(101),TO_N(110),TO_N(116),TO_N(114),TO_N(121),TO_N(45),TO_N(112),TO_N(111),TO_N(105),TO_N(110),TO_N(116));
lf[381]=sequence(0);
lf[382]=sequence(240,TO_N(117),TO_N(115),TO_N(97),TO_N(103),TO_N(101),TO_N(58),TO_N(32),TO_N(102),TO_N(112),TO_N(99),TO_N(32),TO_N(91),TO_N(45),TO_N(104),TO_N(101),TO_N(108),TO_N(112),TO_N(93),TO_N(32),TO_N(91),TO_N(45),TO_N(118),TO_N(101),TO_N(114),TO_N(115),TO_N(105),TO_N(111),TO_N(110),TO_N(93),TO_N(32),TO_N(91),TO_N(45),TO_N(100),TO_N(101),TO_N(98),TO_N(117),TO_N(103),TO_N(93),TO_N(32),TO_N(91),TO_N(45),TO_N(100),TO_N(117),TO_N(109),TO_N(112),TO_N(93),TO_N(32),TO_N(91),TO_N(45),TO_N(99),TO_N(93),TO_N(32),TO_N(91),TO_N(45),TO_N(104),TO_N(101),TO_N(97),TO_N(112),TO_N(32),TO_N(72),TO_N(69),TO_N(65),TO_N(80),TO_N(83),TO_N(73),TO_N(90),TO_N(69),TO_N(93),TO_N(32),TO_N(10),TO_N(32),TO_N(32),TO_N(32),TO_N(32),TO_N(32),TO_N(32),TO_N(32),TO_N(32),TO_N(32),TO_N(32),TO_N(32),TO_N(91),TO_N(45),TO_N(60),TO_N(99),TO_N(99),TO_N(111),TO_N(112),TO_N(116),TO_N(105),TO_N(111),TO_N(110),TO_N(62),TO_N(93),TO_N(32),TO_N(91),TO_N(45),TO_N(111),TO_N(32),TO_N(70),TO_N(73),TO_N(76),TO_N(69),TO_N(78),TO_N(65),TO_N(77),TO_N(69),TO_N(93),TO_N(32),TO_N(91),TO_N(45),TO_N(105),TO_N(110),TO_N(99),TO_N(108),TO_N(117),TO_N(100),TO_N(101),TO_N(32),TO_N(70),TO_N(73),TO_N(76),TO_N(69),TO_N(78),TO_N(65),TO_N(77),TO_N(69),TO_N(93),TO_N(32),TO_N(91),TO_N(45),TO_N(112),TO_N(114),TO_N(101),TO_N(108),TO_N(117),TO_N(100),TO_N(101),TO_N(32),TO_N(70),TO_N(73),TO_N(76),TO_N(69),TO_N(78),TO_N(65),TO_N(77),TO_N(69),TO_N(93),TO_N(32),TO_N(10),TO_N(32),TO_N(32),TO_N(32),TO_N(32),TO_N(32),TO_N(32),TO_N(32),TO_N(32),TO_N(32),TO_N(32),TO_N(32),TO_N(91),TO_N(45),TO_N(108),TO_N(105),TO_N(109),TO_N(105),TO_N(116),TO_N(32),TO_N(76),TO_N(73),TO_N(77),TO_N(73),TO_N(84),TO_N(93),TO_N(32),TO_N(91),TO_N(45),TO_N(110),TO_N(111),TO_N(45),TO_N(99),TO_N(104),TO_N(101),TO_N(99),TO_N(107),TO_N(93),TO_N(32),TO_N(91),TO_N(45),TO_N(110),TO_N(111),TO_N(100),TO_N(101),TO_N(115),TO_N(93),TO_N(32),TO_N(91),TO_N(45),TO_N(117),TO_N(110),TO_N(112),TO_N(97),TO_N(114),TO_N(115),TO_N(101),TO_N(93),TO_N(32),TO_N(91),TO_N(45),TO_N(101),TO_N(110),TO_N(116),TO_N(114),TO_N(121),TO_N(45),TO_N(112),TO_N(111),TO_N(105),TO_N(110),TO_N(116),TO_N(32),TO_N(78),TO_N(65),TO_N(77),TO_N(69),TO_N(93),TO_N(10),TO_N(9),TO_N(32),TO_N(32),TO_N(32),TO_N(70),TO_N(73),TO_N(76),TO_N(69),TO_N(78),TO_N(65),TO_N(77),TO_N(69));
lf[383]=intern("ENTRY_POINT");
lf[384]=sequence(1,TO_N(46));
lf[385]=sequence(2,TO_N(107),TO_N(75));
lf[386]=sequence(2,TO_N(109),TO_N(77));
lf[387]=sequence(2,TO_N(103),TO_N(71));}
DEFINE(___ge);
DEFINE(___le);
DEFINE(___trans);
DEFINE(___trans_27);
DEFINE(___take);
DEFINE(___drop);
DEFINE(___tlr);
DEFINE(___compress);
DEFINE(___partition);
DEFINE(___index);
DEFINE(___index_27);
DEFINE(___indexr_27);
DEFINE(___assoc);
DEFINE(___fetch);
DEFINE(___fetchall);
DEFINE(___nodups);
DEFINE(___any);
DEFINE(___all);
DEFINE(____5fstart);
DEFINE(___quit);
DEFINE(___constnode);
DEFINE(___lex_3alex);
DEFINE(___lex_3aloop);
DEFINE(___lex_3ascan);
DEFINE(___lex_3ascan_27);
DEFINE(___lex_3ascanquoted);
DEFINE(___lex_3ascanquoted_27);
DEFINE(___lex_3anext);
DEFINE(___lex_3askip);
DEFINE(___lex_3aadvance);
DEFINE(___lex_3askip_5fcomment);
DEFINE(___lex_3apush);
DEFINE(___lex_3asourceinfo);
DEFINE(___lex_3adigit);
DEFINE(___lex_3afail);
DEFINE(___parse_3anextval);
DEFINE(___parse_3ap_5fdefinition);
DEFINE(___parse_3arename_5frec);
DEFINE(___parse_3ap_5fseq);
DEFINE(___parse_3ap_5fexp0);
DEFINE(___parse_3ap_5fconditional);
DEFINE(___parse_3ap_5fbexpression);
DEFINE(___parse_3ap_5fbexp_27);
DEFINE(___parse_3ap_5ffexpression);
DEFINE(___parse_3ap_5fexpression);
DEFINE(___parse_3ap_5fvalue);
DEFINE(___parse_3ap_5fval_5fcons_27);
DEFINE(___parse_3ap_5fval_5fpcons_27);
DEFINE(___parse_3ap_5fconstant);
DEFINE(___parse_3amatch);
DEFINE(___parse_3amatchval);
DEFINE(___parse_3aexpect);
DEFINE(___parse_3aexpectval);
DEFINE(___parse_3apush);
DEFINE(___parse_3aprefix);
DEFINE(___parse_3aadvance);
DEFINE(___parse_3afail);
DEFINE(___analysis_3atreesize);
DEFINE(___analysis_3acollectrefs);
DEFINE(___analysis_3areport_5fstats);
DEFINE(___treeshake_3aloop);
DEFINE(___treeshake_3aused_5fdef);
DEFINE(___inline_3awalkrec);
DEFINE(___inline_3afind_5fin_5fdefs);
DEFINE(___rewrite_3arewrite);
DEFINE(___rewrite_3arewrite_5fdef2);
DEFINE(___rewrite_3aadd_5fdef);
DEFINE(___rewrite_3anode_5feq);
DEFINE(___rewrite_3alog);
DEFINE(___rewrite_3agen_5fdef);
DEFINE(___rewrite_3acommon_5fcomp);
DEFINE(___rewrite_3auncomp);
DEFINE(___rewrite_3acheck_5fselect);
DEFINE(___rewrite_3arewrite1);
DEFINE(___literals_3aextract1);
DEFINE(___literals_3awalk);
DEFINE(___literals_3aconstruct);
DEFINE(___backend_3at_5fexpr);
DEFINE(___backend_3at_5fpcons);
DEFINE(___backend_3atwo_5farg_5ffun);
DEFINE(___backend_3aunit_5fvalue);
DEFINE(___backend_3atail);
DEFINE(___backend_3anontail);
DEFINE(___backend_3aconc);
DEFINE(___backend_3aconcx);
DEFINE(___backend_3asave_5farg_5fif);
DEFINE(___backend_3arestore_5farg_5fif);
DEFINE(___backend_3aconst_5farg);
DEFINE(___backend_3agensym);
DEFINE(___backend_3agensyms);
DEFINE(___backend_3amangle);
DEFINE(___backend_3ahexdigit);
DEFINE(___unparse_3aunparse);
DEFINE(___unparse_3aindent);
DEFINE(___unparse_3aprec);
DEFINE(___unparse_3apr);
DEFINE(___unparse_3aprnl);
DEFINE(___unparse_3aprnl0);
DEFINE(___unparse_3aup);
DEFINE(___tmp_251_25314);
DEFINE(___unparse_3aup1_27);
DEFINE(___unparse_3aup2);
DEFINE(___unparse_3aup_5fconst);
DEFINE(___tmp_251_25320);
DEFINE(___unparse_3aup_5fseq);
DEFINE(___tmp_251_25322);
DEFINE(___unparse_3anode_5fprec);
DEFINE(___unparse_3awith_5fprec);
DEFINE(___toplevel_3aloop_27);
DEFINE(___toplevel_3aoptions_27);
DEFINE(___toplevel_3areplace_5fext);
DEFINE(___toplevel_3aflag);
DEFINE(___toplevel_3aflag2);
DEFINE(___toplevel_3aoption_5fvalue);
//---------------------------------------- ge (prelude.fp:35)
DEFINE(___ge){
ENTRY;
loop:;
tracecall("prelude.fp:35:  ge");
X t2=TO_N(-1); // ICONST
save(t2);
X t3=___cmp(x); // REF: cmp
t2=restore();
X t1=(t2==t3)||eq1(t2,t3)?T:F; // EQ
save(t1); // COND
X t5=___id(t1); // REF: id
X t4;
t1=restore();
if(t5!=F){
X t6=F; // ICONST
t4=t6;
}else{
X t7=T; // ICONST
t4=t7;}
RETURN(t4);}
//---------------------------------------- le (prelude.fp:36)
DEFINE(___le){
ENTRY;
loop:;
tracecall("prelude.fp:36:  le");
X t2=TO_N(1); // ICONST
save(t2);
X t3=___cmp(x); // REF: cmp
t2=restore();
X t1=(t2==t3)||eq1(t2,t3)?T:F; // EQ
save(t1); // COND
X t5=___id(t1); // REF: id
X t4;
t1=restore();
if(t5!=F){
X t6=F; // ICONST
t4=t6;
}else{
X t7=T; // ICONST
t4=t7;}
RETURN(t4);}
//---------------------------------------- trans (prelude.fp:56)
DEFINE(___trans){
ENTRY;
loop:;
tracecall("prelude.fp:56:  trans");
save(x); // COND
X t3=lf[381]; // CONST
save(t3);
X t4=___id(x); // REF: id
t3=restore();
X t2=(t3==t4)||eq1(t3,t4)?T:F; // EQ
X t1;
x=restore();
if(t2!=F){
X t5=___id(x); // REF: id
t1=t5;
}else{
X t6=___trans_27(x); // REF: trans'
t1=t6;}
RETURN(t1);}
//---------------------------------------- trans' (prelude.fp:57)
DEFINE(___trans_27){
ENTRY;
loop:;
tracecall("prelude.fp:57:  trans'");
save(x); // COND
X t3=lf[381]; // CONST
save(t3);
X t4=select1(TO_N(1),x); // SELECT (simple)
t3=restore();
X t2=(t3==t4)||eq1(t3,t4)?T:F; // EQ
X t1;
x=restore();
if(t2!=F){
X t5=EMPTY; // ICONST
t1=t5;
}else{
save(x);
X t6=allocate(2); // CONS
save(t6);
x=pick(2);
int t8; // ALPHA
check_S(x,"@");
int t9=S_LENGTH(x);
save(x);
X t7=allocate(t9);
save(t7);
for(t8=0;t8<t9;++t8){
X t7=S_DATA(pick(2))[t8];
X t10=select1(TO_N(1),t7); // SELECT (simple)
S_DATA(top())[t8]=t10;}
t7=restore();
restore();
t6=top();
S_DATA(t6)[0]=t7;
t7=pick(2);
int t12; // ALPHA
check_S(t7,"@");
int t13=S_LENGTH(t7);
save(t7);
X t11=allocate(t13);
save(t11);
for(t12=0;t12<t13;++t12){
X t11=S_DATA(pick(2))[t12];
X t14=___tl(t11); // REF: tl
S_DATA(top())[t12]=t14;}
t11=restore();
restore();
X t15=___trans_27(t11); // REF: trans'
t6=top();
S_DATA(t6)[1]=t15;
t6=restore();
restore();
X t16=___al(t6); // REF: al
t1=t16;}
RETURN(t1);}
//---------------------------------------- take (prelude.fp:72)
DEFINE(___take){
ENTRY;
loop:;
tracecall("prelude.fp:72:  take");
save(x);
X t1=allocate(3); // CONS
save(t1);
x=pick(2);
X t2=TO_N(1); // ICONST
t1=top();
S_DATA(t1)[0]=t2;
t2=pick(2);
X t3=select1(TO_N(1),t2); // SELECT (simple)
t1=top();
S_DATA(t1)[1]=t3;
t3=pick(2);
X t4=select1(TO_N(2),t3); // SELECT (simple)
t1=top();
S_DATA(t1)[2]=t4;
t1=restore();
restore();
X t5=___subseq(t1); // REF: subseq
RETURN(t5);}
//---------------------------------------- drop (prelude.fp:73)
DEFINE(___drop){
ENTRY;
loop:;
tracecall("prelude.fp:73:  drop");
save(x);
X t1=allocate(3); // CONS
save(t1);
x=pick(2);
X t2=select1(TO_N(1),x); // SELECT (simple)
save(t2);
X t3=allocate(2); // CONS
save(t3);
t2=pick(2);
X t4=___id(t2); // REF: id
t3=top();
S_DATA(t3)[0]=t4;
t4=pick(2);
X t5=TO_N(1); // ICONST
t3=top();
S_DATA(t3)[1]=t5;
t3=restore();
restore();
X t6=___add(t3); // REF: add
t1=top();
S_DATA(t1)[0]=t6;
t6=pick(2);
X t7=select1(TO_N(2),t6); // SELECT (simple)
X t8=___len(t7); // REF: len
t1=top();
S_DATA(t1)[1]=t8;
t8=pick(2);
X t9=select1(TO_N(2),t8); // SELECT (simple)
t1=top();
S_DATA(t1)[2]=t9;
t1=restore();
restore();
X t10=___subseq(t1); // REF: subseq
RETURN(t10);}
//---------------------------------------- tlr (prelude.fp:77)
DEFINE(___tlr){
ENTRY;
loop:;
tracecall("prelude.fp:77:  tlr");
save(x);
X t1=allocate(2); // CONS
save(t1);
x=pick(2);
X t2=___len(x); // REF: len
save(t2);
X t3=allocate(2); // CONS
save(t3);
t2=pick(2);
X t4=___id(t2); // REF: id
t3=top();
S_DATA(t3)[0]=t4;
t4=pick(2);
X t5=TO_N(1); // ICONST
t3=top();
S_DATA(t3)[1]=t5;
t3=restore();
restore();
X t6=___sub(t3); // REF: sub
t1=top();
S_DATA(t1)[0]=t6;
t6=pick(2);
X t7=___id(t6); // REF: id
t1=top();
S_DATA(t1)[1]=t7;
t1=restore();
restore();
X t8=___take(t1); // REF: take
RETURN(t8);}
//---------------------------------------- compress (prelude.fp:93)
DEFINE(___compress){
ENTRY;
loop:;
tracecall("prelude.fp:93:  compress");
save(x);
X t1=allocate(2); // CONS
save(t1);
x=pick(2);
X t2=select1(TO_N(1),x); // SELECT (simple)
int t4; // ALPHA
check_S(t2,"@");
int t5=S_LENGTH(t2);
save(t2);
X t3=allocate(t5);
save(t3);
for(t4=0;t4<t5;++t4){
X t3=S_DATA(pick(2))[t4];
save(t3); // COND
X t7=___id(t3); // REF: id
X t6;
t3=restore();
if(t7!=F){
X t8=TO_N(1); // ICONST
t6=t8;
}else{
X t9=TO_N(0); // ICONST
t6=t9;}
S_DATA(top())[t4]=t6;}
t3=restore();
restore();
t1=top();
S_DATA(t1)[0]=t3;
t3=pick(2);
X t10=select1(TO_N(2),t3); // SELECT (simple)
t1=top();
S_DATA(t1)[1]=t10;
t1=restore();
restore();
X t11=___trans(t1); // REF: trans
int t13; // ALPHA
check_S(t11,"@");
int t14=S_LENGTH(t11);
save(t11);
X t12=allocate(t14);
save(t12);
for(t13=0;t13<t14;++t13){
X t12=S_DATA(pick(2))[t13];
X t15=___make(t12); // REF: make
S_DATA(top())[t13]=t15;}
t12=restore();
restore();
X t16=___cat(t12); // REF: cat
RETURN(t16);}
//---------------------------------------- partition (prelude.fp:96)
DEFINE(___partition){
ENTRY;
loop:;
tracecall("prelude.fp:96:  partition");
save(x);
X t1=allocate(2); // CONS
save(t1);
x=pick(2);
X t2=___compress(x); // REF: compress
t1=top();
S_DATA(t1)[0]=t2;
t2=pick(2);
save(t2);
X t3=allocate(2); // CONS
save(t3);
t2=pick(2);
X t4=select1(TO_N(1),t2); // SELECT (simple)
int t6; // ALPHA
check_S(t4,"@");
int t7=S_LENGTH(t4);
save(t4);
X t5=allocate(t7);
save(t5);
for(t6=0;t6<t7;++t6){
X t5=S_DATA(pick(2))[t6];
save(t5); // COND
X t9=___id(t5); // REF: id
X t8;
t5=restore();
if(t9!=F){
X t10=F; // ICONST
t8=t10;
}else{
X t11=T; // ICONST
t8=t11;}
S_DATA(top())[t6]=t8;}
t5=restore();
restore();
t3=top();
S_DATA(t3)[0]=t5;
t5=pick(2);
X t12=select1(TO_N(2),t5); // SELECT (simple)
t3=top();
S_DATA(t3)[1]=t12;
t3=restore();
restore();
X t13=___compress(t3); // REF: compress
t1=top();
S_DATA(t1)[1]=t13;
t1=restore();
restore();
RETURN(t1);}
//---------------------------------------- index (prelude.fp:101)
DEFINE(___index){
ENTRY;
loop:;
tracecall("prelude.fp:101:  index");
save(x);
X t1=allocate(3); // CONS
save(t1);
x=pick(2);
X t2=select1(TO_N(1),x); // SELECT (simple)
t1=top();
S_DATA(t1)[0]=t2;
t2=pick(2);
X t3=select1(TO_N(2),t2); // SELECT (simple)
t1=top();
S_DATA(t1)[1]=t3;
t3=pick(2);
X t4=TO_N(1); // ICONST
t1=top();
S_DATA(t1)[2]=t4;
t1=restore();
restore();
X t5=___index_27(t1); // REF: index'
RETURN(t5);}
//---------------------------------------- index' (prelude.fp:102)
DEFINE(___index_27){
ENTRY;
loop:;
tracecall("prelude.fp:102:  index'");
save(x); // COND
save(x);
X t2=allocate(2); // CONS
save(t2);
x=pick(2);
X t3=select1(TO_N(3),x); // SELECT (simple)
t2=top();
S_DATA(t2)[0]=t3;
t3=pick(2);
X t4=select1(TO_N(2),t3); // SELECT (simple)
X t5=___len(t4); // REF: len
t2=top();
S_DATA(t2)[1]=t5;
t2=restore();
restore();
X t7=TO_N(1); // ICONST
save(t7);
X t8=___cmp(t2); // REF: cmp
t7=restore();
X t6=(t7==t8)||eq1(t7,t8)?T:F; // EQ
X t1;
x=restore();
if(t6!=F){
X t9=F; // ICONST
t1=t9;
}else{
save(x); // COND
save(x);
X t12=select1(TO_N(1),x); // SELECT (simple)
x=restore();
save(t12);
save(x);
X t14=select1(TO_N(3),x); // SELECT (simple)
x=restore();
save(t14);
X t15=select1(TO_N(2),x); // SELECT (simple)
t14=restore();
X t13=select1(t14,t15); // SELECT
t12=restore();
X t11=(t12==t13)||eq1(t12,t13)?T:F; // EQ
X t10;
x=restore();
if(t11!=F){
X t16=select1(TO_N(3),x); // SELECT (simple)
t10=t16;
}else{
save(x);
X t17=allocate(3); // CONS
save(t17);
x=pick(2);
X t18=select1(TO_N(1),x); // SELECT (simple)
t17=top();
S_DATA(t17)[0]=t18;
t18=pick(2);
X t19=select1(TO_N(2),t18); // SELECT (simple)
t17=top();
S_DATA(t17)[1]=t19;
t19=pick(2);
X t20=select1(TO_N(3),t19); // SELECT (simple)
save(t20);
X t21=allocate(2); // CONS
save(t21);
t20=pick(2);
X t22=___id(t20); // REF: id
t21=top();
S_DATA(t21)[0]=t22;
t22=pick(2);
X t23=TO_N(1); // ICONST
t21=top();
S_DATA(t21)[1]=t23;
t21=restore();
restore();
X t24=___add(t21); // REF: add
t17=top();
S_DATA(t17)[2]=t24;
t17=restore();
restore();
x=t17; // REF: index'
goto loop; // tail call: index'
X t25;
t10=t25;}
t1=t10;}
RETURN(t1);}
//---------------------------------------- indexr' (prelude.fp:106)
DEFINE(___indexr_27){
ENTRY;
loop:;
tracecall("prelude.fp:106:  indexr'");
save(x); // COND
X t3=TO_N(0); // ICONST
save(t3);
X t4=select1(TO_N(3),x); // SELECT (simple)
t3=restore();
X t2=(t3==t4)||eq1(t3,t4)?T:F; // EQ
X t1;
x=restore();
if(t2!=F){
X t5=F; // ICONST
t1=t5;
}else{
save(x); // COND
save(x);
X t8=select1(TO_N(1),x); // SELECT (simple)
x=restore();
save(t8);
save(x);
X t10=select1(TO_N(3),x); // SELECT (simple)
x=restore();
save(t10);
X t11=select1(TO_N(2),x); // SELECT (simple)
t10=restore();
X t9=select1(t10,t11); // SELECT
t8=restore();
X t7=(t8==t9)||eq1(t8,t9)?T:F; // EQ
X t6;
x=restore();
if(t7!=F){
X t12=select1(TO_N(3),x); // SELECT (simple)
t6=t12;
}else{
save(x);
X t13=allocate(3); // CONS
save(t13);
x=pick(2);
X t14=select1(TO_N(1),x); // SELECT (simple)
t13=top();
S_DATA(t13)[0]=t14;
t14=pick(2);
X t15=select1(TO_N(2),t14); // SELECT (simple)
t13=top();
S_DATA(t13)[1]=t15;
t15=pick(2);
X t16=select1(TO_N(3),t15); // SELECT (simple)
save(t16);
X t17=allocate(2); // CONS
save(t17);
t16=pick(2);
X t18=___id(t16); // REF: id
t17=top();
S_DATA(t17)[0]=t18;
t18=pick(2);
X t19=TO_N(1); // ICONST
t17=top();
S_DATA(t17)[1]=t19;
t17=restore();
restore();
X t20=___sub(t17); // REF: sub
t13=top();
S_DATA(t13)[2]=t20;
t13=restore();
restore();
x=t13; // REF: indexr'
goto loop; // tail call: indexr'
X t21;
t6=t21;}
t1=t6;}
RETURN(t1);}
//---------------------------------------- assoc (prelude.fp:116)
DEFINE(___assoc){
ENTRY;
loop:;
tracecall("prelude.fp:116:  assoc");
save(x);
X t1=allocate(3); // CONS
save(t1);
x=pick(2);
X t2=select1(TO_N(1),x); // SELECT (simple)
t1=top();
S_DATA(t1)[0]=t2;
t2=pick(2);
X t3=select1(TO_N(2),t2); // SELECT (simple)
t1=top();
S_DATA(t1)[1]=t3;
t3=pick(2);
X t4=TO_N(1); // ICONST
t1=top();
S_DATA(t1)[2]=t4;
t1=restore();
restore();
X t5=t1; // WHILE
for(;;){
save(t5);
save(t5); // OR
save(t5);
X t7=allocate(2); // CONS
save(t7);
t5=pick(2);
X t8=select1(TO_N(3),t5); // SELECT (simple)
t7=top();
S_DATA(t7)[0]=t8;
t8=pick(2);
X t9=select1(TO_N(2),t8); // SELECT (simple)
X t10=___len(t9); // REF: len
t7=top();
S_DATA(t7)[1]=t10;
t7=restore();
restore();
X t12=TO_N(1); // ICONST
save(t12);
X t13=___cmp(t7); // REF: cmp
t12=restore();
X t11=(t12==t13)||eq1(t12,t13)?T:F; // EQ
X t6=t11;
t5=restore();
if(t6==F){
save(t5);
X t15=select1(TO_N(1),t5); // SELECT (simple)
t5=restore();
save(t15);
X t17=TO_N(1); // ICONST
save(t17);
save(t5);
X t19=select1(TO_N(3),t5); // SELECT (simple)
t5=restore();
save(t19);
X t20=select1(TO_N(2),t5); // SELECT (simple)
t19=restore();
X t18=select1(t19,t20); // SELECT
t17=restore();
X t16=select1(t17,t18); // SELECT
t15=restore();
X t14=(t15==t16)||eq1(t15,t16)?T:F; // EQ
t6=t14;}
save(t6); // COND
X t22=___id(t6); // REF: id
X t21;
t6=restore();
if(t22!=F){
X t23=F; // ICONST
t21=t23;
}else{
X t24=T; // ICONST
t21=t24;}
t5=restore();
if(t21==F) break;
save(t5);
X t25=allocate(3); // CONS
save(t25);
t5=pick(2);
X t26=select1(TO_N(1),t5); // SELECT (simple)
t25=top();
S_DATA(t25)[0]=t26;
t26=pick(2);
X t27=select1(TO_N(2),t26); // SELECT (simple)
t25=top();
S_DATA(t25)[1]=t27;
t27=pick(2);
X t28=select1(TO_N(3),t27); // SELECT (simple)
save(t28);
X t29=allocate(2); // CONS
save(t29);
t28=pick(2);
X t30=___id(t28); // REF: id
t29=top();
S_DATA(t29)[0]=t30;
t30=pick(2);
X t31=TO_N(1); // ICONST
t29=top();
S_DATA(t29)[1]=t31;
t29=restore();
restore();
X t32=___add(t29); // REF: add
t25=top();
S_DATA(t25)[2]=t32;
t25=restore();
restore();
t5=t25;}
save(t5); // COND
save(t5);
X t34=allocate(2); // CONS
save(t34);
t5=pick(2);
X t35=select1(TO_N(3),t5); // SELECT (simple)
t34=top();
S_DATA(t34)[0]=t35;
t35=pick(2);
X t36=select1(TO_N(2),t35); // SELECT (simple)
X t37=___len(t36); // REF: len
t34=top();
S_DATA(t34)[1]=t37;
t34=restore();
restore();
X t38=___le(t34); // REF: le
X t33;
t5=restore();
if(t38!=F){
save(t5);
X t40=select1(TO_N(3),t5); // SELECT (simple)
t5=restore();
save(t40);
X t41=select1(TO_N(2),t5); // SELECT (simple)
t40=restore();
X t39=select1(t40,t41); // SELECT
t33=t39;
}else{
X t42=F; // ICONST
t33=t42;}
RETURN(t33);}
//---------------------------------------- fetch (prelude.fp:126)
DEFINE(___fetch){
ENTRY;
loop:;
tracecall("prelude.fp:126:  fetch");
X t1=___assoc(x); // REF: assoc
save(t1); // COND
X t3=___id(t1); // REF: id
X t2;
t1=restore();
if(t3!=F){
X t4=select1(TO_N(2),t1); // SELECT (simple)
t2=t4;
}else{
X t5=___id(t1); // REF: id
t2=t5;}
RETURN(t2);}
//---------------------------------------- fetchall (prelude.fp:130)
DEFINE(___fetchall){
ENTRY;
loop:;
tracecall("prelude.fp:130:  fetchall");
X t1=___dl(x); // REF: dl
int t3; // ALPHA
check_S(t1,"@");
int t4=S_LENGTH(t1);
save(t1);
X t2=allocate(t4);
save(t2);
for(t3=0;t3<t4;++t3){
X t2=S_DATA(pick(2))[t3];
save(t2);
X t5=allocate(2); // CONS
save(t5);
t2=pick(2);
save(t2);
X t7=select1(TO_N(1),t2); // SELECT (simple)
t2=restore();
save(t7);
X t9=TO_N(1); // ICONST
save(t9);
X t10=select1(TO_N(2),t2); // SELECT (simple)
t9=restore();
X t8=select1(t9,t10); // SELECT
t7=restore();
X t6=(t7==t8)||eq1(t7,t8)?T:F; // EQ
t5=top();
S_DATA(t5)[0]=t6;
t6=pick(2);
X t12=TO_N(2); // ICONST
save(t12);
X t13=select1(TO_N(2),t6); // SELECT (simple)
t12=restore();
X t11=select1(t12,t13); // SELECT
t5=top();
S_DATA(t5)[1]=t11;
t5=restore();
restore();
S_DATA(top())[t3]=t5;}
t2=restore();
restore();
X t14=___trans(t2); // REF: trans
save(t14); // COND
X t17=lf[381]; // CONST
save(t17);
X t18=___id(t14); // REF: id
t17=restore();
X t16=(t17==t18)||eq1(t17,t18)?T:F; // EQ
X t15;
t14=restore();
if(t16!=F){
X t19=___id(t14); // REF: id
t15=t19;
}else{
X t20=___compress(t14); // REF: compress
t15=t20;}
RETURN(t15);}
//---------------------------------------- nodups (prelude.fp:238)
DEFINE(___nodups){
ENTRY;
loop:;
tracecall("prelude.fp:238:  nodups");
save(x); // COND
X t3=lf[381]; // CONST
save(t3);
X t4=___id(x); // REF: id
t3=restore();
X t2=(t3==t4)||eq1(t3,t4)?T:F; // EQ
X t1;
x=restore();
if(t2!=F){
X t5=EMPTY; // ICONST
t1=t5;
}else{
save(x); // COND
save(x);
X t7=allocate(2); // CONS
save(t7);
x=pick(2);
X t8=select1(TO_N(1),x); // SELECT (simple)
t7=top();
S_DATA(t7)[0]=t8;
t8=pick(2);
X t9=___tl(t8); // REF: tl
t7=top();
S_DATA(t7)[1]=t9;
t7=restore();
restore();
X t10=___index(t7); // REF: index
X t6;
x=restore();
if(t10!=F){
X t11=___tl(x); // REF: tl
x=t11; // REF: nodups
goto loop; // tail call: nodups
X t12;
t6=t12;
}else{
save(x);
X t13=allocate(2); // CONS
save(t13);
x=pick(2);
X t14=select1(TO_N(1),x); // SELECT (simple)
t13=top();
S_DATA(t13)[0]=t14;
t14=pick(2);
X t15=___tl(t14); // REF: tl
X t16=___nodups(t15); // REF: nodups
t13=top();
S_DATA(t13)[1]=t16;
t13=restore();
restore();
X t17=___al(t13); // REF: al
t6=t17;}
t1=t6;}
RETURN(t1);}
//---------------------------------------- any (prelude.fp:262)
DEFINE(___any){
ENTRY;
loop:;
tracecall("prelude.fp:262:  any");
save(x); // COND
X t3=lf[381]; // CONST
save(t3);
X t4=___id(x); // REF: id
t3=restore();
X t2=(t3==t4)||eq1(t3,t4)?T:F; // EQ
X t1;
x=restore();
if(t2!=F){
X t5=F; // ICONST
t1=t5;
}else{
int t8; // INSERTR
check_S(x,"/");
int t9=S_LENGTH(x);
X t6=t9==0?fail("no unit value"):S_DATA(x)[t9-1];
save(x);
for(t8=t9-2;t8>=0;--t8){
X t7=sequence(2,S_DATA(pick(1))[t8],t6);
save(t7); // COND
X t11=select1(TO_N(1),t7); // SELECT (simple)
X t10;
t7=restore();
if(t11!=F){
X t12=T; // ICONST
t10=t12;
}else{
save(t7); // COND
X t14=select1(TO_N(2),t7); // SELECT (simple)
X t13;
t7=restore();
if(t14!=F){
X t15=T; // ICONST
t13=t15;
}else{
X t16=F; // ICONST
t13=t16;}
t10=t13;}
t6=t10;}
restore();
t1=t6;}
RETURN(t1);}
//---------------------------------------- all (prelude.fp:263)
DEFINE(___all){
ENTRY;
loop:;
tracecall("prelude.fp:263:  all");
save(x); // COND
X t3=lf[381]; // CONST
save(t3);
X t4=___id(x); // REF: id
t3=restore();
X t2=(t3==t4)||eq1(t3,t4)?T:F; // EQ
X t1;
x=restore();
if(t2!=F){
X t5=T; // ICONST
t1=t5;
}else{
int t8; // INSERTR
check_S(x,"/");
int t9=S_LENGTH(x);
X t6=t9==0?fail("no unit value"):S_DATA(x)[t9-1];
save(x);
for(t8=t9-2;t8>=0;--t8){
X t7=sequence(2,S_DATA(pick(1))[t8],t6);
save(t7); // COND
X t11=select1(TO_N(1),t7); // SELECT (simple)
X t10;
t7=restore();
if(t11!=F){
save(t7); // COND
X t13=select1(TO_N(2),t7); // SELECT (simple)
X t12;
t7=restore();
if(t13!=F){
X t14=T; // ICONST
t12=t14;
}else{
X t15=F; // ICONST
t12=t15;}
t10=t12;
}else{
X t16=F; // ICONST
t10=t16;}
t6=t10;}
restore();
t1=t6;}
RETURN(t1);}
//---------------------------------------- _start (prelude.fp:283)
DEFINE(____5fstart){
ENTRY;
loop:;
tracecall("prelude.fp:283:  _start");
save(x);
X t1=allocate(2); // CONS
save(t1);
x=pick(2);
int t3; // ALPHA
check_S(x,"@");
int t4=S_LENGTH(x);
save(x);
X t2=allocate(t4);
save(t2);
for(t3=0;t3<t4;++t3){
X t2=S_DATA(pick(2))[t3];
X t5=___tos(t2); // REF: tos
S_DATA(top())[t3]=t5;}
t2=restore();
restore();
t1=top();
S_DATA(t1)[0]=t2;
t2=pick(2);
X t6=lf[30]; // CONST
t1=top();
S_DATA(t1)[1]=t6;
t1=restore();
restore();
X t7=___toplevel_3aoptions_27(t1); // REF: toplevel:options'
save(t7); // COND
X t9=___id(t7); // REF: id
X t8;
t7=restore();
if(t9!=F){
save(t7);
X t10=allocate(3); // CONS
save(t10);
t7=pick(2);
X t11=select1(TO_N(2),t7); // SELECT (simple)
t10=top();
S_DATA(t10)[0]=t11;
t11=pick(2);
X t12=select1(TO_N(4),t11); // SELECT (simple)
t10=top();
S_DATA(t10)[1]=t12;
t12=pick(2);
save(t12);
X t13=allocate(2); // CONS
save(t13);
t12=pick(2);
X t14=select1(TO_N(1),t12); // SELECT (simple)
t13=top();
S_DATA(t13)[0]=t14;
t14=pick(2);
X t15=select1(TO_N(3),t14); // SELECT (simple)
t13=top();
S_DATA(t13)[1]=t15;
t13=restore();
restore();
save(t13);
X t16=allocate(2); // CONS
save(t16);
t13=pick(2);
X t17=select1(TO_N(2),t13); // SELECT (simple)
t16=top();
S_DATA(t16)[0]=t17;
t17=pick(2);
save(t17);
X t18=allocate(2); // CONS
save(t18);
t17=pick(2);
X t19=select1(TO_N(2),t17); // SELECT (simple)
save(t19);
X t20=allocate(2); // CONS
save(t20);
t19=pick(2);
X t21=lf[371]; // CONST
t20=top();
S_DATA(t20)[0]=t21;
t21=pick(2);
X t22=___id(t21); // REF: id
t20=top();
S_DATA(t20)[1]=t22;
t20=restore();
restore();
save(t20); // OR
X t24=___fetch(t20); // REF: fetch
X t23=t24;
t20=restore();
if(t23==F){
X t25=lf[29]; // CONST
t23=t25;}
X t26=___lex_3alex(t23); // REF: lex:lex
t18=top();
S_DATA(t18)[0]=t26;
t26=pick(2);
X t27=select1(TO_N(1),t26); // SELECT (simple)
X t28=___lex_3alex(t27); // REF: lex:lex
t18=top();
S_DATA(t18)[1]=t28;
t18=restore();
restore();
X t29=___cat(t18); // REF: cat
t16=top();
S_DATA(t16)[1]=t29;
t16=restore();
restore();
save(t16);
X t30=allocate(3); // CONS
save(t30);
t16=pick(2);
X t31=select1(TO_N(1),t16); // SELECT (simple)
t30=top();
S_DATA(t30)[0]=t31;
t31=pick(2);
X t32=select1(TO_N(2),t31); // SELECT (simple)
t30=top();
S_DATA(t30)[1]=t32;
t32=pick(2);
save(t32); // COND
X t34=select1(TO_N(1),t32); // SELECT (simple)
save(t34);
X t35=allocate(2); // CONS
save(t35);
t34=pick(2);
X t36=lf[362]; // CONST
t35=top();
S_DATA(t35)[0]=t36;
t36=pick(2);
X t37=___id(t36); // REF: id
t35=top();
S_DATA(t35)[1]=t37;
t35=restore();
restore();
X t38=___fetch(t35); // REF: fetch
X t33;
t32=restore();
if(t38!=F){
save(t32); // SEQ
X t39=lf[28]; // CONST
save(t39); // SEQ
X t40=____5femit(t39); // REF: _emit
t39=restore();
X t41=TO_N(10); // ICONST
X t42=____5femit(t41); // REF: _emit
t32=restore();
X t43=select1(TO_N(2),t32); // SELECT (simple)
X t44=____5fshow(t43); // REF: _show
t33=t44;
}else{
X t45=___id(t32); // REF: id
t33=t45;}
t30=top();
S_DATA(t30)[2]=t33;
t30=restore();
restore();
save(t30);
X t46=allocate(2); // CONS
save(t46);
t30=pick(2);
X t47=select1(TO_N(1),t30); // SELECT (simple)
t46=top();
S_DATA(t46)[0]=t47;
t47=pick(2);
X t48=select1(TO_N(2),t47); // SELECT (simple)
save(t48);
X t49=allocate(2); // CONS
save(t49);
t48=pick(2);
X t50=___id(t48); // REF: id
t49=top();
S_DATA(t49)[0]=t50;
t50=pick(2);
X t51=EMPTY; // ICONST
t49=top();
S_DATA(t49)[1]=t51;
t49=restore();
restore();
save(t49); // COND
X t54=lf[381]; // CONST
save(t54);
X t55=select1(TO_N(1),t49); // SELECT (simple)
t54=restore();
X t53=(t54==t55)||eq1(t54,t55)?T:F; // EQ
X t52;
t49=restore();
if(t53!=F){
X t56=select1(TO_N(2),t49); // SELECT (simple)
t52=t56;
}else{
save(t49); // COND
save(t49);
X t58=allocate(2); // CONS
save(t58);
t49=pick(2);
X t59=___id(t49); // REF: id
t58=top();
S_DATA(t58)[0]=t59;
t59=pick(2);
X t60=lf[92]; // CONST
t58=top();
S_DATA(t58)[1]=t60;
t58=restore();
restore();
X t61=___parse_3amatch(t58); // REF: parse:match
X t57;
t49=restore();
if(t61!=F){
save(t49); // COND
X t63=___parse_3anextval(t49); // REF: parse:nextval
X t64=___tos(t63); // REF: tos
save(t64); // COND
save(t64);
X t66=allocate(2); // CONS
save(t66);
t64=pick(2);
X t67=___len(t64); // REF: len
t66=top();
S_DATA(t66)[0]=t67;
t67=pick(2);
X t68=TO_N(2); // ICONST
t66=top();
S_DATA(t66)[1]=t68;
t66=restore();
restore();
X t70=TO_N(1); // ICONST
save(t70);
X t71=___cmp(t66); // REF: cmp
t70=restore();
X t69=(t70==t71)||eq1(t70,t71)?T:F; // EQ
X t65;
t64=restore();
if(t69!=F){
save(t64);
X t72=allocate(2); // CONS
save(t72);
t64=pick(2);
save(t64);
X t73=allocate(2); // CONS
save(t73);
t64=pick(2);
X t74=TO_N(46); // ICONST
t73=top();
S_DATA(t73)[0]=t74;
t74=pick(2);
X t75=___id(t74); // REF: id
t73=top();
S_DATA(t73)[1]=t75;
t73=restore();
restore();
save(t73); // OR
X t77=___index(t73); // REF: index
X t76=t77;
t73=restore();
if(t76==F){
X t78=TO_N(0); // ICONST
t76=t78;}
t72=top();
S_DATA(t72)[0]=t76;
t76=pick(2);
X t79=___id(t76); // REF: id
t72=top();
S_DATA(t72)[1]=t79;
t72=restore();
restore();
X t80=___drop(t72); // REF: drop
save(t80);
X t81=allocate(2); // CONS
save(t81);
t80=pick(2);
X t82=___id(t80); // REF: id
t81=top();
S_DATA(t81)[0]=t82;
t82=pick(2);
X t83=lf[27]; // CONST
t81=top();
S_DATA(t81)[1]=t83;
t81=restore();
restore();
X t84=___index(t81); // REF: index
t65=t84;
}else{
X t85=F; // ICONST
t65=t85;}
X t62;
t49=restore();
if(t65!=F){
save(t49);
X t86=allocate(2); // CONS
save(t86);
t49=pick(2);
X t87=___parse_3anextval(t49); // REF: parse:nextval
X t88=___toa(t87); // REF: toa
t86=top();
S_DATA(t86)[0]=t88;
t88=pick(2);
X t89=___parse_3aadvance(t88); // REF: parse:advance
t86=top();
S_DATA(t86)[1]=t89;
t86=restore();
restore();
X t90=___parse_3apush(t86); // REF: parse:push
t62=t90;
}else{
save(t49);
X t91=allocate(2); // CONS
save(t91);
t49=pick(2);
save(t49);
X t92=allocate(2); // CONS
save(t92);
t49=pick(2);
X t93=___parse_3anextval(t49); // REF: parse:nextval
X t94=___lex_3alex(t93); // REF: lex:lex
t92=top();
S_DATA(t92)[0]=t94;
t94=pick(2);
X t95=select1(TO_N(1),t94); // SELECT (simple)
X t96=___tl(t95); // REF: tl
t92=top();
S_DATA(t92)[1]=t96;
t92=restore();
restore();
X t97=___cat(t92); // REF: cat
t91=top();
S_DATA(t91)[0]=t97;
t97=pick(2);
X t98=___tl(t97); // REF: tl
t91=top();
S_DATA(t91)[1]=t98;
t91=restore();
restore();
X t99=___al(t91); // REF: al
t62=t99;}
t57=t62;
}else{
X t100=___parse_3ap_5fdefinition(t49); // REF: parse:p_definition
X t101=___parse_3apush(t100); // REF: parse:push
t57=t101;}
X t103=TO_N(2); // ICONST
save(t103);
X t104=t57; // WHILE
for(;;){
save(t104);
save(t104); // COND
X t107=lf[381]; // CONST
save(t107);
X t108=select1(TO_N(1),t104); // SELECT (simple)
t107=restore();
X t106=(t107==t108)||eq1(t107,t108)?T:F; // EQ
X t105;
t104=restore();
if(t106!=F){
X t109=F; // ICONST
t105=t109;
}else{
X t110=T; // ICONST
t105=t110;}
t104=restore();
if(t105==F) break;
save(t104); // COND
save(t104);
X t112=allocate(2); // CONS
save(t112);
t104=pick(2);
X t113=___id(t104); // REF: id
t112=top();
S_DATA(t112)[0]=t113;
t113=pick(2);
X t114=lf[92]; // CONST
t112=top();
S_DATA(t112)[1]=t114;
t112=restore();
restore();
X t115=___parse_3amatch(t112); // REF: parse:match
X t111;
t104=restore();
if(t115!=F){
save(t104); // COND
X t117=___parse_3anextval(t104); // REF: parse:nextval
X t118=___tos(t117); // REF: tos
save(t118); // COND
save(t118);
X t120=allocate(2); // CONS
save(t120);
t118=pick(2);
X t121=___len(t118); // REF: len
t120=top();
S_DATA(t120)[0]=t121;
t121=pick(2);
X t122=TO_N(2); // ICONST
t120=top();
S_DATA(t120)[1]=t122;
t120=restore();
restore();
X t124=TO_N(1); // ICONST
save(t124);
X t125=___cmp(t120); // REF: cmp
t124=restore();
X t123=(t124==t125)||eq1(t124,t125)?T:F; // EQ
X t119;
t118=restore();
if(t123!=F){
save(t118);
X t126=allocate(2); // CONS
save(t126);
t118=pick(2);
save(t118);
X t127=allocate(2); // CONS
save(t127);
t118=pick(2);
X t128=TO_N(46); // ICONST
t127=top();
S_DATA(t127)[0]=t128;
t128=pick(2);
X t129=___id(t128); // REF: id
t127=top();
S_DATA(t127)[1]=t129;
t127=restore();
restore();
save(t127); // OR
X t131=___index(t127); // REF: index
X t130=t131;
t127=restore();
if(t130==F){
X t132=TO_N(0); // ICONST
t130=t132;}
t126=top();
S_DATA(t126)[0]=t130;
t130=pick(2);
X t133=___id(t130); // REF: id
t126=top();
S_DATA(t126)[1]=t133;
t126=restore();
restore();
X t134=___drop(t126); // REF: drop
save(t134);
X t135=allocate(2); // CONS
save(t135);
t134=pick(2);
X t136=___id(t134); // REF: id
t135=top();
S_DATA(t135)[0]=t136;
t136=pick(2);
X t137=lf[27]; // CONST
t135=top();
S_DATA(t135)[1]=t137;
t135=restore();
restore();
X t138=___index(t135); // REF: index
t119=t138;
}else{
X t139=F; // ICONST
t119=t139;}
X t116;
t104=restore();
if(t119!=F){
save(t104);
X t140=allocate(2); // CONS
save(t140);
t104=pick(2);
X t141=___parse_3anextval(t104); // REF: parse:nextval
X t142=___toa(t141); // REF: toa
t140=top();
S_DATA(t140)[0]=t142;
t142=pick(2);
X t143=___parse_3aadvance(t142); // REF: parse:advance
t140=top();
S_DATA(t140)[1]=t143;
t140=restore();
restore();
X t144=___parse_3apush(t140); // REF: parse:push
t116=t144;
}else{
save(t104);
X t145=allocate(2); // CONS
save(t145);
t104=pick(2);
save(t104);
X t146=allocate(2); // CONS
save(t146);
t104=pick(2);
X t147=___parse_3anextval(t104); // REF: parse:nextval
X t148=___lex_3alex(t147); // REF: lex:lex
t146=top();
S_DATA(t146)[0]=t148;
t148=pick(2);
X t149=select1(TO_N(1),t148); // SELECT (simple)
X t150=___tl(t149); // REF: tl
t146=top();
S_DATA(t146)[1]=t150;
t146=restore();
restore();
X t151=___cat(t146); // REF: cat
t145=top();
S_DATA(t145)[0]=t151;
t151=pick(2);
X t152=___tl(t151); // REF: tl
t145=top();
S_DATA(t145)[1]=t152;
t145=restore();
restore();
X t153=___al(t145); // REF: al
t116=t153;}
t111=t116;
}else{
X t154=___parse_3ap_5fdefinition(t104); // REF: parse:p_definition
X t155=___parse_3apush(t154); // REF: parse:push
t111=t155;}
t104=t111;}
t103=restore();
X t102=select1(t103,t104); // SELECT
t52=t102;}
save(t52);
X t156=allocate(2); // CONS
save(t156);
t52=pick(2);
int t158; // ALPHA
check_S(t52,"@");
int t159=S_LENGTH(t52);
save(t52);
X t157=allocate(t159);
save(t157);
for(t158=0;t158<t159;++t158){
X t157=S_DATA(pick(2))[t158];
X t160=___atom(t157); // REF: atom
S_DATA(top())[t158]=t160;}
t157=restore();
restore();
t156=top();
S_DATA(t156)[0]=t157;
t157=pick(2);
X t161=___id(t157); // REF: id
t156=top();
S_DATA(t156)[1]=t161;
t156=restore();
restore();
X t162=___partition(t156); // REF: partition
t46=top();
S_DATA(t46)[1]=t162;
t46=restore();
restore();
save(t46);
X t163=allocate(2); // CONS
save(t163);
t46=pick(2);
X t165=TO_N(2); // ICONST
save(t165);
X t166=select1(TO_N(2),t46); // SELECT (simple)
t165=restore();
X t164=select1(t165,t166); // SELECT
t163=top();
S_DATA(t163)[0]=t164;
t164=pick(2);
save(t164);
X t167=allocate(2); // CONS
save(t167);
t164=pick(2);
X t168=select1(TO_N(1),t164); // SELECT (simple)
t167=top();
S_DATA(t167)[0]=t168;
t168=pick(2);
X t170=TO_N(1); // ICONST
save(t170);
X t171=select1(TO_N(2),t168); // SELECT (simple)
t170=restore();
X t169=select1(t170,t171); // SELECT
int t173; // ALPHA
check_S(t169,"@");
int t174=S_LENGTH(t169);
save(t169);
X t172=allocate(t174);
save(t172);
for(t173=0;t173<t174;++t173){
X t172=S_DATA(pick(2))[t173];
save(t172);
X t175=allocate(2); // CONS
save(t175);
t172=pick(2);
X t176=lf[373]; // CONST
t175=top();
S_DATA(t175)[0]=t176;
t176=pick(2);
X t177=___id(t176); // REF: id
t175=top();
S_DATA(t175)[1]=t177;
t175=restore();
restore();
S_DATA(top())[t173]=t175;}
t172=restore();
restore();
t167=top();
S_DATA(t167)[1]=t172;
t167=restore();
restore();
X t178=___cat(t167); // REF: cat
t163=top();
S_DATA(t163)[1]=t178;
t163=restore();
restore();
t10=top();
S_DATA(t10)[2]=t163;
t10=restore();
restore();
save(t10);
X t179=allocate(4); // CONS
save(t179);
t10=pick(2);
X t180=select1(TO_N(1),t10); // SELECT (simple)
t179=top();
S_DATA(t179)[0]=t180;
t180=pick(2);
X t181=select1(TO_N(2),t180); // SELECT (simple)
t179=top();
S_DATA(t179)[1]=t181;
t181=pick(2);
X t182=select1(TO_N(3),t181); // SELECT (simple)
t179=top();
S_DATA(t179)[2]=t182;
t182=pick(2);
save(t182); // COND
X t185=TO_N(2); // ICONST
save(t185);
X t186=select1(TO_N(3),t182); // SELECT (simple)
t185=restore();
X t184=select1(t185,t186); // SELECT
save(t184);
X t187=allocate(2); // CONS
save(t187);
t184=pick(2);
X t188=lf[362]; // CONST
t187=top();
S_DATA(t187)[0]=t188;
t188=pick(2);
X t189=___id(t188); // REF: id
t187=top();
S_DATA(t187)[1]=t189;
t187=restore();
restore();
X t190=___fetch(t187); // REF: fetch
X t183;
t182=restore();
if(t190!=F){
save(t182); // SEQ
X t191=lf[341]; // CONST
save(t191); // SEQ
X t192=____5femit(t191); // REF: _emit
t191=restore();
X t193=TO_N(10); // ICONST
X t194=____5femit(t193); // REF: _emit
t182=restore();
X t196=TO_N(1); // ICONST
save(t196);
X t197=select1(TO_N(3),t182); // SELECT (simple)
t196=restore();
X t195=select1(t196,t197); // SELECT
X t198=____5fshow(t195); // REF: _show
t183=t198;
}else{
X t199=___id(t182); // REF: id
t183=t199;}
t179=top();
S_DATA(t179)[3]=t183;
t179=restore();
restore();
save(t179);
X t200=allocate(4); // CONS
save(t200);
t179=pick(2);
X t202=TO_N(1); // ICONST
save(t202);
X t203=select1(TO_N(3),t179); // SELECT (simple)
t202=restore();
X t201=select1(t202,t203); // SELECT
t200=top();
S_DATA(t200)[0]=t201;
t201=pick(2);
X t204=select1(TO_N(1),t201); // SELECT (simple)
t200=top();
S_DATA(t200)[1]=t204;
t204=pick(2);
X t206=TO_N(2); // ICONST
save(t206);
X t207=select1(TO_N(3),t204); // SELECT (simple)
t206=restore();
X t205=select1(t206,t207); // SELECT
t200=top();
S_DATA(t200)[2]=t205;
t205=pick(2);
X t208=select1(TO_N(2),t205); // SELECT (simple)
t200=top();
S_DATA(t200)[3]=t208;
t200=restore();
restore();
save(t200);
X t209=allocate(2); // CONS
save(t209);
t200=pick(2);
X t210=___tl(t200); // REF: tl
t209=top();
S_DATA(t209)[0]=t210;
t210=pick(2);
save(t210);
X t211=allocate(2); // CONS
save(t211);
t210=pick(2);
X t212=select1(TO_N(1),t210); // SELECT (simple)
t211=top();
S_DATA(t211)[0]=t212;
t212=pick(2);
X t213=select1(TO_N(3),t212); // SELECT (simple)
t211=top();
S_DATA(t211)[1]=t213;
t211=restore();
restore();
save(t211);
X t214=allocate(3); // CONS
save(t214);
t211=pick(2);
X t215=TO_N(1); // ICONST
t214=top();
S_DATA(t214)[0]=t215;
t215=pick(2);
X t216=select1(TO_N(1),t215); // SELECT (simple)
t214=top();
S_DATA(t214)[1]=t216;
t216=pick(2);
X t217=select1(TO_N(2),t216); // SELECT (simple)
t214=top();
S_DATA(t214)[2]=t217;
t214=restore();
restore();
X t218=___toplevel_3aloop_27(t214); // REF: toplevel:loop'
t209=top();
S_DATA(t209)[1]=t218;
t209=restore();
restore();
save(t209); // SEQ
save(t209); // COND
save(t209);
X t220=allocate(2); // CONS
save(t220);
t209=pick(2);
X t221=lf[375]; // CONST
t220=top();
S_DATA(t220)[0]=t221;
t221=pick(2);
X t223=TO_N(2); // ICONST
save(t223);
X t224=select1(TO_N(1),t221); // SELECT (simple)
t223=restore();
X t222=select1(t223,t224); // SELECT
t220=top();
S_DATA(t220)[1]=t222;
t220=restore();
restore();
X t225=___fetch(t220); // REF: fetch
X t219;
t209=restore();
if(t225!=F){
save(t209); // SEQ
X t226=select1(TO_N(2),t209); // SELECT (simple)
X t227=____5fshow(t226); // REF: _show
t209=restore();
X t228=lf[358]; // CONST
X t229=____5fthrow(t228); // REF: _throw
t219=t229;
}else{
save(t209); // COND
save(t209);
X t231=allocate(2); // CONS
save(t231);
t209=pick(2);
X t232=lf[377]; // CONST
t231=top();
S_DATA(t231)[0]=t232;
t232=pick(2);
X t234=TO_N(2); // ICONST
save(t234);
X t235=select1(TO_N(1),t232); // SELECT (simple)
t234=restore();
X t233=select1(t234,t235); // SELECT
t231=top();
S_DATA(t231)[1]=t233;
t231=restore();
restore();
X t236=___fetch(t231); // REF: fetch
X t230;
t209=restore();
if(t236!=F){
save(t209); // SEQ
X t237=select1(TO_N(2),t209); // SELECT (simple)
X t238=___unparse_3aunparse(t237); // REF: unparse:unparse
t209=restore();
X t239=lf[358]; // CONST
X t240=____5fthrow(t239); // REF: _throw
t230=t240;
}else{
save(t209);
X t241=allocate(3); // CONS
save(t241);
t209=pick(2);
X t242=select1(TO_N(1),t209); // SELECT (simple)
t241=top();
S_DATA(t241)[0]=t242;
t242=pick(2);
X t243=select1(TO_N(2),t242); // SELECT (simple)
t241=top();
S_DATA(t241)[1]=t243;
t243=pick(2);
X t244=select1(TO_N(2),t243); // SELECT (simple)
int t246; // ALPHA
check_S(t244,"@");
int t247=S_LENGTH(t244);
save(t244);
X t245=allocate(t247);
save(t245);
for(t246=0;t246<t247;++t246){
X t245=S_DATA(pick(2))[t246];
X t248=select1(TO_N(2),t245); // SELECT (simple)
X t249=___literals_3aextract1(t248); // REF: literals:extract1
S_DATA(top())[t246]=t249;}
t245=restore();
restore();
X t250=___cat(t245); // REF: cat
X t251=___nodups(t250); // REF: nodups
t241=top();
S_DATA(t241)[2]=t251;
t241=restore();
restore();
save(t241);
X t252=allocate(4); // CONS
save(t252);
t241=pick(2);
X t253=select1(TO_N(1),t241); // SELECT (simple)
t252=top();
S_DATA(t252)[0]=t253;
t253=pick(2);
X t254=select1(TO_N(2),t253); // SELECT (simple)
t252=top();
S_DATA(t252)[1]=t254;
t254=pick(2);
X t255=select1(TO_N(3),t254); // SELECT (simple)
t252=top();
S_DATA(t252)[2]=t255;
t255=pick(2);
save(t255); // COND
X t258=TO_N(2); // ICONST
save(t258);
X t259=select1(TO_N(1),t255); // SELECT (simple)
t258=restore();
X t257=select1(t258,t259); // SELECT
save(t257);
X t260=allocate(2); // CONS
save(t260);
t257=pick(2);
X t261=lf[362]; // CONST
t260=top();
S_DATA(t260)[0]=t261;
t261=pick(2);
X t262=___id(t261); // REF: id
t260=top();
S_DATA(t260)[1]=t262;
t260=restore();
restore();
X t263=___fetch(t260); // REF: fetch
X t256;
t255=restore();
if(t263!=F){
save(t255); // SEQ
X t264=lf[26]; // CONST
save(t264); // SEQ
X t265=____5femit(t264); // REF: _emit
t264=restore();
X t266=TO_N(10); // ICONST
X t267=____5femit(t266); // REF: _emit
t255=restore();
save(t255);
X t268=allocate(2); // CONS
save(t268);
t255=pick(2);
X t269=select1(TO_N(3),t255); // SELECT (simple)
X t270=___len(t269); // REF: len
X t271=___iota(t270); // REF: iota
int t273; // ALPHA
check_S(t271,"@");
int t274=S_LENGTH(t271);
save(t271);
X t272=allocate(t274);
save(t272);
for(t273=0;t273<t274;++t273){
X t272=S_DATA(pick(2))[t273];
save(t272);
X t276=___id(t272); // REF: id
t272=restore();
X t277=TO_N(1); // ICONST
X t275=___sub_5fop2(t276,t277); // OP2: sub
S_DATA(top())[t273]=t275;}
t272=restore();
restore();
t268=top();
S_DATA(t268)[0]=t272;
t272=pick(2);
X t278=select1(TO_N(3),t272); // SELECT (simple)
t268=top();
S_DATA(t268)[1]=t278;
t268=restore();
restore();
X t279=___trans(t268); // REF: trans
int t281; // ALPHA
check_S(t279,"@");
int t282=S_LENGTH(t279);
save(t279);
X t280=allocate(t282);
save(t280);
for(t281=0;t281<t282;++t281){
X t280=S_DATA(pick(2))[t281];
X t283=____5fshow(t280); // REF: _show
S_DATA(top())[t281]=t283;}
t280=restore();
restore();
t256=t280;
}else{
X t284=___id(t255); // REF: id
t256=t284;}
t252=top();
S_DATA(t252)[3]=t256;
t252=restore();
restore();
save(t252);
X t285=allocate(3); // CONS
save(t285);
t252=pick(2);
X t286=select1(TO_N(1),t252); // SELECT (simple)
t285=top();
S_DATA(t285)[0]=t286;
t286=pick(2);
save(t286);
X t287=allocate(2); // CONS
save(t287);
t286=pick(2);
X t288=select1(TO_N(2),t286); // SELECT (simple)
t287=top();
S_DATA(t287)[0]=t288;
t288=pick(2);
X t289=select1(TO_N(3),t288); // SELECT (simple)
t287=top();
S_DATA(t287)[1]=t289;
t287=restore();
restore();
save(t287); // COND
X t292=lf[381]; // CONST
save(t292);
X t293=select1(TO_N(2),t287); // SELECT (simple)
t292=restore();
X t291=(t292==t293)||eq1(t292,t293)?T:F; // EQ
X t290;
t287=restore();
if(t291!=F){
X t294=select1(TO_N(1),t287); // SELECT (simple)
t290=t294;
}else{
save(t287);
X t295=allocate(2); // CONS
save(t295);
t287=pick(2);
X t296=select1(TO_N(1),t287); // SELECT (simple)
t295=top();
S_DATA(t295)[0]=t296;
t296=pick(2);
save(t296);
X t297=allocate(2); // CONS
save(t297);
t296=pick(2);
X t298=select1(TO_N(1),t296); // SELECT (simple)
int t300; // ALPHA
check_S(t298,"@");
int t301=S_LENGTH(t298);
save(t298);
X t299=allocate(t301);
save(t299);
for(t300=0;t300<t301;++t300){
X t299=S_DATA(pick(2))[t300];
X t302=select1(TO_N(2),t299); // SELECT (simple)
S_DATA(top())[t300]=t302;}
t299=restore();
restore();
t297=top();
S_DATA(t297)[0]=t299;
t299=pick(2);
X t303=select1(TO_N(2),t299); // SELECT (simple)
t297=top();
S_DATA(t297)[1]=t303;
t297=restore();
restore();
X t304=___dr(t297); // REF: dr
int t306; // ALPHA
check_S(t304,"@");
int t307=S_LENGTH(t304);
save(t304);
X t305=allocate(t307);
save(t305);
for(t306=0;t306<t307;++t306){
X t305=S_DATA(pick(2))[t306];
X t308=___literals_3awalk(t305); // REF: literals:walk
S_DATA(top())[t306]=t308;}
t305=restore();
restore();
t295=top();
S_DATA(t295)[1]=t305;
t295=restore();
restore();
X t309=___trans(t295); // REF: trans
int t311; // ALPHA
check_S(t309,"@");
int t312=S_LENGTH(t309);
save(t309);
X t310=allocate(t312);
save(t310);
for(t311=0;t311<t312;++t311){
X t310=S_DATA(pick(2))[t311];
save(t310);
X t313=allocate(3); // CONS
save(t313);
t310=pick(2);
X t314=TO_N(2); // ICONST
t313=top();
S_DATA(t313)[0]=t314;
t314=pick(2);
X t315=select1(TO_N(2),t314); // SELECT (simple)
t313=top();
S_DATA(t313)[1]=t315;
t315=pick(2);
X t316=select1(TO_N(1),t315); // SELECT (simple)
t313=top();
S_DATA(t313)[2]=t316;
t313=restore();
restore();
save(t313);
X t317=allocate(3); // CONS
save(t317);
t313=pick(2);
save(t313);
X t318=allocate(2); // CONS
save(t318);
t313=pick(2);
X t319=select1(TO_N(1),t313); // SELECT (simple)
save(t319);
X t320=allocate(2); // CONS
save(t320);
t319=pick(2);
X t321=___id(t319); // REF: id
t320=top();
S_DATA(t320)[0]=t321;
t321=pick(2);
X t322=TO_N(1); // ICONST
t320=top();
S_DATA(t320)[1]=t322;
t320=restore();
restore();
X t323=___sub(t320); // REF: sub
t318=top();
S_DATA(t318)[0]=t323;
t323=pick(2);
X t324=select1(TO_N(3),t323); // SELECT (simple)
t318=top();
S_DATA(t318)[1]=t324;
t318=restore();
restore();
X t325=___take(t318); // REF: take
t317=top();
S_DATA(t317)[0]=t325;
t325=pick(2);
save(t325);
X t326=allocate(1); // CONS
save(t326);
t325=pick(2);
X t327=select1(TO_N(2),t325); // SELECT (simple)
t326=top();
S_DATA(t326)[0]=t327;
t326=restore();
restore();
t317=top();
S_DATA(t317)[1]=t326;
t326=pick(2);
save(t326);
X t328=allocate(2); // CONS
save(t328);
t326=pick(2);
X t329=select1(TO_N(1),t326); // SELECT (simple)
t328=top();
S_DATA(t328)[0]=t329;
t329=pick(2);
X t330=select1(TO_N(3),t329); // SELECT (simple)
t328=top();
S_DATA(t328)[1]=t330;
t328=restore();
restore();
X t331=___drop(t328); // REF: drop
t317=top();
S_DATA(t317)[2]=t331;
t317=restore();
restore();
X t332=___cat(t317); // REF: cat
S_DATA(top())[t311]=t332;}
t310=restore();
restore();
t290=t310;}
t285=top();
S_DATA(t285)[1]=t290;
t290=pick(2);
X t333=select1(TO_N(3),t290); // SELECT (simple)
t285=top();
S_DATA(t285)[2]=t333;
t285=restore();
restore();
save(t285);
X t334=allocate(2); // CONS
save(t334);
t285=pick(2);
X t335=select1(TO_N(1),t285); // SELECT (simple)
t334=top();
S_DATA(t334)[0]=t335;
t335=pick(2);
save(t335);
X t336=allocate(3); // CONS
save(t336);
t335=pick(2);
X t337=select1(TO_N(2),t335); // SELECT (simple)
t336=top();
S_DATA(t336)[0]=t337;
t337=pick(2);
X t339=TO_N(2); // ICONST
save(t339);
X t340=select1(TO_N(1),t337); // SELECT (simple)
t339=restore();
X t338=select1(t339,t340); // SELECT
t336=top();
S_DATA(t336)[1]=t338;
t338=pick(2);
X t341=select1(TO_N(3),t338); // SELECT (simple)
t336=top();
S_DATA(t336)[2]=t341;
t336=restore();
restore();
save(t336); // SEQ
save(t336); // COND
save(t336);
X t343=allocate(2); // CONS
save(t343);
t336=pick(2);
X t344=lf[361]; // CONST
t343=top();
S_DATA(t343)[0]=t344;
t344=pick(2);
X t345=select1(TO_N(2),t344); // SELECT (simple)
t343=top();
S_DATA(t343)[1]=t345;
t343=restore();
restore();
X t346=___fetch(t343); // REF: fetch
X t342;
t336=restore();
if(t346!=F){
X t347=lf[7]; // CONST
save(t347); // SEQ
X t348=____5femit(t347); // REF: _emit
t347=restore();
X t349=TO_N(10); // ICONST
X t350=____5femit(t349); // REF: _emit
t342=t350;
}else{
X t351=___id(t336); // REF: id
t342=t351;}
t336=restore();
save(t336);
X t352=allocate(5); // CONS
save(t352);
t336=pick(2);
X t353=lf[8]; // CONST
X t354=___cat(t353); // REF: cat
t352=top();
S_DATA(t352)[0]=t354;
t354=pick(2);
X t355=select1(TO_N(3),t354); // SELECT (simple)
save(t355);
X t356=allocate(8); // CONS
save(t356);
t355=pick(2);
X t357=lf[280]; // CONST
t356=top();
S_DATA(t356)[0]=t357;
t357=pick(2);
X t358=lf[9]; // CONST
t356=top();
S_DATA(t356)[1]=t358;
t358=pick(2);
X t359=___len(t358); // REF: len
X t360=___tos(t359); // REF: tos
t356=top();
S_DATA(t356)[2]=t360;
t360=pick(2);
X t361=lf[261]; // CONST
t356=top();
S_DATA(t356)[3]=t361;
t361=pick(2);
X t362=lf[280]; // CONST
t356=top();
S_DATA(t356)[4]=t362;
t362=pick(2);
X t363=lf[10]; // CONST
t356=top();
S_DATA(t356)[5]=t363;
t363=pick(2);
save(t363);
X t364=allocate(2); // CONS
save(t364);
t363=pick(2);
X t365=___len(t363); // REF: len
X t366=___iota(t365); // REF: iota
t364=top();
S_DATA(t364)[0]=t366;
t366=pick(2);
X t367=___id(t366); // REF: id
t364=top();
S_DATA(t364)[1]=t367;
t364=restore();
restore();
X t368=___trans(t364); // REF: trans
int t370; // ALPHA
check_S(t368,"@");
int t371=S_LENGTH(t368);
save(t368);
X t369=allocate(t371);
save(t369);
for(t370=0;t370<t371;++t370){
X t369=S_DATA(pick(2))[t370];
save(t369);
X t372=allocate(6); // CONS
save(t372);
t369=pick(2);
X t373=lf[280]; // CONST
t372=top();
S_DATA(t372)[0]=t373;
t373=pick(2);
X t374=lf[175]; // CONST
t372=top();
S_DATA(t372)[1]=t374;
t374=pick(2);
X t375=select1(TO_N(1),t374); // SELECT (simple)
save(t375);
X t376=allocate(2); // CONS
save(t376);
t375=pick(2);
X t377=___id(t375); // REF: id
t376=top();
S_DATA(t376)[0]=t377;
t377=pick(2);
X t378=TO_N(1); // ICONST
t376=top();
S_DATA(t376)[1]=t378;
t376=restore();
restore();
X t379=___sub(t376); // REF: sub
X t380=___tos(t379); // REF: tos
t372=top();
S_DATA(t372)[2]=t380;
t380=pick(2);
X t381=lf[183]; // CONST
t372=top();
S_DATA(t372)[3]=t381;
t381=pick(2);
X t382=select1(TO_N(2),t381); // SELECT (simple)
X t383=___literals_3aconstruct(t382); // REF: literals:construct
t372=top();
S_DATA(t372)[4]=t383;
t383=pick(2);
X t384=lf[288]; // CONST
t372=top();
S_DATA(t372)[5]=t384;
t372=restore();
restore();
X t385=___cat(t372); // REF: cat
S_DATA(top())[t370]=t385;}
t369=restore();
restore();
X t386=___cat(t369); // REF: cat
t356=top();
S_DATA(t356)[6]=t386;
t386=pick(2);
X t387=lf[11]; // CONST
t356=top();
S_DATA(t356)[7]=t387;
t356=restore();
restore();
X t388=___cat(t356); // REF: cat
t352=top();
S_DATA(t352)[1]=t388;
t388=pick(2);
X t389=select1(TO_N(1),t388); // SELECT (simple)
int t391; // ALPHA
check_S(t389,"@");
int t392=S_LENGTH(t389);
save(t389);
X t390=allocate(t392);
save(t390);
for(t391=0;t391<t392;++t391){
X t390=S_DATA(pick(2))[t391];
save(t390);
X t393=allocate(4); // CONS
save(t393);
t390=pick(2);
X t394=lf[280]; // CONST
t393=top();
S_DATA(t393)[0]=t394;
t394=pick(2);
X t395=lf[13]; // CONST
t393=top();
S_DATA(t393)[1]=t395;
t395=pick(2);
X t396=select1(TO_N(1),t395); // SELECT (simple)
X t397=___backend_3amangle(t396); // REF: backend:mangle
t393=top();
S_DATA(t393)[2]=t397;
t397=pick(2);
X t398=lf[272]; // CONST
t393=top();
S_DATA(t393)[3]=t398;
t393=restore();
restore();
X t399=___cat(t393); // REF: cat
S_DATA(top())[t391]=t399;}
t390=restore();
restore();
X t400=___cat(t390); // REF: cat
t352=top();
S_DATA(t352)[2]=t400;
t400=pick(2);
X t401=select1(TO_N(1),t400); // SELECT (simple)
int t403; // ALPHA
check_S(t401,"@");
int t404=S_LENGTH(t401);
save(t401);
X t402=allocate(t404);
save(t402);
for(t403=0;t403<t404;++t403){
X t402=S_DATA(pick(2))[t403];
save(t402);
X t405=allocate(2); // CONS
save(t405);
t402=pick(2);
save(t402);
X t406=allocate(22); // CONS
save(t406);
t402=pick(2);
X t407=lf[280]; // CONST
t406=top();
S_DATA(t406)[0]=t407;
t407=pick(2);
X t408=lf[12]; // CONST
t406=top();
S_DATA(t406)[1]=t408;
t408=pick(2);
X t409=select1(TO_N(1),t408); // SELECT (simple)
X t410=___tos(t409); // REF: tos
t406=top();
S_DATA(t406)[2]=t410;
t410=pick(2);
X t411=lf[115]; // CONST
t406=top();
S_DATA(t406)[3]=t411;
t411=pick(2);
X t413=TO_N(2); // ICONST
save(t413);
X t414=select1(TO_N(2),t411); // SELECT (simple)
t413=restore();
X t412=select1(t413,t414); // SELECT
X t415=___tos(t412); // REF: tos
t406=top();
S_DATA(t406)[4]=t415;
t415=pick(2);
X t416=lf[284]; // CONST
t406=top();
S_DATA(t406)[5]=t416;
t416=pick(2);
X t417=lf[280]; // CONST
t406=top();
S_DATA(t406)[6]=t417;
t417=pick(2);
X t418=lf[13]; // CONST
t406=top();
S_DATA(t406)[7]=t418;
t418=pick(2);
X t419=select1(TO_N(1),t418); // SELECT (simple)
X t420=___backend_3amangle(t419); // REF: backend:mangle
t406=top();
S_DATA(t406)[8]=t420;
t420=pick(2);
X t421=lf[212]; // CONST
t406=top();
S_DATA(t406)[9]=t421;
t421=pick(2);
X t422=lf[280]; // CONST
t406=top();
S_DATA(t406)[10]=t422;
t422=pick(2);
X t423=lf[14]; // CONST
t406=top();
S_DATA(t406)[11]=t423;
t423=pick(2);
X t424=lf[280]; // CONST
t406=top();
S_DATA(t406)[12]=t424;
t424=pick(2);
X t425=lf[15]; // CONST
t406=top();
S_DATA(t406)[13]=t425;
t425=pick(2);
X t426=lf[280]; // CONST
t406=top();
S_DATA(t406)[14]=t426;
t426=pick(2);
X t427=lf[16]; // CONST
t406=top();
S_DATA(t406)[15]=t427;
t427=pick(2);
X t428=lf[206]; // CONST
t406=top();
S_DATA(t406)[16]=t428;
t428=pick(2);
X t430=TO_N(2); // ICONST
save(t430);
X t431=select1(TO_N(2),t428); // SELECT (simple)
t430=restore();
X t429=select1(t430,t431); // SELECT
X t432=___tos(t429); // REF: tos
t406=top();
S_DATA(t406)[17]=t432;
t432=pick(2);
X t433=lf[17]; // CONST
t406=top();
S_DATA(t406)[18]=t433;
t433=pick(2);
X t434=select1(TO_N(1),t433); // SELECT (simple)
X t435=___tos(t434); // REF: tos
t406=top();
S_DATA(t406)[19]=t435;
t435=pick(2);
X t436=lf[206]; // CONST
t406=top();
S_DATA(t406)[20]=t436;
t436=pick(2);
X t437=lf[272]; // CONST
t406=top();
S_DATA(t406)[21]=t437;
t406=restore();
restore();
X t438=___cat(t406); // REF: cat
t405=top();
S_DATA(t405)[0]=t438;
t438=pick(2);
save(t438);
X t439=allocate(2); // CONS
save(t439);
t438=pick(2);
X t440=select1(TO_N(2),t438); // SELECT (simple)
t439=top();
S_DATA(t439)[0]=t440;
t440=pick(2);
X t441=select1(TO_N(1),t440); // SELECT (simple)
save(t441);
X t442=allocate(4); // CONS
save(t442);
t441=pick(2);
X t443=lf[20]; // CONST
t442=top();
S_DATA(t442)[0]=t443;
t443=pick(2);
X t444=TO_N(1); // ICONST
t442=top();
S_DATA(t442)[1]=t444;
t444=pick(2);
X t445=___id(t444); // REF: id
t442=top();
S_DATA(t442)[2]=t445;
t445=pick(2);
X t446=EMPTY; // ICONST
t442=top();
S_DATA(t442)[3]=t446;
t442=restore();
restore();
t439=top();
S_DATA(t439)[1]=t442;
t439=restore();
restore();
X t447=___backend_3at_5fexpr(t439); // REF: backend:t_expr
save(t447);
X t448=allocate(5); // CONS
save(t448);
t447=pick(2);
X t449=select1(TO_N(4),t447); // SELECT (simple)
X t450=___cat(t449); // REF: cat
t448=top();
S_DATA(t448)[0]=t450;
t450=pick(2);
X t451=lf[280]; // CONST
t448=top();
S_DATA(t448)[1]=t451;
t451=pick(2);
X t452=lf[18]; // CONST
t448=top();
S_DATA(t448)[2]=t452;
t452=pick(2);
X t453=select1(TO_N(1),t452); // SELECT (simple)
t448=top();
S_DATA(t448)[3]=t453;
t453=pick(2);
X t454=lf[19]; // CONST
t448=top();
S_DATA(t448)[4]=t454;
t448=restore();
restore();
X t455=___cat(t448); // REF: cat
t405=top();
S_DATA(t405)[1]=t455;
t405=restore();
restore();
X t456=___cat(t405); // REF: cat
S_DATA(top())[t403]=t456;}
t402=restore();
restore();
X t457=___cat(t402); // REF: cat
t352=top();
S_DATA(t352)[3]=t457;
t457=pick(2);
save(t457);
X t458=allocate(10); // CONS
save(t458);
t457=pick(2);
save(t457);
X t459=allocate(2); // CONS
save(t459);
t457=pick(2);
X t460=lf[373]; // CONST
t459=top();
S_DATA(t459)[0]=t460;
t460=pick(2);
X t461=select1(TO_N(2),t460); // SELECT (simple)
t459=top();
S_DATA(t459)[1]=t461;
t459=restore();
restore();
X t462=___fetchall(t459); // REF: fetchall
int t464; // ALPHA
check_S(t462,"@");
int t465=S_LENGTH(t462);
save(t462);
X t463=allocate(t465);
save(t463);
for(t464=0;t464<t465;++t464){
X t463=S_DATA(pick(2))[t464];
save(t463);
X t466=allocate(4); // CONS
save(t466);
t463=pick(2);
X t467=lf[280]; // CONST
t466=top();
S_DATA(t466)[0]=t467;
t467=pick(2);
X t468=lf[21]; // CONST
t466=top();
S_DATA(t466)[1]=t468;
t468=pick(2);
X t469=___tos(t468); // REF: tos
t466=top();
S_DATA(t466)[2]=t469;
t469=pick(2);
X t470=lf[206]; // CONST
t466=top();
S_DATA(t466)[3]=t470;
t466=restore();
restore();
X t471=___cat(t466); // REF: cat
S_DATA(top())[t464]=t471;}
t463=restore();
restore();
X t472=___cat(t463); // REF: cat
t458=top();
S_DATA(t458)[0]=t472;
t472=pick(2);
X t473=lf[280]; // CONST
t458=top();
S_DATA(t458)[1]=t473;
t473=pick(2);
X t474=lf[22]; // CONST
t458=top();
S_DATA(t458)[2]=t474;
t474=pick(2);
X t475=lf[280]; // CONST
t458=top();
S_DATA(t458)[3]=t475;
t475=pick(2);
X t476=lf[23]; // CONST
t458=top();
S_DATA(t458)[4]=t476;
t476=pick(2);
save(t476);
X t477=allocate(2); // CONS
save(t477);
t476=pick(2);
X t478=lf[383]; // CONST
t477=top();
S_DATA(t477)[0]=t478;
t478=pick(2);
X t479=select1(TO_N(2),t478); // SELECT (simple)
t477=top();
S_DATA(t477)[1]=t479;
t477=restore();
restore();
save(t477); // OR
X t481=___fetch(t477); // REF: fetch
X t480=t481;
t477=restore();
if(t480==F){
X t482=lf[346]; // CONST
t480=t482;}
X t483=___backend_3amangle(t480); // REF: backend:mangle
t458=top();
S_DATA(t458)[5]=t483;
t483=pick(2);
X t484=lf[24]; // CONST
t458=top();
S_DATA(t458)[6]=t484;
t484=pick(2);
X t485=lf[280]; // CONST
t458=top();
S_DATA(t458)[7]=t485;
t485=pick(2);
X t486=lf[25]; // CONST
t458=top();
S_DATA(t458)[8]=t486;
t486=pick(2);
X t487=lf[280]; // CONST
t458=top();
S_DATA(t458)[9]=t487;
t458=restore();
restore();
X t488=___cat(t458); // REF: cat
t352=top();
S_DATA(t352)[4]=t488;
t352=restore();
restore();
X t489=___cat(t352); // REF: cat
t334=top();
S_DATA(t334)[1]=t489;
t334=restore();
restore();
save(t334);
X t490=allocate(2); // CONS
save(t490);
t334=pick(2);
X t491=select1(TO_N(1),t334); // SELECT (simple)
t490=top();
S_DATA(t490)[0]=t491;
t491=pick(2);
save(t491);
X t492=allocate(2); // CONS
save(t492);
t491=pick(2);
X t494=TO_N(1); // ICONST
save(t494);
X t495=select1(TO_N(1),t491); // SELECT (simple)
t494=restore();
X t493=select1(t494,t495); // SELECT
t492=top();
S_DATA(t492)[0]=t493;
t493=pick(2);
X t496=select1(TO_N(2),t493); // SELECT (simple)
t492=top();
S_DATA(t492)[1]=t496;
t492=restore();
restore();
X t497=____5fout(t492); // REF: _out
t490=top();
S_DATA(t490)[1]=t497;
t490=restore();
restore();
save(t490); // COND
save(t490);
X t499=allocate(2); // CONS
save(t499);
t490=pick(2);
X t500=lf[364]; // CONST
t499=top();
S_DATA(t499)[0]=t500;
t500=pick(2);
X t502=TO_N(2); // ICONST
save(t502);
X t503=select1(TO_N(1),t500); // SELECT (simple)
t502=restore();
X t501=select1(t502,t503); // SELECT
t499=top();
S_DATA(t499)[1]=t501;
t499=restore();
restore();
X t504=___fetch(t499); // REF: fetch
X t498;
t490=restore();
if(t504!=F){
save(t490);
X t505=allocate(3); // CONS
save(t505);
t490=pick(2);
X t507=TO_N(1); // ICONST
save(t507);
X t508=select1(TO_N(1),t490); // SELECT (simple)
t507=restore();
X t506=select1(t507,t508); // SELECT
t505=top();
S_DATA(t505)[0]=t506;
t506=pick(2);
X t510=TO_N(3); // ICONST
save(t510);
X t511=select1(TO_N(1),t506); // SELECT (simple)
t510=restore();
X t509=select1(t510,t511); // SELECT
t505=top();
S_DATA(t505)[1]=t509;
t509=pick(2);
X t513=TO_N(2); // ICONST
save(t513);
X t514=select1(TO_N(1),t509); // SELECT (simple)
t513=restore();
X t512=select1(t513,t514); // SELECT
t505=top();
S_DATA(t505)[2]=t512;
t505=restore();
restore();
save(t505);
X t515=allocate(2); // CONS
save(t515);
t505=pick(2);
save(t505);
X t516=allocate(2); // CONS
save(t516);
t505=pick(2);
X t517=lf[3]; // CONST
t516=top();
S_DATA(t516)[0]=t517;
t517=pick(2);
save(t517);
X t518=allocate(2); // CONS
save(t518);
t517=pick(2);
save(t517);
X t519=allocate(3); // CONS
save(t519);
t517=pick(2);
X t520=lf[5]; // CONST
save(t520); // OR
X t522=____5fenv(t520); // REF: _env
X t521=t522;
t520=restore();
if(t521==F){
X t523=lf[4]; // CONST
t521=t523;}
X t524=___tos(t521); // REF: tos
t519=top();
S_DATA(t519)[0]=t524;
t524=pick(2);
X t525=lf[6]; // CONST
t519=top();
S_DATA(t519)[1]=t525;
t525=pick(2);
X t526=select1(TO_N(1),t525); // SELECT (simple)
t519=top();
S_DATA(t519)[2]=t526;
t519=restore();
restore();
t518=top();
S_DATA(t518)[0]=t519;
t519=pick(2);
X t527=select1(TO_N(2),t519); // SELECT (simple)
t518=top();
S_DATA(t518)[1]=t527;
t518=restore();
restore();
X t528=___cat(t518); // REF: cat
t516=top();
S_DATA(t516)[1]=t528;
t516=restore();
restore();
X t529=___dl(t516); // REF: dl
X t530=___cat(t529); // REF: cat
X t531=___tl(t530); // REF: tl
X t532=___cat(t531); // REF: cat
t515=top();
S_DATA(t515)[0]=t532;
t532=pick(2);
X t533=select1(TO_N(3),t532); // SELECT (simple)
t515=top();
S_DATA(t515)[1]=t533;
t515=restore();
restore();
save(t515); // COND
X t535=select1(TO_N(2),t515); // SELECT (simple)
save(t535);
X t536=allocate(2); // CONS
save(t536);
t535=pick(2);
X t537=lf[361]; // CONST
t536=top();
S_DATA(t536)[0]=t537;
t537=pick(2);
X t538=___id(t537); // REF: id
t536=top();
S_DATA(t536)[1]=t538;
t536=restore();
restore();
X t539=___fetch(t536); // REF: fetch
X t534;
t515=restore();
if(t539!=F){
save(t515); // SEQ
save(t515);
X t540=allocate(2); // CONS
save(t540);
t515=pick(2);
X t541=lf[343]; // CONST
t540=top();
S_DATA(t540)[0]=t541;
t541=pick(2);
X t542=select1(TO_N(1),t541); // SELECT (simple)
t540=top();
S_DATA(t540)[1]=t542;
t540=restore();
restore();
X t543=___cat(t540); // REF: cat
save(t543); // SEQ
X t544=____5femit(t543); // REF: _emit
t543=restore();
X t545=TO_N(10); // ICONST
X t546=____5femit(t545); // REF: _emit
t515=restore();
X t547=select1(TO_N(1),t515); // SELECT (simple)
t534=t547;
}else{
X t548=select1(TO_N(1),t515); // SELECT (simple)
t534=t548;}
save(t534); // COND
X t551=TO_N(0); // ICONST
save(t551);
X t552=____5fsystem(t534); // REF: _system
t551=restore();
X t550=(t551==t552)||eq1(t551,t552)?T:F; // EQ
X t549;
t534=restore();
if(t550!=F){
X t553=T; // ICONST
t549=t553;
}else{
save(t534);
X t554=allocate(2); // CONS
save(t554);
t534=pick(2);
X t555=lf[2]; // CONST
t554=top();
S_DATA(t554)[0]=t555;
t555=pick(2);
X t556=___id(t555); // REF: id
t554=top();
S_DATA(t554)[1]=t556;
t554=restore();
restore();
X t557=___cat(t554); // REF: cat
X t558=___toa(t557); // REF: toa
X t559=___quit(t558); // REF: quit
t549=t559;}
t498=t549;
}else{
X t560=___id(t490); // REF: id
t498=t560;}
t230=t498;}
t219=t230;}
t209=restore();
X t561=lf[358]; // CONST
X t562=____5fthrow(t561); // REF: _throw
t8=t562;
}else{
X t563=lf[382]; // CONST
X t564=___quit(t563); // REF: quit
t8=t564;}
save(t8);
X t565=allocate(2); // CONS
save(t565);
t8=pick(2);
X t566=TO_N(0); // ICONST
t565=top();
S_DATA(t565)[0]=t566;
t566=pick(2);
X t567=select1(TO_N(2),t566); // SELECT (simple)
t565=top();
S_DATA(t565)[1]=t567;
t565=restore();
restore();
save(t565); // SEQ
save(t565);
X t568=allocate(2); // CONS
save(t568);
t565=pick(2);
X t569=select1(TO_N(2),t565); // SELECT (simple)
t568=top();
S_DATA(t568)[0]=t569;
t569=pick(2);
X t570=lf[0]; // CONST
t568=top();
S_DATA(t568)[1]=t570;
t568=restore();
restore();
X t571=____5fiostep(t568); // REF: _iostep
t565=restore();
save(t565);
X t572=allocate(2); // CONS
save(t572);
t565=pick(2);
X t573=lf[1]; // CONST
t572=top();
S_DATA(t572)[0]=t573;
t573=pick(2);
X t574=select1(TO_N(1),t573); // SELECT (simple)
t572=top();
S_DATA(t572)[1]=t574;
t572=restore();
restore();
X t575=____5fthrow(t572); // REF: _throw
RETURN(t575);}
//---------------------------------------- quit (util.fp:9)
DEFINE(___quit){
ENTRY;
loop:;
tracecall("util.fp:9:  quit");
save(x); // SEQ
X t1=____5femit(x); // REF: _emit
x=restore();
save(x); // SEQ
X t2=TO_N(10); // ICONST
X t3=____5femit(t2); // REF: _emit
x=restore();
X t4=lf[31]; // CONST
X t5=____5fthrow(t4); // REF: _throw
RETURN(t5);}
//---------------------------------------- constnode (util.fp:31)
DEFINE(___constnode){
ENTRY;
loop:;
tracecall("util.fp:31:  constnode");
save(x);
X t1=allocate(2); // CONS
save(t1);
x=pick(2);
X t2=select1(TO_N(1),x); // SELECT (simple)
t1=top();
S_DATA(t1)[0]=t2;
t2=pick(2);
X t3=lf[102]; // CONST
t1=top();
S_DATA(t1)[1]=t3;
t1=restore();
restore();
X t4=___index(t1); // REF: index
RETURN(t4);}
//---------------------------------------- lex:lex (lex.fp:26)
DEFINE(___lex_3alex){
ENTRY;
loop:;
tracecall("lex.fp:26:  lex:lex");
save(x);
X t1=allocate(3); // CONS
save(t1);
x=pick(2);
X t2=____5fin(x); // REF: _in
t1=top();
S_DATA(t1)[0]=t2;
t2=pick(2);
save(t2);
X t3=allocate(2); // CONS
save(t3);
t2=pick(2);
X t4=TO_N(1); // ICONST
t3=top();
S_DATA(t3)[0]=t4;
t4=pick(2);
X t5=___toa(t4); // REF: toa
t3=top();
S_DATA(t3)[1]=t5;
t3=restore();
restore();
t1=top();
S_DATA(t1)[1]=t3;
t3=pick(2);
X t6=EMPTY; // ICONST
t1=top();
S_DATA(t1)[2]=t6;
t1=restore();
restore();
save(t1); // COND
X t8=select1(TO_N(1),t1); // SELECT (simple)
X t9=F;
save(t8); // PCONSL
if(!IS_S(t8)||S_LENGTH(t8)<2) goto t10;
t8=S_DATA(top())[0];
X t12=lf[32]; // CONST
save(t12);
X t13=___id(t8); // REF: id
t12=restore();
X t11=(t12==t13)||eq1(t12,t13)?T:F; // EQ
if(t11==F) goto t10;
t11=S_DATA(top())[1];
X t15=lf[33]; // CONST
save(t15);
X t16=___id(t11); // REF: id
t15=restore();
X t14=(t15==t16)||eq1(t15,t16)?T:F; // EQ
if(t14==F) goto t10;
t9=T;
t10:
restore();
X t7;
t1=restore();
if(t9!=F){
X t17=___lex_3askip_5fcomment(t1); // REF: lex:skip_comment
t7=t17;
}else{
X t18=___id(t1); // REF: id
t7=t18;}
X t19=___lex_3aloop(t7); // REF: lex:loop
RETURN(t19);}
//---------------------------------------- lex:loop (lex.fp:98)
DEFINE(___lex_3aloop){
ENTRY;
loop:;
tracecall("lex.fp:98:  lex:loop");
X t1=___lex_3askip(x); // REF: lex:skip
save(t1); // COND
X t4=lf[381]; // CONST
save(t4);
X t5=select1(TO_N(1),t1); // SELECT (simple)
t4=restore();
X t3=(t4==t5)||eq1(t4,t5)?T:F; // EQ
X t2;
t1=restore();
if(t3!=F){
X t6=select1(TO_N(3),t1); // SELECT (simple)
t2=t6;
}else{
save(t1); // COND
save(t1);
X t9=___lex_3anext(t1); // REF: lex:next
t1=restore();
X t10=TO_N(45); // ICONST
X t8=(t9==t10)||eq1(t9,t10)?T:F; // EQ
X t7;
t1=restore();
if(t8!=F){
save(t1); // COND
X t13=lf[381]; // CONST
save(t13);
X t14=select1(TO_N(1),t1); // SELECT (simple)
X t15=___tl(t14); // REF: tl
t13=restore();
X t12=(t13==t15)||eq1(t13,t15)?T:F; // EQ
X t11;
t1=restore();
if(t12!=F){
X t17=TO_N(3); // ICONST
save(t17);
save(t1);
X t18=allocate(2); // CONS
save(t18);
t1=pick(2);
save(t1);
X t19=allocate(3); // CONS
save(t19);
t1=pick(2);
X t20=lf[90]; // CONST
t19=top();
S_DATA(t19)[0]=t20;
t20=pick(2);
X t21=___lex_3asourceinfo(t20); // REF: lex:sourceinfo
t19=top();
S_DATA(t19)[1]=t21;
t21=pick(2);
X t22=lf[34]; // CONST
t19=top();
S_DATA(t19)[2]=t22;
t19=restore();
restore();
t18=top();
S_DATA(t18)[0]=t19;
t19=pick(2);
save(t19);
X t23=allocate(3); // CONS
save(t23);
t19=pick(2);
X t24=EMPTY; // ICONST
t23=top();
S_DATA(t23)[0]=t24;
t24=pick(2);
X t25=select1(TO_N(2),t24); // SELECT (simple)
t23=top();
S_DATA(t23)[1]=t25;
t25=pick(2);
X t26=select1(TO_N(3),t25); // SELECT (simple)
t23=top();
S_DATA(t23)[2]=t26;
t23=restore();
restore();
t18=top();
S_DATA(t18)[1]=t23;
t18=restore();
restore();
X t27=___lex_3apush(t18); // REF: lex:push
t17=restore();
X t16=select1(t17,t27); // SELECT
t11=t16;
}else{
save(t1); // COND
X t30=TO_N(2); // ICONST
save(t30);
X t31=select1(TO_N(1),t1); // SELECT (simple)
t30=restore();
X t29=select1(t30,t31); // SELECT
X t32=___lex_3adigit(t29); // REF: lex:digit
X t28;
t1=restore();
if(t32!=F){
save(t1);
X t33=allocate(2); // CONS
save(t33);
t1=pick(2);
X t34=lf[37]; // CONST
t33=top();
S_DATA(t33)[0]=t34;
t34=pick(2);
X t35=___lex_3aadvance(t34); // REF: lex:advance
t33=top();
S_DATA(t33)[1]=t35;
t33=restore();
restore();
X t36=___lex_3ascan(t33); // REF: lex:scan
save(t36);
X t37=allocate(2); // CONS
save(t37);
t36=pick(2);
save(t36);
X t38=allocate(3); // CONS
save(t38);
t36=pick(2);
X t39=lf[86]; // CONST
t38=top();
S_DATA(t38)[0]=t39;
t39=pick(2);
X t40=select1(TO_N(1),t39); // SELECT (simple)
X t41=___lex_3asourceinfo(t40); // REF: lex:sourceinfo
t38=top();
S_DATA(t38)[1]=t41;
t41=pick(2);
X t42=select1(TO_N(2),t41); // SELECT (simple)
X t43=___ton(t42); // REF: ton
save(t43);
X t44=allocate(2); // CONS
save(t44);
t43=pick(2);
X t45=TO_N(0); // ICONST
t44=top();
S_DATA(t44)[0]=t45;
t45=pick(2);
X t46=___id(t45); // REF: id
t44=top();
S_DATA(t44)[1]=t46;
t44=restore();
restore();
X t47=___sub(t44); // REF: sub
t38=top();
S_DATA(t38)[2]=t47;
t38=restore();
restore();
t37=top();
S_DATA(t37)[0]=t38;
t38=pick(2);
X t48=select1(TO_N(1),t38); // SELECT (simple)
t37=top();
S_DATA(t37)[1]=t48;
t37=restore();
restore();
t28=t37;
}else{
save(t1); // COND
save(t1);
X t52=TO_N(2); // ICONST
save(t52);
X t53=select1(TO_N(1),t1); // SELECT (simple)
t52=restore();
X t51=select1(t52,t53); // SELECT
t1=restore();
X t54=TO_N(62); // ICONST
X t50=(t51==t54)||eq1(t51,t54)?T:F; // EQ
X t49;
t1=restore();
if(t50!=F){
save(t1);
X t55=allocate(2); // CONS
save(t55);
t1=pick(2);
save(t1);
X t56=allocate(3); // CONS
save(t56);
t1=pick(2);
X t57=lf[90]; // CONST
t56=top();
S_DATA(t56)[0]=t57;
t57=pick(2);
X t58=___lex_3asourceinfo(t57); // REF: lex:sourceinfo
t56=top();
S_DATA(t56)[1]=t58;
t58=pick(2);
X t59=lf[51]; // CONST
t56=top();
S_DATA(t56)[2]=t59;
t56=restore();
restore();
t55=top();
S_DATA(t55)[0]=t56;
t56=pick(2);
save(t56);
X t60=allocate(3); // CONS
save(t60);
t56=pick(2);
X t61=select1(TO_N(1),t56); // SELECT (simple)
X t62=___tl(t61); // REF: tl
X t63=___tl(t62); // REF: tl
t60=top();
S_DATA(t60)[0]=t63;
t63=pick(2);
X t64=select1(TO_N(2),t63); // SELECT (simple)
t60=top();
S_DATA(t60)[1]=t64;
t64=pick(2);
X t65=select1(TO_N(3),t64); // SELECT (simple)
t60=top();
S_DATA(t60)[2]=t65;
t60=restore();
restore();
t55=top();
S_DATA(t55)[1]=t60;
t55=restore();
restore();
t49=t55;
}else{
save(t1);
X t66=allocate(2); // CONS
save(t66);
t1=pick(2);
save(t1);
X t67=allocate(3); // CONS
save(t67);
t1=pick(2);
X t68=lf[90]; // CONST
t67=top();
S_DATA(t67)[0]=t68;
t68=pick(2);
X t69=___lex_3asourceinfo(t68); // REF: lex:sourceinfo
t67=top();
S_DATA(t67)[1]=t69;
t69=pick(2);
X t70=lf[34]; // CONST
t67=top();
S_DATA(t67)[2]=t70;
t67=restore();
restore();
t66=top();
S_DATA(t66)[0]=t67;
t67=pick(2);
save(t67);
X t71=allocate(3); // CONS
save(t71);
t67=pick(2);
X t72=select1(TO_N(1),t67); // SELECT (simple)
X t73=___tl(t72); // REF: tl
t71=top();
S_DATA(t71)[0]=t73;
t73=pick(2);
X t74=select1(TO_N(2),t73); // SELECT (simple)
t71=top();
S_DATA(t71)[1]=t74;
t74=pick(2);
X t75=select1(TO_N(3),t74); // SELECT (simple)
t71=top();
S_DATA(t71)[2]=t75;
t71=restore();
restore();
t66=top();
S_DATA(t66)[1]=t71;
t66=restore();
restore();
t49=t66;}
t28=t49;}
X t76=___lex_3apush(t28); // REF: lex:push
x=t76; // REF: lex:loop
goto loop; // tail call: lex:loop
X t77;
t11=t77;}
t7=t11;
}else{
save(t1); // COND
save(t1);
X t80=___lex_3anext(t1); // REF: lex:next
t1=restore();
X t81=TO_N(46); // ICONST
X t79=(t80==t81)||eq1(t80,t81)?T:F; // EQ
X t78;
t1=restore();
if(t79!=F){
save(t1); // COND
save(t1); // OR
X t85=lf[381]; // CONST
save(t85);
X t86=select1(TO_N(1),t1); // SELECT (simple)
X t87=___tl(t86); // REF: tl
t85=restore();
X t84=(t85==t87)||eq1(t85,t87)?T:F; // EQ
X t83=t84;
t1=restore();
if(t83==F){
X t89=TO_N(46); // ICONST
save(t89);
X t91=TO_N(2); // ICONST
save(t91);
X t92=select1(TO_N(1),t1); // SELECT (simple)
t91=restore();
X t90=select1(t91,t92); // SELECT
t89=restore();
X t88=(t89==t90)||eq1(t89,t90)?T:F; // EQ
save(t88); // COND
X t94=___id(t88); // REF: id
X t93;
t88=restore();
if(t94!=F){
X t95=F; // ICONST
t93=t95;
}else{
X t96=T; // ICONST
t93=t96;}
t83=t93;}
X t82;
t1=restore();
if(t83!=F){
save(t1);
X t97=allocate(2); // CONS
save(t97);
t1=pick(2);
save(t1);
X t98=allocate(3); // CONS
save(t98);
t1=pick(2);
X t99=lf[90]; // CONST
t98=top();
S_DATA(t98)[0]=t99;
t99=pick(2);
X t100=___lex_3asourceinfo(t99); // REF: lex:sourceinfo
t98=top();
S_DATA(t98)[1]=t100;
t100=pick(2);
X t101=lf[76]; // CONST
t98=top();
S_DATA(t98)[2]=t101;
t98=restore();
restore();
t97=top();
S_DATA(t97)[0]=t98;
t98=pick(2);
save(t98);
X t102=allocate(3); // CONS
save(t102);
t98=pick(2);
X t103=select1(TO_N(1),t98); // SELECT (simple)
X t104=___tl(t103); // REF: tl
t102=top();
S_DATA(t102)[0]=t104;
t104=pick(2);
X t105=select1(TO_N(2),t104); // SELECT (simple)
t102=top();
S_DATA(t102)[1]=t105;
t105=pick(2);
X t106=select1(TO_N(3),t105); // SELECT (simple)
t102=top();
S_DATA(t102)[2]=t106;
t102=restore();
restore();
t97=top();
S_DATA(t97)[1]=t102;
t97=restore();
restore();
t82=t97;
}else{
save(t1);
X t107=allocate(2); // CONS
save(t107);
t1=pick(2);
save(t1);
X t108=allocate(3); // CONS
save(t108);
t1=pick(2);
X t109=lf[90]; // CONST
t108=top();
S_DATA(t108)[0]=t109;
t109=pick(2);
X t110=___lex_3asourceinfo(t109); // REF: lex:sourceinfo
t108=top();
S_DATA(t108)[1]=t110;
t110=pick(2);
X t111=lf[83]; // CONST
t108=top();
S_DATA(t108)[2]=t111;
t108=restore();
restore();
t107=top();
S_DATA(t107)[0]=t108;
t108=pick(2);
save(t108);
X t112=allocate(3); // CONS
save(t112);
t108=pick(2);
X t113=select1(TO_N(1),t108); // SELECT (simple)
X t114=___tl(t113); // REF: tl
X t115=___tl(t114); // REF: tl
t112=top();
S_DATA(t112)[0]=t115;
t115=pick(2);
X t116=select1(TO_N(2),t115); // SELECT (simple)
t112=top();
S_DATA(t112)[1]=t116;
t116=pick(2);
X t117=select1(TO_N(3),t116); // SELECT (simple)
t112=top();
S_DATA(t112)[2]=t117;
t112=restore();
restore();
t107=top();
S_DATA(t107)[1]=t112;
t107=restore();
restore();
t82=t107;}
X t118=___lex_3apush(t82); // REF: lex:push
x=t118; // REF: lex:loop
goto loop; // tail call: lex:loop
X t119;
t78=t119;
}else{
save(t1); // COND
save(t1);
X t122=___lex_3anext(t1); // REF: lex:next
t1=restore();
X t123=TO_N(91); // ICONST
X t121=(t122==t123)||eq1(t122,t123)?T:F; // EQ
X t120;
t1=restore();
if(t121!=F){
save(t1); // COND
X t126=lf[381]; // CONST
save(t126);
X t127=select1(TO_N(1),t1); // SELECT (simple)
X t128=___tl(t127); // REF: tl
t126=restore();
X t125=(t126==t128)||eq1(t126,t128)?T:F; // EQ
X t124;
t1=restore();
if(t125!=F){
X t130=TO_N(3); // ICONST
save(t130);
save(t1);
X t131=allocate(2); // CONS
save(t131);
t1=pick(2);
save(t1);
X t132=allocate(3); // CONS
save(t132);
t1=pick(2);
X t133=lf[90]; // CONST
t132=top();
S_DATA(t132)[0]=t133;
t133=pick(2);
X t134=___lex_3asourceinfo(t133); // REF: lex:sourceinfo
t132=top();
S_DATA(t132)[1]=t134;
t134=pick(2);
X t135=lf[58]; // CONST
t132=top();
S_DATA(t132)[2]=t135;
t132=restore();
restore();
t131=top();
S_DATA(t131)[0]=t132;
t132=pick(2);
save(t132);
X t136=allocate(3); // CONS
save(t136);
t132=pick(2);
X t137=EMPTY; // ICONST
t136=top();
S_DATA(t136)[0]=t137;
t137=pick(2);
X t138=select1(TO_N(2),t137); // SELECT (simple)
t136=top();
S_DATA(t136)[1]=t138;
t138=pick(2);
X t139=select1(TO_N(3),t138); // SELECT (simple)
t136=top();
S_DATA(t136)[2]=t139;
t136=restore();
restore();
t131=top();
S_DATA(t131)[1]=t136;
t131=restore();
restore();
X t140=___lex_3apush(t131); // REF: lex:push
t130=restore();
X t129=select1(t130,t140); // SELECT
t124=t129;
}else{
save(t1); // COND
save(t1);
X t144=TO_N(2); // ICONST
save(t144);
X t145=select1(TO_N(1),t1); // SELECT (simple)
t144=restore();
X t143=select1(t144,t145); // SELECT
t1=restore();
X t146=TO_N(124); // ICONST
X t142=(t143==t146)||eq1(t143,t146)?T:F; // EQ
X t141;
t1=restore();
if(t142!=F){
save(t1);
X t147=allocate(2); // CONS
save(t147);
t1=pick(2);
save(t1);
X t148=allocate(3); // CONS
save(t148);
t1=pick(2);
X t149=lf[90]; // CONST
t148=top();
S_DATA(t148)[0]=t149;
t149=pick(2);
X t150=___lex_3asourceinfo(t149); // REF: lex:sourceinfo
t148=top();
S_DATA(t148)[1]=t150;
t150=pick(2);
X t151=lf[59]; // CONST
t148=top();
S_DATA(t148)[2]=t151;
t148=restore();
restore();
t147=top();
S_DATA(t147)[0]=t148;
t148=pick(2);
save(t148);
X t152=allocate(3); // CONS
save(t152);
t148=pick(2);
X t153=select1(TO_N(1),t148); // SELECT (simple)
X t154=___tl(t153); // REF: tl
X t155=___tl(t154); // REF: tl
t152=top();
S_DATA(t152)[0]=t155;
t155=pick(2);
X t156=select1(TO_N(2),t155); // SELECT (simple)
t152=top();
S_DATA(t152)[1]=t156;
t156=pick(2);
X t157=select1(TO_N(3),t156); // SELECT (simple)
t152=top();
S_DATA(t152)[2]=t157;
t152=restore();
restore();
t147=top();
S_DATA(t147)[1]=t152;
t147=restore();
restore();
t141=t147;
}else{
save(t1);
X t158=allocate(2); // CONS
save(t158);
t1=pick(2);
save(t1);
X t159=allocate(3); // CONS
save(t159);
t1=pick(2);
X t160=lf[90]; // CONST
t159=top();
S_DATA(t159)[0]=t160;
t160=pick(2);
X t161=___lex_3asourceinfo(t160); // REF: lex:sourceinfo
t159=top();
S_DATA(t159)[1]=t161;
t161=pick(2);
X t162=lf[58]; // CONST
t159=top();
S_DATA(t159)[2]=t162;
t159=restore();
restore();
t158=top();
S_DATA(t158)[0]=t159;
t159=pick(2);
save(t159);
X t163=allocate(3); // CONS
save(t163);
t159=pick(2);
X t164=select1(TO_N(1),t159); // SELECT (simple)
X t165=___tl(t164); // REF: tl
t163=top();
S_DATA(t163)[0]=t165;
t165=pick(2);
X t166=select1(TO_N(2),t165); // SELECT (simple)
t163=top();
S_DATA(t163)[1]=t166;
t166=pick(2);
X t167=select1(TO_N(3),t166); // SELECT (simple)
t163=top();
S_DATA(t163)[2]=t167;
t163=restore();
restore();
t158=top();
S_DATA(t158)[1]=t163;
t158=restore();
restore();
t141=t158;}
X t168=___lex_3apush(t141); // REF: lex:push
x=t168; // REF: lex:loop
goto loop; // tail call: lex:loop
X t169;
t124=t169;}
t120=t124;
}else{
save(t1); // COND
save(t1);
X t172=___lex_3anext(t1); // REF: lex:next
t1=restore();
X t173=TO_N(40); // ICONST
X t171=(t172==t173)||eq1(t172,t173)?T:F; // EQ
X t170;
t1=restore();
if(t171!=F){
save(t1); // COND
X t176=lf[381]; // CONST
save(t176);
X t177=select1(TO_N(1),t1); // SELECT (simple)
X t178=___tl(t177); // REF: tl
t176=restore();
X t175=(t176==t178)||eq1(t176,t178)?T:F; // EQ
X t174;
t1=restore();
if(t175!=F){
X t180=TO_N(3); // ICONST
save(t180);
save(t1);
X t181=allocate(2); // CONS
save(t181);
t1=pick(2);
save(t1);
X t182=allocate(3); // CONS
save(t182);
t1=pick(2);
X t183=lf[90]; // CONST
t182=top();
S_DATA(t182)[0]=t183;
t183=pick(2);
X t184=___lex_3asourceinfo(t183); // REF: lex:sourceinfo
t182=top();
S_DATA(t182)[1]=t184;
t184=pick(2);
X t185=lf[56]; // CONST
t182=top();
S_DATA(t182)[2]=t185;
t182=restore();
restore();
t181=top();
S_DATA(t181)[0]=t182;
t182=pick(2);
save(t182);
X t186=allocate(3); // CONS
save(t186);
t182=pick(2);
X t187=EMPTY; // ICONST
t186=top();
S_DATA(t186)[0]=t187;
t187=pick(2);
X t188=select1(TO_N(2),t187); // SELECT (simple)
t186=top();
S_DATA(t186)[1]=t188;
t188=pick(2);
X t189=select1(TO_N(3),t188); // SELECT (simple)
t186=top();
S_DATA(t186)[2]=t189;
t186=restore();
restore();
t181=top();
S_DATA(t181)[1]=t186;
t181=restore();
restore();
X t190=___lex_3apush(t181); // REF: lex:push
t180=restore();
X t179=select1(t180,t190); // SELECT
t174=t179;
}else{
save(t1); // COND
save(t1);
X t194=TO_N(2); // ICONST
save(t194);
X t195=select1(TO_N(1),t1); // SELECT (simple)
t194=restore();
X t193=select1(t194,t195); // SELECT
t1=restore();
X t196=TO_N(124); // ICONST
X t192=(t193==t196)||eq1(t193,t196)?T:F; // EQ
X t191;
t1=restore();
if(t192!=F){
save(t1);
X t197=allocate(2); // CONS
save(t197);
t1=pick(2);
save(t1);
X t198=allocate(3); // CONS
save(t198);
t1=pick(2);
X t199=lf[90]; // CONST
t198=top();
S_DATA(t198)[0]=t199;
t199=pick(2);
X t200=___lex_3asourceinfo(t199); // REF: lex:sourceinfo
t198=top();
S_DATA(t198)[1]=t200;
t200=pick(2);
X t201=lf[60]; // CONST
t198=top();
S_DATA(t198)[2]=t201;
t198=restore();
restore();
t197=top();
S_DATA(t197)[0]=t198;
t198=pick(2);
save(t198);
X t202=allocate(3); // CONS
save(t202);
t198=pick(2);
X t203=select1(TO_N(1),t198); // SELECT (simple)
X t204=___tl(t203); // REF: tl
X t205=___tl(t204); // REF: tl
t202=top();
S_DATA(t202)[0]=t205;
t205=pick(2);
X t206=select1(TO_N(2),t205); // SELECT (simple)
t202=top();
S_DATA(t202)[1]=t206;
t206=pick(2);
X t207=select1(TO_N(3),t206); // SELECT (simple)
t202=top();
S_DATA(t202)[2]=t207;
t202=restore();
restore();
t197=top();
S_DATA(t197)[1]=t202;
t197=restore();
restore();
t191=t197;
}else{
save(t1);
X t208=allocate(2); // CONS
save(t208);
t1=pick(2);
save(t1);
X t209=allocate(3); // CONS
save(t209);
t1=pick(2);
X t210=lf[90]; // CONST
t209=top();
S_DATA(t209)[0]=t210;
t210=pick(2);
X t211=___lex_3asourceinfo(t210); // REF: lex:sourceinfo
t209=top();
S_DATA(t209)[1]=t211;
t211=pick(2);
X t212=lf[56]; // CONST
t209=top();
S_DATA(t209)[2]=t212;
t209=restore();
restore();
t208=top();
S_DATA(t208)[0]=t209;
t209=pick(2);
save(t209);
X t213=allocate(3); // CONS
save(t213);
t209=pick(2);
X t214=select1(TO_N(1),t209); // SELECT (simple)
X t215=___tl(t214); // REF: tl
t213=top();
S_DATA(t213)[0]=t215;
t215=pick(2);
X t216=select1(TO_N(2),t215); // SELECT (simple)
t213=top();
S_DATA(t213)[1]=t216;
t216=pick(2);
X t217=select1(TO_N(3),t216); // SELECT (simple)
t213=top();
S_DATA(t213)[2]=t217;
t213=restore();
restore();
t208=top();
S_DATA(t208)[1]=t213;
t208=restore();
restore();
t191=t208;}
X t218=___lex_3apush(t191); // REF: lex:push
x=t218; // REF: lex:loop
goto loop; // tail call: lex:loop
X t219;
t174=t219;}
t170=t174;
}else{
save(t1); // COND
save(t1);
X t222=___lex_3anext(t1); // REF: lex:next
t1=restore();
X t223=TO_N(124); // ICONST
X t221=(t222==t223)||eq1(t222,t223)?T:F; // EQ
X t220;
t1=restore();
if(t221!=F){
save(t1); // COND
X t226=lf[381]; // CONST
save(t226);
X t227=select1(TO_N(1),t1); // SELECT (simple)
X t228=___tl(t227); // REF: tl
t226=restore();
X t225=(t226==t228)||eq1(t226,t228)?T:F; // EQ
X t224;
t1=restore();
if(t225!=F){
X t230=TO_N(3); // ICONST
save(t230);
save(t1);
X t231=allocate(2); // CONS
save(t231);
t1=pick(2);
save(t1);
X t232=allocate(3); // CONS
save(t232);
t1=pick(2);
X t233=lf[90]; // CONST
t232=top();
S_DATA(t232)[0]=t233;
t233=pick(2);
X t234=___lex_3asourceinfo(t233); // REF: lex:sourceinfo
t232=top();
S_DATA(t232)[1]=t234;
t234=pick(2);
X t235=lf[54]; // CONST
t232=top();
S_DATA(t232)[2]=t235;
t232=restore();
restore();
t231=top();
S_DATA(t231)[0]=t232;
t232=pick(2);
save(t232);
X t236=allocate(3); // CONS
save(t236);
t232=pick(2);
X t237=EMPTY; // ICONST
t236=top();
S_DATA(t236)[0]=t237;
t237=pick(2);
X t238=select1(TO_N(2),t237); // SELECT (simple)
t236=top();
S_DATA(t236)[1]=t238;
t238=pick(2);
X t239=select1(TO_N(3),t238); // SELECT (simple)
t236=top();
S_DATA(t236)[2]=t239;
t236=restore();
restore();
t231=top();
S_DATA(t231)[1]=t236;
t231=restore();
restore();
X t240=___lex_3apush(t231); // REF: lex:push
t230=restore();
X t229=select1(t230,t240); // SELECT
t224=t229;
}else{
save(t1); // COND
save(t1);
X t244=TO_N(2); // ICONST
save(t244);
X t245=select1(TO_N(1),t1); // SELECT (simple)
t244=restore();
X t243=select1(t244,t245); // SELECT
t1=restore();
X t246=TO_N(93); // ICONST
X t242=(t243==t246)||eq1(t243,t246)?T:F; // EQ
X t241;
t1=restore();
if(t242!=F){
save(t1);
X t247=allocate(2); // CONS
save(t247);
t1=pick(2);
save(t1);
X t248=allocate(3); // CONS
save(t248);
t1=pick(2);
X t249=lf[90]; // CONST
t248=top();
S_DATA(t248)[0]=t249;
t249=pick(2);
X t250=___lex_3asourceinfo(t249); // REF: lex:sourceinfo
t248=top();
S_DATA(t248)[1]=t250;
t250=pick(2);
X t251=lf[84]; // CONST
t248=top();
S_DATA(t248)[2]=t251;
t248=restore();
restore();
t247=top();
S_DATA(t247)[0]=t248;
t248=pick(2);
save(t248);
X t252=allocate(3); // CONS
save(t252);
t248=pick(2);
X t253=select1(TO_N(1),t248); // SELECT (simple)
X t254=___tl(t253); // REF: tl
X t255=___tl(t254); // REF: tl
t252=top();
S_DATA(t252)[0]=t255;
t255=pick(2);
X t256=select1(TO_N(2),t255); // SELECT (simple)
t252=top();
S_DATA(t252)[1]=t256;
t256=pick(2);
X t257=select1(TO_N(3),t256); // SELECT (simple)
t252=top();
S_DATA(t252)[2]=t257;
t252=restore();
restore();
t247=top();
S_DATA(t247)[1]=t252;
t247=restore();
restore();
t241=t247;
}else{
save(t1); // COND
save(t1);
X t261=TO_N(2); // ICONST
save(t261);
X t262=select1(TO_N(1),t1); // SELECT (simple)
t261=restore();
X t260=select1(t261,t262); // SELECT
t1=restore();
X t263=TO_N(41); // ICONST
X t259=(t260==t263)||eq1(t260,t263)?T:F; // EQ
X t258;
t1=restore();
if(t259!=F){
save(t1);
X t264=allocate(2); // CONS
save(t264);
t1=pick(2);
save(t1);
X t265=allocate(3); // CONS
save(t265);
t1=pick(2);
X t266=lf[90]; // CONST
t265=top();
S_DATA(t265)[0]=t266;
t266=pick(2);
X t267=___lex_3asourceinfo(t266); // REF: lex:sourceinfo
t265=top();
S_DATA(t265)[1]=t267;
t267=pick(2);
X t268=lf[61]; // CONST
t265=top();
S_DATA(t265)[2]=t268;
t265=restore();
restore();
t264=top();
S_DATA(t264)[0]=t265;
t265=pick(2);
save(t265);
X t269=allocate(3); // CONS
save(t269);
t265=pick(2);
X t270=select1(TO_N(1),t265); // SELECT (simple)
X t271=___tl(t270); // REF: tl
X t272=___tl(t271); // REF: tl
t269=top();
S_DATA(t269)[0]=t272;
t272=pick(2);
X t273=select1(TO_N(2),t272); // SELECT (simple)
t269=top();
S_DATA(t269)[1]=t273;
t273=pick(2);
X t274=select1(TO_N(3),t273); // SELECT (simple)
t269=top();
S_DATA(t269)[2]=t274;
t269=restore();
restore();
t264=top();
S_DATA(t264)[1]=t269;
t264=restore();
restore();
t258=t264;
}else{
save(t1);
X t275=allocate(2); // CONS
save(t275);
t1=pick(2);
save(t1);
X t276=allocate(3); // CONS
save(t276);
t1=pick(2);
X t277=lf[90]; // CONST
t276=top();
S_DATA(t276)[0]=t277;
t277=pick(2);
X t278=___lex_3asourceinfo(t277); // REF: lex:sourceinfo
t276=top();
S_DATA(t276)[1]=t278;
t278=pick(2);
X t279=lf[54]; // CONST
t276=top();
S_DATA(t276)[2]=t279;
t276=restore();
restore();
t275=top();
S_DATA(t275)[0]=t276;
t276=pick(2);
save(t276);
X t280=allocate(3); // CONS
save(t280);
t276=pick(2);
X t281=select1(TO_N(1),t276); // SELECT (simple)
X t282=___tl(t281); // REF: tl
t280=top();
S_DATA(t280)[0]=t282;
t282=pick(2);
X t283=select1(TO_N(2),t282); // SELECT (simple)
t280=top();
S_DATA(t280)[1]=t283;
t283=pick(2);
X t284=select1(TO_N(3),t283); // SELECT (simple)
t280=top();
S_DATA(t280)[2]=t284;
t280=restore();
restore();
t275=top();
S_DATA(t275)[1]=t280;
t275=restore();
restore();
t258=t275;}
t241=t258;}
X t285=___lex_3apush(t241); // REF: lex:push
x=t285; // REF: lex:loop
goto loop; // tail call: lex:loop
X t286;
t224=t286;}
t220=t224;
}else{
save(t1); // COND
save(t1);
X t289=___lex_3anext(t1); // REF: lex:next
t1=restore();
X t290=TO_N(36); // ICONST
X t288=(t289==t290)||eq1(t289,t290)?T:F; // EQ
X t287;
t1=restore();
if(t288!=F){
X t291=___lex_3aadvance(t1); // REF: lex:advance
save(t291);
X t292=allocate(2); // CONS
save(t292);
t291=pick(2);
save(t291);
X t293=allocate(3); // CONS
save(t293);
t291=pick(2);
X t294=lf[86]; // CONST
t293=top();
S_DATA(t293)[0]=t294;
t294=pick(2);
X t295=___lex_3asourceinfo(t294); // REF: lex:sourceinfo
t293=top();
S_DATA(t293)[1]=t295;
t295=pick(2);
X t296=___lex_3anext(t295); // REF: lex:next
t293=top();
S_DATA(t293)[2]=t296;
t293=restore();
restore();
t292=top();
S_DATA(t292)[0]=t293;
t293=pick(2);
X t297=___lex_3aadvance(t293); // REF: lex:advance
t292=top();
S_DATA(t292)[1]=t297;
t292=restore();
restore();
X t298=___lex_3apush(t292); // REF: lex:push
x=t298; // REF: lex:loop
goto loop; // tail call: lex:loop
X t299;
t287=t299;
}else{
save(t1); // COND
save(t1);
X t302=___lex_3anext(t1); // REF: lex:next
t1=restore();
X t303=TO_N(96); // ICONST
X t301=(t302==t303)||eq1(t302,t303)?T:F; // EQ
X t300;
t1=restore();
if(t301!=F){
save(t1);
X t304=allocate(2); // CONS
save(t304);
t1=pick(2);
X t305=___lex_3aadvance(t1); // REF: lex:advance
t304=top();
S_DATA(t304)[0]=t305;
t305=pick(2);
X t306=TO_N(96); // ICONST
t304=top();
S_DATA(t304)[1]=t306;
t304=restore();
restore();
X t307=___lex_3ascanquoted(t304); // REF: lex:scanquoted
save(t307);
X t308=allocate(2); // CONS
save(t308);
t307=pick(2);
save(t307);
X t309=allocate(3); // CONS
save(t309);
t307=pick(2);
X t310=lf[87]; // CONST
t309=top();
S_DATA(t309)[0]=t310;
t310=pick(2);
X t311=select1(TO_N(1),t310); // SELECT (simple)
X t312=___lex_3asourceinfo(t311); // REF: lex:sourceinfo
t309=top();
S_DATA(t309)[1]=t312;
t312=pick(2);
X t313=select1(TO_N(2),t312); // SELECT (simple)
t309=top();
S_DATA(t309)[2]=t313;
t309=restore();
restore();
t308=top();
S_DATA(t308)[0]=t309;
t309=pick(2);
X t314=select1(TO_N(1),t309); // SELECT (simple)
t308=top();
S_DATA(t308)[1]=t314;
t308=restore();
restore();
X t315=___lex_3apush(t308); // REF: lex:push
x=t315; // REF: lex:loop
goto loop; // tail call: lex:loop
X t316;
t300=t316;
}else{
save(t1); // COND
save(t1);
X t319=___lex_3anext(t1); // REF: lex:next
t1=restore();
X t320=TO_N(34); // ICONST
X t318=(t319==t320)||eq1(t319,t320)?T:F; // EQ
X t317;
t1=restore();
if(t318!=F){
save(t1);
X t321=allocate(2); // CONS
save(t321);
t1=pick(2);
X t322=___lex_3aadvance(t1); // REF: lex:advance
t321=top();
S_DATA(t321)[0]=t322;
t322=pick(2);
X t323=TO_N(34); // ICONST
t321=top();
S_DATA(t321)[1]=t323;
t321=restore();
restore();
X t324=___lex_3ascanquoted(t321); // REF: lex:scanquoted
save(t324);
X t325=allocate(2); // CONS
save(t325);
t324=pick(2);
save(t324);
X t326=allocate(3); // CONS
save(t326);
t324=pick(2);
X t327=lf[92]; // CONST
t326=top();
S_DATA(t326)[0]=t327;
t327=pick(2);
X t328=select1(TO_N(1),t327); // SELECT (simple)
X t329=___lex_3asourceinfo(t328); // REF: lex:sourceinfo
t326=top();
S_DATA(t326)[1]=t329;
t329=pick(2);
X t330=select1(TO_N(2),t329); // SELECT (simple)
t326=top();
S_DATA(t326)[2]=t330;
t326=restore();
restore();
t325=top();
S_DATA(t325)[0]=t326;
t326=pick(2);
X t331=select1(TO_N(1),t326); // SELECT (simple)
t325=top();
S_DATA(t325)[1]=t331;
t325=restore();
restore();
X t332=___lex_3apush(t325); // REF: lex:push
x=t332; // REF: lex:loop
goto loop; // tail call: lex:loop
X t333;
t317=t333;
}else{
save(t1); // COND
X t335=___lex_3anext(t1); // REF: lex:next
save(t335); // OR
X t338=lf[35]; // CONST
save(t338);
X t339=___id(t335); // REF: id
t338=restore();
X t337=(t338==t339)||eq1(t338,t339)?T:F; // EQ
X t336=t337;
t335=restore();
if(t336==F){
save(t335); // COND
save(t335);
X t341=allocate(2); // CONS
save(t341);
t335=pick(2);
X t342=___id(t335); // REF: id
t341=top();
S_DATA(t341)[0]=t342;
t342=pick(2);
X t343=TO_N(97); // ICONST
t341=top();
S_DATA(t341)[1]=t343;
t341=restore();
restore();
X t344=___ge(t341); // REF: ge
X t340;
t335=restore();
if(t344!=F){
save(t335);
X t345=allocate(2); // CONS
save(t345);
t335=pick(2);
X t346=___id(t335); // REF: id
t345=top();
S_DATA(t345)[0]=t346;
t346=pick(2);
X t347=TO_N(122); // ICONST
t345=top();
S_DATA(t345)[1]=t347;
t345=restore();
restore();
X t348=___le(t345); // REF: le
t340=t348;
}else{
X t349=F; // ICONST
t340=t349;}
t336=t340;}
X t334;
t1=restore();
if(t336!=F){
save(t1);
X t350=allocate(2); // CONS
save(t350);
t1=pick(2);
X t351=lf[36]; // CONST
t350=top();
S_DATA(t350)[0]=t351;
t351=pick(2);
X t352=___id(t351); // REF: id
t350=top();
S_DATA(t350)[1]=t352;
t350=restore();
restore();
X t353=___lex_3ascan(t350); // REF: lex:scan
save(t353);
X t354=allocate(2); // CONS
save(t354);
t353=pick(2);
save(t353);
X t355=allocate(3); // CONS
save(t355);
t353=pick(2);
X t356=lf[80]; // CONST
t355=top();
S_DATA(t355)[0]=t356;
t356=pick(2);
X t357=select1(TO_N(1),t356); // SELECT (simple)
X t358=___lex_3asourceinfo(t357); // REF: lex:sourceinfo
t355=top();
S_DATA(t355)[1]=t358;
t358=pick(2);
X t359=select1(TO_N(2),t358); // SELECT (simple)
X t360=___toa(t359); // REF: toa
t355=top();
S_DATA(t355)[2]=t360;
t355=restore();
restore();
t354=top();
S_DATA(t354)[0]=t355;
t355=pick(2);
X t361=select1(TO_N(1),t355); // SELECT (simple)
t354=top();
S_DATA(t354)[1]=t361;
t354=restore();
restore();
X t362=___lex_3apush(t354); // REF: lex:push
x=t362; // REF: lex:loop
goto loop; // tail call: lex:loop
X t363;
t334=t363;
}else{
save(t1); // COND
save(t1); // COND
X t366=___lex_3anext(t1); // REF: lex:next
X t367=___lex_3adigit(t366); // REF: lex:digit
X t365;
t1=restore();
if(t367!=F){
X t368=T; // ICONST
t365=t368;
}else{
save(t1);
X t369=allocate(2); // CONS
save(t369);
t1=pick(2);
X t370=TO_N(45); // ICONST
t369=top();
S_DATA(t369)[0]=t370;
t370=pick(2);
X t371=___lex_3anext(t370); // REF: lex:next
t369=top();
S_DATA(t369)[1]=t371;
t369=restore();
restore();
save(t369); // COND
X t373=___eq(t369); // REF: eq
X t372;
t369=restore();
if(t373!=F){
X t374=T; // ICONST
t372=t374;
}else{
X t375=F; // ICONST
t372=t375;}
t365=t372;}
X t364;
t1=restore();
if(t365!=F){
save(t1);
X t376=allocate(2); // CONS
save(t376);
t1=pick(2);
X t377=lf[37]; // CONST
t376=top();
S_DATA(t376)[0]=t377;
t377=pick(2);
X t378=___id(t377); // REF: id
t376=top();
S_DATA(t376)[1]=t378;
t376=restore();
restore();
X t379=___lex_3ascan(t376); // REF: lex:scan
save(t379);
X t380=allocate(2); // CONS
save(t380);
t379=pick(2);
save(t379);
X t381=allocate(3); // CONS
save(t381);
t379=pick(2);
X t382=lf[86]; // CONST
t381=top();
S_DATA(t381)[0]=t382;
t382=pick(2);
X t383=select1(TO_N(1),t382); // SELECT (simple)
X t384=___lex_3asourceinfo(t383); // REF: lex:sourceinfo
t381=top();
S_DATA(t381)[1]=t384;
t384=pick(2);
X t385=select1(TO_N(2),t384); // SELECT (simple)
X t386=___ton(t385); // REF: ton
t381=top();
S_DATA(t381)[2]=t386;
t381=restore();
restore();
t380=top();
S_DATA(t380)[0]=t381;
t381=pick(2);
X t387=select1(TO_N(1),t381); // SELECT (simple)
t380=top();
S_DATA(t380)[1]=t387;
t380=restore();
restore();
X t388=___lex_3apush(t380); // REF: lex:push
x=t388; // REF: lex:loop
goto loop; // tail call: lex:loop
X t389;
t364=t389;
}else{
save(t1); // COND
X t391=___lex_3anext(t1); // REF: lex:next
save(t391); // COND
save(t391);
X t393=allocate(2); // CONS
save(t393);
t391=pick(2);
X t394=___id(t391); // REF: id
t393=top();
S_DATA(t393)[0]=t394;
t394=pick(2);
X t395=TO_N(65); // ICONST
t393=top();
S_DATA(t393)[1]=t395;
t393=restore();
restore();
X t396=___ge(t393); // REF: ge
X t392;
t391=restore();
if(t396!=F){
save(t391);
X t397=allocate(2); // CONS
save(t397);
t391=pick(2);
X t398=___id(t391); // REF: id
t397=top();
S_DATA(t397)[0]=t398;
t398=pick(2);
X t399=TO_N(90); // ICONST
t397=top();
S_DATA(t397)[1]=t399;
t397=restore();
restore();
X t400=___le(t397); // REF: le
t392=t400;
}else{
X t401=F; // ICONST
t392=t401;}
X t390;
t1=restore();
if(t392!=F){
save(t1);
X t402=allocate(2); // CONS
save(t402);
t1=pick(2);
X t403=lf[38]; // CONST
t402=top();
S_DATA(t402)[0]=t403;
t403=pick(2);
X t404=___id(t403); // REF: id
t402=top();
S_DATA(t402)[1]=t404;
t402=restore();
restore();
X t405=___lex_3ascan(t402); // REF: lex:scan
save(t405);
X t406=allocate(2); // CONS
save(t406);
t405=pick(2);
save(t405);
X t407=allocate(3); // CONS
save(t407);
t405=pick(2);
X t408=lf[87]; // CONST
t407=top();
S_DATA(t407)[0]=t408;
t408=pick(2);
X t409=select1(TO_N(1),t408); // SELECT (simple)
X t410=___lex_3asourceinfo(t409); // REF: lex:sourceinfo
t407=top();
S_DATA(t407)[1]=t410;
t410=pick(2);
X t411=select1(TO_N(2),t410); // SELECT (simple)
X t412=___toa(t411); // REF: toa
t407=top();
S_DATA(t407)[2]=t412;
t407=restore();
restore();
t406=top();
S_DATA(t406)[0]=t407;
t407=pick(2);
X t413=select1(TO_N(1),t407); // SELECT (simple)
t406=top();
S_DATA(t406)[1]=t413;
t406=restore();
restore();
X t414=___lex_3apush(t406); // REF: lex:push
x=t414; // REF: lex:loop
goto loop; // tail call: lex:loop
X t415;
t390=t415;
}else{
save(t1); // COND
X t417=___lex_3anext(t1); // REF: lex:next
save(t417);
X t418=allocate(2); // CONS
save(t418);
t417=pick(2);
X t419=___id(t417); // REF: id
t418=top();
S_DATA(t418)[0]=t419;
t419=pick(2);
X t420=lf[39]; // CONST
t418=top();
S_DATA(t418)[1]=t420;
t418=restore();
restore();
X t421=___index(t418); // REF: index
X t416;
t1=restore();
if(t421!=F){
save(t1);
X t422=allocate(2); // CONS
save(t422);
t1=pick(2);
save(t1);
X t423=allocate(3); // CONS
save(t423);
t1=pick(2);
X t424=lf[90]; // CONST
t423=top();
S_DATA(t423)[0]=t424;
t424=pick(2);
X t425=___lex_3asourceinfo(t424); // REF: lex:sourceinfo
t423=top();
S_DATA(t423)[1]=t425;
t425=pick(2);
save(t425);
X t426=allocate(1); // CONS
save(t426);
t425=pick(2);
X t427=___lex_3anext(t425); // REF: lex:next
t426=top();
S_DATA(t426)[0]=t427;
t426=restore();
restore();
X t428=___toa(t426); // REF: toa
t423=top();
S_DATA(t423)[2]=t428;
t423=restore();
restore();
t422=top();
S_DATA(t422)[0]=t423;
t423=pick(2);
save(t423);
X t429=allocate(3); // CONS
save(t429);
t423=pick(2);
X t430=select1(TO_N(1),t423); // SELECT (simple)
X t431=___tl(t430); // REF: tl
t429=top();
S_DATA(t429)[0]=t431;
t431=pick(2);
X t432=select1(TO_N(2),t431); // SELECT (simple)
t429=top();
S_DATA(t429)[1]=t432;
t432=pick(2);
X t433=select1(TO_N(3),t432); // SELECT (simple)
t429=top();
S_DATA(t429)[2]=t433;
t429=restore();
restore();
t422=top();
S_DATA(t422)[1]=t429;
t422=restore();
restore();
X t434=___lex_3apush(t422); // REF: lex:push
x=t434; // REF: lex:loop
goto loop; // tail call: lex:loop
X t435;
t416=t435;
}else{
save(t1);
X t436=allocate(2); // CONS
save(t436);
t1=pick(2);
save(t1);
X t437=allocate(3); // CONS
save(t437);
t1=pick(2);
X t438=lf[40]; // CONST
t437=top();
S_DATA(t437)[0]=t438;
t438=pick(2);
save(t438);
X t439=allocate(1); // CONS
save(t439);
t438=pick(2);
X t440=___lex_3anext(t438); // REF: lex:next
t439=top();
S_DATA(t439)[0]=t440;
t439=restore();
restore();
t437=top();
S_DATA(t437)[1]=t439;
t439=pick(2);
X t441=lf[351]; // CONST
t437=top();
S_DATA(t437)[2]=t441;
t437=restore();
restore();
X t442=___cat(t437); // REF: cat
t436=top();
S_DATA(t436)[0]=t442;
t442=pick(2);
X t443=___id(t442); // REF: id
t436=top();
S_DATA(t436)[1]=t443;
t436=restore();
restore();
X t444=___lex_3afail(t436); // REF: lex:fail
t416=t444;}
t390=t416;}
t364=t390;}
t334=t364;}
t317=t334;}
t300=t317;}
t287=t300;}
t220=t287;}
t170=t220;}
t120=t170;}
t78=t120;}
t7=t78;}
t2=t7;}
RETURN(t2);}
//---------------------------------------- lex:scan (lex.fp:104)
DEFINE(___lex_3ascan){
ENTRY;
loop:;
tracecall("lex.fp:104:  lex:scan");
save(x);
X t1=allocate(3); // CONS
save(t1);
x=pick(2);
X t2=select1(TO_N(2),x); // SELECT (simple)
t1=top();
S_DATA(t1)[0]=t2;
t2=pick(2);
X t3=select1(TO_N(1),t2); // SELECT (simple)
t1=top();
S_DATA(t1)[1]=t3;
t3=pick(2);
X t4=EMPTY; // ICONST
t1=top();
S_DATA(t1)[2]=t4;
t1=restore();
restore();
X t5=___lex_3ascan_27(t1); // REF: lex:scan'
save(t5);
X t6=allocate(2); // CONS
save(t6);
t5=pick(2);
X t7=select1(TO_N(1),t5); // SELECT (simple)
t6=top();
S_DATA(t6)[0]=t7;
t7=pick(2);
X t8=select1(TO_N(3),t7); // SELECT (simple)
t6=top();
S_DATA(t6)[1]=t8;
t6=restore();
restore();
RETURN(t6);}
//---------------------------------------- lex:scan' (lex.fp:105)
DEFINE(___lex_3ascan_27){
ENTRY;
loop:;
tracecall("lex.fp:105:  lex:scan'");
save(x); // COND
X t3=lf[381]; // CONST
save(t3);
X t5=TO_N(1); // ICONST
save(t5);
X t6=select1(TO_N(1),x); // SELECT (simple)
t5=restore();
X t4=select1(t5,t6); // SELECT
t3=restore();
X t2=(t3==t4)||eq1(t3,t4)?T:F; // EQ
X t1;
x=restore();
if(t2!=F){
X t7=___id(x); // REF: id
t1=t7;
}else{
save(x); // COND
save(x);
X t9=allocate(2); // CONS
save(t9);
x=pick(2);
X t10=select1(TO_N(1),x); // SELECT (simple)
X t11=___lex_3anext(t10); // REF: lex:next
t9=top();
S_DATA(t9)[0]=t11;
t11=pick(2);
X t12=select1(TO_N(2),t11); // SELECT (simple)
t9=top();
S_DATA(t9)[1]=t12;
t9=restore();
restore();
X t13=___index(t9); // REF: index
X t8;
x=restore();
if(t13!=F){
save(x);
X t14=allocate(3); // CONS
save(t14);
x=pick(2);
X t15=select1(TO_N(1),x); // SELECT (simple)
X t16=___lex_3aadvance(t15); // REF: lex:advance
t14=top();
S_DATA(t14)[0]=t16;
t16=pick(2);
X t17=select1(TO_N(2),t16); // SELECT (simple)
t14=top();
S_DATA(t14)[1]=t17;
t17=pick(2);
save(t17);
X t18=allocate(2); // CONS
save(t18);
t17=pick(2);
X t19=select1(TO_N(3),t17); // SELECT (simple)
t18=top();
S_DATA(t18)[0]=t19;
t19=pick(2);
X t20=select1(TO_N(1),t19); // SELECT (simple)
X t21=___lex_3anext(t20); // REF: lex:next
t18=top();
S_DATA(t18)[1]=t21;
t18=restore();
restore();
X t22=___ar(t18); // REF: ar
t14=top();
S_DATA(t14)[2]=t22;
t14=restore();
restore();
x=t14; // REF: lex:scan'
goto loop; // tail call: lex:scan'
X t23;
t8=t23;
}else{
X t24=___id(x); // REF: id
t8=t24;}
t1=t8;}
RETURN(t1);}
//---------------------------------------- lex:scanquoted (lex.fp:112)
DEFINE(___lex_3ascanquoted){
ENTRY;
loop:;
tracecall("lex.fp:112:  lex:scanquoted");
save(x);
X t1=allocate(3); // CONS
save(t1);
x=pick(2);
X t2=select1(TO_N(1),x); // SELECT (simple)
t1=top();
S_DATA(t1)[0]=t2;
t2=pick(2);
X t3=select1(TO_N(2),t2); // SELECT (simple)
t1=top();
S_DATA(t1)[1]=t3;
t3=pick(2);
X t4=EMPTY; // ICONST
t1=top();
S_DATA(t1)[2]=t4;
t1=restore();
restore();
X t5=___lex_3ascanquoted_27(t1); // REF: lex:scanquoted'
save(t5);
X t6=allocate(2); // CONS
save(t6);
t5=pick(2);
X t7=select1(TO_N(1),t5); // SELECT (simple)
t6=top();
S_DATA(t6)[0]=t7;
t7=pick(2);
X t8=select1(TO_N(2),t7); // SELECT (simple)
X t9=___toa(t8); // REF: toa
t6=top();
S_DATA(t6)[1]=t9;
t6=restore();
restore();
RETURN(t6);}
//---------------------------------------- lex:scanquoted' (lex.fp:113)
DEFINE(___lex_3ascanquoted_27){
ENTRY;
loop:;
tracecall("lex.fp:113:  lex:scanquoted'");
save(x); // COND
X t3=lf[381]; // CONST
save(t3);
X t5=TO_N(1); // ICONST
save(t5);
X t6=select1(TO_N(1),x); // SELECT (simple)
t5=restore();
X t4=select1(t5,t6); // SELECT
t3=restore();
X t2=(t3==t4)||eq1(t3,t4)?T:F; // EQ
X t1;
x=restore();
if(t2!=F){
save(x);
X t7=allocate(2); // CONS
save(t7);
x=pick(2);
X t8=lf[41]; // CONST
t7=top();
S_DATA(t7)[0]=t8;
t8=pick(2);
X t9=select1(TO_N(1),t8); // SELECT (simple)
t7=top();
S_DATA(t7)[1]=t9;
t7=restore();
restore();
X t10=___lex_3afail(t7); // REF: lex:fail
t1=t10;
}else{
save(x); // COND
save(x);
X t13=select1(TO_N(1),x); // SELECT (simple)
X t14=___lex_3anext(t13); // REF: lex:next
x=restore();
X t15=TO_N(92); // ICONST
X t12=(t14==t15)||eq1(t14,t15)?T:F; // EQ
X t11;
x=restore();
if(t12!=F){
save(x);
X t16=allocate(3); // CONS
save(t16);
x=pick(2);
X t17=select1(TO_N(1),x); // SELECT (simple)
X t18=___lex_3aadvance(t17); // REF: lex:advance
t16=top();
S_DATA(t16)[0]=t18;
t18=pick(2);
X t19=select1(TO_N(2),t18); // SELECT (simple)
t16=top();
S_DATA(t16)[1]=t19;
t19=pick(2);
X t20=select1(TO_N(3),t19); // SELECT (simple)
t16=top();
S_DATA(t16)[2]=t20;
t16=restore();
restore();
save(t16); // COND
X t23=lf[381]; // CONST
save(t23);
X t25=TO_N(1); // ICONST
save(t25);
X t26=select1(TO_N(1),t16); // SELECT (simple)
t25=restore();
X t24=select1(t25,t26); // SELECT
t23=restore();
X t22=(t23==t24)||eq1(t23,t24)?T:F; // EQ
X t21;
t16=restore();
if(t22!=F){
save(t16);
X t27=allocate(3); // CONS
save(t27);
t16=pick(2);
X t28=select1(TO_N(1),t16); // SELECT (simple)
t27=top();
S_DATA(t27)[0]=t28;
t28=pick(2);
X t29=select1(TO_N(2),t28); // SELECT (simple)
t27=top();
S_DATA(t27)[1]=t29;
t29=pick(2);
X t30=select1(TO_N(3),t29); // SELECT (simple)
t27=top();
S_DATA(t27)[2]=t30;
t27=restore();
restore();
t21=t27;
}else{
save(t16);
X t31=allocate(3); // CONS
save(t31);
t16=pick(2);
X t32=select1(TO_N(1),t16); // SELECT (simple)
X t33=___lex_3aadvance(t32); // REF: lex:advance
t31=top();
S_DATA(t31)[0]=t33;
t33=pick(2);
X t34=select1(TO_N(2),t33); // SELECT (simple)
t31=top();
S_DATA(t31)[1]=t34;
t34=pick(2);
save(t34);
X t35=allocate(2); // CONS
save(t35);
t34=pick(2);
X t36=select1(TO_N(3),t34); // SELECT (simple)
t35=top();
S_DATA(t35)[0]=t36;
t36=pick(2);
X t37=select1(TO_N(1),t36); // SELECT (simple)
X t38=___lex_3anext(t37); // REF: lex:next
save(t38); // COND
X t41=lf[42]; // CONST
save(t41);
X t42=___id(t38); // REF: id
t41=restore();
X t40=(t41==t42)||eq1(t41,t42)?T:F; // EQ
X t39;
t38=restore();
if(t40!=F){
X t43=TO_N(10); // ICONST
t39=t43;
}else{
save(t38); // COND
X t46=lf[43]; // CONST
save(t46);
X t47=___id(t38); // REF: id
t46=restore();
X t45=(t46==t47)||eq1(t46,t47)?T:F; // EQ
X t44;
t38=restore();
if(t45!=F){
X t48=TO_N(13); // ICONST
t44=t48;
}else{
save(t38); // COND
X t51=lf[44]; // CONST
save(t51);
X t52=___id(t38); // REF: id
t51=restore();
X t50=(t51==t52)||eq1(t51,t52)?T:F; // EQ
X t49;
t38=restore();
if(t50!=F){
X t53=TO_N(9); // ICONST
t49=t53;
}else{
X t54=___id(t38); // REF: id
t49=t54;}
t44=t49;}
t39=t44;}
t35=top();
S_DATA(t35)[1]=t39;
t35=restore();
restore();
X t55=___ar(t35); // REF: ar
t31=top();
S_DATA(t31)[2]=t55;
t31=restore();
restore();
t21=t31;}
x=t21; // REF: lex:scanquoted'
goto loop; // tail call: lex:scanquoted'
X t56;
t11=t56;
}else{
save(x); // COND
save(x);
X t59=select1(TO_N(1),x); // SELECT (simple)
X t60=___lex_3anext(t59); // REF: lex:next
x=restore();
save(t60);
X t61=select1(TO_N(2),x); // SELECT (simple)
t60=restore();
X t58=(t60==t61)||eq1(t60,t61)?T:F; // EQ
X t57;
x=restore();
if(t58!=F){
save(x);
X t62=allocate(2); // CONS
save(t62);
x=pick(2);
X t63=select1(TO_N(1),x); // SELECT (simple)
X t64=___lex_3aadvance(t63); // REF: lex:advance
t62=top();
S_DATA(t62)[0]=t64;
t64=pick(2);
X t65=select1(TO_N(3),t64); // SELECT (simple)
t62=top();
S_DATA(t62)[1]=t65;
t62=restore();
restore();
t57=t62;
}else{
save(x);
X t66=allocate(3); // CONS
save(t66);
x=pick(2);
X t67=select1(TO_N(1),x); // SELECT (simple)
X t68=___lex_3aadvance(t67); // REF: lex:advance
t66=top();
S_DATA(t66)[0]=t68;
t68=pick(2);
X t69=select1(TO_N(2),t68); // SELECT (simple)
t66=top();
S_DATA(t66)[1]=t69;
t69=pick(2);
save(t69);
X t70=allocate(2); // CONS
save(t70);
t69=pick(2);
X t71=select1(TO_N(3),t69); // SELECT (simple)
t70=top();
S_DATA(t70)[0]=t71;
t71=pick(2);
X t72=select1(TO_N(1),t71); // SELECT (simple)
X t73=___lex_3anext(t72); // REF: lex:next
t70=top();
S_DATA(t70)[1]=t73;
t70=restore();
restore();
X t74=___ar(t70); // REF: ar
t66=top();
S_DATA(t66)[2]=t74;
t66=restore();
restore();
x=t66; // REF: lex:scanquoted'
goto loop; // tail call: lex:scanquoted'
X t75;
t57=t75;}
t11=t57;}
t1=t11;}
RETURN(t1);}
//---------------------------------------- lex:next (lex.fp:134)
DEFINE(___lex_3anext){
ENTRY;
loop:;
tracecall("lex.fp:134:  lex:next");
save(x); // COND
X t3=lf[381]; // CONST
save(t3);
X t4=select1(TO_N(1),x); // SELECT (simple)
t3=restore();
X t2=(t3==t4)||eq1(t3,t4)?T:F; // EQ
X t1;
x=restore();
if(t2!=F){
save(x);
X t5=allocate(2); // CONS
save(t5);
x=pick(2);
X t6=lf[100]; // CONST
t5=top();
S_DATA(t5)[0]=t6;
t6=pick(2);
X t7=___id(t6); // REF: id
t5=top();
S_DATA(t5)[1]=t7;
t5=restore();
restore();
X t8=___lex_3afail(t5); // REF: lex:fail
t1=t8;
}else{
X t10=TO_N(1); // ICONST
save(t10);
X t11=select1(TO_N(1),x); // SELECT (simple)
t10=restore();
X t9=select1(t10,t11); // SELECT
t1=t9;}
RETURN(t1);}
//---------------------------------------- lex:skip (lex.fp:137)
DEFINE(___lex_3askip){
ENTRY;
loop:;
tracecall("lex.fp:137:  lex:skip");
save(x); // COND
X t3=lf[381]; // CONST
save(t3);
X t4=select1(TO_N(1),x); // SELECT (simple)
t3=restore();
X t2=(t3==t4)||eq1(t3,t4)?T:F; // EQ
X t1;
x=restore();
if(t2!=F){
X t5=___id(x); // REF: id
t1=t5;
}else{
save(x); // COND
X t7=___lex_3anext(x); // REF: lex:next
save(t7);
X t8=allocate(2); // CONS
save(t8);
t7=pick(2);
X t9=___id(t7); // REF: id
t8=top();
S_DATA(t8)[0]=t9;
t9=pick(2);
X t10=TO_N(32); // ICONST
t8=top();
S_DATA(t8)[1]=t10;
t8=restore();
restore();
X t11=___le(t8); // REF: le
X t6;
x=restore();
if(t11!=F){
X t12=___lex_3aadvance(x); // REF: lex:advance
x=t12; // REF: lex:skip
goto loop; // tail call: lex:skip
X t13;
t6=t13;
}else{
save(x); // COND
save(x);
X t16=___lex_3anext(x); // REF: lex:next
x=restore();
X t17=TO_N(37); // ICONST
X t15=(t16==t17)||eq1(t16,t17)?T:F; // EQ
X t14;
x=restore();
if(t15!=F){
X t18=___lex_3askip_5fcomment(x); // REF: lex:skip_comment
x=t18; // REF: lex:skip
goto loop; // tail call: lex:skip
X t19;
t14=t19;
}else{
X t20=___id(x); // REF: id
t14=t20;}
t6=t14;}
t1=t6;}
RETURN(t1);}
//---------------------------------------- lex:advance (lex.fp:143)
DEFINE(___lex_3aadvance){
ENTRY;
loop:;
tracecall("lex.fp:143:  lex:advance");
save(x); // COND
save(x);
X t3=___lex_3anext(x); // REF: lex:next
x=restore();
X t4=TO_N(10); // ICONST
X t2=(t3==t4)||eq1(t3,t4)?T:F; // EQ
X t1;
x=restore();
if(t2!=F){
save(x);
X t5=allocate(3); // CONS
save(t5);
x=pick(2);
X t6=select1(TO_N(1),x); // SELECT (simple)
X t7=___tl(t6); // REF: tl
t5=top();
S_DATA(t5)[0]=t7;
t7=pick(2);
save(t7);
X t8=allocate(2); // CONS
save(t8);
t7=pick(2);
X t10=TO_N(1); // ICONST
save(t10);
X t11=select1(TO_N(2),t7); // SELECT (simple)
t10=restore();
X t9=select1(t10,t11); // SELECT
save(t9);
X t12=allocate(2); // CONS
save(t12);
t9=pick(2);
X t13=___id(t9); // REF: id
t12=top();
S_DATA(t12)[0]=t13;
t13=pick(2);
X t14=TO_N(1); // ICONST
t12=top();
S_DATA(t12)[1]=t14;
t12=restore();
restore();
X t15=___add(t12); // REF: add
t8=top();
S_DATA(t8)[0]=t15;
t15=pick(2);
X t17=TO_N(2); // ICONST
save(t17);
X t18=select1(TO_N(2),t15); // SELECT (simple)
t17=restore();
X t16=select1(t17,t18); // SELECT
t8=top();
S_DATA(t8)[1]=t16;
t8=restore();
restore();
t5=top();
S_DATA(t5)[1]=t8;
t8=pick(2);
X t19=select1(TO_N(3),t8); // SELECT (simple)
t5=top();
S_DATA(t5)[2]=t19;
t5=restore();
restore();
t1=t5;
}else{
save(x);
X t20=allocate(3); // CONS
save(t20);
x=pick(2);
X t21=select1(TO_N(1),x); // SELECT (simple)
X t22=___tl(t21); // REF: tl
t20=top();
S_DATA(t20)[0]=t22;
t22=pick(2);
X t23=select1(TO_N(2),t22); // SELECT (simple)
t20=top();
S_DATA(t20)[1]=t23;
t23=pick(2);
X t24=select1(TO_N(3),t23); // SELECT (simple)
t20=top();
S_DATA(t20)[2]=t24;
t20=restore();
restore();
t1=t20;}
RETURN(t1);}
//---------------------------------------- lex:skip_comment (lex.fp:146)
DEFINE(___lex_3askip_5fcomment){
ENTRY;
loop:;
tracecall("lex.fp:146:  lex:skip_comment");
save(x); // COND
X t3=lf[381]; // CONST
save(t3);
X t4=select1(TO_N(1),x); // SELECT (simple)
t3=restore();
X t2=(t3==t4)||eq1(t3,t4)?T:F; // EQ
X t1;
x=restore();
if(t2!=F){
X t5=___id(x); // REF: id
t1=t5;
}else{
save(x); // COND
save(x);
X t8=___lex_3anext(x); // REF: lex:next
x=restore();
X t9=TO_N(10); // ICONST
X t7=(t8==t9)||eq1(t8,t9)?T:F; // EQ
X t6;
x=restore();
if(t7!=F){
X t10=___lex_3aadvance(x); // REF: lex:advance
t6=t10;
}else{
X t11=___lex_3aadvance(x); // REF: lex:advance
x=t11; // REF: lex:skip_comment
goto loop; // tail call: lex:skip_comment
X t12;
t6=t12;}
t1=t6;}
RETURN(t1);}
//---------------------------------------- lex:push (lex.fp:152)
DEFINE(___lex_3apush){
ENTRY;
loop:;
tracecall("lex.fp:152:  lex:push");
save(x);
X t1=allocate(3); // CONS
save(t1);
x=pick(2);
X t3=TO_N(1); // ICONST
save(t3);
X t4=select1(TO_N(2),x); // SELECT (simple)
t3=restore();
X t2=select1(t3,t4); // SELECT
t1=top();
S_DATA(t1)[0]=t2;
t2=pick(2);
X t6=TO_N(2); // ICONST
save(t6);
X t7=select1(TO_N(2),t2); // SELECT (simple)
t6=restore();
X t5=select1(t6,t7); // SELECT
t1=top();
S_DATA(t1)[1]=t5;
t5=pick(2);
save(t5);
X t8=allocate(2); // CONS
save(t8);
t5=pick(2);
X t10=TO_N(3); // ICONST
save(t10);
X t11=select1(TO_N(2),t5); // SELECT (simple)
t10=restore();
X t9=select1(t10,t11); // SELECT
t8=top();
S_DATA(t8)[0]=t9;
t9=pick(2);
X t12=select1(TO_N(1),t9); // SELECT (simple)
t8=top();
S_DATA(t8)[1]=t12;
t8=restore();
restore();
X t13=___ar(t8); // REF: ar
t1=top();
S_DATA(t1)[2]=t13;
t1=restore();
restore();
RETURN(t1);}
//---------------------------------------- lex:sourceinfo (lex.fp:156)
DEFINE(___lex_3asourceinfo){
ENTRY;
loop:;
tracecall("lex.fp:156:  lex:sourceinfo");
X t1=select1(TO_N(2),x); // SELECT (simple)
save(t1);
X t2=allocate(3); // CONS
save(t2);
t1=pick(2);
X t3=select1(TO_N(2),t1); // SELECT (simple)
X t4=___tos(t3); // REF: tos
t2=top();
S_DATA(t2)[0]=t4;
t4=pick(2);
X t5=lf[255]; // CONST
t2=top();
S_DATA(t2)[1]=t5;
t5=pick(2);
X t6=select1(TO_N(1),t5); // SELECT (simple)
X t7=___tos(t6); // REF: tos
t2=top();
S_DATA(t2)[2]=t7;
t2=restore();
restore();
X t8=___cat(t2); // REF: cat
X t9=___toa(t8); // REF: toa
RETURN(t9);}
//---------------------------------------- lex:digit (lex.fp:170)
DEFINE(___lex_3adigit){
ENTRY;
loop:;
tracecall("lex.fp:170:  lex:digit");
save(x); // COND
save(x);
X t2=allocate(2); // CONS
save(t2);
x=pick(2);
X t3=___id(x); // REF: id
t2=top();
S_DATA(t2)[0]=t3;
t3=pick(2);
X t4=TO_N(48); // ICONST
t2=top();
S_DATA(t2)[1]=t4;
t2=restore();
restore();
X t5=___ge(t2); // REF: ge
X t1;
x=restore();
if(t5!=F){
save(x);
X t6=allocate(2); // CONS
save(t6);
x=pick(2);
X t7=___id(x); // REF: id
t6=top();
S_DATA(t6)[0]=t7;
t7=pick(2);
X t8=TO_N(57); // ICONST
t6=top();
S_DATA(t6)[1]=t8;
t6=restore();
restore();
X t9=___le(t6); // REF: le
t1=t9;
}else{
X t10=F; // ICONST
t1=t10;}
RETURN(t1);}
//---------------------------------------- lex:fail (lex.fp:179)
DEFINE(___lex_3afail){
ENTRY;
loop:;
tracecall("lex.fp:179:  lex:fail");
save(x);
X t1=allocate(5); // CONS
save(t1);
x=pick(2);
X t3=TO_N(2); // ICONST
save(t3);
X t5=TO_N(2); // ICONST
save(t5);
X t6=select1(TO_N(2),x); // SELECT (simple)
t5=restore();
X t4=select1(t5,t6); // SELECT
t3=restore();
X t2=select1(t3,t4); // SELECT
X t7=___tos(t2); // REF: tos
t1=top();
S_DATA(t1)[0]=t7;
t7=pick(2);
X t8=lf[255]; // CONST
t1=top();
S_DATA(t1)[1]=t8;
t8=pick(2);
X t10=TO_N(1); // ICONST
save(t10);
X t12=TO_N(2); // ICONST
save(t12);
X t13=select1(TO_N(2),t8); // SELECT (simple)
t12=restore();
X t11=select1(t12,t13); // SELECT
t10=restore();
X t9=select1(t10,t11); // SELECT
X t14=___tos(t9); // REF: tos
t1=top();
S_DATA(t1)[2]=t14;
t14=pick(2);
X t15=lf[45]; // CONST
t1=top();
S_DATA(t1)[3]=t15;
t15=pick(2);
X t16=select1(TO_N(1),t15); // SELECT (simple)
t1=top();
S_DATA(t1)[4]=t16;
t1=restore();
restore();
X t17=___cat(t1); // REF: cat
X t18=___toa(t17); // REF: toa
X t19=___quit(t18); // REF: quit
RETURN(t19);}
//---------------------------------------- parse:nextval (parse.fp:37)
DEFINE(___parse_3anextval){
ENTRY;
loop:;
tracecall("parse.fp:37:  parse:nextval");
X t2=TO_N(3); // ICONST
save(t2);
X t4=TO_N(1); // ICONST
save(t4);
X t5=select1(TO_N(1),x); // SELECT (simple)
t4=restore();
X t3=select1(t4,t5); // SELECT
t2=restore();
X t1=select1(t2,t3); // SELECT
RETURN(t1);}
//---------------------------------------- parse:p_definition (parse.fp:51)
DEFINE(___parse_3ap_5fdefinition){
ENTRY;
loop:;
tracecall("parse.fp:51:  parse:p_definition");
save(x);
X t1=allocate(2); // CONS
save(t1);
x=pick(2);
X t2=___id(x); // REF: id
t1=top();
S_DATA(t1)[0]=t2;
t2=pick(2);
X t3=lf[80]; // CONST
t1=top();
S_DATA(t1)[1]=t3;
t1=restore();
restore();
X t4=___parse_3aexpect(t1); // REF: parse:expect
save(t4);
X t5=allocate(2); // CONS
save(t5);
t4=pick(2);
X t6=___parse_3anextval(t4); // REF: parse:nextval
t5=top();
S_DATA(t5)[0]=t6;
t6=pick(2);
X t7=___parse_3aadvance(t6); // REF: parse:advance
t5=top();
S_DATA(t5)[1]=t7;
t5=restore();
restore();
save(t5); // COND
save(t5);
X t9=allocate(3); // CONS
save(t9);
t5=pick(2);
X t10=select1(TO_N(2),t5); // SELECT (simple)
t9=top();
S_DATA(t9)[0]=t10;
t10=pick(2);
X t11=lf[90]; // CONST
t9=top();
S_DATA(t9)[1]=t11;
t11=pick(2);
X t12=lf[59]; // CONST
t9=top();
S_DATA(t9)[2]=t12;
t9=restore();
restore();
X t13=___parse_3amatchval(t9); // REF: parse:matchval
X t8;
t5=restore();
if(t13!=F){
save(t5);
X t14=allocate(2); // CONS
save(t14);
t5=pick(2);
X t15=select1(TO_N(1),t5); // SELECT (simple)
t14=top();
S_DATA(t14)[0]=t15;
t15=pick(2);
X t16=select1(TO_N(2),t15); // SELECT (simple)
X t17=___parse_3ap_5fvalue(t16); // REF: parse:p_value
t14=top();
S_DATA(t14)[1]=t17;
t14=restore();
restore();
save(t14);
X t18=allocate(3); // CONS
save(t18);
t14=pick(2);
X t19=select1(TO_N(1),t14); // SELECT (simple)
t18=top();
S_DATA(t18)[0]=t19;
t19=pick(2);
X t21=TO_N(1); // ICONST
save(t21);
X t22=select1(TO_N(2),t19); // SELECT (simple)
t21=restore();
X t20=select1(t21,t22); // SELECT
t18=top();
S_DATA(t18)[1]=t20;
t20=pick(2);
save(t20);
X t23=allocate(3); // CONS
save(t23);
t20=pick(2);
X t25=TO_N(2); // ICONST
save(t25);
X t26=select1(TO_N(2),t20); // SELECT (simple)
t25=restore();
X t24=select1(t25,t26); // SELECT
t23=top();
S_DATA(t23)[0]=t24;
t24=pick(2);
X t27=lf[90]; // CONST
t23=top();
S_DATA(t23)[1]=t27;
t27=pick(2);
X t28=lf[49]; // CONST
t23=top();
S_DATA(t23)[2]=t28;
t23=restore();
restore();
X t29=___parse_3aexpectval(t23); // REF: parse:expectval
X t30=___parse_3ap_5fseq(t29); // REF: parse:p_seq
t18=top();
S_DATA(t18)[2]=t30;
t18=restore();
restore();
save(t18);
X t31=allocate(2); // CONS
save(t31);
t18=pick(2);
save(t18);
X t32=allocate(2); // CONS
save(t32);
t18=pick(2);
X t33=select1(TO_N(1),t18); // SELECT (simple)
t32=top();
S_DATA(t32)[0]=t33;
t33=pick(2);
save(t33);
X t34=allocate(5); // CONS
save(t34);
t33=pick(2);
X t35=lf[287]; // CONST
t34=top();
S_DATA(t34)[0]=t35;
t35=pick(2);
X t37=TO_N(2); // ICONST
save(t37);
X t38=select1(TO_N(2),t35); // SELECT (simple)
t37=restore();
X t36=select1(t37,t38); // SELECT
t34=top();
S_DATA(t34)[1]=t36;
t36=pick(2);
X t39=select1(TO_N(2),t36); // SELECT (simple)
t34=top();
S_DATA(t34)[2]=t39;
t39=pick(2);
X t41=TO_N(1); // ICONST
save(t41);
X t42=select1(TO_N(3),t39); // SELECT (simple)
t41=restore();
X t40=select1(t41,t42); // SELECT
t34=top();
S_DATA(t34)[3]=t40;
t40=pick(2);
save(t40);
X t43=allocate(3); // CONS
save(t43);
t40=pick(2);
X t44=lf[293]; // CONST
t43=top();
S_DATA(t43)[0]=t44;
t44=pick(2);
X t46=TO_N(2); // ICONST
save(t46);
X t47=select1(TO_N(2),t44); // SELECT (simple)
t46=restore();
X t45=select1(t46,t47); // SELECT
t43=top();
S_DATA(t43)[1]=t45;
t45=pick(2);
X t48=lf[46]; // CONST
t43=top();
S_DATA(t43)[2]=t48;
t43=restore();
restore();
t34=top();
S_DATA(t34)[4]=t43;
t34=restore();
restore();
t32=top();
S_DATA(t32)[1]=t34;
t32=restore();
restore();
t31=top();
S_DATA(t31)[0]=t32;
t32=pick(2);
X t50=TO_N(2); // ICONST
save(t50);
X t51=select1(TO_N(3),t32); // SELECT (simple)
t50=restore();
X t49=select1(t50,t51); // SELECT
t31=top();
S_DATA(t31)[1]=t49;
t31=restore();
restore();
t8=t31;
}else{
save(t5);
X t52=allocate(2); // CONS
save(t52);
t5=pick(2);
X t53=select1(TO_N(1),t5); // SELECT (simple)
t52=top();
S_DATA(t52)[0]=t53;
t53=pick(2);
save(t53);
X t54=allocate(3); // CONS
save(t54);
t53=pick(2);
X t55=select1(TO_N(2),t53); // SELECT (simple)
t54=top();
S_DATA(t54)[0]=t55;
t55=pick(2);
X t56=lf[90]; // CONST
t54=top();
S_DATA(t54)[1]=t56;
t56=pick(2);
X t57=lf[49]; // CONST
t54=top();
S_DATA(t54)[2]=t57;
t54=restore();
restore();
X t58=___parse_3aexpectval(t54); // REF: parse:expectval
t52=top();
S_DATA(t52)[1]=t58;
t52=restore();
restore();
save(t52); // COND
save(t52);
X t60=allocate(3); // CONS
save(t60);
t52=pick(2);
X t61=select1(TO_N(2),t52); // SELECT (simple)
t60=top();
S_DATA(t60)[0]=t61;
t61=pick(2);
X t62=lf[90]; // CONST
t60=top();
S_DATA(t60)[1]=t62;
t62=pick(2);
X t63=lf[47]; // CONST
t60=top();
S_DATA(t60)[2]=t63;
t60=restore();
restore();
X t64=___parse_3amatchval(t60); // REF: parse:matchval
X t59;
t52=restore();
if(t64!=F){
save(t52);
X t65=allocate(2); // CONS
save(t65);
t52=pick(2);
X t66=select1(TO_N(1),t52); // SELECT (simple)
t65=top();
S_DATA(t65)[0]=t66;
t66=pick(2);
save(t66);
X t67=allocate(2); // CONS
save(t67);
t66=pick(2);
X t68=EMPTY; // ICONST
t67=top();
S_DATA(t67)[0]=t68;
t68=pick(2);
X t69=select1(TO_N(2),t68); // SELECT (simple)
X t70=___parse_3aadvance(t69); // REF: parse:advance
t67=top();
S_DATA(t67)[1]=t70;
t67=restore();
restore();
X t71=t67; // WHILE
for(;;){
save(t71);
save(t71);
X t72=allocate(3); // CONS
save(t72);
t71=pick(2);
X t73=select1(TO_N(2),t71); // SELECT (simple)
t72=top();
S_DATA(t72)[0]=t73;
t73=pick(2);
X t74=lf[90]; // CONST
t72=top();
S_DATA(t72)[1]=t74;
t74=pick(2);
X t75=lf[48]; // CONST
t72=top();
S_DATA(t72)[2]=t75;
t72=restore();
restore();
save(t72); // COND
X t77=___parse_3amatchval(t72); // REF: parse:matchval
X t76;
t72=restore();
if(t77!=F){
X t78=F; // ICONST
t76=t78;
}else{
X t79=T; // ICONST
t76=t79;}
t71=restore();
if(t76==F) break;
save(t71);
X t80=allocate(2); // CONS
save(t80);
t71=pick(2);
X t81=select1(TO_N(1),t71); // SELECT (simple)
t80=top();
S_DATA(t80)[0]=t81;
t81=pick(2);
X t82=select1(TO_N(2),t81); // SELECT (simple)
X t83=___parse_3ap_5fdefinition(t82); // REF: parse:p_definition
t80=top();
S_DATA(t80)[1]=t83;
t80=restore();
restore();
save(t80);
X t84=allocate(2); // CONS
save(t84);
t80=pick(2);
save(t80);
X t85=allocate(2); // CONS
save(t85);
t80=pick(2);
X t86=select1(TO_N(1),t80); // SELECT (simple)
t85=top();
S_DATA(t85)[0]=t86;
t86=pick(2);
X t88=TO_N(1); // ICONST
save(t88);
X t89=select1(TO_N(2),t86); // SELECT (simple)
t88=restore();
X t87=select1(t88,t89); // SELECT
t85=top();
S_DATA(t85)[1]=t87;
t85=restore();
restore();
X t90=___ar(t85); // REF: ar
t84=top();
S_DATA(t84)[0]=t90;
t90=pick(2);
X t92=TO_N(2); // ICONST
save(t92);
X t93=select1(TO_N(2),t90); // SELECT (simple)
t92=restore();
X t91=select1(t92,t93); // SELECT
t84=top();
S_DATA(t84)[1]=t91;
t84=restore();
restore();
t71=t84;}
save(t71);
X t94=allocate(2); // CONS
save(t94);
t71=pick(2);
X t95=select1(TO_N(1),t71); // SELECT (simple)
t94=top();
S_DATA(t94)[0]=t95;
t95=pick(2);
X t96=select1(TO_N(2),t95); // SELECT (simple)
X t97=___parse_3aadvance(t96); // REF: parse:advance
t94=top();
S_DATA(t94)[1]=t97;
t94=restore();
restore();
t65=top();
S_DATA(t65)[1]=t94;
t65=restore();
restore();
save(t65);
X t98=allocate(2); // CONS
save(t98);
t65=pick(2);
save(t65);
X t99=allocate(2); // CONS
save(t99);
t65=pick(2);
X t100=select1(TO_N(1),t65); // SELECT (simple)
t99=top();
S_DATA(t99)[0]=t100;
t100=pick(2);
X t102=TO_N(1); // ICONST
save(t102);
X t103=select1(TO_N(2),t100); // SELECT (simple)
t102=restore();
X t101=select1(t102,t103); // SELECT
t99=top();
S_DATA(t99)[1]=t101;
t99=restore();
restore();
X t104=___dl(t99); // REF: dl
int t106; // ALPHA
check_S(t104,"@");
int t107=S_LENGTH(t104);
save(t104);
X t105=allocate(t107);
save(t105);
for(t106=0;t106<t107;++t106){
X t105=S_DATA(pick(2))[t106];
save(t105);
X t108=allocate(3); // CONS
save(t108);
t105=pick(2);
X t110=TO_N(1); // ICONST
save(t110);
X t111=select1(TO_N(2),t105); // SELECT (simple)
t110=restore();
X t109=select1(t110,t111); // SELECT
t108=top();
S_DATA(t108)[0]=t109;
t109=pick(2);
save(t109);
X t112=allocate(2); // CONS
save(t112);
t109=pick(2);
X t113=select1(TO_N(1),t109); // SELECT (simple)
t112=top();
S_DATA(t112)[0]=t113;
t113=pick(2);
X t115=TO_N(1); // ICONST
save(t115);
X t116=select1(TO_N(2),t113); // SELECT (simple)
t115=restore();
X t114=select1(t115,t116); // SELECT
t112=top();
S_DATA(t112)[1]=t114;
t112=restore();
restore();
X t117=___parse_3aprefix(t112); // REF: parse:prefix
t108=top();
S_DATA(t108)[1]=t117;
t117=pick(2);
X t118=select1(TO_N(2),t117); // SELECT (simple)
t108=top();
S_DATA(t108)[2]=t118;
t108=restore();
restore();
S_DATA(top())[t106]=t108;}
t105=restore();
restore();
save(t105);
X t119=allocate(2); // CONS
save(t119);
t105=pick(2);
X t120=___id(t105); // REF: id
t119=top();
S_DATA(t119)[0]=t120;
t120=pick(2);
X t121=___id(t120); // REF: id
t119=top();
S_DATA(t119)[1]=t121;
t119=restore();
restore();
X t122=___dr(t119); // REF: dr
int t124; // ALPHA
check_S(t122,"@");
int t125=S_LENGTH(t122);
save(t122);
X t123=allocate(t125);
save(t123);
for(t124=0;t124<t125;++t124){
X t123=S_DATA(pick(2))[t124];
save(t123);
X t126=allocate(2); // CONS
save(t126);
t123=pick(2);
X t128=TO_N(2); // ICONST
save(t128);
X t129=select1(TO_N(1),t123); // SELECT (simple)
t128=restore();
X t127=select1(t128,t129); // SELECT
t126=top();
S_DATA(t126)[0]=t127;
t127=pick(2);
save(t127);
X t130=allocate(2); // CONS
save(t130);
t127=pick(2);
X t132=TO_N(2); // ICONST
save(t132);
X t134=TO_N(3); // ICONST
save(t134);
X t135=select1(TO_N(1),t127); // SELECT (simple)
t134=restore();
X t133=select1(t134,t135); // SELECT
t132=restore();
X t131=select1(t132,t133); // SELECT
t130=top();
S_DATA(t130)[0]=t131;
t131=pick(2);
X t136=select1(TO_N(2),t131); // SELECT (simple)
t130=top();
S_DATA(t130)[1]=t136;
t130=restore();
restore();
X t137=___parse_3arename_5frec(t130); // REF: parse:rename_rec
t126=top();
S_DATA(t126)[1]=t137;
t126=restore();
restore();
S_DATA(top())[t124]=t126;}
t123=restore();
restore();
t98=top();
S_DATA(t98)[0]=t123;
t123=pick(2);
X t139=TO_N(2); // ICONST
save(t139);
X t140=select1(TO_N(2),t123); // SELECT (simple)
t139=restore();
X t138=select1(t139,t140); // SELECT
t98=top();
S_DATA(t98)[1]=t138;
t98=restore();
restore();
save(t98);
X t141=allocate(2); // CONS
save(t141);
t98=pick(2);
X t142=F; // ICONST
t141=top();
S_DATA(t141)[0]=t142;
t142=pick(2);
save(t142);
X t143=allocate(2); // CONS
save(t143);
t142=pick(2);
X t145=TO_N(1); // ICONST
save(t145);
X t146=select1(TO_N(2),t142); // SELECT (simple)
t145=restore();
X t144=select1(t145,t146); // SELECT
t143=top();
S_DATA(t143)[0]=t144;
t144=pick(2);
save(t144);
X t147=allocate(2); // CONS
save(t147);
t144=pick(2);
X t149=TO_N(2); // ICONST
save(t149);
X t150=select1(TO_N(2),t144); // SELECT (simple)
t149=restore();
X t148=select1(t149,t150); // SELECT
t147=top();
S_DATA(t147)[0]=t148;
t148=pick(2);
X t151=select1(TO_N(1),t148); // SELECT (simple)
t147=top();
S_DATA(t147)[1]=t151;
t147=restore();
restore();
X t152=___cat(t147); // REF: cat
t143=top();
S_DATA(t143)[1]=t152;
t143=restore();
restore();
t141=top();
S_DATA(t141)[1]=t143;
t141=restore();
restore();
t59=t141;
}else{
save(t52);
X t153=allocate(2); // CONS
save(t153);
t52=pick(2);
X t154=select1(TO_N(1),t52); // SELECT (simple)
t153=top();
S_DATA(t153)[0]=t154;
t154=pick(2);
X t155=select1(TO_N(2),t154); // SELECT (simple)
X t156=___parse_3ap_5fseq(t155); // REF: parse:p_seq
t153=top();
S_DATA(t153)[1]=t156;
t153=restore();
restore();
save(t153);
X t157=allocate(2); // CONS
save(t157);
t153=pick(2);
save(t153);
X t158=allocate(2); // CONS
save(t158);
t153=pick(2);
X t159=select1(TO_N(1),t153); // SELECT (simple)
t158=top();
S_DATA(t158)[0]=t159;
t159=pick(2);
X t161=TO_N(1); // ICONST
save(t161);
X t162=select1(TO_N(2),t159); // SELECT (simple)
t161=restore();
X t160=select1(t161,t162); // SELECT
t158=top();
S_DATA(t158)[1]=t160;
t158=restore();
restore();
t157=top();
S_DATA(t157)[0]=t158;
t158=pick(2);
X t164=TO_N(2); // ICONST
save(t164);
X t165=select1(TO_N(2),t158); // SELECT (simple)
t164=restore();
X t163=select1(t164,t165); // SELECT
t157=top();
S_DATA(t157)[1]=t163;
t157=restore();
restore();
t59=t157;}
t8=t59;}
RETURN(t8);}
//---------------------------------------- parse:rename_rec (parse.fp:85)
DEFINE(___parse_3arename_5frec){
ENTRY;
loop:;
tracecall("parse.fp:85:  parse:rename_rec");
save(x); // COND
X t3=lf[293]; // CONST
save(t3);
X t5=TO_N(1); // ICONST
save(t5);
X t6=select1(TO_N(1),x); // SELECT (simple)
t5=restore();
X t4=select1(t5,t6); // SELECT
t3=restore();
X t2=(t3==t4)||eq1(t3,t4)?T:F; // EQ
X t1;
x=restore();
if(t2!=F){
save(x);
X t7=allocate(2); // CONS
save(t7);
x=pick(2);
X t8=select1(TO_N(1),x); // SELECT (simple)
t7=top();
S_DATA(t7)[0]=t8;
t8=pick(2);
save(t8);
X t9=allocate(2); // CONS
save(t9);
t8=pick(2);
X t11=TO_N(3); // ICONST
save(t11);
X t12=select1(TO_N(1),t8); // SELECT (simple)
t11=restore();
X t10=select1(t11,t12); // SELECT
t9=top();
S_DATA(t9)[0]=t10;
t10=pick(2);
X t13=select1(TO_N(2),t10); // SELECT (simple)
t9=top();
S_DATA(t9)[1]=t13;
t9=restore();
restore();
X t14=___fetch(t9); // REF: fetch
t7=top();
S_DATA(t7)[1]=t14;
t7=restore();
restore();
save(t7); // COND
X t16=select1(TO_N(2),t7); // SELECT (simple)
X t15;
t7=restore();
if(t16!=F){
save(t7);
X t17=allocate(3); // CONS
save(t17);
t7=pick(2);
X t19=TO_N(1); // ICONST
save(t19);
X t20=select1(TO_N(1),t7); // SELECT (simple)
t19=restore();
X t18=select1(t19,t20); // SELECT
t17=top();
S_DATA(t17)[0]=t18;
t18=pick(2);
X t22=TO_N(2); // ICONST
save(t22);
X t23=select1(TO_N(1),t18); // SELECT (simple)
t22=restore();
X t21=select1(t22,t23); // SELECT
t17=top();
S_DATA(t17)[1]=t21;
t21=pick(2);
X t24=select1(TO_N(2),t21); // SELECT (simple)
t17=top();
S_DATA(t17)[2]=t24;
t17=restore();
restore();
t15=t17;
}else{
X t25=select1(TO_N(1),t7); // SELECT (simple)
t15=t25;}
t1=t15;
}else{
save(x); // COND
X t27=select1(TO_N(1),x); // SELECT (simple)
X t28=___constnode(t27); // REF: constnode
X t26;
x=restore();
if(t28!=F){
X t29=select1(TO_N(1),x); // SELECT (simple)
t26=t29;
}else{
save(x);
X t30=allocate(2); // CONS
save(t30);
x=pick(2);
save(x);
X t31=allocate(2); // CONS
save(t31);
x=pick(2);
X t32=TO_N(2); // ICONST
t31=top();
S_DATA(t31)[0]=t32;
t32=pick(2);
X t33=select1(TO_N(1),t32); // SELECT (simple)
t31=top();
S_DATA(t31)[1]=t33;
t31=restore();
restore();
X t34=___take(t31); // REF: take
t30=top();
S_DATA(t30)[0]=t34;
t34=pick(2);
save(t34);
X t35=allocate(2); // CONS
save(t35);
t34=pick(2);
save(t34);
X t36=allocate(2); // CONS
save(t36);
t34=pick(2);
X t37=TO_N(2); // ICONST
t36=top();
S_DATA(t36)[0]=t37;
t37=pick(2);
X t38=select1(TO_N(1),t37); // SELECT (simple)
t36=top();
S_DATA(t36)[1]=t38;
t36=restore();
restore();
X t39=___drop(t36); // REF: drop
t35=top();
S_DATA(t35)[0]=t39;
t39=pick(2);
X t40=select1(TO_N(2),t39); // SELECT (simple)
t35=top();
S_DATA(t35)[1]=t40;
t35=restore();
restore();
X t41=___dr(t35); // REF: dr
int t43; // ALPHA
check_S(t41,"@");
int t44=S_LENGTH(t41);
save(t41);
X t42=allocate(t44);
save(t42);
for(t43=0;t43<t44;++t43){
X t42=S_DATA(pick(2))[t43];
X t45=___parse_3arename_5frec(t42); // REF: parse:rename_rec
S_DATA(top())[t43]=t45;}
t42=restore();
restore();
t30=top();
S_DATA(t30)[1]=t42;
t30=restore();
restore();
X t46=___cat(t30); // REF: cat
t26=t46;}
t1=t26;}
RETURN(t1);}
//---------------------------------------- parse:p_seq (parse.fp:94)
DEFINE(___parse_3ap_5fseq){
ENTRY;
loop:;
tracecall("parse.fp:94:  parse:p_seq");
X t1=___parse_3ap_5fexp0(x); // REF: parse:p_exp0
save(t1); // COND
save(t1);
X t3=allocate(3); // CONS
save(t3);
t1=pick(2);
X t4=select1(TO_N(2),t1); // SELECT (simple)
t3=top();
S_DATA(t3)[0]=t4;
t4=pick(2);
X t5=lf[90]; // CONST
t3=top();
S_DATA(t3)[1]=t5;
t5=pick(2);
X t6=lf[52]; // CONST
t3=top();
S_DATA(t3)[2]=t6;
t3=restore();
restore();
X t7=___parse_3amatchval(t3); // REF: parse:matchval
X t2;
t1=restore();
if(t7!=F){
save(t1);
X t8=allocate(3); // CONS
save(t8);
t1=pick(2);
X t9=select1(TO_N(1),t1); // SELECT (simple)
t8=top();
S_DATA(t8)[0]=t9;
t9=pick(2);
X t11=TO_N(2); // ICONST
save(t11);
X t13=TO_N(1); // ICONST
save(t13);
X t15=TO_N(1); // ICONST
save(t15);
X t16=select1(TO_N(2),t9); // SELECT (simple)
t15=restore();
X t14=select1(t15,t16); // SELECT
t13=restore();
X t12=select1(t13,t14); // SELECT
t11=restore();
X t10=select1(t11,t12); // SELECT
t8=top();
S_DATA(t8)[1]=t10;
t10=pick(2);
X t17=select1(TO_N(2),t10); // SELECT (simple)
X t18=___parse_3aadvance(t17); // REF: parse:advance
X t19=___parse_3ap_5fseq(t18); // REF: parse:p_seq
t8=top();
S_DATA(t8)[2]=t19;
t8=restore();
restore();
save(t8);
X t20=allocate(2); // CONS
save(t20);
t8=pick(2);
save(t8);
X t21=allocate(4); // CONS
save(t21);
t8=pick(2);
X t22=lf[309]; // CONST
t21=top();
S_DATA(t21)[0]=t22;
t22=pick(2);
X t23=select1(TO_N(2),t22); // SELECT (simple)
t21=top();
S_DATA(t21)[1]=t23;
t23=pick(2);
X t24=select1(TO_N(1),t23); // SELECT (simple)
t21=top();
S_DATA(t21)[2]=t24;
t24=pick(2);
X t26=TO_N(1); // ICONST
save(t26);
X t27=select1(TO_N(3),t24); // SELECT (simple)
t26=restore();
X t25=select1(t26,t27); // SELECT
t21=top();
S_DATA(t21)[3]=t25;
t21=restore();
restore();
t20=top();
S_DATA(t20)[0]=t21;
t21=pick(2);
X t29=TO_N(2); // ICONST
save(t29);
X t30=select1(TO_N(3),t21); // SELECT (simple)
t29=restore();
X t28=select1(t29,t30); // SELECT
t20=top();
S_DATA(t20)[1]=t28;
t20=restore();
restore();
t2=t20;
}else{
X t31=___id(t1); // REF: id
t2=t31;}
RETURN(t2);}
//---------------------------------------- parse:p_exp0 (parse.fp:100)
DEFINE(___parse_3ap_5fexp0){
ENTRY;
loop:;
tracecall("parse.fp:100:  parse:p_exp0");
X t1=___parse_3ap_5fconditional(x); // REF: parse:p_conditional
save(t1); // COND
save(t1);
X t3=allocate(3); // CONS
save(t3);
t1=pick(2);
X t4=select1(TO_N(2),t1); // SELECT (simple)
t3=top();
S_DATA(t3)[0]=t4;
t4=pick(2);
X t5=lf[90]; // CONST
t3=top();
S_DATA(t3)[1]=t5;
t5=pick(2);
X t6=lf[50]; // CONST
t3=top();
S_DATA(t3)[2]=t6;
t3=restore();
restore();
X t7=___parse_3amatchval(t3); // REF: parse:matchval
X t2;
t1=restore();
if(t7!=F){
save(t1);
X t8=allocate(3); // CONS
save(t8);
t1=pick(2);
X t9=select1(TO_N(1),t1); // SELECT (simple)
t8=top();
S_DATA(t8)[0]=t9;
t9=pick(2);
X t11=TO_N(2); // ICONST
save(t11);
X t13=TO_N(1); // ICONST
save(t13);
X t15=TO_N(1); // ICONST
save(t15);
X t16=select1(TO_N(2),t9); // SELECT (simple)
t15=restore();
X t14=select1(t15,t16); // SELECT
t13=restore();
X t12=select1(t13,t14); // SELECT
t11=restore();
X t10=select1(t11,t12); // SELECT
t8=top();
S_DATA(t8)[1]=t10;
t10=pick(2);
X t17=select1(TO_N(2),t10); // SELECT (simple)
X t18=___parse_3aadvance(t17); // REF: parse:advance
X t19=___parse_3ap_5fexp0(t18); // REF: parse:p_exp0
t8=top();
S_DATA(t8)[2]=t19;
t8=restore();
restore();
save(t8);
X t20=allocate(2); // CONS
save(t20);
t8=pick(2);
save(t8);
X t21=allocate(4); // CONS
save(t21);
t8=pick(2);
X t22=lf[320]; // CONST
t21=top();
S_DATA(t21)[0]=t22;
t22=pick(2);
X t23=select1(TO_N(2),t22); // SELECT (simple)
t21=top();
S_DATA(t21)[1]=t23;
t23=pick(2);
X t25=TO_N(1); // ICONST
save(t25);
X t26=select1(TO_N(3),t23); // SELECT (simple)
t25=restore();
X t24=select1(t25,t26); // SELECT
t21=top();
S_DATA(t21)[2]=t24;
t24=pick(2);
X t27=select1(TO_N(1),t24); // SELECT (simple)
t21=top();
S_DATA(t21)[3]=t27;
t21=restore();
restore();
t20=top();
S_DATA(t20)[0]=t21;
t21=pick(2);
X t29=TO_N(2); // ICONST
save(t29);
X t30=select1(TO_N(3),t21); // SELECT (simple)
t29=restore();
X t28=select1(t29,t30); // SELECT
t20=top();
S_DATA(t20)[1]=t28;
t20=restore();
restore();
t2=t20;
}else{
X t31=___id(t1); // REF: id
t2=t31;}
RETURN(t2);}
//---------------------------------------- parse:p_conditional (parse.fp:106)
DEFINE(___parse_3ap_5fconditional){
ENTRY;
loop:;
tracecall("parse.fp:106:  parse:p_conditional");
X t1=___parse_3ap_5ffexpression(x); // REF: parse:p_fexpression
X t2=___parse_3ap_5fbexp_27(t1); // REF: parse:p_bexp'
save(t2); // COND
save(t2);
X t4=allocate(3); // CONS
save(t4);
t2=pick(2);
X t5=select1(TO_N(2),t2); // SELECT (simple)
t4=top();
S_DATA(t4)[0]=t5;
t5=pick(2);
X t6=lf[90]; // CONST
t4=top();
S_DATA(t4)[1]=t6;
t6=pick(2);
X t7=lf[51]; // CONST
t4=top();
S_DATA(t4)[2]=t7;
t4=restore();
restore();
X t8=___parse_3amatchval(t4); // REF: parse:matchval
X t3;
t2=restore();
if(t8!=F){
save(t2);
X t9=allocate(2); // CONS
save(t9);
t2=pick(2);
save(t2);
X t10=allocate(2); // CONS
save(t10);
t2=pick(2);
X t12=TO_N(2); // ICONST
save(t12);
X t14=TO_N(1); // ICONST
save(t14);
X t16=TO_N(1); // ICONST
save(t16);
X t17=select1(TO_N(2),t2); // SELECT (simple)
t16=restore();
X t15=select1(t16,t17); // SELECT
t14=restore();
X t13=select1(t14,t15); // SELECT
t12=restore();
X t11=select1(t12,t13); // SELECT
t10=top();
S_DATA(t10)[0]=t11;
t11=pick(2);
X t18=select1(TO_N(1),t11); // SELECT (simple)
t10=top();
S_DATA(t10)[1]=t18;
t10=restore();
restore();
t9=top();
S_DATA(t9)[0]=t10;
t10=pick(2);
X t19=select1(TO_N(2),t10); // SELECT (simple)
X t20=___parse_3aadvance(t19); // REF: parse:advance
X t21=___parse_3ap_5ffexpression(t20); // REF: parse:p_fexpression
X t22=___parse_3ap_5fbexp_27(t21); // REF: parse:p_bexp'
t9=top();
S_DATA(t9)[1]=t22;
t9=restore();
restore();
save(t9); // COND
save(t9); // COND
X t26=lf[381]; // CONST
save(t26);
X t28=TO_N(1); // ICONST
save(t28);
X t30=TO_N(2); // ICONST
save(t30);
X t31=select1(TO_N(2),t9); // SELECT (simple)
t30=restore();
X t29=select1(t30,t31); // SELECT
t28=restore();
X t27=select1(t28,t29); // SELECT
t26=restore();
X t25=(t26==t27)||eq1(t26,t27)?T:F; // EQ
X t24;
t9=restore();
if(t25!=F){
X t32=F; // ICONST
t24=t32;
}else{
save(t9);
X t33=allocate(3); // CONS
save(t33);
t9=pick(2);
X t35=TO_N(2); // ICONST
save(t35);
X t36=select1(TO_N(2),t9); // SELECT (simple)
t35=restore();
X t34=select1(t35,t36); // SELECT
t33=top();
S_DATA(t33)[0]=t34;
t34=pick(2);
X t37=lf[90]; // CONST
t33=top();
S_DATA(t33)[1]=t37;
t37=pick(2);
X t38=lf[52]; // CONST
t33=top();
S_DATA(t33)[2]=t38;
t33=restore();
restore();
X t39=___parse_3amatchval(t33); // REF: parse:matchval
t24=t39;}
X t23;
t9=restore();
if(t24!=F){
save(t9);
X t40=allocate(3); // CONS
save(t40);
t9=pick(2);
X t41=select1(TO_N(1),t9); // SELECT (simple)
t40=top();
S_DATA(t40)[0]=t41;
t41=pick(2);
X t43=TO_N(1); // ICONST
save(t43);
X t44=select1(TO_N(2),t41); // SELECT (simple)
t43=restore();
X t42=select1(t43,t44); // SELECT
t40=top();
S_DATA(t40)[1]=t42;
t42=pick(2);
X t46=TO_N(2); // ICONST
save(t46);
X t47=select1(TO_N(2),t42); // SELECT (simple)
t46=restore();
X t45=select1(t46,t47); // SELECT
X t48=___parse_3aadvance(t45); // REF: parse:advance
t40=top();
S_DATA(t40)[2]=t48;
t40=restore();
restore();
save(t40);
X t49=allocate(3); // CONS
save(t49);
t40=pick(2);
X t50=select1(TO_N(1),t40); // SELECT (simple)
t49=top();
S_DATA(t49)[0]=t50;
t50=pick(2);
X t51=select1(TO_N(2),t50); // SELECT (simple)
t49=top();
S_DATA(t49)[1]=t51;
t51=pick(2);
X t52=select1(TO_N(3),t51); // SELECT (simple)
X t53=___parse_3ap_5fconditional(t52); // REF: parse:p_conditional
t49=top();
S_DATA(t49)[2]=t53;
t49=restore();
restore();
save(t49);
X t54=allocate(2); // CONS
save(t54);
t49=pick(2);
save(t49);
X t55=allocate(5); // CONS
save(t55);
t49=pick(2);
X t56=lf[287]; // CONST
t55=top();
S_DATA(t55)[0]=t56;
t56=pick(2);
X t58=TO_N(1); // ICONST
save(t58);
X t59=select1(TO_N(1),t56); // SELECT (simple)
t58=restore();
X t57=select1(t58,t59); // SELECT
t55=top();
S_DATA(t55)[1]=t57;
t57=pick(2);
X t61=TO_N(2); // ICONST
save(t61);
X t62=select1(TO_N(1),t57); // SELECT (simple)
t61=restore();
X t60=select1(t61,t62); // SELECT
t55=top();
S_DATA(t55)[2]=t60;
t60=pick(2);
X t63=select1(TO_N(2),t60); // SELECT (simple)
t55=top();
S_DATA(t55)[3]=t63;
t63=pick(2);
X t65=TO_N(1); // ICONST
save(t65);
X t66=select1(TO_N(3),t63); // SELECT (simple)
t65=restore();
X t64=select1(t65,t66); // SELECT
t55=top();
S_DATA(t55)[4]=t64;
t55=restore();
restore();
t54=top();
S_DATA(t54)[0]=t55;
t55=pick(2);
X t68=TO_N(2); // ICONST
save(t68);
X t69=select1(TO_N(3),t55); // SELECT (simple)
t68=restore();
X t67=select1(t68,t69); // SELECT
t54=top();
S_DATA(t54)[1]=t67;
t54=restore();
restore();
t23=t54;
}else{
save(t9);
X t70=allocate(2); // CONS
save(t70);
t9=pick(2);
save(t9);
X t71=allocate(5); // CONS
save(t71);
t9=pick(2);
X t72=lf[287]; // CONST
t71=top();
S_DATA(t71)[0]=t72;
t72=pick(2);
X t74=TO_N(1); // ICONST
save(t74);
X t75=select1(TO_N(1),t72); // SELECT (simple)
t74=restore();
X t73=select1(t74,t75); // SELECT
t71=top();
S_DATA(t71)[1]=t73;
t73=pick(2);
X t77=TO_N(2); // ICONST
save(t77);
X t78=select1(TO_N(1),t73); // SELECT (simple)
t77=restore();
X t76=select1(t77,t78); // SELECT
t71=top();
S_DATA(t71)[2]=t76;
t76=pick(2);
X t80=TO_N(1); // ICONST
save(t80);
X t81=select1(TO_N(2),t76); // SELECT (simple)
t80=restore();
X t79=select1(t80,t81); // SELECT
t71=top();
S_DATA(t71)[3]=t79;
t79=pick(2);
save(t79);
X t82=allocate(3); // CONS
save(t82);
t79=pick(2);
X t83=lf[293]; // CONST
t82=top();
S_DATA(t82)[0]=t83;
t83=pick(2);
X t85=TO_N(1); // ICONST
save(t85);
X t86=select1(TO_N(1),t83); // SELECT (simple)
t85=restore();
X t84=select1(t85,t86); // SELECT
t82=top();
S_DATA(t82)[1]=t84;
t84=pick(2);
X t87=lf[282]; // CONST
t82=top();
S_DATA(t82)[2]=t87;
t82=restore();
restore();
t71=top();
S_DATA(t71)[4]=t82;
t71=restore();
restore();
t70=top();
S_DATA(t70)[0]=t71;
t71=pick(2);
X t89=TO_N(2); // ICONST
save(t89);
X t90=select1(TO_N(2),t71); // SELECT (simple)
t89=restore();
X t88=select1(t89,t90); // SELECT
t70=top();
S_DATA(t70)[1]=t88;
t70=restore();
restore();
t23=t70;}
t3=t23;
}else{
X t91=___id(t2); // REF: id
t3=t91;}
RETURN(t3);}
//---------------------------------------- parse:p_bexpression (parse.fp:117)
DEFINE(___parse_3ap_5fbexpression){
ENTRY;
loop:;
tracecall("parse.fp:117:  parse:p_bexpression");
X t1=___parse_3ap_5fexpression(x); // REF: parse:p_expression
save(t1); // COND
save(t1);
X t3=allocate(3); // CONS
save(t3);
t1=pick(2);
X t4=select1(TO_N(2),t1); // SELECT (simple)
t3=top();
S_DATA(t3)[0]=t4;
t4=pick(2);
X t5=lf[90]; // CONST
t3=top();
S_DATA(t3)[1]=t5;
t5=pick(2);
X t6=lf[55]; // CONST
t3=top();
S_DATA(t3)[2]=t6;
t3=restore();
restore();
X t7=___parse_3amatchval(t3); // REF: parse:matchval
X t2;
t1=restore();
if(t7!=F){
save(t1);
X t8=allocate(3); // CONS
save(t8);
t1=pick(2);
X t9=select1(TO_N(1),t1); // SELECT (simple)
t8=top();
S_DATA(t8)[0]=t9;
t9=pick(2);
X t11=TO_N(2); // ICONST
save(t11);
X t13=TO_N(1); // ICONST
save(t13);
X t15=TO_N(1); // ICONST
save(t15);
X t16=select1(TO_N(2),t9); // SELECT (simple)
t15=restore();
X t14=select1(t15,t16); // SELECT
t13=restore();
X t12=select1(t13,t14); // SELECT
t11=restore();
X t10=select1(t11,t12); // SELECT
t8=top();
S_DATA(t8)[1]=t10;
t10=pick(2);
X t17=select1(TO_N(2),t10); // SELECT (simple)
X t18=___parse_3aadvance(t17); // REF: parse:advance
X t19=___parse_3ap_5ffexpression(t18); // REF: parse:p_fexpression
t8=top();
S_DATA(t8)[2]=t19;
t8=restore();
restore();
save(t8);
X t20=allocate(2); // CONS
save(t20);
t8=pick(2);
save(t8);
X t21=allocate(4); // CONS
save(t21);
t8=pick(2);
X t22=lf[320]; // CONST
t21=top();
S_DATA(t21)[0]=t22;
t22=pick(2);
X t23=select1(TO_N(2),t22); // SELECT (simple)
t21=top();
S_DATA(t21)[1]=t23;
t23=pick(2);
save(t23);
X t24=allocate(3); // CONS
save(t24);
t23=pick(2);
X t25=lf[293]; // CONST
t24=top();
S_DATA(t24)[0]=t25;
t25=pick(2);
X t26=select1(TO_N(2),t25); // SELECT (simple)
t24=top();
S_DATA(t24)[1]=t26;
t26=pick(2);
X t27=lf[294]; // CONST
t24=top();
S_DATA(t24)[2]=t27;
t24=restore();
restore();
t21=top();
S_DATA(t21)[2]=t24;
t24=pick(2);
save(t24);
X t28=allocate(4); // CONS
save(t28);
t24=pick(2);
X t29=lf[326]; // CONST
t28=top();
S_DATA(t28)[0]=t29;
t29=pick(2);
X t30=select1(TO_N(2),t29); // SELECT (simple)
t28=top();
S_DATA(t28)[1]=t30;
t30=pick(2);
X t31=select1(TO_N(1),t30); // SELECT (simple)
t28=top();
S_DATA(t28)[2]=t31;
t31=pick(2);
X t33=TO_N(1); // ICONST
save(t33);
X t34=select1(TO_N(3),t31); // SELECT (simple)
t33=restore();
X t32=select1(t33,t34); // SELECT
t28=top();
S_DATA(t28)[3]=t32;
t28=restore();
restore();
t21=top();
S_DATA(t21)[3]=t28;
t21=restore();
restore();
t20=top();
S_DATA(t20)[0]=t21;
t21=pick(2);
X t36=TO_N(2); // ICONST
save(t36);
X t37=select1(TO_N(3),t21); // SELECT (simple)
t36=restore();
X t35=select1(t36,t37); // SELECT
t20=top();
S_DATA(t20)[1]=t35;
t20=restore();
restore();
t2=t20;
}else{
X t38=___id(t1); // REF: id
t2=t38;}
save(t2); // COND
save(t2);
X t40=allocate(3); // CONS
save(t40);
t2=pick(2);
X t41=select1(TO_N(2),t2); // SELECT (simple)
t40=top();
S_DATA(t40)[0]=t41;
t41=pick(2);
X t42=lf[90]; // CONST
t40=top();
S_DATA(t40)[1]=t42;
t42=pick(2);
X t43=lf[53]; // CONST
t40=top();
S_DATA(t40)[2]=t43;
t40=restore();
restore();
X t44=___parse_3amatchval(t40); // REF: parse:matchval
X t39;
t2=restore();
if(t44!=F){
save(t2);
X t45=allocate(3); // CONS
save(t45);
t2=pick(2);
X t46=select1(TO_N(1),t2); // SELECT (simple)
t45=top();
S_DATA(t45)[0]=t46;
t46=pick(2);
X t48=TO_N(2); // ICONST
save(t48);
X t50=TO_N(1); // ICONST
save(t50);
X t52=TO_N(1); // ICONST
save(t52);
X t53=select1(TO_N(2),t46); // SELECT (simple)
t52=restore();
X t51=select1(t52,t53); // SELECT
t50=restore();
X t49=select1(t50,t51); // SELECT
t48=restore();
X t47=select1(t48,t49); // SELECT
t45=top();
S_DATA(t45)[1]=t47;
t47=pick(2);
X t54=select1(TO_N(2),t47); // SELECT (simple)
X t55=___parse_3aadvance(t54); // REF: parse:advance
X t56=___parse_3ap_5fbexpression(t55); // REF: parse:p_bexpression
t45=top();
S_DATA(t45)[2]=t56;
t45=restore();
restore();
save(t45);
X t57=allocate(2); // CONS
save(t57);
t45=pick(2);
save(t45);
X t58=allocate(5); // CONS
save(t58);
t45=pick(2);
X t59=lf[287]; // CONST
t58=top();
S_DATA(t58)[0]=t59;
t59=pick(2);
X t60=select1(TO_N(2),t59); // SELECT (simple)
t58=top();
S_DATA(t58)[1]=t60;
t60=pick(2);
X t61=select1(TO_N(1),t60); // SELECT (simple)
t58=top();
S_DATA(t58)[2]=t61;
t61=pick(2);
X t63=TO_N(1); // ICONST
save(t63);
X t64=select1(TO_N(3),t61); // SELECT (simple)
t63=restore();
X t62=select1(t63,t64); // SELECT
t58=top();
S_DATA(t58)[3]=t62;
t62=pick(2);
save(t62);
X t65=allocate(3); // CONS
save(t65);
t62=pick(2);
X t66=lf[291]; // CONST
t65=top();
S_DATA(t65)[0]=t66;
t66=pick(2);
X t67=select1(TO_N(2),t66); // SELECT (simple)
t65=top();
S_DATA(t65)[1]=t67;
t67=pick(2);
X t68=F; // ICONST
t65=top();
S_DATA(t65)[2]=t68;
t65=restore();
restore();
t58=top();
S_DATA(t58)[4]=t65;
t58=restore();
restore();
t57=top();
S_DATA(t57)[0]=t58;
t58=pick(2);
X t70=TO_N(2); // ICONST
save(t70);
X t71=select1(TO_N(3),t58); // SELECT (simple)
t70=restore();
X t69=select1(t70,t71); // SELECT
t57=top();
S_DATA(t57)[1]=t69;
t57=restore();
restore();
t39=t57;
}else{
save(t2); // COND
save(t2);
X t73=allocate(3); // CONS
save(t73);
t2=pick(2);
X t74=select1(TO_N(2),t2); // SELECT (simple)
t73=top();
S_DATA(t73)[0]=t74;
t74=pick(2);
X t75=lf[90]; // CONST
t73=top();
S_DATA(t73)[1]=t75;
t75=pick(2);
X t76=lf[54]; // CONST
t73=top();
S_DATA(t73)[2]=t76;
t73=restore();
restore();
X t77=___parse_3amatchval(t73); // REF: parse:matchval
X t72;
t2=restore();
if(t77!=F){
save(t2);
X t78=allocate(3); // CONS
save(t78);
t2=pick(2);
X t79=select1(TO_N(1),t2); // SELECT (simple)
t78=top();
S_DATA(t78)[0]=t79;
t79=pick(2);
X t81=TO_N(2); // ICONST
save(t81);
X t83=TO_N(1); // ICONST
save(t83);
X t85=TO_N(1); // ICONST
save(t85);
X t86=select1(TO_N(2),t79); // SELECT (simple)
t85=restore();
X t84=select1(t85,t86); // SELECT
t83=restore();
X t82=select1(t83,t84); // SELECT
t81=restore();
X t80=select1(t81,t82); // SELECT
t78=top();
S_DATA(t78)[1]=t80;
t80=pick(2);
X t87=select1(TO_N(2),t80); // SELECT (simple)
X t88=___parse_3aadvance(t87); // REF: parse:advance
X t89=___parse_3ap_5fbexpression(t88); // REF: parse:p_bexpression
t78=top();
S_DATA(t78)[2]=t89;
t78=restore();
restore();
save(t78);
X t90=allocate(2); // CONS
save(t90);
t78=pick(2);
save(t78);
X t91=allocate(4); // CONS
save(t91);
t78=pick(2);
X t92=lf[315]; // CONST
t91=top();
S_DATA(t91)[0]=t92;
t92=pick(2);
X t93=select1(TO_N(2),t92); // SELECT (simple)
t91=top();
S_DATA(t91)[1]=t93;
t93=pick(2);
X t94=select1(TO_N(1),t93); // SELECT (simple)
t91=top();
S_DATA(t91)[2]=t94;
t94=pick(2);
X t96=TO_N(1); // ICONST
save(t96);
X t97=select1(TO_N(3),t94); // SELECT (simple)
t96=restore();
X t95=select1(t96,t97); // SELECT
t91=top();
S_DATA(t91)[3]=t95;
t91=restore();
restore();
t90=top();
S_DATA(t90)[0]=t91;
t91=pick(2);
X t99=TO_N(2); // ICONST
save(t99);
X t100=select1(TO_N(3),t91); // SELECT (simple)
t99=restore();
X t98=select1(t99,t100); // SELECT
t90=top();
S_DATA(t90)[1]=t98;
t90=restore();
restore();
t72=t90;
}else{
X t101=___id(t2); // REF: id
t72=t101;}
t39=t72;}
RETURN(t39);}
//---------------------------------------- parse:p_bexp' (parse.fp:118)
DEFINE(___parse_3ap_5fbexp_27){
ENTRY;
loop:;
tracecall("parse.fp:118:  parse:p_bexp'");
save(x); // COND
save(x);
X t2=allocate(3); // CONS
save(t2);
x=pick(2);
X t3=select1(TO_N(2),x); // SELECT (simple)
t2=top();
S_DATA(t2)[0]=t3;
t3=pick(2);
X t4=lf[90]; // CONST
t2=top();
S_DATA(t2)[1]=t4;
t4=pick(2);
X t5=lf[53]; // CONST
t2=top();
S_DATA(t2)[2]=t5;
t2=restore();
restore();
X t6=___parse_3amatchval(t2); // REF: parse:matchval
X t1;
x=restore();
if(t6!=F){
save(x);
X t7=allocate(3); // CONS
save(t7);
x=pick(2);
X t8=select1(TO_N(1),x); // SELECT (simple)
t7=top();
S_DATA(t7)[0]=t8;
t8=pick(2);
X t10=TO_N(2); // ICONST
save(t10);
X t12=TO_N(1); // ICONST
save(t12);
X t14=TO_N(1); // ICONST
save(t14);
X t15=select1(TO_N(2),t8); // SELECT (simple)
t14=restore();
X t13=select1(t14,t15); // SELECT
t12=restore();
X t11=select1(t12,t13); // SELECT
t10=restore();
X t9=select1(t10,t11); // SELECT
t7=top();
S_DATA(t7)[1]=t9;
t9=pick(2);
X t16=select1(TO_N(2),t9); // SELECT (simple)
X t17=___parse_3aadvance(t16); // REF: parse:advance
X t18=___parse_3ap_5fbexpression(t17); // REF: parse:p_bexpression
t7=top();
S_DATA(t7)[2]=t18;
t7=restore();
restore();
save(t7);
X t19=allocate(2); // CONS
save(t19);
t7=pick(2);
save(t7);
X t20=allocate(5); // CONS
save(t20);
t7=pick(2);
X t21=lf[287]; // CONST
t20=top();
S_DATA(t20)[0]=t21;
t21=pick(2);
X t22=select1(TO_N(2),t21); // SELECT (simple)
t20=top();
S_DATA(t20)[1]=t22;
t22=pick(2);
X t23=select1(TO_N(1),t22); // SELECT (simple)
t20=top();
S_DATA(t20)[2]=t23;
t23=pick(2);
X t25=TO_N(1); // ICONST
save(t25);
X t26=select1(TO_N(3),t23); // SELECT (simple)
t25=restore();
X t24=select1(t25,t26); // SELECT
t20=top();
S_DATA(t20)[3]=t24;
t24=pick(2);
save(t24);
X t27=allocate(3); // CONS
save(t27);
t24=pick(2);
X t28=lf[291]; // CONST
t27=top();
S_DATA(t27)[0]=t28;
t28=pick(2);
X t29=select1(TO_N(2),t28); // SELECT (simple)
t27=top();
S_DATA(t27)[1]=t29;
t29=pick(2);
X t30=F; // ICONST
t27=top();
S_DATA(t27)[2]=t30;
t27=restore();
restore();
t20=top();
S_DATA(t20)[4]=t27;
t20=restore();
restore();
t19=top();
S_DATA(t19)[0]=t20;
t20=pick(2);
X t32=TO_N(2); // ICONST
save(t32);
X t33=select1(TO_N(3),t20); // SELECT (simple)
t32=restore();
X t31=select1(t32,t33); // SELECT
t19=top();
S_DATA(t19)[1]=t31;
t19=restore();
restore();
t1=t19;
}else{
save(x); // COND
save(x);
X t35=allocate(3); // CONS
save(t35);
x=pick(2);
X t36=select1(TO_N(2),x); // SELECT (simple)
t35=top();
S_DATA(t35)[0]=t36;
t36=pick(2);
X t37=lf[90]; // CONST
t35=top();
S_DATA(t35)[1]=t37;
t37=pick(2);
X t38=lf[54]; // CONST
t35=top();
S_DATA(t35)[2]=t38;
t35=restore();
restore();
X t39=___parse_3amatchval(t35); // REF: parse:matchval
X t34;
x=restore();
if(t39!=F){
save(x);
X t40=allocate(3); // CONS
save(t40);
x=pick(2);
X t41=select1(TO_N(1),x); // SELECT (simple)
t40=top();
S_DATA(t40)[0]=t41;
t41=pick(2);
X t43=TO_N(2); // ICONST
save(t43);
X t45=TO_N(1); // ICONST
save(t45);
X t47=TO_N(1); // ICONST
save(t47);
X t48=select1(TO_N(2),t41); // SELECT (simple)
t47=restore();
X t46=select1(t47,t48); // SELECT
t45=restore();
X t44=select1(t45,t46); // SELECT
t43=restore();
X t42=select1(t43,t44); // SELECT
t40=top();
S_DATA(t40)[1]=t42;
t42=pick(2);
X t49=select1(TO_N(2),t42); // SELECT (simple)
X t50=___parse_3aadvance(t49); // REF: parse:advance
X t51=___parse_3ap_5fbexpression(t50); // REF: parse:p_bexpression
t40=top();
S_DATA(t40)[2]=t51;
t40=restore();
restore();
save(t40);
X t52=allocate(2); // CONS
save(t52);
t40=pick(2);
save(t40);
X t53=allocate(4); // CONS
save(t53);
t40=pick(2);
X t54=lf[315]; // CONST
t53=top();
S_DATA(t53)[0]=t54;
t54=pick(2);
X t55=select1(TO_N(2),t54); // SELECT (simple)
t53=top();
S_DATA(t53)[1]=t55;
t55=pick(2);
X t56=select1(TO_N(1),t55); // SELECT (simple)
t53=top();
S_DATA(t53)[2]=t56;
t56=pick(2);
X t58=TO_N(1); // ICONST
save(t58);
X t59=select1(TO_N(3),t56); // SELECT (simple)
t58=restore();
X t57=select1(t58,t59); // SELECT
t53=top();
S_DATA(t53)[3]=t57;
t53=restore();
restore();
t52=top();
S_DATA(t52)[0]=t53;
t53=pick(2);
X t61=TO_N(2); // ICONST
save(t61);
X t62=select1(TO_N(3),t53); // SELECT (simple)
t61=restore();
X t60=select1(t61,t62); // SELECT
t52=top();
S_DATA(t52)[1]=t60;
t52=restore();
restore();
t34=t52;
}else{
X t63=___id(x); // REF: id
t34=t63;}
t1=t34;}
RETURN(t1);}
//---------------------------------------- parse:p_fexpression (parse.fp:126)
DEFINE(___parse_3ap_5ffexpression){
ENTRY;
loop:;
tracecall("parse.fp:126:  parse:p_fexpression");
X t1=___parse_3ap_5fvalue(x); // REF: parse:p_value
save(t1); // COND
save(t1);
X t3=allocate(3); // CONS
save(t3);
t1=pick(2);
X t4=select1(TO_N(2),t1); // SELECT (simple)
t3=top();
S_DATA(t3)[0]=t4;
t4=pick(2);
X t5=lf[90]; // CONST
t3=top();
S_DATA(t3)[1]=t5;
t5=pick(2);
X t6=lf[76]; // CONST
t3=top();
S_DATA(t3)[2]=t6;
t3=restore();
restore();
X t7=___parse_3amatchval(t3); // REF: parse:matchval
X t2;
t1=restore();
if(t7!=F){
save(t1);
X t8=allocate(3); // CONS
save(t8);
t1=pick(2);
X t9=select1(TO_N(1),t1); // SELECT (simple)
t8=top();
S_DATA(t8)[0]=t9;
t9=pick(2);
X t11=TO_N(2); // ICONST
save(t11);
X t13=TO_N(1); // ICONST
save(t13);
X t15=TO_N(1); // ICONST
save(t15);
X t16=select1(TO_N(2),t9); // SELECT (simple)
t15=restore();
X t14=select1(t15,t16); // SELECT
t13=restore();
X t12=select1(t13,t14); // SELECT
t11=restore();
X t10=select1(t11,t12); // SELECT
t8=top();
S_DATA(t8)[1]=t10;
t10=pick(2);
X t17=select1(TO_N(2),t10); // SELECT (simple)
X t18=___parse_3aadvance(t17); // REF: parse:advance
X t19=___parse_3ap_5fvalue(t18); // REF: parse:p_value
save(t19); // COND
save(t19);
X t21=allocate(3); // CONS
save(t21);
t19=pick(2);
X t22=select1(TO_N(2),t19); // SELECT (simple)
t21=top();
S_DATA(t21)[0]=t22;
t22=pick(2);
X t23=lf[90]; // CONST
t21=top();
S_DATA(t21)[1]=t23;
t23=pick(2);
X t24=lf[76]; // CONST
t21=top();
S_DATA(t21)[2]=t24;
t21=restore();
restore();
X t25=___parse_3amatchval(t21); // REF: parse:matchval
X t20;
t19=restore();
if(t25!=F){
save(t19);
X t26=allocate(3); // CONS
save(t26);
t19=pick(2);
X t27=select1(TO_N(1),t19); // SELECT (simple)
t26=top();
S_DATA(t26)[0]=t27;
t27=pick(2);
X t29=TO_N(2); // ICONST
save(t29);
X t31=TO_N(1); // ICONST
save(t31);
X t33=TO_N(1); // ICONST
save(t33);
X t34=select1(TO_N(2),t27); // SELECT (simple)
t33=restore();
X t32=select1(t33,t34); // SELECT
t31=restore();
X t30=select1(t31,t32); // SELECT
t29=restore();
X t28=select1(t29,t30); // SELECT
t26=top();
S_DATA(t26)[1]=t28;
t28=pick(2);
X t35=select1(TO_N(2),t28); // SELECT (simple)
X t36=___parse_3aadvance(t35); // REF: parse:advance
X t37=___parse_3ap_5fexpression(t36); // REF: parse:p_expression
t26=top();
S_DATA(t26)[2]=t37;
t26=restore();
restore();
save(t26);
X t38=allocate(2); // CONS
save(t38);
t26=pick(2);
save(t26);
X t39=allocate(4); // CONS
save(t39);
t26=pick(2);
X t40=lf[320]; // CONST
t39=top();
S_DATA(t39)[0]=t40;
t40=pick(2);
X t41=select1(TO_N(2),t40); // SELECT (simple)
t39=top();
S_DATA(t39)[1]=t41;
t41=pick(2);
X t42=select1(TO_N(1),t41); // SELECT (simple)
t39=top();
S_DATA(t39)[2]=t42;
t42=pick(2);
X t44=TO_N(1); // ICONST
save(t44);
X t45=select1(TO_N(3),t42); // SELECT (simple)
t44=restore();
X t43=select1(t44,t45); // SELECT
t39=top();
S_DATA(t39)[3]=t43;
t39=restore();
restore();
t38=top();
S_DATA(t38)[0]=t39;
t39=pick(2);
X t47=TO_N(2); // ICONST
save(t47);
X t48=select1(TO_N(3),t39); // SELECT (simple)
t47=restore();
X t46=select1(t47,t48); // SELECT
t38=top();
S_DATA(t38)[1]=t46;
t38=restore();
restore();
t20=t38;
}else{
X t49=___id(t19); // REF: id
t20=t49;}
t8=top();
S_DATA(t8)[2]=t20;
t8=restore();
restore();
save(t8);
X t50=allocate(2); // CONS
save(t50);
t8=pick(2);
save(t8);
X t51=allocate(4); // CONS
save(t51);
t8=pick(2);
X t52=lf[320]; // CONST
t51=top();
S_DATA(t51)[0]=t52;
t52=pick(2);
X t53=select1(TO_N(2),t52); // SELECT (simple)
t51=top();
S_DATA(t51)[1]=t53;
t53=pick(2);
X t54=select1(TO_N(1),t53); // SELECT (simple)
t51=top();
S_DATA(t51)[2]=t54;
t54=pick(2);
X t56=TO_N(1); // ICONST
save(t56);
X t57=select1(TO_N(3),t54); // SELECT (simple)
t56=restore();
X t55=select1(t56,t57); // SELECT
t51=top();
S_DATA(t51)[3]=t55;
t51=restore();
restore();
t50=top();
S_DATA(t50)[0]=t51;
t51=pick(2);
X t59=TO_N(2); // ICONST
save(t59);
X t60=select1(TO_N(3),t51); // SELECT (simple)
t59=restore();
X t58=select1(t59,t60); // SELECT
t50=top();
S_DATA(t50)[1]=t58;
t50=restore();
restore();
t2=t50;
}else{
X t61=___id(t1); // REF: id
t2=t61;}
save(t2); // COND
save(t2);
X t63=allocate(3); // CONS
save(t63);
t2=pick(2);
X t64=select1(TO_N(2),t2); // SELECT (simple)
t63=top();
S_DATA(t63)[0]=t64;
t64=pick(2);
X t65=lf[90]; // CONST
t63=top();
S_DATA(t63)[1]=t65;
t65=pick(2);
X t66=lf[55]; // CONST
t63=top();
S_DATA(t63)[2]=t66;
t63=restore();
restore();
X t67=___parse_3amatchval(t63); // REF: parse:matchval
X t62;
t2=restore();
if(t67!=F){
save(t2);
X t68=allocate(3); // CONS
save(t68);
t2=pick(2);
X t69=select1(TO_N(1),t2); // SELECT (simple)
t68=top();
S_DATA(t68)[0]=t69;
t69=pick(2);
X t71=TO_N(2); // ICONST
save(t71);
X t73=TO_N(1); // ICONST
save(t73);
X t75=TO_N(1); // ICONST
save(t75);
X t76=select1(TO_N(2),t69); // SELECT (simple)
t75=restore();
X t74=select1(t75,t76); // SELECT
t73=restore();
X t72=select1(t73,t74); // SELECT
t71=restore();
X t70=select1(t71,t72); // SELECT
t68=top();
S_DATA(t68)[1]=t70;
t70=pick(2);
X t77=select1(TO_N(2),t70); // SELECT (simple)
X t78=___parse_3aadvance(t77); // REF: parse:advance
X t79=___parse_3ap_5ffexpression(t78); // REF: parse:p_fexpression
t68=top();
S_DATA(t68)[2]=t79;
t68=restore();
restore();
save(t68);
X t80=allocate(2); // CONS
save(t80);
t68=pick(2);
save(t68);
X t81=allocate(4); // CONS
save(t81);
t68=pick(2);
X t82=lf[320]; // CONST
t81=top();
S_DATA(t81)[0]=t82;
t82=pick(2);
X t83=select1(TO_N(2),t82); // SELECT (simple)
t81=top();
S_DATA(t81)[1]=t83;
t83=pick(2);
save(t83);
X t84=allocate(3); // CONS
save(t84);
t83=pick(2);
X t85=lf[293]; // CONST
t84=top();
S_DATA(t84)[0]=t85;
t85=pick(2);
X t86=select1(TO_N(2),t85); // SELECT (simple)
t84=top();
S_DATA(t84)[1]=t86;
t86=pick(2);
X t87=lf[294]; // CONST
t84=top();
S_DATA(t84)[2]=t87;
t84=restore();
restore();
t81=top();
S_DATA(t81)[2]=t84;
t84=pick(2);
save(t84);
X t88=allocate(4); // CONS
save(t88);
t84=pick(2);
X t89=lf[326]; // CONST
t88=top();
S_DATA(t88)[0]=t89;
t89=pick(2);
X t90=select1(TO_N(2),t89); // SELECT (simple)
t88=top();
S_DATA(t88)[1]=t90;
t90=pick(2);
X t91=select1(TO_N(1),t90); // SELECT (simple)
t88=top();
S_DATA(t88)[2]=t91;
t91=pick(2);
X t93=TO_N(1); // ICONST
save(t93);
X t94=select1(TO_N(3),t91); // SELECT (simple)
t93=restore();
X t92=select1(t93,t94); // SELECT
t88=top();
S_DATA(t88)[3]=t92;
t88=restore();
restore();
t81=top();
S_DATA(t81)[3]=t88;
t81=restore();
restore();
t80=top();
S_DATA(t80)[0]=t81;
t81=pick(2);
X t96=TO_N(2); // ICONST
save(t96);
X t97=select1(TO_N(3),t81); // SELECT (simple)
t96=restore();
X t95=select1(t96,t97); // SELECT
t80=top();
S_DATA(t80)[1]=t95;
t80=restore();
restore();
t62=t80;
}else{
X t98=___id(t2); // REF: id
t62=t98;}
RETURN(t62);}
//---------------------------------------- parse:p_expression (parse.fp:132)
DEFINE(___parse_3ap_5fexpression){
ENTRY;
loop:;
tracecall("parse.fp:132:  parse:p_expression");
X t1=___parse_3ap_5fvalue(x); // REF: parse:p_value
save(t1); // COND
save(t1);
X t3=allocate(3); // CONS
save(t3);
t1=pick(2);
X t4=select1(TO_N(2),t1); // SELECT (simple)
t3=top();
S_DATA(t3)[0]=t4;
t4=pick(2);
X t5=lf[90]; // CONST
t3=top();
S_DATA(t3)[1]=t5;
t5=pick(2);
X t6=lf[76]; // CONST
t3=top();
S_DATA(t3)[2]=t6;
t3=restore();
restore();
X t7=___parse_3amatchval(t3); // REF: parse:matchval
X t2;
t1=restore();
if(t7!=F){
save(t1);
X t8=allocate(3); // CONS
save(t8);
t1=pick(2);
X t9=select1(TO_N(1),t1); // SELECT (simple)
t8=top();
S_DATA(t8)[0]=t9;
t9=pick(2);
X t11=TO_N(2); // ICONST
save(t11);
X t13=TO_N(1); // ICONST
save(t13);
X t15=TO_N(1); // ICONST
save(t15);
X t16=select1(TO_N(2),t9); // SELECT (simple)
t15=restore();
X t14=select1(t15,t16); // SELECT
t13=restore();
X t12=select1(t13,t14); // SELECT
t11=restore();
X t10=select1(t11,t12); // SELECT
t8=top();
S_DATA(t8)[1]=t10;
t10=pick(2);
X t17=select1(TO_N(2),t10); // SELECT (simple)
X t18=___parse_3aadvance(t17); // REF: parse:advance
X t19=___parse_3ap_5fexpression(t18); // REF: parse:p_expression
t8=top();
S_DATA(t8)[2]=t19;
t8=restore();
restore();
save(t8);
X t20=allocate(2); // CONS
save(t20);
t8=pick(2);
save(t8);
X t21=allocate(4); // CONS
save(t21);
t8=pick(2);
X t22=lf[320]; // CONST
t21=top();
S_DATA(t21)[0]=t22;
t22=pick(2);
X t23=select1(TO_N(2),t22); // SELECT (simple)
t21=top();
S_DATA(t21)[1]=t23;
t23=pick(2);
X t24=select1(TO_N(1),t23); // SELECT (simple)
t21=top();
S_DATA(t21)[2]=t24;
t24=pick(2);
X t26=TO_N(1); // ICONST
save(t26);
X t27=select1(TO_N(3),t24); // SELECT (simple)
t26=restore();
X t25=select1(t26,t27); // SELECT
t21=top();
S_DATA(t21)[3]=t25;
t21=restore();
restore();
t20=top();
S_DATA(t20)[0]=t21;
t21=pick(2);
X t29=TO_N(2); // ICONST
save(t29);
X t30=select1(TO_N(3),t21); // SELECT (simple)
t29=restore();
X t28=select1(t29,t30); // SELECT
t20=top();
S_DATA(t20)[1]=t28;
t20=restore();
restore();
t2=t20;
}else{
X t31=___id(t1); // REF: id
t2=t31;}
RETURN(t2);}
//---------------------------------------- parse:p_value (parse.fp:138)
DEFINE(___parse_3ap_5fvalue){
ENTRY;
loop:;
tracecall("parse.fp:138:  parse:p_value");
save(x); // COND
save(x);
X t2=allocate(3); // CONS
save(t2);
x=pick(2);
X t3=___id(x); // REF: id
t2=top();
S_DATA(t2)[0]=t3;
t3=pick(2);
X t4=lf[90]; // CONST
t2=top();
S_DATA(t2)[1]=t4;
t4=pick(2);
X t5=lf[56]; // CONST
t2=top();
S_DATA(t2)[2]=t5;
t2=restore();
restore();
X t6=___parse_3amatchval(t2); // REF: parse:matchval
X t1;
x=restore();
if(t6!=F){
X t7=___parse_3aadvance(x); // REF: parse:advance
X t8=___parse_3ap_5fseq(t7); // REF: parse:p_seq
save(t8); // COND
save(t8);
X t10=allocate(3); // CONS
save(t10);
t8=pick(2);
X t11=select1(TO_N(2),t8); // SELECT (simple)
t10=top();
S_DATA(t10)[0]=t11;
t11=pick(2);
X t12=lf[90]; // CONST
t10=top();
S_DATA(t10)[1]=t12;
t12=pick(2);
X t13=lf[57]; // CONST
t10=top();
S_DATA(t10)[2]=t13;
t10=restore();
restore();
X t14=___parse_3amatchval(t10); // REF: parse:matchval
X t9;
t8=restore();
if(t14!=F){
save(t8);
X t15=allocate(2); // CONS
save(t15);
t8=pick(2);
X t16=select1(TO_N(1),t8); // SELECT (simple)
t15=top();
S_DATA(t15)[0]=t16;
t16=pick(2);
X t17=select1(TO_N(2),t16); // SELECT (simple)
X t18=___parse_3aadvance(t17); // REF: parse:advance
t15=top();
S_DATA(t15)[1]=t18;
t15=restore();
restore();
t9=t15;
}else{
save(t8); // COND
X t21=lf[381]; // CONST
save(t21);
X t23=TO_N(1); // ICONST
save(t23);
X t24=select1(TO_N(2),t8); // SELECT (simple)
t23=restore();
X t22=select1(t23,t24); // SELECT
t21=restore();
X t20=(t21==t22)||eq1(t21,t22)?T:F; // EQ
X t19;
t8=restore();
if(t20!=F){
save(t8);
X t25=allocate(2); // CONS
save(t25);
t8=pick(2);
X t26=lf[100]; // CONST
t25=top();
S_DATA(t25)[0]=t26;
t26=pick(2);
X t27=select1(TO_N(2),t26); // SELECT (simple)
t25=top();
S_DATA(t25)[1]=t27;
t25=restore();
restore();
X t28=___parse_3afail(t25); // REF: parse:fail
t19=t28;
}else{
save(t8);
X t29=allocate(3); // CONS
save(t29);
t8=pick(2);
X t31=TO_N(2); // ICONST
save(t31);
X t33=TO_N(1); // ICONST
save(t33);
X t35=TO_N(1); // ICONST
save(t35);
X t36=select1(TO_N(2),t8); // SELECT (simple)
t35=restore();
X t34=select1(t35,t36); // SELECT
t33=restore();
X t32=select1(t33,t34); // SELECT
t31=restore();
X t30=select1(t31,t32); // SELECT
t29=top();
S_DATA(t29)[0]=t30;
t30=pick(2);
save(t30);
X t37=allocate(1); // CONS
save(t37);
t30=pick(2);
X t38=select1(TO_N(1),t30); // SELECT (simple)
t37=top();
S_DATA(t37)[0]=t38;
t37=restore();
restore();
t29=top();
S_DATA(t29)[1]=t37;
t37=pick(2);
X t39=select1(TO_N(2),t37); // SELECT (simple)
t29=top();
S_DATA(t29)[2]=t39;
t29=restore();
restore();
X t40=t29; // WHILE
for(;;){
save(t40);
save(t40);
X t41=allocate(3); // CONS
save(t41);
t40=pick(2);
X t42=select1(TO_N(3),t40); // SELECT (simple)
t41=top();
S_DATA(t41)[0]=t42;
t42=pick(2);
X t43=lf[90]; // CONST
t41=top();
S_DATA(t41)[1]=t43;
t43=pick(2);
X t44=lf[57]; // CONST
t41=top();
S_DATA(t41)[2]=t44;
t41=restore();
restore();
save(t41); // COND
X t46=___parse_3amatchval(t41); // REF: parse:matchval
X t45;
t41=restore();
if(t46!=F){
X t47=F; // ICONST
t45=t47;
}else{
X t48=T; // ICONST
t45=t48;}
t40=restore();
if(t45==F) break;
save(t40);
X t49=allocate(3); // CONS
save(t49);
t40=pick(2);
X t50=select1(TO_N(1),t40); // SELECT (simple)
t49=top();
S_DATA(t49)[0]=t50;
t50=pick(2);
X t51=select1(TO_N(2),t50); // SELECT (simple)
t49=top();
S_DATA(t49)[1]=t51;
t51=pick(2);
X t52=select1(TO_N(3),t51); // SELECT (simple)
X t53=___parse_3ap_5fseq(t52); // REF: parse:p_seq
t49=top();
S_DATA(t49)[2]=t53;
t49=restore();
restore();
save(t49);
X t54=allocate(3); // CONS
save(t54);
t49=pick(2);
X t55=select1(TO_N(1),t49); // SELECT (simple)
t54=top();
S_DATA(t54)[0]=t55;
t55=pick(2);
save(t55);
X t56=allocate(2); // CONS
save(t56);
t55=pick(2);
X t57=select1(TO_N(2),t55); // SELECT (simple)
t56=top();
S_DATA(t56)[0]=t57;
t57=pick(2);
X t59=TO_N(1); // ICONST
save(t59);
X t60=select1(TO_N(3),t57); // SELECT (simple)
t59=restore();
X t58=select1(t59,t60); // SELECT
t56=top();
S_DATA(t56)[1]=t58;
t56=restore();
restore();
X t61=___ar(t56); // REF: ar
t54=top();
S_DATA(t54)[1]=t61;
t61=pick(2);
X t63=TO_N(2); // ICONST
save(t63);
X t64=select1(TO_N(3),t61); // SELECT (simple)
t63=restore();
X t62=select1(t63,t64); // SELECT
t54=top();
S_DATA(t54)[2]=t62;
t54=restore();
restore();
t40=t54;}
save(t40);
X t65=allocate(2); // CONS
save(t65);
t40=pick(2);
save(t40);
X t66=allocate(4); // CONS
save(t66);
t40=pick(2);
X t67=lf[320]; // CONST
t66=top();
S_DATA(t66)[0]=t67;
t67=pick(2);
X t68=select1(TO_N(1),t67); // SELECT (simple)
t66=top();
S_DATA(t66)[1]=t68;
t68=pick(2);
X t70=TO_N(1); // ICONST
save(t70);
X t71=select1(TO_N(2),t68); // SELECT (simple)
t70=restore();
X t69=select1(t70,t71); // SELECT
t66=top();
S_DATA(t66)[2]=t69;
t69=pick(2);
save(t69);
X t72=allocate(2); // CONS
save(t72);
t69=pick(2);
save(t69);
X t73=allocate(2); // CONS
save(t73);
t69=pick(2);
X t74=lf[326]; // CONST
t73=top();
S_DATA(t73)[0]=t74;
t74=pick(2);
X t75=select1(TO_N(1),t74); // SELECT (simple)
t73=top();
S_DATA(t73)[1]=t75;
t73=restore();
restore();
t72=top();
S_DATA(t72)[0]=t73;
t73=pick(2);
save(t73);
X t76=allocate(2); // CONS
save(t76);
t73=pick(2);
save(t73);
X t77=allocate(3); // CONS
save(t77);
t73=pick(2);
X t78=lf[293]; // CONST
t77=top();
S_DATA(t77)[0]=t78;
t78=pick(2);
X t79=select1(TO_N(1),t78); // SELECT (simple)
t77=top();
S_DATA(t77)[1]=t79;
t79=pick(2);
X t80=lf[282]; // CONST
t77=top();
S_DATA(t77)[2]=t80;
t77=restore();
restore();
t76=top();
S_DATA(t76)[0]=t77;
t77=pick(2);
save(t77);
X t81=allocate(2); // CONS
save(t81);
t77=pick(2);
X t82=select1(TO_N(1),t77); // SELECT (simple)
t81=top();
S_DATA(t81)[0]=t82;
t82=pick(2);
X t83=select1(TO_N(2),t82); // SELECT (simple)
X t84=___tl(t83); // REF: tl
t81=top();
S_DATA(t81)[1]=t84;
t81=restore();
restore();
X t85=___dl(t81); // REF: dl
int t87; // ALPHA
check_S(t85,"@");
int t88=S_LENGTH(t85);
save(t85);
X t86=allocate(t88);
save(t86);
for(t87=0;t87<t88;++t87){
X t86=S_DATA(pick(2))[t87];
save(t86);
X t89=allocate(3); // CONS
save(t89);
t86=pick(2);
X t90=lf[323]; // CONST
t89=top();
S_DATA(t89)[0]=t90;
t90=pick(2);
X t91=select1(TO_N(1),t90); // SELECT (simple)
t89=top();
S_DATA(t89)[1]=t91;
t91=pick(2);
X t92=select1(TO_N(2),t91); // SELECT (simple)
t89=top();
S_DATA(t89)[2]=t92;
t89=restore();
restore();
S_DATA(top())[t87]=t89;}
t86=restore();
restore();
t76=top();
S_DATA(t76)[1]=t86;
t76=restore();
restore();
X t93=___al(t76); // REF: al
t72=top();
S_DATA(t72)[1]=t93;
t72=restore();
restore();
X t94=___cat(t72); // REF: cat
t66=top();
S_DATA(t66)[3]=t94;
t66=restore();
restore();
t65=top();
S_DATA(t65)[0]=t66;
t66=pick(2);
X t95=select1(TO_N(3),t66); // SELECT (simple)
X t96=___parse_3aadvance(t95); // REF: parse:advance
t65=top();
S_DATA(t65)[1]=t96;
t65=restore();
restore();
t19=t65;}
t9=t19;}
t1=t9;
}else{
save(x); // COND
save(x);
X t98=allocate(3); // CONS
save(t98);
x=pick(2);
X t99=___id(x); // REF: id
t98=top();
S_DATA(t98)[0]=t99;
t99=pick(2);
X t100=lf[90]; // CONST
t98=top();
S_DATA(t98)[1]=t100;
t100=pick(2);
X t101=lf[58]; // CONST
t98=top();
S_DATA(t98)[2]=t101;
t98=restore();
restore();
X t102=___parse_3amatchval(t98); // REF: parse:matchval
X t97;
x=restore();
if(t102!=F){
save(x);
X t103=allocate(2); // CONS
save(t103);
x=pick(2);
X t105=TO_N(2); // ICONST
save(t105);
X t107=TO_N(1); // ICONST
save(t107);
X t108=select1(TO_N(1),x); // SELECT (simple)
t107=restore();
X t106=select1(t107,t108); // SELECT
t105=restore();
X t104=select1(t105,t106); // SELECT
t103=top();
S_DATA(t103)[0]=t104;
t104=pick(2);
X t109=___parse_3aadvance(t104); // REF: parse:advance
t103=top();
S_DATA(t103)[1]=t109;
t103=restore();
restore();
save(t103); // COND
save(t103);
X t111=allocate(3); // CONS
save(t111);
t103=pick(2);
X t112=select1(TO_N(2),t103); // SELECT (simple)
t111=top();
S_DATA(t111)[0]=t112;
t112=pick(2);
X t113=lf[90]; // CONST
t111=top();
S_DATA(t111)[1]=t113;
t113=pick(2);
X t114=lf[81]; // CONST
t111=top();
S_DATA(t111)[2]=t114;
t111=restore();
restore();
X t115=___parse_3amatchval(t111); // REF: parse:matchval
X t110;
t103=restore();
if(t115!=F){
save(t103);
X t116=allocate(2); // CONS
save(t116);
t103=pick(2);
save(t103);
X t117=allocate(3); // CONS
save(t117);
t103=pick(2);
X t118=lf[291]; // CONST
t117=top();
S_DATA(t117)[0]=t118;
t118=pick(2);
X t119=select1(TO_N(1),t118); // SELECT (simple)
t117=top();
S_DATA(t117)[1]=t119;
t119=pick(2);
X t120=EMPTY; // ICONST
t117=top();
S_DATA(t117)[2]=t120;
t117=restore();
restore();
t116=top();
S_DATA(t116)[0]=t117;
t117=pick(2);
X t121=select1(TO_N(2),t117); // SELECT (simple)
X t122=___parse_3aadvance(t121); // REF: parse:advance
t116=top();
S_DATA(t116)[1]=t122;
t116=restore();
restore();
t110=t116;
}else{
save(t103);
X t123=allocate(3); // CONS
save(t123);
t103=pick(2);
X t124=select1(TO_N(1),t103); // SELECT (simple)
t123=top();
S_DATA(t123)[0]=t124;
t124=pick(2);
X t125=EMPTY; // ICONST
t123=top();
S_DATA(t123)[1]=t125;
t125=pick(2);
X t126=select1(TO_N(2),t125); // SELECT (simple)
X t127=___parse_3ap_5fseq(t126); // REF: parse:p_seq
t123=top();
S_DATA(t123)[2]=t127;
t123=restore();
restore();
X t128=___parse_3ap_5fval_5fcons_27(t123); // REF: parse:p_val_cons'
t110=t128;}
t97=t110;
}else{
save(x); // COND
save(x);
X t130=allocate(3); // CONS
save(t130);
x=pick(2);
X t131=___id(x); // REF: id
t130=top();
S_DATA(t130)[0]=t131;
t131=pick(2);
X t132=lf[90]; // CONST
t130=top();
S_DATA(t130)[1]=t132;
t132=pick(2);
X t133=lf[59]; // CONST
t130=top();
S_DATA(t130)[2]=t133;
t130=restore();
restore();
X t134=___parse_3amatchval(t130); // REF: parse:matchval
X t129;
x=restore();
if(t134!=F){
save(x);
X t135=allocate(2); // CONS
save(t135);
x=pick(2);
X t137=TO_N(2); // ICONST
save(t137);
X t139=TO_N(1); // ICONST
save(t139);
X t140=select1(TO_N(1),x); // SELECT (simple)
t139=restore();
X t138=select1(t139,t140); // SELECT
t137=restore();
X t136=select1(t137,t138); // SELECT
t135=top();
S_DATA(t135)[0]=t136;
t136=pick(2);
X t141=___parse_3aadvance(t136); // REF: parse:advance
t135=top();
S_DATA(t135)[1]=t141;
t135=restore();
restore();
save(t135); // COND
save(t135);
X t143=allocate(3); // CONS
save(t143);
t135=pick(2);
X t144=select1(TO_N(2),t135); // SELECT (simple)
t143=top();
S_DATA(t143)[0]=t144;
t144=pick(2);
X t145=lf[90]; // CONST
t143=top();
S_DATA(t143)[1]=t145;
t145=pick(2);
X t146=lf[84]; // CONST
t143=top();
S_DATA(t143)[2]=t146;
t143=restore();
restore();
X t147=___parse_3amatchval(t143); // REF: parse:matchval
X t142;
t135=restore();
if(t147!=F){
save(t135);
X t148=allocate(2); // CONS
save(t148);
t135=pick(2);
save(t135);
X t149=allocate(2); // CONS
save(t149);
t135=pick(2);
X t150=lf[329]; // CONST
t149=top();
S_DATA(t149)[0]=t150;
t150=pick(2);
X t151=select1(TO_N(1),t150); // SELECT (simple)
t149=top();
S_DATA(t149)[1]=t151;
t149=restore();
restore();
t148=top();
S_DATA(t148)[0]=t149;
t149=pick(2);
X t152=select1(TO_N(2),t149); // SELECT (simple)
X t153=___parse_3aadvance(t152); // REF: parse:advance
t148=top();
S_DATA(t148)[1]=t153;
t148=restore();
restore();
t142=t148;
}else{
save(t135);
X t154=allocate(3); // CONS
save(t154);
t135=pick(2);
X t155=select1(TO_N(1),t135); // SELECT (simple)
t154=top();
S_DATA(t154)[0]=t155;
t155=pick(2);
X t156=EMPTY; // ICONST
t154=top();
S_DATA(t154)[1]=t156;
t156=pick(2);
X t157=select1(TO_N(2),t156); // SELECT (simple)
X t158=___parse_3ap_5fseq(t157); // REF: parse:p_seq
t154=top();
S_DATA(t154)[2]=t158;
t154=restore();
restore();
X t159=___parse_3ap_5fval_5fpcons_27(t154); // REF: parse:p_val_pcons'
t142=t159;}
t129=t142;
}else{
save(x); // COND
save(x);
X t161=allocate(3); // CONS
save(t161);
x=pick(2);
X t162=___id(x); // REF: id
t161=top();
S_DATA(t161)[0]=t162;
t162=pick(2);
X t163=lf[90]; // CONST
t161=top();
S_DATA(t161)[1]=t163;
t163=pick(2);
X t164=lf[60]; // CONST
t161=top();
S_DATA(t161)[2]=t164;
t161=restore();
restore();
X t165=___parse_3amatchval(t161); // REF: parse:matchval
X t160;
x=restore();
if(t165!=F){
save(x);
X t166=allocate(2); // CONS
save(t166);
x=pick(2);
X t168=TO_N(2); // ICONST
save(t168);
X t170=TO_N(1); // ICONST
save(t170);
X t171=select1(TO_N(1),x); // SELECT (simple)
t170=restore();
X t169=select1(t170,t171); // SELECT
t168=restore();
X t167=select1(t168,t169); // SELECT
t166=top();
S_DATA(t166)[0]=t167;
t167=pick(2);
X t172=___parse_3aadvance(t167); // REF: parse:advance
t166=top();
S_DATA(t166)[1]=t172;
t166=restore();
restore();
save(t166);
X t173=allocate(2); // CONS
save(t173);
t166=pick(2);
X t174=select1(TO_N(1),t166); // SELECT (simple)
t173=top();
S_DATA(t173)[0]=t174;
t174=pick(2);
X t175=select1(TO_N(2),t174); // SELECT (simple)
X t176=___parse_3ap_5fseq(t175); // REF: parse:p_seq
t173=top();
S_DATA(t173)[1]=t176;
t173=restore();
restore();
save(t173);
X t177=allocate(3); // CONS
save(t177);
t173=pick(2);
X t178=select1(TO_N(1),t173); // SELECT (simple)
t177=top();
S_DATA(t177)[0]=t178;
t178=pick(2);
X t180=TO_N(1); // ICONST
save(t180);
X t181=select1(TO_N(2),t178); // SELECT (simple)
t180=restore();
X t179=select1(t180,t181); // SELECT
t177=top();
S_DATA(t177)[1]=t179;
t179=pick(2);
save(t179);
X t182=allocate(3); // CONS
save(t182);
t179=pick(2);
X t184=TO_N(2); // ICONST
save(t184);
X t185=select1(TO_N(2),t179); // SELECT (simple)
t184=restore();
X t183=select1(t184,t185); // SELECT
t182=top();
S_DATA(t182)[0]=t183;
t183=pick(2);
X t186=lf[90]; // CONST
t182=top();
S_DATA(t182)[1]=t186;
t186=pick(2);
X t187=lf[91]; // CONST
t182=top();
S_DATA(t182)[2]=t187;
t182=restore();
restore();
X t188=___parse_3aexpectval(t182); // REF: parse:expectval
X t189=___parse_3ap_5fseq(t188); // REF: parse:p_seq
t177=top();
S_DATA(t177)[2]=t189;
t177=restore();
restore();
save(t177);
X t190=allocate(2); // CONS
save(t190);
t177=pick(2);
save(t177);
X t191=allocate(4); // CONS
save(t191);
t177=pick(2);
X t192=lf[302]; // CONST
t191=top();
S_DATA(t191)[0]=t192;
t192=pick(2);
X t193=select1(TO_N(1),t192); // SELECT (simple)
t191=top();
S_DATA(t191)[1]=t193;
t193=pick(2);
X t194=select1(TO_N(2),t193); // SELECT (simple)
t191=top();
S_DATA(t191)[2]=t194;
t194=pick(2);
X t196=TO_N(1); // ICONST
save(t196);
X t197=select1(TO_N(3),t194); // SELECT (simple)
t196=restore();
X t195=select1(t196,t197); // SELECT
t191=top();
S_DATA(t191)[3]=t195;
t191=restore();
restore();
t190=top();
S_DATA(t190)[0]=t191;
t191=pick(2);
save(t191);
X t198=allocate(3); // CONS
save(t198);
t191=pick(2);
X t200=TO_N(2); // ICONST
save(t200);
X t201=select1(TO_N(3),t191); // SELECT (simple)
t200=restore();
X t199=select1(t200,t201); // SELECT
t198=top();
S_DATA(t198)[0]=t199;
t199=pick(2);
X t202=lf[90]; // CONST
t198=top();
S_DATA(t198)[1]=t202;
t202=pick(2);
X t203=lf[61]; // CONST
t198=top();
S_DATA(t198)[2]=t203;
t198=restore();
restore();
X t204=___parse_3aexpectval(t198); // REF: parse:expectval
t190=top();
S_DATA(t190)[1]=t204;
t190=restore();
restore();
t160=t190;
}else{
save(x); // COND
save(x);
X t206=allocate(3); // CONS
save(t206);
x=pick(2);
X t207=___id(x); // REF: id
t206=top();
S_DATA(t206)[0]=t207;
t207=pick(2);
X t208=lf[90]; // CONST
t206=top();
S_DATA(t206)[1]=t208;
t208=pick(2);
X t209=lf[62]; // CONST
t206=top();
S_DATA(t206)[2]=t209;
t206=restore();
restore();
X t210=___parse_3amatchval(t206); // REF: parse:matchval
X t205;
x=restore();
if(t210!=F){
save(x);
X t211=allocate(2); // CONS
save(t211);
x=pick(2);
X t213=TO_N(2); // ICONST
save(t213);
X t215=TO_N(1); // ICONST
save(t215);
X t216=select1(TO_N(1),x); // SELECT (simple)
t215=restore();
X t214=select1(t215,t216); // SELECT
t213=restore();
X t212=select1(t213,t214); // SELECT
t211=top();
S_DATA(t211)[0]=t212;
t212=pick(2);
X t217=___parse_3aadvance(t212); // REF: parse:advance
X t218=___parse_3ap_5fconstant(t217); // REF: parse:p_constant
t211=top();
S_DATA(t211)[1]=t218;
t211=restore();
restore();
save(t211); // COND
X t221=TO_N(1); // ICONST
save(t221);
X t222=select1(TO_N(2),t211); // SELECT (simple)
t221=restore();
X t220=select1(t221,t222); // SELECT
save(t220); // OR
X t225=lf[381]; // CONST
save(t225);
X t226=___id(t220); // REF: id
t225=restore();
X t224=(t225==t226)||eq1(t225,t226)?T:F; // EQ
X t223=t224;
t220=restore();
if(t223==F){
save(t220); // OR
X t228=___num(t220); // REF: num
X t227=t228;
t220=restore();
if(t227==F){
save(t220);
X t229=allocate(2); // CONS
save(t229);
t220=pick(2);
X t230=___id(t220); // REF: id
t229=top();
S_DATA(t229)[0]=t230;
t230=pick(2);
X t231=lf[63]; // CONST
t229=top();
S_DATA(t229)[1]=t231;
t229=restore();
restore();
X t232=___index(t229); // REF: index
t227=t232;}
t223=t227;}
X t219;
t211=restore();
if(t223!=F){
save(t211);
X t233=allocate(2); // CONS
save(t233);
t211=pick(2);
save(t211);
X t234=allocate(3); // CONS
save(t234);
t211=pick(2);
X t235=lf[291]; // CONST
t234=top();
S_DATA(t234)[0]=t235;
t235=pick(2);
X t236=select1(TO_N(1),t235); // SELECT (simple)
t234=top();
S_DATA(t234)[1]=t236;
t236=pick(2);
X t238=TO_N(1); // ICONST
save(t238);
X t239=select1(TO_N(2),t236); // SELECT (simple)
t238=restore();
X t237=select1(t238,t239); // SELECT
t234=top();
S_DATA(t234)[2]=t237;
t234=restore();
restore();
t233=top();
S_DATA(t233)[0]=t234;
t234=pick(2);
X t241=TO_N(2); // ICONST
save(t241);
X t242=select1(TO_N(2),t234); // SELECT (simple)
t241=restore();
X t240=select1(t241,t242); // SELECT
t233=top();
S_DATA(t233)[1]=t240;
t233=restore();
restore();
t219=t233;
}else{
save(t211);
X t243=allocate(2); // CONS
save(t243);
t211=pick(2);
save(t211);
X t244=allocate(3); // CONS
save(t244);
t211=pick(2);
X t245=lf[290]; // CONST
t244=top();
S_DATA(t244)[0]=t245;
t245=pick(2);
X t246=select1(TO_N(1),t245); // SELECT (simple)
t244=top();
S_DATA(t244)[1]=t246;
t246=pick(2);
X t248=TO_N(1); // ICONST
save(t248);
X t249=select1(TO_N(2),t246); // SELECT (simple)
t248=restore();
X t247=select1(t248,t249); // SELECT
t244=top();
S_DATA(t244)[2]=t247;
t244=restore();
restore();
t243=top();
S_DATA(t243)[0]=t244;
t244=pick(2);
X t251=TO_N(2); // ICONST
save(t251);
X t252=select1(TO_N(2),t244); // SELECT (simple)
t251=restore();
X t250=select1(t251,t252); // SELECT
t243=top();
S_DATA(t243)[1]=t250;
t243=restore();
restore();
t219=t243;}
t205=t219;
}else{
save(x); // COND
save(x);
X t254=allocate(3); // CONS
save(t254);
x=pick(2);
X t255=___id(x); // REF: id
t254=top();
S_DATA(t254)[0]=t255;
t255=pick(2);
X t256=lf[90]; // CONST
t254=top();
S_DATA(t254)[1]=t256;
t256=pick(2);
X t257=lf[64]; // CONST
t254=top();
S_DATA(t254)[2]=t257;
t254=restore();
restore();
X t258=___parse_3amatchval(t254); // REF: parse:matchval
X t253;
x=restore();
if(t258!=F){
save(x);
X t259=allocate(2); // CONS
save(t259);
x=pick(2);
X t261=TO_N(2); // ICONST
save(t261);
X t263=TO_N(1); // ICONST
save(t263);
X t264=select1(TO_N(1),x); // SELECT (simple)
t263=restore();
X t262=select1(t263,t264); // SELECT
t261=restore();
X t260=select1(t261,t262); // SELECT
t259=top();
S_DATA(t259)[0]=t260;
t260=pick(2);
X t265=___parse_3aadvance(t260); // REF: parse:advance
X t266=___parse_3ap_5fvalue(t265); // REF: parse:p_value
t259=top();
S_DATA(t259)[1]=t266;
t259=restore();
restore();
save(t259);
X t267=allocate(2); // CONS
save(t267);
t259=pick(2);
save(t259);
X t268=allocate(3); // CONS
save(t268);
t259=pick(2);
X t269=lf[296]; // CONST
t268=top();
S_DATA(t268)[0]=t269;
t269=pick(2);
X t270=select1(TO_N(1),t269); // SELECT (simple)
t268=top();
S_DATA(t268)[1]=t270;
t270=pick(2);
X t272=TO_N(1); // ICONST
save(t272);
X t273=select1(TO_N(2),t270); // SELECT (simple)
t272=restore();
X t271=select1(t272,t273); // SELECT
t268=top();
S_DATA(t268)[2]=t271;
t268=restore();
restore();
t267=top();
S_DATA(t267)[0]=t268;
t268=pick(2);
X t275=TO_N(2); // ICONST
save(t275);
X t276=select1(TO_N(2),t268); // SELECT (simple)
t275=restore();
X t274=select1(t275,t276); // SELECT
t267=top();
S_DATA(t267)[1]=t274;
t267=restore();
restore();
t253=t267;
}else{
save(x); // COND
save(x);
X t278=allocate(3); // CONS
save(t278);
x=pick(2);
X t279=___id(x); // REF: id
t278=top();
S_DATA(t278)[0]=t279;
t279=pick(2);
X t280=lf[90]; // CONST
t278=top();
S_DATA(t278)[1]=t280;
t280=pick(2);
X t281=lf[65]; // CONST
t278=top();
S_DATA(t278)[2]=t281;
t278=restore();
restore();
X t282=___parse_3amatchval(t278); // REF: parse:matchval
X t277;
x=restore();
if(t282!=F){
save(x);
X t283=allocate(2); // CONS
save(t283);
x=pick(2);
X t285=TO_N(2); // ICONST
save(t285);
X t287=TO_N(1); // ICONST
save(t287);
X t288=select1(TO_N(1),x); // SELECT (simple)
t287=restore();
X t286=select1(t287,t288); // SELECT
t285=restore();
X t284=select1(t285,t286); // SELECT
t283=top();
S_DATA(t283)[0]=t284;
t284=pick(2);
X t289=___parse_3aadvance(t284); // REF: parse:advance
X t290=___parse_3ap_5fvalue(t289); // REF: parse:p_value
t283=top();
S_DATA(t283)[1]=t290;
t283=restore();
restore();
save(t283);
X t291=allocate(2); // CONS
save(t291);
t283=pick(2);
save(t283);
X t292=allocate(3); // CONS
save(t292);
t283=pick(2);
X t293=lf[298]; // CONST
t292=top();
S_DATA(t292)[0]=t293;
t293=pick(2);
X t294=select1(TO_N(1),t293); // SELECT (simple)
t292=top();
S_DATA(t292)[1]=t294;
t294=pick(2);
X t296=TO_N(1); // ICONST
save(t296);
X t297=select1(TO_N(2),t294); // SELECT (simple)
t296=restore();
X t295=select1(t296,t297); // SELECT
t292=top();
S_DATA(t292)[2]=t295;
t292=restore();
restore();
t291=top();
S_DATA(t291)[0]=t292;
t292=pick(2);
X t299=TO_N(2); // ICONST
save(t299);
X t300=select1(TO_N(2),t292); // SELECT (simple)
t299=restore();
X t298=select1(t299,t300); // SELECT
t291=top();
S_DATA(t291)[1]=t298;
t291=restore();
restore();
t277=t291;
}else{
save(x); // COND
save(x);
X t302=allocate(3); // CONS
save(t302);
x=pick(2);
X t303=___id(x); // REF: id
t302=top();
S_DATA(t302)[0]=t303;
t303=pick(2);
X t304=lf[90]; // CONST
t302=top();
S_DATA(t302)[1]=t304;
t304=pick(2);
X t305=lf[66]; // CONST
t302=top();
S_DATA(t302)[2]=t305;
t302=restore();
restore();
X t306=___parse_3amatchval(t302); // REF: parse:matchval
X t301;
x=restore();
if(t306!=F){
save(x);
X t307=allocate(2); // CONS
save(t307);
x=pick(2);
X t309=TO_N(2); // ICONST
save(t309);
X t311=TO_N(1); // ICONST
save(t311);
X t312=select1(TO_N(1),x); // SELECT (simple)
t311=restore();
X t310=select1(t311,t312); // SELECT
t309=restore();
X t308=select1(t309,t310); // SELECT
t307=top();
S_DATA(t307)[0]=t308;
t308=pick(2);
X t313=___parse_3aadvance(t308); // REF: parse:advance
X t314=___parse_3ap_5fvalue(t313); // REF: parse:p_value
t307=top();
S_DATA(t307)[1]=t314;
t307=restore();
restore();
save(t307);
X t315=allocate(2); // CONS
save(t315);
t307=pick(2);
save(t307);
X t316=allocate(3); // CONS
save(t316);
t307=pick(2);
X t317=lf[300]; // CONST
t316=top();
S_DATA(t316)[0]=t317;
t317=pick(2);
X t318=select1(TO_N(1),t317); // SELECT (simple)
t316=top();
S_DATA(t316)[1]=t318;
t318=pick(2);
X t320=TO_N(1); // ICONST
save(t320);
X t321=select1(TO_N(2),t318); // SELECT (simple)
t320=restore();
X t319=select1(t320,t321); // SELECT
t316=top();
S_DATA(t316)[2]=t319;
t316=restore();
restore();
t315=top();
S_DATA(t315)[0]=t316;
t316=pick(2);
X t323=TO_N(2); // ICONST
save(t323);
X t324=select1(TO_N(2),t316); // SELECT (simple)
t323=restore();
X t322=select1(t323,t324); // SELECT
t315=top();
S_DATA(t315)[1]=t322;
t315=restore();
restore();
t301=t315;
}else{
save(x); // COND
save(x);
X t326=allocate(3); // CONS
save(t326);
x=pick(2);
X t327=___id(x); // REF: id
t326=top();
S_DATA(t326)[0]=t327;
t327=pick(2);
X t328=lf[80]; // CONST
t326=top();
S_DATA(t326)[1]=t328;
t328=pick(2);
X t329=lf[67]; // CONST
t326=top();
S_DATA(t326)[2]=t329;
t326=restore();
restore();
X t330=___parse_3amatchval(t326); // REF: parse:matchval
X t325;
x=restore();
if(t330!=F){
save(x);
X t331=allocate(2); // CONS
save(t331);
x=pick(2);
X t333=TO_N(2); // ICONST
save(t333);
X t335=TO_N(1); // ICONST
save(t335);
X t336=select1(TO_N(1),x); // SELECT (simple)
t335=restore();
X t334=select1(t335,t336); // SELECT
t333=restore();
X t332=select1(t333,t334); // SELECT
t331=top();
S_DATA(t331)[0]=t332;
t332=pick(2);
X t337=___parse_3aadvance(t332); // REF: parse:advance
X t338=___parse_3ap_5fconstant(t337); // REF: parse:p_constant
t331=top();
S_DATA(t331)[1]=t338;
t331=restore();
restore();
save(t331);
X t339=allocate(2); // CONS
save(t339);
t331=pick(2);
save(t331);
X t340=allocate(4); // CONS
save(t340);
t331=pick(2);
X t341=lf[320]; // CONST
t340=top();
S_DATA(t340)[0]=t341;
t341=pick(2);
X t342=select1(TO_N(1),t341); // SELECT (simple)
t340=top();
S_DATA(t340)[1]=t342;
t342=pick(2);
save(t342);
X t343=allocate(3); // CONS
save(t343);
t342=pick(2);
X t344=lf[293]; // CONST
t343=top();
S_DATA(t343)[0]=t344;
t344=pick(2);
X t345=select1(TO_N(1),t344); // SELECT (simple)
t343=top();
S_DATA(t343)[1]=t345;
t345=pick(2);
X t346=lf[68]; // CONST
t343=top();
S_DATA(t343)[2]=t346;
t343=restore();
restore();
t340=top();
S_DATA(t340)[2]=t343;
t343=pick(2);
save(t343);
X t347=allocate(4); // CONS
save(t347);
t343=pick(2);
X t348=lf[326]; // CONST
t347=top();
S_DATA(t347)[0]=t348;
t348=pick(2);
X t349=select1(TO_N(1),t348); // SELECT (simple)
t347=top();
S_DATA(t347)[1]=t349;
t349=pick(2);
save(t349);
X t350=allocate(3); // CONS
save(t350);
t349=pick(2);
X t351=lf[290]; // CONST
t350=top();
S_DATA(t350)[0]=t351;
t351=pick(2);
X t352=select1(TO_N(1),t351); // SELECT (simple)
t350=top();
S_DATA(t350)[1]=t352;
t352=pick(2);
X t354=TO_N(1); // ICONST
save(t354);
X t355=select1(TO_N(2),t352); // SELECT (simple)
t354=restore();
X t353=select1(t354,t355); // SELECT
t350=top();
S_DATA(t350)[2]=t353;
t350=restore();
restore();
t347=top();
S_DATA(t347)[2]=t350;
t350=pick(2);
save(t350);
X t356=allocate(3); // CONS
save(t356);
t350=pick(2);
X t357=lf[293]; // CONST
t356=top();
S_DATA(t356)[0]=t357;
t357=pick(2);
X t358=select1(TO_N(1),t357); // SELECT (simple)
t356=top();
S_DATA(t356)[1]=t358;
t358=pick(2);
X t359=lf[282]; // CONST
t356=top();
S_DATA(t356)[2]=t359;
t356=restore();
restore();
t347=top();
S_DATA(t347)[3]=t356;
t347=restore();
restore();
t340=top();
S_DATA(t340)[3]=t347;
t340=restore();
restore();
t339=top();
S_DATA(t339)[0]=t340;
t340=pick(2);
X t361=TO_N(2); // ICONST
save(t361);
X t362=select1(TO_N(2),t340); // SELECT (simple)
t361=restore();
X t360=select1(t361,t362); // SELECT
t339=top();
S_DATA(t339)[1]=t360;
t339=restore();
restore();
t325=t339;
}else{
save(x); // COND
save(x);
X t364=allocate(3); // CONS
save(t364);
x=pick(2);
X t365=___id(x); // REF: id
t364=top();
S_DATA(t364)[0]=t365;
t365=pick(2);
X t366=lf[90]; // CONST
t364=top();
S_DATA(t364)[1]=t366;
t366=pick(2);
X t367=lf[69]; // CONST
t364=top();
S_DATA(t364)[2]=t367;
t364=restore();
restore();
X t368=___parse_3amatchval(t364); // REF: parse:matchval
X t363;
x=restore();
if(t368!=F){
save(x);
X t369=allocate(2); // CONS
save(t369);
x=pick(2);
save(x);
X t370=allocate(3); // CONS
save(t370);
x=pick(2);
X t371=lf[291]; // CONST
t370=top();
S_DATA(t370)[0]=t371;
t371=pick(2);
X t373=TO_N(2); // ICONST
save(t373);
X t375=TO_N(1); // ICONST
save(t375);
X t376=select1(TO_N(1),t371); // SELECT (simple)
t375=restore();
X t374=select1(t375,t376); // SELECT
t373=restore();
X t372=select1(t373,t374); // SELECT
t370=top();
S_DATA(t370)[1]=t372;
t372=pick(2);
X t377=T; // ICONST
t370=top();
S_DATA(t370)[2]=t377;
t370=restore();
restore();
t369=top();
S_DATA(t369)[0]=t370;
t370=pick(2);
X t378=___parse_3aadvance(t370); // REF: parse:advance
t369=top();
S_DATA(t369)[1]=t378;
t369=restore();
restore();
t363=t369;
}else{
save(x); // COND
save(x);
X t380=allocate(3); // CONS
save(t380);
x=pick(2);
X t381=___id(x); // REF: id
t380=top();
S_DATA(t380)[0]=t381;
t381=pick(2);
X t382=lf[90]; // CONST
t380=top();
S_DATA(t380)[1]=t382;
t382=pick(2);
X t383=lf[70]; // CONST
t380=top();
S_DATA(t380)[2]=t383;
t380=restore();
restore();
X t384=___parse_3amatchval(t380); // REF: parse:matchval
X t379;
x=restore();
if(t384!=F){
save(x);
X t385=allocate(2); // CONS
save(t385);
x=pick(2);
X t387=TO_N(2); // ICONST
save(t387);
X t389=TO_N(1); // ICONST
save(t389);
X t390=select1(TO_N(1),x); // SELECT (simple)
t389=restore();
X t388=select1(t389,t390); // SELECT
t387=restore();
X t386=select1(t387,t388); // SELECT
t385=top();
S_DATA(t385)[0]=t386;
t386=pick(2);
X t391=___parse_3aadvance(t386); // REF: parse:advance
X t392=___parse_3ap_5fconstant(t391); // REF: parse:p_constant
t385=top();
S_DATA(t385)[1]=t392;
t385=restore();
restore();
save(t385);
X t393=allocate(2); // CONS
save(t393);
t385=pick(2);
save(t385);
X t394=allocate(4); // CONS
save(t394);
t385=pick(2);
X t395=lf[320]; // CONST
t394=top();
S_DATA(t394)[0]=t395;
t395=pick(2);
X t396=select1(TO_N(1),t395); // SELECT (simple)
t394=top();
S_DATA(t394)[1]=t396;
t396=pick(2);
save(t396);
X t397=allocate(3); // CONS
save(t397);
t396=pick(2);
X t398=lf[293]; // CONST
t397=top();
S_DATA(t397)[0]=t398;
t398=pick(2);
X t399=select1(TO_N(1),t398); // SELECT (simple)
t397=top();
S_DATA(t397)[1]=t399;
t399=pick(2);
X t400=lf[125]; // CONST
t397=top();
S_DATA(t397)[2]=t400;
t397=restore();
restore();
t394=top();
S_DATA(t394)[2]=t397;
t397=pick(2);
save(t397);
X t401=allocate(4); // CONS
save(t401);
t397=pick(2);
X t402=lf[326]; // CONST
t401=top();
S_DATA(t401)[0]=t402;
t402=pick(2);
X t403=select1(TO_N(1),t402); // SELECT (simple)
t401=top();
S_DATA(t401)[1]=t403;
t403=pick(2);
save(t403);
X t404=allocate(3); // CONS
save(t404);
t403=pick(2);
X t405=lf[290]; // CONST
t404=top();
S_DATA(t404)[0]=t405;
t405=pick(2);
X t406=select1(TO_N(1),t405); // SELECT (simple)
t404=top();
S_DATA(t404)[1]=t406;
t406=pick(2);
X t408=TO_N(1); // ICONST
save(t408);
X t409=select1(TO_N(2),t406); // SELECT (simple)
t408=restore();
X t407=select1(t408,t409); // SELECT
t404=top();
S_DATA(t404)[2]=t407;
t404=restore();
restore();
t401=top();
S_DATA(t401)[2]=t404;
t404=pick(2);
save(t404);
X t410=allocate(3); // CONS
save(t410);
t404=pick(2);
X t411=lf[293]; // CONST
t410=top();
S_DATA(t410)[0]=t411;
t411=pick(2);
X t412=select1(TO_N(1),t411); // SELECT (simple)
t410=top();
S_DATA(t410)[1]=t412;
t412=pick(2);
X t413=lf[282]; // CONST
t410=top();
S_DATA(t410)[2]=t413;
t410=restore();
restore();
t401=top();
S_DATA(t401)[3]=t410;
t401=restore();
restore();
t394=top();
S_DATA(t394)[3]=t401;
t394=restore();
restore();
t393=top();
S_DATA(t393)[0]=t394;
t394=pick(2);
X t415=TO_N(2); // ICONST
save(t415);
X t416=select1(TO_N(2),t394); // SELECT (simple)
t415=restore();
X t414=select1(t415,t416); // SELECT
t393=top();
S_DATA(t393)[1]=t414;
t393=restore();
restore();
t379=t393;
}else{
save(x); // COND
save(x);
X t418=allocate(3); // CONS
save(t418);
x=pick(2);
X t419=___id(x); // REF: id
t418=top();
S_DATA(t418)[0]=t419;
t419=pick(2);
X t420=lf[90]; // CONST
t418=top();
S_DATA(t418)[1]=t420;
t420=pick(2);
X t421=lf[71]; // CONST
t418=top();
S_DATA(t418)[2]=t421;
t418=restore();
restore();
X t422=___parse_3amatchval(t418); // REF: parse:matchval
X t417;
x=restore();
if(t422!=F){
save(x);
X t423=allocate(2); // CONS
save(t423);
x=pick(2);
X t425=TO_N(2); // ICONST
save(t425);
X t427=TO_N(1); // ICONST
save(t427);
X t428=select1(TO_N(1),x); // SELECT (simple)
t427=restore();
X t426=select1(t427,t428); // SELECT
t425=restore();
X t424=select1(t425,t426); // SELECT
t423=top();
S_DATA(t423)[0]=t424;
t424=pick(2);
X t429=___parse_3aadvance(t424); // REF: parse:advance
X t430=___parse_3ap_5fconstant(t429); // REF: parse:p_constant
t423=top();
S_DATA(t423)[1]=t430;
t423=restore();
restore();
save(t423); // COND
X t433=TO_N(1); // ICONST
save(t433);
X t434=select1(TO_N(2),t423); // SELECT (simple)
t433=restore();
X t432=select1(t433,t434); // SELECT
X t435=___num(t432); // REF: num
X t431;
t423=restore();
if(t435!=F){
X t436=___id(t423); // REF: id
t431=t436;
}else{
save(t423);
X t437=allocate(2); // CONS
save(t437);
t423=pick(2);
X t438=lf[72]; // CONST
t437=top();
S_DATA(t437)[0]=t438;
t438=pick(2);
X t440=TO_N(2); // ICONST
save(t440);
X t441=select1(TO_N(2),t438); // SELECT (simple)
t440=restore();
X t439=select1(t440,t441); // SELECT
t437=top();
S_DATA(t437)[1]=t439;
t437=restore();
restore();
X t442=___parse_3afail(t437); // REF: parse:fail
t431=t442;}
save(t431);
X t443=allocate(2); // CONS
save(t443);
t431=pick(2);
save(t431);
X t444=allocate(4); // CONS
save(t444);
t431=pick(2);
X t445=lf[320]; // CONST
t444=top();
S_DATA(t444)[0]=t445;
t445=pick(2);
X t446=select1(TO_N(1),t445); // SELECT (simple)
t444=top();
S_DATA(t444)[1]=t446;
t446=pick(2);
save(t446);
X t447=allocate(3); // CONS
save(t447);
t446=pick(2);
X t448=lf[293]; // CONST
t447=top();
S_DATA(t447)[0]=t448;
t448=pick(2);
X t449=select1(TO_N(1),t448); // SELECT (simple)
t447=top();
S_DATA(t447)[1]=t449;
t449=pick(2);
X t450=lf[128]; // CONST
t447=top();
S_DATA(t447)[2]=t450;
t447=restore();
restore();
t444=top();
S_DATA(t444)[2]=t447;
t447=pick(2);
save(t447);
X t451=allocate(4); // CONS
save(t451);
t447=pick(2);
X t452=lf[326]; // CONST
t451=top();
S_DATA(t451)[0]=t452;
t452=pick(2);
X t453=select1(TO_N(1),t452); // SELECT (simple)
t451=top();
S_DATA(t451)[1]=t453;
t453=pick(2);
save(t453);
X t454=allocate(3); // CONS
save(t454);
t453=pick(2);
X t455=lf[291]; // CONST
t454=top();
S_DATA(t454)[0]=t455;
t455=pick(2);
X t456=select1(TO_N(1),t455); // SELECT (simple)
t454=top();
S_DATA(t454)[1]=t456;
t456=pick(2);
X t458=TO_N(1); // ICONST
save(t458);
X t459=select1(TO_N(2),t456); // SELECT (simple)
t458=restore();
X t457=select1(t458,t459); // SELECT
t454=top();
S_DATA(t454)[2]=t457;
t454=restore();
restore();
t451=top();
S_DATA(t451)[2]=t454;
t454=pick(2);
save(t454);
X t460=allocate(3); // CONS
save(t460);
t454=pick(2);
X t461=lf[293]; // CONST
t460=top();
S_DATA(t460)[0]=t461;
t461=pick(2);
X t462=select1(TO_N(1),t461); // SELECT (simple)
t460=top();
S_DATA(t460)[1]=t462;
t462=pick(2);
X t463=lf[282]; // CONST
t460=top();
S_DATA(t460)[2]=t463;
t460=restore();
restore();
t451=top();
S_DATA(t451)[3]=t460;
t451=restore();
restore();
t444=top();
S_DATA(t444)[3]=t451;
t444=restore();
restore();
t443=top();
S_DATA(t443)[0]=t444;
t444=pick(2);
X t465=TO_N(2); // ICONST
save(t465);
X t466=select1(TO_N(2),t444); // SELECT (simple)
t465=restore();
X t464=select1(t465,t466); // SELECT
t443=top();
S_DATA(t443)[1]=t464;
t443=restore();
restore();
t417=t443;
}else{
save(x); // COND
save(x);
X t468=allocate(3); // CONS
save(t468);
x=pick(2);
X t469=___id(x); // REF: id
t468=top();
S_DATA(t468)[0]=t469;
t469=pick(2);
X t470=lf[90]; // CONST
t468=top();
S_DATA(t468)[1]=t470;
t470=pick(2);
X t471=lf[73]; // CONST
t468=top();
S_DATA(t468)[2]=t471;
t468=restore();
restore();
X t472=___parse_3amatchval(t468); // REF: parse:matchval
X t467;
x=restore();
if(t472!=F){
save(x);
X t473=allocate(2); // CONS
save(t473);
x=pick(2);
X t475=TO_N(2); // ICONST
save(t475);
X t477=TO_N(1); // ICONST
save(t477);
X t478=select1(TO_N(1),x); // SELECT (simple)
t477=restore();
X t476=select1(t477,t478); // SELECT
t475=restore();
X t474=select1(t475,t476); // SELECT
t473=top();
S_DATA(t473)[0]=t474;
t474=pick(2);
X t479=___parse_3aadvance(t474); // REF: parse:advance
X t480=___parse_3ap_5fvalue(t479); // REF: parse:p_value
t473=top();
S_DATA(t473)[1]=t480;
t473=restore();
restore();
save(t473);
X t481=allocate(2); // CONS
save(t481);
t473=pick(2);
save(t473);
X t482=allocate(3); // CONS
save(t482);
t473=pick(2);
X t483=lf[323]; // CONST
t482=top();
S_DATA(t482)[0]=t483;
t483=pick(2);
X t484=select1(TO_N(1),t483); // SELECT (simple)
t482=top();
S_DATA(t482)[1]=t484;
t484=pick(2);
X t486=TO_N(1); // ICONST
save(t486);
X t487=select1(TO_N(2),t484); // SELECT (simple)
t486=restore();
X t485=select1(t486,t487); // SELECT
t482=top();
S_DATA(t482)[2]=t485;
t482=restore();
restore();
t481=top();
S_DATA(t481)[0]=t482;
t482=pick(2);
X t489=TO_N(2); // ICONST
save(t489);
X t490=select1(TO_N(2),t482); // SELECT (simple)
t489=restore();
X t488=select1(t489,t490); // SELECT
t481=top();
S_DATA(t481)[1]=t488;
t481=restore();
restore();
t467=t481;
}else{
save(x); // COND
save(x);
X t492=allocate(3); // CONS
save(t492);
x=pick(2);
X t493=___id(x); // REF: id
t492=top();
S_DATA(t492)[0]=t493;
t493=pick(2);
X t494=lf[80]; // CONST
t492=top();
S_DATA(t492)[1]=t494;
t494=pick(2);
X t495=lf[74]; // CONST
t492=top();
S_DATA(t492)[2]=t495;
t492=restore();
restore();
X t496=___parse_3amatchval(t492); // REF: parse:matchval
X t491;
x=restore();
if(t496!=F){
save(x);
X t497=allocate(2); // CONS
save(t497);
x=pick(2);
X t499=TO_N(2); // ICONST
save(t499);
X t501=TO_N(1); // ICONST
save(t501);
X t502=select1(TO_N(1),x); // SELECT (simple)
t501=restore();
X t500=select1(t501,t502); // SELECT
t499=restore();
X t498=select1(t499,t500); // SELECT
t497=top();
S_DATA(t497)[0]=t498;
t498=pick(2);
X t503=___parse_3aadvance(t498); // REF: parse:advance
X t504=___parse_3ap_5fvalue(t503); // REF: parse:p_value
save(t504); // COND
save(t504);
X t506=allocate(3); // CONS
save(t506);
t504=pick(2);
X t507=select1(TO_N(2),t504); // SELECT (simple)
t506=top();
S_DATA(t506)[0]=t507;
t507=pick(2);
X t508=lf[90]; // CONST
t506=top();
S_DATA(t506)[1]=t508;
t508=pick(2);
X t509=lf[76]; // CONST
t506=top();
S_DATA(t506)[2]=t509;
t506=restore();
restore();
X t510=___parse_3amatchval(t506); // REF: parse:matchval
X t505;
t504=restore();
if(t510!=F){
save(t504);
X t511=allocate(3); // CONS
save(t511);
t504=pick(2);
X t512=select1(TO_N(1),t504); // SELECT (simple)
t511=top();
S_DATA(t511)[0]=t512;
t512=pick(2);
X t514=TO_N(2); // ICONST
save(t514);
X t516=TO_N(1); // ICONST
save(t516);
X t518=TO_N(1); // ICONST
save(t518);
X t519=select1(TO_N(2),t512); // SELECT (simple)
t518=restore();
X t517=select1(t518,t519); // SELECT
t516=restore();
X t515=select1(t516,t517); // SELECT
t514=restore();
X t513=select1(t514,t515); // SELECT
t511=top();
S_DATA(t511)[1]=t513;
t513=pick(2);
X t520=select1(TO_N(2),t513); // SELECT (simple)
X t521=___parse_3aadvance(t520); // REF: parse:advance
X t522=___parse_3ap_5fexpression(t521); // REF: parse:p_expression
t511=top();
S_DATA(t511)[2]=t522;
t511=restore();
restore();
save(t511);
X t523=allocate(2); // CONS
save(t523);
t511=pick(2);
save(t511);
X t524=allocate(4); // CONS
save(t524);
t511=pick(2);
X t525=lf[320]; // CONST
t524=top();
S_DATA(t524)[0]=t525;
t525=pick(2);
X t526=select1(TO_N(2),t525); // SELECT (simple)
t524=top();
S_DATA(t524)[1]=t526;
t526=pick(2);
X t527=select1(TO_N(1),t526); // SELECT (simple)
t524=top();
S_DATA(t524)[2]=t527;
t527=pick(2);
X t529=TO_N(1); // ICONST
save(t529);
X t530=select1(TO_N(3),t527); // SELECT (simple)
t529=restore();
X t528=select1(t529,t530); // SELECT
t524=top();
S_DATA(t524)[3]=t528;
t524=restore();
restore();
t523=top();
S_DATA(t523)[0]=t524;
t524=pick(2);
X t532=TO_N(2); // ICONST
save(t532);
X t533=select1(TO_N(3),t524); // SELECT (simple)
t532=restore();
X t531=select1(t532,t533); // SELECT
t523=top();
S_DATA(t523)[1]=t531;
t523=restore();
restore();
t505=t523;
}else{
X t534=___id(t504); // REF: id
t505=t534;}
t497=top();
S_DATA(t497)[1]=t505;
t497=restore();
restore();
save(t497);
X t535=allocate(3); // CONS
save(t535);
t497=pick(2);
X t536=select1(TO_N(1),t497); // SELECT (simple)
t535=top();
S_DATA(t535)[0]=t536;
t536=pick(2);
X t538=TO_N(1); // ICONST
save(t538);
X t539=select1(TO_N(2),t536); // SELECT (simple)
t538=restore();
X t537=select1(t538,t539); // SELECT
t535=top();
S_DATA(t535)[1]=t537;
t537=pick(2);
X t541=TO_N(2); // ICONST
save(t541);
X t542=select1(TO_N(2),t537); // SELECT (simple)
t541=restore();
X t540=select1(t541,t542); // SELECT
X t543=___parse_3ap_5fvalue(t540); // REF: parse:p_value
save(t543); // COND
save(t543);
X t545=allocate(3); // CONS
save(t545);
t543=pick(2);
X t546=select1(TO_N(2),t543); // SELECT (simple)
t545=top();
S_DATA(t545)[0]=t546;
t546=pick(2);
X t547=lf[90]; // CONST
t545=top();
S_DATA(t545)[1]=t547;
t547=pick(2);
X t548=lf[76]; // CONST
t545=top();
S_DATA(t545)[2]=t548;
t545=restore();
restore();
X t549=___parse_3amatchval(t545); // REF: parse:matchval
X t544;
t543=restore();
if(t549!=F){
save(t543);
X t550=allocate(3); // CONS
save(t550);
t543=pick(2);
X t551=select1(TO_N(1),t543); // SELECT (simple)
t550=top();
S_DATA(t550)[0]=t551;
t551=pick(2);
X t553=TO_N(2); // ICONST
save(t553);
X t555=TO_N(1); // ICONST
save(t555);
X t557=TO_N(1); // ICONST
save(t557);
X t558=select1(TO_N(2),t551); // SELECT (simple)
t557=restore();
X t556=select1(t557,t558); // SELECT
t555=restore();
X t554=select1(t555,t556); // SELECT
t553=restore();
X t552=select1(t553,t554); // SELECT
t550=top();
S_DATA(t550)[1]=t552;
t552=pick(2);
X t559=select1(TO_N(2),t552); // SELECT (simple)
X t560=___parse_3aadvance(t559); // REF: parse:advance
X t561=___parse_3ap_5fexpression(t560); // REF: parse:p_expression
t550=top();
S_DATA(t550)[2]=t561;
t550=restore();
restore();
save(t550);
X t562=allocate(2); // CONS
save(t562);
t550=pick(2);
save(t550);
X t563=allocate(4); // CONS
save(t563);
t550=pick(2);
X t564=lf[320]; // CONST
t563=top();
S_DATA(t563)[0]=t564;
t564=pick(2);
X t565=select1(TO_N(2),t564); // SELECT (simple)
t563=top();
S_DATA(t563)[1]=t565;
t565=pick(2);
X t566=select1(TO_N(1),t565); // SELECT (simple)
t563=top();
S_DATA(t563)[2]=t566;
t566=pick(2);
X t568=TO_N(1); // ICONST
save(t568);
X t569=select1(TO_N(3),t566); // SELECT (simple)
t568=restore();
X t567=select1(t568,t569); // SELECT
t563=top();
S_DATA(t563)[3]=t567;
t563=restore();
restore();
t562=top();
S_DATA(t562)[0]=t563;
t563=pick(2);
X t571=TO_N(2); // ICONST
save(t571);
X t572=select1(TO_N(3),t563); // SELECT (simple)
t571=restore();
X t570=select1(t571,t572); // SELECT
t562=top();
S_DATA(t562)[1]=t570;
t562=restore();
restore();
t544=t562;
}else{
X t573=___id(t543); // REF: id
t544=t573;}
t535=top();
S_DATA(t535)[2]=t544;
t535=restore();
restore();
save(t535);
X t574=allocate(2); // CONS
save(t574);
t535=pick(2);
save(t535);
X t575=allocate(4); // CONS
save(t575);
t535=pick(2);
X t576=lf[305]; // CONST
t575=top();
S_DATA(t575)[0]=t576;
t576=pick(2);
X t577=select1(TO_N(1),t576); // SELECT (simple)
t575=top();
S_DATA(t575)[1]=t577;
t577=pick(2);
X t578=select1(TO_N(2),t577); // SELECT (simple)
t575=top();
S_DATA(t575)[2]=t578;
t578=pick(2);
X t580=TO_N(1); // ICONST
save(t580);
X t581=select1(TO_N(3),t578); // SELECT (simple)
t580=restore();
X t579=select1(t580,t581); // SELECT
t575=top();
S_DATA(t575)[3]=t579;
t575=restore();
restore();
t574=top();
S_DATA(t574)[0]=t575;
t575=pick(2);
X t583=TO_N(2); // ICONST
save(t583);
X t584=select1(TO_N(3),t575); // SELECT (simple)
t583=restore();
X t582=select1(t583,t584); // SELECT
t574=top();
S_DATA(t574)[1]=t582;
t574=restore();
restore();
t491=t574;
}else{
save(x); // COND
save(x);
X t586=allocate(3); // CONS
save(t586);
x=pick(2);
X t587=___id(x); // REF: id
t586=top();
S_DATA(t586)[0]=t587;
t587=pick(2);
X t588=lf[80]; // CONST
t586=top();
S_DATA(t586)[1]=t588;
t588=pick(2);
X t589=lf[75]; // CONST
t586=top();
S_DATA(t586)[2]=t589;
t586=restore();
restore();
X t590=___parse_3amatchval(t586); // REF: parse:matchval
X t585;
x=restore();
if(t590!=F){
save(x);
X t591=allocate(2); // CONS
save(t591);
x=pick(2);
X t593=TO_N(2); // ICONST
save(t593);
X t595=TO_N(1); // ICONST
save(t595);
X t596=select1(TO_N(1),x); // SELECT (simple)
t595=restore();
X t594=select1(t595,t596); // SELECT
t593=restore();
X t592=select1(t593,t594); // SELECT
t591=top();
S_DATA(t591)[0]=t592;
t592=pick(2);
X t597=___parse_3aadvance(t592); // REF: parse:advance
X t598=___parse_3ap_5fvalue(t597); // REF: parse:p_value
save(t598); // COND
save(t598);
X t600=allocate(3); // CONS
save(t600);
t598=pick(2);
X t601=select1(TO_N(2),t598); // SELECT (simple)
t600=top();
S_DATA(t600)[0]=t601;
t601=pick(2);
X t602=lf[90]; // CONST
t600=top();
S_DATA(t600)[1]=t602;
t602=pick(2);
X t603=lf[76]; // CONST
t600=top();
S_DATA(t600)[2]=t603;
t600=restore();
restore();
X t604=___parse_3amatchval(t600); // REF: parse:matchval
X t599;
t598=restore();
if(t604!=F){
save(t598);
X t605=allocate(3); // CONS
save(t605);
t598=pick(2);
X t606=select1(TO_N(1),t598); // SELECT (simple)
t605=top();
S_DATA(t605)[0]=t606;
t606=pick(2);
X t608=TO_N(2); // ICONST
save(t608);
X t610=TO_N(1); // ICONST
save(t610);
X t612=TO_N(1); // ICONST
save(t612);
X t613=select1(TO_N(2),t606); // SELECT (simple)
t612=restore();
X t611=select1(t612,t613); // SELECT
t610=restore();
X t609=select1(t610,t611); // SELECT
t608=restore();
X t607=select1(t608,t609); // SELECT
t605=top();
S_DATA(t605)[1]=t607;
t607=pick(2);
X t614=select1(TO_N(2),t607); // SELECT (simple)
X t615=___parse_3aadvance(t614); // REF: parse:advance
X t616=___parse_3ap_5fexpression(t615); // REF: parse:p_expression
t605=top();
S_DATA(t605)[2]=t616;
t605=restore();
restore();
save(t605);
X t617=allocate(2); // CONS
save(t617);
t605=pick(2);
save(t605);
X t618=allocate(4); // CONS
save(t618);
t605=pick(2);
X t619=lf[320]; // CONST
t618=top();
S_DATA(t618)[0]=t619;
t619=pick(2);
X t620=select1(TO_N(2),t619); // SELECT (simple)
t618=top();
S_DATA(t618)[1]=t620;
t620=pick(2);
X t621=select1(TO_N(1),t620); // SELECT (simple)
t618=top();
S_DATA(t618)[2]=t621;
t621=pick(2);
X t623=TO_N(1); // ICONST
save(t623);
X t624=select1(TO_N(3),t621); // SELECT (simple)
t623=restore();
X t622=select1(t623,t624); // SELECT
t618=top();
S_DATA(t618)[3]=t622;
t618=restore();
restore();
t617=top();
S_DATA(t617)[0]=t618;
t618=pick(2);
X t626=TO_N(2); // ICONST
save(t626);
X t627=select1(TO_N(3),t618); // SELECT (simple)
t626=restore();
X t625=select1(t626,t627); // SELECT
t617=top();
S_DATA(t617)[1]=t625;
t617=restore();
restore();
t599=t617;
}else{
X t628=___id(t598); // REF: id
t599=t628;}
t591=top();
S_DATA(t591)[1]=t599;
t591=restore();
restore();
save(t591);
X t629=allocate(3); // CONS
save(t629);
t591=pick(2);
X t630=select1(TO_N(1),t591); // SELECT (simple)
t629=top();
S_DATA(t629)[0]=t630;
t630=pick(2);
X t632=TO_N(1); // ICONST
save(t632);
X t633=select1(TO_N(2),t630); // SELECT (simple)
t632=restore();
X t631=select1(t632,t633); // SELECT
t629=top();
S_DATA(t629)[1]=t631;
t631=pick(2);
X t635=TO_N(2); // ICONST
save(t635);
X t636=select1(TO_N(2),t631); // SELECT (simple)
t635=restore();
X t634=select1(t635,t636); // SELECT
X t637=___parse_3ap_5fvalue(t634); // REF: parse:p_value
save(t637); // COND
save(t637);
X t639=allocate(3); // CONS
save(t639);
t637=pick(2);
X t640=select1(TO_N(2),t637); // SELECT (simple)
t639=top();
S_DATA(t639)[0]=t640;
t640=pick(2);
X t641=lf[90]; // CONST
t639=top();
S_DATA(t639)[1]=t641;
t641=pick(2);
X t642=lf[76]; // CONST
t639=top();
S_DATA(t639)[2]=t642;
t639=restore();
restore();
X t643=___parse_3amatchval(t639); // REF: parse:matchval
X t638;
t637=restore();
if(t643!=F){
save(t637);
X t644=allocate(3); // CONS
save(t644);
t637=pick(2);
X t645=select1(TO_N(1),t637); // SELECT (simple)
t644=top();
S_DATA(t644)[0]=t645;
t645=pick(2);
X t647=TO_N(2); // ICONST
save(t647);
X t649=TO_N(1); // ICONST
save(t649);
X t651=TO_N(1); // ICONST
save(t651);
X t652=select1(TO_N(2),t645); // SELECT (simple)
t651=restore();
X t650=select1(t651,t652); // SELECT
t649=restore();
X t648=select1(t649,t650); // SELECT
t647=restore();
X t646=select1(t647,t648); // SELECT
t644=top();
S_DATA(t644)[1]=t646;
t646=pick(2);
X t653=select1(TO_N(2),t646); // SELECT (simple)
X t654=___parse_3aadvance(t653); // REF: parse:advance
X t655=___parse_3ap_5fexpression(t654); // REF: parse:p_expression
t644=top();
S_DATA(t644)[2]=t655;
t644=restore();
restore();
save(t644);
X t656=allocate(2); // CONS
save(t656);
t644=pick(2);
save(t644);
X t657=allocate(4); // CONS
save(t657);
t644=pick(2);
X t658=lf[320]; // CONST
t657=top();
S_DATA(t657)[0]=t658;
t658=pick(2);
X t659=select1(TO_N(2),t658); // SELECT (simple)
t657=top();
S_DATA(t657)[1]=t659;
t659=pick(2);
X t660=select1(TO_N(1),t659); // SELECT (simple)
t657=top();
S_DATA(t657)[2]=t660;
t660=pick(2);
X t662=TO_N(1); // ICONST
save(t662);
X t663=select1(TO_N(3),t660); // SELECT (simple)
t662=restore();
X t661=select1(t662,t663); // SELECT
t657=top();
S_DATA(t657)[3]=t661;
t657=restore();
restore();
t656=top();
S_DATA(t656)[0]=t657;
t657=pick(2);
X t665=TO_N(2); // ICONST
save(t665);
X t666=select1(TO_N(3),t657); // SELECT (simple)
t665=restore();
X t664=select1(t665,t666); // SELECT
t656=top();
S_DATA(t656)[1]=t664;
t656=restore();
restore();
t638=t656;
}else{
X t667=___id(t637); // REF: id
t638=t667;}
t629=top();
S_DATA(t629)[2]=t638;
t629=restore();
restore();
save(t629);
X t668=allocate(2); // CONS
save(t668);
t629=pick(2);
save(t629);
X t669=allocate(4); // CONS
save(t669);
t629=pick(2);
X t670=lf[307]; // CONST
t669=top();
S_DATA(t669)[0]=t670;
t670=pick(2);
X t671=select1(TO_N(1),t670); // SELECT (simple)
t669=top();
S_DATA(t669)[1]=t671;
t671=pick(2);
X t672=select1(TO_N(2),t671); // SELECT (simple)
t669=top();
S_DATA(t669)[2]=t672;
t672=pick(2);
X t674=TO_N(1); // ICONST
save(t674);
X t675=select1(TO_N(3),t672); // SELECT (simple)
t674=restore();
X t673=select1(t674,t675); // SELECT
t669=top();
S_DATA(t669)[3]=t673;
t669=restore();
restore();
t668=top();
S_DATA(t668)[0]=t669;
t669=pick(2);
X t677=TO_N(2); // ICONST
save(t677);
X t678=select1(TO_N(3),t669); // SELECT (simple)
t677=restore();
X t676=select1(t677,t678); // SELECT
t668=top();
S_DATA(t668)[1]=t676;
t668=restore();
restore();
t585=t668;
}else{
save(x); // COND
save(x);
X t680=allocate(3); // CONS
save(t680);
x=pick(2);
X t681=___id(x); // REF: id
t680=top();
S_DATA(t680)[0]=t681;
t681=pick(2);
X t682=lf[80]; // CONST
t680=top();
S_DATA(t680)[1]=t682;
t682=pick(2);
X t683=lf[77]; // CONST
t680=top();
S_DATA(t680)[2]=t683;
t680=restore();
restore();
X t684=___parse_3amatchval(t680); // REF: parse:matchval
X t679;
x=restore();
if(t684!=F){
save(x);
X t685=allocate(2); // CONS
save(t685);
x=pick(2);
X t687=TO_N(2); // ICONST
save(t687);
X t689=TO_N(1); // ICONST
save(t689);
X t690=select1(TO_N(1),x); // SELECT (simple)
t689=restore();
X t688=select1(t689,t690); // SELECT
t687=restore();
X t686=select1(t687,t688); // SELECT
t685=top();
S_DATA(t685)[0]=t686;
t686=pick(2);
X t691=___parse_3aadvance(t686); // REF: parse:advance
X t692=___parse_3ap_5fvalue(t691); // REF: parse:p_value
t685=top();
S_DATA(t685)[1]=t692;
t685=restore();
restore();
save(t685);
X t693=allocate(2); // CONS
save(t693);
t685=pick(2);
save(t685);
X t694=allocate(3); // CONS
save(t694);
t685=pick(2);
X t695=lf[313]; // CONST
t694=top();
S_DATA(t694)[0]=t695;
t695=pick(2);
X t696=select1(TO_N(1),t695); // SELECT (simple)
t694=top();
S_DATA(t694)[1]=t696;
t696=pick(2);
X t698=TO_N(1); // ICONST
save(t698);
X t699=select1(TO_N(2),t696); // SELECT (simple)
t698=restore();
X t697=select1(t698,t699); // SELECT
t694=top();
S_DATA(t694)[2]=t697;
t694=restore();
restore();
t693=top();
S_DATA(t693)[0]=t694;
t694=pick(2);
X t701=TO_N(2); // ICONST
save(t701);
X t702=select1(TO_N(2),t694); // SELECT (simple)
t701=restore();
X t700=select1(t701,t702); // SELECT
t693=top();
S_DATA(t693)[1]=t700;
t693=restore();
restore();
t679=t693;
}else{
save(x); // COND
save(x);
X t704=allocate(3); // CONS
save(t704);
x=pick(2);
X t705=___id(x); // REF: id
t704=top();
S_DATA(t704)[0]=t705;
t705=pick(2);
X t706=lf[80]; // CONST
t704=top();
S_DATA(t704)[1]=t706;
t706=pick(2);
X t707=lf[78]; // CONST
t704=top();
S_DATA(t704)[2]=t707;
t704=restore();
restore();
X t708=___parse_3amatchval(t704); // REF: parse:matchval
X t703;
x=restore();
if(t708!=F){
save(x);
X t709=allocate(2); // CONS
save(t709);
x=pick(2);
X t711=TO_N(2); // ICONST
save(t711);
X t713=TO_N(1); // ICONST
save(t713);
X t714=select1(TO_N(1),x); // SELECT (simple)
t713=restore();
X t712=select1(t713,t714); // SELECT
t711=restore();
X t710=select1(t711,t712); // SELECT
t709=top();
S_DATA(t709)[0]=t710;
t710=pick(2);
X t715=___parse_3aadvance(t710); // REF: parse:advance
X t716=___parse_3ap_5fconstant(t715); // REF: parse:p_constant
t709=top();
S_DATA(t709)[1]=t716;
t709=restore();
restore();
save(t709);
X t717=allocate(2); // CONS
save(t717);
t709=pick(2);
save(t709);
X t718=allocate(3); // CONS
save(t718);
t709=pick(2);
X t719=lf[311]; // CONST
t718=top();
S_DATA(t718)[0]=t719;
t719=pick(2);
X t720=select1(TO_N(1),t719); // SELECT (simple)
t718=top();
S_DATA(t718)[1]=t720;
t720=pick(2);
X t722=TO_N(1); // ICONST
save(t722);
X t723=select1(TO_N(2),t720); // SELECT (simple)
t722=restore();
X t721=select1(t722,t723); // SELECT
t718=top();
S_DATA(t718)[2]=t721;
t718=restore();
restore();
t717=top();
S_DATA(t717)[0]=t718;
t718=pick(2);
X t725=TO_N(2); // ICONST
save(t725);
X t726=select1(TO_N(2),t718); // SELECT (simple)
t725=restore();
X t724=select1(t725,t726); // SELECT
t717=top();
S_DATA(t717)[1]=t724;
t717=restore();
restore();
t703=t717;
}else{
save(x);
X t727=allocate(2); // CONS
save(t727);
x=pick(2);
X t729=TO_N(1); // ICONST
save(t729);
X t730=select1(TO_N(1),x); // SELECT (simple)
t729=restore();
X t728=select1(t729,t730); // SELECT
t727=top();
S_DATA(t727)[0]=t728;
t728=pick(2);
save(t728);
X t731=allocate(2); // CONS
save(t731);
t728=pick(2);
X t732=___id(t728); // REF: id
t731=top();
S_DATA(t731)[0]=t732;
t732=pick(2);
X t733=lf[80]; // CONST
t731=top();
S_DATA(t731)[1]=t733;
t731=restore();
restore();
X t734=___parse_3aexpect(t731); // REF: parse:expect
X t735=___parse_3aadvance(t734); // REF: parse:advance
t727=top();
S_DATA(t727)[1]=t735;
t727=restore();
restore();
save(t727); // COND
save(t727);
X t737=allocate(3); // CONS
save(t737);
t727=pick(2);
X t738=select1(TO_N(2),t727); // SELECT (simple)
t737=top();
S_DATA(t737)[0]=t738;
t738=pick(2);
X t739=lf[90]; // CONST
t737=top();
S_DATA(t737)[1]=t739;
t739=pick(2);
X t740=lf[79]; // CONST
t737=top();
S_DATA(t737)[2]=t740;
t737=restore();
restore();
X t741=___parse_3amatchval(t737); // REF: parse:matchval
X t736;
t727=restore();
if(t741!=F){
save(t727);
X t742=allocate(3); // CONS
save(t742);
t727=pick(2);
X t743=select1(TO_N(1),t727); // SELECT (simple)
t742=top();
S_DATA(t742)[0]=t743;
t743=pick(2);
X t745=TO_N(2); // ICONST
save(t745);
X t747=TO_N(1); // ICONST
save(t747);
X t749=TO_N(1); // ICONST
save(t749);
X t750=select1(TO_N(2),t743); // SELECT (simple)
t749=restore();
X t748=select1(t749,t750); // SELECT
t747=restore();
X t746=select1(t747,t748); // SELECT
t745=restore();
X t744=select1(t745,t746); // SELECT
t742=top();
S_DATA(t742)[1]=t744;
t744=pick(2);
save(t744);
X t751=allocate(2); // CONS
save(t751);
t744=pick(2);
X t752=select1(TO_N(2),t744); // SELECT (simple)
X t753=___parse_3aadvance(t752); // REF: parse:advance
t751=top();
S_DATA(t751)[0]=t753;
t753=pick(2);
X t754=lf[80]; // CONST
t751=top();
S_DATA(t751)[1]=t754;
t751=restore();
restore();
X t755=___parse_3aexpect(t751); // REF: parse:expect
t742=top();
S_DATA(t742)[2]=t755;
t742=restore();
restore();
save(t742);
X t756=allocate(2); // CONS
save(t756);
t742=pick(2);
save(t742);
X t757=allocate(3); // CONS
save(t757);
t742=pick(2);
X t758=lf[293]; // CONST
t757=top();
S_DATA(t757)[0]=t758;
t758=pick(2);
X t759=select1(TO_N(2),t758); // SELECT (simple)
t757=top();
S_DATA(t757)[1]=t759;
t759=pick(2);
save(t759);
X t760=allocate(2); // CONS
save(t760);
t759=pick(2);
X t762=TO_N(3); // ICONST
save(t762);
X t763=select1(TO_N(1),t759); // SELECT (simple)
t762=restore();
X t761=select1(t762,t763); // SELECT
t760=top();
S_DATA(t760)[0]=t761;
t761=pick(2);
X t764=select1(TO_N(3),t761); // SELECT (simple)
X t765=___parse_3anextval(t764); // REF: parse:nextval
t760=top();
S_DATA(t760)[1]=t765;
t760=restore();
restore();
X t766=___parse_3aprefix(t760); // REF: parse:prefix
t757=top();
S_DATA(t757)[2]=t766;
t757=restore();
restore();
t756=top();
S_DATA(t756)[0]=t757;
t757=pick(2);
X t767=select1(TO_N(3),t757); // SELECT (simple)
X t768=___parse_3aadvance(t767); // REF: parse:advance
t756=top();
S_DATA(t756)[1]=t768;
t756=restore();
restore();
t736=t756;
}else{
save(t727);
X t769=allocate(2); // CONS
save(t769);
t727=pick(2);
save(t727);
X t770=allocate(3); // CONS
save(t770);
t727=pick(2);
X t771=lf[293]; // CONST
t770=top();
S_DATA(t770)[0]=t771;
t771=pick(2);
X t773=TO_N(2); // ICONST
save(t773);
X t774=select1(TO_N(1),t771); // SELECT (simple)
t773=restore();
X t772=select1(t773,t774); // SELECT
t770=top();
S_DATA(t770)[1]=t772;
t772=pick(2);
X t776=TO_N(3); // ICONST
save(t776);
X t777=select1(TO_N(1),t772); // SELECT (simple)
t776=restore();
X t775=select1(t776,t777); // SELECT
t770=top();
S_DATA(t770)[2]=t775;
t770=restore();
restore();
t769=top();
S_DATA(t769)[0]=t770;
t770=pick(2);
X t778=select1(TO_N(2),t770); // SELECT (simple)
t769=top();
S_DATA(t769)[1]=t778;
t769=restore();
restore();
t736=t769;}
t703=t736;}
t679=t703;}
t585=t679;}
t491=t585;}
t467=t491;}
t417=t467;}
t379=t417;}
t363=t379;}
t325=t363;}
t301=t325;}
t277=t301;}
t253=t277;}
t205=t253;}
t160=t205;}
t129=t160;}
t97=t129;}
t1=t97;}
RETURN(t1);}
//---------------------------------------- parse:p_val_cons' (parse.fp:209)
DEFINE(___parse_3ap_5fval_5fcons_27){
ENTRY;
loop:;
tracecall("parse.fp:209:  parse:p_val_cons'");
save(x); // COND
save(x);
X t2=allocate(3); // CONS
save(t2);
x=pick(2);
X t4=TO_N(2); // ICONST
save(t4);
X t5=select1(TO_N(3),x); // SELECT (simple)
t4=restore();
X t3=select1(t4,t5); // SELECT
t2=top();
S_DATA(t2)[0]=t3;
t3=pick(2);
X t6=lf[90]; // CONST
t2=top();
S_DATA(t2)[1]=t6;
t6=pick(2);
X t7=lf[81]; // CONST
t2=top();
S_DATA(t2)[2]=t7;
t2=restore();
restore();
X t8=___parse_3amatchval(t2); // REF: parse:matchval
X t1;
x=restore();
if(t8!=F){
save(x);
X t9=allocate(2); // CONS
save(t9);
x=pick(2);
save(x);
X t10=allocate(2); // CONS
save(t10);
x=pick(2);
save(x);
X t11=allocate(2); // CONS
save(t11);
x=pick(2);
X t12=lf[326]; // CONST
t11=top();
S_DATA(t11)[0]=t12;
t12=pick(2);
X t13=select1(TO_N(1),t12); // SELECT (simple)
t11=top();
S_DATA(t11)[1]=t13;
t11=restore();
restore();
t10=top();
S_DATA(t10)[0]=t11;
t11=pick(2);
save(t11);
X t14=allocate(2); // CONS
save(t14);
t11=pick(2);
X t15=select1(TO_N(2),t11); // SELECT (simple)
t14=top();
S_DATA(t14)[0]=t15;
t15=pick(2);
X t17=TO_N(1); // ICONST
save(t17);
X t18=select1(TO_N(3),t15); // SELECT (simple)
t17=restore();
X t16=select1(t17,t18); // SELECT
t14=top();
S_DATA(t14)[1]=t16;
t14=restore();
restore();
X t19=___ar(t14); // REF: ar
t10=top();
S_DATA(t10)[1]=t19;
t10=restore();
restore();
X t20=___cat(t10); // REF: cat
t9=top();
S_DATA(t9)[0]=t20;
t20=pick(2);
X t22=TO_N(2); // ICONST
save(t22);
X t23=select1(TO_N(3),t20); // SELECT (simple)
t22=restore();
X t21=select1(t22,t23); // SELECT
X t24=___parse_3aadvance(t21); // REF: parse:advance
t9=top();
S_DATA(t9)[1]=t24;
t9=restore();
restore();
t1=t9;
}else{
save(x); // COND
save(x);
X t26=allocate(3); // CONS
save(t26);
x=pick(2);
X t28=TO_N(2); // ICONST
save(t28);
X t29=select1(TO_N(3),x); // SELECT (simple)
t28=restore();
X t27=select1(t28,t29); // SELECT
t26=top();
S_DATA(t26)[0]=t27;
t27=pick(2);
X t30=lf[90]; // CONST
t26=top();
S_DATA(t26)[1]=t30;
t30=pick(2);
X t31=lf[91]; // CONST
t26=top();
S_DATA(t26)[2]=t31;
t26=restore();
restore();
X t32=___parse_3amatchval(t26); // REF: parse:matchval
X t25;
x=restore();
if(t32!=F){
save(x);
X t33=allocate(3); // CONS
save(t33);
x=pick(2);
X t34=select1(TO_N(1),x); // SELECT (simple)
t33=top();
S_DATA(t33)[0]=t34;
t34=pick(2);
save(t34);
X t35=allocate(2); // CONS
save(t35);
t34=pick(2);
X t36=select1(TO_N(2),t34); // SELECT (simple)
t35=top();
S_DATA(t35)[0]=t36;
t36=pick(2);
X t38=TO_N(1); // ICONST
save(t38);
X t39=select1(TO_N(3),t36); // SELECT (simple)
t38=restore();
X t37=select1(t38,t39); // SELECT
t35=top();
S_DATA(t35)[1]=t37;
t35=restore();
restore();
X t40=___ar(t35); // REF: ar
t33=top();
S_DATA(t33)[1]=t40;
t40=pick(2);
X t42=TO_N(2); // ICONST
save(t42);
X t43=select1(TO_N(3),t40); // SELECT (simple)
t42=restore();
X t41=select1(t42,t43); // SELECT
X t44=___parse_3aadvance(t41); // REF: parse:advance
X t45=___parse_3ap_5fseq(t44); // REF: parse:p_seq
t33=top();
S_DATA(t33)[2]=t45;
t33=restore();
restore();
x=t33; // REF: parse:p_val_cons'
goto loop; // tail call: parse:p_val_cons'
X t46;
t25=t46;
}else{
save(x);
X t47=allocate(2); // CONS
save(t47);
x=pick(2);
X t48=lf[82]; // CONST
t47=top();
S_DATA(t47)[0]=t48;
t48=pick(2);
X t50=TO_N(2); // ICONST
save(t50);
X t51=select1(TO_N(3),t48); // SELECT (simple)
t50=restore();
X t49=select1(t50,t51); // SELECT
t47=top();
S_DATA(t47)[1]=t49;
t47=restore();
restore();
X t52=___parse_3afail(t47); // REF: parse:fail
t25=t52;}
t1=t25;}
RETURN(t1);}
//---------------------------------------- parse:p_val_pcons' (parse.fp:217)
DEFINE(___parse_3ap_5fval_5fpcons_27){
ENTRY;
loop:;
tracecall("parse.fp:217:  parse:p_val_pcons'");
save(x); // COND
save(x);
X t2=allocate(3); // CONS
save(t2);
x=pick(2);
X t4=TO_N(2); // ICONST
save(t4);
X t5=select1(TO_N(3),x); // SELECT (simple)
t4=restore();
X t3=select1(t4,t5); // SELECT
t2=top();
S_DATA(t2)[0]=t3;
t3=pick(2);
X t6=lf[90]; // CONST
t2=top();
S_DATA(t2)[1]=t6;
t6=pick(2);
X t7=lf[84]; // CONST
t2=top();
S_DATA(t2)[2]=t7;
t2=restore();
restore();
X t8=___parse_3amatchval(t2); // REF: parse:matchval
X t1;
x=restore();
if(t8!=F){
save(x);
X t9=allocate(2); // CONS
save(t9);
x=pick(2);
save(x);
X t10=allocate(2); // CONS
save(t10);
x=pick(2);
save(x);
X t11=allocate(2); // CONS
save(t11);
x=pick(2);
X t12=lf[329]; // CONST
t11=top();
S_DATA(t11)[0]=t12;
t12=pick(2);
X t13=select1(TO_N(1),t12); // SELECT (simple)
t11=top();
S_DATA(t11)[1]=t13;
t11=restore();
restore();
t10=top();
S_DATA(t10)[0]=t11;
t11=pick(2);
save(t11);
X t14=allocate(2); // CONS
save(t14);
t11=pick(2);
X t15=select1(TO_N(2),t11); // SELECT (simple)
t14=top();
S_DATA(t14)[0]=t15;
t15=pick(2);
X t17=TO_N(1); // ICONST
save(t17);
X t18=select1(TO_N(3),t15); // SELECT (simple)
t17=restore();
X t16=select1(t17,t18); // SELECT
t14=top();
S_DATA(t14)[1]=t16;
t14=restore();
restore();
X t19=___ar(t14); // REF: ar
t10=top();
S_DATA(t10)[1]=t19;
t10=restore();
restore();
X t20=___cat(t10); // REF: cat
t9=top();
S_DATA(t9)[0]=t20;
t20=pick(2);
X t22=TO_N(2); // ICONST
save(t22);
X t23=select1(TO_N(3),t20); // SELECT (simple)
t22=restore();
X t21=select1(t22,t23); // SELECT
X t24=___parse_3aadvance(t21); // REF: parse:advance
t9=top();
S_DATA(t9)[1]=t24;
t9=restore();
restore();
t1=t9;
}else{
save(x); // COND
save(x);
X t26=allocate(3); // CONS
save(t26);
x=pick(2);
X t28=TO_N(2); // ICONST
save(t28);
X t29=select1(TO_N(3),x); // SELECT (simple)
t28=restore();
X t27=select1(t28,t29); // SELECT
t26=top();
S_DATA(t26)[0]=t27;
t27=pick(2);
X t30=lf[90]; // CONST
t26=top();
S_DATA(t26)[1]=t30;
t30=pick(2);
X t31=lf[91]; // CONST
t26=top();
S_DATA(t26)[2]=t31;
t26=restore();
restore();
X t32=___parse_3amatchval(t26); // REF: parse:matchval
X t25;
x=restore();
if(t32!=F){
save(x);
X t33=allocate(4); // CONS
save(t33);
x=pick(2);
X t34=select1(TO_N(1),x); // SELECT (simple)
t33=top();
S_DATA(t33)[0]=t34;
t34=pick(2);
X t35=select1(TO_N(2),t34); // SELECT (simple)
t33=top();
S_DATA(t33)[1]=t35;
t35=pick(2);
X t37=TO_N(1); // ICONST
save(t37);
X t38=select1(TO_N(3),t35); // SELECT (simple)
t37=restore();
X t36=select1(t37,t38); // SELECT
t33=top();
S_DATA(t33)[2]=t36;
t36=pick(2);
X t40=TO_N(2); // ICONST
save(t40);
X t41=select1(TO_N(3),t36); // SELECT (simple)
t40=restore();
X t39=select1(t40,t41); // SELECT
X t42=___parse_3aadvance(t39); // REF: parse:advance
t33=top();
S_DATA(t33)[3]=t42;
t33=restore();
restore();
save(t33); // COND
save(t33);
X t44=allocate(3); // CONS
save(t44);
t33=pick(2);
X t45=select1(TO_N(4),t33); // SELECT (simple)
t44=top();
S_DATA(t44)[0]=t45;
t45=pick(2);
X t46=lf[90]; // CONST
t44=top();
S_DATA(t44)[1]=t46;
t46=pick(2);
X t47=lf[83]; // CONST
t44=top();
S_DATA(t44)[2]=t47;
t44=restore();
restore();
X t48=___parse_3amatchval(t44); // REF: parse:matchval
X t43;
t33=restore();
if(t48!=F){
save(t33);
X t49=allocate(2); // CONS
save(t49);
t33=pick(2);
save(t33);
X t50=allocate(2); // CONS
save(t50);
t33=pick(2);
save(t33);
X t51=allocate(2); // CONS
save(t51);
t33=pick(2);
X t52=lf[331]; // CONST
t51=top();
S_DATA(t51)[0]=t52;
t52=pick(2);
X t53=select1(TO_N(1),t52); // SELECT (simple)
t51=top();
S_DATA(t51)[1]=t53;
t51=restore();
restore();
t50=top();
S_DATA(t50)[0]=t51;
t51=pick(2);
save(t51);
X t54=allocate(2); // CONS
save(t54);
t51=pick(2);
X t55=select1(TO_N(2),t51); // SELECT (simple)
t54=top();
S_DATA(t54)[0]=t55;
t55=pick(2);
X t56=select1(TO_N(3),t55); // SELECT (simple)
t54=top();
S_DATA(t54)[1]=t56;
t54=restore();
restore();
X t57=___ar(t54); // REF: ar
t50=top();
S_DATA(t50)[1]=t57;
t50=restore();
restore();
X t58=___cat(t50); // REF: cat
t49=top();
S_DATA(t49)[0]=t58;
t58=pick(2);
save(t58);
X t59=allocate(3); // CONS
save(t59);
t58=pick(2);
X t60=select1(TO_N(4),t58); // SELECT (simple)
X t61=___parse_3aadvance(t60); // REF: parse:advance
t59=top();
S_DATA(t59)[0]=t61;
t61=pick(2);
X t62=lf[90]; // CONST
t59=top();
S_DATA(t59)[1]=t62;
t62=pick(2);
X t63=lf[84]; // CONST
t59=top();
S_DATA(t59)[2]=t63;
t59=restore();
restore();
X t64=___parse_3aexpectval(t59); // REF: parse:expectval
t49=top();
S_DATA(t49)[1]=t64;
t49=restore();
restore();
t43=t49;
}else{
save(t33);
X t65=allocate(3); // CONS
save(t65);
t33=pick(2);
X t66=select1(TO_N(1),t33); // SELECT (simple)
t65=top();
S_DATA(t65)[0]=t66;
t66=pick(2);
save(t66);
X t67=allocate(2); // CONS
save(t67);
t66=pick(2);
X t68=select1(TO_N(2),t66); // SELECT (simple)
t67=top();
S_DATA(t67)[0]=t68;
t68=pick(2);
X t69=select1(TO_N(3),t68); // SELECT (simple)
t67=top();
S_DATA(t67)[1]=t69;
t67=restore();
restore();
X t70=___ar(t67); // REF: ar
t65=top();
S_DATA(t65)[1]=t70;
t70=pick(2);
X t71=select1(TO_N(4),t70); // SELECT (simple)
X t72=___parse_3ap_5fseq(t71); // REF: parse:p_seq
t65=top();
S_DATA(t65)[2]=t72;
t65=restore();
restore();
x=t65; // REF: parse:p_val_pcons'
goto loop; // tail call: parse:p_val_pcons'
X t73;
t43=t73;}
t25=t43;
}else{
save(x);
X t74=allocate(2); // CONS
save(t74);
x=pick(2);
X t75=lf[85]; // CONST
t74=top();
S_DATA(t74)[0]=t75;
t75=pick(2);
X t77=TO_N(2); // ICONST
save(t77);
X t78=select1(TO_N(3),t75); // SELECT (simple)
t77=restore();
X t76=select1(t77,t78); // SELECT
t74=top();
S_DATA(t74)[1]=t76;
t74=restore();
restore();
X t79=___parse_3afail(t74); // REF: parse:fail
t25=t79;}
t1=t25;}
RETURN(t1);}
//---------------------------------------- parse:p_constant (parse.fp:234)
DEFINE(___parse_3ap_5fconstant){
ENTRY;
loop:;
tracecall("parse.fp:234:  parse:p_constant");
save(x); // COND
save(x);
X t2=allocate(2); // CONS
save(t2);
x=pick(2);
X t3=___id(x); // REF: id
t2=top();
S_DATA(t2)[0]=t3;
t3=pick(2);
X t4=lf[86]; // CONST
t2=top();
S_DATA(t2)[1]=t4;
t2=restore();
restore();
X t5=___parse_3amatch(t2); // REF: parse:match
X t1;
x=restore();
if(t5!=F){
save(x);
X t6=allocate(2); // CONS
save(t6);
x=pick(2);
X t7=___parse_3anextval(x); // REF: parse:nextval
t6=top();
S_DATA(t6)[0]=t7;
t7=pick(2);
X t8=___parse_3aadvance(t7); // REF: parse:advance
t6=top();
S_DATA(t6)[1]=t8;
t6=restore();
restore();
t1=t6;
}else{
save(x); // COND
save(x);
X t10=allocate(2); // CONS
save(t10);
x=pick(2);
X t11=___id(x); // REF: id
t10=top();
S_DATA(t10)[0]=t11;
t11=pick(2);
X t12=lf[87]; // CONST
t10=top();
S_DATA(t10)[1]=t12;
t10=restore();
restore();
X t13=___parse_3amatch(t10); // REF: parse:match
X t9;
x=restore();
if(t13!=F){
save(x);
X t14=allocate(2); // CONS
save(t14);
x=pick(2);
X t15=___parse_3anextval(x); // REF: parse:nextval
t14=top();
S_DATA(t14)[0]=t15;
t15=pick(2);
X t16=___parse_3aadvance(t15); // REF: parse:advance
t14=top();
S_DATA(t14)[1]=t16;
t14=restore();
restore();
t9=t14;
}else{
save(x); // COND
save(x);
X t18=allocate(3); // CONS
save(t18);
x=pick(2);
X t19=___id(x); // REF: id
t18=top();
S_DATA(t18)[0]=t19;
t19=pick(2);
X t20=lf[90]; // CONST
t18=top();
S_DATA(t18)[1]=t20;
t20=pick(2);
X t21=lf[88]; // CONST
t18=top();
S_DATA(t18)[2]=t21;
t18=restore();
restore();
X t22=___parse_3amatchval(t18); // REF: parse:matchval
X t17;
x=restore();
if(t22!=F){
X t23=___parse_3aadvance(x); // REF: parse:advance
save(t23); // COND
save(t23);
X t25=allocate(3); // CONS
save(t25);
t23=pick(2);
X t26=___id(t23); // REF: id
t25=top();
S_DATA(t25)[0]=t26;
t26=pick(2);
X t27=lf[90]; // CONST
t25=top();
S_DATA(t25)[1]=t27;
t27=pick(2);
X t28=lf[89]; // CONST
t25=top();
S_DATA(t25)[2]=t28;
t25=restore();
restore();
X t29=___parse_3amatchval(t25); // REF: parse:matchval
X t24;
t23=restore();
if(t29!=F){
save(t23);
X t30=allocate(2); // CONS
save(t30);
t23=pick(2);
X t31=EMPTY; // ICONST
t30=top();
S_DATA(t30)[0]=t31;
t31=pick(2);
X t32=___parse_3aadvance(t31); // REF: parse:advance
t30=top();
S_DATA(t30)[1]=t32;
t30=restore();
restore();
t24=t30;
}else{
X t33=___parse_3ap_5fconstant(t23); // REF: parse:p_constant
save(t33);
X t34=allocate(2); // CONS
save(t34);
t33=pick(2);
save(t33);
X t35=allocate(1); // CONS
save(t35);
t33=pick(2);
X t36=select1(TO_N(1),t33); // SELECT (simple)
t35=top();
S_DATA(t35)[0]=t36;
t35=restore();
restore();
t34=top();
S_DATA(t34)[0]=t35;
t35=pick(2);
X t37=select1(TO_N(2),t35); // SELECT (simple)
t34=top();
S_DATA(t34)[1]=t37;
t34=restore();
restore();
X t38=t34; // WHILE
for(;;){
save(t38);
save(t38);
X t39=allocate(3); // CONS
save(t39);
t38=pick(2);
X t40=select1(TO_N(2),t38); // SELECT (simple)
t39=top();
S_DATA(t39)[0]=t40;
t40=pick(2);
X t41=lf[90]; // CONST
t39=top();
S_DATA(t39)[1]=t41;
t41=pick(2);
X t42=lf[89]; // CONST
t39=top();
S_DATA(t39)[2]=t42;
t39=restore();
restore();
save(t39); // COND
X t44=___parse_3amatchval(t39); // REF: parse:matchval
X t43;
t39=restore();
if(t44!=F){
X t45=F; // ICONST
t43=t45;
}else{
X t46=T; // ICONST
t43=t46;}
t38=restore();
if(t43==F) break;
save(t38);
X t47=allocate(2); // CONS
save(t47);
t38=pick(2);
X t48=select1(TO_N(1),t38); // SELECT (simple)
t47=top();
S_DATA(t47)[0]=t48;
t48=pick(2);
save(t48);
X t49=allocate(3); // CONS
save(t49);
t48=pick(2);
X t50=select1(TO_N(2),t48); // SELECT (simple)
t49=top();
S_DATA(t49)[0]=t50;
t50=pick(2);
X t51=lf[90]; // CONST
t49=top();
S_DATA(t49)[1]=t51;
t51=pick(2);
X t52=lf[91]; // CONST
t49=top();
S_DATA(t49)[2]=t52;
t49=restore();
restore();
X t53=___parse_3aexpectval(t49); // REF: parse:expectval
X t54=___parse_3ap_5fconstant(t53); // REF: parse:p_constant
t47=top();
S_DATA(t47)[1]=t54;
t47=restore();
restore();
save(t47);
X t55=allocate(2); // CONS
save(t55);
t47=pick(2);
save(t47);
X t56=allocate(2); // CONS
save(t56);
t47=pick(2);
X t57=select1(TO_N(1),t47); // SELECT (simple)
t56=top();
S_DATA(t56)[0]=t57;
t57=pick(2);
X t59=TO_N(1); // ICONST
save(t59);
X t60=select1(TO_N(2),t57); // SELECT (simple)
t59=restore();
X t58=select1(t59,t60); // SELECT
t56=top();
S_DATA(t56)[1]=t58;
t56=restore();
restore();
X t61=___ar(t56); // REF: ar
t55=top();
S_DATA(t55)[0]=t61;
t61=pick(2);
X t63=TO_N(2); // ICONST
save(t63);
X t64=select1(TO_N(2),t61); // SELECT (simple)
t63=restore();
X t62=select1(t63,t64); // SELECT
t55=top();
S_DATA(t55)[1]=t62;
t55=restore();
restore();
t38=t55;}
save(t38);
X t65=allocate(2); // CONS
save(t65);
t38=pick(2);
X t66=select1(TO_N(1),t38); // SELECT (simple)
t65=top();
S_DATA(t65)[0]=t66;
t66=pick(2);
X t67=select1(TO_N(2),t66); // SELECT (simple)
X t68=___parse_3aadvance(t67); // REF: parse:advance
t65=top();
S_DATA(t65)[1]=t68;
t65=restore();
restore();
t24=t65;}
t17=t24;
}else{
save(x); // COND
save(x);
X t70=allocate(2); // CONS
save(t70);
x=pick(2);
X t71=___id(x); // REF: id
t70=top();
S_DATA(t70)[0]=t71;
t71=pick(2);
X t72=lf[92]; // CONST
t70=top();
S_DATA(t70)[1]=t72;
t70=restore();
restore();
X t73=___parse_3amatch(t70); // REF: parse:match
X t69;
x=restore();
if(t73!=F){
save(x);
X t74=allocate(2); // CONS
save(t74);
x=pick(2);
X t75=___parse_3anextval(x); // REF: parse:nextval
X t76=___tos(t75); // REF: tos
t74=top();
S_DATA(t74)[0]=t76;
t76=pick(2);
X t77=___parse_3aadvance(t76); // REF: parse:advance
t74=top();
S_DATA(t74)[1]=t77;
t74=restore();
restore();
t69=t74;
}else{
save(x);
X t78=allocate(2); // CONS
save(t78);
x=pick(2);
X t79=lf[93]; // CONST
t78=top();
S_DATA(t78)[0]=t79;
t79=pick(2);
X t80=___id(t79); // REF: id
t78=top();
S_DATA(t78)[1]=t80;
t78=restore();
restore();
X t81=___parse_3afail(t78); // REF: parse:fail
t69=t81;}
t17=t69;}
t9=t17;}
t1=t9;}
RETURN(t1);}
//---------------------------------------- parse:match (parse.fp:259)
DEFINE(___parse_3amatch){
ENTRY;
loop:;
tracecall("parse.fp:259:  parse:match");
save(x); // COND
X t3=lf[381]; // CONST
save(t3);
X t5=TO_N(1); // ICONST
save(t5);
X t6=select1(TO_N(1),x); // SELECT (simple)
t5=restore();
X t4=select1(t5,t6); // SELECT
t3=restore();
X t2=(t3==t4)||eq1(t3,t4)?T:F; // EQ
X t1;
x=restore();
if(t2!=F){
X t7=F; // ICONST
t1=t7;
}else{
save(x);
X t9=select1(TO_N(2),x); // SELECT (simple)
x=restore();
save(t9);
X t11=TO_N(1); // ICONST
save(t11);
X t13=TO_N(1); // ICONST
save(t13);
X t15=TO_N(1); // ICONST
save(t15);
X t16=select1(TO_N(1),x); // SELECT (simple)
t15=restore();
X t14=select1(t15,t16); // SELECT
t13=restore();
X t12=select1(t13,t14); // SELECT
t11=restore();
X t10=select1(t11,t12); // SELECT
t9=restore();
X t8=(t9==t10)||eq1(t9,t10)?T:F; // EQ
t1=t8;}
RETURN(t1);}
//---------------------------------------- parse:matchval (parse.fp:260)
DEFINE(___parse_3amatchval){
ENTRY;
loop:;
tracecall("parse.fp:260:  parse:matchval");
save(x); // COND
X t2=___parse_3amatch(x); // REF: parse:match
X t1;
x=restore();
if(t2!=F){
save(x);
X t4=select1(TO_N(3),x); // SELECT (simple)
x=restore();
save(t4);
X t6=TO_N(3); // ICONST
save(t6);
X t8=TO_N(1); // ICONST
save(t8);
X t10=TO_N(1); // ICONST
save(t10);
X t11=select1(TO_N(1),x); // SELECT (simple)
t10=restore();
X t9=select1(t10,t11); // SELECT
t8=restore();
X t7=select1(t8,t9); // SELECT
t6=restore();
X t5=select1(t6,t7); // SELECT
t4=restore();
X t3=(t4==t5)||eq1(t4,t5)?T:F; // EQ
t1=t3;
}else{
X t12=F; // ICONST
t1=t12;}
RETURN(t1);}
//---------------------------------------- parse:expect (parse.fp:262)
DEFINE(___parse_3aexpect){
ENTRY;
loop:;
tracecall("parse.fp:262:  parse:expect");
save(x); // COND
X t3=lf[381]; // CONST
save(t3);
X t5=TO_N(1); // ICONST
save(t5);
X t6=select1(TO_N(1),x); // SELECT (simple)
t5=restore();
X t4=select1(t5,t6); // SELECT
t3=restore();
X t2=(t3==t4)||eq1(t3,t4)?T:F; // EQ
X t1;
x=restore();
if(t2!=F){
save(x);
X t7=allocate(2); // CONS
save(t7);
x=pick(2);
X t8=lf[100]; // CONST
t7=top();
S_DATA(t7)[0]=t8;
t8=pick(2);
X t9=select1(TO_N(1),t8); // SELECT (simple)
t7=top();
S_DATA(t7)[1]=t9;
t7=restore();
restore();
X t10=___parse_3afail(t7); // REF: parse:fail
t1=t10;
}else{
save(x); // COND
X t12=___parse_3amatch(x); // REF: parse:match
X t11;
x=restore();
if(t12!=F){
X t13=select1(TO_N(1),x); // SELECT (simple)
t11=t13;
}else{
save(x);
X t14=allocate(2); // CONS
save(t14);
x=pick(2);
save(x);
X t15=allocate(4); // CONS
save(t15);
x=pick(2);
X t16=lf[95]; // CONST
t15=top();
S_DATA(t15)[0]=t16;
t16=pick(2);
X t17=select1(TO_N(2),t16); // SELECT (simple)
save(t17);
X t18=allocate(2); // CONS
save(t18);
t17=pick(2);
X t19=___id(t17); // REF: id
t18=top();
S_DATA(t18)[0]=t19;
t19=pick(2);
X t20=lf[96]; // CONST
t18=top();
S_DATA(t18)[1]=t20;
t18=restore();
restore();
X t21=___fetch(t18); // REF: fetch
t15=top();
S_DATA(t15)[1]=t21;
t21=pick(2);
X t22=lf[94]; // CONST
t15=top();
S_DATA(t15)[2]=t22;
t22=pick(2);
X t24=TO_N(1); // ICONST
save(t24);
X t26=TO_N(1); // ICONST
save(t26);
X t28=TO_N(1); // ICONST
save(t28);
X t29=select1(TO_N(1),t22); // SELECT (simple)
t28=restore();
X t27=select1(t28,t29); // SELECT
t26=restore();
X t25=select1(t26,t27); // SELECT
t24=restore();
X t23=select1(t24,t25); // SELECT
save(t23);
X t30=allocate(2); // CONS
save(t30);
t23=pick(2);
X t31=___id(t23); // REF: id
t30=top();
S_DATA(t30)[0]=t31;
t31=pick(2);
X t32=lf[96]; // CONST
t30=top();
S_DATA(t30)[1]=t32;
t30=restore();
restore();
X t33=___fetch(t30); // REF: fetch
t15=top();
S_DATA(t15)[3]=t33;
t15=restore();
restore();
X t34=___cat(t15); // REF: cat
t14=top();
S_DATA(t14)[0]=t34;
t34=pick(2);
X t35=select1(TO_N(1),t34); // SELECT (simple)
t14=top();
S_DATA(t14)[1]=t35;
t14=restore();
restore();
X t36=___parse_3afail(t14); // REF: parse:fail
t11=t36;}
t1=t11;}
RETURN(t1);}
//---------------------------------------- parse:expectval (parse.fp:268)
DEFINE(___parse_3aexpectval){
ENTRY;
loop:;
tracecall("parse.fp:268:  parse:expectval");
save(x); // COND
X t3=lf[381]; // CONST
save(t3);
X t5=TO_N(1); // ICONST
save(t5);
X t6=select1(TO_N(1),x); // SELECT (simple)
t5=restore();
X t4=select1(t5,t6); // SELECT
t3=restore();
X t2=(t3==t4)||eq1(t3,t4)?T:F; // EQ
X t1;
x=restore();
if(t2!=F){
save(x);
X t7=allocate(2); // CONS
save(t7);
x=pick(2);
save(x);
X t8=allocate(3); // CONS
save(t8);
x=pick(2);
X t9=lf[95]; // CONST
t8=top();
S_DATA(t8)[0]=t9;
t9=pick(2);
X t10=select1(TO_N(2),t9); // SELECT (simple)
save(t10);
X t11=allocate(2); // CONS
save(t11);
t10=pick(2);
X t12=___id(t10); // REF: id
t11=top();
S_DATA(t11)[0]=t12;
t12=pick(2);
X t13=lf[96]; // CONST
t11=top();
S_DATA(t11)[1]=t13;
t11=restore();
restore();
X t14=___fetch(t11); // REF: fetch
t8=top();
S_DATA(t8)[1]=t14;
t14=pick(2);
X t15=lf[97]; // CONST
t8=top();
S_DATA(t8)[2]=t15;
t8=restore();
restore();
X t16=___cat(t8); // REF: cat
t7=top();
S_DATA(t7)[0]=t16;
t16=pick(2);
X t17=select1(TO_N(1),t16); // SELECT (simple)
t7=top();
S_DATA(t7)[1]=t17;
t7=restore();
restore();
X t18=___parse_3afail(t7); // REF: parse:fail
t1=t18;
}else{
save(x); // COND
X t20=___parse_3amatchval(x); // REF: parse:matchval
X t19;
x=restore();
if(t20!=F){
X t21=select1(TO_N(1),x); // SELECT (simple)
X t22=___parse_3aadvance(t21); // REF: parse:advance
t19=t22;
}else{
save(x);
X t23=allocate(2); // CONS
save(t23);
x=pick(2);
save(x);
X t24=allocate(5); // CONS
save(t24);
x=pick(2);
X t25=lf[98]; // CONST
t24=top();
S_DATA(t24)[0]=t25;
t25=pick(2);
X t26=select1(TO_N(3),t25); // SELECT (simple)
X t27=___tos(t26); // REF: tos
t24=top();
S_DATA(t24)[1]=t27;
t27=pick(2);
X t28=lf[99]; // CONST
t24=top();
S_DATA(t24)[2]=t28;
t28=pick(2);
X t30=TO_N(3); // ICONST
save(t30);
X t32=TO_N(1); // ICONST
save(t32);
X t34=TO_N(1); // ICONST
save(t34);
X t35=select1(TO_N(1),t28); // SELECT (simple)
t34=restore();
X t33=select1(t34,t35); // SELECT
t32=restore();
X t31=select1(t32,t33); // SELECT
t30=restore();
X t29=select1(t30,t31); // SELECT
X t36=___tos(t29); // REF: tos
t24=top();
S_DATA(t24)[3]=t36;
t36=pick(2);
X t37=lf[351]; // CONST
t24=top();
S_DATA(t24)[4]=t37;
t24=restore();
restore();
X t38=___cat(t24); // REF: cat
t23=top();
S_DATA(t23)[0]=t38;
t38=pick(2);
X t39=select1(TO_N(1),t38); // SELECT (simple)
t23=top();
S_DATA(t23)[1]=t39;
t23=restore();
restore();
X t40=___parse_3afail(t23); // REF: parse:fail
t19=t40;}
t1=t19;}
RETURN(t1);}
//---------------------------------------- parse:push (parse.fp:280)
DEFINE(___parse_3apush){
ENTRY;
loop:;
tracecall("parse.fp:280:  parse:push");
save(x); // COND
X t2=select1(TO_N(1),x); // SELECT (simple)
X t1;
x=restore();
if(t2!=F){
save(x);
X t3=allocate(2); // CONS
save(t3);
x=pick(2);
X t5=TO_N(1); // ICONST
save(t5);
X t6=select1(TO_N(2),x); // SELECT (simple)
t5=restore();
X t4=select1(t5,t6); // SELECT
t3=top();
S_DATA(t3)[0]=t4;
t4=pick(2);
save(t4);
X t7=allocate(2); // CONS
save(t7);
t4=pick(2);
X t9=TO_N(2); // ICONST
save(t9);
X t10=select1(TO_N(2),t4); // SELECT (simple)
t9=restore();
X t8=select1(t9,t10); // SELECT
t7=top();
S_DATA(t7)[0]=t8;
t8=pick(2);
X t11=select1(TO_N(1),t8); // SELECT (simple)
t7=top();
S_DATA(t7)[1]=t11;
t7=restore();
restore();
X t12=___ar(t7); // REF: ar
t3=top();
S_DATA(t3)[1]=t12;
t3=restore();
restore();
t1=t3;
}else{
X t13=select1(TO_N(2),x); // SELECT (simple)
t1=t13;}
RETURN(t1);}
//---------------------------------------- parse:prefix (parse.fp:289)
DEFINE(___parse_3aprefix){
ENTRY;
loop:;
tracecall("parse.fp:289:  parse:prefix");
save(x);
X t1=allocate(3); // CONS
save(t1);
x=pick(2);
X t2=select1(TO_N(1),x); // SELECT (simple)
X t3=___tos(t2); // REF: tos
t1=top();
S_DATA(t1)[0]=t3;
t3=pick(2);
X t4=lf[255]; // CONST
t1=top();
S_DATA(t1)[1]=t4;
t4=pick(2);
X t5=select1(TO_N(2),t4); // SELECT (simple)
X t6=___tos(t5); // REF: tos
t1=top();
S_DATA(t1)[2]=t6;
t1=restore();
restore();
X t7=___cat(t1); // REF: cat
X t8=___toa(t7); // REF: toa
RETURN(t8);}
//---------------------------------------- parse:advance (parse.fp:298)
DEFINE(___parse_3aadvance){
ENTRY;
loop:;
tracecall("parse.fp:298:  parse:advance");
save(x); // COND
X t3=lf[381]; // CONST
save(t3);
X t4=select1(TO_N(1),x); // SELECT (simple)
t3=restore();
X t2=(t3==t4)||eq1(t3,t4)?T:F; // EQ
X t1;
x=restore();
if(t2!=F){
save(x);
X t5=allocate(2); // CONS
save(t5);
x=pick(2);
X t6=lf[100]; // CONST
t5=top();
S_DATA(t5)[0]=t6;
t6=pick(2);
X t7=___id(t6); // REF: id
t5=top();
S_DATA(t5)[1]=t7;
t5=restore();
restore();
X t8=___parse_3afail(t5); // REF: parse:fail
t1=t8;
}else{
save(x);
X t9=allocate(2); // CONS
save(t9);
x=pick(2);
X t10=select1(TO_N(1),x); // SELECT (simple)
X t11=___tl(t10); // REF: tl
t9=top();
S_DATA(t9)[0]=t11;
t11=pick(2);
X t12=___tl(t11); // REF: tl
t9=top();
S_DATA(t9)[1]=t12;
t9=restore();
restore();
X t13=___al(t9); // REF: al
t1=t13;}
RETURN(t1);}
//---------------------------------------- parse:fail (parse.fp:304)
DEFINE(___parse_3afail){
ENTRY;
loop:;
tracecall("parse.fp:304:  parse:fail");
save(x);
X t1=allocate(3); // CONS
save(t1);
x=pick(2);
save(x); // COND
X t4=lf[381]; // CONST
save(t4);
X t6=TO_N(1); // ICONST
save(t6);
X t7=select1(TO_N(2),x); // SELECT (simple)
t6=restore();
X t5=select1(t6,t7); // SELECT
t4=restore();
X t3=(t4==t5)||eq1(t4,t5)?T:F; // EQ
X t2;
x=restore();
if(t3!=F){
X t8=lf[101]; // CONST
t2=t8;
}else{
X t10=TO_N(2); // ICONST
save(t10);
X t12=TO_N(1); // ICONST
save(t12);
X t14=TO_N(1); // ICONST
save(t14);
X t15=select1(TO_N(2),x); // SELECT (simple)
t14=restore();
X t13=select1(t14,t15); // SELECT
t12=restore();
X t11=select1(t12,t13); // SELECT
t10=restore();
X t9=select1(t10,t11); // SELECT
X t16=___tos(t9); // REF: tos
t2=t16;}
t1=top();
S_DATA(t1)[0]=t2;
t2=pick(2);
X t17=lf[344]; // CONST
t1=top();
S_DATA(t1)[1]=t17;
t17=pick(2);
X t18=select1(TO_N(1),t17); // SELECT (simple)
t1=top();
S_DATA(t1)[2]=t18;
t1=restore();
restore();
X t19=___cat(t1); // REF: cat
X t20=___toa(t19); // REF: toa
X t21=___quit(t20); // REF: quit
RETURN(t21);}
//---------------------------------------- analysis:treesize (analysis.fp:18)
DEFINE(___analysis_3atreesize){
ENTRY;
loop:;
tracecall("analysis.fp:18:  analysis:treesize");
save(x); // COND
save(x);
X t2=allocate(2); // CONS
save(t2);
x=pick(2);
X t3=select1(TO_N(1),x); // SELECT (simple)
t2=top();
S_DATA(t2)[0]=t3;
t3=pick(2);
X t4=lf[102]; // CONST
t2=top();
S_DATA(t2)[1]=t4;
t2=restore();
restore();
X t5=___index(t2); // REF: index
X t1;
x=restore();
if(t5!=F){
X t6=TO_N(1); // ICONST
t1=t6;
}else{
X t7=___tl(x); // REF: tl
X t8=___tl(t7); // REF: tl
int t10; // ALPHA
check_S(t8,"@");
int t11=S_LENGTH(t8);
save(t8);
X t9=allocate(t11);
save(t9);
for(t10=0;t10<t11;++t10){
X t9=S_DATA(pick(2))[t10];
X t12=___analysis_3atreesize(t9); // REF: analysis:treesize
S_DATA(top())[t10]=t12;}
t9=restore();
restore();
int t15; // INSERTR
check_S(t9,"/");
int t16=S_LENGTH(t9);
X t13=t16==0?TO_N(0):S_DATA(t9)[t16-1];
save(t9);
for(t15=t16-2;t15>=0;--t15){
X t14=sequence(2,S_DATA(pick(1))[t15],t13);
X t17=___add(t14); // REF: add
t13=t17;}
restore();
save(t13);
X t18=allocate(2); // CONS
save(t18);
t13=pick(2);
X t19=___id(t13); // REF: id
t18=top();
S_DATA(t18)[0]=t19;
t19=pick(2);
X t20=TO_N(1); // ICONST
t18=top();
S_DATA(t18)[1]=t20;
t18=restore();
restore();
X t21=___add(t18); // REF: add
t1=t21;}
RETURN(t1);}
//---------------------------------------- analysis:collectrefs (analysis.fp:21)
DEFINE(___analysis_3acollectrefs){
ENTRY;
loop:;
tracecall("analysis.fp:21:  analysis:collectrefs");
save(x); // COND
X t2=select1(TO_N(1),x); // SELECT (simple)
save(t2); // OR
X t5=lf[293]; // CONST
save(t5);
X t6=___id(t2); // REF: id
t5=restore();
X t4=(t5==t6)||eq1(t5,t6)?T:F; // EQ
X t3=t4;
t2=restore();
if(t3==F){
X t8=lf[324]; // CONST
save(t8);
X t9=___id(t2); // REF: id
t8=restore();
X t7=(t8==t9)||eq1(t8,t9)?T:F; // EQ
t3=t7;}
X t1;
x=restore();
if(t3!=F){
save(x);
X t10=allocate(1); // CONS
save(t10);
x=pick(2);
X t11=___id(x); // REF: id
t10=top();
S_DATA(t10)[0]=t11;
t10=restore();
restore();
t1=t10;
}else{
save(x); // COND
X t13=___constnode(x); // REF: constnode
X t12;
x=restore();
if(t13!=F){
X t14=EMPTY; // ICONST
t12=t14;
}else{
X t15=___tl(x); // REF: tl
X t16=___tl(t15); // REF: tl
int t18; // ALPHA
check_S(t16,"@");
int t19=S_LENGTH(t16);
save(t16);
X t17=allocate(t19);
save(t17);
for(t18=0;t18<t19;++t18){
X t17=S_DATA(pick(2))[t18];
X t20=___analysis_3acollectrefs(t17); // REF: analysis:collectrefs
S_DATA(top())[t18]=t20;}
t17=restore();
restore();
X t21=___cat(t17); // REF: cat
t12=t21;}
t1=t12;}
RETURN(t1);}
//---------------------------------------- analysis:report_stats (analysis.fp:26)
DEFINE(___analysis_3areport_5fstats){
ENTRY;
loop:;
tracecall("analysis.fp:26:  analysis:report_stats");
save(x);
X t1=allocate(4); // CONS
save(t1);
x=pick(2);
X t2=lf[103]; // CONST
t1=top();
S_DATA(t1)[0]=t2;
t2=pick(2);
X t3=___len(t2); // REF: len
X t4=___tos(t3); // REF: tos
t1=top();
S_DATA(t1)[1]=t4;
t4=pick(2);
X t5=lf[104]; // CONST
t1=top();
S_DATA(t1)[2]=t5;
t5=pick(2);
int t7; // ALPHA
check_S(t5,"@");
int t8=S_LENGTH(t5);
save(t5);
X t6=allocate(t8);
save(t6);
for(t7=0;t7<t8;++t7){
X t6=S_DATA(pick(2))[t7];
X t9=select1(TO_N(3),t6); // SELECT (simple)
S_DATA(top())[t7]=t9;}
t6=restore();
restore();
int t12; // INSERTR
check_S(t6,"/");
int t13=S_LENGTH(t6);
X t10=t13==0?TO_N(0):S_DATA(t6)[t13-1];
save(t6);
for(t12=t13-2;t12>=0;--t12){
X t11=sequence(2,S_DATA(pick(1))[t12],t10);
X t14=___add(t11); // REF: add
t10=t14;}
restore();
X t15=___tos(t10); // REF: tos
t1=top();
S_DATA(t1)[3]=t15;
t1=restore();
restore();
X t16=___cat(t1); // REF: cat
save(t16); // SEQ
X t17=____5femit(t16); // REF: _emit
t16=restore();
X t18=TO_N(10); // ICONST
X t19=____5femit(t18); // REF: _emit
RETURN(t19);}
//---------------------------------------- treeshake:loop (treeshake.fp:17)
DEFINE(___treeshake_3aloop){
ENTRY;
loop:;
tracecall("treeshake.fp:17:  treeshake:loop");
save(x);
X t1=allocate(4); // CONS
save(t1);
x=pick(2);
save(x);
X t2=allocate(2); // CONS
save(t2);
x=pick(2);
save(x);
X t3=allocate(2); // CONS
save(t3);
x=pick(2);
X t4=select1(TO_N(1),x); // SELECT (simple)
save(t4);
X t5=allocate(2); // CONS
save(t5);
t4=pick(2);
X t6=___id(t4); // REF: id
t5=top();
S_DATA(t5)[0]=t6;
t6=pick(2);
X t7=___id(t6); // REF: id
t5=top();
S_DATA(t5)[1]=t7;
t5=restore();
restore();
X t8=___dr(t5); // REF: dr
t3=top();
S_DATA(t3)[0]=t8;
t8=pick(2);
X t9=select1(TO_N(3),t8); // SELECT (simple)
t3=top();
S_DATA(t3)[1]=t9;
t3=restore();
restore();
X t10=___dr(t3); // REF: dr
int t12; // ALPHA
check_S(t10,"@");
int t13=S_LENGTH(t10);
save(t10);
X t11=allocate(t13);
save(t11);
for(t12=0;t12<t13;++t12){
X t11=S_DATA(pick(2))[t12];
X t14=___treeshake_3aused_5fdef(t11); // REF: treeshake:used_def
S_DATA(top())[t12]=t14;}
t11=restore();
restore();
t2=top();
S_DATA(t2)[0]=t11;
t11=pick(2);
X t15=select1(TO_N(1),t11); // SELECT (simple)
t2=top();
S_DATA(t2)[1]=t15;
t2=restore();
restore();
X t16=___partition(t2); // REF: partition
t1=top();
S_DATA(t1)[0]=t16;
t16=pick(2);
X t17=select1(TO_N(2),t16); // SELECT (simple)
t1=top();
S_DATA(t1)[1]=t17;
t17=pick(2);
X t18=select1(TO_N(3),t17); // SELECT (simple)
t1=top();
S_DATA(t1)[2]=t18;
t18=pick(2);
X t19=select1(TO_N(4),t18); // SELECT (simple)
t1=top();
S_DATA(t1)[3]=t19;
t1=restore();
restore();
save(t1); // COND
X t22=lf[381]; // CONST
save(t22);
X t24=TO_N(2); // ICONST
save(t24);
X t25=select1(TO_N(1),t1); // SELECT (simple)
t24=restore();
X t23=select1(t24,t25); // SELECT
t22=restore();
X t21=(t22==t23)||eq1(t22,t23)?T:F; // EQ
X t20;
t1=restore();
if(t21!=F){
X t27=TO_N(1); // ICONST
save(t27);
X t28=select1(TO_N(1),t1); // SELECT (simple)
t27=restore();
X t26=select1(t27,t28); // SELECT
t20=t26;
}else{
save(t1); // SEQ
save(t1); // COND
X t30=select1(TO_N(2),t1); // SELECT (simple)
X t29;
t1=restore();
if(t30!=F){
save(t1);
X t31=allocate(2); // CONS
save(t31);
t1=pick(2);
X t32=select1(TO_N(4),t1); // SELECT (simple)
t31=top();
S_DATA(t31)[0]=t32;
t32=pick(2);
X t34=TO_N(2); // ICONST
save(t34);
X t35=select1(TO_N(1),t32); // SELECT (simple)
t34=restore();
X t33=select1(t34,t35); // SELECT
t31=top();
S_DATA(t31)[1]=t33;
t31=restore();
restore();
save(t31); // SEQ
save(t31);
X t36=allocate(3); // CONS
save(t36);
t31=pick(2);
X t37=lf[105]; // CONST
t36=top();
S_DATA(t36)[0]=t37;
t37=pick(2);
X t38=select1(TO_N(1),t37); // SELECT (simple)
X t39=___tos(t38); // REF: tos
t36=top();
S_DATA(t36)[1]=t39;
t39=pick(2);
X t40=lf[106]; // CONST
t36=top();
S_DATA(t36)[2]=t40;
t36=restore();
restore();
X t41=___cat(t36); // REF: cat
save(t41); // SEQ
X t42=____5femit(t41); // REF: _emit
t41=restore();
X t43=TO_N(10); // ICONST
X t44=____5femit(t43); // REF: _emit
t31=restore();
X t45=select1(TO_N(2),t31); // SELECT (simple)
int t47; // ALPHA
check_S(t45,"@");
int t48=S_LENGTH(t45);
save(t45);
X t46=allocate(t48);
save(t46);
for(t47=0;t47<t48;++t47){
X t46=S_DATA(pick(2))[t47];
save(t46);
X t49=allocate(4); // CONS
save(t49);
t46=pick(2);
X t50=lf[107]; // CONST
t49=top();
S_DATA(t49)[0]=t50;
t50=pick(2);
X t52=TO_N(2); // ICONST
save(t52);
X t53=select1(TO_N(2),t50); // SELECT (simple)
t52=restore();
X t51=select1(t52,t53); // SELECT
X t54=___tos(t51); // REF: tos
t49=top();
S_DATA(t49)[1]=t54;
t54=pick(2);
X t55=lf[344]; // CONST
t49=top();
S_DATA(t49)[2]=t55;
t55=pick(2);
X t56=select1(TO_N(1),t55); // SELECT (simple)
X t57=___tos(t56); // REF: tos
t49=top();
S_DATA(t49)[3]=t57;
t49=restore();
restore();
X t58=___cat(t49); // REF: cat
save(t58); // SEQ
X t59=____5femit(t58); // REF: _emit
t58=restore();
X t60=TO_N(10); // ICONST
X t61=____5femit(t60); // REF: _emit
S_DATA(top())[t47]=t61;}
t46=restore();
restore();
t29=t46;
}else{
X t62=___id(t1); // REF: id
t29=t62;}
t1=restore();
save(t1);
X t63=allocate(4); // CONS
save(t63);
t1=pick(2);
X t65=TO_N(1); // ICONST
save(t65);
X t66=select1(TO_N(1),t1); // SELECT (simple)
t65=restore();
X t64=select1(t65,t66); // SELECT
t63=top();
S_DATA(t63)[0]=t64;
t64=pick(2);
X t67=select1(TO_N(2),t64); // SELECT (simple)
t63=top();
S_DATA(t63)[1]=t67;
t67=pick(2);
X t68=select1(TO_N(3),t67); // SELECT (simple)
t63=top();
S_DATA(t63)[2]=t68;
t68=pick(2);
X t69=select1(TO_N(4),t68); // SELECT (simple)
save(t69);
X t70=allocate(2); // CONS
save(t70);
t69=pick(2);
X t71=___id(t69); // REF: id
t70=top();
S_DATA(t70)[0]=t71;
t71=pick(2);
X t72=TO_N(1); // ICONST
t70=top();
S_DATA(t70)[1]=t72;
t70=restore();
restore();
X t73=___add(t70); // REF: add
t63=top();
S_DATA(t63)[3]=t73;
t63=restore();
restore();
x=t63; // REF: treeshake:loop
goto loop; // tail call: treeshake:loop
X t74;
t20=t74;}
RETURN(t20);}
//---------------------------------------- treeshake:used_def (treeshake.fp:23)
DEFINE(___treeshake_3aused_5fdef){
ENTRY;
loop:;
tracecall("treeshake.fp:23:  treeshake:used_def");
save(x); // COND
X t3=lf[381]; // CONST
save(t3);
X t5=TO_N(2); // ICONST
save(t5);
X t6=select1(TO_N(1),x); // SELECT (simple)
t5=restore();
X t4=select1(t5,t6); // SELECT
t3=restore();
X t2=(t3==t4)||eq1(t3,t4)?T:F; // EQ
X t1;
x=restore();
if(t2!=F){
save(x);
X t9=TO_N(1); // ICONST
save(t9);
X t11=TO_N(1); // ICONST
save(t11);
X t12=select1(TO_N(1),x); // SELECT (simple)
t11=restore();
X t10=select1(t11,t12); // SELECT
t9=restore();
X t8=select1(t9,t10); // SELECT
x=restore();
save(t8);
X t13=select1(TO_N(2),x); // SELECT (simple)
t8=restore();
X t7=(t8==t13)||eq1(t8,t13)?T:F; // EQ
t1=t7;
}else{
save(x); // COND
save(x);
X t17=TO_N(1); // ICONST
save(t17);
X t19=TO_N(1); // ICONST
save(t19);
X t20=select1(TO_N(1),x); // SELECT (simple)
t19=restore();
X t18=select1(t19,t20); // SELECT
t17=restore();
X t16=select1(t17,t18); // SELECT
x=restore();
save(t16);
X t22=TO_N(1); // ICONST
save(t22);
X t24=TO_N(1); // ICONST
save(t24);
X t26=TO_N(2); // ICONST
save(t26);
X t27=select1(TO_N(1),x); // SELECT (simple)
t26=restore();
X t25=select1(t26,t27); // SELECT
t24=restore();
X t23=select1(t24,t25); // SELECT
t22=restore();
X t21=select1(t22,t23); // SELECT
t16=restore();
X t15=(t16==t21)||eq1(t16,t21)?T:F; // EQ
X t14;
x=restore();
if(t15!=F){
save(x);
X t28=allocate(2); // CONS
save(t28);
x=pick(2);
save(x);
X t29=allocate(2); // CONS
save(t29);
x=pick(2);
X t31=TO_N(1); // ICONST
save(t31);
X t32=select1(TO_N(1),x); // SELECT (simple)
t31=restore();
X t30=select1(t31,t32); // SELECT
t29=top();
S_DATA(t29)[0]=t30;
t30=pick(2);
X t34=TO_N(2); // ICONST
save(t34);
X t35=select1(TO_N(1),t30); // SELECT (simple)
t34=restore();
X t33=select1(t34,t35); // SELECT
X t36=___tl(t33); // REF: tl
t29=top();
S_DATA(t29)[1]=t36;
t29=restore();
restore();
t28=top();
S_DATA(t28)[0]=t29;
t29=pick(2);
X t37=select1(TO_N(2),t29); // SELECT (simple)
t28=top();
S_DATA(t28)[1]=t37;
t28=restore();
restore();
x=t28; // REF: treeshake:used_def
goto loop; // tail call: treeshake:used_def
X t38;
t14=t38;
}else{
save(x); // COND
save(x);
X t40=allocate(3); // CONS
save(t40);
x=pick(2);
X t42=TO_N(1); // ICONST
save(t42);
X t44=TO_N(1); // ICONST
save(t44);
X t45=select1(TO_N(1),x); // SELECT (simple)
t44=restore();
X t43=select1(t44,t45); // SELECT
t42=restore();
X t41=select1(t42,t43); // SELECT
t40=top();
S_DATA(t40)[0]=t41;
t41=pick(2);
X t47=TO_N(4); // ICONST
save(t47);
X t49=TO_N(1); // ICONST
save(t49);
X t51=TO_N(2); // ICONST
save(t51);
X t52=select1(TO_N(1),t41); // SELECT (simple)
t51=restore();
X t50=select1(t51,t52); // SELECT
t49=restore();
X t48=select1(t49,t50); // SELECT
t47=restore();
X t46=select1(t47,t48); // SELECT
t40=top();
S_DATA(t40)[1]=t46;
t46=pick(2);
X t53=select1(TO_N(2),t46); // SELECT (simple)
t40=top();
S_DATA(t40)[2]=t53;
t40=restore();
restore();
save(t40); // COND
save(t40);
X t55=allocate(2); // CONS
save(t55);
t40=pick(2);
X t56=select1(TO_N(1),t40); // SELECT (simple)
t55=top();
S_DATA(t55)[0]=t56;
t56=pick(2);
X t57=select1(TO_N(2),t56); // SELECT (simple)
int t59; // ALPHA
check_S(t57,"@");
int t60=S_LENGTH(t57);
save(t57);
X t58=allocate(t60);
save(t58);
for(t59=0;t59<t60;++t59){
X t58=S_DATA(pick(2))[t59];
X t61=select1(TO_N(3),t58); // SELECT (simple)
S_DATA(top())[t59]=t61;}
t58=restore();
restore();
t55=top();
S_DATA(t55)[1]=t58;
t55=restore();
restore();
X t62=___index(t55); // REF: index
X t54;
t40=restore();
if(t62!=F){
X t63=T; // ICONST
t54=t63;
}else{
save(t40);
X t65=select1(TO_N(1),t40); // SELECT (simple)
t40=restore();
save(t65);
X t66=select1(TO_N(3),t40); // SELECT (simple)
t65=restore();
X t64=(t65==t66)||eq1(t65,t66)?T:F; // EQ
t54=t64;}
X t39;
x=restore();
if(t54!=F){
X t67=T; // ICONST
t39=t67;
}else{
save(x);
X t68=allocate(2); // CONS
save(t68);
x=pick(2);
save(x);
X t69=allocate(2); // CONS
save(t69);
x=pick(2);
X t71=TO_N(1); // ICONST
save(t71);
X t72=select1(TO_N(1),x); // SELECT (simple)
t71=restore();
X t70=select1(t71,t72); // SELECT
t69=top();
S_DATA(t69)[0]=t70;
t70=pick(2);
X t74=TO_N(2); // ICONST
save(t74);
X t75=select1(TO_N(1),t70); // SELECT (simple)
t74=restore();
X t73=select1(t74,t75); // SELECT
X t76=___tl(t73); // REF: tl
t69=top();
S_DATA(t69)[1]=t76;
t69=restore();
restore();
t68=top();
S_DATA(t68)[0]=t69;
t69=pick(2);
X t77=select1(TO_N(2),t69); // SELECT (simple)
t68=top();
S_DATA(t68)[1]=t77;
t68=restore();
restore();
x=t68; // REF: treeshake:used_def
goto loop; // tail call: treeshake:used_def
X t78;
t39=t78;}
t14=t39;}
t1=t14;}
RETURN(t1);}
//---------------------------------------- inline:walkrec (inline.fp:37)
DEFINE(___inline_3awalkrec){
ENTRY;
loop:;
tracecall("inline.fp:37:  inline:walkrec");
save(x); // COND
X t3=lf[293]; // CONST
save(t3);
X t5=TO_N(1); // ICONST
save(t5);
X t6=select1(TO_N(1),x); // SELECT (simple)
t5=restore();
X t4=select1(t5,t6); // SELECT
t3=restore();
X t2=(t3==t4)||eq1(t3,t4)?T:F; // EQ
X t1;
x=restore();
if(t2!=F){
save(x); // COND
save(x);
X t10=TO_N(3); // ICONST
save(t10);
X t11=select1(TO_N(1),x); // SELECT (simple)
t10=restore();
X t9=select1(t10,t11); // SELECT
x=restore();
save(t9);
X t13=TO_N(1); // ICONST
save(t13);
X t14=select1(TO_N(2),x); // SELECT (simple)
t13=restore();
X t12=select1(t13,t14); // SELECT
t9=restore();
X t8=(t9==t12)||eq1(t9,t12)?T:F; // EQ
X t7;
x=restore();
if(t8!=F){
X t15=___tlr(x); // REF: tlr
t7=t15;
}else{
save(x);
X t16=allocate(4); // CONS
save(t16);
x=pick(2);
X t17=select1(TO_N(1),x); // SELECT (simple)
t16=top();
S_DATA(t16)[0]=t17;
t17=pick(2);
X t18=select1(TO_N(2),t17); // SELECT (simple)
t16=top();
S_DATA(t16)[1]=t18;
t18=pick(2);
X t19=select1(TO_N(3),t18); // SELECT (simple)
t16=top();
S_DATA(t16)[2]=t19;
t19=pick(2);
save(t19);
X t20=allocate(2); // CONS
save(t20);
t19=pick(2);
X t22=TO_N(3); // ICONST
save(t22);
X t23=select1(TO_N(1),t19); // SELECT (simple)
t22=restore();
X t21=select1(t22,t23); // SELECT
t20=top();
S_DATA(t20)[0]=t21;
t21=pick(2);
X t24=select1(TO_N(3),t21); // SELECT (simple)
t20=top();
S_DATA(t20)[1]=t24;
t20=restore();
restore();
save(t20);
X t25=allocate(2); // CONS
save(t25);
t20=pick(2);
save(t20); // OR
save(t20);
X t27=allocate(2); // CONS
save(t27);
t20=pick(2);
X t28=select1(TO_N(1),t20); // SELECT (simple)
t27=top();
S_DATA(t27)[0]=t28;
t28=pick(2);
X t30=TO_N(2); // ICONST
save(t30);
X t31=select1(TO_N(2),t28); // SELECT (simple)
t30=restore();
X t29=select1(t30,t31); // SELECT
t27=top();
S_DATA(t27)[1]=t29;
t27=restore();
restore();
X t32=___inline_3afind_5fin_5fdefs(t27); // REF: inline:find_in_defs
X t26=t32;
t20=restore();
if(t26==F){
save(t20);
X t33=allocate(2); // CONS
save(t33);
t20=pick(2);
X t34=select1(TO_N(1),t20); // SELECT (simple)
t33=top();
S_DATA(t33)[0]=t34;
t34=pick(2);
X t36=TO_N(1); // ICONST
save(t36);
X t37=select1(TO_N(2),t34); // SELECT (simple)
t36=restore();
X t35=select1(t36,t37); // SELECT
t33=top();
S_DATA(t33)[1]=t35;
t33=restore();
restore();
X t38=___inline_3afind_5fin_5fdefs(t33); // REF: inline:find_in_defs
t26=t38;}
t25=top();
S_DATA(t25)[0]=t26;
t26=pick(2);
X t39=select1(TO_N(2),t26); // SELECT (simple)
t25=top();
S_DATA(t25)[1]=t39;
t25=restore();
restore();
save(t25); // COND
save(t25); // COND
X t42=select1(TO_N(1),t25); // SELECT (simple)
X t41;
t25=restore();
if(t42!=F){
save(t25);
X t43=allocate(2); // CONS
save(t43);
t25=pick(2);
X t45=TO_N(3); // ICONST
save(t45);
X t46=select1(TO_N(1),t25); // SELECT (simple)
t45=restore();
X t44=select1(t45,t46); // SELECT
t43=top();
S_DATA(t43)[0]=t44;
t44=pick(2);
X t48=TO_N(3); // ICONST
save(t48);
X t49=select1(TO_N(2),t44); // SELECT (simple)
t48=restore();
X t47=select1(t48,t49); // SELECT
t43=top();
S_DATA(t43)[1]=t47;
t43=restore();
restore();
X t50=___le(t43); // REF: le
t41=t50;
}else{
X t51=F; // ICONST
t41=t51;}
X t40;
t25=restore();
if(t41!=F){
X t52=select1(TO_N(1),t25); // SELECT (simple)
t40=t52;
}else{
X t53=F; // ICONST
t40=t53;}
t16=top();
S_DATA(t16)[3]=t40;
t16=restore();
restore();
save(t16); // COND
X t55=select1(TO_N(4),t16); // SELECT (simple)
X t54;
t16=restore();
if(t55!=F){
save(t16);
X t56=allocate(4); // CONS
save(t56);
t16=pick(2);
X t57=select1(TO_N(1),t16); // SELECT (simple)
t56=top();
S_DATA(t56)[0]=t57;
t57=pick(2);
X t58=select1(TO_N(4),t57); // SELECT (simple)
t56=top();
S_DATA(t56)[1]=t58;
t58=pick(2);
X t59=select1(TO_N(2),t58); // SELECT (simple)
t56=top();
S_DATA(t56)[2]=t59;
t59=pick(2);
X t60=select1(TO_N(3),t59); // SELECT (simple)
t56=top();
S_DATA(t56)[3]=t60;
t56=restore();
restore();
save(t56); // SEQ
save(t56); // COND
X t63=TO_N(4); // ICONST
save(t63);
X t64=select1(TO_N(4),t56); // SELECT (simple)
t63=restore();
X t62=select1(t63,t64); // SELECT
X t61;
t56=restore();
if(t62!=F){
save(t56);
X t65=allocate(11); // CONS
save(t65);
t56=pick(2);
X t66=lf[343]; // CONST
t65=top();
S_DATA(t65)[0]=t66;
t66=pick(2);
X t68=TO_N(2); // ICONST
save(t68);
X t70=TO_N(2); // ICONST
save(t70);
X t71=select1(TO_N(3),t66); // SELECT (simple)
t70=restore();
X t69=select1(t70,t71); // SELECT
t68=restore();
X t67=select1(t68,t69); // SELECT
X t72=___tos(t67); // REF: tos
t65=top();
S_DATA(t65)[1]=t72;
t72=pick(2);
X t73=lf[108]; // CONST
t65=top();
S_DATA(t65)[2]=t73;
t73=pick(2);
X t75=TO_N(1); // ICONST
save(t75);
X t76=select1(TO_N(3),t73); // SELECT (simple)
t75=restore();
X t74=select1(t75,t76); // SELECT
X t77=___tos(t74); // REF: tos
t65=top();
S_DATA(t65)[3]=t77;
t77=pick(2);
X t78=lf[109]; // CONST
t65=top();
S_DATA(t65)[4]=t78;
t78=pick(2);
X t80=TO_N(1); // ICONST
save(t80);
X t81=select1(TO_N(2),t78); // SELECT (simple)
t80=restore();
X t79=select1(t80,t81); // SELECT
X t82=___tos(t79); // REF: tos
t65=top();
S_DATA(t65)[5]=t82;
t82=pick(2);
X t83=lf[115]; // CONST
t65=top();
S_DATA(t65)[6]=t83;
t83=pick(2);
X t85=TO_N(2); // ICONST
save(t85);
X t86=select1(TO_N(1),t83); // SELECT (simple)
t85=restore();
X t84=select1(t85,t86); // SELECT
X t87=___tos(t84); // REF: tos
t65=top();
S_DATA(t65)[7]=t87;
t87=pick(2);
X t88=lf[338]; // CONST
t65=top();
S_DATA(t65)[8]=t88;
t88=pick(2);
X t90=TO_N(3); // ICONST
save(t90);
X t91=select1(TO_N(2),t88); // SELECT (simple)
t90=restore();
X t89=select1(t90,t91); // SELECT
save(t89); // COND
X t94=TO_N(0); // ICONST
save(t94);
X t95=___id(t89); // REF: id
t94=restore();
X t93=(t94==t95)||eq1(t94,t95)?T:F; // EQ
X t92;
t89=restore();
if(t93!=F){
X t96=lf[110]; // CONST
t92=t96;
}else{
save(t89);
X t97=allocate(2); // CONS
save(t97);
t89=pick(2);
X t98=lf[111]; // CONST
t97=top();
S_DATA(t97)[0]=t98;
t98=pick(2);
X t99=___tos(t98); // REF: tos
t97=top();
S_DATA(t97)[1]=t99;
t97=restore();
restore();
X t100=___cat(t97); // REF: cat
t92=t100;}
t65=top();
S_DATA(t65)[9]=t92;
t92=pick(2);
X t101=lf[284]; // CONST
t65=top();
S_DATA(t65)[10]=t101;
t65=restore();
restore();
X t102=___cat(t65); // REF: cat
save(t102); // SEQ
X t103=____5femit(t102); // REF: _emit
t102=restore();
X t104=TO_N(10); // ICONST
X t105=____5femit(t104); // REF: _emit
t61=t105;
}else{
X t106=___id(t56); // REF: id
t61=t106;}
t56=restore();
save(t56);
X t107=allocate(2); // CONS
save(t107);
t56=pick(2);
X t109=TO_N(2); // ICONST
save(t109);
X t110=select1(TO_N(2),t56); // SELECT (simple)
t109=restore();
X t108=select1(t109,t110); // SELECT
t107=top();
S_DATA(t107)[0]=t108;
t108=pick(2);
save(t108);
X t111=allocate(2); // CONS
save(t111);
t108=pick(2);
X t112=select1(TO_N(3),t108); // SELECT (simple)
t111=top();
S_DATA(t111)[0]=t112;
t112=pick(2);
X t114=TO_N(3); // ICONST
save(t114);
X t115=select1(TO_N(2),t112); // SELECT (simple)
t114=restore();
X t113=select1(t114,t115); // SELECT
t111=top();
S_DATA(t111)[1]=t113;
t111=restore();
restore();
save(t111);
X t116=allocate(5); // CONS
save(t116);
t111=pick(2);
X t118=TO_N(1); // ICONST
save(t118);
X t119=select1(TO_N(1),t111); // SELECT (simple)
t118=restore();
X t117=select1(t118,t119); // SELECT
t116=top();
S_DATA(t116)[0]=t117;
t117=pick(2);
save(t117);
X t120=allocate(2); // CONS
save(t120);
t117=pick(2);
X t121=lf[112]; // CONST
t120=top();
S_DATA(t120)[0]=t121;
t121=pick(2);
X t123=TO_N(2); // ICONST
save(t123);
X t125=TO_N(2); // ICONST
save(t125);
X t126=select1(TO_N(1),t121); // SELECT (simple)
t125=restore();
X t124=select1(t125,t126); // SELECT
t123=restore();
X t122=select1(t123,t124); // SELECT
t120=top();
S_DATA(t120)[1]=t122;
t120=restore();
restore();
t116=top();
S_DATA(t116)[1]=t120;
t120=pick(2);
save(t120);
X t129=TO_N(3); // ICONST
save(t129);
X t130=select1(TO_N(1),t120); // SELECT (simple)
t129=restore();
X t128=select1(t129,t130); // SELECT
t120=restore();
save(t128);
X t131=select1(TO_N(2),t120); // SELECT (simple)
t128=restore();
X t127=___add_5fop2(t128,t131); // OP2: add
t116=top();
S_DATA(t116)[2]=t127;
t127=pick(2);
X t133=TO_N(4); // ICONST
save(t133);
X t134=select1(TO_N(1),t127); // SELECT (simple)
t133=restore();
X t132=select1(t133,t134); // SELECT
t116=top();
S_DATA(t116)[3]=t132;
t132=pick(2);
X t135=T; // ICONST
t116=top();
S_DATA(t116)[4]=t135;
t116=restore();
restore();
t107=top();
S_DATA(t107)[1]=t116;
t107=restore();
restore();
t54=t107;
}else{
X t136=___tlr(t16); // REF: tlr
t54=t136;}
t7=t54;}
t1=t7;
}else{
save(x); // COND
X t138=select1(TO_N(1),x); // SELECT (simple)
X t139=___constnode(t138); // REF: constnode
X t137;
x=restore();
if(t139!=F){
X t140=___tlr(x); // REF: tlr
t137=t140;
}else{
save(x);
X t141=allocate(2); // CONS
save(t141);
x=pick(2);
X t142=select1(TO_N(1),x); // SELECT (simple)
t141=top();
S_DATA(t141)[0]=t142;
t142=pick(2);
save(t142);
X t143=allocate(3); // CONS
save(t143);
t142=pick(2);
X t144=select1(TO_N(1),t142); // SELECT (simple)
X t145=___tl(t144); // REF: tl
X t146=___tl(t145); // REF: tl
t143=top();
S_DATA(t143)[0]=t146;
t146=pick(2);
X t147=select1(TO_N(2),t146); // SELECT (simple)
t143=top();
S_DATA(t143)[1]=t147;
t147=pick(2);
X t148=select1(TO_N(3),t147); // SELECT (simple)
t143=top();
S_DATA(t143)[2]=t148;
t143=restore();
restore();
save(t143);
X t149=allocate(5); // CONS
save(t149);
t143=pick(2);
X t150=select1(TO_N(1),t143); // SELECT (simple)
t149=top();
S_DATA(t149)[0]=t150;
t150=pick(2);
X t151=TO_N(1); // ICONST
t149=top();
S_DATA(t149)[1]=t151;
t151=pick(2);
X t152=select1(TO_N(2),t151); // SELECT (simple)
t149=top();
S_DATA(t149)[2]=t152;
t152=pick(2);
X t153=select1(TO_N(3),t152); // SELECT (simple)
t149=top();
S_DATA(t149)[3]=t153;
t153=pick(2);
X t154=EMPTY; // ICONST
t149=top();
S_DATA(t149)[4]=t154;
t149=restore();
restore();
X t155=t149; // WHILE
for(;;){
save(t155);
save(t155);
X t156=allocate(2); // CONS
save(t156);
t155=pick(2);
X t157=select1(TO_N(2),t155); // SELECT (simple)
t156=top();
S_DATA(t156)[0]=t157;
t157=pick(2);
X t158=select1(TO_N(1),t157); // SELECT (simple)
X t159=___len(t158); // REF: len
t156=top();
S_DATA(t156)[1]=t159;
t156=restore();
restore();
X t160=___le(t156); // REF: le
t155=restore();
if(t160==F) break;
save(t155);
X t161=allocate(2); // CONS
save(t161);
t155=pick(2);
X t162=___id(t155); // REF: id
t161=top();
S_DATA(t161)[0]=t162;
t162=pick(2);
save(t162);
X t163=allocate(3); // CONS
save(t163);
t162=pick(2);
save(t162);
X t165=select1(TO_N(2),t162); // SELECT (simple)
t162=restore();
save(t165);
X t166=select1(TO_N(1),t162); // SELECT (simple)
t165=restore();
X t164=select1(t165,t166); // SELECT
t163=top();
S_DATA(t163)[0]=t164;
t164=pick(2);
X t167=select1(TO_N(3),t164); // SELECT (simple)
t163=top();
S_DATA(t163)[1]=t167;
t167=pick(2);
X t168=select1(TO_N(4),t167); // SELECT (simple)
t163=top();
S_DATA(t163)[2]=t168;
t163=restore();
restore();
X t169=___inline_3awalkrec(t163); // REF: inline:walkrec
t161=top();
S_DATA(t161)[1]=t169;
t161=restore();
restore();
save(t161);
X t170=allocate(5); // CONS
save(t170);
t161=pick(2);
X t172=TO_N(1); // ICONST
save(t172);
X t173=select1(TO_N(1),t161); // SELECT (simple)
t172=restore();
X t171=select1(t172,t173); // SELECT
t170=top();
S_DATA(t170)[0]=t171;
t171=pick(2);
X t175=TO_N(2); // ICONST
save(t175);
X t176=select1(TO_N(1),t171); // SELECT (simple)
t175=restore();
X t174=select1(t175,t176); // SELECT
save(t174);
X t177=allocate(2); // CONS
save(t177);
t174=pick(2);
X t178=___id(t174); // REF: id
t177=top();
S_DATA(t177)[0]=t178;
t178=pick(2);
X t179=TO_N(1); // ICONST
t177=top();
S_DATA(t177)[1]=t179;
t177=restore();
restore();
X t180=___add(t177); // REF: add
t170=top();
S_DATA(t170)[1]=t180;
t180=pick(2);
X t182=TO_N(2); // ICONST
save(t182);
X t183=select1(TO_N(2),t180); // SELECT (simple)
t182=restore();
X t181=select1(t182,t183); // SELECT
t170=top();
S_DATA(t170)[2]=t181;
t181=pick(2);
X t185=TO_N(4); // ICONST
save(t185);
X t186=select1(TO_N(1),t181); // SELECT (simple)
t185=restore();
X t184=select1(t185,t186); // SELECT
t170=top();
S_DATA(t170)[3]=t184;
t184=pick(2);
save(t184);
X t187=allocate(2); // CONS
save(t187);
t184=pick(2);
X t189=TO_N(5); // ICONST
save(t189);
X t190=select1(TO_N(1),t184); // SELECT (simple)
t189=restore();
X t188=select1(t189,t190); // SELECT
t187=top();
S_DATA(t187)[0]=t188;
t188=pick(2);
X t192=TO_N(1); // ICONST
save(t192);
X t193=select1(TO_N(2),t188); // SELECT (simple)
t192=restore();
X t191=select1(t192,t193); // SELECT
t187=top();
S_DATA(t187)[1]=t191;
t187=restore();
restore();
X t194=___ar(t187); // REF: ar
t170=top();
S_DATA(t170)[4]=t194;
t170=restore();
restore();
t155=t170;}
save(t155);
X t195=allocate(2); // CONS
save(t195);
t155=pick(2);
X t196=select1(TO_N(5),t155); // SELECT (simple)
t195=top();
S_DATA(t195)[0]=t196;
t196=pick(2);
X t197=select1(TO_N(3),t196); // SELECT (simple)
t195=top();
S_DATA(t195)[1]=t197;
t195=restore();
restore();
t141=top();
S_DATA(t141)[1]=t195;
t141=restore();
restore();
save(t141);
X t198=allocate(2); // CONS
save(t198);
t141=pick(2);
save(t141);
X t199=allocate(2); // CONS
save(t199);
t141=pick(2);
save(t141);
X t200=allocate(2); // CONS
save(t200);
t141=pick(2);
X t201=TO_N(2); // ICONST
t200=top();
S_DATA(t200)[0]=t201;
t201=pick(2);
X t202=select1(TO_N(1),t201); // SELECT (simple)
t200=top();
S_DATA(t200)[1]=t202;
t200=restore();
restore();
X t203=___take(t200); // REF: take
t199=top();
S_DATA(t199)[0]=t203;
t203=pick(2);
X t205=TO_N(1); // ICONST
save(t205);
X t206=select1(TO_N(2),t203); // SELECT (simple)
t205=restore();
X t204=select1(t205,t206); // SELECT
t199=top();
S_DATA(t199)[1]=t204;
t199=restore();
restore();
X t207=___cat(t199); // REF: cat
t198=top();
S_DATA(t198)[0]=t207;
t207=pick(2);
X t209=TO_N(2); // ICONST
save(t209);
X t210=select1(TO_N(2),t207); // SELECT (simple)
t209=restore();
X t208=select1(t209,t210); // SELECT
t198=top();
S_DATA(t198)[1]=t208;
t198=restore();
restore();
t137=t198;}
t1=t137;}
RETURN(t1);}
//---------------------------------------- inline:find_in_defs (inline.fp:59)
DEFINE(___inline_3afind_5fin_5fdefs){
ENTRY;
loop:;
tracecall("inline.fp:59:  inline:find_in_defs");
X t1=___assoc(x); // REF: assoc
save(t1); // COND
save(t1); // COND
X t4=___id(t1); // REF: id
X t3;
t1=restore();
if(t4!=F){
save(t1);
X t5=allocate(2); // CONS
save(t5);
t1=pick(2);
X t6=select1(TO_N(1),t1); // SELECT (simple)
t5=top();
S_DATA(t5)[0]=t6;
t6=pick(2);
X t7=select1(TO_N(4),t6); // SELECT (simple)
int t9; // ALPHA
check_S(t7,"@");
int t10=S_LENGTH(t7);
save(t7);
X t8=allocate(t10);
save(t8);
for(t9=0;t9<t10;++t9){
X t8=S_DATA(pick(2))[t9];
X t11=select1(TO_N(3),t8); // SELECT (simple)
S_DATA(top())[t9]=t11;}
t8=restore();
restore();
t5=top();
S_DATA(t5)[1]=t8;
t5=restore();
restore();
X t12=___index(t5); // REF: index
save(t12); // COND
X t14=___id(t12); // REF: id
X t13;
t12=restore();
if(t14!=F){
X t15=F; // ICONST
t13=t15;
}else{
X t16=T; // ICONST
t13=t16;}
t3=t13;
}else{
X t17=F; // ICONST
t3=t17;}
X t2;
t1=restore();
if(t3!=F){
X t18=___id(t1); // REF: id
t2=t18;
}else{
X t19=F; // ICONST
t2=t19;}
RETURN(t2);}
//---------------------------------------- rewrite:rewrite (rewrite.fp:21)
DEFINE(___rewrite_3arewrite){
ENTRY;
loop:;
tracecall("rewrite.fp:21:  rewrite:rewrite");
save(x); // SEQ
save(x); // COND
save(x);
X t2=allocate(2); // CONS
save(t2);
x=pick(2);
X t3=lf[361]; // CONST
t2=top();
S_DATA(t2)[0]=t3;
t3=pick(2);
X t4=select1(TO_N(3),t3); // SELECT (simple)
t2=top();
S_DATA(t2)[1]=t4;
t2=restore();
restore();
X t5=___fetch(t2); // REF: fetch
X t1;
x=restore();
if(t5!=F){
X t6=lf[113]; // CONST
save(t6); // SEQ
X t7=____5femit(t6); // REF: _emit
t6=restore();
X t8=TO_N(10); // ICONST
X t9=____5femit(t8); // REF: _emit
t1=t9;
}else{
X t10=___id(x); // REF: id
t1=t10;}
x=restore();
save(x); // COND
X t13=lf[381]; // CONST
save(t13);
X t14=select1(TO_N(1),x); // SELECT (simple)
t13=restore();
X t12=(t13==t14)||eq1(t13,t14)?T:F; // EQ
X t11;
x=restore();
if(t12!=F){
X t15=lf[114]; // CONST
t11=t15;
}else{
save(x);
X t16=allocate(2); // CONS
save(t16);
x=pick(2);
save(x);
X t17=allocate(2); // CONS
save(t17);
x=pick(2);
save(x);
X t18=allocate(3); // CONS
save(t18);
x=pick(2);
X t19=EMPTY; // ICONST
t18=top();
S_DATA(t18)[0]=t19;
t19=pick(2);
X t20=EMPTY; // ICONST
t18=top();
S_DATA(t18)[1]=t20;
t20=pick(2);
X t21=select1(TO_N(2),t20); // SELECT (simple)
t18=top();
S_DATA(t18)[2]=t21;
t18=restore();
restore();
t17=top();
S_DATA(t17)[0]=t18;
t18=pick(2);
X t22=select1(TO_N(1),t18); // SELECT (simple)
t17=top();
S_DATA(t17)[1]=t22;
t17=restore();
restore();
X t23=___al(t17); // REF: al
int t26; // INSERTL
check_S(t23,"/");
int t27=S_LENGTH(t23);
X t24=t27==0?fail("no unit value"):S_DATA(t23)[0];
save(t23);
for(t26=1;t26<t27;++t26){
X t25=sequence(2,t24,S_DATA(pick(1))[t26]);
save(t25); // COND
X t29=select1(TO_N(2),t25); // SELECT (simple)
save(t29); // COND
X t31=select1(TO_N(2),t29); // SELECT (simple)
X t32=F;
save(t31); // PCONS
if(!IS_S(t31)||S_LENGTH(t31)!=5) goto t33;
t31=S_DATA(top())[0];
X t35=lf[287]; // CONST
save(t35);
X t36=___id(t31); // REF: id
t35=restore();
X t34=(t35==t36)||eq1(t35,t36)?T:F; // EQ
if(t34==F) goto t33;
// skipped
// skipped
// skipped
t34=S_DATA(top())[4];
X t37=F;
save(t34); // PCONS
if(!IS_S(t34)||S_LENGTH(t34)!=4) goto t38;
t34=S_DATA(top())[0];
X t40=lf[320]; // CONST
save(t40);
X t41=___id(t34); // REF: id
t40=restore();
X t39=(t40==t41)||eq1(t40,t41)?T:F; // EQ
if(t39==F) goto t38;
// skipped
t39=S_DATA(top())[2];
X t42=F;
save(t39); // PCONSL
if(!IS_S(t39)||S_LENGTH(t39)<1) goto t43;
t39=S_DATA(top())[0];
X t45=lf[293]; // CONST
save(t45);
X t46=___id(t39); // REF: id
t45=restore();
X t44=(t45==t46)||eq1(t45,t46)?T:F; // EQ
if(t44==F) goto t43;
t42=T;
t43:
restore();
if(t42==F) goto t38;
// skipped
t37=T;
t38:
restore();
if(t37==F) goto t33;
t32=T;
t33:
restore();
X t30;
t29=restore();
if(t32!=F){
save(t29);
X t48=select1(TO_N(1),t29); // SELECT (simple)
t29=restore();
save(t48);
X t50=TO_N(3); // ICONST
save(t50);
X t52=TO_N(3); // ICONST
save(t52);
X t54=TO_N(5); // ICONST
save(t54);
X t55=select1(TO_N(2),t29); // SELECT (simple)
t54=restore();
X t53=select1(t54,t55); // SELECT
t52=restore();
X t51=select1(t52,t53); // SELECT
t50=restore();
X t49=select1(t50,t51); // SELECT
t48=restore();
X t47=(t48==t49)||eq1(t48,t49)?T:F; // EQ
t30=t47;
}else{
X t56=F; // ICONST
t30=t56;}
X t28;
t25=restore();
if(t30!=F){
save(t25);
X t57=allocate(2); // CONS
save(t57);
t25=pick(2);
save(t25);
X t58=allocate(3); // CONS
save(t58);
t25=pick(2);
X t59=lf[116]; // CONST
t58=top();
S_DATA(t58)[0]=t59;
t59=pick(2);
X t61=TO_N(2); // ICONST
save(t61);
X t63=TO_N(2); // ICONST
save(t63);
X t64=select1(TO_N(2),t59); // SELECT (simple)
t63=restore();
X t62=select1(t63,t64); // SELECT
t61=restore();
X t60=select1(t61,t62); // SELECT
t58=top();
S_DATA(t58)[1]=t60;
t60=pick(2);
X t65=select1(TO_N(1),t60); // SELECT (simple)
t58=top();
S_DATA(t58)[2]=t65;
t58=restore();
restore();
X t66=___rewrite_3alog(t58); // REF: rewrite:log
t57=top();
S_DATA(t57)[0]=t66;
t66=pick(2);
X t67=select1(TO_N(2),t66); // SELECT (simple)
save(t67);
X t68=allocate(2); // CONS
save(t68);
t67=pick(2);
X t69=select1(TO_N(1),t67); // SELECT (simple)
t68=top();
S_DATA(t68)[0]=t69;
t69=pick(2);
save(t69);
X t70=allocate(4); // CONS
save(t70);
t69=pick(2);
X t71=lf[320]; // CONST
t70=top();
S_DATA(t70)[0]=t71;
t71=pick(2);
X t73=TO_N(2); // ICONST
save(t73);
X t74=select1(TO_N(2),t71); // SELECT (simple)
t73=restore();
X t72=select1(t73,t74); // SELECT
t70=top();
S_DATA(t70)[1]=t72;
t72=pick(2);
X t76=TO_N(4); // ICONST
save(t76);
X t77=select1(TO_N(2),t72); // SELECT (simple)
t76=restore();
X t75=select1(t76,t77); // SELECT
t70=top();
S_DATA(t70)[2]=t75;
t75=pick(2);
save(t75);
X t78=allocate(4); // CONS
save(t78);
t75=pick(2);
X t79=lf[305]; // CONST
t78=top();
S_DATA(t78)[0]=t79;
t79=pick(2);
X t81=TO_N(2); // ICONST
save(t81);
X t82=select1(TO_N(2),t79); // SELECT (simple)
t81=restore();
X t80=select1(t81,t82); // SELECT
t78=top();
S_DATA(t78)[1]=t80;
t80=pick(2);
save(t80);
X t83=allocate(5); // CONS
save(t83);
t80=pick(2);
X t84=lf[287]; // CONST
t83=top();
S_DATA(t83)[0]=t84;
t84=pick(2);
X t86=TO_N(2); // ICONST
save(t86);
X t87=select1(TO_N(2),t84); // SELECT (simple)
t86=restore();
X t85=select1(t86,t87); // SELECT
t83=top();
S_DATA(t83)[1]=t85;
t85=pick(2);
X t89=TO_N(3); // ICONST
save(t89);
X t90=select1(TO_N(2),t85); // SELECT (simple)
t89=restore();
X t88=select1(t89,t90); // SELECT
t83=top();
S_DATA(t83)[2]=t88;
t88=pick(2);
save(t88);
X t91=allocate(3); // CONS
save(t91);
t88=pick(2);
X t92=lf[291]; // CONST
t91=top();
S_DATA(t91)[0]=t92;
t92=pick(2);
X t94=TO_N(2); // ICONST
save(t94);
X t95=select1(TO_N(2),t92); // SELECT (simple)
t94=restore();
X t93=select1(t94,t95); // SELECT
t91=top();
S_DATA(t91)[1]=t93;
t93=pick(2);
X t96=F; // ICONST
t91=top();
S_DATA(t91)[2]=t96;
t91=restore();
restore();
t83=top();
S_DATA(t83)[3]=t91;
t91=pick(2);
save(t91);
X t97=allocate(3); // CONS
save(t97);
t91=pick(2);
X t98=lf[291]; // CONST
t97=top();
S_DATA(t97)[0]=t98;
t98=pick(2);
X t100=TO_N(2); // ICONST
save(t100);
X t101=select1(TO_N(2),t98); // SELECT (simple)
t100=restore();
X t99=select1(t100,t101); // SELECT
t97=top();
S_DATA(t97)[1]=t99;
t99=pick(2);
X t102=T; // ICONST
t97=top();
S_DATA(t97)[2]=t102;
t97=restore();
restore();
t83=top();
S_DATA(t83)[4]=t97;
t83=restore();
restore();
t78=top();
S_DATA(t78)[2]=t83;
t83=pick(2);
X t104=TO_N(4); // ICONST
save(t104);
X t106=TO_N(5); // ICONST
save(t106);
X t107=select1(TO_N(2),t83); // SELECT (simple)
t106=restore();
X t105=select1(t106,t107); // SELECT
t104=restore();
X t103=select1(t104,t105); // SELECT
t78=top();
S_DATA(t78)[3]=t103;
t78=restore();
restore();
t70=top();
S_DATA(t70)[3]=t78;
t70=restore();
restore();
t68=top();
S_DATA(t68)[1]=t70;
t68=restore();
restore();
t57=top();
S_DATA(t57)[1]=t68;
t57=restore();
restore();
X t108=___rewrite_3arewrite_5fdef2(t57); // REF: rewrite:rewrite_def2
t28=t108;
}else{
X t109=___rewrite_3arewrite_5fdef2(t25); // REF: rewrite:rewrite_def2
t28=t109;}
t24=t28;}
restore();
t16=top();
S_DATA(t16)[0]=t24;
t24=pick(2);
X t110=select1(TO_N(3),t24); // SELECT (simple)
t16=top();
S_DATA(t16)[1]=t110;
t16=restore();
restore();
save(t16); // SEQ
save(t16); // COND
save(t16);
X t112=allocate(2); // CONS
save(t112);
t16=pick(2);
X t113=lf[362]; // CONST
t112=top();
S_DATA(t112)[0]=t113;
t113=pick(2);
X t114=select1(TO_N(2),t113); // SELECT (simple)
t112=top();
S_DATA(t112)[1]=t114;
t112=restore();
restore();
X t115=___fetch(t112); // REF: fetch
X t111;
t16=restore();
if(t115!=F){
X t117=TO_N(2); // ICONST
save(t117);
X t118=select1(TO_N(1),t16); // SELECT (simple)
t117=restore();
X t116=select1(t117,t118); // SELECT
int t120; // ALPHA
check_S(t116,"@");
int t121=S_LENGTH(t116);
save(t116);
X t119=allocate(t121);
save(t119);
for(t120=0;t120<t121;++t120){
X t119=S_DATA(pick(2))[t120];
save(t119);
X t122=allocate(4); // CONS
save(t122);
t119=pick(2);
X t123=lf[343]; // CONST
t122=top();
S_DATA(t122)[0]=t123;
t123=pick(2);
X t124=select1(TO_N(2),t123); // SELECT (simple)
X t125=___tos(t124); // REF: tos
t122=top();
S_DATA(t122)[1]=t125;
t125=pick(2);
X t126=lf[344]; // CONST
t122=top();
S_DATA(t122)[2]=t126;
t126=pick(2);
X t127=select1(TO_N(1),t126); // SELECT (simple)
X t128=___tos(t127); // REF: tos
t122=top();
S_DATA(t122)[3]=t128;
t122=restore();
restore();
X t129=___cat(t122); // REF: cat
save(t129); // SEQ
X t130=____5femit(t129); // REF: _emit
t129=restore();
X t131=TO_N(10); // ICONST
X t132=____5femit(t131); // REF: _emit
S_DATA(top())[t120]=t132;}
t119=restore();
restore();
t111=t119;
}else{
X t133=___id(t16); // REF: id
t111=t133;}
t16=restore();
save(t16); // SEQ
save(t16); // COND
save(t16);
X t135=allocate(2); // CONS
save(t135);
t16=pick(2);
X t136=lf[361]; // CONST
t135=top();
S_DATA(t135)[0]=t136;
t136=pick(2);
X t137=select1(TO_N(2),t136); // SELECT (simple)
t135=top();
S_DATA(t135)[1]=t137;
t135=restore();
restore();
X t138=___fetch(t135); // REF: fetch
X t134;
t16=restore();
if(t138!=F){
X t140=TO_N(2); // ICONST
save(t140);
X t141=select1(TO_N(1),t16); // SELECT (simple)
t140=restore();
X t139=select1(t140,t141); // SELECT
save(t139); // COND
X t144=lf[381]; // CONST
save(t144);
X t145=___id(t139); // REF: id
t144=restore();
X t143=(t144==t145)||eq1(t144,t145)?T:F; // EQ
save(t143); // COND
X t147=___id(t143); // REF: id
X t146;
t143=restore();
if(t147!=F){
X t148=F; // ICONST
t146=t148;
}else{
X t149=T; // ICONST
t146=t149;}
X t142;
t139=restore();
if(t146!=F){
save(t139);
X t150=allocate(2); // CONS
save(t150);
t139=pick(2);
X t152=TO_N(1); // ICONST
save(t152);
X t153=___trans(t139); // REF: trans
t152=restore();
X t151=select1(t152,t153); // SELECT
X t154=___nodups(t151); // REF: nodups
t150=top();
S_DATA(t150)[0]=t154;
t154=pick(2);
X t155=___id(t154); // REF: id
t150=top();
S_DATA(t150)[1]=t155;
t150=restore();
restore();
X t156=___dr(t150); // REF: dr
int t158; // ALPHA
check_S(t156,"@");
int t159=S_LENGTH(t156);
save(t156);
X t157=allocate(t159);
save(t157);
for(t158=0;t158<t159;++t158){
X t157=S_DATA(pick(2))[t158];
save(t157);
X t160=allocate(5); // CONS
save(t160);
t157=pick(2);
X t161=lf[343]; // CONST
t160=top();
S_DATA(t160)[0]=t161;
t161=pick(2);
X t162=select1(TO_N(1),t161); // SELECT (simple)
X t163=___tos(t162); // REF: tos
t160=top();
S_DATA(t160)[1]=t163;
t163=pick(2);
X t164=lf[115]; // CONST
t160=top();
S_DATA(t160)[2]=t164;
t164=pick(2);
save(t164);
X t165=allocate(2); // CONS
save(t165);
t164=pick(2);
X t166=select1(TO_N(1),t164); // SELECT (simple)
t165=top();
S_DATA(t165)[0]=t166;
t166=pick(2);
X t167=select1(TO_N(2),t166); // SELECT (simple)
t165=top();
S_DATA(t165)[1]=t167;
t165=restore();
restore();
X t168=___fetchall(t165); // REF: fetchall
X t169=___len(t168); // REF: len
X t170=___tos(t169); // REF: tos
t160=top();
S_DATA(t160)[3]=t170;
t170=pick(2);
X t171=lf[284]; // CONST
t160=top();
S_DATA(t160)[4]=t171;
t160=restore();
restore();
X t172=___cat(t160); // REF: cat
save(t172); // SEQ
X t173=____5femit(t172); // REF: _emit
t172=restore();
X t174=TO_N(10); // ICONST
X t175=____5femit(t174); // REF: _emit
S_DATA(top())[t158]=t175;}
t157=restore();
restore();
t142=t157;
}else{
X t176=___id(t139); // REF: id
t142=t176;}
t134=t142;
}else{
X t177=___id(t16); // REF: id
t134=t177;}
t16=restore();
save(t16);
X t178=allocate(2); // CONS
save(t178);
t16=pick(2);
X t180=TO_N(1); // ICONST
save(t180);
X t181=select1(TO_N(1),t16); // SELECT (simple)
t180=restore();
X t179=select1(t180,t181); // SELECT
t178=top();
S_DATA(t178)[0]=t179;
t179=pick(2);
X t183=TO_N(0); // ICONST
save(t183);
X t185=TO_N(2); // ICONST
save(t185);
X t186=select1(TO_N(1),t179); // SELECT (simple)
t185=restore();
X t184=select1(t185,t186); // SELECT
X t187=___len(t184); // REF: len
t183=restore();
X t182=(t183==t187)||eq1(t183,t187)?T:F; // EQ
save(t182); // COND
X t189=___id(t182); // REF: id
X t188;
t182=restore();
if(t189!=F){
X t190=F; // ICONST
t188=t190;
}else{
X t191=T; // ICONST
t188=t191;}
t178=top();
S_DATA(t178)[1]=t188;
t178=restore();
restore();
t11=t178;}
RETURN(t11);}
//---------------------------------------- rewrite:rewrite_def2 (rewrite.fp:33)
DEFINE(___rewrite_3arewrite_5fdef2){
ENTRY;
loop:;
tracecall("rewrite.fp:33:  rewrite:rewrite_def2");
save(x);
X t1=allocate(2); // CONS
save(t1);
x=pick(2);
X t2=select1(TO_N(2),x); // SELECT (simple)
t1=top();
S_DATA(t1)[0]=t2;
t2=pick(2);
save(t2);
X t3=allocate(2); // CONS
save(t3);
t2=pick(2);
X t5=TO_N(2); // ICONST
save(t5);
X t6=select1(TO_N(2),t2); // SELECT (simple)
t5=restore();
X t4=select1(t5,t6); // SELECT
t3=top();
S_DATA(t3)[0]=t4;
t4=pick(2);
X t7=select1(TO_N(1),t4); // SELECT (simple)
t3=top();
S_DATA(t3)[1]=t7;
t3=restore();
restore();
X t8=___rewrite_3arewrite1(t3); // REF: rewrite:rewrite1
t1=top();
S_DATA(t1)[1]=t8;
t1=restore();
restore();
save(t1);
X t9=allocate(3); // CONS
save(t9);
t1=pick(2);
X t11=TO_N(1); // ICONST
save(t11);
X t12=select1(TO_N(1),t1); // SELECT (simple)
t11=restore();
X t10=select1(t11,t12); // SELECT
t9=top();
S_DATA(t9)[0]=t10;
t10=pick(2);
X t14=TO_N(1); // ICONST
save(t14);
X t15=select1(TO_N(2),t10); // SELECT (simple)
t14=restore();
X t13=select1(t14,t15); // SELECT
t9=top();
S_DATA(t9)[1]=t13;
t13=pick(2);
X t17=TO_N(2); // ICONST
save(t17);
X t18=select1(TO_N(2),t13); // SELECT (simple)
t17=restore();
X t16=select1(t17,t18); // SELECT
t9=top();
S_DATA(t9)[2]=t16;
t9=restore();
restore();
X t19=___rewrite_3aadd_5fdef(t9); // REF: rewrite:add_def
RETURN(t19);}
//---------------------------------------- rewrite:add_def (rewrite.fp:62)
DEFINE(___rewrite_3aadd_5fdef){
ENTRY;
loop:;
tracecall("rewrite.fp:62:  rewrite:add_def");
save(x);
X t1=allocate(2); // CONS
save(t1);
x=pick(2);
save(x);
X t2=allocate(2); // CONS
save(t2);
x=pick(2);
X t4=TO_N(1); // ICONST
save(t4);
X t5=select1(TO_N(3),x); // SELECT (simple)
t4=restore();
X t3=select1(t4,t5); // SELECT
t2=top();
S_DATA(t2)[0]=t3;
t3=pick(2);
save(t3);
X t6=allocate(2); // CONS
save(t6);
t3=pick(2);
X t7=select1(TO_N(1),t3); // SELECT (simple)
t6=top();
S_DATA(t6)[0]=t7;
t7=pick(2);
X t8=select1(TO_N(2),t7); // SELECT (simple)
t6=top();
S_DATA(t6)[1]=t8;
t6=restore();
restore();
t2=top();
S_DATA(t2)[1]=t6;
t2=restore();
restore();
X t9=___ar(t2); // REF: ar
t1=top();
S_DATA(t1)[0]=t9;
t9=pick(2);
X t10=select1(TO_N(3),t9); // SELECT (simple)
X t11=___tl(t10); // REF: tl
t1=top();
S_DATA(t1)[1]=t11;
t1=restore();
restore();
X t12=___al(t1); // REF: al
RETURN(t12);}
//---------------------------------------- rewrite:node_eq (rewrite.fp:65)
DEFINE(___rewrite_3anode_5feq){
ENTRY;
loop:;
tracecall("rewrite.fp:65:  rewrite:node_eq");
save(x); // COND
X t2=select1(TO_N(1),x); // SELECT (simple)
X t3=___atom(t2); // REF: atom
X t1;
x=restore();
if(t3!=F){
X t4=___eq(x); // REF: eq
t1=t4;
}else{
save(x); // COND
X t6=select1(TO_N(2),x); // SELECT (simple)
X t7=___atom(t6); // REF: atom
X t5;
x=restore();
if(t7!=F){
X t8=F; // ICONST
t5=t8;
}else{
save(x); // COND
save(x);
X t11=select1(TO_N(1),x); // SELECT (simple)
X t12=___len(t11); // REF: len
x=restore();
save(t12);
X t13=select1(TO_N(2),x); // SELECT (simple)
X t14=___len(t13); // REF: len
t12=restore();
X t10=(t12==t14)||eq1(t12,t14)?T:F; // EQ
save(t10); // COND
X t16=___id(t10); // REF: id
X t15;
t10=restore();
if(t16!=F){
X t17=F; // ICONST
t15=t17;
}else{
X t18=T; // ICONST
t15=t18;}
X t9;
x=restore();
if(t15!=F){
X t19=F; // ICONST
t9=t19;
}else{
save(x); // COND
save(x);
X t23=TO_N(1); // ICONST
save(t23);
X t24=select1(TO_N(1),x); // SELECT (simple)
t23=restore();
X t22=select1(t23,t24); // SELECT
x=restore();
save(t22);
X t26=TO_N(1); // ICONST
save(t26);
X t27=select1(TO_N(2),x); // SELECT (simple)
t26=restore();
X t25=select1(t26,t27); // SELECT
t22=restore();
X t21=(t22==t25)||eq1(t22,t25)?T:F; // EQ
X t20;
x=restore();
if(t21!=F){
save(x); // COND
X t29=select1(TO_N(1),x); // SELECT (simple)
X t30=___constnode(t29); // REF: constnode
X t28;
x=restore();
if(t30!=F){
save(x); // COND
X t32=select1(TO_N(2),x); // SELECT (simple)
X t33=___constnode(t32); // REF: constnode
X t31;
x=restore();
if(t33!=F){
save(x);
X t35=select1(TO_N(1),x); // SELECT (simple)
X t36=___tl(t35); // REF: tl
X t37=___tl(t36); // REF: tl
x=restore();
save(t37);
X t38=select1(TO_N(2),x); // SELECT (simple)
X t39=___tl(t38); // REF: tl
X t40=___tl(t39); // REF: tl
t37=restore();
X t34=(t37==t40)||eq1(t37,t40)?T:F; // EQ
t31=t34;
}else{
X t41=F; // ICONST
t31=t41;}
t28=t31;
}else{
save(x); // COND
X t44=lf[381]; // CONST
save(t44);
X t45=select1(TO_N(1),x); // SELECT (simple)
X t46=___tl(t45); // REF: tl
X t47=___tl(t46); // REF: tl
t44=restore();
X t43=(t44==t47)||eq1(t44,t47)?T:F; // EQ
X t42;
x=restore();
if(t43!=F){
X t48=T; // ICONST
t42=t48;
}else{
save(x);
X t49=allocate(2); // CONS
save(t49);
x=pick(2);
X t50=select1(TO_N(1),x); // SELECT (simple)
X t51=___tl(t50); // REF: tl
X t52=___tl(t51); // REF: tl
t49=top();
S_DATA(t49)[0]=t52;
t52=pick(2);
X t53=select1(TO_N(2),t52); // SELECT (simple)
X t54=___tl(t53); // REF: tl
X t55=___tl(t54); // REF: tl
t49=top();
S_DATA(t49)[1]=t55;
t49=restore();
restore();
X t56=___trans(t49); // REF: trans
int t58; // ALPHA
check_S(t56,"@");
int t59=S_LENGTH(t56);
save(t56);
X t57=allocate(t59);
save(t57);
for(t58=0;t58<t59;++t58){
X t57=S_DATA(pick(2))[t58];
X t60=___rewrite_3anode_5feq(t57); // REF: rewrite:node_eq
S_DATA(top())[t58]=t60;}
t57=restore();
restore();
X t61=___all(t57); // REF: all
t42=t61;}
t28=t42;}
t20=t28;
}else{
X t62=F; // ICONST
t20=t62;}
t9=t20;}
t5=t9;}
t1=t5;}
RETURN(t1);}
//---------------------------------------- rewrite:log (rewrite.fp:78)
DEFINE(___rewrite_3alog){
ENTRY;
loop:;
tracecall("rewrite.fp:78:  rewrite:log");
save(x);
X t1=allocate(3); // CONS
save(t1);
x=pick(2);
X t3=TO_N(1); // ICONST
save(t3);
X t4=select1(TO_N(3),x); // SELECT (simple)
t3=restore();
X t2=select1(t3,t4); // SELECT
t1=top();
S_DATA(t1)[0]=t2;
t2=pick(2);
save(t2);
X t5=allocate(2); // CONS
save(t5);
t2=pick(2);
X t7=TO_N(2); // ICONST
save(t7);
X t8=select1(TO_N(3),t2); // SELECT (simple)
t7=restore();
X t6=select1(t7,t8); // SELECT
t5=top();
S_DATA(t5)[0]=t6;
t6=pick(2);
save(t6);
X t9=allocate(2); // CONS
save(t9);
t6=pick(2);
X t10=select1(TO_N(1),t6); // SELECT (simple)
t9=top();
S_DATA(t9)[0]=t10;
t10=pick(2);
X t11=select1(TO_N(2),t10); // SELECT (simple)
t9=top();
S_DATA(t9)[1]=t11;
t9=restore();
restore();
t5=top();
S_DATA(t5)[1]=t9;
t5=restore();
restore();
X t12=___ar(t5); // REF: ar
t1=top();
S_DATA(t1)[1]=t12;
t12=pick(2);
X t14=TO_N(3); // ICONST
save(t14);
X t15=select1(TO_N(3),t12); // SELECT (simple)
t14=restore();
X t13=select1(t14,t15); // SELECT
t1=top();
S_DATA(t1)[2]=t13;
t1=restore();
restore();
RETURN(t1);}
//---------------------------------------- rewrite:gen_def (rewrite.fp:81)
DEFINE(___rewrite_3agen_5fdef){
ENTRY;
loop:;
tracecall("rewrite.fp:81:  rewrite:gen_def");
save(x);
X t1=allocate(3); // CONS
save(t1);
x=pick(2);
X t2=select1(TO_N(2),x); // SELECT (simple)
save(t2);
X t3=allocate(4); // CONS
save(t3);
t2=pick(2);
X t4=lf[117]; // CONST
t3=top();
S_DATA(t3)[0]=t4;
t4=pick(2);
X t5=select1(TO_N(3),t4); // SELECT (simple)
X t6=___tos(t5); // REF: tos
t3=top();
S_DATA(t3)[1]=t6;
t6=pick(2);
X t7=lf[118]; // CONST
t3=top();
S_DATA(t3)[2]=t7;
t7=pick(2);
X t8=select1(TO_N(1),t7); // SELECT (simple)
X t9=___len(t8); // REF: len
X t10=___tos(t9); // REF: tos
t3=top();
S_DATA(t3)[3]=t10;
t3=restore();
restore();
X t11=___cat(t3); // REF: cat
X t12=___toa(t11); // REF: toa
t1=top();
S_DATA(t1)[0]=t12;
t12=pick(2);
X t13=select1(TO_N(1),t12); // SELECT (simple)
t1=top();
S_DATA(t1)[1]=t13;
t13=pick(2);
X t14=select1(TO_N(2),t13); // SELECT (simple)
t1=top();
S_DATA(t1)[2]=t14;
t1=restore();
restore();
save(t1);
X t15=allocate(2); // CONS
save(t15);
t1=pick(2);
X t16=select1(TO_N(1),t1); // SELECT (simple)
t15=top();
S_DATA(t15)[0]=t16;
t16=pick(2);
X t17=___rewrite_3aadd_5fdef(t16); // REF: rewrite:add_def
t15=top();
S_DATA(t15)[1]=t17;
t15=restore();
restore();
RETURN(t15);}
//---------------------------------------- rewrite:common_comp (rewrite.fp:90)
DEFINE(___rewrite_3acommon_5fcomp){
ENTRY;
loop:;
tracecall("rewrite.fp:90:  rewrite:common_comp");
save(x); // KATA
X t5=T; // ICONST
int t2;
check_S(top(),"(|...|)");
int t3=S_LENGTH(top());
for(t2=t3-1;t2>=0;--t2){
X t1=sequence(2,S_DATA(top())[t2],t5);
save(t1); // COND
X t7=select1(TO_N(1),t1); // SELECT (simple)
save(t7);
X t8=allocate(2); // CONS
save(t8);
t7=pick(2);
X t9=select1(TO_N(1),t7); // SELECT (simple)
t8=top();
S_DATA(t8)[0]=t9;
t9=pick(2);
X t10=lf[154]; // CONST
t8=top();
S_DATA(t8)[1]=t10;
t8=restore();
restore();
X t11=___index(t8); // REF: index
X t6;
t1=restore();
if(t11!=F){
X t12=select1(TO_N(2),t1); // SELECT (simple)
t6=t12;
}else{
save(t1); // COND
X t14=select1(TO_N(1),t1); // SELECT (simple)
X t15=F;
save(t14); // PCONSL
if(!IS_S(t14)||S_LENGTH(t14)<1) goto t16;
t14=S_DATA(top())[0];
X t18=lf[320]; // CONST
save(t18);
X t19=___id(t14); // REF: id
t18=restore();
X t17=(t18==t19)||eq1(t18,t19)?T:F; // EQ
if(t17==F) goto t16;
t15=T;
t16:
restore();
X t13;
t1=restore();
if(t15!=F){
save(t1); // COND
save(t1); // OR
X t23=T; // ICONST
save(t23);
X t24=select1(TO_N(2),t1); // SELECT (simple)
t23=restore();
X t22=(t23==t24)||eq1(t23,t24)?T:F; // EQ
X t21=t22;
t1=restore();
if(t21==F){
save(t1);
X t25=allocate(2); // CONS
save(t25);
t1=pick(2);
X t26=select1(TO_N(2),t1); // SELECT (simple)
t25=top();
S_DATA(t25)[0]=t26;
t26=pick(2);
X t28=TO_N(4); // ICONST
save(t28);
X t29=select1(TO_N(1),t26); // SELECT (simple)
t28=restore();
X t27=select1(t28,t29); // SELECT
t25=top();
S_DATA(t25)[1]=t27;
t25=restore();
restore();
X t30=___rewrite_3anode_5feq(t25); // REF: rewrite:node_eq
t21=t30;}
X t20;
t1=restore();
if(t21!=F){
X t32=TO_N(4); // ICONST
save(t32);
X t33=select1(TO_N(1),t1); // SELECT (simple)
t32=restore();
X t31=select1(t32,t33); // SELECT
t20=t31;
}else{
X t34=F; // ICONST
t20=t34;}
t13=t20;
}else{
X t35=F; // ICONST
t13=t35;}
t6=t13;}
t5=t6;}
restore();
X t4=t5;
save(t4); // COND
X t38=lf[119]; // CONST
save(t38);
X t39=___id(t4); // REF: id
t38=restore();
X t37=(t38==t39)||eq1(t38,t39)?T:F; // EQ
save(t37); // COND
X t41=___id(t37); // REF: id
X t40;
t37=restore();
if(t41!=F){
X t42=F; // ICONST
t40=t42;
}else{
X t43=T; // ICONST
t40=t43;}
X t36;
t4=restore();
if(t40!=F){
X t44=___id(t4); // REF: id
t36=t44;
}else{
X t45=F; // ICONST
t36=t45;}
RETURN(t36);}
//---------------------------------------- rewrite:uncomp (rewrite.fp:98)
DEFINE(___rewrite_3auncomp){
ENTRY;
loop:;
tracecall("rewrite.fp:98:  rewrite:uncomp");
save(x); // COND
X t2=F;
save(x); // PCONSL
if(!IS_S(x)||S_LENGTH(x)<1) goto t3;
x=S_DATA(top())[0];
X t5=lf[320]; // CONST
save(t5);
X t6=___id(x); // REF: id
t5=restore();
X t4=(t5==t6)||eq1(t5,t6)?T:F; // EQ
if(t4==F) goto t3;
t2=T;
t3:
restore();
X t1;
x=restore();
if(t2!=F){
X t7=select1(TO_N(3),x); // SELECT (simple)
t1=t7;
}else{
X t8=___id(x); // REF: id
t1=t8;}
RETURN(t1);}
//---------------------------------------- rewrite:check_select (rewrite.fp:102)
DEFINE(___rewrite_3acheck_5fselect){
ENTRY;
loop:;
tracecall("rewrite.fp:102:  rewrite:check_select");
save(x); // COND
save(x);
X t2=allocate(2); // CONS
save(t2);
x=pick(2);
X t4=TO_N(3); // ICONST
save(t4);
X t6=TO_N(3); // ICONST
save(t6);
X t7=select1(TO_N(1),x); // SELECT (simple)
t6=restore();
X t5=select1(t6,t7); // SELECT
t4=restore();
X t3=select1(t4,t5); // SELECT
t2=top();
S_DATA(t2)[0]=t3;
t3=pick(2);
X t8=select1(TO_N(2),t3); // SELECT (simple)
X t9=___len(t8); // REF: len
t2=top();
S_DATA(t2)[1]=t9;
t2=restore();
restore();
save(t2); // COND
save(t2);
X t11=allocate(2); // CONS
save(t11);
t2=pick(2);
X t12=select1(TO_N(1),t2); // SELECT (simple)
t11=top();
S_DATA(t11)[0]=t12;
t12=pick(2);
X t13=TO_N(0); // ICONST
t11=top();
S_DATA(t11)[1]=t13;
t11=restore();
restore();
X t14=___ge(t11); // REF: ge
X t10;
t2=restore();
if(t14!=F){
X t15=___le(t2); // REF: le
t10=t15;
}else{
X t16=F; // ICONST
t10=t16;}
X t1;
x=restore();
if(t10!=F){
save(x);
X t19=TO_N(3); // ICONST
save(t19);
X t21=TO_N(3); // ICONST
save(t21);
X t22=select1(TO_N(1),x); // SELECT (simple)
t21=restore();
X t20=select1(t21,t22); // SELECT
t19=restore();
X t18=select1(t19,t20); // SELECT
x=restore();
save(t18);
X t23=select1(TO_N(2),x); // SELECT (simple)
t18=restore();
X t17=select1(t18,t23); // SELECT
t1=t17;
}else{
save(x);
X t24=allocate(5); // CONS
save(t24);
x=pick(2);
X t25=lf[150]; // CONST
t24=top();
S_DATA(t24)[0]=t25;
t25=pick(2);
X t27=TO_N(2); // ICONST
save(t27);
X t28=select1(TO_N(1),t25); // SELECT (simple)
t27=restore();
X t26=select1(t27,t28); // SELECT
X t29=___tos(t26); // REF: tos
t24=top();
S_DATA(t24)[1]=t29;
t29=pick(2);
X t30=lf[120]; // CONST
t24=top();
S_DATA(t24)[2]=t30;
t30=pick(2);
X t32=TO_N(3); // ICONST
save(t32);
X t34=TO_N(3); // ICONST
save(t34);
X t35=select1(TO_N(1),t30); // SELECT (simple)
t34=restore();
X t33=select1(t34,t35); // SELECT
t32=restore();
X t31=select1(t32,t33); // SELECT
X t36=___tos(t31); // REF: tos
t24=top();
S_DATA(t24)[3]=t36;
t36=pick(2);
X t37=lf[121]; // CONST
t24=top();
S_DATA(t24)[4]=t37;
t24=restore();
restore();
X t38=___cat(t24); // REF: cat
X t39=___quit(t38); // REF: quit
t1=t39;}
RETURN(t1);}
//---------------------------------------- rewrite:rewrite1 (rewrite.fp:116)
DEFINE(___rewrite_3arewrite1){
ENTRY;
loop:;
tracecall("rewrite.fp:116:  rewrite:rewrite1");
save(x); // COND
X t2=select1(TO_N(1),x); // SELECT (simple)
X t3=F;
save(t2); // PCONS
if(!IS_S(t2)||S_LENGTH(t2)!=4) goto t4;
t2=S_DATA(top())[0];
X t6=lf[320]; // CONST
save(t6);
X t7=___id(t2); // REF: id
t6=restore();
X t5=(t6==t7)||eq1(t6,t7)?T:F; // EQ
if(t5==F) goto t4;
// skipped
t5=S_DATA(top())[2];
X t8=F;
save(t5); // PCONS
if(!IS_S(t5)||S_LENGTH(t5)!=3) goto t9;
t5=S_DATA(top())[0];
X t11=lf[293]; // CONST
save(t11);
X t12=___id(t5); // REF: id
t11=restore();
X t10=(t11==t12)||eq1(t11,t12)?T:F; // EQ
if(t10==F) goto t9;
// skipped
t10=S_DATA(top())[2];
X t14=lf[282]; // CONST
save(t14);
X t15=___id(t10); // REF: id
t14=restore();
X t13=(t14==t15)||eq1(t14,t15)?T:F; // EQ
if(t13==F) goto t9;
t8=T;
t9:
restore();
if(t8==F) goto t4;
// skipped
t3=T;
t4:
restore();
X t1;
x=restore();
if(t3!=F){
save(x);
X t16=allocate(2); // CONS
save(t16);
x=pick(2);
X t18=TO_N(4); // ICONST
save(t18);
X t19=select1(TO_N(1),x); // SELECT (simple)
t18=restore();
X t17=select1(t18,t19); // SELECT
t16=top();
S_DATA(t16)[0]=t17;
t17=pick(2);
save(t17);
X t20=allocate(3); // CONS
save(t20);
t17=pick(2);
X t21=lf[122]; // CONST
t20=top();
S_DATA(t20)[0]=t21;
t21=pick(2);
X t23=TO_N(2); // ICONST
save(t23);
X t24=select1(TO_N(1),t21); // SELECT (simple)
t23=restore();
X t22=select1(t23,t24); // SELECT
t20=top();
S_DATA(t20)[1]=t22;
t22=pick(2);
X t25=select1(TO_N(2),t22); // SELECT (simple)
t20=top();
S_DATA(t20)[2]=t25;
t20=restore();
restore();
X t26=___rewrite_3alog(t20); // REF: rewrite:log
t16=top();
S_DATA(t16)[1]=t26;
t16=restore();
restore();
x=t16; // REF: rewrite:rewrite1
goto loop; // tail call: rewrite:rewrite1
X t27;
t1=t27;
}else{
save(x); // COND
X t29=select1(TO_N(1),x); // SELECT (simple)
X t30=F;
save(t29); // PCONS
if(!IS_S(t29)||S_LENGTH(t29)!=4) goto t31;
t29=S_DATA(top())[0];
X t33=lf[320]; // CONST
save(t33);
X t34=___id(t29); // REF: id
t33=restore();
X t32=(t33==t34)||eq1(t33,t34)?T:F; // EQ
if(t32==F) goto t31;
// skipped
// skipped
t32=S_DATA(top())[3];
X t35=F;
save(t32); // PCONS
if(!IS_S(t32)||S_LENGTH(t32)!=3) goto t36;
t32=S_DATA(top())[0];
X t38=lf[293]; // CONST
save(t38);
X t39=___id(t32); // REF: id
t38=restore();
X t37=(t38==t39)||eq1(t38,t39)?T:F; // EQ
if(t37==F) goto t36;
// skipped
t37=S_DATA(top())[2];
X t41=lf[282]; // CONST
save(t41);
X t42=___id(t37); // REF: id
t41=restore();
X t40=(t41==t42)||eq1(t41,t42)?T:F; // EQ
if(t40==F) goto t36;
t35=T;
t36:
restore();
if(t35==F) goto t31;
t30=T;
t31:
restore();
X t28;
x=restore();
if(t30!=F){
save(x);
X t43=allocate(2); // CONS
save(t43);
x=pick(2);
X t45=TO_N(3); // ICONST
save(t45);
X t46=select1(TO_N(1),x); // SELECT (simple)
t45=restore();
X t44=select1(t45,t46); // SELECT
t43=top();
S_DATA(t43)[0]=t44;
t44=pick(2);
save(t44);
X t47=allocate(3); // CONS
save(t47);
t44=pick(2);
X t48=lf[122]; // CONST
t47=top();
S_DATA(t47)[0]=t48;
t48=pick(2);
X t50=TO_N(2); // ICONST
save(t50);
X t51=select1(TO_N(1),t48); // SELECT (simple)
t50=restore();
X t49=select1(t50,t51); // SELECT
t47=top();
S_DATA(t47)[1]=t49;
t49=pick(2);
X t52=select1(TO_N(2),t49); // SELECT (simple)
t47=top();
S_DATA(t47)[2]=t52;
t47=restore();
restore();
X t53=___rewrite_3alog(t47); // REF: rewrite:log
t43=top();
S_DATA(t43)[1]=t53;
t43=restore();
restore();
x=t43; // REF: rewrite:rewrite1
goto loop; // tail call: rewrite:rewrite1
X t54;
t28=t54;
}else{
save(x); // COND
X t56=select1(TO_N(1),x); // SELECT (simple)
X t57=F;
save(t56); // PCONS
if(!IS_S(t56)||S_LENGTH(t56)!=4) goto t58;
t56=S_DATA(top())[0];
X t60=lf[320]; // CONST
save(t60);
X t61=___id(t56); // REF: id
t60=restore();
X t59=(t60==t61)||eq1(t60,t61)?T:F; // EQ
if(t59==F) goto t58;
// skipped
t59=S_DATA(top())[2];
X t62=F;
save(t59); // PCONSL
if(!IS_S(t59)||S_LENGTH(t59)<1) goto t63;
t59=S_DATA(top())[0];
X t65=lf[320]; // CONST
save(t65);
X t66=___id(t59); // REF: id
t65=restore();
X t64=(t65==t66)||eq1(t65,t66)?T:F; // EQ
if(t64==F) goto t63;
t62=T;
t63:
restore();
if(t62==F) goto t58;
// skipped
t57=T;
t58:
restore();
X t55;
x=restore();
if(t57!=F){
save(x);
X t67=allocate(2); // CONS
save(t67);
x=pick(2);
save(x);
X t68=allocate(4); // CONS
save(t68);
x=pick(2);
X t69=lf[320]; // CONST
t68=top();
S_DATA(t68)[0]=t69;
t69=pick(2);
X t71=TO_N(2); // ICONST
save(t71);
X t72=select1(TO_N(1),t69); // SELECT (simple)
t71=restore();
X t70=select1(t71,t72); // SELECT
t68=top();
S_DATA(t68)[1]=t70;
t70=pick(2);
X t74=TO_N(3); // ICONST
save(t74);
X t76=TO_N(3); // ICONST
save(t76);
X t77=select1(TO_N(1),t70); // SELECT (simple)
t76=restore();
X t75=select1(t76,t77); // SELECT
t74=restore();
X t73=select1(t74,t75); // SELECT
t68=top();
S_DATA(t68)[2]=t73;
t73=pick(2);
save(t73);
X t78=allocate(4); // CONS
save(t78);
t73=pick(2);
X t79=lf[320]; // CONST
t78=top();
S_DATA(t78)[0]=t79;
t79=pick(2);
X t81=TO_N(2); // ICONST
save(t81);
X t83=TO_N(3); // ICONST
save(t83);
X t84=select1(TO_N(1),t79); // SELECT (simple)
t83=restore();
X t82=select1(t83,t84); // SELECT
t81=restore();
X t80=select1(t81,t82); // SELECT
t78=top();
S_DATA(t78)[1]=t80;
t80=pick(2);
X t86=TO_N(4); // ICONST
save(t86);
X t88=TO_N(3); // ICONST
save(t88);
X t89=select1(TO_N(1),t80); // SELECT (simple)
t88=restore();
X t87=select1(t88,t89); // SELECT
t86=restore();
X t85=select1(t86,t87); // SELECT
t78=top();
S_DATA(t78)[2]=t85;
t85=pick(2);
X t91=TO_N(4); // ICONST
save(t91);
X t92=select1(TO_N(1),t85); // SELECT (simple)
t91=restore();
X t90=select1(t91,t92); // SELECT
t78=top();
S_DATA(t78)[3]=t90;
t78=restore();
restore();
t68=top();
S_DATA(t68)[3]=t78;
t68=restore();
restore();
t67=top();
S_DATA(t67)[0]=t68;
t68=pick(2);
save(t68);
X t93=allocate(3); // CONS
save(t93);
t68=pick(2);
X t94=lf[123]; // CONST
t93=top();
S_DATA(t93)[0]=t94;
t94=pick(2);
X t96=TO_N(2); // ICONST
save(t96);
X t97=select1(TO_N(1),t94); // SELECT (simple)
t96=restore();
X t95=select1(t96,t97); // SELECT
t93=top();
S_DATA(t93)[1]=t95;
t95=pick(2);
X t98=select1(TO_N(2),t95); // SELECT (simple)
t93=top();
S_DATA(t93)[2]=t98;
t93=restore();
restore();
X t99=___rewrite_3alog(t93); // REF: rewrite:log
t67=top();
S_DATA(t67)[1]=t99;
t67=restore();
restore();
x=t67; // REF: rewrite:rewrite1
goto loop; // tail call: rewrite:rewrite1
X t100;
t55=t100;
}else{
save(x); // COND
X t102=select1(TO_N(1),x); // SELECT (simple)
X t103=F;
save(t102); // PCONS
if(!IS_S(t102)||S_LENGTH(t102)!=4) goto t104;
t102=S_DATA(top())[0];
X t106=lf[320]; // CONST
save(t106);
X t107=___id(t102); // REF: id
t106=restore();
X t105=(t106==t107)||eq1(t106,t107)?T:F; // EQ
if(t105==F) goto t104;
// skipped
t105=S_DATA(top())[2];
X t108=F;
save(t105); // PCONS
if(!IS_S(t105)||S_LENGTH(t105)!=3) goto t109;
t105=S_DATA(top())[0];
X t111=lf[293]; // CONST
save(t111);
X t112=___id(t105); // REF: id
t111=restore();
X t110=(t111==t112)||eq1(t111,t112)?T:F; // EQ
if(t110==F) goto t109;
// skipped
t110=S_DATA(top())[2];
X t114=lf[125]; // CONST
save(t114);
X t115=___id(t110); // REF: id
t114=restore();
X t113=(t114==t115)||eq1(t114,t115)?T:F; // EQ
if(t113==F) goto t109;
t108=T;
t109:
restore();
if(t108==F) goto t104;
t108=S_DATA(top())[3];
X t116=F;
save(t108); // PCONSL
if(!IS_S(t108)||S_LENGTH(t108)<1) goto t117;
t108=S_DATA(top())[0];
X t119=lf[326]; // CONST
save(t119);
X t120=___id(t108); // REF: id
t119=restore();
X t118=(t119==t120)||eq1(t119,t120)?T:F; // EQ
if(t118==F) goto t117;
t116=T;
t117:
restore();
if(t116==F) goto t104;
t103=T;
t104:
restore();
X t101;
x=restore();
if(t103!=F){
save(x);
X t121=allocate(2); // CONS
save(t121);
x=pick(2);
save(x);
X t122=allocate(2); // CONS
save(t122);
x=pick(2);
save(x);
X t123=allocate(2); // CONS
save(t123);
x=pick(2);
X t124=lf[317]; // CONST
t123=top();
S_DATA(t123)[0]=t124;
t124=pick(2);
X t126=TO_N(2); // ICONST
save(t126);
X t127=select1(TO_N(1),t124); // SELECT (simple)
t126=restore();
X t125=select1(t126,t127); // SELECT
t123=top();
S_DATA(t123)[1]=t125;
t123=restore();
restore();
t122=top();
S_DATA(t122)[0]=t123;
t123=pick(2);
X t129=TO_N(4); // ICONST
save(t129);
X t130=select1(TO_N(1),t123); // SELECT (simple)
t129=restore();
X t128=select1(t129,t130); // SELECT
X t131=___tl(t128); // REF: tl
X t132=___tl(t131); // REF: tl
t122=top();
S_DATA(t122)[1]=t132;
t122=restore();
restore();
X t133=___cat(t122); // REF: cat
t121=top();
S_DATA(t121)[0]=t133;
t133=pick(2);
save(t133);
X t134=allocate(3); // CONS
save(t134);
t133=pick(2);
X t135=lf[124]; // CONST
t134=top();
S_DATA(t134)[0]=t135;
t135=pick(2);
X t137=TO_N(2); // ICONST
save(t137);
X t138=select1(TO_N(1),t135); // SELECT (simple)
t137=restore();
X t136=select1(t137,t138); // SELECT
t134=top();
S_DATA(t134)[1]=t136;
t136=pick(2);
X t139=select1(TO_N(2),t136); // SELECT (simple)
t134=top();
S_DATA(t134)[2]=t139;
t134=restore();
restore();
X t140=___rewrite_3alog(t134); // REF: rewrite:log
t121=top();
S_DATA(t121)[1]=t140;
t121=restore();
restore();
x=t121; // REF: rewrite:rewrite1
goto loop; // tail call: rewrite:rewrite1
X t141;
t101=t141;
}else{
save(x); // COND
X t143=select1(TO_N(1),x); // SELECT (simple)
X t144=F;
save(t143); // PCONS
if(!IS_S(t143)||S_LENGTH(t143)!=4) goto t145;
t143=S_DATA(top())[0];
X t147=lf[320]; // CONST
save(t147);
X t148=___id(t143); // REF: id
t147=restore();
X t146=(t147==t148)||eq1(t147,t148)?T:F; // EQ
if(t146==F) goto t145;
// skipped
t146=S_DATA(top())[2];
X t149=F;
save(t146); // PCONS
if(!IS_S(t146)||S_LENGTH(t146)!=3) goto t150;
t146=S_DATA(top())[0];
X t152=lf[293]; // CONST
save(t152);
X t153=___id(t146); // REF: id
t152=restore();
X t151=(t152==t153)||eq1(t152,t153)?T:F; // EQ
if(t151==F) goto t150;
// skipped
t151=S_DATA(top())[2];
X t155=lf[125]; // CONST
save(t155);
X t156=___id(t151); // REF: id
t155=restore();
X t154=(t155==t156)||eq1(t155,t156)?T:F; // EQ
if(t154==F) goto t150;
t149=T;
t150:
restore();
if(t149==F) goto t145;
t149=S_DATA(top())[3];
X t157=F;
save(t149); // PCONSL
if(!IS_S(t149)||S_LENGTH(t149)<3) goto t158;
t149=S_DATA(top())[0];
X t160=lf[320]; // CONST
save(t160);
X t161=___id(t149); // REF: id
t160=restore();
X t159=(t160==t161)||eq1(t160,t161)?T:F; // EQ
if(t159==F) goto t158;
// skipped
t159=S_DATA(top())[2];
X t162=F;
save(t159); // PCONSL
if(!IS_S(t159)||S_LENGTH(t159)<1) goto t163;
t159=S_DATA(top())[0];
X t165=lf[326]; // CONST
save(t165);
X t166=___id(t159); // REF: id
t165=restore();
X t164=(t165==t166)||eq1(t165,t166)?T:F; // EQ
if(t164==F) goto t163;
t162=T;
t163:
restore();
if(t162==F) goto t158;
t157=T;
t158:
restore();
if(t157==F) goto t145;
t144=T;
t145:
restore();
X t142;
x=restore();
if(t144!=F){
save(x);
X t167=allocate(2); // CONS
save(t167);
x=pick(2);
save(x);
X t168=allocate(4); // CONS
save(t168);
x=pick(2);
X t169=lf[320]; // CONST
t168=top();
S_DATA(t168)[0]=t169;
t169=pick(2);
X t171=TO_N(2); // ICONST
save(t171);
X t172=select1(TO_N(1),t169); // SELECT (simple)
t171=restore();
X t170=select1(t171,t172); // SELECT
t168=top();
S_DATA(t168)[1]=t170;
t170=pick(2);
save(t170);
X t173=allocate(2); // CONS
save(t173);
t170=pick(2);
save(t170);
X t174=allocate(2); // CONS
save(t174);
t170=pick(2);
X t175=lf[317]; // CONST
t174=top();
S_DATA(t174)[0]=t175;
t175=pick(2);
X t177=TO_N(2); // ICONST
save(t177);
X t178=select1(TO_N(1),t175); // SELECT (simple)
t177=restore();
X t176=select1(t177,t178); // SELECT
t174=top();
S_DATA(t174)[1]=t176;
t174=restore();
restore();
t173=top();
S_DATA(t173)[0]=t174;
t174=pick(2);
X t180=TO_N(3); // ICONST
save(t180);
X t182=TO_N(4); // ICONST
save(t182);
X t183=select1(TO_N(1),t174); // SELECT (simple)
t182=restore();
X t181=select1(t182,t183); // SELECT
t180=restore();
X t179=select1(t180,t181); // SELECT
X t184=___tl(t179); // REF: tl
X t185=___tl(t184); // REF: tl
t173=top();
S_DATA(t173)[1]=t185;
t173=restore();
restore();
X t186=___cat(t173); // REF: cat
t168=top();
S_DATA(t168)[2]=t186;
t186=pick(2);
X t188=TO_N(4); // ICONST
save(t188);
X t190=TO_N(4); // ICONST
save(t190);
X t191=select1(TO_N(1),t186); // SELECT (simple)
t190=restore();
X t189=select1(t190,t191); // SELECT
t188=restore();
X t187=select1(t188,t189); // SELECT
t168=top();
S_DATA(t168)[3]=t187;
t168=restore();
restore();
t167=top();
S_DATA(t167)[0]=t168;
t168=pick(2);
save(t168);
X t192=allocate(3); // CONS
save(t192);
t168=pick(2);
X t193=lf[126]; // CONST
t192=top();
S_DATA(t192)[0]=t193;
t193=pick(2);
X t195=TO_N(2); // ICONST
save(t195);
X t196=select1(TO_N(1),t193); // SELECT (simple)
t195=restore();
X t194=select1(t195,t196); // SELECT
t192=top();
S_DATA(t192)[1]=t194;
t194=pick(2);
X t197=select1(TO_N(2),t194); // SELECT (simple)
t192=top();
S_DATA(t192)[2]=t197;
t192=restore();
restore();
X t198=___rewrite_3alog(t192); // REF: rewrite:log
t167=top();
S_DATA(t167)[1]=t198;
t167=restore();
restore();
x=t167; // REF: rewrite:rewrite1
goto loop; // tail call: rewrite:rewrite1
X t199;
t142=t199;
}else{
save(x); // COND
X t201=select1(TO_N(1),x); // SELECT (simple)
X t202=F;
save(t201); // PCONS
if(!IS_S(t201)||S_LENGTH(t201)!=4) goto t203;
t201=S_DATA(top())[0];
X t205=lf[320]; // CONST
save(t205);
X t206=___id(t201); // REF: id
t205=restore();
X t204=(t205==t206)||eq1(t205,t206)?T:F; // EQ
if(t204==F) goto t203;
// skipped
t204=S_DATA(top())[2];
X t207=F;
save(t204); // PCONS
if(!IS_S(t204)||S_LENGTH(t204)!=3) goto t208;
t204=S_DATA(top())[0];
X t210=lf[293]; // CONST
save(t210);
X t211=___id(t204); // REF: id
t210=restore();
X t209=(t210==t211)||eq1(t210,t211)?T:F; // EQ
if(t209==F) goto t208;
// skipped
t209=S_DATA(top())[2];
X t213=lf[128]; // CONST
save(t213);
X t214=___id(t209); // REF: id
t213=restore();
X t212=(t213==t214)||eq1(t213,t214)?T:F; // EQ
if(t212==F) goto t208;
t207=T;
t208:
restore();
if(t207==F) goto t203;
t207=S_DATA(top())[3];
X t215=F;
save(t207); // PCONSL
if(!IS_S(t207)||S_LENGTH(t207)<1) goto t216;
t207=S_DATA(top())[0];
X t218=lf[326]; // CONST
save(t218);
X t219=___id(t207); // REF: id
t218=restore();
X t217=(t218==t219)||eq1(t218,t219)?T:F; // EQ
if(t217==F) goto t216;
t215=T;
t216:
restore();
if(t215==F) goto t203;
t202=T;
t203:
restore();
X t200;
x=restore();
if(t202!=F){
save(x);
X t220=allocate(2); // CONS
save(t220);
x=pick(2);
save(x);
X t221=allocate(2); // CONS
save(t221);
x=pick(2);
save(x);
X t222=allocate(2); // CONS
save(t222);
x=pick(2);
X t223=lf[319]; // CONST
t222=top();
S_DATA(t222)[0]=t223;
t223=pick(2);
X t225=TO_N(2); // ICONST
save(t225);
X t226=select1(TO_N(1),t223); // SELECT (simple)
t225=restore();
X t224=select1(t225,t226); // SELECT
t222=top();
S_DATA(t222)[1]=t224;
t222=restore();
restore();
t221=top();
S_DATA(t221)[0]=t222;
t222=pick(2);
X t228=TO_N(4); // ICONST
save(t228);
X t229=select1(TO_N(1),t222); // SELECT (simple)
t228=restore();
X t227=select1(t228,t229); // SELECT
X t230=___tl(t227); // REF: tl
X t231=___tl(t230); // REF: tl
t221=top();
S_DATA(t221)[1]=t231;
t221=restore();
restore();
X t232=___cat(t221); // REF: cat
t220=top();
S_DATA(t220)[0]=t232;
t232=pick(2);
save(t232);
X t233=allocate(3); // CONS
save(t233);
t232=pick(2);
X t234=lf[127]; // CONST
t233=top();
S_DATA(t233)[0]=t234;
t234=pick(2);
X t236=TO_N(2); // ICONST
save(t236);
X t237=select1(TO_N(1),t234); // SELECT (simple)
t236=restore();
X t235=select1(t236,t237); // SELECT
t233=top();
S_DATA(t233)[1]=t235;
t235=pick(2);
X t238=select1(TO_N(2),t235); // SELECT (simple)
t233=top();
S_DATA(t233)[2]=t238;
t233=restore();
restore();
X t239=___rewrite_3alog(t233); // REF: rewrite:log
t220=top();
S_DATA(t220)[1]=t239;
t220=restore();
restore();
x=t220; // REF: rewrite:rewrite1
goto loop; // tail call: rewrite:rewrite1
X t240;
t200=t240;
}else{
save(x); // COND
X t242=select1(TO_N(1),x); // SELECT (simple)
X t243=F;
save(t242); // PCONS
if(!IS_S(t242)||S_LENGTH(t242)!=4) goto t244;
t242=S_DATA(top())[0];
X t246=lf[320]; // CONST
save(t246);
X t247=___id(t242); // REF: id
t246=restore();
X t245=(t246==t247)||eq1(t246,t247)?T:F; // EQ
if(t245==F) goto t244;
// skipped
t245=S_DATA(top())[2];
X t248=F;
save(t245); // PCONS
if(!IS_S(t245)||S_LENGTH(t245)!=3) goto t249;
t245=S_DATA(top())[0];
X t251=lf[293]; // CONST
save(t251);
X t252=___id(t245); // REF: id
t251=restore();
X t250=(t251==t252)||eq1(t251,t252)?T:F; // EQ
if(t250==F) goto t249;
// skipped
t250=S_DATA(top())[2];
X t254=lf[128]; // CONST
save(t254);
X t255=___id(t250); // REF: id
t254=restore();
X t253=(t254==t255)||eq1(t254,t255)?T:F; // EQ
if(t253==F) goto t249;
t248=T;
t249:
restore();
if(t248==F) goto t244;
t248=S_DATA(top())[3];
X t256=F;
save(t248); // PCONSL
if(!IS_S(t248)||S_LENGTH(t248)<3) goto t257;
t248=S_DATA(top())[0];
X t259=lf[320]; // CONST
save(t259);
X t260=___id(t248); // REF: id
t259=restore();
X t258=(t259==t260)||eq1(t259,t260)?T:F; // EQ
if(t258==F) goto t257;
// skipped
t258=S_DATA(top())[2];
X t261=F;
save(t258); // PCONSL
if(!IS_S(t258)||S_LENGTH(t258)<1) goto t262;
t258=S_DATA(top())[0];
X t264=lf[326]; // CONST
save(t264);
X t265=___id(t258); // REF: id
t264=restore();
X t263=(t264==t265)||eq1(t264,t265)?T:F; // EQ
if(t263==F) goto t262;
t261=T;
t262:
restore();
if(t261==F) goto t257;
t256=T;
t257:
restore();
if(t256==F) goto t244;
t243=T;
t244:
restore();
X t241;
x=restore();
if(t243!=F){
save(x);
X t266=allocate(2); // CONS
save(t266);
x=pick(2);
save(x);
X t267=allocate(4); // CONS
save(t267);
x=pick(2);
X t268=lf[320]; // CONST
t267=top();
S_DATA(t267)[0]=t268;
t268=pick(2);
X t270=TO_N(2); // ICONST
save(t270);
X t271=select1(TO_N(1),t268); // SELECT (simple)
t270=restore();
X t269=select1(t270,t271); // SELECT
t267=top();
S_DATA(t267)[1]=t269;
t269=pick(2);
save(t269);
X t272=allocate(2); // CONS
save(t272);
t269=pick(2);
save(t269);
X t273=allocate(2); // CONS
save(t273);
t269=pick(2);
X t274=lf[319]; // CONST
t273=top();
S_DATA(t273)[0]=t274;
t274=pick(2);
X t276=TO_N(2); // ICONST
save(t276);
X t277=select1(TO_N(1),t274); // SELECT (simple)
t276=restore();
X t275=select1(t276,t277); // SELECT
t273=top();
S_DATA(t273)[1]=t275;
t273=restore();
restore();
t272=top();
S_DATA(t272)[0]=t273;
t273=pick(2);
X t279=TO_N(3); // ICONST
save(t279);
X t281=TO_N(4); // ICONST
save(t281);
X t282=select1(TO_N(1),t273); // SELECT (simple)
t281=restore();
X t280=select1(t281,t282); // SELECT
t279=restore();
X t278=select1(t279,t280); // SELECT
X t283=___tl(t278); // REF: tl
X t284=___tl(t283); // REF: tl
t272=top();
S_DATA(t272)[1]=t284;
t272=restore();
restore();
X t285=___cat(t272); // REF: cat
t267=top();
S_DATA(t267)[2]=t285;
t285=pick(2);
X t287=TO_N(4); // ICONST
save(t287);
X t289=TO_N(4); // ICONST
save(t289);
X t290=select1(TO_N(1),t285); // SELECT (simple)
t289=restore();
X t288=select1(t289,t290); // SELECT
t287=restore();
X t286=select1(t287,t288); // SELECT
t267=top();
S_DATA(t267)[3]=t286;
t267=restore();
restore();
t266=top();
S_DATA(t266)[0]=t267;
t267=pick(2);
save(t267);
X t291=allocate(3); // CONS
save(t291);
t267=pick(2);
X t292=lf[129]; // CONST
t291=top();
S_DATA(t291)[0]=t292;
t292=pick(2);
X t294=TO_N(2); // ICONST
save(t294);
X t295=select1(TO_N(1),t292); // SELECT (simple)
t294=restore();
X t293=select1(t294,t295); // SELECT
t291=top();
S_DATA(t291)[1]=t293;
t293=pick(2);
X t296=select1(TO_N(2),t293); // SELECT (simple)
t291=top();
S_DATA(t291)[2]=t296;
t291=restore();
restore();
X t297=___rewrite_3alog(t291); // REF: rewrite:log
t266=top();
S_DATA(t266)[1]=t297;
t266=restore();
restore();
x=t266; // REF: rewrite:rewrite1
goto loop; // tail call: rewrite:rewrite1
X t298;
t241=t298;
}else{
save(x); // COND
X t300=select1(TO_N(1),x); // SELECT (simple)
X t301=F;
save(t300); // PCONS
if(!IS_S(t300)||S_LENGTH(t300)!=4) goto t302;
t300=S_DATA(top())[0];
X t304=lf[320]; // CONST
save(t304);
X t305=___id(t300); // REF: id
t304=restore();
X t303=(t304==t305)||eq1(t304,t305)?T:F; // EQ
if(t303==F) goto t302;
// skipped
t303=S_DATA(top())[2];
X t306=F;
save(t303); // PCONSL
if(!IS_S(t303)||S_LENGTH(t303)<1) goto t307;
t303=S_DATA(top())[0];
X t309=lf[296]; // CONST
save(t309);
X t310=___id(t303); // REF: id
t309=restore();
X t308=(t309==t310)||eq1(t309,t310)?T:F; // EQ
if(t308==F) goto t307;
t306=T;
t307:
restore();
if(t306==F) goto t302;
t306=S_DATA(top())[3];
X t311=F;
save(t306); // PCONSL
if(!IS_S(t306)||S_LENGTH(t306)<1) goto t312;
t306=S_DATA(top())[0];
X t314=lf[296]; // CONST
save(t314);
X t315=___id(t306); // REF: id
t314=restore();
X t313=(t314==t315)||eq1(t314,t315)?T:F; // EQ
if(t313==F) goto t312;
t311=T;
t312:
restore();
if(t311==F) goto t302;
t301=T;
t302:
restore();
X t299;
x=restore();
if(t301!=F){
save(x);
X t316=allocate(2); // CONS
save(t316);
x=pick(2);
save(x);
X t317=allocate(3); // CONS
save(t317);
x=pick(2);
X t318=lf[296]; // CONST
t317=top();
S_DATA(t317)[0]=t318;
t318=pick(2);
X t320=TO_N(2); // ICONST
save(t320);
X t322=TO_N(3); // ICONST
save(t322);
X t323=select1(TO_N(1),t318); // SELECT (simple)
t322=restore();
X t321=select1(t322,t323); // SELECT
t320=restore();
X t319=select1(t320,t321); // SELECT
t317=top();
S_DATA(t317)[1]=t319;
t319=pick(2);
save(t319);
X t324=allocate(4); // CONS
save(t324);
t319=pick(2);
X t325=lf[320]; // CONST
t324=top();
S_DATA(t324)[0]=t325;
t325=pick(2);
X t327=TO_N(2); // ICONST
save(t327);
X t328=select1(TO_N(1),t325); // SELECT (simple)
t327=restore();
X t326=select1(t327,t328); // SELECT
t324=top();
S_DATA(t324)[1]=t326;
t326=pick(2);
X t330=TO_N(3); // ICONST
save(t330);
X t332=TO_N(3); // ICONST
save(t332);
X t333=select1(TO_N(1),t326); // SELECT (simple)
t332=restore();
X t331=select1(t332,t333); // SELECT
t330=restore();
X t329=select1(t330,t331); // SELECT
t324=top();
S_DATA(t324)[2]=t329;
t329=pick(2);
X t335=TO_N(3); // ICONST
save(t335);
X t337=TO_N(4); // ICONST
save(t337);
X t338=select1(TO_N(1),t329); // SELECT (simple)
t337=restore();
X t336=select1(t337,t338); // SELECT
t335=restore();
X t334=select1(t335,t336); // SELECT
t324=top();
S_DATA(t324)[3]=t334;
t324=restore();
restore();
t317=top();
S_DATA(t317)[2]=t324;
t317=restore();
restore();
t316=top();
S_DATA(t316)[0]=t317;
t317=pick(2);
save(t317);
X t339=allocate(3); // CONS
save(t339);
t317=pick(2);
X t340=lf[130]; // CONST
t339=top();
S_DATA(t339)[0]=t340;
t340=pick(2);
X t342=TO_N(2); // ICONST
save(t342);
X t343=select1(TO_N(1),t340); // SELECT (simple)
t342=restore();
X t341=select1(t342,t343); // SELECT
t339=top();
S_DATA(t339)[1]=t341;
t341=pick(2);
X t344=select1(TO_N(2),t341); // SELECT (simple)
t339=top();
S_DATA(t339)[2]=t344;
t339=restore();
restore();
X t345=___rewrite_3alog(t339); // REF: rewrite:log
t316=top();
S_DATA(t316)[1]=t345;
t316=restore();
restore();
x=t316; // REF: rewrite:rewrite1
goto loop; // tail call: rewrite:rewrite1
X t346;
t299=t346;
}else{
save(x); // COND
X t348=select1(TO_N(1),x); // SELECT (simple)
X t349=F;
save(t348); // PCONS
if(!IS_S(t348)||S_LENGTH(t348)!=4) goto t350;
t348=S_DATA(top())[0];
X t352=lf[320]; // CONST
save(t352);
X t353=___id(t348); // REF: id
t352=restore();
X t351=(t352==t353)||eq1(t352,t353)?T:F; // EQ
if(t351==F) goto t350;
// skipped
t351=S_DATA(top())[2];
X t354=F;
save(t351); // PCONSL
if(!IS_S(t351)||S_LENGTH(t351)<1) goto t355;
t351=S_DATA(top())[0];
X t357=lf[296]; // CONST
save(t357);
X t358=___id(t351); // REF: id
t357=restore();
X t356=(t357==t358)||eq1(t357,t358)?T:F; // EQ
if(t356==F) goto t355;
t354=T;
t355:
restore();
if(t354==F) goto t350;
t354=S_DATA(top())[3];
X t359=F;
save(t354); // PCONSL
if(!IS_S(t354)||S_LENGTH(t354)<3) goto t360;
t354=S_DATA(top())[0];
X t362=lf[320]; // CONST
save(t362);
X t363=___id(t354); // REF: id
t362=restore();
X t361=(t362==t363)||eq1(t362,t363)?T:F; // EQ
if(t361==F) goto t360;
// skipped
t361=S_DATA(top())[2];
X t364=F;
save(t361); // PCONSL
if(!IS_S(t361)||S_LENGTH(t361)<1) goto t365;
t361=S_DATA(top())[0];
X t367=lf[296]; // CONST
save(t367);
X t368=___id(t361); // REF: id
t367=restore();
X t366=(t367==t368)||eq1(t367,t368)?T:F; // EQ
if(t366==F) goto t365;
t364=T;
t365:
restore();
if(t364==F) goto t360;
t359=T;
t360:
restore();
if(t359==F) goto t350;
t349=T;
t350:
restore();
X t347;
x=restore();
if(t349!=F){
save(x);
X t369=allocate(2); // CONS
save(t369);
x=pick(2);
save(x);
X t370=allocate(4); // CONS
save(t370);
x=pick(2);
X t371=lf[320]; // CONST
t370=top();
S_DATA(t370)[0]=t371;
t371=pick(2);
X t373=TO_N(2); // ICONST
save(t373);
X t374=select1(TO_N(1),t371); // SELECT (simple)
t373=restore();
X t372=select1(t373,t374); // SELECT
t370=top();
S_DATA(t370)[1]=t372;
t372=pick(2);
save(t372);
X t375=allocate(3); // CONS
save(t375);
t372=pick(2);
X t376=lf[296]; // CONST
t375=top();
S_DATA(t375)[0]=t376;
t376=pick(2);
X t378=TO_N(2); // ICONST
save(t378);
X t380=TO_N(3); // ICONST
save(t380);
X t381=select1(TO_N(1),t376); // SELECT (simple)
t380=restore();
X t379=select1(t380,t381); // SELECT
t378=restore();
X t377=select1(t378,t379); // SELECT
t375=top();
S_DATA(t375)[1]=t377;
t377=pick(2);
save(t377);
X t382=allocate(4); // CONS
save(t382);
t377=pick(2);
X t383=lf[320]; // CONST
t382=top();
S_DATA(t382)[0]=t383;
t383=pick(2);
X t385=TO_N(2); // ICONST
save(t385);
X t386=select1(TO_N(1),t383); // SELECT (simple)
t385=restore();
X t384=select1(t385,t386); // SELECT
t382=top();
S_DATA(t382)[1]=t384;
t384=pick(2);
X t388=TO_N(3); // ICONST
save(t388);
X t390=TO_N(3); // ICONST
save(t390);
X t391=select1(TO_N(1),t384); // SELECT (simple)
t390=restore();
X t389=select1(t390,t391); // SELECT
t388=restore();
X t387=select1(t388,t389); // SELECT
t382=top();
S_DATA(t382)[2]=t387;
t387=pick(2);
X t393=TO_N(3); // ICONST
save(t393);
X t395=TO_N(3); // ICONST
save(t395);
X t397=TO_N(4); // ICONST
save(t397);
X t398=select1(TO_N(1),t387); // SELECT (simple)
t397=restore();
X t396=select1(t397,t398); // SELECT
t395=restore();
X t394=select1(t395,t396); // SELECT
t393=restore();
X t392=select1(t393,t394); // SELECT
t382=top();
S_DATA(t382)[3]=t392;
t382=restore();
restore();
t375=top();
S_DATA(t375)[2]=t382;
t375=restore();
restore();
t370=top();
S_DATA(t370)[2]=t375;
t375=pick(2);
X t400=TO_N(4); // ICONST
save(t400);
X t402=TO_N(4); // ICONST
save(t402);
X t403=select1(TO_N(1),t375); // SELECT (simple)
t402=restore();
X t401=select1(t402,t403); // SELECT
t400=restore();
X t399=select1(t400,t401); // SELECT
t370=top();
S_DATA(t370)[3]=t399;
t370=restore();
restore();
t369=top();
S_DATA(t369)[0]=t370;
t370=pick(2);
save(t370);
X t404=allocate(3); // CONS
save(t404);
t370=pick(2);
X t405=lf[131]; // CONST
t404=top();
S_DATA(t404)[0]=t405;
t405=pick(2);
X t407=TO_N(2); // ICONST
save(t407);
X t408=select1(TO_N(1),t405); // SELECT (simple)
t407=restore();
X t406=select1(t407,t408); // SELECT
t404=top();
S_DATA(t404)[1]=t406;
t406=pick(2);
X t409=select1(TO_N(2),t406); // SELECT (simple)
t404=top();
S_DATA(t404)[2]=t409;
t404=restore();
restore();
X t410=___rewrite_3alog(t404); // REF: rewrite:log
t369=top();
S_DATA(t369)[1]=t410;
t369=restore();
restore();
x=t369; // REF: rewrite:rewrite1
goto loop; // tail call: rewrite:rewrite1
X t411;
t347=t411;
}else{
save(x); // COND
X t413=select1(TO_N(1),x); // SELECT (simple)
X t414=F;
save(t413); // PCONSL
if(!IS_S(t413)||S_LENGTH(t413)<3) goto t415;
t413=S_DATA(top())[0];
X t417=lf[287]; // CONST
save(t417);
X t418=___id(t413); // REF: id
t417=restore();
X t416=(t417==t418)||eq1(t417,t418)?T:F; // EQ
if(t416==F) goto t415;
// skipped
t416=S_DATA(top())[2];
X t419=F;
save(t416); // PCONSL
if(!IS_S(t416)||S_LENGTH(t416)<1) goto t420;
t416=S_DATA(top())[0];
save(t416); // OR
X t423=lf[290]; // CONST
save(t423);
X t424=___id(t416); // REF: id
t423=restore();
X t422=(t423==t424)||eq1(t423,t424)?T:F; // EQ
X t421=t422;
t416=restore();
if(t421==F){
X t426=lf[291]; // CONST
save(t426);
X t427=___id(t416); // REF: id
t426=restore();
X t425=(t426==t427)||eq1(t426,t427)?T:F; // EQ
t421=t425;}
if(t421==F) goto t420;
t419=T;
t420:
restore();
if(t419==F) goto t415;
t414=T;
t415:
restore();
X t412;
x=restore();
if(t414!=F){
save(x);
X t428=allocate(2); // CONS
save(t428);
x=pick(2);
save(x); // COND
X t431=TO_N(3); // ICONST
save(t431);
X t433=TO_N(3); // ICONST
save(t433);
X t434=select1(TO_N(1),x); // SELECT (simple)
t433=restore();
X t432=select1(t433,t434); // SELECT
t431=restore();
X t430=select1(t431,t432); // SELECT
X t429;
x=restore();
if(t430!=F){
X t436=TO_N(4); // ICONST
save(t436);
X t437=select1(TO_N(1),x); // SELECT (simple)
t436=restore();
X t435=select1(t436,t437); // SELECT
t429=t435;
}else{
X t439=TO_N(5); // ICONST
save(t439);
X t440=select1(TO_N(1),x); // SELECT (simple)
t439=restore();
X t438=select1(t439,t440); // SELECT
t429=t438;}
t428=top();
S_DATA(t428)[0]=t429;
t429=pick(2);
save(t429);
X t441=allocate(3); // CONS
save(t441);
t429=pick(2);
X t442=lf[132]; // CONST
t441=top();
S_DATA(t441)[0]=t442;
t442=pick(2);
X t444=TO_N(2); // ICONST
save(t444);
X t445=select1(TO_N(1),t442); // SELECT (simple)
t444=restore();
X t443=select1(t444,t445); // SELECT
t441=top();
S_DATA(t441)[1]=t443;
t443=pick(2);
X t446=select1(TO_N(2),t443); // SELECT (simple)
t441=top();
S_DATA(t441)[2]=t446;
t441=restore();
restore();
X t447=___rewrite_3alog(t441); // REF: rewrite:log
t428=top();
S_DATA(t428)[1]=t447;
t428=restore();
restore();
x=t428; // REF: rewrite:rewrite1
goto loop; // tail call: rewrite:rewrite1
X t448;
t412=t448;
}else{
save(x); // COND
X t450=select1(TO_N(1),x); // SELECT (simple)
X t451=F;
save(t450); // PCONS
if(!IS_S(t450)||S_LENGTH(t450)!=4) goto t452;
t450=S_DATA(top())[0];
X t454=lf[317]; // CONST
save(t454);
X t455=___id(t450); // REF: id
t454=restore();
X t453=(t454==t455)||eq1(t454,t455)?T:F; // EQ
if(t453==F) goto t452;
// skipped
t453=S_DATA(top())[2];
X t456=F;
save(t453); // PCONS
if(!IS_S(t453)||S_LENGTH(t453)!=3) goto t457;
t453=S_DATA(top())[0];
X t459=lf[293]; // CONST
save(t459);
X t460=___id(t453); // REF: id
t459=restore();
X t458=(t459==t460)||eq1(t459,t460)?T:F; // EQ
if(t458==F) goto t457;
// skipped
t458=S_DATA(top())[2];
X t462=lf[282]; // CONST
save(t462);
X t463=___id(t458); // REF: id
t462=restore();
X t461=(t462==t463)||eq1(t462,t463)?T:F; // EQ
if(t461==F) goto t457;
t456=T;
t457:
restore();
if(t456==F) goto t452;
t456=S_DATA(top())[3];
X t464=F;
save(t456); // PCONS
if(!IS_S(t456)||S_LENGTH(t456)!=3) goto t465;
t456=S_DATA(top())[0];
X t467=lf[293]; // CONST
save(t467);
X t468=___id(t456); // REF: id
t467=restore();
X t466=(t467==t468)||eq1(t467,t468)?T:F; // EQ
if(t466==F) goto t465;
// skipped
t466=S_DATA(top())[2];
X t470=lf[282]; // CONST
save(t470);
X t471=___id(t466); // REF: id
t470=restore();
X t469=(t470==t471)||eq1(t470,t471)?T:F; // EQ
if(t469==F) goto t465;
t464=T;
t465:
restore();
if(t464==F) goto t452;
t451=T;
t452:
restore();
X t449;
x=restore();
if(t451!=F){
save(x);
X t472=allocate(2); // CONS
save(t472);
x=pick(2);
save(x);
X t473=allocate(3); // CONS
save(t473);
x=pick(2);
X t474=lf[291]; // CONST
t473=top();
S_DATA(t473)[0]=t474;
t474=pick(2);
X t476=TO_N(2); // ICONST
save(t476);
X t477=select1(TO_N(1),t474); // SELECT (simple)
t476=restore();
X t475=select1(t476,t477); // SELECT
t473=top();
S_DATA(t473)[1]=t475;
t475=pick(2);
X t478=T; // ICONST
t473=top();
S_DATA(t473)[2]=t478;
t473=restore();
restore();
t472=top();
S_DATA(t472)[0]=t473;
t473=pick(2);
save(t473);
X t479=allocate(3); // CONS
save(t479);
t473=pick(2);
X t480=lf[133]; // CONST
t479=top();
S_DATA(t479)[0]=t480;
t480=pick(2);
X t482=TO_N(2); // ICONST
save(t482);
X t483=select1(TO_N(1),t480); // SELECT (simple)
t482=restore();
X t481=select1(t482,t483); // SELECT
t479=top();
S_DATA(t479)[1]=t481;
t481=pick(2);
X t484=select1(TO_N(2),t481); // SELECT (simple)
t479=top();
S_DATA(t479)[2]=t484;
t479=restore();
restore();
X t485=___rewrite_3alog(t479); // REF: rewrite:log
t472=top();
S_DATA(t472)[1]=t485;
t472=restore();
restore();
x=t472; // REF: rewrite:rewrite1
goto loop; // tail call: rewrite:rewrite1
X t486;
t449=t486;
}else{
save(x); // COND
X t488=select1(TO_N(1),x); // SELECT (simple)
X t489=F;
save(t488); // PCONS
if(!IS_S(t488)||S_LENGTH(t488)!=4) goto t490;
t488=S_DATA(top())[0];
X t492=lf[317]; // CONST
save(t492);
X t493=___id(t488); // REF: id
t492=restore();
X t491=(t492==t493)||eq1(t492,t493)?T:F; // EQ
if(t491==F) goto t490;
// skipped
t491=S_DATA(top())[2];
X t494=F;
save(t491); // PCONSL
if(!IS_S(t491)||S_LENGTH(t491)<1) goto t495;
t491=S_DATA(top())[0];
save(t491); // OR
X t498=lf[290]; // CONST
save(t498);
X t499=___id(t491); // REF: id
t498=restore();
X t497=(t498==t499)||eq1(t498,t499)?T:F; // EQ
X t496=t497;
t491=restore();
if(t496==F){
X t501=lf[291]; // CONST
save(t501);
X t502=___id(t491); // REF: id
t501=restore();
X t500=(t501==t502)||eq1(t501,t502)?T:F; // EQ
t496=t500;}
if(t496==F) goto t495;
t494=T;
t495:
restore();
if(t494==F) goto t490;
t494=S_DATA(top())[3];
X t503=F;
save(t494); // PCONSL
if(!IS_S(t494)||S_LENGTH(t494)<1) goto t504;
t494=S_DATA(top())[0];
save(t494); // OR
X t507=lf[290]; // CONST
save(t507);
X t508=___id(t494); // REF: id
t507=restore();
X t506=(t507==t508)||eq1(t507,t508)?T:F; // EQ
X t505=t506;
t494=restore();
if(t505==F){
X t510=lf[291]; // CONST
save(t510);
X t511=___id(t494); // REF: id
t510=restore();
X t509=(t510==t511)||eq1(t510,t511)?T:F; // EQ
t505=t509;}
if(t505==F) goto t504;
t503=T;
t504:
restore();
if(t503==F) goto t490;
t489=T;
t490:
restore();
X t487;
x=restore();
if(t489!=F){
save(x);
X t512=allocate(2); // CONS
save(t512);
x=pick(2);
save(x);
X t513=allocate(3); // CONS
save(t513);
x=pick(2);
X t514=lf[291]; // CONST
t513=top();
S_DATA(t513)[0]=t514;
t514=pick(2);
X t516=TO_N(2); // ICONST
save(t516);
X t517=select1(TO_N(1),t514); // SELECT (simple)
t516=restore();
X t515=select1(t516,t517); // SELECT
t513=top();
S_DATA(t513)[1]=t515;
t515=pick(2);
save(t515);
X t520=TO_N(3); // ICONST
save(t520);
X t522=TO_N(3); // ICONST
save(t522);
X t523=select1(TO_N(1),t515); // SELECT (simple)
t522=restore();
X t521=select1(t522,t523); // SELECT
t520=restore();
X t519=select1(t520,t521); // SELECT
t515=restore();
save(t519);
X t525=TO_N(3); // ICONST
save(t525);
X t527=TO_N(4); // ICONST
save(t527);
X t528=select1(TO_N(1),t515); // SELECT (simple)
t527=restore();
X t526=select1(t527,t528); // SELECT
t525=restore();
X t524=select1(t525,t526); // SELECT
t519=restore();
X t518=(t519==t524)||eq1(t519,t524)?T:F; // EQ
t513=top();
S_DATA(t513)[2]=t518;
t513=restore();
restore();
t512=top();
S_DATA(t512)[0]=t513;
t513=pick(2);
save(t513);
X t529=allocate(3); // CONS
save(t529);
t513=pick(2);
X t530=lf[134]; // CONST
t529=top();
S_DATA(t529)[0]=t530;
t530=pick(2);
X t532=TO_N(2); // ICONST
save(t532);
X t533=select1(TO_N(1),t530); // SELECT (simple)
t532=restore();
X t531=select1(t532,t533); // SELECT
t529=top();
S_DATA(t529)[1]=t531;
t531=pick(2);
X t534=select1(TO_N(2),t531); // SELECT (simple)
t529=top();
S_DATA(t529)[2]=t534;
t529=restore();
restore();
X t535=___rewrite_3alog(t529); // REF: rewrite:log
t512=top();
S_DATA(t512)[1]=t535;
t512=restore();
restore();
x=t512; // REF: rewrite:rewrite1
goto loop; // tail call: rewrite:rewrite1
X t536;
t487=t536;
}else{
save(x); // COND
X t538=select1(TO_N(1),x); // SELECT (simple)
X t539=F;
save(t538); // PCONSL
if(!IS_S(t538)||S_LENGTH(t538)<1) goto t540;
t538=S_DATA(top())[0];
X t542=lf[323]; // CONST
save(t542);
X t543=___id(t538); // REF: id
t542=restore();
X t541=(t542==t543)||eq1(t542,t543)?T:F; // EQ
if(t541==F) goto t540;
t539=T;
t540:
restore();
X t537;
x=restore();
if(t539!=F){
save(x);
X t544=allocate(2); // CONS
save(t544);
x=pick(2);
X t545=select1(TO_N(1),x); // SELECT (simple)
t544=top();
S_DATA(t544)[0]=t545;
t545=pick(2);
save(t545);
X t546=allocate(2); // CONS
save(t546);
t545=pick(2);
X t548=TO_N(3); // ICONST
save(t548);
X t549=select1(TO_N(1),t545); // SELECT (simple)
t548=restore();
X t547=select1(t548,t549); // SELECT
t546=top();
S_DATA(t546)[0]=t547;
t547=pick(2);
X t550=select1(TO_N(2),t547); // SELECT (simple)
t546=top();
S_DATA(t546)[1]=t550;
t546=restore();
restore();
X t551=___rewrite_3agen_5fdef(t546); // REF: rewrite:gen_def
t544=top();
S_DATA(t544)[1]=t551;
t544=restore();
restore();
save(t544);
X t552=allocate(2); // CONS
save(t552);
t544=pick(2);
save(t544);
X t553=allocate(3); // CONS
save(t553);
t544=pick(2);
X t554=lf[324]; // CONST
t553=top();
S_DATA(t553)[0]=t554;
t554=pick(2);
X t556=TO_N(2); // ICONST
save(t556);
X t557=select1(TO_N(1),t554); // SELECT (simple)
t556=restore();
X t555=select1(t556,t557); // SELECT
t553=top();
S_DATA(t553)[1]=t555;
t555=pick(2);
X t559=TO_N(1); // ICONST
save(t559);
X t560=select1(TO_N(2),t555); // SELECT (simple)
t559=restore();
X t558=select1(t559,t560); // SELECT
t553=top();
S_DATA(t553)[2]=t558;
t553=restore();
restore();
t552=top();
S_DATA(t552)[0]=t553;
t553=pick(2);
save(t553);
X t561=allocate(3); // CONS
save(t561);
t553=pick(2);
X t562=lf[135]; // CONST
t561=top();
S_DATA(t561)[0]=t562;
t562=pick(2);
X t564=TO_N(2); // ICONST
save(t564);
X t565=select1(TO_N(1),t562); // SELECT (simple)
t564=restore();
X t563=select1(t564,t565); // SELECT
t561=top();
S_DATA(t561)[1]=t563;
t563=pick(2);
X t567=TO_N(2); // ICONST
save(t567);
X t568=select1(TO_N(2),t563); // SELECT (simple)
t567=restore();
X t566=select1(t567,t568); // SELECT
t561=top();
S_DATA(t561)[2]=t566;
t561=restore();
restore();
X t569=___rewrite_3alog(t561); // REF: rewrite:log
t552=top();
S_DATA(t552)[1]=t569;
t552=restore();
restore();
x=t552; // REF: rewrite:rewrite1
goto loop; // tail call: rewrite:rewrite1
X t570;
t537=t570;
}else{
save(x); // COND
X t572=select1(TO_N(1),x); // SELECT (simple)
X t573=F;
save(t572); // PCONS
if(!IS_S(t572)||S_LENGTH(t572)!=4) goto t574;
t572=S_DATA(top())[0];
X t576=lf[320]; // CONST
save(t576);
X t577=___id(t572); // REF: id
t576=restore();
X t575=(t576==t577)||eq1(t576,t577)?T:F; // EQ
if(t575==F) goto t574;
// skipped
t575=S_DATA(top())[2];
X t578=F;
save(t575); // PCONS
if(!IS_S(t575)||S_LENGTH(t575)!=3) goto t579;
t575=S_DATA(top())[0];
X t581=lf[293]; // CONST
save(t581);
X t582=___id(t575); // REF: id
t581=restore();
X t580=(t581==t582)||eq1(t581,t582)?T:F; // EQ
if(t580==F) goto t579;
// skipped
t580=S_DATA(top())[2];
X t584=lf[294]; // CONST
save(t584);
X t585=___id(t580); // REF: id
t584=restore();
X t583=(t584==t585)||eq1(t584,t585)?T:F; // EQ
if(t583==F) goto t579;
t578=T;
t579:
restore();
if(t578==F) goto t574;
t578=S_DATA(top())[3];
X t586=F;
save(t578); // PCONS
if(!IS_S(t578)||S_LENGTH(t578)!=4) goto t587;
t578=S_DATA(top())[0];
X t589=lf[326]; // CONST
save(t589);
X t590=___id(t578); // REF: id
t589=restore();
X t588=(t589==t590)||eq1(t589,t590)?T:F; // EQ
if(t588==F) goto t587;
// skipped
t588=S_DATA(top())[2];
X t591=F;
save(t588); // PCONSL
if(!IS_S(t588)||S_LENGTH(t588)<1) goto t592;
t588=S_DATA(top())[0];
X t594=lf[324]; // CONST
save(t594);
X t595=___id(t588); // REF: id
t594=restore();
X t593=(t594==t595)||eq1(t594,t595)?T:F; // EQ
if(t593==F) goto t592;
t591=T;
t592:
restore();
if(t591==F) goto t587;
// skipped
t586=T;
t587:
restore();
if(t586==F) goto t574;
t573=T;
t574:
restore();
X t571;
x=restore();
if(t573!=F){
save(x);
X t596=allocate(2); // CONS
save(t596);
x=pick(2);
save(x);
X t597=allocate(4); // CONS
save(t597);
x=pick(2);
X t598=lf[320]; // CONST
t597=top();
S_DATA(t597)[0]=t598;
t598=pick(2);
X t600=TO_N(2); // ICONST
save(t600);
X t601=select1(TO_N(1),t598); // SELECT (simple)
t600=restore();
X t599=select1(t600,t601); // SELECT
t597=top();
S_DATA(t597)[1]=t599;
t599=pick(2);
save(t599);
X t602=allocate(2); // CONS
save(t602);
t599=pick(2);
X t603=lf[293]; // CONST
t602=top();
S_DATA(t602)[0]=t603;
t603=pick(2);
X t605=TO_N(3); // ICONST
save(t605);
X t607=TO_N(4); // ICONST
save(t607);
X t608=select1(TO_N(1),t603); // SELECT (simple)
t607=restore();
X t606=select1(t607,t608); // SELECT
t605=restore();
X t604=select1(t605,t606); // SELECT
X t609=___tl(t604); // REF: tl
t602=top();
S_DATA(t602)[1]=t609;
t602=restore();
restore();
X t610=___al(t602); // REF: al
t597=top();
S_DATA(t597)[2]=t610;
t610=pick(2);
X t612=TO_N(4); // ICONST
save(t612);
X t614=TO_N(4); // ICONST
save(t614);
X t615=select1(TO_N(1),t610); // SELECT (simple)
t614=restore();
X t613=select1(t614,t615); // SELECT
t612=restore();
X t611=select1(t612,t613); // SELECT
t597=top();
S_DATA(t597)[3]=t611;
t597=restore();
restore();
t596=top();
S_DATA(t596)[0]=t597;
t597=pick(2);
save(t597);
X t616=allocate(3); // CONS
save(t616);
t597=pick(2);
X t617=lf[136]; // CONST
t616=top();
S_DATA(t616)[0]=t617;
t617=pick(2);
X t619=TO_N(2); // ICONST
save(t619);
X t620=select1(TO_N(1),t617); // SELECT (simple)
t619=restore();
X t618=select1(t619,t620); // SELECT
t616=top();
S_DATA(t616)[1]=t618;
t618=pick(2);
X t621=select1(TO_N(2),t618); // SELECT (simple)
t616=top();
S_DATA(t616)[2]=t621;
t616=restore();
restore();
X t622=___rewrite_3alog(t616); // REF: rewrite:log
t596=top();
S_DATA(t596)[1]=t622;
t596=restore();
restore();
x=t596; // REF: rewrite:rewrite1
goto loop; // tail call: rewrite:rewrite1
X t623;
t571=t623;
}else{
save(x); // COND
X t625=select1(TO_N(1),x); // SELECT (simple)
X t626=F;
save(t625); // PCONS
if(!IS_S(t625)||S_LENGTH(t625)!=4) goto t627;
t625=S_DATA(top())[0];
X t629=lf[320]; // CONST
save(t629);
X t630=___id(t625); // REF: id
t629=restore();
X t628=(t629==t630)||eq1(t629,t630)?T:F; // EQ
if(t628==F) goto t627;
// skipped
t628=S_DATA(top())[2];
X t631=F;
save(t628); // PCONS
if(!IS_S(t628)||S_LENGTH(t628)!=3) goto t632;
t628=S_DATA(top())[0];
X t634=lf[293]; // CONST
save(t634);
X t635=___id(t628); // REF: id
t634=restore();
X t633=(t634==t635)||eq1(t634,t635)?T:F; // EQ
if(t633==F) goto t632;
// skipped
t633=S_DATA(top())[2];
X t637=lf[294]; // CONST
save(t637);
X t638=___id(t633); // REF: id
t637=restore();
X t636=(t637==t638)||eq1(t637,t638)?T:F; // EQ
if(t636==F) goto t632;
t631=T;
t632:
restore();
if(t631==F) goto t627;
t631=S_DATA(top())[3];
X t639=F;
save(t631); // PCONSL
if(!IS_S(t631)||S_LENGTH(t631)<3) goto t640;
t631=S_DATA(top())[0];
X t642=lf[320]; // CONST
save(t642);
X t643=___id(t631); // REF: id
t642=restore();
X t641=(t642==t643)||eq1(t642,t643)?T:F; // EQ
if(t641==F) goto t640;
// skipped
t641=S_DATA(top())[2];
X t644=F;
save(t641); // PCONS
if(!IS_S(t641)||S_LENGTH(t641)!=4) goto t645;
t641=S_DATA(top())[0];
X t647=lf[326]; // CONST
save(t647);
X t648=___id(t641); // REF: id
t647=restore();
X t646=(t647==t648)||eq1(t647,t648)?T:F; // EQ
if(t646==F) goto t645;
// skipped
t646=S_DATA(top())[2];
X t649=F;
save(t646); // PCONSL
if(!IS_S(t646)||S_LENGTH(t646)<1) goto t650;
t646=S_DATA(top())[0];
X t652=lf[324]; // CONST
save(t652);
X t653=___id(t646); // REF: id
t652=restore();
X t651=(t652==t653)||eq1(t652,t653)?T:F; // EQ
if(t651==F) goto t650;
t649=T;
t650:
restore();
if(t649==F) goto t645;
// skipped
t644=T;
t645:
restore();
if(t644==F) goto t640;
t639=T;
t640:
restore();
if(t639==F) goto t627;
t626=T;
t627:
restore();
X t624;
x=restore();
if(t626!=F){
save(x);
X t654=allocate(2); // CONS
save(t654);
x=pick(2);
save(x);
X t655=allocate(4); // CONS
save(t655);
x=pick(2);
X t656=lf[320]; // CONST
t655=top();
S_DATA(t655)[0]=t656;
t656=pick(2);
X t658=TO_N(2); // ICONST
save(t658);
X t659=select1(TO_N(1),t656); // SELECT (simple)
t658=restore();
X t657=select1(t658,t659); // SELECT
t655=top();
S_DATA(t655)[1]=t657;
t657=pick(2);
save(t657);
X t660=allocate(2); // CONS
save(t660);
t657=pick(2);
X t661=lf[293]; // CONST
t660=top();
S_DATA(t660)[0]=t661;
t661=pick(2);
X t663=TO_N(3); // ICONST
save(t663);
X t665=TO_N(3); // ICONST
save(t665);
X t667=TO_N(4); // ICONST
save(t667);
X t668=select1(TO_N(1),t661); // SELECT (simple)
t667=restore();
X t666=select1(t667,t668); // SELECT
t665=restore();
X t664=select1(t665,t666); // SELECT
t663=restore();
X t662=select1(t663,t664); // SELECT
X t669=___tl(t662); // REF: tl
t660=top();
S_DATA(t660)[1]=t669;
t660=restore();
restore();
X t670=___al(t660); // REF: al
t655=top();
S_DATA(t655)[2]=t670;
t670=pick(2);
X t672=TO_N(4); // ICONST
save(t672);
X t673=select1(TO_N(1),t670); // SELECT (simple)
t672=restore();
X t671=select1(t672,t673); // SELECT
save(t671);
X t674=allocate(4); // CONS
save(t674);
t671=pick(2);
X t675=lf[320]; // CONST
t674=top();
S_DATA(t674)[0]=t675;
t675=pick(2);
X t676=select1(TO_N(2),t675); // SELECT (simple)
t674=top();
S_DATA(t674)[1]=t676;
t676=pick(2);
X t678=TO_N(4); // ICONST
save(t678);
X t679=select1(TO_N(3),t676); // SELECT (simple)
t678=restore();
X t677=select1(t678,t679); // SELECT
t674=top();
S_DATA(t674)[2]=t677;
t677=pick(2);
X t681=TO_N(4); // ICONST
save(t681);
X t682=select1(TO_N(4),t677); // SELECT (simple)
t681=restore();
X t680=select1(t681,t682); // SELECT
t674=top();
S_DATA(t674)[3]=t680;
t674=restore();
restore();
t655=top();
S_DATA(t655)[3]=t674;
t655=restore();
restore();
t654=top();
S_DATA(t654)[0]=t655;
t655=pick(2);
save(t655);
X t683=allocate(3); // CONS
save(t683);
t655=pick(2);
X t684=lf[137]; // CONST
t683=top();
S_DATA(t683)[0]=t684;
t684=pick(2);
X t686=TO_N(2); // ICONST
save(t686);
X t687=select1(TO_N(1),t684); // SELECT (simple)
t686=restore();
X t685=select1(t686,t687); // SELECT
t683=top();
S_DATA(t683)[1]=t685;
t685=pick(2);
X t688=select1(TO_N(2),t685); // SELECT (simple)
t683=top();
S_DATA(t683)[2]=t688;
t683=restore();
restore();
X t689=___rewrite_3alog(t683); // REF: rewrite:log
t654=top();
S_DATA(t654)[1]=t689;
t654=restore();
restore();
x=t654; // REF: rewrite:rewrite1
goto loop; // tail call: rewrite:rewrite1
X t690;
t624=t690;
}else{
save(x); // COND
X t692=select1(TO_N(1),x); // SELECT (simple)
X t693=F;
save(t692); // PCONS
if(!IS_S(t692)||S_LENGTH(t692)!=4) goto t694;
t692=S_DATA(top())[0];
X t696=lf[320]; // CONST
save(t696);
X t697=___id(t692); // REF: id
t696=restore();
X t695=(t696==t697)||eq1(t696,t697)?T:F; // EQ
if(t695==F) goto t694;
// skipped
t695=S_DATA(top())[2];
X t698=F;
save(t695); // PCONS
if(!IS_S(t695)||S_LENGTH(t695)!=3) goto t699;
t695=S_DATA(top())[0];
X t701=lf[293]; // CONST
save(t701);
X t702=___id(t695); // REF: id
t701=restore();
X t700=(t701==t702)||eq1(t701,t702)?T:F; // EQ
if(t700==F) goto t699;
// skipped
t700=S_DATA(top())[2];
X t704=lf[139]; // CONST
save(t704);
X t705=___id(t700); // REF: id
t704=restore();
X t703=(t704==t705)||eq1(t704,t705)?T:F; // EQ
if(t703==F) goto t699;
t698=T;
t699:
restore();
if(t698==F) goto t694;
t698=S_DATA(top())[3];
X t706=F;
save(t698); // PCONS
if(!IS_S(t698)||S_LENGTH(t698)!=3) goto t707;
t698=S_DATA(top())[0];
X t709=lf[296]; // CONST
save(t709);
X t710=___id(t698); // REF: id
t709=restore();
X t708=(t709==t710)||eq1(t709,t710)?T:F; // EQ
if(t708==F) goto t707;
// skipped
t708=S_DATA(top())[2];
X t711=F;
save(t708); // PCONSL
if(!IS_S(t708)||S_LENGTH(t708)<1) goto t712;
t708=S_DATA(top())[0];
X t714=lf[296]; // CONST
save(t714);
X t715=___id(t708); // REF: id
t714=restore();
X t713=(t714==t715)||eq1(t714,t715)?T:F; // EQ
if(t713==F) goto t712;
t711=T;
t712:
restore();
if(t711==F) goto t707;
t706=T;
t707:
restore();
if(t706==F) goto t694;
t693=T;
t694:
restore();
X t691;
x=restore();
if(t693!=F){
save(x);
X t716=allocate(2); // CONS
save(t716);
x=pick(2);
save(x);
X t717=allocate(4); // CONS
save(t717);
x=pick(2);
X t718=lf[320]; // CONST
t717=top();
S_DATA(t717)[0]=t718;
t718=pick(2);
X t720=TO_N(2); // ICONST
save(t720);
X t721=select1(TO_N(1),t718); // SELECT (simple)
t720=restore();
X t719=select1(t720,t721); // SELECT
t717=top();
S_DATA(t717)[1]=t719;
t719=pick(2);
X t723=TO_N(3); // ICONST
save(t723);
X t725=TO_N(4); // ICONST
save(t725);
X t726=select1(TO_N(1),t719); // SELECT (simple)
t725=restore();
X t724=select1(t725,t726); // SELECT
t723=restore();
X t722=select1(t723,t724); // SELECT
t717=top();
S_DATA(t717)[2]=t722;
t722=pick(2);
X t728=TO_N(3); // ICONST
save(t728);
X t729=select1(TO_N(1),t722); // SELECT (simple)
t728=restore();
X t727=select1(t728,t729); // SELECT
t717=top();
S_DATA(t717)[3]=t727;
t717=restore();
restore();
t716=top();
S_DATA(t716)[0]=t717;
t717=pick(2);
save(t717);
X t730=allocate(3); // CONS
save(t730);
t717=pick(2);
X t731=lf[138]; // CONST
t730=top();
S_DATA(t730)[0]=t731;
t731=pick(2);
X t733=TO_N(2); // ICONST
save(t733);
X t734=select1(TO_N(1),t731); // SELECT (simple)
t733=restore();
X t732=select1(t733,t734); // SELECT
t730=top();
S_DATA(t730)[1]=t732;
t732=pick(2);
X t735=select1(TO_N(2),t732); // SELECT (simple)
t730=top();
S_DATA(t730)[2]=t735;
t730=restore();
restore();
X t736=___rewrite_3alog(t730); // REF: rewrite:log
t716=top();
S_DATA(t716)[1]=t736;
t716=restore();
restore();
x=t716; // REF: rewrite:rewrite1
goto loop; // tail call: rewrite:rewrite1
X t737;
t691=t737;
}else{
save(x); // COND
X t739=select1(TO_N(1),x); // SELECT (simple)
X t740=F;
save(t739); // PCONS
if(!IS_S(t739)||S_LENGTH(t739)!=4) goto t741;
t739=S_DATA(top())[0];
X t743=lf[320]; // CONST
save(t743);
X t744=___id(t739); // REF: id
t743=restore();
X t742=(t743==t744)||eq1(t743,t744)?T:F; // EQ
if(t742==F) goto t741;
// skipped
t742=S_DATA(top())[2];
X t745=F;
save(t742); // PCONS
if(!IS_S(t742)||S_LENGTH(t742)!=3) goto t746;
t742=S_DATA(top())[0];
X t748=lf[293]; // CONST
save(t748);
X t749=___id(t742); // REF: id
t748=restore();
X t747=(t748==t749)||eq1(t748,t749)?T:F; // EQ
if(t747==F) goto t746;
// skipped
t747=S_DATA(top())[2];
X t751=lf[139]; // CONST
save(t751);
X t752=___id(t747); // REF: id
t751=restore();
X t750=(t751==t752)||eq1(t751,t752)?T:F; // EQ
if(t750==F) goto t746;
t745=T;
t746:
restore();
if(t745==F) goto t741;
t745=S_DATA(top())[3];
X t753=F;
save(t745); // PCONSL
if(!IS_S(t745)||S_LENGTH(t745)<3) goto t754;
t745=S_DATA(top())[0];
X t756=lf[320]; // CONST
save(t756);
X t757=___id(t745); // REF: id
t756=restore();
X t755=(t756==t757)||eq1(t756,t757)?T:F; // EQ
if(t755==F) goto t754;
// skipped
t755=S_DATA(top())[2];
X t758=F;
save(t755); // PCONS
if(!IS_S(t755)||S_LENGTH(t755)!=3) goto t759;
t755=S_DATA(top())[0];
X t761=lf[296]; // CONST
save(t761);
X t762=___id(t755); // REF: id
t761=restore();
X t760=(t761==t762)||eq1(t761,t762)?T:F; // EQ
if(t760==F) goto t759;
// skipped
t760=S_DATA(top())[2];
X t763=F;
save(t760); // PCONSL
if(!IS_S(t760)||S_LENGTH(t760)<1) goto t764;
t760=S_DATA(top())[0];
X t766=lf[296]; // CONST
save(t766);
X t767=___id(t760); // REF: id
t766=restore();
X t765=(t766==t767)||eq1(t766,t767)?T:F; // EQ
if(t765==F) goto t764;
t763=T;
t764:
restore();
if(t763==F) goto t759;
t758=T;
t759:
restore();
if(t758==F) goto t754;
t753=T;
t754:
restore();
if(t753==F) goto t741;
t740=T;
t741:
restore();
X t738;
x=restore();
if(t740!=F){
save(x);
X t768=allocate(2); // CONS
save(t768);
x=pick(2);
save(x);
X t769=allocate(4); // CONS
save(t769);
x=pick(2);
X t770=lf[320]; // CONST
t769=top();
S_DATA(t769)[0]=t770;
t770=pick(2);
X t772=TO_N(2); // ICONST
save(t772);
X t773=select1(TO_N(1),t770); // SELECT (simple)
t772=restore();
X t771=select1(t772,t773); // SELECT
t769=top();
S_DATA(t769)[1]=t771;
t771=pick(2);
X t775=TO_N(3); // ICONST
save(t775);
X t777=TO_N(3); // ICONST
save(t777);
X t779=TO_N(4); // ICONST
save(t779);
X t780=select1(TO_N(1),t771); // SELECT (simple)
t779=restore();
X t778=select1(t779,t780); // SELECT
t777=restore();
X t776=select1(t777,t778); // SELECT
t775=restore();
X t774=select1(t775,t776); // SELECT
t769=top();
S_DATA(t769)[2]=t774;
t774=pick(2);
save(t774);
X t781=allocate(4); // CONS
save(t781);
t774=pick(2);
X t782=lf[320]; // CONST
t781=top();
S_DATA(t781)[0]=t782;
t782=pick(2);
X t784=TO_N(2); // ICONST
save(t784);
X t786=TO_N(4); // ICONST
save(t786);
X t787=select1(TO_N(1),t782); // SELECT (simple)
t786=restore();
X t785=select1(t786,t787); // SELECT
t784=restore();
X t783=select1(t784,t785); // SELECT
t781=top();
S_DATA(t781)[1]=t783;
t783=pick(2);
X t789=TO_N(3); // ICONST
save(t789);
X t790=select1(TO_N(1),t783); // SELECT (simple)
t789=restore();
X t788=select1(t789,t790); // SELECT
t781=top();
S_DATA(t781)[2]=t788;
t788=pick(2);
X t792=TO_N(4); // ICONST
save(t792);
X t794=TO_N(4); // ICONST
save(t794);
X t795=select1(TO_N(1),t788); // SELECT (simple)
t794=restore();
X t793=select1(t794,t795); // SELECT
t792=restore();
X t791=select1(t792,t793); // SELECT
t781=top();
S_DATA(t781)[3]=t791;
t781=restore();
restore();
t769=top();
S_DATA(t769)[3]=t781;
t769=restore();
restore();
t768=top();
S_DATA(t768)[0]=t769;
t769=pick(2);
save(t769);
X t796=allocate(3); // CONS
save(t796);
t769=pick(2);
X t797=lf[140]; // CONST
t796=top();
S_DATA(t796)[0]=t797;
t797=pick(2);
X t799=TO_N(2); // ICONST
save(t799);
X t800=select1(TO_N(1),t797); // SELECT (simple)
t799=restore();
X t798=select1(t799,t800); // SELECT
t796=top();
S_DATA(t796)[1]=t798;
t798=pick(2);
X t801=select1(TO_N(2),t798); // SELECT (simple)
t796=top();
S_DATA(t796)[2]=t801;
t796=restore();
restore();
X t802=___rewrite_3alog(t796); // REF: rewrite:log
t768=top();
S_DATA(t768)[1]=t802;
t768=restore();
restore();
x=t768; // REF: rewrite:rewrite1
goto loop; // tail call: rewrite:rewrite1
X t803;
t738=t803;
}else{
save(x); // COND
X t805=select1(TO_N(1),x); // SELECT (simple)
X t806=F;
save(t805); // PCONS
if(!IS_S(t805)||S_LENGTH(t805)!=3) goto t807;
t805=S_DATA(top())[0];
X t809=lf[296]; // CONST
save(t809);
X t810=___id(t805); // REF: id
t809=restore();
X t808=(t809==t810)||eq1(t809,t810)?T:F; // EQ
if(t808==F) goto t807;
// skipped
t808=S_DATA(top())[2];
X t811=F;
save(t808); // PCONS
if(!IS_S(t808)||S_LENGTH(t808)!=3) goto t812;
t808=S_DATA(top())[0];
X t814=lf[293]; // CONST
save(t814);
X t815=___id(t808); // REF: id
t814=restore();
X t813=(t814==t815)||eq1(t814,t815)?T:F; // EQ
if(t813==F) goto t812;
// skipped
t813=S_DATA(top())[2];
X t817=lf[282]; // CONST
save(t817);
X t818=___id(t813); // REF: id
t817=restore();
X t816=(t817==t818)||eq1(t817,t818)?T:F; // EQ
if(t816==F) goto t812;
t811=T;
t812:
restore();
if(t811==F) goto t807;
t806=T;
t807:
restore();
X t804;
x=restore();
if(t806!=F){
save(x);
X t819=allocate(2); // CONS
save(t819);
x=pick(2);
X t821=TO_N(3); // ICONST
save(t821);
X t822=select1(TO_N(1),x); // SELECT (simple)
t821=restore();
X t820=select1(t821,t822); // SELECT
t819=top();
S_DATA(t819)[0]=t820;
t820=pick(2);
save(t820);
X t823=allocate(3); // CONS
save(t823);
t820=pick(2);
X t824=lf[141]; // CONST
t823=top();
S_DATA(t823)[0]=t824;
t824=pick(2);
X t826=TO_N(2); // ICONST
save(t826);
X t827=select1(TO_N(1),t824); // SELECT (simple)
t826=restore();
X t825=select1(t826,t827); // SELECT
t823=top();
S_DATA(t823)[1]=t825;
t825=pick(2);
X t828=select1(TO_N(2),t825); // SELECT (simple)
t823=top();
S_DATA(t823)[2]=t828;
t823=restore();
restore();
X t829=___rewrite_3alog(t823); // REF: rewrite:log
t819=top();
S_DATA(t819)[1]=t829;
t819=restore();
restore();
x=t819; // REF: rewrite:rewrite1
goto loop; // tail call: rewrite:rewrite1
X t830;
t804=t830;
}else{
save(x); // COND
X t832=select1(TO_N(1),x); // SELECT (simple)
X t833=F;
save(t832); // PCONS
if(!IS_S(t832)||S_LENGTH(t832)!=4) goto t834;
t832=S_DATA(top())[0];
X t836=lf[320]; // CONST
save(t836);
X t837=___id(t832); // REF: id
t836=restore();
X t835=(t836==t837)||eq1(t836,t837)?T:F; // EQ
if(t835==F) goto t834;
// skipped
t835=S_DATA(top())[2];
X t838=F;
save(t835); // PCONSL
if(!IS_S(t835)||S_LENGTH(t835)<1) goto t839;
t835=S_DATA(top())[0];
X t841=lf[302]; // CONST
save(t841);
X t842=___id(t835); // REF: id
t841=restore();
X t840=(t841==t842)||eq1(t841,t842)?T:F; // EQ
if(t840==F) goto t839;
t838=T;
t839:
restore();
if(t838==F) goto t834;
t838=S_DATA(top())[3];
X t843=F;
save(t838); // PCONSL
if(!IS_S(t838)||S_LENGTH(t838)<1) goto t844;
t838=S_DATA(top())[0];
X t846=lf[296]; // CONST
save(t846);
X t847=___id(t838); // REF: id
t846=restore();
X t845=(t846==t847)||eq1(t846,t847)?T:F; // EQ
if(t845==F) goto t844;
t843=T;
t844:
restore();
if(t843==F) goto t834;
t833=T;
t834:
restore();
X t831;
x=restore();
if(t833!=F){
save(x);
X t848=allocate(2); // CONS
save(t848);
x=pick(2);
save(x);
X t849=allocate(4); // CONS
save(t849);
x=pick(2);
X t850=lf[302]; // CONST
t849=top();
S_DATA(t849)[0]=t850;
t850=pick(2);
X t852=TO_N(2); // ICONST
save(t852);
X t854=TO_N(3); // ICONST
save(t854);
X t855=select1(TO_N(1),t850); // SELECT (simple)
t854=restore();
X t853=select1(t854,t855); // SELECT
t852=restore();
X t851=select1(t852,t853); // SELECT
t849=top();
S_DATA(t849)[1]=t851;
t851=pick(2);
X t857=TO_N(3); // ICONST
save(t857);
X t859=TO_N(3); // ICONST
save(t859);
X t860=select1(TO_N(1),t851); // SELECT (simple)
t859=restore();
X t858=select1(t859,t860); // SELECT
t857=restore();
X t856=select1(t857,t858); // SELECT
t849=top();
S_DATA(t849)[2]=t856;
t856=pick(2);
save(t856);
X t861=allocate(4); // CONS
save(t861);
t856=pick(2);
X t862=lf[320]; // CONST
t861=top();
S_DATA(t861)[0]=t862;
t862=pick(2);
X t864=TO_N(2); // ICONST
save(t864);
X t866=TO_N(3); // ICONST
save(t866);
X t867=select1(TO_N(1),t862); // SELECT (simple)
t866=restore();
X t865=select1(t866,t867); // SELECT
t864=restore();
X t863=select1(t864,t865); // SELECT
t861=top();
S_DATA(t861)[1]=t863;
t863=pick(2);
X t869=TO_N(4); // ICONST
save(t869);
X t871=TO_N(3); // ICONST
save(t871);
X t872=select1(TO_N(1),t863); // SELECT (simple)
t871=restore();
X t870=select1(t871,t872); // SELECT
t869=restore();
X t868=select1(t869,t870); // SELECT
t861=top();
S_DATA(t861)[2]=t868;
t868=pick(2);
save(t868);
X t873=allocate(4); // CONS
save(t873);
t868=pick(2);
X t874=lf[326]; // CONST
t873=top();
S_DATA(t873)[0]=t874;
t874=pick(2);
X t876=TO_N(2); // ICONST
save(t876);
X t877=select1(TO_N(1),t874); // SELECT (simple)
t876=restore();
X t875=select1(t876,t877); // SELECT
t873=top();
S_DATA(t873)[1]=t875;
t875=pick(2);
save(t875);
X t878=allocate(4); // CONS
save(t878);
t875=pick(2);
X t879=lf[320]; // CONST
t878=top();
S_DATA(t878)[0]=t879;
t879=pick(2);
X t881=TO_N(2); // ICONST
save(t881);
X t882=select1(TO_N(1),t879); // SELECT (simple)
t881=restore();
X t880=select1(t881,t882); // SELECT
t878=top();
S_DATA(t878)[1]=t880;
t880=pick(2);
X t884=TO_N(3); // ICONST
save(t884);
X t886=TO_N(4); // ICONST
save(t886);
X t887=select1(TO_N(1),t880); // SELECT (simple)
t886=restore();
X t885=select1(t886,t887); // SELECT
t884=restore();
X t883=select1(t884,t885); // SELECT
t878=top();
S_DATA(t878)[2]=t883;
t883=pick(2);
save(t883);
X t888=allocate(4); // CONS
save(t888);
t883=pick(2);
X t889=lf[319]; // CONST
t888=top();
S_DATA(t888)[0]=t889;
t889=pick(2);
X t891=TO_N(2); // ICONST
save(t891);
X t892=select1(TO_N(1),t889); // SELECT (simple)
t891=restore();
X t890=select1(t891,t892); // SELECT
t888=top();
S_DATA(t888)[1]=t890;
t890=pick(2);
save(t890);
X t893=allocate(3); // CONS
save(t893);
t890=pick(2);
X t894=lf[291]; // CONST
t893=top();
S_DATA(t893)[0]=t894;
t894=pick(2);
X t896=TO_N(2); // ICONST
save(t896);
X t897=select1(TO_N(1),t894); // SELECT (simple)
t896=restore();
X t895=select1(t896,t897); // SELECT
t893=top();
S_DATA(t893)[1]=t895;
t895=pick(2);
X t898=TO_N(1); // ICONST
t893=top();
S_DATA(t893)[2]=t898;
t893=restore();
restore();
t888=top();
S_DATA(t888)[2]=t893;
t893=pick(2);
save(t893);
X t899=allocate(3); // CONS
save(t899);
t893=pick(2);
X t900=lf[293]; // CONST
t899=top();
S_DATA(t899)[0]=t900;
t900=pick(2);
X t902=TO_N(2); // ICONST
save(t902);
X t903=select1(TO_N(1),t900); // SELECT (simple)
t902=restore();
X t901=select1(t902,t903); // SELECT
t899=top();
S_DATA(t899)[1]=t901;
t901=pick(2);
X t904=lf[282]; // CONST
t899=top();
S_DATA(t899)[2]=t904;
t899=restore();
restore();
t888=top();
S_DATA(t888)[3]=t899;
t888=restore();
restore();
t878=top();
S_DATA(t878)[3]=t888;
t878=restore();
restore();
t873=top();
S_DATA(t873)[2]=t878;
t878=pick(2);
save(t878);
X t905=allocate(4); // CONS
save(t905);
t878=pick(2);
X t906=lf[319]; // CONST
t905=top();
S_DATA(t905)[0]=t906;
t906=pick(2);
X t908=TO_N(2); // ICONST
save(t908);
X t909=select1(TO_N(1),t906); // SELECT (simple)
t908=restore();
X t907=select1(t908,t909); // SELECT
t905=top();
S_DATA(t905)[1]=t907;
t907=pick(2);
save(t907);
X t910=allocate(3); // CONS
save(t910);
t907=pick(2);
X t911=lf[291]; // CONST
t910=top();
S_DATA(t910)[0]=t911;
t911=pick(2);
X t913=TO_N(2); // ICONST
save(t913);
X t914=select1(TO_N(1),t911); // SELECT (simple)
t913=restore();
X t912=select1(t913,t914); // SELECT
t910=top();
S_DATA(t910)[1]=t912;
t912=pick(2);
X t915=TO_N(2); // ICONST
t910=top();
S_DATA(t910)[2]=t915;
t910=restore();
restore();
t905=top();
S_DATA(t905)[2]=t910;
t910=pick(2);
save(t910);
X t916=allocate(3); // CONS
save(t916);
t910=pick(2);
X t917=lf[293]; // CONST
t916=top();
S_DATA(t916)[0]=t917;
t917=pick(2);
X t919=TO_N(2); // ICONST
save(t919);
X t920=select1(TO_N(1),t917); // SELECT (simple)
t919=restore();
X t918=select1(t919,t920); // SELECT
t916=top();
S_DATA(t916)[1]=t918;
t918=pick(2);
X t921=lf[282]; // CONST
t916=top();
S_DATA(t916)[2]=t921;
t916=restore();
restore();
t905=top();
S_DATA(t905)[3]=t916;
t905=restore();
restore();
t873=top();
S_DATA(t873)[3]=t905;
t873=restore();
restore();
t861=top();
S_DATA(t861)[3]=t873;
t861=restore();
restore();
t849=top();
S_DATA(t849)[3]=t861;
t849=restore();
restore();
t848=top();
S_DATA(t848)[0]=t849;
t849=pick(2);
save(t849);
X t922=allocate(3); // CONS
save(t922);
t849=pick(2);
X t923=lf[142]; // CONST
t922=top();
S_DATA(t922)[0]=t923;
t923=pick(2);
X t925=TO_N(2); // ICONST
save(t925);
X t926=select1(TO_N(1),t923); // SELECT (simple)
t925=restore();
X t924=select1(t925,t926); // SELECT
t922=top();
S_DATA(t922)[1]=t924;
t924=pick(2);
X t927=select1(TO_N(2),t924); // SELECT (simple)
t922=top();
S_DATA(t922)[2]=t927;
t922=restore();
restore();
X t928=___rewrite_3alog(t922); // REF: rewrite:log
t848=top();
S_DATA(t848)[1]=t928;
t848=restore();
restore();
x=t848; // REF: rewrite:rewrite1
goto loop; // tail call: rewrite:rewrite1
X t929;
t831=t929;
}else{
save(x); // COND
X t931=select1(TO_N(1),x); // SELECT (simple)
X t932=F;
save(t931); // PCONS
if(!IS_S(t931)||S_LENGTH(t931)!=4) goto t933;
t931=S_DATA(top())[0];
X t935=lf[320]; // CONST
save(t935);
X t936=___id(t931); // REF: id
t935=restore();
X t934=(t935==t936)||eq1(t935,t936)?T:F; // EQ
if(t934==F) goto t933;
// skipped
t934=S_DATA(top())[2];
X t937=F;
save(t934); // PCONSL
if(!IS_S(t934)||S_LENGTH(t934)<1) goto t938;
t934=S_DATA(top())[0];
X t940=lf[302]; // CONST
save(t940);
X t941=___id(t934); // REF: id
t940=restore();
X t939=(t940==t941)||eq1(t940,t941)?T:F; // EQ
if(t939==F) goto t938;
t937=T;
t938:
restore();
if(t937==F) goto t933;
t937=S_DATA(top())[3];
X t942=F;
save(t937); // PCONS
if(!IS_S(t937)||S_LENGTH(t937)!=4) goto t943;
t937=S_DATA(top())[0];
X t945=lf[320]; // CONST
save(t945);
X t946=___id(t937); // REF: id
t945=restore();
X t944=(t945==t946)||eq1(t945,t946)?T:F; // EQ
if(t944==F) goto t943;
// skipped
t944=S_DATA(top())[2];
X t947=F;
save(t944); // PCONSL
if(!IS_S(t944)||S_LENGTH(t944)<1) goto t948;
t944=S_DATA(top())[0];
X t950=lf[296]; // CONST
save(t950);
X t951=___id(t944); // REF: id
t950=restore();
X t949=(t950==t951)||eq1(t950,t951)?T:F; // EQ
if(t949==F) goto t948;
t947=T;
t948:
restore();
if(t947==F) goto t943;
// skipped
t942=T;
t943:
restore();
if(t942==F) goto t933;
t932=T;
t933:
restore();
X t930;
x=restore();
if(t932!=F){
save(x);
X t952=allocate(2); // CONS
save(t952);
x=pick(2);
save(x);
X t953=allocate(4); // CONS
save(t953);
x=pick(2);
X t954=lf[320]; // CONST
t953=top();
S_DATA(t953)[0]=t954;
t954=pick(2);
X t956=TO_N(2); // ICONST
save(t956);
X t958=TO_N(4); // ICONST
save(t958);
X t959=select1(TO_N(1),t954); // SELECT (simple)
t958=restore();
X t957=select1(t958,t959); // SELECT
t956=restore();
X t955=select1(t956,t957); // SELECT
t953=top();
S_DATA(t953)[1]=t955;
t955=pick(2);
save(t955);
X t960=allocate(4); // CONS
save(t960);
t955=pick(2);
X t961=lf[302]; // CONST
t960=top();
S_DATA(t960)[0]=t961;
t961=pick(2);
X t963=TO_N(2); // ICONST
save(t963);
X t965=TO_N(3); // ICONST
save(t965);
X t966=select1(TO_N(1),t961); // SELECT (simple)
t965=restore();
X t964=select1(t965,t966); // SELECT
t963=restore();
X t962=select1(t963,t964); // SELECT
t960=top();
S_DATA(t960)[1]=t962;
t962=pick(2);
X t968=TO_N(3); // ICONST
save(t968);
X t970=TO_N(3); // ICONST
save(t970);
X t971=select1(TO_N(1),t962); // SELECT (simple)
t970=restore();
X t969=select1(t970,t971); // SELECT
t968=restore();
X t967=select1(t968,t969); // SELECT
t960=top();
S_DATA(t960)[2]=t967;
t967=pick(2);
save(t967);
X t972=allocate(4); // CONS
save(t972);
t967=pick(2);
X t973=lf[320]; // CONST
t972=top();
S_DATA(t972)[0]=t973;
t973=pick(2);
X t975=TO_N(2); // ICONST
save(t975);
X t977=TO_N(3); // ICONST
save(t977);
X t978=select1(TO_N(1),t973); // SELECT (simple)
t977=restore();
X t976=select1(t977,t978); // SELECT
t975=restore();
X t974=select1(t975,t976); // SELECT
t972=top();
S_DATA(t972)[1]=t974;
t974=pick(2);
X t980=TO_N(4); // ICONST
save(t980);
X t982=TO_N(3); // ICONST
save(t982);
X t983=select1(TO_N(1),t974); // SELECT (simple)
t982=restore();
X t981=select1(t982,t983); // SELECT
t980=restore();
X t979=select1(t980,t981); // SELECT
t972=top();
S_DATA(t972)[2]=t979;
t979=pick(2);
save(t979);
X t984=allocate(4); // CONS
save(t984);
t979=pick(2);
X t985=lf[326]; // CONST
t984=top();
S_DATA(t984)[0]=t985;
t985=pick(2);
X t987=TO_N(2); // ICONST
save(t987);
X t988=select1(TO_N(1),t985); // SELECT (simple)
t987=restore();
X t986=select1(t987,t988); // SELECT
t984=top();
S_DATA(t984)[1]=t986;
t986=pick(2);
save(t986);
X t989=allocate(4); // CONS
save(t989);
t986=pick(2);
X t990=lf[320]; // CONST
t989=top();
S_DATA(t989)[0]=t990;
t990=pick(2);
X t992=TO_N(2); // ICONST
save(t992);
X t993=select1(TO_N(1),t990); // SELECT (simple)
t992=restore();
X t991=select1(t992,t993); // SELECT
t989=top();
S_DATA(t989)[1]=t991;
t991=pick(2);
X t995=TO_N(3); // ICONST
save(t995);
X t997=TO_N(3); // ICONST
save(t997);
X t999=TO_N(4); // ICONST
save(t999);
X t1000=select1(TO_N(1),t991); // SELECT (simple)
t999=restore();
X t998=select1(t999,t1000); // SELECT
t997=restore();
X t996=select1(t997,t998); // SELECT
t995=restore();
X t994=select1(t995,t996); // SELECT
t989=top();
S_DATA(t989)[2]=t994;
t994=pick(2);
save(t994);
X t1001=allocate(4); // CONS
save(t1001);
t994=pick(2);
X t1002=lf[319]; // CONST
t1001=top();
S_DATA(t1001)[0]=t1002;
t1002=pick(2);
X t1004=TO_N(2); // ICONST
save(t1004);
X t1005=select1(TO_N(1),t1002); // SELECT (simple)
t1004=restore();
X t1003=select1(t1004,t1005); // SELECT
t1001=top();
S_DATA(t1001)[1]=t1003;
t1003=pick(2);
save(t1003);
X t1006=allocate(3); // CONS
save(t1006);
t1003=pick(2);
X t1007=lf[291]; // CONST
t1006=top();
S_DATA(t1006)[0]=t1007;
t1007=pick(2);
X t1009=TO_N(2); // ICONST
save(t1009);
X t1010=select1(TO_N(1),t1007); // SELECT (simple)
t1009=restore();
X t1008=select1(t1009,t1010); // SELECT
t1006=top();
S_DATA(t1006)[1]=t1008;
t1008=pick(2);
X t1011=TO_N(1); // ICONST
t1006=top();
S_DATA(t1006)[2]=t1011;
t1006=restore();
restore();
t1001=top();
S_DATA(t1001)[2]=t1006;
t1006=pick(2);
save(t1006);
X t1012=allocate(3); // CONS
save(t1012);
t1006=pick(2);
X t1013=lf[293]; // CONST
t1012=top();
S_DATA(t1012)[0]=t1013;
t1013=pick(2);
X t1015=TO_N(2); // ICONST
save(t1015);
X t1016=select1(TO_N(1),t1013); // SELECT (simple)
t1015=restore();
X t1014=select1(t1015,t1016); // SELECT
t1012=top();
S_DATA(t1012)[1]=t1014;
t1014=pick(2);
X t1017=lf[282]; // CONST
t1012=top();
S_DATA(t1012)[2]=t1017;
t1012=restore();
restore();
t1001=top();
S_DATA(t1001)[3]=t1012;
t1001=restore();
restore();
t989=top();
S_DATA(t989)[3]=t1001;
t989=restore();
restore();
t984=top();
S_DATA(t984)[2]=t989;
t989=pick(2);
save(t989);
X t1018=allocate(4); // CONS
save(t1018);
t989=pick(2);
X t1019=lf[319]; // CONST
t1018=top();
S_DATA(t1018)[0]=t1019;
t1019=pick(2);
X t1021=TO_N(2); // ICONST
save(t1021);
X t1022=select1(TO_N(1),t1019); // SELECT (simple)
t1021=restore();
X t1020=select1(t1021,t1022); // SELECT
t1018=top();
S_DATA(t1018)[1]=t1020;
t1020=pick(2);
save(t1020);
X t1023=allocate(3); // CONS
save(t1023);
t1020=pick(2);
X t1024=lf[291]; // CONST
t1023=top();
S_DATA(t1023)[0]=t1024;
t1024=pick(2);
X t1026=TO_N(2); // ICONST
save(t1026);
X t1027=select1(TO_N(1),t1024); // SELECT (simple)
t1026=restore();
X t1025=select1(t1026,t1027); // SELECT
t1023=top();
S_DATA(t1023)[1]=t1025;
t1025=pick(2);
X t1028=TO_N(2); // ICONST
t1023=top();
S_DATA(t1023)[2]=t1028;
t1023=restore();
restore();
t1018=top();
S_DATA(t1018)[2]=t1023;
t1023=pick(2);
save(t1023);
X t1029=allocate(3); // CONS
save(t1029);
t1023=pick(2);
X t1030=lf[293]; // CONST
t1029=top();
S_DATA(t1029)[0]=t1030;
t1030=pick(2);
X t1032=TO_N(2); // ICONST
save(t1032);
X t1033=select1(TO_N(1),t1030); // SELECT (simple)
t1032=restore();
X t1031=select1(t1032,t1033); // SELECT
t1029=top();
S_DATA(t1029)[1]=t1031;
t1031=pick(2);
X t1034=lf[282]; // CONST
t1029=top();
S_DATA(t1029)[2]=t1034;
t1029=restore();
restore();
t1018=top();
S_DATA(t1018)[3]=t1029;
t1018=restore();
restore();
t984=top();
S_DATA(t984)[3]=t1018;
t984=restore();
restore();
t972=top();
S_DATA(t972)[3]=t984;
t972=restore();
restore();
t960=top();
S_DATA(t960)[3]=t972;
t960=restore();
restore();
t953=top();
S_DATA(t953)[2]=t960;
t960=pick(2);
X t1036=TO_N(4); // ICONST
save(t1036);
X t1038=TO_N(4); // ICONST
save(t1038);
X t1039=select1(TO_N(1),t960); // SELECT (simple)
t1038=restore();
X t1037=select1(t1038,t1039); // SELECT
t1036=restore();
X t1035=select1(t1036,t1037); // SELECT
t953=top();
S_DATA(t953)[3]=t1035;
t953=restore();
restore();
t952=top();
S_DATA(t952)[0]=t953;
t953=pick(2);
save(t953);
X t1040=allocate(3); // CONS
save(t1040);
t953=pick(2);
X t1041=lf[143]; // CONST
t1040=top();
S_DATA(t1040)[0]=t1041;
t1041=pick(2);
X t1043=TO_N(2); // ICONST
save(t1043);
X t1044=select1(TO_N(1),t1041); // SELECT (simple)
t1043=restore();
X t1042=select1(t1043,t1044); // SELECT
t1040=top();
S_DATA(t1040)[1]=t1042;
t1042=pick(2);
X t1045=select1(TO_N(2),t1042); // SELECT (simple)
t1040=top();
S_DATA(t1040)[2]=t1045;
t1040=restore();
restore();
X t1046=___rewrite_3alog(t1040); // REF: rewrite:log
t952=top();
S_DATA(t952)[1]=t1046;
t952=restore();
restore();
x=t952; // REF: rewrite:rewrite1
goto loop; // tail call: rewrite:rewrite1
X t1047;
t930=t1047;
}else{
save(x); // COND
X t1049=select1(TO_N(1),x); // SELECT (simple)
X t1050=F;
save(t1049); // PCONS
if(!IS_S(t1049)||S_LENGTH(t1049)!=4) goto t1051;
t1049=S_DATA(top())[0];
X t1053=lf[320]; // CONST
save(t1053);
X t1054=___id(t1049); // REF: id
t1053=restore();
X t1052=(t1053==t1054)||eq1(t1053,t1054)?T:F; // EQ
if(t1052==F) goto t1051;
// skipped
t1052=S_DATA(top())[2];
X t1055=F;
save(t1052); // PCONSL
if(!IS_S(t1052)||S_LENGTH(t1052)<1) goto t1056;
t1052=S_DATA(top())[0];
X t1058=lf[296]; // CONST
save(t1058);
X t1059=___id(t1052); // REF: id
t1058=restore();
X t1057=(t1058==t1059)||eq1(t1058,t1059)?T:F; // EQ
if(t1057==F) goto t1056;
t1055=T;
t1056:
restore();
if(t1055==F) goto t1051;
t1055=S_DATA(top())[3];
X t1060=F;
save(t1055); // PCONSL
if(!IS_S(t1055)||S_LENGTH(t1055)<1) goto t1061;
t1055=S_DATA(top())[0];
X t1063=lf[326]; // CONST
save(t1063);
X t1064=___id(t1055); // REF: id
t1063=restore();
X t1062=(t1063==t1064)||eq1(t1063,t1064)?T:F; // EQ
if(t1062==F) goto t1061;
t1060=T;
t1061:
restore();
if(t1060==F) goto t1051;
t1050=T;
t1051:
restore();
X t1048;
x=restore();
if(t1050!=F){
save(x);
X t1065=allocate(2); // CONS
save(t1065);
x=pick(2);
X t1066=select1(TO_N(1),x); // SELECT (simple)
t1065=top();
S_DATA(t1065)[0]=t1066;
t1066=pick(2);
save(t1066);
X t1067=allocate(2); // CONS
save(t1067);
t1066=pick(2);
X t1069=TO_N(3); // ICONST
save(t1069);
X t1071=TO_N(3); // ICONST
save(t1071);
X t1072=select1(TO_N(1),t1066); // SELECT (simple)
t1071=restore();
X t1070=select1(t1071,t1072); // SELECT
t1069=restore();
X t1068=select1(t1069,t1070); // SELECT
t1067=top();
S_DATA(t1067)[0]=t1068;
t1068=pick(2);
X t1073=select1(TO_N(2),t1068); // SELECT (simple)
t1067=top();
S_DATA(t1067)[1]=t1073;
t1067=restore();
restore();
X t1074=___rewrite_3agen_5fdef(t1067); // REF: rewrite:gen_def
t1065=top();
S_DATA(t1065)[1]=t1074;
t1065=restore();
restore();
save(t1065);
X t1075=allocate(2); // CONS
save(t1075);
t1065=pick(2);
save(t1065);
X t1076=allocate(2); // CONS
save(t1076);
t1065=pick(2);
save(t1065);
X t1077=allocate(2); // CONS
save(t1077);
t1065=pick(2);
X t1078=lf[326]; // CONST
t1077=top();
S_DATA(t1077)[0]=t1078;
t1078=pick(2);
X t1080=TO_N(2); // ICONST
save(t1080);
X t1082=TO_N(4); // ICONST
save(t1082);
X t1083=select1(TO_N(1),t1078); // SELECT (simple)
t1082=restore();
X t1081=select1(t1082,t1083); // SELECT
t1080=restore();
X t1079=select1(t1080,t1081); // SELECT
t1077=top();
S_DATA(t1077)[1]=t1079;
t1077=restore();
restore();
t1076=top();
S_DATA(t1076)[0]=t1077;
t1077=pick(2);
save(t1077);
X t1084=allocate(2); // CONS
save(t1084);
t1077=pick(2);
X t1086=TO_N(4); // ICONST
save(t1086);
X t1087=select1(TO_N(1),t1077); // SELECT (simple)
t1086=restore();
X t1085=select1(t1086,t1087); // SELECT
X t1088=___tl(t1085); // REF: tl
X t1089=___tl(t1088); // REF: tl
t1084=top();
S_DATA(t1084)[0]=t1089;
t1089=pick(2);
save(t1089);
X t1090=allocate(2); // CONS
save(t1090);
t1089=pick(2);
X t1092=TO_N(2); // ICONST
save(t1092);
X t1093=select1(TO_N(1),t1089); // SELECT (simple)
t1092=restore();
X t1091=select1(t1092,t1093); // SELECT
t1090=top();
S_DATA(t1090)[0]=t1091;
t1091=pick(2);
save(t1091);
X t1094=allocate(3); // CONS
save(t1094);
t1091=pick(2);
X t1095=lf[293]; // CONST
t1094=top();
S_DATA(t1094)[0]=t1095;
t1095=pick(2);
X t1097=TO_N(2); // ICONST
save(t1097);
X t1099=TO_N(3); // ICONST
save(t1099);
X t1101=TO_N(3); // ICONST
save(t1101);
X t1102=select1(TO_N(1),t1095); // SELECT (simple)
t1101=restore();
X t1100=select1(t1101,t1102); // SELECT
t1099=restore();
X t1098=select1(t1099,t1100); // SELECT
t1097=restore();
X t1096=select1(t1097,t1098); // SELECT
t1094=top();
S_DATA(t1094)[1]=t1096;
t1096=pick(2);
X t1104=TO_N(1); // ICONST
save(t1104);
X t1105=select1(TO_N(2),t1096); // SELECT (simple)
t1104=restore();
X t1103=select1(t1104,t1105); // SELECT
t1094=top();
S_DATA(t1094)[2]=t1103;
t1094=restore();
restore();
t1090=top();
S_DATA(t1090)[1]=t1094;
t1090=restore();
restore();
t1084=top();
S_DATA(t1084)[1]=t1090;
t1084=restore();
restore();
X t1106=___dr(t1084); // REF: dr
int t1108; // ALPHA
check_S(t1106,"@");
int t1109=S_LENGTH(t1106);
save(t1106);
X t1107=allocate(t1109);
save(t1107);
for(t1108=0;t1108<t1109;++t1108){
X t1107=S_DATA(pick(2))[t1108];
save(t1107);
X t1110=allocate(4); // CONS
save(t1110);
t1107=pick(2);
X t1111=lf[320]; // CONST
t1110=top();
S_DATA(t1110)[0]=t1111;
t1111=pick(2);
X t1113=TO_N(1); // ICONST
save(t1113);
X t1114=select1(TO_N(2),t1111); // SELECT (simple)
t1113=restore();
X t1112=select1(t1113,t1114); // SELECT
t1110=top();
S_DATA(t1110)[1]=t1112;
t1112=pick(2);
X t1116=TO_N(2); // ICONST
save(t1116);
X t1117=select1(TO_N(2),t1112); // SELECT (simple)
t1116=restore();
X t1115=select1(t1116,t1117); // SELECT
t1110=top();
S_DATA(t1110)[2]=t1115;
t1115=pick(2);
X t1118=select1(TO_N(1),t1115); // SELECT (simple)
t1110=top();
S_DATA(t1110)[3]=t1118;
t1110=restore();
restore();
S_DATA(top())[t1108]=t1110;}
t1107=restore();
restore();
t1076=top();
S_DATA(t1076)[1]=t1107;
t1076=restore();
restore();
X t1119=___cat(t1076); // REF: cat
t1075=top();
S_DATA(t1075)[0]=t1119;
t1119=pick(2);
save(t1119);
X t1120=allocate(3); // CONS
save(t1120);
t1119=pick(2);
X t1121=lf[144]; // CONST
t1120=top();
S_DATA(t1120)[0]=t1121;
t1121=pick(2);
X t1123=TO_N(2); // ICONST
save(t1123);
X t1124=select1(TO_N(1),t1121); // SELECT (simple)
t1123=restore();
X t1122=select1(t1123,t1124); // SELECT
t1120=top();
S_DATA(t1120)[1]=t1122;
t1122=pick(2);
X t1126=TO_N(2); // ICONST
save(t1126);
X t1127=select1(TO_N(2),t1122); // SELECT (simple)
t1126=restore();
X t1125=select1(t1126,t1127); // SELECT
t1120=top();
S_DATA(t1120)[2]=t1125;
t1120=restore();
restore();
X t1128=___rewrite_3alog(t1120); // REF: rewrite:log
t1075=top();
S_DATA(t1075)[1]=t1128;
t1075=restore();
restore();
x=t1075; // REF: rewrite:rewrite1
goto loop; // tail call: rewrite:rewrite1
X t1129;
t1048=t1129;
}else{
save(x); // COND
X t1131=select1(TO_N(1),x); // SELECT (simple)
X t1132=F;
save(t1131); // PCONS
if(!IS_S(t1131)||S_LENGTH(t1131)!=4) goto t1133;
t1131=S_DATA(top())[0];
X t1135=lf[320]; // CONST
save(t1135);
X t1136=___id(t1131); // REF: id
t1135=restore();
X t1134=(t1135==t1136)||eq1(t1135,t1136)?T:F; // EQ
if(t1134==F) goto t1133;
// skipped
t1134=S_DATA(top())[2];
X t1137=F;
save(t1134); // PCONSL
if(!IS_S(t1134)||S_LENGTH(t1134)<1) goto t1138;
t1134=S_DATA(top())[0];
X t1140=lf[296]; // CONST
save(t1140);
X t1141=___id(t1134); // REF: id
t1140=restore();
X t1139=(t1140==t1141)||eq1(t1140,t1141)?T:F; // EQ
if(t1139==F) goto t1138;
t1137=T;
t1138:
restore();
if(t1137==F) goto t1133;
t1137=S_DATA(top())[3];
X t1142=F;
save(t1137); // PCONS
if(!IS_S(t1137)||S_LENGTH(t1137)!=4) goto t1143;
t1137=S_DATA(top())[0];
X t1145=lf[320]; // CONST
save(t1145);
X t1146=___id(t1137); // REF: id
t1145=restore();
X t1144=(t1145==t1146)||eq1(t1145,t1146)?T:F; // EQ
if(t1144==F) goto t1143;
// skipped
t1144=S_DATA(top())[2];
X t1147=F;
save(t1144); // PCONSL
if(!IS_S(t1144)||S_LENGTH(t1144)<1) goto t1148;
t1144=S_DATA(top())[0];
X t1150=lf[326]; // CONST
save(t1150);
X t1151=___id(t1144); // REF: id
t1150=restore();
X t1149=(t1150==t1151)||eq1(t1150,t1151)?T:F; // EQ
if(t1149==F) goto t1148;
t1147=T;
t1148:
restore();
if(t1147==F) goto t1143;
// skipped
t1142=T;
t1143:
restore();
if(t1142==F) goto t1133;
t1132=T;
t1133:
restore();
X t1130;
x=restore();
if(t1132!=F){
save(x);
X t1152=allocate(2); // CONS
save(t1152);
x=pick(2);
X t1153=select1(TO_N(1),x); // SELECT (simple)
t1152=top();
S_DATA(t1152)[0]=t1153;
t1153=pick(2);
save(t1153);
X t1154=allocate(2); // CONS
save(t1154);
t1153=pick(2);
X t1156=TO_N(3); // ICONST
save(t1156);
X t1158=TO_N(3); // ICONST
save(t1158);
X t1159=select1(TO_N(1),t1153); // SELECT (simple)
t1158=restore();
X t1157=select1(t1158,t1159); // SELECT
t1156=restore();
X t1155=select1(t1156,t1157); // SELECT
t1154=top();
S_DATA(t1154)[0]=t1155;
t1155=pick(2);
X t1160=select1(TO_N(2),t1155); // SELECT (simple)
t1154=top();
S_DATA(t1154)[1]=t1160;
t1154=restore();
restore();
X t1161=___rewrite_3agen_5fdef(t1154); // REF: rewrite:gen_def
t1152=top();
S_DATA(t1152)[1]=t1161;
t1152=restore();
restore();
save(t1152);
X t1162=allocate(2); // CONS
save(t1162);
t1152=pick(2);
save(t1152);
X t1163=allocate(2); // CONS
save(t1163);
t1152=pick(2);
save(t1152);
X t1164=allocate(2); // CONS
save(t1164);
t1152=pick(2);
X t1165=lf[326]; // CONST
t1164=top();
S_DATA(t1164)[0]=t1165;
t1165=pick(2);
X t1167=TO_N(2); // ICONST
save(t1167);
X t1169=TO_N(3); // ICONST
save(t1169);
X t1171=TO_N(4); // ICONST
save(t1171);
X t1172=select1(TO_N(1),t1165); // SELECT (simple)
t1171=restore();
X t1170=select1(t1171,t1172); // SELECT
t1169=restore();
X t1168=select1(t1169,t1170); // SELECT
t1167=restore();
X t1166=select1(t1167,t1168); // SELECT
t1164=top();
S_DATA(t1164)[1]=t1166;
t1164=restore();
restore();
t1163=top();
S_DATA(t1163)[0]=t1164;
t1164=pick(2);
save(t1164);
X t1173=allocate(2); // CONS
save(t1173);
t1164=pick(2);
X t1175=TO_N(3); // ICONST
save(t1175);
X t1177=TO_N(4); // ICONST
save(t1177);
X t1178=select1(TO_N(1),t1164); // SELECT (simple)
t1177=restore();
X t1176=select1(t1177,t1178); // SELECT
t1175=restore();
X t1174=select1(t1175,t1176); // SELECT
X t1179=___tl(t1174); // REF: tl
X t1180=___tl(t1179); // REF: tl
t1173=top();
S_DATA(t1173)[0]=t1180;
t1180=pick(2);
save(t1180);
X t1181=allocate(2); // CONS
save(t1181);
t1180=pick(2);
X t1183=TO_N(2); // ICONST
save(t1183);
X t1184=select1(TO_N(1),t1180); // SELECT (simple)
t1183=restore();
X t1182=select1(t1183,t1184); // SELECT
t1181=top();
S_DATA(t1181)[0]=t1182;
t1182=pick(2);
save(t1182);
X t1185=allocate(3); // CONS
save(t1185);
t1182=pick(2);
X t1186=lf[293]; // CONST
t1185=top();
S_DATA(t1185)[0]=t1186;
t1186=pick(2);
X t1188=TO_N(2); // ICONST
save(t1188);
X t1190=TO_N(3); // ICONST
save(t1190);
X t1192=TO_N(3); // ICONST
save(t1192);
X t1193=select1(TO_N(1),t1186); // SELECT (simple)
t1192=restore();
X t1191=select1(t1192,t1193); // SELECT
t1190=restore();
X t1189=select1(t1190,t1191); // SELECT
t1188=restore();
X t1187=select1(t1188,t1189); // SELECT
t1185=top();
S_DATA(t1185)[1]=t1187;
t1187=pick(2);
X t1195=TO_N(1); // ICONST
save(t1195);
X t1196=select1(TO_N(2),t1187); // SELECT (simple)
t1195=restore();
X t1194=select1(t1195,t1196); // SELECT
t1185=top();
S_DATA(t1185)[2]=t1194;
t1185=restore();
restore();
t1181=top();
S_DATA(t1181)[1]=t1185;
t1181=restore();
restore();
t1173=top();
S_DATA(t1173)[1]=t1181;
t1173=restore();
restore();
X t1197=___dr(t1173); // REF: dr
int t1199; // ALPHA
check_S(t1197,"@");
int t1200=S_LENGTH(t1197);
save(t1197);
X t1198=allocate(t1200);
save(t1198);
for(t1199=0;t1199<t1200;++t1199){
X t1198=S_DATA(pick(2))[t1199];
save(t1198);
X t1201=allocate(4); // CONS
save(t1201);
t1198=pick(2);
X t1202=lf[320]; // CONST
t1201=top();
S_DATA(t1201)[0]=t1202;
t1202=pick(2);
X t1204=TO_N(1); // ICONST
save(t1204);
X t1205=select1(TO_N(2),t1202); // SELECT (simple)
t1204=restore();
X t1203=select1(t1204,t1205); // SELECT
t1201=top();
S_DATA(t1201)[1]=t1203;
t1203=pick(2);
X t1207=TO_N(2); // ICONST
save(t1207);
X t1208=select1(TO_N(2),t1203); // SELECT (simple)
t1207=restore();
X t1206=select1(t1207,t1208); // SELECT
t1201=top();
S_DATA(t1201)[2]=t1206;
t1206=pick(2);
X t1209=select1(TO_N(1),t1206); // SELECT (simple)
t1201=top();
S_DATA(t1201)[3]=t1209;
t1201=restore();
restore();
S_DATA(top())[t1199]=t1201;}
t1198=restore();
restore();
t1163=top();
S_DATA(t1163)[1]=t1198;
t1163=restore();
restore();
X t1210=___cat(t1163); // REF: cat
t1162=top();
S_DATA(t1162)[0]=t1210;
t1210=pick(2);
save(t1210);
X t1211=allocate(3); // CONS
save(t1211);
t1210=pick(2);
X t1212=lf[145]; // CONST
t1211=top();
S_DATA(t1211)[0]=t1212;
t1212=pick(2);
X t1214=TO_N(2); // ICONST
save(t1214);
X t1215=select1(TO_N(1),t1212); // SELECT (simple)
t1214=restore();
X t1213=select1(t1214,t1215); // SELECT
t1211=top();
S_DATA(t1211)[1]=t1213;
t1213=pick(2);
X t1217=TO_N(2); // ICONST
save(t1217);
X t1218=select1(TO_N(2),t1213); // SELECT (simple)
t1217=restore();
X t1216=select1(t1217,t1218); // SELECT
t1211=top();
S_DATA(t1211)[2]=t1216;
t1211=restore();
restore();
X t1219=___rewrite_3alog(t1211); // REF: rewrite:log
t1162=top();
S_DATA(t1162)[1]=t1219;
t1162=restore();
restore();
x=t1162; // REF: rewrite:rewrite1
goto loop; // tail call: rewrite:rewrite1
X t1220;
t1130=t1220;
}else{
save(x); // COND
save(x); // COND
X t1223=select1(TO_N(1),x); // SELECT (simple)
X t1224=F;
save(t1223); // PCONSL
if(!IS_S(t1223)||S_LENGTH(t1223)<3) goto t1225;
t1223=S_DATA(top())[0];
X t1227=lf[326]; // CONST
save(t1227);
X t1228=___id(t1223); // REF: id
t1227=restore();
X t1226=(t1227==t1228)||eq1(t1227,t1228)?T:F; // EQ
if(t1226==F) goto t1225;
// skipped
// skipped
t1224=T;
t1225:
restore();
X t1222;
x=restore();
if(t1224!=F){
X t1229=select1(TO_N(1),x); // SELECT (simple)
X t1230=___tl(t1229); // REF: tl
X t1231=___tl(t1230); // REF: tl
int t1233; // ALPHA
check_S(t1231,"@");
int t1234=S_LENGTH(t1231);
save(t1231);
X t1232=allocate(t1234);
save(t1232);
for(t1233=0;t1233<t1234;++t1233){
X t1232=S_DATA(pick(2))[t1233];
X t1235=F;
save(t1232); // PCONSL
if(!IS_S(t1232)||S_LENGTH(t1232)<1) goto t1236;
t1232=S_DATA(top())[0];
X t1238=lf[302]; // CONST
save(t1238);
X t1239=___id(t1232); // REF: id
t1238=restore();
X t1237=(t1238==t1239)||eq1(t1238,t1239)?T:F; // EQ
if(t1237==F) goto t1236;
t1235=T;
t1236:
restore();
S_DATA(top())[t1233]=t1235;}
t1232=restore();
restore();
X t1240=___all(t1232); // REF: all
t1222=t1240;
}else{
X t1241=F; // ICONST
t1222=t1241;}
X t1221;
x=restore();
if(t1222!=F){
save(x);
X t1242=allocate(2); // CONS
save(t1242);
x=pick(2);
save(x);
X t1243=allocate(4); // CONS
save(t1243);
x=pick(2);
X t1244=lf[302]; // CONST
t1243=top();
S_DATA(t1243)[0]=t1244;
t1244=pick(2);
X t1246=TO_N(2); // ICONST
save(t1246);
X t1247=select1(TO_N(1),t1244); // SELECT (simple)
t1246=restore();
X t1245=select1(t1246,t1247); // SELECT
t1243=top();
S_DATA(t1243)[1]=t1245;
t1245=pick(2);
save(t1245);
X t1248=allocate(2); // CONS
save(t1248);
t1245=pick(2);
save(t1245);
X t1249=allocate(2); // CONS
save(t1249);
t1245=pick(2);
X t1250=lf[326]; // CONST
t1249=top();
S_DATA(t1249)[0]=t1250;
t1250=pick(2);
X t1252=TO_N(2); // ICONST
save(t1252);
X t1253=select1(TO_N(1),t1250); // SELECT (simple)
t1252=restore();
X t1251=select1(t1252,t1253); // SELECT
t1249=top();
S_DATA(t1249)[1]=t1251;
t1249=restore();
restore();
t1248=top();
S_DATA(t1248)[0]=t1249;
t1249=pick(2);
X t1254=select1(TO_N(1),t1249); // SELECT (simple)
X t1255=___tl(t1254); // REF: tl
X t1256=___tl(t1255); // REF: tl
int t1258; // ALPHA
check_S(t1256,"@");
int t1259=S_LENGTH(t1256);
save(t1256);
X t1257=allocate(t1259);
save(t1257);
for(t1258=0;t1258<t1259;++t1258){
X t1257=S_DATA(pick(2))[t1258];
X t1260=select1(TO_N(3),t1257); // SELECT (simple)
S_DATA(top())[t1258]=t1260;}
t1257=restore();
restore();
t1248=top();
S_DATA(t1248)[1]=t1257;
t1248=restore();
restore();
X t1261=___cat(t1248); // REF: cat
t1243=top();
S_DATA(t1243)[2]=t1261;
t1261=pick(2);
save(t1261);
X t1262=allocate(2); // CONS
save(t1262);
t1261=pick(2);
save(t1261);
X t1263=allocate(2); // CONS
save(t1263);
t1261=pick(2);
X t1264=lf[326]; // CONST
t1263=top();
S_DATA(t1263)[0]=t1264;
t1264=pick(2);
X t1266=TO_N(2); // ICONST
save(t1266);
X t1267=select1(TO_N(1),t1264); // SELECT (simple)
t1266=restore();
X t1265=select1(t1266,t1267); // SELECT
t1263=top();
S_DATA(t1263)[1]=t1265;
t1263=restore();
restore();
t1262=top();
S_DATA(t1262)[0]=t1263;
t1263=pick(2);
save(t1263);
X t1268=allocate(2); // CONS
save(t1268);
t1263=pick(2);
X t1270=TO_N(2); // ICONST
save(t1270);
X t1271=select1(TO_N(1),t1263); // SELECT (simple)
t1270=restore();
X t1269=select1(t1270,t1271); // SELECT
t1268=top();
S_DATA(t1268)[0]=t1269;
t1269=pick(2);
save(t1269);
X t1272=allocate(2); // CONS
save(t1272);
t1269=pick(2);
X t1273=select1(TO_N(1),t1269); // SELECT (simple)
X t1274=___tl(t1273); // REF: tl
X t1275=___tl(t1274); // REF: tl
int t1277; // ALPHA
check_S(t1275,"@");
int t1278=S_LENGTH(t1275);
save(t1275);
X t1276=allocate(t1278);
save(t1276);
for(t1277=0;t1277<t1278;++t1277){
X t1276=S_DATA(pick(2))[t1277];
X t1279=select1(TO_N(4),t1276); // SELECT (simple)
S_DATA(top())[t1277]=t1279;}
t1276=restore();
restore();
t1272=top();
S_DATA(t1272)[0]=t1276;
t1276=pick(2);
X t1280=select1(TO_N(1),t1276); // SELECT (simple)
X t1281=___tl(t1280); // REF: tl
X t1282=___tl(t1281); // REF: tl
X t1283=___len(t1282); // REF: len
X t1284=___iota(t1283); // REF: iota
t1272=top();
S_DATA(t1272)[1]=t1284;
t1272=restore();
restore();
X t1285=___trans(t1272); // REF: trans
t1268=top();
S_DATA(t1268)[1]=t1285;
t1268=restore();
restore();
X t1286=___dl(t1268); // REF: dl
int t1288; // ALPHA
check_S(t1286,"@");
int t1289=S_LENGTH(t1286);
save(t1286);
X t1287=allocate(t1289);
save(t1287);
for(t1288=0;t1288<t1289;++t1288){
X t1287=S_DATA(pick(2))[t1288];
save(t1287);
X t1290=allocate(4); // CONS
save(t1290);
t1287=pick(2);
X t1291=lf[320]; // CONST
t1290=top();
S_DATA(t1290)[0]=t1291;
t1291=pick(2);
X t1292=select1(TO_N(1),t1291); // SELECT (simple)
t1290=top();
S_DATA(t1290)[1]=t1292;
t1292=pick(2);
X t1294=TO_N(1); // ICONST
save(t1294);
X t1295=select1(TO_N(2),t1292); // SELECT (simple)
t1294=restore();
X t1293=select1(t1294,t1295); // SELECT
t1290=top();
S_DATA(t1290)[2]=t1293;
t1293=pick(2);
save(t1293);
X t1296=allocate(4); // CONS
save(t1296);
t1293=pick(2);
X t1297=lf[326]; // CONST
t1296=top();
S_DATA(t1296)[0]=t1297;
t1297=pick(2);
X t1298=select1(TO_N(1),t1297); // SELECT (simple)
t1296=top();
S_DATA(t1296)[1]=t1298;
t1298=pick(2);
save(t1298);
X t1299=allocate(4); // CONS
save(t1299);
t1298=pick(2);
X t1300=lf[319]; // CONST
t1299=top();
S_DATA(t1299)[0]=t1300;
t1300=pick(2);
X t1301=select1(TO_N(1),t1300); // SELECT (simple)
t1299=top();
S_DATA(t1299)[1]=t1301;
t1301=pick(2);
save(t1301);
X t1302=allocate(3); // CONS
save(t1302);
t1301=pick(2);
X t1303=lf[291]; // CONST
t1302=top();
S_DATA(t1302)[0]=t1303;
t1303=pick(2);
X t1304=select1(TO_N(1),t1303); // SELECT (simple)
t1302=top();
S_DATA(t1302)[1]=t1304;
t1304=pick(2);
X t1305=TO_N(1); // ICONST
t1302=top();
S_DATA(t1302)[2]=t1305;
t1302=restore();
restore();
t1299=top();
S_DATA(t1299)[2]=t1302;
t1302=pick(2);
save(t1302);
X t1306=allocate(3); // CONS
save(t1306);
t1302=pick(2);
X t1307=lf[293]; // CONST
t1306=top();
S_DATA(t1306)[0]=t1307;
t1307=pick(2);
X t1308=select1(TO_N(1),t1307); // SELECT (simple)
t1306=top();
S_DATA(t1306)[1]=t1308;
t1308=pick(2);
X t1309=lf[282]; // CONST
t1306=top();
S_DATA(t1306)[2]=t1309;
t1306=restore();
restore();
t1299=top();
S_DATA(t1299)[3]=t1306;
t1299=restore();
restore();
t1296=top();
S_DATA(t1296)[2]=t1299;
t1299=pick(2);
save(t1299);
X t1310=allocate(4); // CONS
save(t1310);
t1299=pick(2);
X t1311=lf[319]; // CONST
t1310=top();
S_DATA(t1310)[0]=t1311;
t1311=pick(2);
X t1312=select1(TO_N(1),t1311); // SELECT (simple)
t1310=top();
S_DATA(t1310)[1]=t1312;
t1312=pick(2);
save(t1312);
X t1313=allocate(3); // CONS
save(t1313);
t1312=pick(2);
X t1314=lf[291]; // CONST
t1313=top();
S_DATA(t1313)[0]=t1314;
t1314=pick(2);
X t1315=select1(TO_N(1),t1314); // SELECT (simple)
t1313=top();
S_DATA(t1313)[1]=t1315;
t1315=pick(2);
X t1317=TO_N(2); // ICONST
save(t1317);
X t1318=select1(TO_N(2),t1315); // SELECT (simple)
t1317=restore();
X t1316=select1(t1317,t1318); // SELECT
t1313=top();
S_DATA(t1313)[2]=t1316;
t1313=restore();
restore();
t1310=top();
S_DATA(t1310)[2]=t1313;
t1313=pick(2);
save(t1313);
X t1319=allocate(4); // CONS
save(t1319);
t1313=pick(2);
X t1320=lf[319]; // CONST
t1319=top();
S_DATA(t1319)[0]=t1320;
t1320=pick(2);
X t1321=select1(TO_N(1),t1320); // SELECT (simple)
t1319=top();
S_DATA(t1319)[1]=t1321;
t1321=pick(2);
save(t1321);
X t1322=allocate(3); // CONS
save(t1322);
t1321=pick(2);
X t1323=lf[291]; // CONST
t1322=top();
S_DATA(t1322)[0]=t1323;
t1323=pick(2);
X t1324=select1(TO_N(1),t1323); // SELECT (simple)
t1322=top();
S_DATA(t1322)[1]=t1324;
t1324=pick(2);
X t1325=TO_N(2); // ICONST
t1322=top();
S_DATA(t1322)[2]=t1325;
t1322=restore();
restore();
t1319=top();
S_DATA(t1319)[2]=t1322;
t1322=pick(2);
save(t1322);
X t1326=allocate(3); // CONS
save(t1326);
t1322=pick(2);
X t1327=lf[293]; // CONST
t1326=top();
S_DATA(t1326)[0]=t1327;
t1327=pick(2);
X t1328=select1(TO_N(1),t1327); // SELECT (simple)
t1326=top();
S_DATA(t1326)[1]=t1328;
t1328=pick(2);
X t1329=lf[282]; // CONST
t1326=top();
S_DATA(t1326)[2]=t1329;
t1326=restore();
restore();
t1319=top();
S_DATA(t1319)[3]=t1326;
t1319=restore();
restore();
t1310=top();
S_DATA(t1310)[3]=t1319;
t1310=restore();
restore();
t1296=top();
S_DATA(t1296)[3]=t1310;
t1296=restore();
restore();
t1290=top();
S_DATA(t1290)[3]=t1296;
t1290=restore();
restore();
S_DATA(top())[t1288]=t1290;}
t1287=restore();
restore();
t1262=top();
S_DATA(t1262)[1]=t1287;
t1262=restore();
restore();
X t1330=___cat(t1262); // REF: cat
t1243=top();
S_DATA(t1243)[3]=t1330;
t1243=restore();
restore();
t1242=top();
S_DATA(t1242)[0]=t1243;
t1243=pick(2);
save(t1243);
X t1331=allocate(3); // CONS
save(t1331);
t1243=pick(2);
X t1332=lf[146]; // CONST
t1331=top();
S_DATA(t1331)[0]=t1332;
t1332=pick(2);
X t1334=TO_N(2); // ICONST
save(t1334);
X t1335=select1(TO_N(1),t1332); // SELECT (simple)
t1334=restore();
X t1333=select1(t1334,t1335); // SELECT
t1331=top();
S_DATA(t1331)[1]=t1333;
t1333=pick(2);
X t1336=select1(TO_N(2),t1333); // SELECT (simple)
t1331=top();
S_DATA(t1331)[2]=t1336;
t1331=restore();
restore();
X t1337=___rewrite_3alog(t1331); // REF: rewrite:log
t1242=top();
S_DATA(t1242)[1]=t1337;
t1242=restore();
restore();
x=t1242; // REF: rewrite:rewrite1
goto loop; // tail call: rewrite:rewrite1
X t1338;
t1221=t1338;
}else{
save(x); // COND
X t1340=select1(TO_N(1),x); // SELECT (simple)
X t1341=F;
save(t1340); // PCONSL
if(!IS_S(t1340)||S_LENGTH(t1340)<3) goto t1342;
t1340=S_DATA(top())[0];
X t1344=lf[320]; // CONST
save(t1344);
X t1345=___id(t1340); // REF: id
t1344=restore();
X t1343=(t1344==t1345)||eq1(t1344,t1345)?T:F; // EQ
if(t1343==F) goto t1342;
// skipped
t1343=S_DATA(top())[2];
X t1346=F;
save(t1343); // PCONS
if(!IS_S(t1343)||S_LENGTH(t1343)!=4) goto t1347;
t1343=S_DATA(top())[0];
X t1349=lf[319]; // CONST
save(t1349);
X t1350=___id(t1343); // REF: id
t1349=restore();
X t1348=(t1349==t1350)||eq1(t1349,t1350)?T:F; // EQ
if(t1348==F) goto t1347;
// skipped
// skipped
t1348=S_DATA(top())[3];
X t1351=F;
save(t1348); // PCONS
if(!IS_S(t1348)||S_LENGTH(t1348)!=3) goto t1352;
t1348=S_DATA(top())[0];
X t1354=lf[293]; // CONST
save(t1354);
X t1355=___id(t1348); // REF: id
t1354=restore();
X t1353=(t1354==t1355)||eq1(t1354,t1355)?T:F; // EQ
if(t1353==F) goto t1352;
// skipped
t1353=S_DATA(top())[2];
X t1357=lf[282]; // CONST
save(t1357);
X t1358=___id(t1353); // REF: id
t1357=restore();
X t1356=(t1357==t1358)||eq1(t1357,t1358)?T:F; // EQ
if(t1356==F) goto t1352;
t1351=T;
t1352:
restore();
if(t1351==F) goto t1347;
t1346=T;
t1347:
restore();
if(t1346==F) goto t1342;
t1341=T;
t1342:
restore();
X t1339;
x=restore();
if(t1341!=F){
save(x);
X t1359=allocate(2); // CONS
save(t1359);
x=pick(2);
save(x);
X t1360=allocate(4); // CONS
save(t1360);
x=pick(2);
X t1361=lf[319]; // CONST
t1360=top();
S_DATA(t1360)[0]=t1361;
t1361=pick(2);
X t1363=TO_N(2); // ICONST
save(t1363);
X t1364=select1(TO_N(1),t1361); // SELECT (simple)
t1363=restore();
X t1362=select1(t1363,t1364); // SELECT
t1360=top();
S_DATA(t1360)[1]=t1362;
t1362=pick(2);
X t1366=TO_N(3); // ICONST
save(t1366);
X t1368=TO_N(3); // ICONST
save(t1368);
X t1369=select1(TO_N(1),t1362); // SELECT (simple)
t1368=restore();
X t1367=select1(t1368,t1369); // SELECT
t1366=restore();
X t1365=select1(t1366,t1367); // SELECT
t1360=top();
S_DATA(t1360)[2]=t1365;
t1365=pick(2);
X t1371=TO_N(4); // ICONST
save(t1371);
X t1372=select1(TO_N(1),t1365); // SELECT (simple)
t1371=restore();
X t1370=select1(t1371,t1372); // SELECT
t1360=top();
S_DATA(t1360)[3]=t1370;
t1360=restore();
restore();
t1359=top();
S_DATA(t1359)[0]=t1360;
t1360=pick(2);
save(t1360);
X t1373=allocate(3); // CONS
save(t1373);
t1360=pick(2);
X t1374=lf[147]; // CONST
t1373=top();
S_DATA(t1373)[0]=t1374;
t1374=pick(2);
X t1376=TO_N(2); // ICONST
save(t1376);
X t1377=select1(TO_N(1),t1374); // SELECT (simple)
t1376=restore();
X t1375=select1(t1376,t1377); // SELECT
t1373=top();
S_DATA(t1373)[1]=t1375;
t1375=pick(2);
X t1378=select1(TO_N(2),t1375); // SELECT (simple)
t1373=top();
S_DATA(t1373)[2]=t1378;
t1373=restore();
restore();
X t1379=___rewrite_3alog(t1373); // REF: rewrite:log
t1359=top();
S_DATA(t1359)[1]=t1379;
t1359=restore();
restore();
x=t1359; // REF: rewrite:rewrite1
goto loop; // tail call: rewrite:rewrite1
X t1380;
t1339=t1380;
}else{
save(x); // COND
X t1382=select1(TO_N(1),x); // SELECT (simple)
X t1383=F;
save(t1382); // PCONSL
if(!IS_S(t1382)||S_LENGTH(t1382)<3) goto t1384;
t1382=S_DATA(top())[0];
X t1386=lf[320]; // CONST
save(t1386);
X t1387=___id(t1382); // REF: id
t1386=restore();
X t1385=(t1386==t1387)||eq1(t1386,t1387)?T:F; // EQ
if(t1385==F) goto t1384;
// skipped
t1385=S_DATA(top())[2];
X t1388=F;
save(t1385); // PCONS
if(!IS_S(t1385)||S_LENGTH(t1385)!=4) goto t1389;
t1385=S_DATA(top())[0];
X t1391=lf[317]; // CONST
save(t1391);
X t1392=___id(t1385); // REF: id
t1391=restore();
X t1390=(t1391==t1392)||eq1(t1391,t1392)?T:F; // EQ
if(t1390==F) goto t1389;
// skipped
// skipped
t1390=S_DATA(top())[3];
X t1393=F;
save(t1390); // PCONS
if(!IS_S(t1390)||S_LENGTH(t1390)!=3) goto t1394;
t1390=S_DATA(top())[0];
X t1396=lf[293]; // CONST
save(t1396);
X t1397=___id(t1390); // REF: id
t1396=restore();
X t1395=(t1396==t1397)||eq1(t1396,t1397)?T:F; // EQ
if(t1395==F) goto t1394;
// skipped
t1395=S_DATA(top())[2];
X t1399=lf[282]; // CONST
save(t1399);
X t1400=___id(t1395); // REF: id
t1399=restore();
X t1398=(t1399==t1400)||eq1(t1399,t1400)?T:F; // EQ
if(t1398==F) goto t1394;
t1393=T;
t1394:
restore();
if(t1393==F) goto t1389;
t1388=T;
t1389:
restore();
if(t1388==F) goto t1384;
t1383=T;
t1384:
restore();
X t1381;
x=restore();
if(t1383!=F){
save(x);
X t1401=allocate(2); // CONS
save(t1401);
x=pick(2);
save(x);
X t1402=allocate(4); // CONS
save(t1402);
x=pick(2);
X t1403=lf[317]; // CONST
t1402=top();
S_DATA(t1402)[0]=t1403;
t1403=pick(2);
X t1405=TO_N(2); // ICONST
save(t1405);
X t1406=select1(TO_N(1),t1403); // SELECT (simple)
t1405=restore();
X t1404=select1(t1405,t1406); // SELECT
t1402=top();
S_DATA(t1402)[1]=t1404;
t1404=pick(2);
X t1408=TO_N(3); // ICONST
save(t1408);
X t1410=TO_N(3); // ICONST
save(t1410);
X t1411=select1(TO_N(1),t1404); // SELECT (simple)
t1410=restore();
X t1409=select1(t1410,t1411); // SELECT
t1408=restore();
X t1407=select1(t1408,t1409); // SELECT
t1402=top();
S_DATA(t1402)[2]=t1407;
t1407=pick(2);
X t1413=TO_N(4); // ICONST
save(t1413);
X t1414=select1(TO_N(1),t1407); // SELECT (simple)
t1413=restore();
X t1412=select1(t1413,t1414); // SELECT
t1402=top();
S_DATA(t1402)[3]=t1412;
t1402=restore();
restore();
t1401=top();
S_DATA(t1401)[0]=t1402;
t1402=pick(2);
save(t1402);
X t1415=allocate(3); // CONS
save(t1415);
t1402=pick(2);
X t1416=lf[148]; // CONST
t1415=top();
S_DATA(t1415)[0]=t1416;
t1416=pick(2);
X t1418=TO_N(2); // ICONST
save(t1418);
X t1419=select1(TO_N(1),t1416); // SELECT (simple)
t1418=restore();
X t1417=select1(t1418,t1419); // SELECT
t1415=top();
S_DATA(t1415)[1]=t1417;
t1417=pick(2);
X t1420=select1(TO_N(2),t1417); // SELECT (simple)
t1415=top();
S_DATA(t1415)[2]=t1420;
t1415=restore();
restore();
X t1421=___rewrite_3alog(t1415); // REF: rewrite:log
t1401=top();
S_DATA(t1401)[1]=t1421;
t1401=restore();
restore();
x=t1401; // REF: rewrite:rewrite1
goto loop; // tail call: rewrite:rewrite1
X t1422;
t1381=t1422;
}else{
save(x); // COND
save(x); // COND
X t1425=select1(TO_N(1),x); // SELECT (simple)
X t1426=F;
save(t1425); // PCONSL
if(!IS_S(t1425)||S_LENGTH(t1425)<3) goto t1427;
t1425=S_DATA(top())[0];
X t1429=lf[326]; // CONST
save(t1429);
X t1430=___id(t1425); // REF: id
t1429=restore();
X t1428=(t1429==t1430)||eq1(t1429,t1430)?T:F; // EQ
if(t1428==F) goto t1427;
// skipped
t1428=S_DATA(top())[2];
X t1431=F;
save(t1428); // PCONSL
if(!IS_S(t1428)||S_LENGTH(t1428)<1) goto t1432;
t1428=S_DATA(top())[0];
save(t1428); // OR
X t1435=lf[290]; // CONST
save(t1435);
X t1436=___id(t1428); // REF: id
t1435=restore();
X t1434=(t1435==t1436)||eq1(t1435,t1436)?T:F; // EQ
X t1433=t1434;
t1428=restore();
if(t1433==F){
X t1438=lf[291]; // CONST
save(t1438);
X t1439=___id(t1428); // REF: id
t1438=restore();
X t1437=(t1438==t1439)||eq1(t1438,t1439)?T:F; // EQ
t1433=t1437;}
if(t1433==F) goto t1432;
t1431=T;
t1432:
restore();
if(t1431==F) goto t1427;
t1426=T;
t1427:
restore();
X t1424;
x=restore();
if(t1426!=F){
X t1440=select1(TO_N(1),x); // SELECT (simple)
X t1441=___tl(t1440); // REF: tl
X t1442=___tl(t1441); // REF: tl
save(t1442); // KATA
X t1447=T; // ICONST
int t1444;
check_S(top(),"(|...|)");
int t1445=S_LENGTH(top());
for(t1444=t1445-1;t1444>=0;--t1444){
X t1443=sequence(2,S_DATA(top())[t1444],t1447);
save(t1443); // COND
X t1449=select1(TO_N(2),t1443); // SELECT (simple)
X t1448;
t1443=restore();
if(t1449!=F){
X t1450=select1(TO_N(1),t1443); // SELECT (simple)
X t1451=F;
save(t1450); // PCONSL
if(!IS_S(t1450)||S_LENGTH(t1450)<1) goto t1452;
t1450=S_DATA(top())[0];
save(t1450); // OR
X t1455=lf[291]; // CONST
save(t1455);
X t1456=___id(t1450); // REF: id
t1455=restore();
X t1454=(t1455==t1456)||eq1(t1455,t1456)?T:F; // EQ
X t1453=t1454;
t1450=restore();
if(t1453==F){
X t1458=lf[290]; // CONST
save(t1458);
X t1459=___id(t1450); // REF: id
t1458=restore();
X t1457=(t1458==t1459)||eq1(t1458,t1459)?T:F; // EQ
t1453=t1457;}
if(t1453==F) goto t1452;
t1451=T;
t1452:
restore();
t1448=t1451;
}else{
X t1460=F; // ICONST
t1448=t1460;}
t1447=t1448;}
restore();
X t1446=t1447;
t1424=t1446;
}else{
X t1461=F; // ICONST
t1424=t1461;}
X t1423;
x=restore();
if(t1424!=F){
save(x);
X t1462=allocate(2); // CONS
save(t1462);
x=pick(2);
save(x);
X t1463=allocate(3); // CONS
save(t1463);
x=pick(2);
X t1464=lf[290]; // CONST
t1463=top();
S_DATA(t1463)[0]=t1464;
t1464=pick(2);
X t1466=TO_N(2); // ICONST
save(t1466);
X t1467=select1(TO_N(1),t1464); // SELECT (simple)
t1466=restore();
X t1465=select1(t1466,t1467); // SELECT
t1463=top();
S_DATA(t1463)[1]=t1465;
t1465=pick(2);
X t1468=select1(TO_N(1),t1465); // SELECT (simple)
X t1469=___tl(t1468); // REF: tl
X t1470=___tl(t1469); // REF: tl
int t1472; // ALPHA
check_S(t1470,"@");
int t1473=S_LENGTH(t1470);
save(t1470);
X t1471=allocate(t1473);
save(t1471);
for(t1472=0;t1472<t1473;++t1472){
X t1471=S_DATA(pick(2))[t1472];
X t1474=select1(TO_N(3),t1471); // SELECT (simple)
S_DATA(top())[t1472]=t1474;}
t1471=restore();
restore();
t1463=top();
S_DATA(t1463)[2]=t1471;
t1463=restore();
restore();
t1462=top();
S_DATA(t1462)[0]=t1463;
t1463=pick(2);
save(t1463);
X t1475=allocate(3); // CONS
save(t1475);
t1463=pick(2);
X t1476=lf[149]; // CONST
t1475=top();
S_DATA(t1475)[0]=t1476;
t1476=pick(2);
X t1478=TO_N(2); // ICONST
save(t1478);
X t1479=select1(TO_N(1),t1476); // SELECT (simple)
t1478=restore();
X t1477=select1(t1478,t1479); // SELECT
t1475=top();
S_DATA(t1475)[1]=t1477;
t1477=pick(2);
X t1480=select1(TO_N(2),t1477); // SELECT (simple)
t1475=top();
S_DATA(t1475)[2]=t1480;
t1475=restore();
restore();
X t1481=___rewrite_3alog(t1475); // REF: rewrite:log
t1462=top();
S_DATA(t1462)[1]=t1481;
t1462=restore();
restore();
x=t1462; // REF: rewrite:rewrite1
goto loop; // tail call: rewrite:rewrite1
X t1482;
t1423=t1482;
}else{
save(x); // COND
X t1484=select1(TO_N(1),x); // SELECT (simple)
X t1485=F;
save(t1484); // PCONS
if(!IS_S(t1484)||S_LENGTH(t1484)!=4) goto t1486;
t1484=S_DATA(top())[0];
X t1488=lf[319]; // CONST
save(t1488);
X t1489=___id(t1484); // REF: id
t1488=restore();
X t1487=(t1488==t1489)||eq1(t1488,t1489)?T:F; // EQ
if(t1487==F) goto t1486;
// skipped
t1487=S_DATA(top())[2];
X t1490=F;
save(t1487); // PCONSL
if(!IS_S(t1487)||S_LENGTH(t1487)<1) goto t1491;
t1487=S_DATA(top())[0];
X t1493=lf[291]; // CONST
save(t1493);
X t1494=___id(t1487); // REF: id
t1493=restore();
X t1492=(t1493==t1494)||eq1(t1493,t1494)?T:F; // EQ
if(t1492==F) goto t1491;
t1490=T;
t1491:
restore();
if(t1490==F) goto t1486;
t1490=S_DATA(top())[3];
X t1495=F;
save(t1490); // PCONSL
if(!IS_S(t1490)||S_LENGTH(t1490)<1) goto t1496;
t1490=S_DATA(top())[0];
X t1498=lf[290]; // CONST
save(t1498);
X t1499=___id(t1490); // REF: id
t1498=restore();
X t1497=(t1498==t1499)||eq1(t1498,t1499)?T:F; // EQ
if(t1497==F) goto t1496;
t1495=T;
t1496:
restore();
if(t1495==F) goto t1486;
t1485=T;
t1486:
restore();
X t1483;
x=restore();
if(t1485!=F){
save(x); // COND
X t1502=TO_N(3); // ICONST
save(t1502);
X t1504=TO_N(4); // ICONST
save(t1504);
X t1505=select1(TO_N(1),x); // SELECT (simple)
t1504=restore();
X t1503=select1(t1504,t1505); // SELECT
t1502=restore();
X t1501=select1(t1502,t1503); // SELECT
X t1506=___atom(t1501); // REF: atom
X t1500;
x=restore();
if(t1506!=F){
save(x);
X t1507=allocate(3); // CONS
save(t1507);
x=pick(2);
X t1508=lf[150]; // CONST
t1507=top();
S_DATA(t1507)[0]=t1508;
t1508=pick(2);
X t1510=TO_N(2); // ICONST
save(t1510);
X t1512=TO_N(3); // ICONST
save(t1512);
X t1514=TO_N(4); // ICONST
save(t1514);
X t1515=select1(TO_N(1),t1508); // SELECT (simple)
t1514=restore();
X t1513=select1(t1514,t1515); // SELECT
t1512=restore();
X t1511=select1(t1512,t1513); // SELECT
t1510=restore();
X t1509=select1(t1510,t1511); // SELECT
X t1516=___tos(t1509); // REF: tos
t1507=top();
S_DATA(t1507)[1]=t1516;
t1516=pick(2);
X t1517=lf[151]; // CONST
t1507=top();
S_DATA(t1507)[2]=t1517;
t1507=restore();
restore();
X t1518=___cat(t1507); // REF: cat
X t1519=___quit(t1518); // REF: quit
t1500=t1519;
}else{
save(x);
X t1520=allocate(2); // CONS
save(t1520);
x=pick(2);
save(x);
X t1521=allocate(3); // CONS
save(t1521);
x=pick(2);
save(x);
X t1522=allocate(2); // CONS
save(t1522);
x=pick(2);
X t1523=select1(TO_N(1),x); // SELECT (simple)
t1522=top();
S_DATA(t1522)[0]=t1523;
t1523=pick(2);
X t1525=TO_N(3); // ICONST
save(t1525);
X t1527=TO_N(4); // ICONST
save(t1527);
X t1528=select1(TO_N(1),t1523); // SELECT (simple)
t1527=restore();
X t1526=select1(t1527,t1528); // SELECT
t1525=restore();
X t1524=select1(t1525,t1526); // SELECT
t1522=top();
S_DATA(t1522)[1]=t1524;
t1522=restore();
restore();
X t1529=___rewrite_3acheck_5fselect(t1522); // REF: rewrite:check_select
t1521=top();
S_DATA(t1521)[0]=t1529;
t1529=pick(2);
X t1530=select1(TO_N(1),t1529); // SELECT (simple)
t1521=top();
S_DATA(t1521)[1]=t1530;
t1530=pick(2);
X t1531=select1(TO_N(2),t1530); // SELECT (simple)
t1521=top();
S_DATA(t1521)[2]=t1531;
t1521=restore();
restore();
save(t1521);
X t1532=allocate(3); // CONS
save(t1532);
t1521=pick(2);
X t1533=select1(TO_N(1),t1521); // SELECT (simple)
save(t1533); // COND
X t1535=___num(t1533); // REF: num
X t1534;
t1533=restore();
if(t1535!=F){
X t1536=lf[291]; // CONST
t1534=t1536;
}else{
X t1537=lf[290]; // CONST
t1534=t1537;}
t1532=top();
S_DATA(t1532)[0]=t1534;
t1534=pick(2);
X t1539=TO_N(2); // ICONST
save(t1539);
X t1540=select1(TO_N(2),t1534); // SELECT (simple)
t1539=restore();
X t1538=select1(t1539,t1540); // SELECT
t1532=top();
S_DATA(t1532)[1]=t1538;
t1538=pick(2);
X t1541=select1(TO_N(1),t1538); // SELECT (simple)
t1532=top();
S_DATA(t1532)[2]=t1541;
t1532=restore();
restore();
t1520=top();
S_DATA(t1520)[0]=t1532;
t1532=pick(2);
save(t1532);
X t1542=allocate(3); // CONS
save(t1542);
t1532=pick(2);
X t1543=lf[152]; // CONST
t1542=top();
S_DATA(t1542)[0]=t1543;
t1543=pick(2);
X t1545=TO_N(2); // ICONST
save(t1545);
X t1546=select1(TO_N(1),t1543); // SELECT (simple)
t1545=restore();
X t1544=select1(t1545,t1546); // SELECT
t1542=top();
S_DATA(t1542)[1]=t1544;
t1544=pick(2);
X t1547=select1(TO_N(2),t1544); // SELECT (simple)
t1542=top();
S_DATA(t1542)[2]=t1547;
t1542=restore();
restore();
X t1548=___rewrite_3alog(t1542); // REF: rewrite:log
t1520=top();
S_DATA(t1520)[1]=t1548;
t1520=restore();
restore();
x=t1520; // REF: rewrite:rewrite1
goto loop; // tail call: rewrite:rewrite1
X t1549;
t1500=t1549;}
t1483=t1500;
}else{
save(x); // COND
X t1551=select1(TO_N(1),x); // SELECT (simple)
X t1552=F;
save(t1551); // PCONS
if(!IS_S(t1551)||S_LENGTH(t1551)!=4) goto t1553;
t1551=S_DATA(top())[0];
X t1555=lf[319]; // CONST
save(t1555);
X t1556=___id(t1551); // REF: id
t1555=restore();
X t1554=(t1555==t1556)||eq1(t1555,t1556)?T:F; // EQ
if(t1554==F) goto t1553;
// skipped
t1554=S_DATA(top())[2];
X t1557=F;
save(t1554); // PCONSL
if(!IS_S(t1554)||S_LENGTH(t1554)<1) goto t1558;
t1554=S_DATA(top())[0];
X t1560=lf[291]; // CONST
save(t1560);
X t1561=___id(t1554); // REF: id
t1560=restore();
X t1559=(t1560==t1561)||eq1(t1560,t1561)?T:F; // EQ
if(t1559==F) goto t1558;
t1557=T;
t1558:
restore();
if(t1557==F) goto t1553;
t1557=S_DATA(top())[3];
X t1562=F;
save(t1557); // PCONSL
if(!IS_S(t1557)||S_LENGTH(t1557)<1) goto t1563;
t1557=S_DATA(top())[0];
X t1565=lf[326]; // CONST
save(t1565);
X t1566=___id(t1557); // REF: id
t1565=restore();
X t1564=(t1565==t1566)||eq1(t1565,t1566)?T:F; // EQ
if(t1564==F) goto t1563;
t1562=T;
t1563:
restore();
if(t1562==F) goto t1553;
t1552=T;
t1553:
restore();
X t1550;
x=restore();
if(t1552!=F){
save(x);
X t1567=allocate(2); // CONS
save(t1567);
x=pick(2);
save(x);
X t1568=allocate(2); // CONS
save(t1568);
x=pick(2);
X t1569=select1(TO_N(1),x); // SELECT (simple)
t1568=top();
S_DATA(t1568)[0]=t1569;
t1569=pick(2);
X t1571=TO_N(4); // ICONST
save(t1571);
X t1572=select1(TO_N(1),t1569); // SELECT (simple)
t1571=restore();
X t1570=select1(t1571,t1572); // SELECT
X t1573=___tl(t1570); // REF: tl
X t1574=___tl(t1573); // REF: tl
t1568=top();
S_DATA(t1568)[1]=t1574;
t1568=restore();
restore();
X t1575=___rewrite_3acheck_5fselect(t1568); // REF: rewrite:check_select
t1567=top();
S_DATA(t1567)[0]=t1575;
t1575=pick(2);
save(t1575);
X t1576=allocate(3); // CONS
save(t1576);
t1575=pick(2);
X t1577=lf[153]; // CONST
t1576=top();
S_DATA(t1576)[0]=t1577;
t1577=pick(2);
X t1579=TO_N(2); // ICONST
save(t1579);
X t1580=select1(TO_N(1),t1577); // SELECT (simple)
t1579=restore();
X t1578=select1(t1579,t1580); // SELECT
t1576=top();
S_DATA(t1576)[1]=t1578;
t1578=pick(2);
X t1581=select1(TO_N(2),t1578); // SELECT (simple)
t1576=top();
S_DATA(t1576)[2]=t1581;
t1576=restore();
restore();
X t1582=___rewrite_3alog(t1576); // REF: rewrite:log
t1567=top();
S_DATA(t1567)[1]=t1582;
t1567=restore();
restore();
x=t1567; // REF: rewrite:rewrite1
goto loop; // tail call: rewrite:rewrite1
X t1583;
t1550=t1583;
}else{
save(x); // COND
X t1585=select1(TO_N(1),x); // SELECT (simple)
X t1586=F;
save(t1585); // PCONS
if(!IS_S(t1585)||S_LENGTH(t1585)!=4) goto t1587;
t1585=S_DATA(top())[0];
X t1589=lf[320]; // CONST
save(t1589);
X t1590=___id(t1585); // REF: id
t1589=restore();
X t1588=(t1589==t1590)||eq1(t1589,t1590)?T:F; // EQ
if(t1588==F) goto t1587;
// skipped
t1588=S_DATA(top())[2];
save(t1588);
X t1591=allocate(2); // CONS
save(t1591);
t1588=pick(2);
X t1592=select1(TO_N(1),t1588); // SELECT (simple)
t1591=top();
S_DATA(t1591)[0]=t1592;
t1592=pick(2);
X t1593=lf[154]; // CONST
t1591=top();
S_DATA(t1591)[1]=t1593;
t1591=restore();
restore();
X t1594=___index(t1591); // REF: index
if(t1594==F) goto t1587;
// skipped
t1586=T;
t1587:
restore();
X t1584;
x=restore();
if(t1586!=F){
save(x);
X t1595=allocate(2); // CONS
save(t1595);
x=pick(2);
X t1597=TO_N(4); // ICONST
save(t1597);
X t1598=select1(TO_N(1),x); // SELECT (simple)
t1597=restore();
X t1596=select1(t1597,t1598); // SELECT
t1595=top();
S_DATA(t1595)[0]=t1596;
t1596=pick(2);
save(t1596);
X t1599=allocate(3); // CONS
save(t1599);
t1596=pick(2);
X t1600=lf[155]; // CONST
t1599=top();
S_DATA(t1599)[0]=t1600;
t1600=pick(2);
X t1602=TO_N(2); // ICONST
save(t1602);
X t1603=select1(TO_N(1),t1600); // SELECT (simple)
t1602=restore();
X t1601=select1(t1602,t1603); // SELECT
t1599=top();
S_DATA(t1599)[1]=t1601;
t1601=pick(2);
X t1604=select1(TO_N(2),t1601); // SELECT (simple)
t1599=top();
S_DATA(t1599)[2]=t1604;
t1599=restore();
restore();
X t1605=___rewrite_3alog(t1599); // REF: rewrite:log
t1595=top();
S_DATA(t1595)[1]=t1605;
t1595=restore();
restore();
x=t1595; // REF: rewrite:rewrite1
goto loop; // tail call: rewrite:rewrite1
X t1606;
t1584=t1606;
}else{
save(x); // COND
X t1608=select1(TO_N(1),x); // SELECT (simple)
X t1609=F;
save(t1608); // PCONS
if(!IS_S(t1608)||S_LENGTH(t1608)!=4) goto t1610;
t1608=S_DATA(top())[0];
X t1612=lf[315]; // CONST
save(t1612);
X t1613=___id(t1608); // REF: id
t1612=restore();
X t1611=(t1612==t1613)||eq1(t1612,t1613)?T:F; // EQ
if(t1611==F) goto t1610;
// skipped
t1611=S_DATA(top())[2];
X t1614=F;
save(t1611); // PCONSL
if(!IS_S(t1611)||S_LENGTH(t1611)<1) goto t1615;
t1611=S_DATA(top())[0];
save(t1611); // OR
X t1618=lf[291]; // CONST
save(t1618);
X t1619=___id(t1611); // REF: id
t1618=restore();
X t1617=(t1618==t1619)||eq1(t1618,t1619)?T:F; // EQ
X t1616=t1617;
t1611=restore();
if(t1616==F){
X t1621=lf[290]; // CONST
save(t1621);
X t1622=___id(t1611); // REF: id
t1621=restore();
X t1620=(t1621==t1622)||eq1(t1621,t1622)?T:F; // EQ
t1616=t1620;}
if(t1616==F) goto t1615;
t1614=T;
t1615:
restore();
if(t1614==F) goto t1610;
// skipped
t1609=T;
t1610:
restore();
X t1607;
x=restore();
if(t1609!=F){
save(x);
X t1623=allocate(2); // CONS
save(t1623);
x=pick(2);
save(x); // COND
X t1626=TO_N(3); // ICONST
save(t1626);
X t1628=TO_N(3); // ICONST
save(t1628);
X t1629=select1(TO_N(1),x); // SELECT (simple)
t1628=restore();
X t1627=select1(t1628,t1629); // SELECT
t1626=restore();
X t1625=select1(t1626,t1627); // SELECT
X t1624;
x=restore();
if(t1625!=F){
X t1631=TO_N(3); // ICONST
save(t1631);
X t1632=select1(TO_N(1),x); // SELECT (simple)
t1631=restore();
X t1630=select1(t1631,t1632); // SELECT
t1624=t1630;
}else{
X t1634=TO_N(4); // ICONST
save(t1634);
X t1635=select1(TO_N(1),x); // SELECT (simple)
t1634=restore();
X t1633=select1(t1634,t1635); // SELECT
t1624=t1633;}
t1623=top();
S_DATA(t1623)[0]=t1624;
t1624=pick(2);
save(t1624);
X t1636=allocate(3); // CONS
save(t1636);
t1624=pick(2);
X t1637=lf[156]; // CONST
t1636=top();
S_DATA(t1636)[0]=t1637;
t1637=pick(2);
X t1639=TO_N(2); // ICONST
save(t1639);
X t1640=select1(TO_N(1),t1637); // SELECT (simple)
t1639=restore();
X t1638=select1(t1639,t1640); // SELECT
t1636=top();
S_DATA(t1636)[1]=t1638;
t1638=pick(2);
X t1641=select1(TO_N(2),t1638); // SELECT (simple)
t1636=top();
S_DATA(t1636)[2]=t1641;
t1636=restore();
restore();
X t1642=___rewrite_3alog(t1636); // REF: rewrite:log
t1623=top();
S_DATA(t1623)[1]=t1642;
t1623=restore();
restore();
x=t1623; // REF: rewrite:rewrite1
goto loop; // tail call: rewrite:rewrite1
X t1643;
t1607=t1643;
}else{
save(x); // COND
save(x); // COND
X t1646=select1(TO_N(1),x); // SELECT (simple)
X t1647=F;
save(t1646); // PCONS
if(!IS_S(t1646)||S_LENGTH(t1646)!=4) goto t1648;
t1646=S_DATA(top())[0];
X t1650=lf[315]; // CONST
save(t1650);
X t1651=___id(t1646); // REF: id
t1650=restore();
X t1649=(t1650==t1651)||eq1(t1650,t1651)?T:F; // EQ
if(t1649==F) goto t1648;
// skipped
t1649=S_DATA(top())[2];
X t1652=F;
save(t1649); // PCONSL
if(!IS_S(t1649)||S_LENGTH(t1649)<1) goto t1653;
t1649=S_DATA(top())[0];
X t1655=lf[320]; // CONST
save(t1655);
X t1656=___id(t1649); // REF: id
t1655=restore();
X t1654=(t1655==t1656)||eq1(t1655,t1656)?T:F; // EQ
if(t1654==F) goto t1653;
t1652=T;
t1653:
restore();
if(t1652==F) goto t1648;
// skipped
t1647=T;
t1648:
restore();
X t1645;
x=restore();
if(t1647!=F){
X t1657=select1(TO_N(1),x); // SELECT (simple)
X t1658=___tl(t1657); // REF: tl
X t1659=___tl(t1658); // REF: tl
X t1660=___rewrite_3acommon_5fcomp(t1659); // REF: rewrite:common_comp
t1645=t1660;
}else{
X t1661=F; // ICONST
t1645=t1661;}
X t1644;
x=restore();
if(t1645!=F){
save(x);
X t1662=allocate(2); // CONS
save(t1662);
x=pick(2);
save(x);
X t1663=allocate(4); // CONS
save(t1663);
x=pick(2);
X t1664=lf[320]; // CONST
t1663=top();
S_DATA(t1663)[0]=t1664;
t1664=pick(2);
X t1666=TO_N(2); // ICONST
save(t1666);
X t1667=select1(TO_N(1),t1664); // SELECT (simple)
t1666=restore();
X t1665=select1(t1666,t1667); // SELECT
t1663=top();
S_DATA(t1663)[1]=t1665;
t1665=pick(2);
save(t1665);
X t1668=allocate(4); // CONS
save(t1668);
t1665=pick(2);
X t1669=lf[315]; // CONST
t1668=top();
S_DATA(t1668)[0]=t1669;
t1669=pick(2);
X t1671=TO_N(2); // ICONST
save(t1671);
X t1672=select1(TO_N(1),t1669); // SELECT (simple)
t1671=restore();
X t1670=select1(t1671,t1672); // SELECT
t1668=top();
S_DATA(t1668)[1]=t1670;
t1670=pick(2);
X t1674=TO_N(3); // ICONST
save(t1674);
X t1675=select1(TO_N(1),t1670); // SELECT (simple)
t1674=restore();
X t1673=select1(t1674,t1675); // SELECT
X t1676=___rewrite_3auncomp(t1673); // REF: rewrite:uncomp
t1668=top();
S_DATA(t1668)[2]=t1676;
t1676=pick(2);
X t1678=TO_N(4); // ICONST
save(t1678);
X t1679=select1(TO_N(1),t1676); // SELECT (simple)
t1678=restore();
X t1677=select1(t1678,t1679); // SELECT
X t1680=___rewrite_3auncomp(t1677); // REF: rewrite:uncomp
t1668=top();
S_DATA(t1668)[3]=t1680;
t1668=restore();
restore();
t1663=top();
S_DATA(t1663)[2]=t1668;
t1668=pick(2);
X t1681=select1(TO_N(1),t1668); // SELECT (simple)
X t1682=___tl(t1681); // REF: tl
X t1683=___tl(t1682); // REF: tl
X t1684=___rewrite_3acommon_5fcomp(t1683); // REF: rewrite:common_comp
t1663=top();
S_DATA(t1663)[3]=t1684;
t1663=restore();
restore();
t1662=top();
S_DATA(t1662)[0]=t1663;
t1663=pick(2);
save(t1663);
X t1685=allocate(3); // CONS
save(t1685);
t1663=pick(2);
X t1686=lf[157]; // CONST
t1685=top();
S_DATA(t1685)[0]=t1686;
t1686=pick(2);
X t1688=TO_N(2); // ICONST
save(t1688);
X t1689=select1(TO_N(1),t1686); // SELECT (simple)
t1688=restore();
X t1687=select1(t1688,t1689); // SELECT
t1685=top();
S_DATA(t1685)[1]=t1687;
t1687=pick(2);
X t1690=select1(TO_N(2),t1687); // SELECT (simple)
t1685=top();
S_DATA(t1685)[2]=t1690;
t1685=restore();
restore();
X t1691=___rewrite_3alog(t1685); // REF: rewrite:log
t1662=top();
S_DATA(t1662)[1]=t1691;
t1662=restore();
restore();
x=t1662; // REF: rewrite:rewrite1
goto loop; // tail call: rewrite:rewrite1
X t1692;
t1644=t1692;
}else{
save(x);
X t1693=allocate(3); // CONS
save(t1693);
x=pick(2);
X t1694=select1(TO_N(1),x); // SELECT (simple)
t1693=top();
S_DATA(t1693)[0]=t1694;
t1694=pick(2);
X t1695=select1(TO_N(2),t1694); // SELECT (simple)
t1693=top();
S_DATA(t1693)[1]=t1695;
t1695=pick(2);
save(t1695); // COND
X t1697=select1(TO_N(1),t1695); // SELECT (simple)
X t1698=F;
save(t1697); // PCONSL
if(!IS_S(t1697)||S_LENGTH(t1697)<3) goto t1699;
t1697=S_DATA(top())[0];
X t1701=lf[287]; // CONST
save(t1701);
X t1702=___id(t1697); // REF: id
t1701=restore();
X t1700=(t1701==t1702)||eq1(t1701,t1702)?T:F; // EQ
if(t1700==F) goto t1699;
// skipped
t1700=S_DATA(top())[2];
X t1703=F;
save(t1700); // PCONSL
if(!IS_S(t1700)||S_LENGTH(t1700)<1) goto t1704;
t1700=S_DATA(top())[0];
X t1706=lf[320]; // CONST
save(t1706);
X t1707=___id(t1700); // REF: id
t1706=restore();
X t1705=(t1706==t1707)||eq1(t1706,t1707)?T:F; // EQ
if(t1705==F) goto t1704;
t1703=T;
t1704:
restore();
if(t1703==F) goto t1699;
t1698=T;
t1699:
restore();
X t1696;
t1695=restore();
if(t1698!=F){
X t1708=select1(TO_N(1),t1695); // SELECT (simple)
X t1709=___tl(t1708); // REF: tl
X t1710=___tl(t1709); // REF: tl
X t1711=___rewrite_3acommon_5fcomp(t1710); // REF: rewrite:common_comp
t1696=t1711;
}else{
X t1712=F; // ICONST
t1696=t1712;}
t1693=top();
S_DATA(t1693)[2]=t1696;
t1693=restore();
restore();
save(t1693); // COND
X t1714=select1(TO_N(3),t1693); // SELECT (simple)
X t1713;
t1693=restore();
if(t1714!=F){
save(t1693);
X t1715=allocate(2); // CONS
save(t1715);
t1693=pick(2);
save(t1693);
X t1716=allocate(4); // CONS
save(t1716);
t1693=pick(2);
X t1717=lf[320]; // CONST
t1716=top();
S_DATA(t1716)[0]=t1717;
t1717=pick(2);
X t1719=TO_N(2); // ICONST
save(t1719);
X t1720=select1(TO_N(1),t1717); // SELECT (simple)
t1719=restore();
X t1718=select1(t1719,t1720); // SELECT
t1716=top();
S_DATA(t1716)[1]=t1718;
t1718=pick(2);
save(t1718);
X t1721=allocate(2); // CONS
save(t1721);
t1718=pick(2);
save(t1718);
X t1722=allocate(2); // CONS
save(t1722);
t1718=pick(2);
X t1723=lf[287]; // CONST
t1722=top();
S_DATA(t1722)[0]=t1723;
t1723=pick(2);
X t1725=TO_N(2); // ICONST
save(t1725);
X t1726=select1(TO_N(1),t1723); // SELECT (simple)
t1725=restore();
X t1724=select1(t1725,t1726); // SELECT
t1722=top();
S_DATA(t1722)[1]=t1724;
t1722=restore();
restore();
t1721=top();
S_DATA(t1721)[0]=t1722;
t1722=pick(2);
X t1727=select1(TO_N(1),t1722); // SELECT (simple)
X t1728=___tl(t1727); // REF: tl
X t1729=___tl(t1728); // REF: tl
int t1731; // ALPHA
check_S(t1729,"@");
int t1732=S_LENGTH(t1729);
save(t1729);
X t1730=allocate(t1732);
save(t1730);
for(t1731=0;t1731<t1732;++t1731){
X t1730=S_DATA(pick(2))[t1731];
X t1733=___rewrite_3auncomp(t1730); // REF: rewrite:uncomp
S_DATA(top())[t1731]=t1733;}
t1730=restore();
restore();
t1721=top();
S_DATA(t1721)[1]=t1730;
t1721=restore();
restore();
X t1734=___cat(t1721); // REF: cat
t1716=top();
S_DATA(t1716)[2]=t1734;
t1734=pick(2);
X t1735=select1(TO_N(3),t1734); // SELECT (simple)
t1716=top();
S_DATA(t1716)[3]=t1735;
t1716=restore();
restore();
t1715=top();
S_DATA(t1715)[0]=t1716;
t1716=pick(2);
save(t1716);
X t1736=allocate(3); // CONS
save(t1736);
t1716=pick(2);
X t1737=lf[158]; // CONST
t1736=top();
S_DATA(t1736)[0]=t1737;
t1737=pick(2);
X t1739=TO_N(2); // ICONST
save(t1739);
X t1740=select1(TO_N(1),t1737); // SELECT (simple)
t1739=restore();
X t1738=select1(t1739,t1740); // SELECT
t1736=top();
S_DATA(t1736)[1]=t1738;
t1738=pick(2);
X t1741=select1(TO_N(2),t1738); // SELECT (simple)
t1736=top();
S_DATA(t1736)[2]=t1741;
t1736=restore();
restore();
X t1742=___rewrite_3alog(t1736); // REF: rewrite:log
t1715=top();
S_DATA(t1715)[1]=t1742;
t1715=restore();
restore();
x=t1715; // REF: rewrite:rewrite1
goto loop; // tail call: rewrite:rewrite1
X t1743;
t1713=t1743;
}else{
save(t1693); // COND
save(t1693); // COND
X t1746=select1(TO_N(1),t1693); // SELECT (simple)
X t1747=F;
save(t1746); // PCONS
if(!IS_S(t1746)||S_LENGTH(t1746)!=5) goto t1748;
t1746=S_DATA(top())[0];
X t1750=lf[287]; // CONST
save(t1750);
X t1751=___id(t1746); // REF: id
t1750=restore();
X t1749=(t1750==t1751)||eq1(t1750,t1751)?T:F; // EQ
if(t1749==F) goto t1748;
// skipped
// skipped
t1749=S_DATA(top())[3];
X t1752=F;
save(t1749); // PCONSL
if(!IS_S(t1749)||S_LENGTH(t1749)<1) goto t1753;
t1749=S_DATA(top())[0];
X t1755=lf[320]; // CONST
save(t1755);
X t1756=___id(t1749); // REF: id
t1755=restore();
X t1754=(t1755==t1756)||eq1(t1755,t1756)?T:F; // EQ
if(t1754==F) goto t1753;
t1752=T;
t1753:
restore();
if(t1752==F) goto t1748;
t1752=S_DATA(top())[4];
X t1757=F;
save(t1752); // PCONSL
if(!IS_S(t1752)||S_LENGTH(t1752)<1) goto t1758;
t1752=S_DATA(top())[0];
X t1760=lf[320]; // CONST
save(t1760);
X t1761=___id(t1752); // REF: id
t1760=restore();
X t1759=(t1760==t1761)||eq1(t1760,t1761)?T:F; // EQ
if(t1759==F) goto t1758;
t1757=T;
t1758:
restore();
if(t1757==F) goto t1748;
t1747=T;
t1748:
restore();
X t1745;
t1693=restore();
if(t1747!=F){
X t1762=select1(TO_N(1),t1693); // SELECT (simple)
save(t1762);
X t1763=allocate(2); // CONS
save(t1763);
t1762=pick(2);
X t1765=TO_N(3); // ICONST
save(t1765);
X t1766=select1(TO_N(4),t1762); // SELECT (simple)
t1765=restore();
X t1764=select1(t1765,t1766); // SELECT
t1763=top();
S_DATA(t1763)[0]=t1764;
t1764=pick(2);
X t1768=TO_N(3); // ICONST
save(t1768);
X t1769=select1(TO_N(5),t1764); // SELECT (simple)
t1768=restore();
X t1767=select1(t1768,t1769); // SELECT
t1763=top();
S_DATA(t1763)[1]=t1767;
t1763=restore();
restore();
X t1770=___rewrite_3anode_5feq(t1763); // REF: rewrite:node_eq
t1745=t1770;
}else{
X t1771=F; // ICONST
t1745=t1771;}
X t1744;
t1693=restore();
if(t1745!=F){
save(t1693);
X t1772=allocate(2); // CONS
save(t1772);
t1693=pick(2);
save(t1693);
X t1773=allocate(4); // CONS
save(t1773);
t1693=pick(2);
X t1774=lf[320]; // CONST
t1773=top();
S_DATA(t1773)[0]=t1774;
t1774=pick(2);
X t1776=TO_N(2); // ICONST
save(t1776);
X t1777=select1(TO_N(1),t1774); // SELECT (simple)
t1776=restore();
X t1775=select1(t1776,t1777); // SELECT
t1773=top();
S_DATA(t1773)[1]=t1775;
t1775=pick(2);
X t1779=TO_N(3); // ICONST
save(t1779);
X t1781=TO_N(4); // ICONST
save(t1781);
X t1782=select1(TO_N(1),t1775); // SELECT (simple)
t1781=restore();
X t1780=select1(t1781,t1782); // SELECT
t1779=restore();
X t1778=select1(t1779,t1780); // SELECT
t1773=top();
S_DATA(t1773)[2]=t1778;
t1778=pick(2);
save(t1778);
X t1783=allocate(5); // CONS
save(t1783);
t1778=pick(2);
X t1784=lf[287]; // CONST
t1783=top();
S_DATA(t1783)[0]=t1784;
t1784=pick(2);
X t1786=TO_N(2); // ICONST
save(t1786);
X t1787=select1(TO_N(1),t1784); // SELECT (simple)
t1786=restore();
X t1785=select1(t1786,t1787); // SELECT
t1783=top();
S_DATA(t1783)[1]=t1785;
t1785=pick(2);
X t1789=TO_N(3); // ICONST
save(t1789);
X t1790=select1(TO_N(1),t1785); // SELECT (simple)
t1789=restore();
X t1788=select1(t1789,t1790); // SELECT
t1783=top();
S_DATA(t1783)[2]=t1788;
t1788=pick(2);
X t1792=TO_N(4); // ICONST
save(t1792);
X t1794=TO_N(4); // ICONST
save(t1794);
X t1795=select1(TO_N(1),t1788); // SELECT (simple)
t1794=restore();
X t1793=select1(t1794,t1795); // SELECT
t1792=restore();
X t1791=select1(t1792,t1793); // SELECT
t1783=top();
S_DATA(t1783)[3]=t1791;
t1791=pick(2);
X t1797=TO_N(4); // ICONST
save(t1797);
X t1799=TO_N(5); // ICONST
save(t1799);
X t1800=select1(TO_N(1),t1791); // SELECT (simple)
t1799=restore();
X t1798=select1(t1799,t1800); // SELECT
t1797=restore();
X t1796=select1(t1797,t1798); // SELECT
t1783=top();
S_DATA(t1783)[4]=t1796;
t1783=restore();
restore();
t1773=top();
S_DATA(t1773)[3]=t1783;
t1773=restore();
restore();
t1772=top();
S_DATA(t1772)[0]=t1773;
t1773=pick(2);
save(t1773);
X t1801=allocate(3); // CONS
save(t1801);
t1773=pick(2);
X t1802=lf[159]; // CONST
t1801=top();
S_DATA(t1801)[0]=t1802;
t1802=pick(2);
X t1804=TO_N(2); // ICONST
save(t1804);
X t1805=select1(TO_N(1),t1802); // SELECT (simple)
t1804=restore();
X t1803=select1(t1804,t1805); // SELECT
t1801=top();
S_DATA(t1801)[1]=t1803;
t1803=pick(2);
X t1806=select1(TO_N(2),t1803); // SELECT (simple)
t1801=top();
S_DATA(t1801)[2]=t1806;
t1801=restore();
restore();
X t1807=___rewrite_3alog(t1801); // REF: rewrite:log
t1772=top();
S_DATA(t1772)[1]=t1807;
t1772=restore();
restore();
x=t1772; // REF: rewrite:rewrite1
goto loop; // tail call: rewrite:rewrite1
X t1808;
t1744=t1808;
}else{
save(t1693); // COND
save(t1693); // OR
X t1811=select1(TO_N(1),t1693); // SELECT (simple)
X t1812=___constnode(t1811); // REF: constnode
X t1810=t1812;
t1693=restore();
if(t1810==F){
X t1814=TO_N(2); // ICONST
save(t1814);
X t1815=select1(TO_N(1),t1693); // SELECT (simple)
X t1816=___len(t1815); // REF: len
t1814=restore();
X t1813=(t1814==t1816)||eq1(t1814,t1816)?T:F; // EQ
t1810=t1813;}
X t1809;
t1693=restore();
if(t1810!=F){
X t1817=___id(t1693); // REF: id
t1809=t1817;
}else{
save(t1693);
X t1818=allocate(2); // CONS
save(t1818);
t1693=pick(2);
X t1819=select1(TO_N(1),t1693); // SELECT (simple)
t1818=top();
S_DATA(t1818)[0]=t1819;
t1819=pick(2);
save(t1819);
X t1820=allocate(2); // CONS
save(t1820);
t1819=pick(2);
save(t1819);
X t1821=allocate(2); // CONS
save(t1821);
t1819=pick(2);
X t1822=select1(TO_N(2),t1819); // SELECT (simple)
t1821=top();
S_DATA(t1821)[0]=t1822;
t1822=pick(2);
X t1823=EMPTY; // ICONST
t1821=top();
S_DATA(t1821)[1]=t1823;
t1821=restore();
restore();
t1820=top();
S_DATA(t1820)[0]=t1821;
t1821=pick(2);
X t1824=select1(TO_N(1),t1821); // SELECT (simple)
X t1825=___tl(t1824); // REF: tl
X t1826=___tl(t1825); // REF: tl
t1820=top();
S_DATA(t1820)[1]=t1826;
t1820=restore();
restore();
X t1827=___al(t1820); // REF: al
int t1830; // INSERTL
check_S(t1827,"/");
int t1831=S_LENGTH(t1827);
X t1828=t1831==0?fail("no unit value"):S_DATA(t1827)[0];
save(t1827);
for(t1830=1;t1830<t1831;++t1830){
X t1829=sequence(2,t1828,S_DATA(pick(1))[t1830]);
save(t1829);
X t1832=allocate(2); // CONS
save(t1832);
t1829=pick(2);
X t1834=TO_N(2); // ICONST
save(t1834);
X t1835=select1(TO_N(1),t1829); // SELECT (simple)
t1834=restore();
X t1833=select1(t1834,t1835); // SELECT
t1832=top();
S_DATA(t1832)[0]=t1833;
t1833=pick(2);
save(t1833);
X t1836=allocate(2); // CONS
save(t1836);
t1833=pick(2);
X t1837=select1(TO_N(2),t1833); // SELECT (simple)
t1836=top();
S_DATA(t1836)[0]=t1837;
t1837=pick(2);
X t1839=TO_N(1); // ICONST
save(t1839);
X t1840=select1(TO_N(1),t1837); // SELECT (simple)
t1839=restore();
X t1838=select1(t1839,t1840); // SELECT
t1836=top();
S_DATA(t1836)[1]=t1838;
t1836=restore();
restore();
X t1841=___rewrite_3arewrite1(t1836); // REF: rewrite:rewrite1
t1832=top();
S_DATA(t1832)[1]=t1841;
t1832=restore();
restore();
save(t1832);
X t1842=allocate(2); // CONS
save(t1842);
t1832=pick(2);
X t1844=TO_N(2); // ICONST
save(t1844);
X t1845=select1(TO_N(2),t1832); // SELECT (simple)
t1844=restore();
X t1843=select1(t1844,t1845); // SELECT
t1842=top();
S_DATA(t1842)[0]=t1843;
t1843=pick(2);
save(t1843);
X t1846=allocate(2); // CONS
save(t1846);
t1843=pick(2);
X t1847=select1(TO_N(1),t1843); // SELECT (simple)
t1846=top();
S_DATA(t1846)[0]=t1847;
t1847=pick(2);
X t1849=TO_N(1); // ICONST
save(t1849);
X t1850=select1(TO_N(2),t1847); // SELECT (simple)
t1849=restore();
X t1848=select1(t1849,t1850); // SELECT
t1846=top();
S_DATA(t1846)[1]=t1848;
t1846=restore();
restore();
X t1851=___ar(t1846); // REF: ar
t1842=top();
S_DATA(t1842)[1]=t1851;
t1842=restore();
restore();
t1828=t1842;}
restore();
t1818=top();
S_DATA(t1818)[1]=t1828;
t1818=restore();
restore();
save(t1818);
X t1852=allocate(2); // CONS
save(t1852);
t1818=pick(2);
save(t1818);
X t1853=allocate(2); // CONS
save(t1853);
t1818=pick(2);
save(t1818);
X t1854=allocate(2); // CONS
save(t1854);
t1818=pick(2);
X t1856=TO_N(1); // ICONST
save(t1856);
X t1857=select1(TO_N(1),t1818); // SELECT (simple)
t1856=restore();
X t1855=select1(t1856,t1857); // SELECT
t1854=top();
S_DATA(t1854)[0]=t1855;
t1855=pick(2);
X t1859=TO_N(2); // ICONST
save(t1859);
X t1860=select1(TO_N(1),t1855); // SELECT (simple)
t1859=restore();
X t1858=select1(t1859,t1860); // SELECT
t1854=top();
S_DATA(t1854)[1]=t1858;
t1854=restore();
restore();
t1853=top();
S_DATA(t1853)[0]=t1854;
t1854=pick(2);
X t1862=TO_N(2); // ICONST
save(t1862);
X t1863=select1(TO_N(2),t1854); // SELECT (simple)
t1862=restore();
X t1861=select1(t1862,t1863); // SELECT
t1853=top();
S_DATA(t1853)[1]=t1861;
t1853=restore();
restore();
X t1864=___cat(t1853); // REF: cat
t1852=top();
S_DATA(t1852)[0]=t1864;
t1864=pick(2);
X t1866=TO_N(1); // ICONST
save(t1866);
X t1867=select1(TO_N(2),t1864); // SELECT (simple)
t1866=restore();
X t1865=select1(t1866,t1867); // SELECT
t1852=top();
S_DATA(t1852)[1]=t1865;
t1852=restore();
restore();
t1809=t1852;}
t1744=t1809;}
t1713=t1744;}
t1644=t1713;}
t1607=t1644;}
t1584=t1607;}
t1550=t1584;}
t1483=t1550;}
t1423=t1483;}
t1381=t1423;}
t1339=t1381;}
t1221=t1339;}
t1130=t1221;}
t1048=t1130;}
t930=t1048;}
t831=t930;}
t804=t831;}
t738=t804;}
t691=t738;}
t624=t691;}
t571=t624;}
t537=t571;}
t487=t537;}
t449=t487;}
t412=t449;}
t347=t412;}
t299=t347;}
t241=t299;}
t200=t241;}
t142=t200;}
t101=t142;}
t55=t101;}
t28=t55;}
t1=t28;}
RETURN(t1);}
//---------------------------------------- literals:extract1 (c-literals.fp:23)
DEFINE(___literals_3aextract1){
ENTRY;
loop:;
tracecall("c-literals.fp:23:  literals:extract1");
save(x); // COND
save(x);
X t2=allocate(2); // CONS
save(t2);
x=pick(2);
X t3=select1(TO_N(1),x); // SELECT (simple)
t2=top();
S_DATA(t2)[0]=t3;
t3=pick(2);
X t4=lf[160]; // CONST
t2=top();
S_DATA(t2)[1]=t4;
t2=restore();
restore();
X t5=___index(t2); // REF: index
X t1;
x=restore();
if(t5!=F){
X t6=EMPTY; // ICONST
t1=t6;
}else{
save(x); // COND
X t9=lf[290]; // CONST
save(t9);
X t10=select1(TO_N(1),x); // SELECT (simple)
t9=restore();
X t8=(t9==t10)||eq1(t9,t10)?T:F; // EQ
X t7;
x=restore();
if(t8!=F){
save(x);
X t11=allocate(1); // CONS
save(t11);
x=pick(2);
X t12=select1(TO_N(3),x); // SELECT (simple)
t11=top();
S_DATA(t11)[0]=t12;
t11=restore();
restore();
t7=t11;
}else{
X t13=___tl(x); // REF: tl
X t14=___tl(t13); // REF: tl
int t16; // ALPHA
check_S(t14,"@");
int t17=S_LENGTH(t14);
save(t14);
X t15=allocate(t17);
save(t15);
for(t16=0;t16<t17;++t16){
X t15=S_DATA(pick(2))[t16];
X t18=___literals_3aextract1(t15); // REF: literals:extract1
S_DATA(top())[t16]=t18;}
t15=restore();
restore();
X t19=___cat(t15); // REF: cat
t7=t19;}
t1=t7;}
RETURN(t1);}
//---------------------------------------- literals:walk (c-literals.fp:32)
DEFINE(___literals_3awalk){
ENTRY;
loop:;
tracecall("c-literals.fp:32:  literals:walk");
save(x); // COND
X t2=select1(TO_N(1),x); // SELECT (simple)
save(t2);
X t3=allocate(2); // CONS
save(t3);
t2=pick(2);
X t4=select1(TO_N(1),t2); // SELECT (simple)
t3=top();
S_DATA(t3)[0]=t4;
t4=pick(2);
X t5=lf[160]; // CONST
t3=top();
S_DATA(t3)[1]=t5;
t3=restore();
restore();
X t6=___index(t3); // REF: index
X t1;
x=restore();
if(t6!=F){
X t7=select1(TO_N(1),x); // SELECT (simple)
t1=t7;
}else{
save(x); // COND
save(x);
X t11=TO_N(1); // ICONST
save(t11);
X t12=select1(TO_N(1),x); // SELECT (simple)
t11=restore();
X t10=select1(t11,t12); // SELECT
x=restore();
X t13=lf[290]; // CONST
X t9=(t10==t13)||eq1(t10,t13)?T:F; // EQ
X t8;
x=restore();
if(t9!=F){
save(x);
X t14=allocate(3); // CONS
save(t14);
x=pick(2);
X t15=lf[290]; // CONST
t14=top();
S_DATA(t14)[0]=t15;
t15=pick(2);
X t17=TO_N(2); // ICONST
save(t17);
X t18=select1(TO_N(1),t15); // SELECT (simple)
t17=restore();
X t16=select1(t17,t18); // SELECT
t14=top();
S_DATA(t14)[1]=t16;
t16=pick(2);
save(t16);
X t19=allocate(2); // CONS
save(t19);
t16=pick(2);
X t21=TO_N(3); // ICONST
save(t21);
X t22=select1(TO_N(1),t16); // SELECT (simple)
t21=restore();
X t20=select1(t21,t22); // SELECT
t19=top();
S_DATA(t19)[0]=t20;
t20=pick(2);
X t23=select1(TO_N(2),t20); // SELECT (simple)
t19=top();
S_DATA(t19)[1]=t23;
t19=restore();
restore();
X t24=___index(t19); // REF: index
save(t24);
X t25=allocate(2); // CONS
save(t25);
t24=pick(2);
X t26=___id(t24); // REF: id
t25=top();
S_DATA(t25)[0]=t26;
t26=pick(2);
X t27=TO_N(1); // ICONST
t25=top();
S_DATA(t25)[1]=t27;
t25=restore();
restore();
X t28=___sub(t25); // REF: sub
t14=top();
S_DATA(t14)[2]=t28;
t14=restore();
restore();
t8=t14;
}else{
save(x);
X t29=allocate(2); // CONS
save(t29);
x=pick(2);
X t30=select1(TO_N(1),x); // SELECT (simple)
t29=top();
S_DATA(t29)[0]=t30;
t30=pick(2);
save(t30);
X t31=allocate(2); // CONS
save(t31);
t30=pick(2);
X t32=select1(TO_N(1),t30); // SELECT (simple)
X t33=___tl(t32); // REF: tl
X t34=___tl(t33); // REF: tl
t31=top();
S_DATA(t31)[0]=t34;
t34=pick(2);
X t35=select1(TO_N(2),t34); // SELECT (simple)
t31=top();
S_DATA(t31)[1]=t35;
t31=restore();
restore();
X t36=___dr(t31); // REF: dr
int t38; // ALPHA
check_S(t36,"@");
int t39=S_LENGTH(t36);
save(t36);
X t37=allocate(t39);
save(t37);
for(t38=0;t38<t39;++t38){
X t37=S_DATA(pick(2))[t38];
X t40=___literals_3awalk(t37); // REF: literals:walk
S_DATA(top())[t38]=t40;}
t37=restore();
restore();
t29=top();
S_DATA(t29)[1]=t37;
t29=restore();
restore();
save(t29);
X t41=allocate(2); // CONS
save(t41);
t29=pick(2);
X t42=select1(TO_N(1),t29); // SELECT (simple)
save(t42);
X t43=allocate(2); // CONS
save(t43);
t42=pick(2);
X t44=select1(TO_N(1),t42); // SELECT (simple)
t43=top();
S_DATA(t43)[0]=t44;
t44=pick(2);
X t45=select1(TO_N(2),t44); // SELECT (simple)
t43=top();
S_DATA(t43)[1]=t45;
t43=restore();
restore();
t41=top();
S_DATA(t41)[0]=t43;
t43=pick(2);
X t46=select1(TO_N(2),t43); // SELECT (simple)
t41=top();
S_DATA(t41)[1]=t46;
t41=restore();
restore();
X t47=___cat(t41); // REF: cat
t8=t47;}
t1=t8;}
RETURN(t1);}
//---------------------------------------- literals:construct (c-literals.fp:41)
DEFINE(___literals_3aconstruct){
ENTRY;
loop:;
tracecall("c-literals.fp:41:  literals:construct");
save(x); // COND
X t2=___num(x); // REF: num
X t1;
x=restore();
if(t2!=F){
save(x);
X t3=allocate(3); // CONS
save(t3);
x=pick(2);
X t4=lf[177]; // CONST
t3=top();
S_DATA(t3)[0]=t4;
t4=pick(2);
X t5=___tos(t4); // REF: tos
t3=top();
S_DATA(t3)[1]=t5;
t5=pick(2);
X t6=lf[284]; // CONST
t3=top();
S_DATA(t3)[2]=t6;
t3=restore();
restore();
t1=t3;
}else{
save(x); // COND
X t8=___atom(x); // REF: atom
X t7;
x=restore();
if(t8!=F){
save(x);
X t9=allocate(5); // CONS
save(t9);
x=pick(2);
X t10=lf[161]; // CONST
t9=top();
S_DATA(t9)[0]=t10;
t10=pick(2);
X t11=lf[206]; // CONST
t9=top();
S_DATA(t9)[1]=t11;
t11=pick(2);
X t12=___tos(t11); // REF: tos
int t14; // ALPHA
check_S(t12,"@");
int t15=S_LENGTH(t12);
save(t12);
X t13=allocate(t15);
save(t13);
for(t14=0;t14<t15;++t14){
X t13=S_DATA(pick(2))[t14];
save(t13); // COND
save(t13);
X t17=allocate(2); // CONS
save(t17);
t13=pick(2);
X t18=___id(t13); // REF: id
t17=top();
S_DATA(t17)[0]=t18;
t18=pick(2);
X t19=lf[162]; // CONST
t17=top();
S_DATA(t17)[1]=t19;
t17=restore();
restore();
X t20=___index(t17); // REF: index
X t16;
t13=restore();
if(t20!=F){
save(t13);
X t21=allocate(2); // CONS
save(t21);
t13=pick(2);
X t22=TO_N(92); // ICONST
t21=top();
S_DATA(t21)[0]=t22;
t22=pick(2);
X t23=___id(t22); // REF: id
t21=top();
S_DATA(t21)[1]=t23;
t21=restore();
restore();
t16=t21;
}else{
save(t13);
X t24=allocate(1); // CONS
save(t24);
t13=pick(2);
X t25=___id(t13); // REF: id
t24=top();
S_DATA(t24)[0]=t25;
t24=restore();
restore();
t16=t24;}
S_DATA(top())[t14]=t16;}
t13=restore();
restore();
X t26=___cat(t13); // REF: cat
t9=top();
S_DATA(t9)[2]=t26;
t26=pick(2);
X t27=lf[206]; // CONST
t9=top();
S_DATA(t9)[3]=t27;
t27=pick(2);
X t28=lf[284]; // CONST
t9=top();
S_DATA(t9)[4]=t28;
t9=restore();
restore();
t7=t9;
}else{
save(x);
X t29=allocate(4); // CONS
save(t29);
x=pick(2);
X t30=lf[163]; // CONST
t29=top();
S_DATA(t29)[0]=t30;
t30=pick(2);
X t31=___len(t30); // REF: len
X t32=___tos(t31); // REF: tos
t29=top();
S_DATA(t29)[1]=t32;
t32=pick(2);
int t34; // ALPHA
check_S(t32,"@");
int t35=S_LENGTH(t32);
save(t32);
X t33=allocate(t35);
save(t33);
for(t34=0;t34<t35;++t34){
X t33=S_DATA(pick(2))[t34];
save(t33);
X t36=allocate(2); // CONS
save(t36);
t33=pick(2);
X t37=TO_N(44); // ICONST
t36=top();
S_DATA(t36)[0]=t37;
t37=pick(2);
X t38=___literals_3aconstruct(t37); // REF: literals:construct
t36=top();
S_DATA(t36)[1]=t38;
t36=restore();
restore();
X t39=___al(t36); // REF: al
S_DATA(top())[t34]=t39;}
t33=restore();
restore();
X t40=___cat(t33); // REF: cat
t29=top();
S_DATA(t29)[2]=t40;
t40=pick(2);
X t41=lf[284]; // CONST
t29=top();
S_DATA(t29)[3]=t41;
t29=restore();
restore();
t7=t29;}
t1=t7;}
X t42=___cat(t1); // REF: cat
RETURN(t42);}
//---------------------------------------- backend:t_expr (c-backend.fp:71)
DEFINE(___backend_3at_5fexpr){
ENTRY;
loop:;
tracecall("c-backend.fp:71:  backend:t_expr");
save(x); // COND
X t3=lf[320]; // CONST
save(t3);
X t5=TO_N(1); // ICONST
save(t5);
X t6=select1(TO_N(1),x); // SELECT (simple)
t5=restore();
X t4=select1(t5,t6); // SELECT
t3=restore();
X t2=(t3==t4)||eq1(t3,t4)?T:F; // EQ
X t1;
x=restore();
if(t2!=F){
save(x); // COND
X t8=select1(TO_N(1),x); // SELECT (simple)
X t9=F;
save(t8); // PCONS
if(!IS_S(t8)||S_LENGTH(t8)!=4) goto t10;
t8=S_DATA(top())[0];
X t12=lf[320]; // CONST
save(t12);
X t13=___id(t8); // REF: id
t12=restore();
X t11=(t12==t13)||eq1(t12,t13)?T:F; // EQ
if(t11==F) goto t10;
// skipped
t11=S_DATA(top())[2];
X t14=F;
save(t11); // PCONS
if(!IS_S(t11)||S_LENGTH(t11)!=3) goto t15;
t11=S_DATA(top())[0];
X t17=lf[293]; // CONST
save(t17);
X t18=___id(t11); // REF: id
t17=restore();
X t16=(t17==t18)||eq1(t17,t18)?T:F; // EQ
if(t16==F) goto t15;
// skipped
t16=S_DATA(top())[2];
save(t16);
X t19=allocate(2); // CONS
save(t19);
t16=pick(2);
X t20=___id(t16); // REF: id
t19=top();
S_DATA(t19)[0]=t20;
t20=pick(2);
X t21=lf[164]; // CONST
t19=top();
S_DATA(t19)[1]=t21;
t19=restore();
restore();
X t22=___index(t19); // REF: index
if(t22==F) goto t15;
t14=T;
t15:
restore();
if(t14==F) goto t10;
t14=S_DATA(top())[3];
X t23=F;
save(t14); // PCONS
if(!IS_S(t14)||S_LENGTH(t14)!=4) goto t24;
t14=S_DATA(top())[0];
X t26=lf[326]; // CONST
save(t26);
X t27=___id(t14); // REF: id
t26=restore();
X t25=(t26==t27)||eq1(t26,t27)?T:F; // EQ
if(t25==F) goto t24;
// skipped
// skipped
// skipped
t23=T;
t24:
restore();
if(t23==F) goto t10;
t9=T;
t10:
restore();
X t7;
x=restore();
if(t9!=F){
save(x);
X t28=allocate(2); // CONS
save(t28);
x=pick(2);
X t30=TO_N(3); // ICONST
save(t30);
X t32=TO_N(3); // ICONST
save(t32);
X t33=select1(TO_N(1),x); // SELECT (simple)
t32=restore();
X t31=select1(t32,t33); // SELECT
t30=restore();
X t29=select1(t30,t31); // SELECT
t28=top();
S_DATA(t28)[0]=t29;
t29=pick(2);
save(t29);
X t34=allocate(2); // CONS
save(t34);
t29=pick(2);
X t36=TO_N(4); // ICONST
save(t36);
X t37=select1(TO_N(1),t29); // SELECT (simple)
t36=restore();
X t35=select1(t36,t37); // SELECT
t34=top();
S_DATA(t34)[0]=t35;
t35=pick(2);
X t38=select1(TO_N(2),t35); // SELECT (simple)
t34=top();
S_DATA(t34)[1]=t38;
t34=restore();
restore();
X t39=___backend_3atwo_5farg_5ffun(t34); // REF: backend:two_arg_fun
t28=top();
S_DATA(t28)[1]=t39;
t28=restore();
restore();
save(t28);
X t40=allocate(3); // CONS
save(t40);
t28=pick(2);
save(t28);
X t41=allocate(11); // CONS
save(t41);
t28=pick(2);
X t42=lf[280]; // CONST
t41=top();
S_DATA(t41)[0]=t42;
t42=pick(2);
X t43=lf[262]; // CONST
t41=top();
S_DATA(t41)[1]=t43;
t43=pick(2);
X t45=TO_N(2); // ICONST
save(t45);
X t46=select1(TO_N(2),t43); // SELECT (simple)
t45=restore();
X t44=select1(t45,t46); // SELECT
t41=top();
S_DATA(t41)[2]=t44;
t44=pick(2);
X t47=lf[165]; // CONST
t41=top();
S_DATA(t41)[3]=t47;
t47=pick(2);
X t48=select1(TO_N(1),t47); // SELECT (simple)
X t49=___tos(t48); // REF: tos
t41=top();
S_DATA(t41)[4]=t49;
t49=pick(2);
X t50=lf[166]; // CONST
t41=top();
S_DATA(t41)[5]=t50;
t50=pick(2);
X t52=TO_N(1); // ICONST
save(t52);
X t54=TO_N(3); // ICONST
save(t54);
X t55=select1(TO_N(2),t50); // SELECT (simple)
t54=restore();
X t53=select1(t54,t55); // SELECT
t52=restore();
X t51=select1(t52,t53); // SELECT
t41=top();
S_DATA(t41)[6]=t51;
t51=pick(2);
X t56=lf[252]; // CONST
t41=top();
S_DATA(t41)[7]=t56;
t56=pick(2);
X t58=TO_N(1); // ICONST
save(t58);
X t60=TO_N(4); // ICONST
save(t60);
X t61=select1(TO_N(2),t56); // SELECT (simple)
t60=restore();
X t59=select1(t60,t61); // SELECT
t58=restore();
X t57=select1(t58,t59); // SELECT
t41=top();
S_DATA(t41)[8]=t57;
t57=pick(2);
X t62=lf[167]; // CONST
t41=top();
S_DATA(t41)[9]=t62;
t62=pick(2);
X t63=select1(TO_N(1),t62); // SELECT (simple)
X t64=___tos(t63); // REF: tos
t41=top();
S_DATA(t41)[10]=t64;
t41=restore();
restore();
t40=top();
S_DATA(t40)[0]=t41;
t41=pick(2);
X t66=TO_N(2); // ICONST
save(t66);
X t67=select1(TO_N(2),t41); // SELECT (simple)
t66=restore();
X t65=select1(t66,t67); // SELECT
t40=top();
S_DATA(t40)[1]=t65;
t65=pick(2);
X t69=TO_N(4); // ICONST
save(t69);
X t70=select1(TO_N(2),t65); // SELECT (simple)
t69=restore();
X t68=select1(t69,t70); // SELECT
t40=top();
S_DATA(t40)[2]=t68;
t40=restore();
restore();
X t71=___backend_3aconcx(t40); // REF: backend:concx
t7=t71;
}else{
save(x);
X t72=allocate(3); // CONS
save(t72);
x=pick(2);
X t73=select1(TO_N(1),x); // SELECT (simple)
t72=top();
S_DATA(t72)[0]=t73;
t73=pick(2);
X t75=TO_N(3); // ICONST
save(t75);
X t76=select1(TO_N(2),t73); // SELECT (simple)
t75=restore();
X t74=select1(t75,t76); // SELECT
t72=top();
S_DATA(t72)[1]=t74;
t74=pick(2);
save(t74);
X t77=allocate(2); // CONS
save(t77);
t74=pick(2);
X t79=TO_N(4); // ICONST
save(t79);
X t80=select1(TO_N(1),t74); // SELECT (simple)
t79=restore();
X t78=select1(t79,t80); // SELECT
t77=top();
S_DATA(t77)[0]=t78;
t78=pick(2);
X t81=select1(TO_N(2),t78); // SELECT (simple)
X t82=___backend_3anontail(t81); // REF: backend:nontail
t77=top();
S_DATA(t77)[1]=t82;
t77=restore();
restore();
X t83=___backend_3at_5fexpr(t77); // REF: backend:t_expr
t72=top();
S_DATA(t72)[2]=t83;
t72=restore();
restore();
save(t72);
X t84=allocate(2); // CONS
save(t84);
t72=pick(2);
X t86=TO_N(3); // ICONST
save(t86);
X t87=select1(TO_N(1),t72); // SELECT (simple)
t86=restore();
X t85=select1(t86,t87); // SELECT
t84=top();
S_DATA(t84)[0]=t85;
t85=pick(2);
save(t85);
X t88=allocate(2); // CONS
save(t88);
t85=pick(2);
X t89=select1(TO_N(2),t85); // SELECT (simple)
t88=top();
S_DATA(t88)[0]=t89;
t89=pick(2);
X t90=select1(TO_N(3),t89); // SELECT (simple)
t88=top();
S_DATA(t88)[1]=t90;
t88=restore();
restore();
X t91=___backend_3atail(t88); // REF: backend:tail
t84=top();
S_DATA(t84)[1]=t91;
t84=restore();
restore();
x=t84; // REF: backend:t_expr
goto loop; // tail call: backend:t_expr
X t92;
t7=t92;}
t1=t7;
}else{
save(x); // COND
X t95=lf[287]; // CONST
save(t95);
X t97=TO_N(1); // ICONST
save(t97);
X t98=select1(TO_N(1),x); // SELECT (simple)
t97=restore();
X t96=select1(t97,t98); // SELECT
t95=restore();
X t94=(t95==t96)||eq1(t95,t96)?T:F; // EQ
X t93;
x=restore();
if(t94!=F){
save(x);
X t99=allocate(2); // CONS
save(t99);
x=pick(2);
X t100=select1(TO_N(1),x); // SELECT (simple)
t99=top();
S_DATA(t99)[0]=t100;
t100=pick(2);
X t101=select1(TO_N(2),t100); // SELECT (simple)
X t102=___backend_3agensym(t101); // REF: backend:gensym
t99=top();
S_DATA(t99)[1]=t102;
t99=restore();
restore();
save(t99);
X t103=allocate(4); // CONS
save(t103);
t99=pick(2);
X t104=select1(TO_N(1),t99); // SELECT (simple)
t103=top();
S_DATA(t103)[0]=t104;
t104=pick(2);
X t106=TO_N(1); // ICONST
save(t106);
X t107=select1(TO_N(2),t104); // SELECT (simple)
t106=restore();
X t105=select1(t106,t107); // SELECT
t103=top();
S_DATA(t103)[1]=t105;
t105=pick(2);
X t109=TO_N(2); // ICONST
save(t109);
X t110=select1(TO_N(2),t105); // SELECT (simple)
t109=restore();
X t108=select1(t109,t110); // SELECT
t103=top();
S_DATA(t103)[2]=t108;
t108=pick(2);
save(t108);
X t111=allocate(2); // CONS
save(t111);
t108=pick(2);
X t113=TO_N(3); // ICONST
save(t113);
X t114=select1(TO_N(1),t108); // SELECT (simple)
t113=restore();
X t112=select1(t113,t114); // SELECT
t111=top();
S_DATA(t111)[0]=t112;
t112=pick(2);
save(t112);
X t115=allocate(2); // CONS
save(t115);
t112=pick(2);
save(t112);
X t116=allocate(4); // CONS
save(t116);
t112=pick(2);
X t117=lf[280]; // CONST
t116=top();
S_DATA(t116)[0]=t117;
t117=pick(2);
X t118=lf[271]; // CONST
t116=top();
S_DATA(t116)[1]=t118;
t118=pick(2);
X t120=TO_N(1); // ICONST
save(t120);
X t122=TO_N(2); // ICONST
save(t122);
X t123=select1(TO_N(2),t118); // SELECT (simple)
t122=restore();
X t121=select1(t122,t123); // SELECT
t120=restore();
X t119=select1(t120,t121); // SELECT
t116=top();
S_DATA(t116)[2]=t119;
t119=pick(2);
X t124=lf[170]; // CONST
t116=top();
S_DATA(t116)[3]=t124;
t116=restore();
restore();
t115=top();
S_DATA(t115)[0]=t116;
t116=pick(2);
X t126=TO_N(2); // ICONST
save(t126);
X t127=select1(TO_N(2),t116); // SELECT (simple)
t126=restore();
X t125=select1(t126,t127); // SELECT
t115=top();
S_DATA(t115)[1]=t125;
t115=restore();
restore();
X t128=___backend_3aconc(t115); // REF: backend:conc
X t129=___backend_3anontail(t128); // REF: backend:nontail
t111=top();
S_DATA(t111)[1]=t129;
t111=restore();
restore();
X t130=___backend_3at_5fexpr(t111); // REF: backend:t_expr
t103=top();
S_DATA(t103)[3]=t130;
t103=restore();
restore();
save(t103);
X t131=allocate(4); // CONS
save(t131);
t103=pick(2);
X t132=select1(TO_N(1),t103); // SELECT (simple)
t131=top();
S_DATA(t131)[0]=t132;
t132=pick(2);
X t133=select1(TO_N(2),t132); // SELECT (simple)
t131=top();
S_DATA(t131)[1]=t133;
t133=pick(2);
X t134=select1(TO_N(3),t133); // SELECT (simple)
t131=top();
S_DATA(t131)[2]=t134;
t134=pick(2);
save(t134);
X t135=allocate(2); // CONS
save(t135);
t134=pick(2);
X t137=TO_N(4); // ICONST
save(t137);
X t138=select1(TO_N(1),t134); // SELECT (simple)
t137=restore();
X t136=select1(t137,t138); // SELECT
t135=top();
S_DATA(t135)[0]=t136;
t136=pick(2);
save(t136);
X t139=allocate(3); // CONS
save(t139);
t136=pick(2);
save(t136);
X t140=allocate(11); // CONS
save(t140);
t136=pick(2);
X t141=lf[280]; // CONST
t140=top();
S_DATA(t140)[0]=t141;
t141=pick(2);
X t142=lf[262]; // CONST
t140=top();
S_DATA(t140)[1]=t142;
t142=pick(2);
X t143=select1(TO_N(2),t142); // SELECT (simple)
t140=top();
S_DATA(t140)[2]=t143;
t143=pick(2);
X t144=lf[288]; // CONST
t140=top();
S_DATA(t140)[3]=t144;
t144=pick(2);
X t145=lf[280]; // CONST
t140=top();
S_DATA(t140)[4]=t145;
t145=pick(2);
X t147=TO_N(1); // ICONST
save(t147);
X t148=select1(TO_N(3),t145); // SELECT (simple)
t147=restore();
X t146=select1(t147,t148); // SELECT
t140=top();
S_DATA(t140)[5]=t146;
t146=pick(2);
X t149=lf[273]; // CONST
t140=top();
S_DATA(t140)[6]=t149;
t149=pick(2);
X t150=lf[280]; // CONST
t140=top();
S_DATA(t140)[7]=t150;
t150=pick(2);
X t151=lf[258]; // CONST
t140=top();
S_DATA(t140)[8]=t151;
t151=pick(2);
X t153=TO_N(1); // ICONST
save(t153);
X t154=select1(TO_N(4),t151); // SELECT (simple)
t153=restore();
X t152=select1(t153,t154); // SELECT
t140=top();
S_DATA(t140)[9]=t152;
t152=pick(2);
X t155=lf[169]; // CONST
t140=top();
S_DATA(t140)[10]=t155;
t140=restore();
restore();
t139=top();
S_DATA(t139)[0]=t140;
t140=pick(2);
X t157=TO_N(1); // ICONST
save(t157);
X t158=select1(TO_N(3),t140); // SELECT (simple)
t157=restore();
X t156=select1(t157,t158); // SELECT
t139=top();
S_DATA(t139)[1]=t156;
t156=pick(2);
save(t156);
X t159=allocate(2); // CONS
save(t159);
t156=pick(2);
X t161=TO_N(3); // ICONST
save(t161);
X t162=select1(TO_N(3),t156); // SELECT (simple)
t161=restore();
X t160=select1(t161,t162); // SELECT
t159=top();
S_DATA(t159)[0]=t160;
t160=pick(2);
X t163=select1(TO_N(4),t160); // SELECT (simple)
t159=top();
S_DATA(t159)[1]=t163;
t159=restore();
restore();
X t164=___backend_3atail(t159); // REF: backend:tail
t139=top();
S_DATA(t139)[2]=t164;
t139=restore();
restore();
X t165=___backend_3aconcx(t139); // REF: backend:concx
t135=top();
S_DATA(t135)[1]=t165;
t135=restore();
restore();
X t166=___backend_3at_5fexpr(t135); // REF: backend:t_expr
t131=top();
S_DATA(t131)[3]=t166;
t131=restore();
restore();
save(t131);
X t167=allocate(3); // CONS
save(t167);
t131=pick(2);
X t168=select1(TO_N(1),t131); // SELECT (simple)
t167=top();
S_DATA(t167)[0]=t168;
t168=pick(2);
X t169=select1(TO_N(2),t168); // SELECT (simple)
t167=top();
S_DATA(t167)[1]=t169;
t169=pick(2);
save(t169);
X t170=allocate(2); // CONS
save(t170);
t169=pick(2);
X t171=select1(TO_N(1),t169); // SELECT (simple)
X t173=TO_N(1); // ICONST
save(t173);
save(t171);
X t174=allocate(2); // CONS
save(t174);
t171=pick(2);
X t175=TO_N(4); // ICONST
t174=top();
S_DATA(t174)[0]=t175;
t175=pick(2);
X t176=___id(t175); // REF: id
t174=top();
S_DATA(t174)[1]=t176;
t174=restore();
restore();
X t177=___drop(t174); // REF: drop
t173=restore();
X t172=select1(t173,t177); // SELECT
t170=top();
S_DATA(t170)[0]=t172;
t172=pick(2);
save(t172);
X t178=allocate(3); // CONS
save(t178);
t172=pick(2);
save(t172);
X t179=allocate(7); // CONS
save(t179);
t172=pick(2);
X t180=lf[280]; // CONST
t179=top();
S_DATA(t179)[0]=t180;
t180=pick(2);
X t181=select1(TO_N(2),t180); // SELECT (simple)
t179=top();
S_DATA(t179)[1]=t181;
t181=pick(2);
X t182=lf[243]; // CONST
t179=top();
S_DATA(t179)[2]=t182;
t182=pick(2);
X t184=TO_N(1); // ICONST
save(t184);
X t185=select1(TO_N(4),t182); // SELECT (simple)
t184=restore();
X t183=select1(t184,t185); // SELECT
t179=top();
S_DATA(t179)[3]=t183;
t183=pick(2);
X t186=lf[288]; // CONST
t179=top();
S_DATA(t179)[4]=t186;
t186=pick(2);
X t187=lf[280]; // CONST
t179=top();
S_DATA(t179)[5]=t187;
t187=pick(2);
X t188=lf[168]; // CONST
t179=top();
S_DATA(t179)[6]=t188;
t179=restore();
restore();
t178=top();
S_DATA(t178)[0]=t179;
t179=pick(2);
X t190=TO_N(1); // ICONST
save(t190);
X t191=select1(TO_N(3),t179); // SELECT (simple)
t190=restore();
X t189=select1(t190,t191); // SELECT
t178=top();
S_DATA(t178)[1]=t189;
t189=pick(2);
save(t189);
X t192=allocate(2); // CONS
save(t192);
t189=pick(2);
X t194=TO_N(3); // ICONST
save(t194);
X t195=select1(TO_N(3),t189); // SELECT (simple)
t194=restore();
X t193=select1(t194,t195); // SELECT
t192=top();
S_DATA(t192)[0]=t193;
t193=pick(2);
X t196=select1(TO_N(4),t193); // SELECT (simple)
t192=top();
S_DATA(t192)[1]=t196;
t192=restore();
restore();
X t197=___backend_3atail(t192); // REF: backend:tail
t178=top();
S_DATA(t178)[2]=t197;
t178=restore();
restore();
X t198=___backend_3aconcx(t178); // REF: backend:concx
t170=top();
S_DATA(t170)[1]=t198;
t170=restore();
restore();
X t199=___backend_3at_5fexpr(t170); // REF: backend:t_expr
t167=top();
S_DATA(t167)[2]=t199;
t167=restore();
restore();
save(t167);
X t200=allocate(3); // CONS
save(t200);
t167=pick(2);
save(t167);
X t201=allocate(5); // CONS
save(t201);
t167=pick(2);
X t202=lf[280]; // CONST
t201=top();
S_DATA(t201)[0]=t202;
t202=pick(2);
X t203=select1(TO_N(2),t202); // SELECT (simple)
t201=top();
S_DATA(t201)[1]=t203;
t203=pick(2);
X t204=lf[243]; // CONST
t201=top();
S_DATA(t201)[2]=t204;
t204=pick(2);
X t206=TO_N(1); // ICONST
save(t206);
X t207=select1(TO_N(3),t204); // SELECT (simple)
t206=restore();
X t205=select1(t206,t207); // SELECT
t201=top();
S_DATA(t201)[3]=t205;
t205=pick(2);
X t208=lf[229]; // CONST
t201=top();
S_DATA(t201)[4]=t208;
t201=restore();
restore();
t200=top();
S_DATA(t200)[0]=t201;
t201=pick(2);
X t209=select1(TO_N(2),t201); // SELECT (simple)
t200=top();
S_DATA(t200)[1]=t209;
t209=pick(2);
X t210=select1(TO_N(3),t209); // SELECT (simple)
t200=top();
S_DATA(t200)[2]=t210;
t200=restore();
restore();
X t211=___backend_3aconcx(t200); // REF: backend:concx
t93=t211;
}else{
save(x); // COND
X t214=lf[326]; // CONST
save(t214);
X t216=TO_N(1); // ICONST
save(t216);
X t217=select1(TO_N(1),x); // SELECT (simple)
t216=restore();
X t215=select1(t216,t217); // SELECT
t214=restore();
X t213=(t214==t215)||eq1(t214,t215)?T:F; // EQ
X t212;
x=restore();
if(t213!=F){
save(x);
X t218=allocate(2); // CONS
save(t218);
x=pick(2);
X t219=select1(TO_N(1),x); // SELECT (simple)
X t220=___tl(t219); // REF: tl
X t221=___tl(t220); // REF: tl
t218=top();
S_DATA(t218)[0]=t221;
t221=pick(2);
X t222=select1(TO_N(2),t221); // SELECT (simple)
X t223=___backend_3anontail(t222); // REF: backend:nontail
X t224=___backend_3agensym(t223); // REF: backend:gensym
t218=top();
S_DATA(t218)[1]=t224;
t218=restore();
restore();
X t225=___al(t218); // REF: al
save(t225);
X t226=allocate(3); // CONS
save(t226);
t225=pick(2);
X t227=select1(TO_N(1),t225); // SELECT (simple)
t226=top();
S_DATA(t226)[0]=t227;
t227=pick(2);
X t228=select1(TO_N(2),t227); // SELECT (simple)
t226=top();
S_DATA(t226)[1]=t228;
t228=pick(2);
save(t228);
X t229=allocate(2); // CONS
save(t229);
t228=pick(2);
save(t228);
X t230=allocate(14); // CONS
save(t230);
t228=pick(2);
X t231=lf[280]; // CONST
t230=top();
S_DATA(t230)[0]=t231;
t231=pick(2);
X t232=lf[271]; // CONST
t230=top();
S_DATA(t230)[1]=t232;
t232=pick(2);
X t234=TO_N(1); // ICONST
save(t234);
X t235=select1(TO_N(3),t232); // SELECT (simple)
t234=restore();
X t233=select1(t234,t235); // SELECT
t230=top();
S_DATA(t230)[2]=t233;
t233=pick(2);
X t236=lf[272]; // CONST
t230=top();
S_DATA(t230)[3]=t236;
t236=pick(2);
X t237=lf[280]; // CONST
t230=top();
S_DATA(t230)[4]=t237;
t237=pick(2);
X t238=lf[262]; // CONST
t230=top();
S_DATA(t230)[5]=t238;
t238=pick(2);
X t239=select1(TO_N(2),t238); // SELECT (simple)
t230=top();
S_DATA(t230)[6]=t239;
t239=pick(2);
X t240=lf[185]; // CONST
t230=top();
S_DATA(t230)[7]=t240;
t240=pick(2);
X t241=select1(TO_N(1),t240); // SELECT (simple)
X t242=___len(t241); // REF: len
X t243=___tos(t242); // REF: tos
t230=top();
S_DATA(t230)[8]=t243;
t243=pick(2);
X t244=lf[174]; // CONST
t230=top();
S_DATA(t230)[9]=t244;
t244=pick(2);
X t245=lf[280]; // CONST
t230=top();
S_DATA(t230)[10]=t245;
t245=pick(2);
X t246=lf[271]; // CONST
t230=top();
S_DATA(t230)[11]=t246;
t246=pick(2);
X t247=select1(TO_N(2),t246); // SELECT (simple)
t230=top();
S_DATA(t230)[12]=t247;
t247=pick(2);
X t248=lf[272]; // CONST
t230=top();
S_DATA(t230)[13]=t248;
t230=restore();
restore();
t229=top();
S_DATA(t229)[0]=t230;
t230=pick(2);
X t249=select1(TO_N(3),t230); // SELECT (simple)
t229=top();
S_DATA(t229)[1]=t249;
t229=restore();
restore();
X t250=___backend_3aconc(t229); // REF: backend:conc
t226=top();
S_DATA(t226)[2]=t250;
t226=restore();
restore();
save(t226);
X t251=allocate(2); // CONS
save(t251);
t226=pick(2);
X t252=select1(TO_N(2),t226); // SELECT (simple)
t251=top();
S_DATA(t251)[0]=t252;
t252=pick(2);
save(t252);
X t253=allocate(4); // CONS
save(t253);
t252=pick(2);
X t254=select1(TO_N(1),t252); // SELECT (simple)
t253=top();
S_DATA(t253)[0]=t254;
t254=pick(2);
X t255=select1(TO_N(2),t254); // SELECT (simple)
t253=top();
S_DATA(t253)[1]=t255;
t255=pick(2);
X t256=select1(TO_N(1),t255); // SELECT (simple)
X t257=___len(t256); // REF: len
X t258=___iota(t257); // REF: iota
t253=top();
S_DATA(t253)[2]=t258;
t258=pick(2);
X t259=select1(TO_N(3),t258); // SELECT (simple)
t253=top();
S_DATA(t253)[3]=t259;
t253=restore();
restore();
X t261=TO_N(4); // ICONST
save(t261);
X t262=t253; // WHILE
for(;;){
save(t262);
save(t262); // COND
X t265=lf[381]; // CONST
save(t265);
X t266=select1(TO_N(1),t262); // SELECT (simple)
t265=restore();
X t264=(t265==t266)||eq1(t265,t266)?T:F; // EQ
X t263;
t262=restore();
if(t264!=F){
X t267=F; // ICONST
t263=t267;
}else{
X t268=T; // ICONST
t263=t268;}
t262=restore();
if(t263==F) break;
save(t262);
X t269=allocate(4); // CONS
save(t269);
t262=pick(2);
X t270=select1(TO_N(1),t262); // SELECT (simple)
t269=top();
S_DATA(t269)[0]=t270;
t270=pick(2);
X t271=select1(TO_N(2),t270); // SELECT (simple)
t269=top();
S_DATA(t269)[1]=t271;
t271=pick(2);
X t272=select1(TO_N(3),t271); // SELECT (simple)
t269=top();
S_DATA(t269)[2]=t272;
t272=pick(2);
save(t272);
X t273=allocate(2); // CONS
save(t273);
t272=pick(2);
X t275=TO_N(1); // ICONST
save(t275);
X t276=select1(TO_N(1),t272); // SELECT (simple)
t275=restore();
X t274=select1(t275,t276); // SELECT
t273=top();
S_DATA(t273)[0]=t274;
t274=pick(2);
save(t274);
X t277=allocate(2); // CONS
save(t277);
t274=pick(2);
save(t274);
X t278=allocate(3); // CONS
save(t278);
t274=pick(2);
X t279=lf[280]; // CONST
t278=top();
S_DATA(t278)[0]=t279;
t279=pick(2);
X t281=TO_N(1); // ICONST
save(t281);
X t282=select1(TO_N(4),t279); // SELECT (simple)
t281=restore();
X t280=select1(t281,t282); // SELECT
t278=top();
S_DATA(t278)[1]=t280;
t280=pick(2);
X t283=lf[173]; // CONST
t278=top();
S_DATA(t278)[2]=t283;
t278=restore();
restore();
t277=top();
S_DATA(t277)[0]=t278;
t278=pick(2);
X t284=select1(TO_N(4),t278); // SELECT (simple)
t277=top();
S_DATA(t277)[1]=t284;
t277=restore();
restore();
X t285=___backend_3aconc(t277); // REF: backend:conc
t273=top();
S_DATA(t273)[1]=t285;
t273=restore();
restore();
X t286=___backend_3at_5fexpr(t273); // REF: backend:t_expr
t269=top();
S_DATA(t269)[3]=t286;
t269=restore();
restore();
save(t269);
X t287=allocate(4); // CONS
save(t287);
t269=pick(2);
X t288=select1(TO_N(1),t269); // SELECT (simple)
X t289=___tl(t288); // REF: tl
t287=top();
S_DATA(t287)[0]=t289;
t289=pick(2);
X t290=select1(TO_N(2),t289); // SELECT (simple)
t287=top();
S_DATA(t287)[1]=t290;
t290=pick(2);
X t291=select1(TO_N(3),t290); // SELECT (simple)
X t292=___tl(t291); // REF: tl
t287=top();
S_DATA(t287)[2]=t292;
t292=pick(2);
save(t292);
X t293=allocate(2); // CONS
save(t293);
t292=pick(2);
save(t292);
X t294=allocate(11); // CONS
save(t294);
t292=pick(2);
X t295=lf[280]; // CONST
t294=top();
S_DATA(t294)[0]=t295;
t295=pick(2);
X t296=select1(TO_N(2),t295); // SELECT (simple)
t294=top();
S_DATA(t294)[1]=t296;
t296=pick(2);
X t297=lf[171]; // CONST
t294=top();
S_DATA(t294)[2]=t297;
t297=pick(2);
X t298=lf[280]; // CONST
t294=top();
S_DATA(t294)[3]=t298;
t298=pick(2);
X t299=lf[172]; // CONST
t294=top();
S_DATA(t294)[4]=t299;
t299=pick(2);
X t300=select1(TO_N(2),t299); // SELECT (simple)
t294=top();
S_DATA(t294)[5]=t300;
t300=pick(2);
X t301=lf[189]; // CONST
t294=top();
S_DATA(t294)[6]=t301;
t301=pick(2);
X t303=TO_N(1); // ICONST
save(t303);
X t304=select1(TO_N(3),t301); // SELECT (simple)
t303=restore();
X t302=select1(t303,t304); // SELECT
save(t302);
X t305=allocate(2); // CONS
save(t305);
t302=pick(2);
X t306=___id(t302); // REF: id
t305=top();
S_DATA(t305)[0]=t306;
t306=pick(2);
X t307=TO_N(1); // ICONST
t305=top();
S_DATA(t305)[1]=t307;
t305=restore();
restore();
X t308=___sub(t305); // REF: sub
X t309=___tos(t308); // REF: tos
t294=top();
S_DATA(t294)[7]=t309;
t309=pick(2);
X t310=lf[183]; // CONST
t294=top();
S_DATA(t294)[8]=t310;
t310=pick(2);
X t312=TO_N(1); // ICONST
save(t312);
X t313=select1(TO_N(4),t310); // SELECT (simple)
t312=restore();
X t311=select1(t312,t313); // SELECT
t294=top();
S_DATA(t294)[9]=t311;
t311=pick(2);
X t314=lf[288]; // CONST
t294=top();
S_DATA(t294)[10]=t314;
t294=restore();
restore();
t293=top();
S_DATA(t293)[0]=t294;
t294=pick(2);
X t315=select1(TO_N(4),t294); // SELECT (simple)
t293=top();
S_DATA(t293)[1]=t315;
t293=restore();
restore();
X t316=___backend_3aconc(t293); // REF: backend:conc
t287=top();
S_DATA(t287)[3]=t316;
t287=restore();
restore();
t262=t287;}
t261=restore();
X t260=select1(t261,t262); // SELECT
t251=top();
S_DATA(t251)[1]=t260;
t251=restore();
restore();
save(t251);
X t317=allocate(3); // CONS
save(t317);
t251=pick(2);
save(t251);
X t318=allocate(5); // CONS
save(t318);
t251=pick(2);
X t319=lf[280]; // CONST
t318=top();
S_DATA(t318)[0]=t319;
t319=pick(2);
X t320=select1(TO_N(1),t319); // SELECT (simple)
t318=top();
S_DATA(t318)[1]=t320;
t320=pick(2);
X t321=lf[273]; // CONST
t318=top();
S_DATA(t318)[2]=t321;
t321=pick(2);
X t322=lf[280]; // CONST
t318=top();
S_DATA(t318)[3]=t322;
t322=pick(2);
X t323=lf[256]; // CONST
t318=top();
S_DATA(t318)[4]=t323;
t318=restore();
restore();
t317=top();
S_DATA(t317)[0]=t318;
t318=pick(2);
X t324=select1(TO_N(1),t318); // SELECT (simple)
t317=top();
S_DATA(t317)[1]=t324;
t324=pick(2);
X t325=select1(TO_N(2),t324); // SELECT (simple)
t317=top();
S_DATA(t317)[2]=t325;
t317=restore();
restore();
X t326=___backend_3aconcx(t317); // REF: backend:concx
t212=t326;
}else{
save(x); // COND
X t329=lf[329]; // CONST
save(t329);
X t331=TO_N(1); // ICONST
save(t331);
X t332=select1(TO_N(1),x); // SELECT (simple)
t331=restore();
X t330=select1(t331,t332); // SELECT
t329=restore();
X t328=(t329==t330)||eq1(t329,t330)?T:F; // EQ
X t327;
x=restore();
if(t328!=F){
X t333=___backend_3at_5fpcons(x); // REF: backend:t_pcons
t327=t333;
}else{
save(x); // COND
X t336=lf[331]; // CONST
save(t336);
X t338=TO_N(1); // ICONST
save(t338);
X t339=select1(TO_N(1),x); // SELECT (simple)
t338=restore();
X t337=select1(t338,t339); // SELECT
t336=restore();
X t335=(t336==t337)||eq1(t336,t337)?T:F; // EQ
X t334;
x=restore();
if(t335!=F){
X t340=___backend_3at_5fpcons(x); // REF: backend:t_pcons
t334=t340;
}else{
save(x); // COND
X t343=lf[290]; // CONST
save(t343);
X t345=TO_N(1); // ICONST
save(t345);
X t346=select1(TO_N(1),x); // SELECT (simple)
t345=restore();
X t344=select1(t345,t346); // SELECT
t343=restore();
X t342=(t343==t344)||eq1(t343,t344)?T:F; // EQ
X t341;
x=restore();
if(t342!=F){
save(x);
X t347=allocate(2); // CONS
save(t347);
x=pick(2);
X t348=select1(TO_N(1),x); // SELECT (simple)
t347=top();
S_DATA(t347)[0]=t348;
t348=pick(2);
X t349=select1(TO_N(2),t348); // SELECT (simple)
X t350=___backend_3agensym(t349); // REF: backend:gensym
t347=top();
S_DATA(t347)[1]=t350;
t347=restore();
restore();
X t351=___al(t347); // REF: al
save(t351);
X t352=allocate(3); // CONS
save(t352);
t351=pick(2);
save(t351);
X t353=allocate(6); // CONS
save(t353);
t351=pick(2);
X t354=lf[280]; // CONST
t353=top();
S_DATA(t353)[0]=t354;
t354=pick(2);
X t355=lf[262]; // CONST
t353=top();
S_DATA(t353)[1]=t355;
t355=pick(2);
X t356=select1(TO_N(2),t355); // SELECT (simple)
t353=top();
S_DATA(t353)[2]=t356;
t356=pick(2);
X t357=lf[243]; // CONST
t353=top();
S_DATA(t353)[3]=t357;
t357=pick(2);
X t359=TO_N(3); // ICONST
save(t359);
X t360=select1(TO_N(1),t357); // SELECT (simple)
t359=restore();
X t358=select1(t359,t360); // SELECT
save(t358);
X t361=allocate(3); // CONS
save(t361);
t358=pick(2);
X t362=lf[175]; // CONST
t361=top();
S_DATA(t361)[0]=t362;
t362=pick(2);
X t363=___tos(t362); // REF: tos
t361=top();
S_DATA(t361)[1]=t363;
t363=pick(2);
X t364=lf[327]; // CONST
t361=top();
S_DATA(t361)[2]=t364;
t361=restore();
restore();
X t365=___cat(t361); // REF: cat
t353=top();
S_DATA(t353)[4]=t365;
t365=pick(2);
X t366=lf[176]; // CONST
t353=top();
S_DATA(t353)[5]=t366;
t353=restore();
restore();
t352=top();
S_DATA(t352)[0]=t353;
t353=pick(2);
X t367=select1(TO_N(2),t353); // SELECT (simple)
t352=top();
S_DATA(t352)[1]=t367;
t367=pick(2);
X t368=select1(TO_N(3),t367); // SELECT (simple)
t352=top();
S_DATA(t352)[2]=t368;
t352=restore();
restore();
X t369=___backend_3aconcx(t352); // REF: backend:concx
t341=t369;
}else{
save(x); // COND
X t372=lf[291]; // CONST
save(t372);
X t374=TO_N(1); // ICONST
save(t374);
X t375=select1(TO_N(1),x); // SELECT (simple)
t374=restore();
X t373=select1(t374,t375); // SELECT
t372=restore();
X t371=(t372==t373)||eq1(t372,t373)?T:F; // EQ
X t370;
x=restore();
if(t371!=F){
save(x);
X t376=allocate(2); // CONS
save(t376);
x=pick(2);
X t377=select1(TO_N(1),x); // SELECT (simple)
t376=top();
S_DATA(t376)[0]=t377;
t377=pick(2);
X t378=select1(TO_N(2),t377); // SELECT (simple)
X t379=___backend_3agensym(t378); // REF: backend:gensym
t376=top();
S_DATA(t376)[1]=t379;
t376=restore();
restore();
X t380=___al(t376); // REF: al
save(t380);
X t381=allocate(3); // CONS
save(t381);
t380=pick(2);
save(t380);
X t382=allocate(6); // CONS
save(t382);
t380=pick(2);
X t383=lf[280]; // CONST
t382=top();
S_DATA(t382)[0]=t383;
t383=pick(2);
X t384=lf[262]; // CONST
t382=top();
S_DATA(t382)[1]=t384;
t384=pick(2);
X t385=select1(TO_N(2),t384); // SELECT (simple)
t382=top();
S_DATA(t382)[2]=t385;
t385=pick(2);
X t386=lf[243]; // CONST
t382=top();
S_DATA(t382)[3]=t386;
t386=pick(2);
X t388=TO_N(3); // ICONST
save(t388);
X t389=select1(TO_N(1),t386); // SELECT (simple)
t388=restore();
X t387=select1(t388,t389); // SELECT
save(t387); // COND
X t391=___num(t387); // REF: num
X t390;
t387=restore();
if(t391!=F){
save(t387);
X t392=allocate(3); // CONS
save(t392);
t387=pick(2);
X t393=lf[177]; // CONST
t392=top();
S_DATA(t392)[0]=t393;
t393=pick(2);
X t394=___tos(t393); // REF: tos
t392=top();
S_DATA(t392)[1]=t394;
t394=pick(2);
X t395=lf[284]; // CONST
t392=top();
S_DATA(t392)[2]=t395;
t392=restore();
restore();
X t396=___cat(t392); // REF: cat
t390=t396;
}else{
save(t387); // COND
X t399=lf[381]; // CONST
save(t399);
X t400=___id(t387); // REF: id
t399=restore();
X t398=(t399==t400)||eq1(t399,t400)?T:F; // EQ
X t397;
t387=restore();
if(t398!=F){
X t401=lf[178]; // CONST
t397=t401;
}else{
save(t387); // COND
X t404=T; // ICONST
save(t404);
X t405=___id(t387); // REF: id
t404=restore();
X t403=(t404==t405)||eq1(t404,t405)?T:F; // EQ
X t402;
t387=restore();
if(t403!=F){
X t406=lf[179]; // CONST
t402=t406;
}else{
save(t387); // COND
X t409=F; // ICONST
save(t409);
X t410=___id(t387); // REF: id
t409=restore();
X t408=(t409==t410)||eq1(t409,t410)?T:F; // EQ
X t407;
t387=restore();
if(t408!=F){
X t411=lf[180]; // CONST
t407=t411;
}else{
X t412=____5f(t387); // REF: _
t407=t412;}
t402=t407;}
t397=t402;}
t390=t397;}
t382=top();
S_DATA(t382)[4]=t390;
t390=pick(2);
X t413=lf[181]; // CONST
t382=top();
S_DATA(t382)[5]=t413;
t382=restore();
restore();
t381=top();
S_DATA(t381)[0]=t382;
t382=pick(2);
X t414=select1(TO_N(2),t382); // SELECT (simple)
t381=top();
S_DATA(t381)[1]=t414;
t414=pick(2);
X t415=select1(TO_N(3),t414); // SELECT (simple)
t381=top();
S_DATA(t381)[2]=t415;
t381=restore();
restore();
X t416=___backend_3aconcx(t381); // REF: backend:concx
t370=t416;
}else{
save(x); // COND
X t419=lf[296]; // CONST
save(t419);
X t421=TO_N(1); // ICONST
save(t421);
X t422=select1(TO_N(1),x); // SELECT (simple)
t421=restore();
X t420=select1(t421,t422); // SELECT
t419=restore();
X t418=(t419==t420)||eq1(t419,t420)?T:F; // EQ
X t417;
x=restore();
if(t418!=F){
save(x);
X t423=allocate(2); // CONS
save(t423);
x=pick(2);
X t424=select1(TO_N(1),x); // SELECT (simple)
t423=top();
S_DATA(t423)[0]=t424;
t424=pick(2);
save(t424);
X t425=allocate(2); // CONS
save(t425);
t424=pick(2);
X t426=TO_N(3); // ICONST
t425=top();
S_DATA(t425)[0]=t426;
t426=pick(2);
X t427=select1(TO_N(2),t426); // SELECT (simple)
t425=top();
S_DATA(t425)[1]=t427;
t425=restore();
restore();
X t428=___backend_3agensyms(t425); // REF: backend:gensyms
t423=top();
S_DATA(t423)[1]=t428;
t423=restore();
restore();
X t429=___al(t423); // REF: al
save(t429);
X t430=allocate(3); // CONS
save(t430);
t429=pick(2);
X t431=select1(TO_N(1),t429); // SELECT (simple)
t430=top();
S_DATA(t430)[0]=t431;
t431=pick(2);
X t432=select1(TO_N(2),t431); // SELECT (simple)
t430=top();
S_DATA(t430)[1]=t432;
t432=pick(2);
save(t432);
X t433=allocate(3); // CONS
save(t433);
t432=pick(2);
save(t432);
X t434=allocate(48); // CONS
save(t434);
t432=pick(2);
X t435=lf[280]; // CONST
t434=top();
S_DATA(t434)[0]=t435;
t435=pick(2);
X t436=lf[207]; // CONST
t434=top();
S_DATA(t434)[1]=t436;
t436=pick(2);
X t438=TO_N(2); // ICONST
save(t438);
X t439=select1(TO_N(2),t436); // SELECT (simple)
t438=restore();
X t437=select1(t438,t439); // SELECT
t434=top();
S_DATA(t434)[2]=t437;
t437=pick(2);
X t440=lf[184]; // CONST
t434=top();
S_DATA(t434)[3]=t440;
t440=pick(2);
X t441=lf[280]; // CONST
t434=top();
S_DATA(t434)[4]=t441;
t441=pick(2);
X t442=lf[194]; // CONST
t434=top();
S_DATA(t434)[5]=t442;
t442=pick(2);
X t444=TO_N(1); // ICONST
save(t444);
X t445=select1(TO_N(3),t442); // SELECT (simple)
t444=restore();
X t443=select1(t444,t445); // SELECT
t434=top();
S_DATA(t434)[6]=t443;
t443=pick(2);
X t446=lf[252]; // CONST
t434=top();
S_DATA(t434)[7]=t446;
t446=pick(2);
X t447=lf[206]; // CONST
t434=top();
S_DATA(t434)[8]=t447;
t447=pick(2);
X t448=lf[297]; // CONST
t434=top();
S_DATA(t434)[9]=t448;
t448=pick(2);
X t449=lf[206]; // CONST
t434=top();
S_DATA(t434)[10]=t449;
t449=pick(2);
X t450=lf[272]; // CONST
t434=top();
S_DATA(t434)[11]=t450;
t450=pick(2);
X t451=lf[280]; // CONST
t434=top();
S_DATA(t434)[12]=t451;
t451=pick(2);
X t452=lf[207]; // CONST
t434=top();
S_DATA(t434)[13]=t452;
t452=pick(2);
X t454=TO_N(3); // ICONST
save(t454);
X t455=select1(TO_N(2),t452); // SELECT (simple)
t454=restore();
X t453=select1(t454,t455); // SELECT
t434=top();
S_DATA(t434)[14]=t453;
t453=pick(2);
X t456=lf[195]; // CONST
t434=top();
S_DATA(t434)[15]=t456;
t456=pick(2);
X t458=TO_N(1); // ICONST
save(t458);
X t459=select1(TO_N(3),t456); // SELECT (simple)
t458=restore();
X t457=select1(t458,t459); // SELECT
t434=top();
S_DATA(t434)[16]=t457;
t457=pick(2);
X t460=lf[272]; // CONST
t434=top();
S_DATA(t434)[17]=t460;
t460=pick(2);
X t461=lf[280]; // CONST
t434=top();
S_DATA(t434)[18]=t461;
t461=pick(2);
X t462=lf[271]; // CONST
t434=top();
S_DATA(t434)[19]=t462;
t462=pick(2);
X t464=TO_N(1); // ICONST
save(t464);
X t465=select1(TO_N(3),t462); // SELECT (simple)
t464=restore();
X t463=select1(t464,t465); // SELECT
t434=top();
S_DATA(t434)[20]=t463;
t463=pick(2);
X t466=lf[272]; // CONST
t434=top();
S_DATA(t434)[21]=t466;
t466=pick(2);
X t467=lf[280]; // CONST
t434=top();
S_DATA(t434)[22]=t467;
t467=pick(2);
X t468=lf[262]; // CONST
t434=top();
S_DATA(t434)[23]=t468;
t468=pick(2);
X t470=TO_N(1); // ICONST
save(t470);
X t471=select1(TO_N(2),t468); // SELECT (simple)
t470=restore();
X t469=select1(t470,t471); // SELECT
t434=top();
S_DATA(t434)[24]=t469;
t469=pick(2);
X t472=lf[185]; // CONST
t434=top();
S_DATA(t434)[25]=t472;
t472=pick(2);
X t474=TO_N(3); // ICONST
save(t474);
X t475=select1(TO_N(2),t472); // SELECT (simple)
t474=restore();
X t473=select1(t474,t475); // SELECT
t434=top();
S_DATA(t434)[26]=t473;
t473=pick(2);
X t476=lf[272]; // CONST
t434=top();
S_DATA(t434)[27]=t476;
t476=pick(2);
X t477=lf[280]; // CONST
t434=top();
S_DATA(t434)[28]=t477;
t477=pick(2);
X t478=lf[271]; // CONST
t434=top();
S_DATA(t434)[29]=t478;
t478=pick(2);
X t480=TO_N(1); // ICONST
save(t480);
X t481=select1(TO_N(2),t478); // SELECT (simple)
t480=restore();
X t479=select1(t480,t481); // SELECT
t434=top();
S_DATA(t434)[30]=t479;
t479=pick(2);
X t482=lf[272]; // CONST
t434=top();
S_DATA(t434)[31]=t482;
t482=pick(2);
X t483=lf[280]; // CONST
t434=top();
S_DATA(t434)[32]=t483;
t483=pick(2);
X t484=lf[209]; // CONST
t434=top();
S_DATA(t434)[33]=t484;
t484=pick(2);
X t486=TO_N(2); // ICONST
save(t486);
X t487=select1(TO_N(2),t484); // SELECT (simple)
t486=restore();
X t485=select1(t486,t487); // SELECT
t434=top();
S_DATA(t434)[34]=t485;
t485=pick(2);
X t488=lf[186]; // CONST
t434=top();
S_DATA(t434)[35]=t488;
t488=pick(2);
X t490=TO_N(2); // ICONST
save(t490);
X t491=select1(TO_N(2),t488); // SELECT (simple)
t490=restore();
X t489=select1(t490,t491); // SELECT
t434=top();
S_DATA(t434)[36]=t489;
t489=pick(2);
X t492=lf[337]; // CONST
t434=top();
S_DATA(t434)[37]=t492;
t492=pick(2);
X t494=TO_N(3); // ICONST
save(t494);
X t495=select1(TO_N(2),t492); // SELECT (simple)
t494=restore();
X t493=select1(t494,t495); // SELECT
t434=top();
S_DATA(t434)[38]=t493;
t493=pick(2);
X t496=lf[200]; // CONST
t434=top();
S_DATA(t434)[39]=t496;
t496=pick(2);
X t498=TO_N(2); // ICONST
save(t498);
X t499=select1(TO_N(2),t496); // SELECT (simple)
t498=restore();
X t497=select1(t498,t499); // SELECT
t434=top();
S_DATA(t434)[40]=t497;
t497=pick(2);
X t500=lf[212]; // CONST
t434=top();
S_DATA(t434)[41]=t500;
t500=pick(2);
X t501=lf[280]; // CONST
t434=top();
S_DATA(t434)[42]=t501;
t501=pick(2);
X t502=lf[262]; // CONST
t434=top();
S_DATA(t434)[43]=t502;
t502=pick(2);
X t504=TO_N(1); // ICONST
save(t504);
X t505=select1(TO_N(2),t502); // SELECT (simple)
t504=restore();
X t503=select1(t504,t505); // SELECT
t434=top();
S_DATA(t434)[44]=t503;
t503=pick(2);
X t506=lf[187]; // CONST
t434=top();
S_DATA(t434)[45]=t506;
t506=pick(2);
X t508=TO_N(2); // ICONST
save(t508);
X t509=select1(TO_N(2),t506); // SELECT (simple)
t508=restore();
X t507=select1(t508,t509); // SELECT
t434=top();
S_DATA(t434)[46]=t507;
t507=pick(2);
X t510=lf[261]; // CONST
t434=top();
S_DATA(t434)[47]=t510;
t434=restore();
restore();
t433=top();
S_DATA(t433)[0]=t434;
t434=pick(2);
X t512=TO_N(1); // ICONST
save(t512);
X t513=select1(TO_N(2),t434); // SELECT (simple)
t512=restore();
X t511=select1(t512,t513); // SELECT
t433=top();
S_DATA(t433)[1]=t511;
t511=pick(2);
X t514=select1(TO_N(3),t511); // SELECT (simple)
t433=top();
S_DATA(t433)[2]=t514;
t433=restore();
restore();
X t515=___backend_3aconcx(t433); // REF: backend:concx
t430=top();
S_DATA(t430)[2]=t515;
t430=restore();
restore();
save(t430);
X t516=allocate(2); // CONS
save(t516);
t430=pick(2);
X t517=select1(TO_N(2),t430); // SELECT (simple)
t516=top();
S_DATA(t516)[0]=t517;
t517=pick(2);
save(t517);
X t518=allocate(2); // CONS
save(t518);
t517=pick(2);
X t520=TO_N(3); // ICONST
save(t520);
X t521=select1(TO_N(1),t517); // SELECT (simple)
t520=restore();
X t519=select1(t520,t521); // SELECT
t518=top();
S_DATA(t518)[0]=t519;
t519=pick(2);
X t522=select1(TO_N(3),t519); // SELECT (simple)
X t523=___backend_3anontail(t522); // REF: backend:nontail
t518=top();
S_DATA(t518)[1]=t523;
t518=restore();
restore();
X t524=___backend_3at_5fexpr(t518); // REF: backend:t_expr
t516=top();
S_DATA(t516)[1]=t524;
t516=restore();
restore();
save(t516);
X t525=allocate(3); // CONS
save(t525);
t516=pick(2);
save(t516);
X t526=allocate(11); // CONS
save(t526);
t516=pick(2);
X t527=lf[280]; // CONST
t526=top();
S_DATA(t526)[0]=t527;
t527=pick(2);
X t528=lf[182]; // CONST
t526=top();
S_DATA(t526)[1]=t528;
t528=pick(2);
X t530=TO_N(2); // ICONST
save(t530);
X t531=select1(TO_N(1),t528); // SELECT (simple)
t530=restore();
X t529=select1(t530,t531); // SELECT
t526=top();
S_DATA(t526)[2]=t529;
t529=pick(2);
X t532=lf[183]; // CONST
t526=top();
S_DATA(t526)[3]=t532;
t532=pick(2);
X t534=TO_N(1); // ICONST
save(t534);
X t535=select1(TO_N(2),t532); // SELECT (simple)
t534=restore();
X t533=select1(t534,t535); // SELECT
t526=top();
S_DATA(t526)[4]=t533;
t533=pick(2);
X t536=lf[229]; // CONST
t526=top();
S_DATA(t526)[5]=t536;
t536=pick(2);
X t537=lf[280]; // CONST
t526=top();
S_DATA(t526)[6]=t537;
t537=pick(2);
X t539=TO_N(1); // ICONST
save(t539);
X t540=select1(TO_N(1),t537); // SELECT (simple)
t539=restore();
X t538=select1(t539,t540); // SELECT
t526=top();
S_DATA(t526)[7]=t538;
t538=pick(2);
X t541=lf[273]; // CONST
t526=top();
S_DATA(t526)[8]=t541;
t541=pick(2);
X t542=lf[280]; // CONST
t526=top();
S_DATA(t526)[9]=t542;
t542=pick(2);
X t543=lf[256]; // CONST
t526=top();
S_DATA(t526)[10]=t543;
t526=restore();
restore();
t525=top();
S_DATA(t525)[0]=t526;
t526=pick(2);
X t545=TO_N(1); // ICONST
save(t545);
X t546=select1(TO_N(1),t526); // SELECT (simple)
t545=restore();
X t544=select1(t545,t546); // SELECT
t525=top();
S_DATA(t525)[1]=t544;
t544=pick(2);
X t547=select1(TO_N(2),t544); // SELECT (simple)
t525=top();
S_DATA(t525)[2]=t547;
t525=restore();
restore();
X t548=___backend_3aconcx(t525); // REF: backend:concx
t417=t548;
}else{
save(x); // COND
X t551=lf[298]; // CONST
save(t551);
X t553=TO_N(1); // ICONST
save(t553);
X t554=select1(TO_N(1),x); // SELECT (simple)
t553=restore();
X t552=select1(t553,t554); // SELECT
t551=restore();
X t550=(t551==t552)||eq1(t551,t552)?T:F; // EQ
X t549;
x=restore();
if(t550!=F){
save(x);
X t555=allocate(2); // CONS
save(t555);
x=pick(2);
X t556=select1(TO_N(1),x); // SELECT (simple)
t555=top();
S_DATA(t555)[0]=t556;
t556=pick(2);
save(t556);
X t557=allocate(2); // CONS
save(t557);
t556=pick(2);
X t558=TO_N(4); // ICONST
t557=top();
S_DATA(t557)[0]=t558;
t558=pick(2);
X t559=select1(TO_N(2),t558); // SELECT (simple)
t557=top();
S_DATA(t557)[1]=t559;
t557=restore();
restore();
X t560=___backend_3agensyms(t557); // REF: backend:gensyms
t555=top();
S_DATA(t555)[1]=t560;
t555=restore();
restore();
X t561=___al(t555); // REF: al
save(t561);
X t562=allocate(3); // CONS
save(t562);
t561=pick(2);
X t563=select1(TO_N(1),t561); // SELECT (simple)
t562=top();
S_DATA(t562)[0]=t563;
t563=pick(2);
X t564=select1(TO_N(2),t563); // SELECT (simple)
t562=top();
S_DATA(t562)[1]=t564;
t564=pick(2);
save(t564);
X t565=allocate(3); // CONS
save(t565);
t564=pick(2);
save(t564);
X t566=allocate(52); // CONS
save(t566);
t564=pick(2);
X t567=lf[280]; // CONST
t566=top();
S_DATA(t566)[0]=t567;
t567=pick(2);
X t568=lf[207]; // CONST
t566=top();
S_DATA(t566)[1]=t568;
t568=pick(2);
X t570=TO_N(3); // ICONST
save(t570);
X t571=select1(TO_N(2),t568); // SELECT (simple)
t570=restore();
X t569=select1(t570,t571); // SELECT
t566=top();
S_DATA(t566)[2]=t569;
t569=pick(2);
X t572=lf[188]; // CONST
t566=top();
S_DATA(t566)[3]=t572;
t572=pick(2);
X t573=lf[280]; // CONST
t566=top();
S_DATA(t566)[4]=t573;
t573=pick(2);
X t574=lf[194]; // CONST
t566=top();
S_DATA(t566)[5]=t574;
t574=pick(2);
X t576=TO_N(1); // ICONST
save(t576);
X t577=select1(TO_N(3),t574); // SELECT (simple)
t576=restore();
X t575=select1(t576,t577); // SELECT
t566=top();
S_DATA(t566)[6]=t575;
t575=pick(2);
X t578=lf[252]; // CONST
t566=top();
S_DATA(t566)[7]=t578;
t578=pick(2);
X t579=lf[206]; // CONST
t566=top();
S_DATA(t566)[8]=t579;
t579=pick(2);
X t580=lf[299]; // CONST
t566=top();
S_DATA(t566)[9]=t580;
t580=pick(2);
X t581=lf[206]; // CONST
t566=top();
S_DATA(t566)[10]=t581;
t581=pick(2);
X t582=lf[272]; // CONST
t566=top();
S_DATA(t566)[11]=t582;
t582=pick(2);
X t583=lf[280]; // CONST
t566=top();
S_DATA(t566)[12]=t583;
t583=pick(2);
X t584=lf[207]; // CONST
t566=top();
S_DATA(t566)[13]=t584;
t584=pick(2);
X t586=TO_N(4); // ICONST
save(t586);
X t587=select1(TO_N(2),t584); // SELECT (simple)
t586=restore();
X t585=select1(t586,t587); // SELECT
t566=top();
S_DATA(t566)[14]=t585;
t585=pick(2);
X t588=lf[195]; // CONST
t566=top();
S_DATA(t566)[15]=t588;
t588=pick(2);
X t590=TO_N(1); // ICONST
save(t590);
X t591=select1(TO_N(3),t588); // SELECT (simple)
t590=restore();
X t589=select1(t590,t591); // SELECT
t566=top();
S_DATA(t566)[16]=t589;
t589=pick(2);
X t592=lf[272]; // CONST
t566=top();
S_DATA(t566)[17]=t592;
t592=pick(2);
X t593=lf[280]; // CONST
t566=top();
S_DATA(t566)[18]=t593;
t593=pick(2);
X t594=lf[262]; // CONST
t566=top();
S_DATA(t566)[19]=t594;
t594=pick(2);
X t596=TO_N(1); // ICONST
save(t596);
X t597=select1(TO_N(2),t594); // SELECT (simple)
t596=restore();
X t595=select1(t596,t597); // SELECT
t566=top();
S_DATA(t566)[20]=t595;
t595=pick(2);
X t598=lf[243]; // CONST
t566=top();
S_DATA(t566)[21]=t598;
t598=pick(2);
X t600=TO_N(4); // ICONST
save(t600);
X t601=select1(TO_N(2),t598); // SELECT (simple)
t600=restore();
X t599=select1(t600,t601); // SELECT
t566=top();
S_DATA(t566)[22]=t599;
t599=pick(2);
X t602=lf[196]; // CONST
t566=top();
S_DATA(t566)[23]=t602;
t602=pick(2);
X t604=TO_N(3); // ICONST
save(t604);
X t605=select1(TO_N(1),t602); // SELECT (simple)
t604=restore();
X t603=select1(t604,t605); // SELECT
X t606=___backend_3aunit_5fvalue(t603); // REF: backend:unit_value
t566=top();
S_DATA(t566)[24]=t606;
t606=pick(2);
X t607=lf[197]; // CONST
t566=top();
S_DATA(t566)[25]=t607;
t607=pick(2);
X t609=TO_N(1); // ICONST
save(t609);
X t610=select1(TO_N(3),t607); // SELECT (simple)
t609=restore();
X t608=select1(t609,t610); // SELECT
t566=top();
S_DATA(t566)[26]=t608;
t608=pick(2);
X t611=lf[189]; // CONST
t566=top();
S_DATA(t566)[27]=t611;
t611=pick(2);
X t613=TO_N(4); // ICONST
save(t613);
X t614=select1(TO_N(2),t611); // SELECT (simple)
t613=restore();
X t612=select1(t613,t614); // SELECT
t566=top();
S_DATA(t566)[28]=t612;
t612=pick(2);
X t615=lf[190]; // CONST
t566=top();
S_DATA(t566)[29]=t615;
t615=pick(2);
X t616=lf[280]; // CONST
t566=top();
S_DATA(t566)[30]=t616;
t616=pick(2);
X t617=lf[271]; // CONST
t566=top();
S_DATA(t566)[31]=t617;
t617=pick(2);
X t619=TO_N(1); // ICONST
save(t619);
X t620=select1(TO_N(3),t617); // SELECT (simple)
t619=restore();
X t618=select1(t619,t620); // SELECT
t566=top();
S_DATA(t566)[32]=t618;
t618=pick(2);
X t621=lf[272]; // CONST
t566=top();
S_DATA(t566)[33]=t621;
t621=pick(2);
X t622=lf[280]; // CONST
t566=top();
S_DATA(t566)[34]=t622;
t622=pick(2);
X t623=lf[209]; // CONST
t566=top();
S_DATA(t566)[35]=t623;
t623=pick(2);
X t625=TO_N(3); // ICONST
save(t625);
X t626=select1(TO_N(2),t623); // SELECT (simple)
t625=restore();
X t624=select1(t625,t626); // SELECT
t566=top();
S_DATA(t566)[36]=t624;
t624=pick(2);
X t627=lf[243]; // CONST
t566=top();
S_DATA(t566)[37]=t627;
t627=pick(2);
X t629=TO_N(4); // ICONST
save(t629);
X t630=select1(TO_N(2),t627); // SELECT (simple)
t629=restore();
X t628=select1(t629,t630); // SELECT
t566=top();
S_DATA(t566)[38]=t628;
t628=pick(2);
X t631=lf[191]; // CONST
t566=top();
S_DATA(t566)[39]=t631;
t631=pick(2);
X t633=TO_N(3); // ICONST
save(t633);
X t634=select1(TO_N(2),t631); // SELECT (simple)
t633=restore();
X t632=select1(t633,t634); // SELECT
t566=top();
S_DATA(t566)[40]=t632;
t632=pick(2);
X t635=lf[211]; // CONST
t566=top();
S_DATA(t566)[41]=t635;
t635=pick(2);
X t637=TO_N(3); // ICONST
save(t637);
X t638=select1(TO_N(2),t635); // SELECT (simple)
t637=restore();
X t636=select1(t637,t638); // SELECT
t566=top();
S_DATA(t566)[42]=t636;
t636=pick(2);
X t639=lf[212]; // CONST
t566=top();
S_DATA(t566)[43]=t639;
t639=pick(2);
X t640=lf[280]; // CONST
t566=top();
S_DATA(t566)[44]=t640;
t640=pick(2);
X t641=lf[262]; // CONST
t566=top();
S_DATA(t566)[45]=t641;
t641=pick(2);
X t643=TO_N(2); // ICONST
save(t643);
X t644=select1(TO_N(2),t641); // SELECT (simple)
t643=restore();
X t642=select1(t643,t644); // SELECT
t566=top();
S_DATA(t566)[46]=t642;
t642=pick(2);
X t645=lf[192]; // CONST
t566=top();
S_DATA(t566)[47]=t645;
t645=pick(2);
X t647=TO_N(3); // ICONST
save(t647);
X t648=select1(TO_N(2),t645); // SELECT (simple)
t647=restore();
X t646=select1(t647,t648); // SELECT
t566=top();
S_DATA(t566)[48]=t646;
t646=pick(2);
X t649=lf[214]; // CONST
t566=top();
S_DATA(t566)[49]=t649;
t649=pick(2);
X t651=TO_N(1); // ICONST
save(t651);
X t652=select1(TO_N(2),t649); // SELECT (simple)
t651=restore();
X t650=select1(t651,t652); // SELECT
t566=top();
S_DATA(t566)[50]=t650;
t650=pick(2);
X t653=lf[272]; // CONST
t566=top();
S_DATA(t566)[51]=t653;
t566=restore();
restore();
t565=top();
S_DATA(t565)[0]=t566;
t566=pick(2);
X t655=TO_N(2); // ICONST
save(t655);
X t656=select1(TO_N(2),t566); // SELECT (simple)
t655=restore();
X t654=select1(t655,t656); // SELECT
t565=top();
S_DATA(t565)[1]=t654;
t654=pick(2);
X t657=select1(TO_N(3),t654); // SELECT (simple)
t565=top();
S_DATA(t565)[2]=t657;
t565=restore();
restore();
X t658=___backend_3aconcx(t565); // REF: backend:concx
t562=top();
S_DATA(t562)[2]=t658;
t562=restore();
restore();
save(t562);
X t659=allocate(2); // CONS
save(t659);
t562=pick(2);
X t660=select1(TO_N(2),t562); // SELECT (simple)
t659=top();
S_DATA(t659)[0]=t660;
t660=pick(2);
save(t660);
X t661=allocate(2); // CONS
save(t661);
t660=pick(2);
X t663=TO_N(3); // ICONST
save(t663);
X t664=select1(TO_N(1),t660); // SELECT (simple)
t663=restore();
X t662=select1(t663,t664); // SELECT
t661=top();
S_DATA(t661)[0]=t662;
t662=pick(2);
X t665=select1(TO_N(3),t662); // SELECT (simple)
X t666=___backend_3anontail(t665); // REF: backend:nontail
t661=top();
S_DATA(t661)[1]=t666;
t661=restore();
restore();
X t667=___backend_3at_5fexpr(t661); // REF: backend:t_expr
t659=top();
S_DATA(t659)[1]=t667;
t659=restore();
restore();
save(t659);
X t668=allocate(3); // CONS
save(t668);
t659=pick(2);
save(t659);
X t669=allocate(7); // CONS
save(t669);
t659=pick(2);
X t670=lf[280]; // CONST
t669=top();
S_DATA(t669)[0]=t670;
t670=pick(2);
X t672=TO_N(1); // ICONST
save(t672);
X t673=select1(TO_N(1),t670); // SELECT (simple)
t672=restore();
X t671=select1(t672,t673); // SELECT
t669=top();
S_DATA(t669)[1]=t671;
t671=pick(2);
X t674=lf[243]; // CONST
t669=top();
S_DATA(t669)[2]=t674;
t674=pick(2);
X t676=TO_N(1); // ICONST
save(t676);
X t677=select1(TO_N(2),t674); // SELECT (simple)
t676=restore();
X t675=select1(t676,t677); // SELECT
t669=top();
S_DATA(t669)[3]=t675;
t675=pick(2);
X t678=lf[229]; // CONST
t669=top();
S_DATA(t669)[4]=t678;
t678=pick(2);
X t679=lf[280]; // CONST
t669=top();
S_DATA(t669)[5]=t679;
t679=pick(2);
X t680=lf[256]; // CONST
t669=top();
S_DATA(t669)[6]=t680;
t669=restore();
restore();
t668=top();
S_DATA(t668)[0]=t669;
t669=pick(2);
X t682=TO_N(1); // ICONST
save(t682);
X t683=select1(TO_N(1),t669); // SELECT (simple)
t682=restore();
X t681=select1(t682,t683); // SELECT
t668=top();
S_DATA(t668)[1]=t681;
t681=pick(2);
X t684=select1(TO_N(2),t681); // SELECT (simple)
t668=top();
S_DATA(t668)[2]=t684;
t668=restore();
restore();
X t685=___backend_3aconcx(t668); // REF: backend:concx
t549=t685;
}else{
save(x); // COND
X t688=lf[300]; // CONST
save(t688);
X t690=TO_N(1); // ICONST
save(t690);
X t691=select1(TO_N(1),x); // SELECT (simple)
t690=restore();
X t689=select1(t690,t691); // SELECT
t688=restore();
X t687=(t688==t689)||eq1(t688,t689)?T:F; // EQ
X t686;
x=restore();
if(t687!=F){
save(x);
X t692=allocate(2); // CONS
save(t692);
x=pick(2);
X t693=select1(TO_N(1),x); // SELECT (simple)
t692=top();
S_DATA(t692)[0]=t693;
t693=pick(2);
save(t693);
X t694=allocate(2); // CONS
save(t694);
t693=pick(2);
X t695=TO_N(4); // ICONST
t694=top();
S_DATA(t694)[0]=t695;
t695=pick(2);
X t696=select1(TO_N(2),t695); // SELECT (simple)
t694=top();
S_DATA(t694)[1]=t696;
t694=restore();
restore();
X t697=___backend_3agensyms(t694); // REF: backend:gensyms
t692=top();
S_DATA(t692)[1]=t697;
t692=restore();
restore();
X t698=___al(t692); // REF: al
save(t698);
X t699=allocate(3); // CONS
save(t699);
t698=pick(2);
X t700=select1(TO_N(1),t698); // SELECT (simple)
t699=top();
S_DATA(t699)[0]=t700;
t700=pick(2);
X t701=select1(TO_N(2),t700); // SELECT (simple)
t699=top();
S_DATA(t699)[1]=t701;
t701=pick(2);
save(t701);
X t702=allocate(3); // CONS
save(t702);
t701=pick(2);
save(t701);
X t703=allocate(50); // CONS
save(t703);
t701=pick(2);
X t704=lf[280]; // CONST
t703=top();
S_DATA(t703)[0]=t704;
t704=pick(2);
X t705=lf[207]; // CONST
t703=top();
S_DATA(t703)[1]=t705;
t705=pick(2);
X t707=TO_N(3); // ICONST
save(t707);
X t708=select1(TO_N(2),t705); // SELECT (simple)
t707=restore();
X t706=select1(t707,t708); // SELECT
t703=top();
S_DATA(t703)[2]=t706;
t706=pick(2);
X t709=lf[193]; // CONST
t703=top();
S_DATA(t703)[3]=t709;
t709=pick(2);
X t710=lf[280]; // CONST
t703=top();
S_DATA(t703)[4]=t710;
t710=pick(2);
X t711=lf[194]; // CONST
t703=top();
S_DATA(t703)[5]=t711;
t711=pick(2);
X t713=TO_N(1); // ICONST
save(t713);
X t714=select1(TO_N(3),t711); // SELECT (simple)
t713=restore();
X t712=select1(t713,t714); // SELECT
t703=top();
S_DATA(t703)[6]=t712;
t712=pick(2);
X t715=lf[252]; // CONST
t703=top();
S_DATA(t703)[7]=t715;
t715=pick(2);
X t716=lf[206]; // CONST
t703=top();
S_DATA(t703)[8]=t716;
t716=pick(2);
X t717=lf[299]; // CONST
t703=top();
S_DATA(t703)[9]=t717;
t717=pick(2);
X t718=lf[206]; // CONST
t703=top();
S_DATA(t703)[10]=t718;
t718=pick(2);
X t719=lf[272]; // CONST
t703=top();
S_DATA(t703)[11]=t719;
t719=pick(2);
X t720=lf[280]; // CONST
t703=top();
S_DATA(t703)[12]=t720;
t720=pick(2);
X t721=lf[207]; // CONST
t703=top();
S_DATA(t703)[13]=t721;
t721=pick(2);
X t723=TO_N(4); // ICONST
save(t723);
X t724=select1(TO_N(2),t721); // SELECT (simple)
t723=restore();
X t722=select1(t723,t724); // SELECT
t703=top();
S_DATA(t703)[14]=t722;
t722=pick(2);
X t725=lf[195]; // CONST
t703=top();
S_DATA(t703)[15]=t725;
t725=pick(2);
X t727=TO_N(1); // ICONST
save(t727);
X t728=select1(TO_N(3),t725); // SELECT (simple)
t727=restore();
X t726=select1(t727,t728); // SELECT
t703=top();
S_DATA(t703)[16]=t726;
t726=pick(2);
X t729=lf[272]; // CONST
t703=top();
S_DATA(t703)[17]=t729;
t729=pick(2);
X t730=lf[280]; // CONST
t703=top();
S_DATA(t703)[18]=t730;
t730=pick(2);
X t731=lf[262]; // CONST
t703=top();
S_DATA(t703)[19]=t731;
t731=pick(2);
X t733=TO_N(1); // ICONST
save(t733);
X t734=select1(TO_N(2),t731); // SELECT (simple)
t733=restore();
X t732=select1(t733,t734); // SELECT
t703=top();
S_DATA(t703)[20]=t732;
t732=pick(2);
X t735=lf[243]; // CONST
t703=top();
S_DATA(t703)[21]=t735;
t735=pick(2);
X t737=TO_N(4); // ICONST
save(t737);
X t738=select1(TO_N(2),t735); // SELECT (simple)
t737=restore();
X t736=select1(t737,t738); // SELECT
t703=top();
S_DATA(t703)[22]=t736;
t736=pick(2);
X t739=lf[196]; // CONST
t703=top();
S_DATA(t703)[23]=t739;
t739=pick(2);
X t741=TO_N(3); // ICONST
save(t741);
X t742=select1(TO_N(1),t739); // SELECT (simple)
t741=restore();
X t740=select1(t741,t742); // SELECT
X t743=___backend_3aunit_5fvalue(t740); // REF: backend:unit_value
t703=top();
S_DATA(t703)[24]=t743;
t743=pick(2);
X t744=lf[197]; // CONST
t703=top();
S_DATA(t703)[25]=t744;
t744=pick(2);
X t746=TO_N(1); // ICONST
save(t746);
X t747=select1(TO_N(3),t744); // SELECT (simple)
t746=restore();
X t745=select1(t746,t747); // SELECT
t703=top();
S_DATA(t703)[26]=t745;
t745=pick(2);
X t748=lf[198]; // CONST
t703=top();
S_DATA(t703)[27]=t748;
t748=pick(2);
X t749=lf[280]; // CONST
t703=top();
S_DATA(t703)[28]=t749;
t749=pick(2);
X t750=lf[271]; // CONST
t703=top();
S_DATA(t703)[29]=t750;
t750=pick(2);
X t752=TO_N(1); // ICONST
save(t752);
X t753=select1(TO_N(3),t750); // SELECT (simple)
t752=restore();
X t751=select1(t752,t753); // SELECT
t703=top();
S_DATA(t703)[30]=t751;
t751=pick(2);
X t754=lf[272]; // CONST
t703=top();
S_DATA(t703)[31]=t754;
t754=pick(2);
X t755=lf[280]; // CONST
t703=top();
S_DATA(t703)[32]=t755;
t755=pick(2);
X t756=lf[209]; // CONST
t703=top();
S_DATA(t703)[33]=t756;
t756=pick(2);
X t758=TO_N(3); // ICONST
save(t758);
X t759=select1(TO_N(2),t756); // SELECT (simple)
t758=restore();
X t757=select1(t758,t759); // SELECT
t703=top();
S_DATA(t703)[34]=t757;
t757=pick(2);
X t760=lf[199]; // CONST
t703=top();
S_DATA(t703)[35]=t760;
t760=pick(2);
X t762=TO_N(3); // ICONST
save(t762);
X t763=select1(TO_N(2),t760); // SELECT (simple)
t762=restore();
X t761=select1(t762,t763); // SELECT
t703=top();
S_DATA(t703)[36]=t761;
t761=pick(2);
X t764=lf[337]; // CONST
t703=top();
S_DATA(t703)[37]=t764;
t764=pick(2);
X t766=TO_N(4); // ICONST
save(t766);
X t767=select1(TO_N(2),t764); // SELECT (simple)
t766=restore();
X t765=select1(t766,t767); // SELECT
t703=top();
S_DATA(t703)[38]=t765;
t765=pick(2);
X t768=lf[200]; // CONST
t703=top();
S_DATA(t703)[39]=t768;
t768=pick(2);
X t770=TO_N(3); // ICONST
save(t770);
X t771=select1(TO_N(2),t768); // SELECT (simple)
t770=restore();
X t769=select1(t770,t771); // SELECT
t703=top();
S_DATA(t703)[40]=t769;
t769=pick(2);
X t772=lf[212]; // CONST
t703=top();
S_DATA(t703)[41]=t772;
t772=pick(2);
X t773=lf[280]; // CONST
t703=top();
S_DATA(t703)[42]=t773;
t773=pick(2);
X t774=lf[262]; // CONST
t703=top();
S_DATA(t703)[43]=t774;
t774=pick(2);
X t776=TO_N(2); // ICONST
save(t776);
X t777=select1(TO_N(2),t774); // SELECT (simple)
t776=restore();
X t775=select1(t776,t777); // SELECT
t703=top();
S_DATA(t703)[44]=t775;
t775=pick(2);
X t778=lf[201]; // CONST
t703=top();
S_DATA(t703)[45]=t778;
t778=pick(2);
X t780=TO_N(1); // ICONST
save(t780);
X t781=select1(TO_N(2),t778); // SELECT (simple)
t780=restore();
X t779=select1(t780,t781); // SELECT
t703=top();
S_DATA(t703)[46]=t779;
t779=pick(2);
X t782=lf[202]; // CONST
t703=top();
S_DATA(t703)[47]=t782;
t782=pick(2);
X t784=TO_N(3); // ICONST
save(t784);
X t785=select1(TO_N(2),t782); // SELECT (simple)
t784=restore();
X t783=select1(t784,t785); // SELECT
t703=top();
S_DATA(t703)[48]=t783;
t783=pick(2);
X t786=lf[203]; // CONST
t703=top();
S_DATA(t703)[49]=t786;
t703=restore();
restore();
t702=top();
S_DATA(t702)[0]=t703;
t703=pick(2);
X t788=TO_N(2); // ICONST
save(t788);
X t789=select1(TO_N(2),t703); // SELECT (simple)
t788=restore();
X t787=select1(t788,t789); // SELECT
t702=top();
S_DATA(t702)[1]=t787;
t787=pick(2);
X t790=select1(TO_N(3),t787); // SELECT (simple)
t702=top();
S_DATA(t702)[2]=t790;
t702=restore();
restore();
X t791=___backend_3aconcx(t702); // REF: backend:concx
t699=top();
S_DATA(t699)[2]=t791;
t699=restore();
restore();
save(t699);
X t792=allocate(2); // CONS
save(t792);
t699=pick(2);
X t793=select1(TO_N(2),t699); // SELECT (simple)
t792=top();
S_DATA(t792)[0]=t793;
t793=pick(2);
save(t793);
X t794=allocate(2); // CONS
save(t794);
t793=pick(2);
X t796=TO_N(3); // ICONST
save(t796);
X t797=select1(TO_N(1),t793); // SELECT (simple)
t796=restore();
X t795=select1(t796,t797); // SELECT
t794=top();
S_DATA(t794)[0]=t795;
t795=pick(2);
X t798=select1(TO_N(3),t795); // SELECT (simple)
X t799=___backend_3anontail(t798); // REF: backend:nontail
t794=top();
S_DATA(t794)[1]=t799;
t794=restore();
restore();
X t800=___backend_3at_5fexpr(t794); // REF: backend:t_expr
t792=top();
S_DATA(t792)[1]=t800;
t792=restore();
restore();
save(t792);
X t801=allocate(3); // CONS
save(t801);
t792=pick(2);
save(t792);
X t802=allocate(7); // CONS
save(t802);
t792=pick(2);
X t803=lf[280]; // CONST
t802=top();
S_DATA(t802)[0]=t803;
t803=pick(2);
X t805=TO_N(1); // ICONST
save(t805);
X t806=select1(TO_N(1),t803); // SELECT (simple)
t805=restore();
X t804=select1(t805,t806); // SELECT
t802=top();
S_DATA(t802)[1]=t804;
t804=pick(2);
X t807=lf[243]; // CONST
t802=top();
S_DATA(t802)[2]=t807;
t807=pick(2);
X t809=TO_N(1); // ICONST
save(t809);
X t810=select1(TO_N(2),t807); // SELECT (simple)
t809=restore();
X t808=select1(t809,t810); // SELECT
t802=top();
S_DATA(t802)[3]=t808;
t808=pick(2);
X t811=lf[229]; // CONST
t802=top();
S_DATA(t802)[4]=t811;
t811=pick(2);
X t812=lf[280]; // CONST
t802=top();
S_DATA(t802)[5]=t812;
t812=pick(2);
X t813=lf[256]; // CONST
t802=top();
S_DATA(t802)[6]=t813;
t802=restore();
restore();
t801=top();
S_DATA(t801)[0]=t802;
t802=pick(2);
X t815=TO_N(1); // ICONST
save(t815);
X t816=select1(TO_N(1),t802); // SELECT (simple)
t815=restore();
X t814=select1(t815,t816); // SELECT
t801=top();
S_DATA(t801)[1]=t814;
t814=pick(2);
X t817=select1(TO_N(2),t814); // SELECT (simple)
t801=top();
S_DATA(t801)[2]=t817;
t801=restore();
restore();
X t818=___backend_3aconcx(t801); // REF: backend:concx
t686=t818;
}else{
save(x); // COND
X t821=lf[302]; // CONST
save(t821);
X t823=TO_N(1); // ICONST
save(t823);
X t824=select1(TO_N(1),x); // SELECT (simple)
t823=restore();
X t822=select1(t823,t824); // SELECT
t821=restore();
X t820=(t821==t822)||eq1(t821,t822)?T:F; // EQ
X t819;
x=restore();
if(t820!=F){
save(x);
X t825=allocate(2); // CONS
save(t825);
x=pick(2);
X t826=select1(TO_N(1),x); // SELECT (simple)
t825=top();
S_DATA(t825)[0]=t826;
t826=pick(2);
save(t826);
X t827=allocate(2); // CONS
save(t827);
t826=pick(2);
X t828=TO_N(4); // ICONST
t827=top();
S_DATA(t827)[0]=t828;
t828=pick(2);
X t829=select1(TO_N(2),t828); // SELECT (simple)
t827=top();
S_DATA(t827)[1]=t829;
t827=restore();
restore();
X t830=___backend_3agensyms(t827); // REF: backend:gensyms
t825=top();
S_DATA(t825)[1]=t830;
t825=restore();
restore();
X t831=___al(t825); // REF: al
save(t831);
X t832=allocate(3); // CONS
save(t832);
t831=pick(2);
X t833=select1(TO_N(1),t831); // SELECT (simple)
t832=top();
S_DATA(t832)[0]=t833;
t833=pick(2);
X t834=select1(TO_N(2),t833); // SELECT (simple)
t832=top();
S_DATA(t832)[1]=t834;
t834=pick(2);
save(t834);
X t835=allocate(2); // CONS
save(t835);
t834=pick(2);
save(t834);
X t836=allocate(4); // CONS
save(t836);
t834=pick(2);
X t837=lf[280]; // CONST
t836=top();
S_DATA(t836)[0]=t837;
t837=pick(2);
X t838=lf[271]; // CONST
t836=top();
S_DATA(t836)[1]=t838;
t838=pick(2);
X t840=TO_N(1); // ICONST
save(t840);
X t841=select1(TO_N(3),t838); // SELECT (simple)
t840=restore();
X t839=select1(t840,t841); // SELECT
t836=top();
S_DATA(t836)[2]=t839;
t839=pick(2);
X t842=lf[215]; // CONST
t836=top();
S_DATA(t836)[3]=t842;
t836=restore();
restore();
t835=top();
S_DATA(t835)[0]=t836;
t836=pick(2);
X t843=select1(TO_N(3),t836); // SELECT (simple)
t835=top();
S_DATA(t835)[1]=t843;
t835=restore();
restore();
X t844=___backend_3aconc(t835); // REF: backend:conc
t832=top();
S_DATA(t832)[2]=t844;
t832=restore();
restore();
save(t832);
X t845=allocate(3); // CONS
save(t845);
t832=pick(2);
X t846=select1(TO_N(1),t832); // SELECT (simple)
t845=top();
S_DATA(t845)[0]=t846;
t846=pick(2);
X t847=select1(TO_N(2),t846); // SELECT (simple)
t845=top();
S_DATA(t845)[1]=t847;
t847=pick(2);
save(t847);
X t848=allocate(2); // CONS
save(t848);
t847=pick(2);
X t850=TO_N(3); // ICONST
save(t850);
X t851=select1(TO_N(1),t847); // SELECT (simple)
t850=restore();
X t849=select1(t850,t851); // SELECT
t848=top();
S_DATA(t848)[0]=t849;
t849=pick(2);
X t852=select1(TO_N(3),t849); // SELECT (simple)
X t853=___backend_3anontail(t852); // REF: backend:nontail
t848=top();
S_DATA(t848)[1]=t853;
t848=restore();
restore();
X t854=___backend_3at_5fexpr(t848); // REF: backend:t_expr
t845=top();
S_DATA(t845)[2]=t854;
t845=restore();
restore();
save(t845);
X t855=allocate(4); // CONS
save(t855);
t845=pick(2);
X t856=select1(TO_N(1),t845); // SELECT (simple)
t855=top();
S_DATA(t855)[0]=t856;
t856=pick(2);
X t857=select1(TO_N(2),t856); // SELECT (simple)
t855=top();
S_DATA(t855)[1]=t857;
t857=pick(2);
X t859=TO_N(1); // ICONST
save(t859);
X t860=select1(TO_N(3),t857); // SELECT (simple)
t859=restore();
X t858=select1(t859,t860); // SELECT
t855=top();
S_DATA(t855)[2]=t858;
t858=pick(2);
save(t858);
X t861=allocate(3); // CONS
save(t861);
t858=pick(2);
save(t858);
X t862=allocate(32); // CONS
save(t862);
t858=pick(2);
X t863=lf[280]; // CONST
t862=top();
S_DATA(t862)[0]=t863;
t863=pick(2);
X t864=lf[207]; // CONST
t862=top();
S_DATA(t862)[1]=t864;
t864=pick(2);
X t866=TO_N(2); // ICONST
save(t866);
X t867=select1(TO_N(2),t864); // SELECT (simple)
t866=restore();
X t865=select1(t866,t867); // SELECT
t862=top();
S_DATA(t862)[2]=t865;
t865=pick(2);
X t868=lf[288]; // CONST
t862=top();
S_DATA(t862)[3]=t868;
t868=pick(2);
X t869=lf[280]; // CONST
t862=top();
S_DATA(t862)[4]=t869;
t869=pick(2);
X t870=lf[204]; // CONST
t862=top();
S_DATA(t862)[5]=t870;
t870=pick(2);
X t871=lf[206]; // CONST
t862=top();
S_DATA(t862)[6]=t871;
t871=pick(2);
X t872=lf[205]; // CONST
t862=top();
S_DATA(t862)[7]=t872;
t872=pick(2);
X t873=lf[206]; // CONST
t862=top();
S_DATA(t862)[8]=t873;
t873=pick(2);
X t874=lf[272]; // CONST
t862=top();
S_DATA(t862)[9]=t874;
t874=pick(2);
X t875=lf[280]; // CONST
t862=top();
S_DATA(t862)[10]=t875;
t875=pick(2);
X t876=lf[207]; // CONST
t862=top();
S_DATA(t862)[11]=t876;
t876=pick(2);
X t878=TO_N(3); // ICONST
save(t878);
X t879=select1(TO_N(2),t876); // SELECT (simple)
t878=restore();
X t877=select1(t878,t879); // SELECT
t862=top();
S_DATA(t862)[12]=t877;
t877=pick(2);
X t880=lf[208]; // CONST
t862=top();
S_DATA(t862)[13]=t880;
t880=pick(2);
X t881=lf[280]; // CONST
t862=top();
S_DATA(t862)[14]=t881;
t881=pick(2);
X t882=lf[209]; // CONST
t862=top();
S_DATA(t862)[15]=t882;
t882=pick(2);
X t884=TO_N(2); // ICONST
save(t884);
X t885=select1(TO_N(2),t882); // SELECT (simple)
t884=restore();
X t883=select1(t884,t885); // SELECT
t862=top();
S_DATA(t862)[16]=t883;
t883=pick(2);
X t886=lf[243]; // CONST
t862=top();
S_DATA(t862)[17]=t886;
t886=pick(2);
X t888=TO_N(3); // ICONST
save(t888);
X t889=select1(TO_N(2),t886); // SELECT (simple)
t888=restore();
X t887=select1(t888,t889); // SELECT
t862=top();
S_DATA(t862)[18]=t887;
t887=pick(2);
X t890=lf[210]; // CONST
t862=top();
S_DATA(t862)[19]=t890;
t890=pick(2);
X t892=TO_N(2); // ICONST
save(t892);
X t893=select1(TO_N(2),t890); // SELECT (simple)
t892=restore();
X t891=select1(t892,t893); // SELECT
t862=top();
S_DATA(t862)[20]=t891;
t891=pick(2);
X t894=lf[211]; // CONST
t862=top();
S_DATA(t862)[21]=t894;
t894=pick(2);
X t896=TO_N(2); // ICONST
save(t896);
X t897=select1(TO_N(2),t894); // SELECT (simple)
t896=restore();
X t895=select1(t896,t897); // SELECT
t862=top();
S_DATA(t862)[22]=t895;
t895=pick(2);
X t898=lf[212]; // CONST
t862=top();
S_DATA(t862)[23]=t898;
t898=pick(2);
X t899=lf[280]; // CONST
t862=top();
S_DATA(t862)[24]=t899;
t899=pick(2);
X t900=lf[262]; // CONST
t862=top();
S_DATA(t862)[25]=t900;
t900=pick(2);
X t902=TO_N(1); // ICONST
save(t902);
X t903=select1(TO_N(2),t900); // SELECT (simple)
t902=restore();
X t901=select1(t902,t903); // SELECT
t862=top();
S_DATA(t862)[26]=t901;
t901=pick(2);
X t904=lf[213]; // CONST
t862=top();
S_DATA(t862)[27]=t904;
t904=pick(2);
X t906=TO_N(2); // ICONST
save(t906);
X t907=select1(TO_N(2),t904); // SELECT (simple)
t906=restore();
X t905=select1(t906,t907); // SELECT
t862=top();
S_DATA(t862)[28]=t905;
t905=pick(2);
X t908=lf[214]; // CONST
t862=top();
S_DATA(t862)[29]=t908;
t908=pick(2);
X t910=TO_N(1); // ICONST
save(t910);
X t911=select1(TO_N(3),t908); // SELECT (simple)
t910=restore();
X t909=select1(t910,t911); // SELECT
t862=top();
S_DATA(t862)[30]=t909;
t909=pick(2);
X t912=lf[272]; // CONST
t862=top();
S_DATA(t862)[31]=t912;
t862=restore();
restore();
t861=top();
S_DATA(t861)[0]=t862;
t862=pick(2);
X t914=TO_N(1); // ICONST
save(t914);
X t915=select1(TO_N(2),t862); // SELECT (simple)
t914=restore();
X t913=select1(t914,t915); // SELECT
t861=top();
S_DATA(t861)[1]=t913;
t913=pick(2);
X t916=select1(TO_N(3),t913); // SELECT (simple)
t861=top();
S_DATA(t861)[2]=t916;
t861=restore();
restore();
X t917=___backend_3aconcx(t861); // REF: backend:concx
t855=top();
S_DATA(t855)[3]=t917;
t855=restore();
restore();
save(t855);
X t918=allocate(3); // CONS
save(t918);
t855=pick(2);
X t919=select1(TO_N(2),t855); // SELECT (simple)
t918=top();
S_DATA(t918)[0]=t919;
t919=pick(2);
X t920=select1(TO_N(3),t919); // SELECT (simple)
t918=top();
S_DATA(t918)[1]=t920;
t920=pick(2);
save(t920);
X t921=allocate(2); // CONS
save(t921);
t920=pick(2);
X t923=TO_N(4); // ICONST
save(t923);
X t924=select1(TO_N(1),t920); // SELECT (simple)
t923=restore();
X t922=select1(t923,t924); // SELECT
t921=top();
S_DATA(t921)[0]=t922;
t922=pick(2);
X t925=select1(TO_N(4),t922); // SELECT (simple)
X t926=___backend_3anontail(t925); // REF: backend:nontail
t921=top();
S_DATA(t921)[1]=t926;
t921=restore();
restore();
X t927=___backend_3at_5fexpr(t921); // REF: backend:t_expr
t918=top();
S_DATA(t918)[2]=t927;
t918=restore();
restore();
save(t918);
X t928=allocate(3); // CONS
save(t928);
t918=pick(2);
save(t918);
X t929=allocate(13); // CONS
save(t929);
t918=pick(2);
X t930=lf[280]; // CONST
t929=top();
S_DATA(t929)[0]=t930;
t930=pick(2);
X t931=select1(TO_N(2),t930); // SELECT (simple)
t929=top();
S_DATA(t929)[1]=t931;
t931=pick(2);
X t932=lf[243]; // CONST
t929=top();
S_DATA(t929)[2]=t932;
t932=pick(2);
X t934=TO_N(1); // ICONST
save(t934);
X t935=select1(TO_N(3),t932); // SELECT (simple)
t934=restore();
X t933=select1(t934,t935); // SELECT
t929=top();
S_DATA(t929)[3]=t933;
t933=pick(2);
X t936=lf[229]; // CONST
t929=top();
S_DATA(t929)[4]=t936;
t936=pick(2);
X t937=lf[280]; // CONST
t929=top();
S_DATA(t929)[5]=t937;
t937=pick(2);
X t938=lf[256]; // CONST
t929=top();
S_DATA(t929)[6]=t938;
t938=pick(2);
X t939=lf[280]; // CONST
t929=top();
S_DATA(t929)[7]=t939;
t939=pick(2);
X t940=lf[262]; // CONST
t929=top();
S_DATA(t929)[8]=t940;
t940=pick(2);
X t942=TO_N(4); // ICONST
save(t942);
X t943=select1(TO_N(1),t940); // SELECT (simple)
t942=restore();
X t941=select1(t942,t943); // SELECT
t929=top();
S_DATA(t929)[9]=t941;
t941=pick(2);
X t944=lf[243]; // CONST
t929=top();
S_DATA(t929)[10]=t944;
t944=pick(2);
X t945=select1(TO_N(2),t944); // SELECT (simple)
t929=top();
S_DATA(t929)[11]=t945;
t945=pick(2);
X t946=lf[288]; // CONST
t929=top();
S_DATA(t929)[12]=t946;
t929=restore();
restore();
t928=top();
S_DATA(t928)[0]=t929;
t929=pick(2);
X t948=TO_N(4); // ICONST
save(t948);
X t949=select1(TO_N(1),t929); // SELECT (simple)
t948=restore();
X t947=select1(t948,t949); // SELECT
t928=top();
S_DATA(t928)[1]=t947;
t947=pick(2);
X t950=select1(TO_N(3),t947); // SELECT (simple)
t928=top();
S_DATA(t928)[2]=t950;
t928=restore();
restore();
X t951=___backend_3aconcx(t928); // REF: backend:concx
t819=t951;
}else{
save(x); // COND
X t954=lf[293]; // CONST
save(t954);
X t956=TO_N(1); // ICONST
save(t956);
X t957=select1(TO_N(1),x); // SELECT (simple)
t956=restore();
X t955=select1(t956,t957); // SELECT
t954=restore();
X t953=(t954==t955)||eq1(t954,t955)?T:F; // EQ
X t952;
x=restore();
if(t953!=F){
save(x);
X t958=allocate(2); // CONS
save(t958);
x=pick(2);
X t959=select1(TO_N(1),x); // SELECT (simple)
t958=top();
S_DATA(t958)[0]=t959;
t959=pick(2);
X t960=select1(TO_N(2),t959); // SELECT (simple)
X t961=___backend_3agensym(t960); // REF: backend:gensym
t958=top();
S_DATA(t958)[1]=t961;
t958=restore();
restore();
X t962=___al(t958); // REF: al
save(t962); // COND
save(t962);
X t966=TO_N(3); // ICONST
save(t966);
X t967=select1(TO_N(1),t962); // SELECT (simple)
t966=restore();
X t965=select1(t966,t967); // SELECT
t962=restore();
save(t965);
X t969=TO_N(3); // ICONST
save(t969);
X t970=select1(TO_N(3),t962); // SELECT (simple)
t969=restore();
X t968=select1(t969,t970); // SELECT
t965=restore();
X t964=(t965==t968)||eq1(t965,t968)?T:F; // EQ
X t963;
t962=restore();
if(t964!=F){
save(t962);
X t971=allocate(3); // CONS
save(t971);
t962=pick(2);
save(t962);
X t972=allocate(12); // CONS
save(t972);
t962=pick(2);
X t973=lf[280]; // CONST
t972=top();
S_DATA(t972)[0]=t973;
t973=pick(2);
X t974=lf[216]; // CONST
t972=top();
S_DATA(t972)[1]=t974;
t974=pick(2);
X t976=TO_N(1); // ICONST
save(t976);
X t977=select1(TO_N(3),t974); // SELECT (simple)
t976=restore();
X t975=select1(t976,t977); // SELECT
t972=top();
S_DATA(t972)[2]=t975;
t975=pick(2);
X t978=lf[217]; // CONST
t972=top();
S_DATA(t972)[3]=t978;
t978=pick(2);
X t980=TO_N(3); // ICONST
save(t980);
X t981=select1(TO_N(1),t978); // SELECT (simple)
t980=restore();
X t979=select1(t980,t981); // SELECT
X t982=___tos(t979); // REF: tos
t972=top();
S_DATA(t972)[4]=t982;
t982=pick(2);
X t983=lf[280]; // CONST
t972=top();
S_DATA(t972)[5]=t983;
t983=pick(2);
X t984=lf[218]; // CONST
t972=top();
S_DATA(t972)[6]=t984;
t984=pick(2);
X t986=TO_N(3); // ICONST
save(t986);
X t987=select1(TO_N(1),t984); // SELECT (simple)
t986=restore();
X t985=select1(t986,t987); // SELECT
X t988=___tos(t985); // REF: tos
t972=top();
S_DATA(t972)[7]=t988;
t988=pick(2);
X t989=lf[280]; // CONST
t972=top();
S_DATA(t972)[8]=t989;
t989=pick(2);
X t990=lf[262]; // CONST
t972=top();
S_DATA(t972)[9]=t990;
t990=pick(2);
X t991=select1(TO_N(2),t990); // SELECT (simple)
t972=top();
S_DATA(t972)[10]=t991;
t991=pick(2);
X t992=lf[288]; // CONST
t972=top();
S_DATA(t972)[11]=t992;
t972=restore();
restore();
t971=top();
S_DATA(t971)[0]=t972;
t972=pick(2);
X t993=select1(TO_N(2),t972); // SELECT (simple)
t971=top();
S_DATA(t971)[1]=t993;
t993=pick(2);
X t994=select1(TO_N(3),t993); // SELECT (simple)
t971=top();
S_DATA(t971)[2]=t994;
t971=restore();
restore();
t963=t971;
}else{
save(t962);
X t995=allocate(3); // CONS
save(t995);
t962=pick(2);
save(t962);
X t996=allocate(9); // CONS
save(t996);
t962=pick(2);
X t997=lf[280]; // CONST
t996=top();
S_DATA(t996)[0]=t997;
t997=pick(2);
X t998=lf[262]; // CONST
t996=top();
S_DATA(t996)[1]=t998;
t998=pick(2);
X t999=select1(TO_N(2),t998); // SELECT (simple)
t996=top();
S_DATA(t996)[2]=t999;
t999=pick(2);
X t1000=lf[243]; // CONST
t996=top();
S_DATA(t996)[3]=t1000;
t1000=pick(2);
X t1002=TO_N(3); // ICONST
save(t1002);
X t1003=select1(TO_N(1),t1000); // SELECT (simple)
t1002=restore();
X t1001=select1(t1002,t1003); // SELECT
X t1004=___backend_3amangle(t1001); // REF: backend:mangle
t996=top();
S_DATA(t996)[4]=t1004;
t1004=pick(2);
X t1005=lf[285]; // CONST
t996=top();
S_DATA(t996)[5]=t1005;
t1005=pick(2);
X t1007=TO_N(1); // ICONST
save(t1007);
X t1008=select1(TO_N(3),t1005); // SELECT (simple)
t1007=restore();
X t1006=select1(t1007,t1008); // SELECT
t996=top();
S_DATA(t996)[6]=t1006;
t1006=pick(2);
X t1009=lf[219]; // CONST
t996=top();
S_DATA(t996)[7]=t1009;
t1009=pick(2);
X t1011=TO_N(3); // ICONST
save(t1011);
X t1012=select1(TO_N(1),t1009); // SELECT (simple)
t1011=restore();
X t1010=select1(t1011,t1012); // SELECT
X t1013=___tos(t1010); // REF: tos
t996=top();
S_DATA(t996)[8]=t1013;
t996=restore();
restore();
t995=top();
S_DATA(t995)[0]=t996;
t996=pick(2);
X t1014=select1(TO_N(2),t996); // SELECT (simple)
t995=top();
S_DATA(t995)[1]=t1014;
t1014=pick(2);
X t1015=select1(TO_N(3),t1014); // SELECT (simple)
t995=top();
S_DATA(t995)[2]=t1015;
t995=restore();
restore();
t963=t995;}
X t1016=___backend_3aconcx(t963); // REF: backend:concx
t952=t1016;
}else{
save(x); // COND
X t1019=lf[324]; // CONST
save(t1019);
X t1021=TO_N(1); // ICONST
save(t1021);
X t1022=select1(TO_N(1),x); // SELECT (simple)
t1021=restore();
X t1020=select1(t1021,t1022); // SELECT
t1019=restore();
X t1018=(t1019==t1020)||eq1(t1019,t1020)?T:F; // EQ
X t1017;
x=restore();
if(t1018!=F){
save(x);
X t1023=allocate(2); // CONS
save(t1023);
x=pick(2);
save(x);
X t1024=allocate(2); // CONS
save(t1024);
x=pick(2);
X t1025=select1(TO_N(1),x); // SELECT (simple)
t1024=top();
S_DATA(t1024)[0]=t1025;
t1025=pick(2);
X t1027=TO_N(3); // ICONST
save(t1027);
X t1028=select1(TO_N(1),t1025); // SELECT (simple)
t1027=restore();
X t1026=select1(t1027,t1028); // SELECT
X t1029=___backend_3amangle(t1026); // REF: backend:mangle
t1024=top();
S_DATA(t1024)[1]=t1029;
t1024=restore();
restore();
t1023=top();
S_DATA(t1023)[0]=t1024;
t1024=pick(2);
X t1030=select1(TO_N(2),t1024); // SELECT (simple)
X t1031=___backend_3agensym(t1030); // REF: backend:gensym
t1023=top();
S_DATA(t1023)[1]=t1031;
t1023=restore();
restore();
X t1032=___cat(t1023); // REF: cat
save(t1032);
X t1033=allocate(3); // CONS
save(t1033);
t1032=pick(2);
save(t1032);
X t1034=allocate(11); // CONS
save(t1034);
t1032=pick(2);
X t1035=lf[280]; // CONST
t1034=top();
S_DATA(t1034)[0]=t1035;
t1035=pick(2);
X t1036=lf[262]; // CONST
t1034=top();
S_DATA(t1034)[1]=t1036;
t1036=pick(2);
X t1037=select1(TO_N(2),t1036); // SELECT (simple)
t1034=top();
S_DATA(t1034)[2]=t1037;
t1037=pick(2);
X t1038=lf[220]; // CONST
t1034=top();
S_DATA(t1034)[3]=t1038;
t1038=pick(2);
X t1039=lf[280]; // CONST
t1034=top();
S_DATA(t1034)[4]=t1039;
t1039=pick(2);
X t1040=lf[262]; // CONST
t1034=top();
S_DATA(t1034)[5]=t1040;
t1040=pick(2);
X t1041=select1(TO_N(3),t1040); // SELECT (simple)
t1034=top();
S_DATA(t1034)[6]=t1041;
t1041=pick(2);
X t1042=lf[221]; // CONST
t1034=top();
S_DATA(t1034)[7]=t1042;
t1042=pick(2);
X t1043=select1(TO_N(2),t1042); // SELECT (simple)
t1034=top();
S_DATA(t1034)[8]=t1043;
t1043=pick(2);
X t1044=lf[222]; // CONST
t1034=top();
S_DATA(t1034)[9]=t1044;
t1044=pick(2);
X t1046=TO_N(3); // ICONST
save(t1046);
X t1047=select1(TO_N(1),t1044); // SELECT (simple)
t1046=restore();
X t1045=select1(t1046,t1047); // SELECT
X t1048=___tos(t1045); // REF: tos
t1034=top();
S_DATA(t1034)[10]=t1048;
t1034=restore();
restore();
t1033=top();
S_DATA(t1033)[0]=t1034;
t1034=pick(2);
X t1049=select1(TO_N(3),t1034); // SELECT (simple)
t1033=top();
S_DATA(t1033)[1]=t1049;
t1049=pick(2);
X t1050=select1(TO_N(4),t1049); // SELECT (simple)
t1033=top();
S_DATA(t1033)[2]=t1050;
t1033=restore();
restore();
X t1051=___backend_3aconcx(t1033); // REF: backend:concx
t1017=t1051;
}else{
save(x); // COND
X t1054=lf[309]; // CONST
save(t1054);
X t1056=TO_N(1); // ICONST
save(t1056);
X t1057=select1(TO_N(1),x); // SELECT (simple)
t1056=restore();
X t1055=select1(t1056,t1057); // SELECT
t1054=restore();
X t1053=(t1054==t1055)||eq1(t1054,t1055)?T:F; // EQ
X t1052;
x=restore();
if(t1053!=F){
save(x);
X t1058=allocate(4); // CONS
save(t1058);
x=pick(2);
X t1059=select1(TO_N(1),x); // SELECT (simple)
t1058=top();
S_DATA(t1058)[0]=t1059;
t1059=pick(2);
X t1060=select1(TO_N(2),t1059); // SELECT (simple)
t1058=top();
S_DATA(t1058)[1]=t1060;
t1060=pick(2);
save(t1060);
X t1061=allocate(2); // CONS
save(t1061);
t1060=pick(2);
X t1063=TO_N(3); // ICONST
save(t1063);
X t1064=select1(TO_N(1),t1060); // SELECT (simple)
t1063=restore();
X t1062=select1(t1063,t1064); // SELECT
t1061=top();
S_DATA(t1061)[0]=t1062;
t1062=pick(2);
save(t1062);
X t1065=allocate(2); // CONS
save(t1065);
t1062=pick(2);
save(t1062);
X t1066=allocate(4); // CONS
save(t1066);
t1062=pick(2);
X t1067=lf[280]; // CONST
t1066=top();
S_DATA(t1066)[0]=t1067;
t1067=pick(2);
X t1068=lf[271]; // CONST
t1066=top();
S_DATA(t1066)[1]=t1068;
t1068=pick(2);
X t1070=TO_N(1); // ICONST
save(t1070);
X t1071=select1(TO_N(2),t1068); // SELECT (simple)
t1070=restore();
X t1069=select1(t1070,t1071); // SELECT
t1066=top();
S_DATA(t1066)[2]=t1069;
t1069=pick(2);
X t1072=lf[223]; // CONST
t1066=top();
S_DATA(t1066)[3]=t1072;
t1066=restore();
restore();
t1065=top();
S_DATA(t1065)[0]=t1066;
t1066=pick(2);
X t1073=select1(TO_N(2),t1066); // SELECT (simple)
t1065=top();
S_DATA(t1065)[1]=t1073;
t1065=restore();
restore();
X t1074=___backend_3aconc(t1065); // REF: backend:conc
X t1075=___backend_3anontail(t1074); // REF: backend:nontail
t1061=top();
S_DATA(t1061)[1]=t1075;
t1061=restore();
restore();
X t1076=___backend_3at_5fexpr(t1061); // REF: backend:t_expr
t1058=top();
S_DATA(t1058)[2]=t1076;
t1076=pick(2);
X t1078=TO_N(3); // ICONST
save(t1078);
X t1079=select1(TO_N(2),t1076); // SELECT (simple)
t1078=restore();
X t1077=select1(t1078,t1079); // SELECT
t1058=top();
S_DATA(t1058)[3]=t1077;
t1058=restore();
restore();
save(t1058);
X t1080=allocate(4); // CONS
save(t1080);
t1058=pick(2);
X t1081=select1(TO_N(1),t1058); // SELECT (simple)
t1080=top();
S_DATA(t1080)[0]=t1081;
t1081=pick(2);
X t1082=select1(TO_N(2),t1081); // SELECT (simple)
t1080=top();
S_DATA(t1080)[1]=t1082;
t1082=pick(2);
save(t1082);
X t1083=allocate(2); // CONS
save(t1083);
t1082=pick(2);
save(t1082);
X t1084=allocate(3); // CONS
save(t1084);
t1082=pick(2);
X t1085=lf[280]; // CONST
t1084=top();
S_DATA(t1084)[0]=t1085;
t1085=pick(2);
X t1087=TO_N(1); // ICONST
save(t1087);
X t1088=select1(TO_N(2),t1085); // SELECT (simple)
t1087=restore();
X t1086=select1(t1087,t1088); // SELECT
t1084=top();
S_DATA(t1084)[1]=t1086;
t1086=pick(2);
X t1089=lf[273]; // CONST
t1084=top();
S_DATA(t1084)[2]=t1089;
t1084=restore();
restore();
t1083=top();
S_DATA(t1083)[0]=t1084;
t1084=pick(2);
X t1090=select1(TO_N(3),t1084); // SELECT (simple)
t1083=top();
S_DATA(t1083)[1]=t1090;
t1083=restore();
restore();
X t1091=___backend_3aconc(t1083); // REF: backend:conc
t1080=top();
S_DATA(t1080)[2]=t1091;
t1091=pick(2);
X t1092=select1(TO_N(4),t1091); // SELECT (simple)
t1080=top();
S_DATA(t1080)[3]=t1092;
t1080=restore();
restore();
save(t1080);
X t1093=allocate(2); // CONS
save(t1093);
t1080=pick(2);
X t1095=TO_N(4); // ICONST
save(t1095);
X t1096=select1(TO_N(1),t1080); // SELECT (simple)
t1095=restore();
X t1094=select1(t1095,t1096); // SELECT
t1093=top();
S_DATA(t1093)[0]=t1094;
t1094=pick(2);
save(t1094);
X t1097=allocate(2); // CONS
save(t1097);
t1094=pick(2);
X t1099=TO_N(1); // ICONST
save(t1099);
X t1100=select1(TO_N(2),t1094); // SELECT (simple)
t1099=restore();
X t1098=select1(t1099,t1100); // SELECT
t1097=top();
S_DATA(t1097)[0]=t1098;
t1098=pick(2);
save(t1098);
X t1101=allocate(2); // CONS
save(t1101);
t1098=pick(2);
X t1102=select1(TO_N(4),t1098); // SELECT (simple)
t1101=top();
S_DATA(t1101)[0]=t1102;
t1102=pick(2);
X t1103=select1(TO_N(3),t1102); // SELECT (simple)
t1101=top();
S_DATA(t1101)[1]=t1103;
t1101=restore();
restore();
X t1104=___backend_3atail(t1101); // REF: backend:tail
X t1105=___tl(t1104); // REF: tl
t1097=top();
S_DATA(t1097)[1]=t1105;
t1097=restore();
restore();
X t1106=___al(t1097); // REF: al
t1093=top();
S_DATA(t1093)[1]=t1106;
t1093=restore();
restore();
x=t1093; // REF: backend:t_expr
goto loop; // tail call: backend:t_expr
X t1107;
t1052=t1107;
}else{
save(x); // COND
X t1110=lf[315]; // CONST
save(t1110);
X t1112=TO_N(1); // ICONST
save(t1112);
X t1113=select1(TO_N(1),x); // SELECT (simple)
t1112=restore();
X t1111=select1(t1112,t1113); // SELECT
t1110=restore();
X t1109=(t1110==t1111)||eq1(t1110,t1111)?T:F; // EQ
X t1108;
x=restore();
if(t1109!=F){
save(x);
X t1114=allocate(4); // CONS
save(t1114);
x=pick(2);
X t1115=select1(TO_N(1),x); // SELECT (simple)
t1114=top();
S_DATA(t1114)[0]=t1115;
t1115=pick(2);
X t1117=TO_N(1); // ICONST
save(t1117);
X t1118=select1(TO_N(2),t1115); // SELECT (simple)
t1117=restore();
X t1116=select1(t1117,t1118); // SELECT
t1114=top();
S_DATA(t1114)[1]=t1116;
t1116=pick(2);
X t1120=TO_N(3); // ICONST
save(t1120);
X t1121=select1(TO_N(2),t1116); // SELECT (simple)
t1120=restore();
X t1119=select1(t1120,t1121); // SELECT
t1114=top();
S_DATA(t1114)[2]=t1119;
t1119=pick(2);
X t1122=select1(TO_N(2),t1119); // SELECT (simple)
X t1123=___backend_3anontail(t1122); // REF: backend:nontail
X t1124=___backend_3agensym(t1123); // REF: backend:gensym
t1114=top();
S_DATA(t1114)[3]=t1124;
t1114=restore();
restore();
save(t1114);
X t1125=allocate(5); // CONS
save(t1125);
t1114=pick(2);
X t1126=select1(TO_N(1),t1114); // SELECT (simple)
t1125=top();
S_DATA(t1125)[0]=t1126;
t1126=pick(2);
X t1127=select1(TO_N(2),t1126); // SELECT (simple)
t1125=top();
S_DATA(t1125)[1]=t1127;
t1127=pick(2);
X t1128=select1(TO_N(3),t1127); // SELECT (simple)
t1125=top();
S_DATA(t1125)[2]=t1128;
t1128=pick(2);
X t1130=TO_N(1); // ICONST
save(t1130);
X t1131=select1(TO_N(4),t1128); // SELECT (simple)
t1130=restore();
X t1129=select1(t1130,t1131); // SELECT
t1125=top();
S_DATA(t1125)[3]=t1129;
t1129=pick(2);
save(t1129);
X t1132=allocate(2); // CONS
save(t1132);
t1129=pick(2);
X t1134=TO_N(3); // ICONST
save(t1134);
X t1135=select1(TO_N(1),t1129); // SELECT (simple)
t1134=restore();
X t1133=select1(t1134,t1135); // SELECT
t1132=top();
S_DATA(t1132)[0]=t1133;
t1133=pick(2);
save(t1133);
X t1136=allocate(2); // CONS
save(t1136);
t1133=pick(2);
save(t1133);
X t1137=allocate(4); // CONS
save(t1137);
t1133=pick(2);
X t1138=lf[280]; // CONST
t1137=top();
S_DATA(t1137)[0]=t1138;
t1138=pick(2);
X t1139=lf[271]; // CONST
t1137=top();
S_DATA(t1137)[1]=t1139;
t1139=pick(2);
X t1140=select1(TO_N(2),t1139); // SELECT (simple)
t1137=top();
S_DATA(t1137)[2]=t1140;
t1140=pick(2);
X t1141=lf[225]; // CONST
t1137=top();
S_DATA(t1137)[3]=t1141;
t1137=restore();
restore();
t1136=top();
S_DATA(t1136)[0]=t1137;
t1137=pick(2);
X t1143=TO_N(2); // ICONST
save(t1143);
X t1144=select1(TO_N(4),t1137); // SELECT (simple)
t1143=restore();
X t1142=select1(t1143,t1144); // SELECT
t1136=top();
S_DATA(t1136)[1]=t1142;
t1136=restore();
restore();
X t1145=___backend_3aconc(t1136); // REF: backend:conc
t1132=top();
S_DATA(t1132)[1]=t1145;
t1132=restore();
restore();
X t1146=___backend_3at_5fexpr(t1132); // REF: backend:t_expr
t1125=top();
S_DATA(t1125)[4]=t1146;
t1125=restore();
restore();
save(t1125);
X t1147=allocate(3); // CONS
save(t1147);
t1125=pick(2);
X t1148=select1(TO_N(3),t1125); // SELECT (simple)
t1147=top();
S_DATA(t1147)[0]=t1148;
t1148=pick(2);
X t1149=select1(TO_N(4),t1148); // SELECT (simple)
t1147=top();
S_DATA(t1147)[1]=t1149;
t1149=pick(2);
save(t1149);
X t1150=allocate(2); // CONS
save(t1150);
t1149=pick(2);
X t1152=TO_N(4); // ICONST
save(t1152);
X t1153=select1(TO_N(1),t1149); // SELECT (simple)
t1152=restore();
X t1151=select1(t1152,t1153); // SELECT
t1150=top();
S_DATA(t1150)[0]=t1151;
t1151=pick(2);
save(t1151);
X t1154=allocate(2); // CONS
save(t1154);
t1151=pick(2);
X t1155=select1(TO_N(3),t1151); // SELECT (simple)
t1154=top();
S_DATA(t1154)[0]=t1155;
t1155=pick(2);
save(t1155);
X t1156=allocate(3); // CONS
save(t1156);
t1155=pick(2);
save(t1155);
X t1157=allocate(13); // CONS
save(t1157);
t1155=pick(2);
X t1158=lf[280]; // CONST
t1157=top();
S_DATA(t1157)[0]=t1158;
t1158=pick(2);
X t1159=lf[262]; // CONST
t1157=top();
S_DATA(t1157)[1]=t1159;
t1159=pick(2);
X t1160=select1(TO_N(4),t1159); // SELECT (simple)
t1157=top();
S_DATA(t1157)[2]=t1160;
t1160=pick(2);
X t1161=lf[243]; // CONST
t1157=top();
S_DATA(t1157)[3]=t1161;
t1161=pick(2);
X t1163=TO_N(1); // ICONST
save(t1163);
X t1164=select1(TO_N(5),t1161); // SELECT (simple)
t1163=restore();
X t1162=select1(t1163,t1164); // SELECT
t1157=top();
S_DATA(t1157)[4]=t1162;
t1162=pick(2);
X t1165=lf[288]; // CONST
t1157=top();
S_DATA(t1157)[5]=t1165;
t1165=pick(2);
X t1166=lf[280]; // CONST
t1157=top();
S_DATA(t1157)[6]=t1166;
t1166=pick(2);
X t1167=select1(TO_N(2),t1166); // SELECT (simple)
t1157=top();
S_DATA(t1157)[7]=t1167;
t1167=pick(2);
X t1168=lf[273]; // CONST
t1157=top();
S_DATA(t1157)[8]=t1168;
t1168=pick(2);
X t1169=lf[280]; // CONST
t1157=top();
S_DATA(t1157)[9]=t1169;
t1169=pick(2);
X t1170=lf[258]; // CONST
t1157=top();
S_DATA(t1157)[10]=t1170;
t1170=pick(2);
X t1171=select1(TO_N(4),t1170); // SELECT (simple)
t1157=top();
S_DATA(t1157)[11]=t1171;
t1171=pick(2);
X t1172=lf[224]; // CONST
t1157=top();
S_DATA(t1157)[12]=t1172;
t1157=restore();
restore();
t1156=top();
S_DATA(t1156)[0]=t1157;
t1157=pick(2);
X t1173=select1(TO_N(2),t1157); // SELECT (simple)
t1156=top();
S_DATA(t1156)[1]=t1173;
t1173=pick(2);
X t1174=select1(TO_N(5),t1173); // SELECT (simple)
t1156=top();
S_DATA(t1156)[2]=t1174;
t1156=restore();
restore();
X t1175=___backend_3aconcx(t1156); // REF: backend:concx
t1154=top();
S_DATA(t1154)[1]=t1175;
t1154=restore();
restore();
X t1176=___backend_3atail(t1154); // REF: backend:tail
t1150=top();
S_DATA(t1150)[1]=t1176;
t1150=restore();
restore();
X t1177=___backend_3at_5fexpr(t1150); // REF: backend:t_expr
t1147=top();
S_DATA(t1147)[2]=t1177;
t1147=restore();
restore();
save(t1147);
X t1178=allocate(3); // CONS
save(t1178);
t1147=pick(2);
save(t1147);
X t1179=allocate(5); // CONS
save(t1179);
t1147=pick(2);
X t1180=lf[280]; // CONST
t1179=top();
S_DATA(t1179)[0]=t1180;
t1180=pick(2);
X t1181=select1(TO_N(2),t1180); // SELECT (simple)
t1179=top();
S_DATA(t1179)[1]=t1181;
t1181=pick(2);
X t1182=lf[243]; // CONST
t1179=top();
S_DATA(t1179)[2]=t1182;
t1182=pick(2);
X t1184=TO_N(1); // ICONST
save(t1184);
X t1185=select1(TO_N(3),t1182); // SELECT (simple)
t1184=restore();
X t1183=select1(t1184,t1185); // SELECT
t1179=top();
S_DATA(t1179)[3]=t1183;
t1183=pick(2);
X t1186=lf[229]; // CONST
t1179=top();
S_DATA(t1179)[4]=t1186;
t1179=restore();
restore();
t1178=top();
S_DATA(t1178)[0]=t1179;
t1179=pick(2);
X t1187=select1(TO_N(2),t1179); // SELECT (simple)
t1178=top();
S_DATA(t1178)[1]=t1187;
t1187=pick(2);
X t1188=select1(TO_N(3),t1187); // SELECT (simple)
t1178=top();
S_DATA(t1178)[2]=t1188;
t1178=restore();
restore();
X t1189=___backend_3aconcx(t1178); // REF: backend:concx
t1108=t1189;
}else{
save(x); // COND
X t1192=lf[305]; // CONST
save(t1192);
X t1194=TO_N(1); // ICONST
save(t1194);
X t1195=select1(TO_N(1),x); // SELECT (simple)
t1194=restore();
X t1193=select1(t1194,t1195); // SELECT
t1192=restore();
X t1191=(t1192==t1193)||eq1(t1192,t1193)?T:F; // EQ
X t1190;
x=restore();
if(t1191!=F){
save(x);
X t1196=allocate(2); // CONS
save(t1196);
x=pick(2);
X t1197=select1(TO_N(1),x); // SELECT (simple)
t1196=top();
S_DATA(t1196)[0]=t1197;
t1197=pick(2);
X t1198=select1(TO_N(2),t1197); // SELECT (simple)
X t1199=___backend_3agensym(t1198); // REF: backend:gensym
t1196=top();
S_DATA(t1196)[1]=t1199;
t1196=restore();
restore();
X t1200=___al(t1196); // REF: al
save(t1200);
X t1201=allocate(3); // CONS
save(t1201);
t1200=pick(2);
X t1202=select1(TO_N(1),t1200); // SELECT (simple)
t1201=top();
S_DATA(t1201)[0]=t1202;
t1202=pick(2);
X t1203=select1(TO_N(2),t1202); // SELECT (simple)
t1201=top();
S_DATA(t1201)[1]=t1203;
t1203=pick(2);
save(t1203);
X t1204=allocate(3); // CONS
save(t1204);
t1203=pick(2);
save(t1203);
X t1205=allocate(12); // CONS
save(t1205);
t1203=pick(2);
X t1206=lf[280]; // CONST
t1205=top();
S_DATA(t1205)[0]=t1206;
t1206=pick(2);
X t1207=lf[262]; // CONST
t1205=top();
S_DATA(t1205)[1]=t1207;
t1207=pick(2);
X t1208=select1(TO_N(2),t1207); // SELECT (simple)
t1205=top();
S_DATA(t1205)[2]=t1208;
t1208=pick(2);
X t1209=lf[243]; // CONST
t1205=top();
S_DATA(t1205)[3]=t1209;
t1209=pick(2);
X t1211=TO_N(1); // ICONST
save(t1211);
X t1212=select1(TO_N(3),t1209); // SELECT (simple)
t1211=restore();
X t1210=select1(t1211,t1212); // SELECT
t1205=top();
S_DATA(t1205)[4]=t1210;
t1210=pick(2);
X t1213=lf[227]; // CONST
t1205=top();
S_DATA(t1205)[5]=t1213;
t1213=pick(2);
X t1214=lf[280]; // CONST
t1205=top();
S_DATA(t1205)[6]=t1214;
t1214=pick(2);
X t1215=lf[228]; // CONST
t1205=top();
S_DATA(t1205)[7]=t1215;
t1215=pick(2);
X t1216=lf[280]; // CONST
t1205=top();
S_DATA(t1205)[8]=t1216;
t1216=pick(2);
X t1217=lf[271]; // CONST
t1205=top();
S_DATA(t1205)[9]=t1217;
t1217=pick(2);
X t1218=select1(TO_N(2),t1217); // SELECT (simple)
t1205=top();
S_DATA(t1205)[10]=t1218;
t1218=pick(2);
X t1219=lf[272]; // CONST
t1205=top();
S_DATA(t1205)[11]=t1219;
t1205=restore();
restore();
t1204=top();
S_DATA(t1204)[0]=t1205;
t1205=pick(2);
X t1220=select1(TO_N(2),t1205); // SELECT (simple)
t1204=top();
S_DATA(t1204)[1]=t1220;
t1220=pick(2);
X t1221=select1(TO_N(3),t1220); // SELECT (simple)
t1204=top();
S_DATA(t1204)[2]=t1221;
t1204=restore();
restore();
X t1222=___backend_3aconcx(t1204); // REF: backend:concx
t1201=top();
S_DATA(t1201)[2]=t1222;
t1201=restore();
restore();
save(t1201);
X t1223=allocate(3); // CONS
save(t1223);
t1201=pick(2);
X t1224=select1(TO_N(1),t1201); // SELECT (simple)
t1223=top();
S_DATA(t1223)[0]=t1224;
t1224=pick(2);
X t1225=select1(TO_N(2),t1224); // SELECT (simple)
t1223=top();
S_DATA(t1223)[1]=t1225;
t1225=pick(2);
save(t1225);
X t1226=allocate(2); // CONS
save(t1226);
t1225=pick(2);
X t1228=TO_N(3); // ICONST
save(t1228);
X t1229=select1(TO_N(1),t1225); // SELECT (simple)
t1228=restore();
X t1227=select1(t1228,t1229); // SELECT
t1226=top();
S_DATA(t1226)[0]=t1227;
t1227=pick(2);
X t1230=select1(TO_N(3),t1227); // SELECT (simple)
X t1231=___backend_3anontail(t1230); // REF: backend:nontail
t1226=top();
S_DATA(t1226)[1]=t1231;
t1226=restore();
restore();
X t1232=___backend_3at_5fexpr(t1226); // REF: backend:t_expr
t1223=top();
S_DATA(t1223)[2]=t1232;
t1223=restore();
restore();
save(t1223);
X t1233=allocate(3); // CONS
save(t1233);
t1223=pick(2);
X t1234=select1(TO_N(1),t1223); // SELECT (simple)
t1233=top();
S_DATA(t1233)[0]=t1234;
t1234=pick(2);
X t1235=select1(TO_N(2),t1234); // SELECT (simple)
t1233=top();
S_DATA(t1233)[1]=t1235;
t1235=pick(2);
save(t1235);
X t1236=allocate(3); // CONS
save(t1236);
t1235=pick(2);
save(t1235);
X t1237=allocate(7); // CONS
save(t1237);
t1235=pick(2);
X t1238=lf[280]; // CONST
t1237=top();
S_DATA(t1237)[0]=t1238;
t1238=pick(2);
X t1239=select1(TO_N(2),t1238); // SELECT (simple)
t1237=top();
S_DATA(t1237)[1]=t1239;
t1239=pick(2);
X t1240=lf[273]; // CONST
t1237=top();
S_DATA(t1237)[2]=t1240;
t1240=pick(2);
X t1241=lf[280]; // CONST
t1237=top();
S_DATA(t1237)[3]=t1241;
t1241=pick(2);
X t1242=lf[258]; // CONST
t1237=top();
S_DATA(t1237)[4]=t1242;
t1242=pick(2);
X t1244=TO_N(1); // ICONST
save(t1244);
X t1245=select1(TO_N(3),t1242); // SELECT (simple)
t1244=restore();
X t1243=select1(t1244,t1245); // SELECT
t1237=top();
S_DATA(t1237)[5]=t1243;
t1243=pick(2);
X t1246=lf[226]; // CONST
t1237=top();
S_DATA(t1237)[6]=t1246;
t1237=restore();
restore();
t1236=top();
S_DATA(t1236)[0]=t1237;
t1237=pick(2);
X t1247=select1(TO_N(2),t1237); // SELECT (simple)
t1236=top();
S_DATA(t1236)[1]=t1247;
t1247=pick(2);
X t1248=select1(TO_N(3),t1247); // SELECT (simple)
t1236=top();
S_DATA(t1236)[2]=t1248;
t1236=restore();
restore();
X t1249=___backend_3aconcx(t1236); // REF: backend:concx
t1233=top();
S_DATA(t1233)[2]=t1249;
t1233=restore();
restore();
save(t1233);
X t1250=allocate(2); // CONS
save(t1250);
t1233=pick(2);
X t1251=select1(TO_N(2),t1233); // SELECT (simple)
t1250=top();
S_DATA(t1250)[0]=t1251;
t1251=pick(2);
save(t1251);
X t1252=allocate(2); // CONS
save(t1252);
t1251=pick(2);
X t1254=TO_N(4); // ICONST
save(t1254);
X t1255=select1(TO_N(1),t1251); // SELECT (simple)
t1254=restore();
X t1253=select1(t1254,t1255); // SELECT
t1252=top();
S_DATA(t1252)[0]=t1253;
t1253=pick(2);
X t1256=select1(TO_N(3),t1253); // SELECT (simple)
X t1257=___backend_3anontail(t1256); // REF: backend:nontail
t1252=top();
S_DATA(t1252)[1]=t1257;
t1252=restore();
restore();
X t1258=___backend_3at_5fexpr(t1252); // REF: backend:t_expr
t1250=top();
S_DATA(t1250)[1]=t1258;
t1250=restore();
restore();
save(t1250);
X t1259=allocate(3); // CONS
save(t1259);
t1250=pick(2);
save(t1250);
X t1260=allocate(5); // CONS
save(t1260);
t1250=pick(2);
X t1261=lf[280]; // CONST
t1260=top();
S_DATA(t1260)[0]=t1261;
t1261=pick(2);
X t1262=select1(TO_N(1),t1261); // SELECT (simple)
t1260=top();
S_DATA(t1260)[1]=t1262;
t1262=pick(2);
X t1263=lf[243]; // CONST
t1260=top();
S_DATA(t1260)[2]=t1263;
t1263=pick(2);
X t1265=TO_N(1); // ICONST
save(t1265);
X t1266=select1(TO_N(2),t1263); // SELECT (simple)
t1265=restore();
X t1264=select1(t1265,t1266); // SELECT
t1260=top();
S_DATA(t1260)[3]=t1264;
t1264=pick(2);
X t1267=lf[229]; // CONST
t1260=top();
S_DATA(t1260)[4]=t1267;
t1260=restore();
restore();
t1259=top();
S_DATA(t1259)[0]=t1260;
t1260=pick(2);
X t1268=select1(TO_N(1),t1260); // SELECT (simple)
t1259=top();
S_DATA(t1259)[1]=t1268;
t1268=pick(2);
X t1269=select1(TO_N(2),t1268); // SELECT (simple)
t1259=top();
S_DATA(t1259)[2]=t1269;
t1259=restore();
restore();
X t1270=___backend_3aconcx(t1259); // REF: backend:concx
t1190=t1270;
}else{
save(x); // COND
X t1273=lf[307]; // CONST
save(t1273);
X t1275=TO_N(1); // ICONST
save(t1275);
X t1276=select1(TO_N(1),x); // SELECT (simple)
t1275=restore();
X t1274=select1(t1275,t1276); // SELECT
t1273=restore();
X t1272=(t1273==t1274)||eq1(t1273,t1274)?T:F; // EQ
X t1271;
x=restore();
if(t1272!=F){
save(x);
X t1277=allocate(2); // CONS
save(t1277);
x=pick(2);
X t1278=select1(TO_N(1),x); // SELECT (simple)
t1277=top();
S_DATA(t1277)[0]=t1278;
t1278=pick(2);
save(t1278);
X t1279=allocate(2); // CONS
save(t1279);
t1278=pick(2);
X t1280=TO_N(4); // ICONST
t1279=top();
S_DATA(t1279)[0]=t1280;
t1280=pick(2);
X t1281=select1(TO_N(2),t1280); // SELECT (simple)
t1279=top();
S_DATA(t1279)[1]=t1281;
t1279=restore();
restore();
X t1282=___backend_3agensyms(t1279); // REF: backend:gensyms
t1277=top();
S_DATA(t1277)[1]=t1282;
t1277=restore();
restore();
X t1283=___al(t1277); // REF: al
save(t1283);
X t1284=allocate(4); // CONS
save(t1284);
t1283=pick(2);
X t1285=select1(TO_N(1),t1283); // SELECT (simple)
t1284=top();
S_DATA(t1284)[0]=t1285;
t1285=pick(2);
X t1286=select1(TO_N(2),t1285); // SELECT (simple)
t1284=top();
S_DATA(t1284)[1]=t1286;
t1286=pick(2);
X t1288=TO_N(1); // ICONST
save(t1288);
X t1289=select1(TO_N(3),t1286); // SELECT (simple)
t1288=restore();
X t1287=select1(t1288,t1289); // SELECT
t1284=top();
S_DATA(t1284)[2]=t1287;
t1287=pick(2);
save(t1287);
X t1290=allocate(3); // CONS
save(t1290);
t1287=pick(2);
save(t1287);
X t1291=allocate(31); // CONS
save(t1291);
t1287=pick(2);
X t1292=lf[280]; // CONST
t1291=top();
S_DATA(t1291)[0]=t1292;
t1292=pick(2);
X t1293=lf[231]; // CONST
t1291=top();
S_DATA(t1291)[1]=t1293;
t1293=pick(2);
X t1295=TO_N(1); // ICONST
save(t1295);
X t1296=select1(TO_N(2),t1293); // SELECT (simple)
t1295=restore();
X t1294=select1(t1295,t1296); // SELECT
t1291=top();
S_DATA(t1291)[2]=t1294;
t1294=pick(2);
X t1297=lf[232]; // CONST
t1291=top();
S_DATA(t1291)[3]=t1297;
t1297=pick(2);
X t1299=TO_N(3); // ICONST
save(t1299);
X t1300=select1(TO_N(2),t1297); // SELECT (simple)
t1299=restore();
X t1298=select1(t1299,t1300); // SELECT
t1291=top();
S_DATA(t1291)[4]=t1298;
t1298=pick(2);
X t1301=lf[233]; // CONST
t1291=top();
S_DATA(t1291)[5]=t1301;
t1301=pick(2);
X t1302=lf[280]; // CONST
t1291=top();
S_DATA(t1291)[6]=t1302;
t1302=pick(2);
X t1303=lf[234]; // CONST
t1291=top();
S_DATA(t1291)[7]=t1303;
t1303=pick(2);
X t1305=TO_N(4); // ICONST
save(t1305);
X t1306=select1(TO_N(2),t1303); // SELECT (simple)
t1305=restore();
X t1304=select1(t1305,t1306); // SELECT
t1291=top();
S_DATA(t1291)[8]=t1304;
t1304=pick(2);
X t1307=lf[235]; // CONST
t1291=top();
S_DATA(t1291)[9]=t1307;
t1307=pick(2);
X t1309=TO_N(2); // ICONST
save(t1309);
X t1310=select1(TO_N(2),t1307); // SELECT (simple)
t1309=restore();
X t1308=select1(t1309,t1310); // SELECT
t1291=top();
S_DATA(t1291)[10]=t1308;
t1308=pick(2);
X t1311=lf[288]; // CONST
t1291=top();
S_DATA(t1291)[11]=t1311;
t1311=pick(2);
X t1312=lf[280]; // CONST
t1291=top();
S_DATA(t1291)[12]=t1312;
t1312=pick(2);
X t1313=lf[236]; // CONST
t1291=top();
S_DATA(t1291)[13]=t1313;
t1313=pick(2);
X t1315=TO_N(2); // ICONST
save(t1315);
X t1316=select1(TO_N(2),t1313); // SELECT (simple)
t1315=restore();
X t1314=select1(t1315,t1316); // SELECT
t1291=top();
S_DATA(t1291)[14]=t1314;
t1314=pick(2);
X t1317=lf[288]; // CONST
t1291=top();
S_DATA(t1291)[15]=t1317;
t1317=pick(2);
X t1318=lf[280]; // CONST
t1291=top();
S_DATA(t1291)[16]=t1318;
t1318=pick(2);
X t1319=lf[237]; // CONST
t1291=top();
S_DATA(t1291)[17]=t1319;
t1319=pick(2);
X t1321=TO_N(2); // ICONST
save(t1321);
X t1322=select1(TO_N(2),t1319); // SELECT (simple)
t1321=restore();
X t1320=select1(t1321,t1322); // SELECT
t1291=top();
S_DATA(t1291)[18]=t1320;
t1320=pick(2);
X t1323=lf[238]; // CONST
t1291=top();
S_DATA(t1291)[19]=t1323;
t1323=pick(2);
X t1324=lf[280]; // CONST
t1291=top();
S_DATA(t1291)[20]=t1324;
t1324=pick(2);
X t1326=TO_N(3); // ICONST
save(t1326);
X t1327=select1(TO_N(2),t1324); // SELECT (simple)
t1326=restore();
X t1325=select1(t1326,t1327); // SELECT
t1291=top();
S_DATA(t1291)[21]=t1325;
t1325=pick(2);
X t1328=lf[273]; // CONST
t1291=top();
S_DATA(t1291)[22]=t1328;
t1328=pick(2);
X t1329=lf[280]; // CONST
t1291=top();
S_DATA(t1291)[23]=t1329;
t1329=pick(2);
X t1330=lf[239]; // CONST
t1291=top();
S_DATA(t1291)[24]=t1330;
t1330=pick(2);
X t1332=TO_N(1); // ICONST
save(t1332);
X t1333=select1(TO_N(2),t1330); // SELECT (simple)
t1332=restore();
X t1331=select1(t1332,t1333); // SELECT
t1291=top();
S_DATA(t1291)[25]=t1331;
t1331=pick(2);
X t1334=lf[288]; // CONST
t1291=top();
S_DATA(t1291)[26]=t1334;
t1334=pick(2);
X t1335=lf[280]; // CONST
t1291=top();
S_DATA(t1291)[27]=t1335;
t1335=pick(2);
X t1336=lf[240]; // CONST
t1291=top();
S_DATA(t1291)[28]=t1336;
t1336=pick(2);
X t1338=TO_N(4); // ICONST
save(t1338);
X t1339=select1(TO_N(2),t1336); // SELECT (simple)
t1338=restore();
X t1337=select1(t1338,t1339); // SELECT
t1291=top();
S_DATA(t1291)[29]=t1337;
t1337=pick(2);
X t1340=lf[288]; // CONST
t1291=top();
S_DATA(t1291)[30]=t1340;
t1291=restore();
restore();
t1290=top();
S_DATA(t1290)[0]=t1291;
t1291=pick(2);
X t1342=TO_N(3); // ICONST
save(t1342);
X t1343=select1(TO_N(2),t1291); // SELECT (simple)
t1342=restore();
X t1341=select1(t1342,t1343); // SELECT
t1290=top();
S_DATA(t1290)[1]=t1341;
t1341=pick(2);
X t1344=select1(TO_N(3),t1341); // SELECT (simple)
t1290=top();
S_DATA(t1290)[2]=t1344;
t1290=restore();
restore();
X t1345=___backend_3aconcx(t1290); // REF: backend:concx
t1284=top();
S_DATA(t1284)[3]=t1345;
t1284=restore();
restore();
save(t1284);
X t1346=allocate(4); // CONS
save(t1346);
t1284=pick(2);
X t1347=select1(TO_N(1),t1284); // SELECT (simple)
t1346=top();
S_DATA(t1346)[0]=t1347;
t1347=pick(2);
X t1348=select1(TO_N(2),t1347); // SELECT (simple)
t1346=top();
S_DATA(t1346)[1]=t1348;
t1348=pick(2);
X t1349=select1(TO_N(3),t1348); // SELECT (simple)
t1346=top();
S_DATA(t1346)[2]=t1349;
t1349=pick(2);
save(t1349);
X t1350=allocate(2); // CONS
save(t1350);
t1349=pick(2);
X t1352=TO_N(3); // ICONST
save(t1352);
X t1353=select1(TO_N(1),t1349); // SELECT (simple)
t1352=restore();
X t1351=select1(t1352,t1353); // SELECT
t1350=top();
S_DATA(t1350)[0]=t1351;
t1351=pick(2);
X t1354=select1(TO_N(4),t1351); // SELECT (simple)
t1350=top();
S_DATA(t1350)[1]=t1354;
t1350=restore();
restore();
X t1355=___backend_3at_5fexpr(t1350); // REF: backend:t_expr
t1346=top();
S_DATA(t1346)[3]=t1355;
t1346=restore();
restore();
save(t1346);
X t1356=allocate(3); // CONS
save(t1356);
t1346=pick(2);
X t1357=select1(TO_N(1),t1346); // SELECT (simple)
t1356=top();
S_DATA(t1356)[0]=t1357;
t1357=pick(2);
X t1358=select1(TO_N(2),t1357); // SELECT (simple)
t1356=top();
S_DATA(t1356)[1]=t1358;
t1358=pick(2);
save(t1358);
X t1359=allocate(3); // CONS
save(t1359);
t1358=pick(2);
save(t1358);
X t1360=allocate(7); // CONS
save(t1360);
t1358=pick(2);
X t1361=lf[280]; // CONST
t1360=top();
S_DATA(t1360)[0]=t1361;
t1361=pick(2);
X t1363=TO_N(3); // ICONST
save(t1363);
X t1364=select1(TO_N(2),t1361); // SELECT (simple)
t1363=restore();
X t1362=select1(t1363,t1364); // SELECT
t1360=top();
S_DATA(t1360)[1]=t1362;
t1362=pick(2);
X t1365=lf[243]; // CONST
t1360=top();
S_DATA(t1360)[2]=t1365;
t1365=pick(2);
X t1367=TO_N(1); // ICONST
save(t1367);
X t1368=select1(TO_N(4),t1365); // SELECT (simple)
t1367=restore();
X t1366=select1(t1367,t1368); // SELECT
t1360=top();
S_DATA(t1360)[3]=t1366;
t1366=pick(2);
X t1369=lf[229]; // CONST
t1360=top();
S_DATA(t1360)[4]=t1369;
t1369=pick(2);
X t1370=lf[280]; // CONST
t1360=top();
S_DATA(t1360)[5]=t1370;
t1370=pick(2);
X t1371=lf[230]; // CONST
t1360=top();
S_DATA(t1360)[6]=t1371;
t1360=restore();
restore();
t1359=top();
S_DATA(t1359)[0]=t1360;
t1360=pick(2);
X t1372=select1(TO_N(3),t1360); // SELECT (simple)
t1359=top();
S_DATA(t1359)[1]=t1372;
t1372=pick(2);
X t1373=select1(TO_N(4),t1372); // SELECT (simple)
t1359=top();
S_DATA(t1359)[2]=t1373;
t1359=restore();
restore();
X t1374=___backend_3aconcx(t1359); // REF: backend:concx
t1356=top();
S_DATA(t1356)[2]=t1374;
t1356=restore();
restore();
save(t1356);
X t1375=allocate(2); // CONS
save(t1375);
t1356=pick(2);
X t1376=select1(TO_N(2),t1356); // SELECT (simple)
t1375=top();
S_DATA(t1375)[0]=t1376;
t1376=pick(2);
save(t1376);
X t1377=allocate(2); // CONS
save(t1377);
t1376=pick(2);
X t1379=TO_N(4); // ICONST
save(t1379);
X t1380=select1(TO_N(1),t1376); // SELECT (simple)
t1379=restore();
X t1378=select1(t1379,t1380); // SELECT
t1377=top();
S_DATA(t1377)[0]=t1378;
t1378=pick(2);
X t1381=select1(TO_N(3),t1378); // SELECT (simple)
X t1382=___backend_3anontail(t1381); // REF: backend:nontail
t1377=top();
S_DATA(t1377)[1]=t1382;
t1377=restore();
restore();
X t1383=___backend_3at_5fexpr(t1377); // REF: backend:t_expr
t1375=top();
S_DATA(t1375)[1]=t1383;
t1375=restore();
restore();
save(t1375);
X t1384=allocate(3); // CONS
save(t1384);
t1375=pick(2);
save(t1375);
X t1385=allocate(9); // CONS
save(t1385);
t1375=pick(2);
X t1386=lf[280]; // CONST
t1385=top();
S_DATA(t1385)[0]=t1386;
t1386=pick(2);
X t1387=lf[240]; // CONST
t1385=top();
S_DATA(t1385)[1]=t1387;
t1387=pick(2);
X t1389=TO_N(4); // ICONST
save(t1389);
X t1390=select1(TO_N(1),t1387); // SELECT (simple)
t1389=restore();
X t1388=select1(t1389,t1390); // SELECT
t1385=top();
S_DATA(t1385)[2]=t1388;
t1388=pick(2);
X t1391=lf[288]; // CONST
t1385=top();
S_DATA(t1385)[3]=t1391;
t1391=pick(2);
X t1392=lf[280]; // CONST
t1385=top();
S_DATA(t1385)[4]=t1392;
t1392=pick(2);
X t1394=TO_N(3); // ICONST
save(t1394);
X t1395=select1(TO_N(1),t1392); // SELECT (simple)
t1394=restore();
X t1393=select1(t1394,t1395); // SELECT
t1385=top();
S_DATA(t1385)[5]=t1393;
t1393=pick(2);
X t1396=lf[243]; // CONST
t1385=top();
S_DATA(t1385)[6]=t1396;
t1396=pick(2);
X t1398=TO_N(1); // ICONST
save(t1398);
X t1399=select1(TO_N(2),t1396); // SELECT (simple)
t1398=restore();
X t1397=select1(t1398,t1399); // SELECT
t1385=top();
S_DATA(t1385)[7]=t1397;
t1397=pick(2);
X t1400=lf[229]; // CONST
t1385=top();
S_DATA(t1385)[8]=t1400;
t1385=restore();
restore();
t1384=top();
S_DATA(t1384)[0]=t1385;
t1385=pick(2);
X t1402=TO_N(3); // ICONST
save(t1402);
X t1403=select1(TO_N(1),t1385); // SELECT (simple)
t1402=restore();
X t1401=select1(t1402,t1403); // SELECT
t1384=top();
S_DATA(t1384)[1]=t1401;
t1401=pick(2);
X t1404=select1(TO_N(2),t1401); // SELECT (simple)
t1384=top();
S_DATA(t1384)[2]=t1404;
t1384=restore();
restore();
X t1405=___backend_3aconcx(t1384); // REF: backend:concx
t1271=t1405;
}else{
save(x); // COND
X t1408=lf[311]; // CONST
save(t1408);
X t1410=TO_N(1); // ICONST
save(t1410);
X t1411=select1(TO_N(1),x); // SELECT (simple)
t1410=restore();
X t1409=select1(t1410,t1411); // SELECT
t1408=restore();
X t1407=(t1408==t1409)||eq1(t1408,t1409)?T:F; // EQ
X t1406;
x=restore();
if(t1407!=F){
save(x);
X t1412=allocate(2); // CONS
save(t1412);
x=pick(2);
X t1413=select1(TO_N(1),x); // SELECT (simple)
t1412=top();
S_DATA(t1412)[0]=t1413;
t1413=pick(2);
X t1414=select1(TO_N(2),t1413); // SELECT (simple)
X t1415=___backend_3agensym(t1414); // REF: backend:gensym
t1412=top();
S_DATA(t1412)[1]=t1415;
t1412=restore();
restore();
X t1416=___al(t1412); // REF: al
save(t1416);
X t1417=allocate(3); // CONS
save(t1417);
t1416=pick(2);
save(t1416);
X t1418=allocate(12); // CONS
save(t1418);
t1416=pick(2);
X t1419=lf[280]; // CONST
t1418=top();
S_DATA(t1418)[0]=t1419;
t1419=pick(2);
X t1420=lf[241]; // CONST
t1418=top();
S_DATA(t1418)[1]=t1420;
t1420=pick(2);
X t1422=TO_N(3); // ICONST
save(t1422);
X t1423=select1(TO_N(1),t1420); // SELECT (simple)
t1422=restore();
X t1421=select1(t1422,t1423); // SELECT
X t1424=___tos(t1421); // REF: tos
t1418=top();
S_DATA(t1418)[2]=t1424;
t1424=pick(2);
X t1425=lf[242]; // CONST
t1418=top();
S_DATA(t1418)[3]=t1425;
t1425=pick(2);
X t1426=lf[280]; // CONST
t1418=top();
S_DATA(t1418)[4]=t1426;
t1426=pick(2);
X t1427=lf[262]; // CONST
t1418=top();
S_DATA(t1418)[5]=t1427;
t1427=pick(2);
X t1428=select1(TO_N(2),t1427); // SELECT (simple)
t1418=top();
S_DATA(t1418)[6]=t1428;
t1428=pick(2);
X t1429=lf[243]; // CONST
t1418=top();
S_DATA(t1418)[7]=t1429;
t1429=pick(2);
X t1431=TO_N(3); // ICONST
save(t1431);
X t1432=select1(TO_N(1),t1429); // SELECT (simple)
t1431=restore();
X t1430=select1(t1431,t1432); // SELECT
X t1433=___tos(t1430); // REF: tos
t1418=top();
S_DATA(t1418)[8]=t1433;
t1433=pick(2);
X t1434=lf[285]; // CONST
t1418=top();
S_DATA(t1418)[9]=t1434;
t1434=pick(2);
X t1436=TO_N(1); // ICONST
save(t1436);
X t1437=select1(TO_N(3),t1434); // SELECT (simple)
t1436=restore();
X t1435=select1(t1436,t1437); // SELECT
t1418=top();
S_DATA(t1418)[10]=t1435;
t1435=pick(2);
X t1438=lf[272]; // CONST
t1418=top();
S_DATA(t1418)[11]=t1438;
t1418=restore();
restore();
t1417=top();
S_DATA(t1417)[0]=t1418;
t1418=pick(2);
X t1439=select1(TO_N(2),t1418); // SELECT (simple)
t1417=top();
S_DATA(t1417)[1]=t1439;
t1439=pick(2);
X t1440=select1(TO_N(3),t1439); // SELECT (simple)
t1417=top();
S_DATA(t1417)[2]=t1440;
t1417=restore();
restore();
X t1441=___backend_3aconcx(t1417); // REF: backend:concx
t1406=t1441;
}else{
save(x); // COND
X t1444=lf[313]; // CONST
save(t1444);
X t1446=TO_N(1); // ICONST
save(t1446);
X t1447=select1(TO_N(1),x); // SELECT (simple)
t1446=restore();
X t1445=select1(t1446,t1447); // SELECT
t1444=restore();
X t1443=(t1444==t1445)||eq1(t1444,t1445)?T:F; // EQ
X t1442;
x=restore();
if(t1443!=F){
save(x);
X t1448=allocate(2); // CONS
save(t1448);
x=pick(2);
X t1449=select1(TO_N(1),x); // SELECT (simple)
t1448=top();
S_DATA(t1448)[0]=t1449;
t1449=pick(2);
X t1450=select1(TO_N(2),t1449); // SELECT (simple)
X t1451=___backend_3agensym(t1450); // REF: backend:gensym
t1448=top();
S_DATA(t1448)[1]=t1451;
t1448=restore();
restore();
X t1452=___al(t1448); // REF: al
save(t1452);
X t1453=allocate(3); // CONS
save(t1453);
t1452=pick(2);
save(t1452);
X t1454=allocate(6); // CONS
save(t1454);
t1452=pick(2);
X t1455=lf[280]; // CONST
t1454=top();
S_DATA(t1454)[0]=t1455;
t1455=pick(2);
X t1456=lf[262]; // CONST
t1454=top();
S_DATA(t1454)[1]=t1456;
t1456=pick(2);
X t1457=select1(TO_N(2),t1456); // SELECT (simple)
t1454=top();
S_DATA(t1454)[2]=t1457;
t1457=pick(2);
X t1458=lf[243]; // CONST
t1454=top();
S_DATA(t1454)[3]=t1458;
t1458=pick(2);
X t1460=TO_N(3); // ICONST
save(t1460);
X t1461=select1(TO_N(1),t1458); // SELECT (simple)
t1460=restore();
X t1459=select1(t1460,t1461); // SELECT
X t1462=___backend_3aunit_5fvalue(t1459); // REF: backend:unit_value
t1454=top();
S_DATA(t1454)[4]=t1462;
t1462=pick(2);
X t1463=lf[288]; // CONST
t1454=top();
S_DATA(t1454)[5]=t1463;
t1454=restore();
restore();
t1453=top();
S_DATA(t1453)[0]=t1454;
t1454=pick(2);
X t1464=select1(TO_N(2),t1454); // SELECT (simple)
t1453=top();
S_DATA(t1453)[1]=t1464;
t1464=pick(2);
X t1465=select1(TO_N(3),t1464); // SELECT (simple)
t1453=top();
S_DATA(t1453)[2]=t1465;
t1453=restore();
restore();
X t1466=___backend_3aconcx(t1453); // REF: backend:concx
t1442=t1466;
}else{
save(x); // COND
X t1469=lf[317]; // CONST
save(t1469);
X t1471=TO_N(1); // ICONST
save(t1471);
X t1472=select1(TO_N(1),x); // SELECT (simple)
t1471=restore();
X t1470=select1(t1471,t1472); // SELECT
t1469=restore();
X t1468=(t1469==t1470)||eq1(t1469,t1470)?T:F; // EQ
X t1467;
x=restore();
if(t1468!=F){
X t1473=___backend_3atwo_5farg_5ffun(x); // REF: backend:two_arg_fun
save(t1473);
X t1474=allocate(3); // CONS
save(t1474);
t1473=pick(2);
save(t1473);
X t1475=allocate(12); // CONS
save(t1475);
t1473=pick(2);
X t1476=lf[280]; // CONST
t1475=top();
S_DATA(t1475)[0]=t1476;
t1476=pick(2);
X t1477=lf[262]; // CONST
t1475=top();
S_DATA(t1475)[1]=t1477;
t1477=pick(2);
X t1478=select1(TO_N(2),t1477); // SELECT (simple)
t1475=top();
S_DATA(t1475)[2]=t1478;
t1478=pick(2);
X t1479=lf[244]; // CONST
t1475=top();
S_DATA(t1475)[3]=t1479;
t1479=pick(2);
X t1481=TO_N(1); // ICONST
save(t1481);
X t1482=select1(TO_N(3),t1479); // SELECT (simple)
t1481=restore();
X t1480=select1(t1481,t1482); // SELECT
t1475=top();
S_DATA(t1475)[4]=t1480;
t1480=pick(2);
X t1483=lf[245]; // CONST
t1475=top();
S_DATA(t1475)[5]=t1483;
t1483=pick(2);
X t1485=TO_N(1); // ICONST
save(t1485);
X t1486=select1(TO_N(4),t1483); // SELECT (simple)
t1485=restore();
X t1484=select1(t1485,t1486); // SELECT
t1475=top();
S_DATA(t1475)[6]=t1484;
t1484=pick(2);
X t1487=lf[246]; // CONST
t1475=top();
S_DATA(t1475)[7]=t1487;
t1487=pick(2);
X t1489=TO_N(1); // ICONST
save(t1489);
X t1490=select1(TO_N(3),t1487); // SELECT (simple)
t1489=restore();
X t1488=select1(t1489,t1490); // SELECT
t1475=top();
S_DATA(t1475)[8]=t1488;
t1488=pick(2);
X t1491=lf[252]; // CONST
t1475=top();
S_DATA(t1475)[9]=t1491;
t1491=pick(2);
X t1493=TO_N(1); // ICONST
save(t1493);
X t1494=select1(TO_N(4),t1491); // SELECT (simple)
t1493=restore();
X t1492=select1(t1493,t1494); // SELECT
t1475=top();
S_DATA(t1475)[10]=t1492;
t1492=pick(2);
X t1495=lf[247]; // CONST
t1475=top();
S_DATA(t1475)[11]=t1495;
t1475=restore();
restore();
t1474=top();
S_DATA(t1474)[0]=t1475;
t1475=pick(2);
X t1496=select1(TO_N(2),t1475); // SELECT (simple)
t1474=top();
S_DATA(t1474)[1]=t1496;
t1496=pick(2);
X t1497=select1(TO_N(4),t1496); // SELECT (simple)
t1474=top();
S_DATA(t1474)[2]=t1497;
t1474=restore();
restore();
X t1498=___backend_3aconcx(t1474); // REF: backend:concx
t1467=t1498;
}else{
save(x); // COND
X t1501=lf[319]; // CONST
save(t1501);
X t1503=TO_N(1); // ICONST
save(t1503);
X t1504=select1(TO_N(1),x); // SELECT (simple)
t1503=restore();
X t1502=select1(t1503,t1504); // SELECT
t1501=restore();
X t1500=(t1501==t1502)||eq1(t1501,t1502)?T:F; // EQ
X t1499;
x=restore();
if(t1500!=F){
save(x); // COND
X t1506=select1(TO_N(1),x); // SELECT (simple)
X t1507=F;
save(t1506); // PCONS
if(!IS_S(t1506)||S_LENGTH(t1506)!=4) goto t1508;
// skipped
// skipped
t1506=S_DATA(top())[2];
X t1509=F;
save(t1506); // PCONS
if(!IS_S(t1506)||S_LENGTH(t1506)!=3) goto t1510;
t1506=S_DATA(top())[0];
X t1512=lf[291]; // CONST
save(t1512);
X t1513=___id(t1506); // REF: id
t1512=restore();
X t1511=(t1512==t1513)||eq1(t1512,t1513)?T:F; // EQ
if(t1511==F) goto t1510;
// skipped
t1511=S_DATA(top())[2];
X t1514=___num(t1511); // REF: num
if(t1514==F) goto t1510;
t1509=T;
t1510:
restore();
if(t1509==F) goto t1508;
t1509=S_DATA(top())[3];
X t1515=F;
save(t1509); // PCONS
if(!IS_S(t1509)||S_LENGTH(t1509)!=3) goto t1516;
t1509=S_DATA(top())[0];
X t1518=lf[293]; // CONST
save(t1518);
X t1519=___id(t1509); // REF: id
t1518=restore();
X t1517=(t1518==t1519)||eq1(t1518,t1519)?T:F; // EQ
if(t1517==F) goto t1516;
// skipped
t1517=S_DATA(top())[2];
X t1521=lf[282]; // CONST
save(t1521);
X t1522=___id(t1517); // REF: id
t1521=restore();
X t1520=(t1521==t1522)||eq1(t1521,t1522)?T:F; // EQ
if(t1520==F) goto t1516;
t1515=T;
t1516:
restore();
if(t1515==F) goto t1508;
t1507=T;
t1508:
restore();
X t1505;
x=restore();
if(t1507!=F){
save(x);
X t1523=allocate(2); // CONS
save(t1523);
x=pick(2);
X t1524=select1(TO_N(1),x); // SELECT (simple)
t1523=top();
S_DATA(t1523)[0]=t1524;
t1524=pick(2);
X t1525=select1(TO_N(2),t1524); // SELECT (simple)
X t1526=___backend_3agensym(t1525); // REF: backend:gensym
t1523=top();
S_DATA(t1523)[1]=t1526;
t1523=restore();
restore();
X t1527=___al(t1523); // REF: al
save(t1527);
X t1528=allocate(3); // CONS
save(t1528);
t1527=pick(2);
save(t1527);
X t1529=allocate(8); // CONS
save(t1529);
t1527=pick(2);
X t1530=lf[280]; // CONST
t1529=top();
S_DATA(t1529)[0]=t1530;
t1530=pick(2);
X t1531=lf[262]; // CONST
t1529=top();
S_DATA(t1529)[1]=t1531;
t1531=pick(2);
X t1532=select1(TO_N(2),t1531); // SELECT (simple)
t1529=top();
S_DATA(t1529)[2]=t1532;
t1532=pick(2);
X t1533=lf[248]; // CONST
t1529=top();
S_DATA(t1529)[3]=t1533;
t1533=pick(2);
X t1535=TO_N(3); // ICONST
save(t1535);
X t1537=TO_N(3); // ICONST
save(t1537);
X t1538=select1(TO_N(1),t1533); // SELECT (simple)
t1537=restore();
X t1536=select1(t1537,t1538); // SELECT
t1535=restore();
X t1534=select1(t1535,t1536); // SELECT
X t1539=___tos(t1534); // REF: tos
t1529=top();
S_DATA(t1529)[4]=t1539;
t1539=pick(2);
X t1540=lf[249]; // CONST
t1529=top();
S_DATA(t1529)[5]=t1540;
t1540=pick(2);
X t1542=TO_N(1); // ICONST
save(t1542);
X t1543=select1(TO_N(3),t1540); // SELECT (simple)
t1542=restore();
X t1541=select1(t1542,t1543); // SELECT
t1529=top();
S_DATA(t1529)[6]=t1541;
t1541=pick(2);
X t1544=lf[250]; // CONST
t1529=top();
S_DATA(t1529)[7]=t1544;
t1529=restore();
restore();
t1528=top();
S_DATA(t1528)[0]=t1529;
t1529=pick(2);
X t1545=select1(TO_N(2),t1529); // SELECT (simple)
t1528=top();
S_DATA(t1528)[1]=t1545;
t1545=pick(2);
X t1546=select1(TO_N(3),t1545); // SELECT (simple)
t1528=top();
S_DATA(t1528)[2]=t1546;
t1528=restore();
restore();
t1505=t1528;
}else{
X t1547=___backend_3atwo_5farg_5ffun(x); // REF: backend:two_arg_fun
save(t1547);
X t1548=allocate(3); // CONS
save(t1548);
t1547=pick(2);
save(t1547);
X t1549=allocate(8); // CONS
save(t1549);
t1547=pick(2);
X t1550=lf[280]; // CONST
t1549=top();
S_DATA(t1549)[0]=t1550;
t1550=pick(2);
X t1551=lf[262]; // CONST
t1549=top();
S_DATA(t1549)[1]=t1551;
t1551=pick(2);
X t1552=select1(TO_N(2),t1551); // SELECT (simple)
t1549=top();
S_DATA(t1549)[2]=t1552;
t1552=pick(2);
X t1553=lf[251]; // CONST
t1549=top();
S_DATA(t1549)[3]=t1553;
t1553=pick(2);
X t1555=TO_N(1); // ICONST
save(t1555);
X t1556=select1(TO_N(3),t1553); // SELECT (simple)
t1555=restore();
X t1554=select1(t1555,t1556); // SELECT
t1549=top();
S_DATA(t1549)[4]=t1554;
t1554=pick(2);
X t1557=lf[252]; // CONST
t1549=top();
S_DATA(t1549)[5]=t1557;
t1557=pick(2);
X t1559=TO_N(1); // ICONST
save(t1559);
X t1560=select1(TO_N(4),t1557); // SELECT (simple)
t1559=restore();
X t1558=select1(t1559,t1560); // SELECT
t1549=top();
S_DATA(t1549)[6]=t1558;
t1558=pick(2);
X t1561=lf[253]; // CONST
t1549=top();
S_DATA(t1549)[7]=t1561;
t1549=restore();
restore();
t1548=top();
S_DATA(t1548)[0]=t1549;
t1549=pick(2);
X t1562=select1(TO_N(2),t1549); // SELECT (simple)
t1548=top();
S_DATA(t1548)[1]=t1562;
t1562=pick(2);
X t1563=select1(TO_N(4),t1562); // SELECT (simple)
t1548=top();
S_DATA(t1548)[2]=t1563;
t1548=restore();
restore();
t1505=t1548;}
X t1564=___backend_3aconcx(t1505); // REF: backend:concx
t1499=t1564;
}else{
X t1565=____5f(x); // REF: _
t1499=t1565;}
t1467=t1499;}
t1442=t1467;}
t1406=t1442;}
t1271=t1406;}
t1190=t1271;}
t1108=t1190;}
t1052=t1108;}
t1017=t1052;}
t952=t1017;}
t819=t952;}
t686=t819;}
t549=t686;}
t417=t549;}
t370=t417;}
t341=t370;}
t334=t341;}
t327=t334;}
t212=t327;}
t93=t212;}
t1=t93;}
RETURN(t1);}
//---------------------------------------- backend:t_pcons (c-backend.fp:152)
DEFINE(___backend_3at_5fpcons){
ENTRY;
loop:;
tracecall("c-backend.fp:152:  backend:t_pcons");
save(x);
X t1=allocate(2); // CONS
save(t1);
x=pick(2);
X t2=select1(TO_N(1),x); // SELECT (simple)
t1=top();
S_DATA(t1)[0]=t2;
t2=pick(2);
save(t2);
X t3=allocate(2); // CONS
save(t3);
t2=pick(2);
X t4=TO_N(2); // ICONST
t3=top();
S_DATA(t3)[0]=t4;
t4=pick(2);
X t5=select1(TO_N(2),t4); // SELECT (simple)
save(t5);
X t6=allocate(4); // CONS
save(t6);
t5=pick(2);
X t7=select1(TO_N(1),t5); // SELECT (simple)
t6=top();
S_DATA(t6)[0]=t7;
t7=pick(2);
X t8=select1(TO_N(2),t7); // SELECT (simple)
t6=top();
S_DATA(t6)[1]=t8;
t8=pick(2);
X t9=F; // ICONST
t6=top();
S_DATA(t6)[2]=t9;
t9=pick(2);
X t10=select1(TO_N(4),t9); // SELECT (simple)
t6=top();
S_DATA(t6)[3]=t10;
t6=restore();
restore();
t3=top();
S_DATA(t3)[1]=t6;
t3=restore();
restore();
X t11=___backend_3agensyms(t3); // REF: backend:gensyms
t1=top();
S_DATA(t1)[1]=t11;
t1=restore();
restore();
X t12=___al(t1); // REF: al
save(t12);
X t13=allocate(3); // CONS
save(t13);
t12=pick(2);
X t14=select1(TO_N(1),t12); // SELECT (simple)
X t15=___tl(t14); // REF: tl
X t16=___tl(t15); // REF: tl
t13=top();
S_DATA(t13)[0]=t16;
t16=pick(2);
X t17=select1(TO_N(2),t16); // SELECT (simple)
t13=top();
S_DATA(t13)[1]=t17;
t17=pick(2);
save(t17);
X t18=allocate(2); // CONS
save(t18);
t17=pick(2);
save(t17);
X t19=allocate(20); // CONS
save(t19);
t17=pick(2);
X t20=lf[280]; // CONST
t19=top();
S_DATA(t19)[0]=t20;
t20=pick(2);
X t21=lf[262]; // CONST
t19=top();
S_DATA(t19)[1]=t21;
t21=pick(2);
X t23=TO_N(1); // ICONST
save(t23);
X t24=select1(TO_N(2),t21); // SELECT (simple)
t23=restore();
X t22=select1(t23,t24); // SELECT
t19=top();
S_DATA(t19)[2]=t22;
t22=pick(2);
X t25=lf[263]; // CONST
t19=top();
S_DATA(t19)[3]=t25;
t25=pick(2);
X t26=lf[280]; // CONST
t19=top();
S_DATA(t19)[4]=t26;
t26=pick(2);
X t27=lf[271]; // CONST
t19=top();
S_DATA(t19)[5]=t27;
t27=pick(2);
X t29=TO_N(1); // ICONST
save(t29);
X t30=select1(TO_N(3),t27); // SELECT (simple)
t29=restore();
X t28=select1(t29,t30); // SELECT
t19=top();
S_DATA(t19)[6]=t28;
t28=pick(2);
X t31=lf[264]; // CONST
t19=top();
S_DATA(t19)[7]=t31;
t31=pick(2);
X t33=TO_N(1); // ICONST
save(t33);
X t34=select1(TO_N(1),t31); // SELECT (simple)
t33=restore();
X t32=select1(t33,t34); // SELECT
X t35=___tos(t32); // REF: tos
t19=top();
S_DATA(t19)[8]=t35;
t35=pick(2);
X t36=lf[280]; // CONST
t19=top();
S_DATA(t19)[9]=t36;
t36=pick(2);
X t37=lf[265]; // CONST
t19=top();
S_DATA(t19)[10]=t37;
t37=pick(2);
X t39=TO_N(1); // ICONST
save(t39);
X t40=select1(TO_N(3),t37); // SELECT (simple)
t39=restore();
X t38=select1(t39,t40); // SELECT
t19=top();
S_DATA(t19)[11]=t38;
t38=pick(2);
X t41=lf[266]; // CONST
t19=top();
S_DATA(t19)[12]=t41;
t41=pick(2);
X t43=TO_N(1); // ICONST
save(t43);
X t44=select1(TO_N(3),t41); // SELECT (simple)
t43=restore();
X t42=select1(t43,t44); // SELECT
t19=top();
S_DATA(t19)[13]=t42;
t42=pick(2);
X t45=lf[284]; // CONST
t19=top();
S_DATA(t19)[14]=t45;
t45=pick(2);
save(t45);
X t46=allocate(2); // CONS
save(t46);
t45=pick(2);
X t47=lf[329]; // CONST
t46=top();
S_DATA(t46)[0]=t47;
t47=pick(2);
X t49=TO_N(1); // ICONST
save(t49);
X t50=select1(TO_N(1),t47); // SELECT (simple)
t49=restore();
X t48=select1(t49,t50); // SELECT
t46=top();
S_DATA(t46)[1]=t48;
t46=restore();
restore();
save(t46); // COND
X t52=___eq(t46); // REF: eq
X t51;
t46=restore();
if(t52!=F){
X t53=lf[267]; // CONST
t51=t53;
}else{
X t54=lf[337]; // CONST
t51=t54;}
t19=top();
S_DATA(t19)[15]=t51;
t51=pick(2);
X t55=select1(TO_N(1),t51); // SELECT (simple)
X t56=___tl(t55); // REF: tl
X t57=___tl(t56); // REF: tl
X t58=___len(t57); // REF: len
X t59=___tos(t58); // REF: tos
t19=top();
S_DATA(t19)[16]=t59;
t59=pick(2);
X t60=lf[268]; // CONST
t19=top();
S_DATA(t19)[17]=t60;
t60=pick(2);
X t62=TO_N(2); // ICONST
save(t62);
X t63=select1(TO_N(2),t60); // SELECT (simple)
t62=restore();
X t61=select1(t62,t63); // SELECT
t19=top();
S_DATA(t19)[18]=t61;
t61=pick(2);
X t64=lf[288]; // CONST
t19=top();
S_DATA(t19)[19]=t64;
t19=restore();
restore();
t18=top();
S_DATA(t18)[0]=t19;
t19=pick(2);
X t65=select1(TO_N(3),t19); // SELECT (simple)
t18=top();
S_DATA(t18)[1]=t65;
t18=restore();
restore();
X t66=___backend_3aconc(t18); // REF: backend:conc
t13=top();
S_DATA(t13)[2]=t66;
t13=restore();
restore();
save(t13);
X t67=allocate(2); // CONS
save(t67);
t13=pick(2);
X t68=select1(TO_N(2),t13); // SELECT (simple)
t67=top();
S_DATA(t67)[0]=t68;
t68=pick(2);
save(t68);
X t69=allocate(4); // CONS
save(t69);
t68=pick(2);
X t70=select1(TO_N(1),t68); // SELECT (simple)
t69=top();
S_DATA(t69)[0]=t70;
t70=pick(2);
X t71=select1(TO_N(2),t70); // SELECT (simple)
t69=top();
S_DATA(t69)[1]=t71;
t71=pick(2);
X t72=select1(TO_N(1),t71); // SELECT (simple)
X t73=___len(t72); // REF: len
X t74=___iota(t73); // REF: iota
t69=top();
S_DATA(t69)[2]=t74;
t74=pick(2);
X t75=select1(TO_N(3),t74); // SELECT (simple)
t69=top();
S_DATA(t69)[3]=t75;
t69=restore();
restore();
X t77=TO_N(4); // ICONST
save(t77);
X t78=t69; // WHILE
for(;;){
save(t78);
save(t78); // COND
X t81=lf[381]; // CONST
save(t81);
X t82=select1(TO_N(1),t78); // SELECT (simple)
t81=restore();
X t80=(t81==t82)||eq1(t81,t82)?T:F; // EQ
X t79;
t78=restore();
if(t80!=F){
X t83=F; // ICONST
t79=t83;
}else{
X t84=T; // ICONST
t79=t84;}
t78=restore();
if(t79==F) break;
save(t78); // COND
X t87=TO_N(1); // ICONST
save(t87);
X t88=select1(TO_N(1),t78); // SELECT (simple)
t87=restore();
X t86=select1(t87,t88); // SELECT
X t89=F;
save(t86); // PCONS
if(!IS_S(t86)||S_LENGTH(t86)!=3) goto t90;
t86=S_DATA(top())[0];
save(t86); // OR
X t93=lf[290]; // CONST
save(t93);
X t94=___id(t86); // REF: id
t93=restore();
X t92=(t93==t94)||eq1(t93,t94)?T:F; // EQ
X t91=t92;
t86=restore();
if(t91==F){
X t96=lf[291]; // CONST
save(t96);
X t97=___id(t86); // REF: id
t96=restore();
X t95=(t96==t97)||eq1(t96,t97)?T:F; // EQ
t91=t95;}
if(t91==F) goto t90;
// skipped
t91=S_DATA(top())[2];
X t98=___id(t91); // REF: id
if(t98==F) goto t90;
t89=T;
t90:
restore();
X t85;
t78=restore();
if(t89!=F){
save(t78);
X t99=allocate(4); // CONS
save(t99);
t78=pick(2);
X t100=select1(TO_N(1),t78); // SELECT (simple)
X t101=___tl(t100); // REF: tl
t99=top();
S_DATA(t99)[0]=t101;
t101=pick(2);
X t102=select1(TO_N(2),t101); // SELECT (simple)
t99=top();
S_DATA(t99)[1]=t102;
t102=pick(2);
X t103=select1(TO_N(3),t102); // SELECT (simple)
X t104=___tl(t103); // REF: tl
t99=top();
S_DATA(t99)[2]=t104;
t104=pick(2);
save(t104);
X t105=allocate(2); // CONS
save(t105);
t104=pick(2);
X t106=lf[257]; // CONST
t105=top();
S_DATA(t105)[0]=t106;
t106=pick(2);
X t107=select1(TO_N(4),t106); // SELECT (simple)
t105=top();
S_DATA(t105)[1]=t107;
t105=restore();
restore();
X t108=___backend_3aconc(t105); // REF: backend:conc
t99=top();
S_DATA(t99)[3]=t108;
t99=restore();
restore();
t85=t99;
}else{
save(t78);
X t109=allocate(4); // CONS
save(t109);
t78=pick(2);
X t110=select1(TO_N(1),t78); // SELECT (simple)
t109=top();
S_DATA(t109)[0]=t110;
t110=pick(2);
X t111=select1(TO_N(2),t110); // SELECT (simple)
t109=top();
S_DATA(t109)[1]=t111;
t111=pick(2);
X t112=select1(TO_N(3),t111); // SELECT (simple)
X t113=___tl(t112); // REF: tl
t109=top();
S_DATA(t109)[2]=t113;
t113=pick(2);
save(t113);
X t114=allocate(2); // CONS
save(t114);
t113=pick(2);
X t116=TO_N(1); // ICONST
save(t116);
X t117=select1(TO_N(1),t113); // SELECT (simple)
t116=restore();
X t115=select1(t116,t117); // SELECT
t114=top();
S_DATA(t114)[0]=t115;
t115=pick(2);
save(t115);
X t118=allocate(2); // CONS
save(t118);
t115=pick(2);
save(t115);
X t119=allocate(5); // CONS
save(t119);
t115=pick(2);
X t120=lf[280]; // CONST
t119=top();
S_DATA(t119)[0]=t120;
t120=pick(2);
X t122=TO_N(1); // ICONST
save(t122);
X t123=select1(TO_N(4),t120); // SELECT (simple)
t122=restore();
X t121=select1(t122,t123); // SELECT
t119=top();
S_DATA(t119)[1]=t121;
t121=pick(2);
X t124=lf[260]; // CONST
t119=top();
S_DATA(t119)[2]=t124;
t124=pick(2);
X t126=TO_N(1); // ICONST
save(t126);
X t127=select1(TO_N(3),t124); // SELECT (simple)
t126=restore();
X t125=select1(t126,t127); // SELECT
save(t125);
X t128=allocate(2); // CONS
save(t128);
t125=pick(2);
X t129=___id(t125); // REF: id
t128=top();
S_DATA(t128)[0]=t129;
t129=pick(2);
X t130=TO_N(1); // ICONST
t128=top();
S_DATA(t128)[1]=t130;
t128=restore();
restore();
X t131=___sub(t128); // REF: sub
X t132=___tos(t131); // REF: tos
t119=top();
S_DATA(t119)[3]=t132;
t132=pick(2);
X t133=lf[261]; // CONST
t119=top();
S_DATA(t119)[4]=t133;
t119=restore();
restore();
t118=top();
S_DATA(t118)[0]=t119;
t119=pick(2);
X t134=select1(TO_N(4),t119); // SELECT (simple)
t118=top();
S_DATA(t118)[1]=t134;
t118=restore();
restore();
X t135=___backend_3aconc(t118); // REF: backend:conc
t114=top();
S_DATA(t114)[1]=t135;
t114=restore();
restore();
X t136=___backend_3at_5fexpr(t114); // REF: backend:t_expr
t109=top();
S_DATA(t109)[3]=t136;
t109=restore();
restore();
save(t109);
X t137=allocate(4); // CONS
save(t137);
t109=pick(2);
X t138=select1(TO_N(1),t109); // SELECT (simple)
X t139=___tl(t138); // REF: tl
t137=top();
S_DATA(t137)[0]=t139;
t139=pick(2);
X t140=select1(TO_N(2),t139); // SELECT (simple)
t137=top();
S_DATA(t137)[1]=t140;
t140=pick(2);
X t141=select1(TO_N(3),t140); // SELECT (simple)
t137=top();
S_DATA(t137)[2]=t141;
t141=pick(2);
save(t141);
X t142=allocate(2); // CONS
save(t142);
t141=pick(2);
save(t141);
X t143=allocate(6); // CONS
save(t143);
t141=pick(2);
X t144=lf[280]; // CONST
t143=top();
S_DATA(t143)[0]=t144;
t144=pick(2);
X t145=lf[258]; // CONST
t143=top();
S_DATA(t143)[1]=t145;
t145=pick(2);
X t147=TO_N(1); // ICONST
save(t147);
X t148=select1(TO_N(4),t145); // SELECT (simple)
t147=restore();
X t146=select1(t147,t148); // SELECT
t143=top();
S_DATA(t143)[2]=t146;
t146=pick(2);
X t149=lf[259]; // CONST
t143=top();
S_DATA(t143)[3]=t149;
t149=pick(2);
X t151=TO_N(2); // ICONST
save(t151);
X t152=select1(TO_N(2),t149); // SELECT (simple)
t151=restore();
X t150=select1(t151,t152); // SELECT
t143=top();
S_DATA(t143)[4]=t150;
t150=pick(2);
X t153=lf[288]; // CONST
t143=top();
S_DATA(t143)[5]=t153;
t143=restore();
restore();
t142=top();
S_DATA(t142)[0]=t143;
t143=pick(2);
X t154=select1(TO_N(4),t143); // SELECT (simple)
t142=top();
S_DATA(t142)[1]=t154;
t142=restore();
restore();
X t155=___backend_3aconc(t142); // REF: backend:conc
t137=top();
S_DATA(t137)[3]=t155;
t137=restore();
restore();
t85=t137;}
t78=t85;}
t77=restore();
X t76=select1(t77,t78); // SELECT
t67=top();
S_DATA(t67)[1]=t76;
t67=restore();
restore();
save(t67);
X t156=allocate(3); // CONS
save(t156);
t67=pick(2);
save(t67);
X t157=allocate(8); // CONS
save(t157);
t67=pick(2);
X t158=lf[280]; // CONST
t157=top();
S_DATA(t157)[0]=t158;
t158=pick(2);
X t160=TO_N(1); // ICONST
save(t160);
X t161=select1(TO_N(1),t158); // SELECT (simple)
t160=restore();
X t159=select1(t160,t161); // SELECT
t157=top();
S_DATA(t157)[1]=t159;
t159=pick(2);
X t162=lf[254]; // CONST
t157=top();
S_DATA(t157)[2]=t162;
t162=pick(2);
X t163=lf[280]; // CONST
t157=top();
S_DATA(t157)[3]=t163;
t163=pick(2);
X t165=TO_N(2); // ICONST
save(t165);
X t166=select1(TO_N(1),t163); // SELECT (simple)
t165=restore();
X t164=select1(t165,t166); // SELECT
t157=top();
S_DATA(t157)[4]=t164;
t164=pick(2);
X t167=lf[255]; // CONST
t157=top();
S_DATA(t157)[5]=t167;
t167=pick(2);
X t168=lf[280]; // CONST
t157=top();
S_DATA(t157)[6]=t168;
t168=pick(2);
X t169=lf[256]; // CONST
t157=top();
S_DATA(t157)[7]=t169;
t157=restore();
restore();
t156=top();
S_DATA(t156)[0]=t157;
t157=pick(2);
X t171=TO_N(1); // ICONST
save(t171);
X t172=select1(TO_N(1),t157); // SELECT (simple)
t171=restore();
X t170=select1(t171,t172); // SELECT
t156=top();
S_DATA(t156)[1]=t170;
t170=pick(2);
X t173=select1(TO_N(2),t170); // SELECT (simple)
t156=top();
S_DATA(t156)[2]=t173;
t156=restore();
restore();
X t174=___backend_3aconcx(t156); // REF: backend:concx
RETURN(t174);}
//---------------------------------------- backend:two_arg_fun (c-backend.fp:303)
DEFINE(___backend_3atwo_5farg_5ffun){
ENTRY;
loop:;
tracecall("c-backend.fp:303:  backend:two_arg_fun");
save(x);
X t1=allocate(2); // CONS
save(t1);
x=pick(2);
X t2=select1(TO_N(1),x); // SELECT (simple)
t1=top();
S_DATA(t1)[0]=t2;
t2=pick(2);
X t3=select1(TO_N(2),t2); // SELECT (simple)
X t4=___backend_3agensym(t3); // REF: backend:gensym
t1=top();
S_DATA(t1)[1]=t4;
t1=restore();
restore();
X t5=___al(t1); // REF: al
save(t5);
X t6=allocate(4); // CONS
save(t6);
t5=pick(2);
X t7=select1(TO_N(1),t5); // SELECT (simple)
t6=top();
S_DATA(t6)[0]=t7;
t7=pick(2);
X t8=select1(TO_N(2),t7); // SELECT (simple)
t6=top();
S_DATA(t6)[1]=t8;
t8=pick(2);
save(t8);
X t9=allocate(2); // CONS
save(t9);
t8=pick(2);
X t11=TO_N(4); // ICONST
save(t11);
X t12=select1(TO_N(1),t8); // SELECT (simple)
t11=restore();
X t10=select1(t11,t12); // SELECT
t9=top();
S_DATA(t9)[0]=t10;
t10=pick(2);
save(t10);
X t13=allocate(3); // CONS
save(t13);
t10=pick(2);
X t15=TO_N(3); // ICONST
save(t15);
X t16=select1(TO_N(1),t10); // SELECT (simple)
t15=restore();
X t14=select1(t15,t16); // SELECT
t13=top();
S_DATA(t13)[0]=t14;
t14=pick(2);
X t18=TO_N(1); // ICONST
save(t18);
X t19=select1(TO_N(3),t14); // SELECT (simple)
t18=restore();
X t17=select1(t18,t19); // SELECT
t13=top();
S_DATA(t13)[1]=t17;
t17=pick(2);
save(t17);
X t20=allocate(2); // CONS
save(t20);
t17=pick(2);
X t22=TO_N(3); // ICONST
save(t22);
X t23=select1(TO_N(1),t17); // SELECT (simple)
t22=restore();
X t21=select1(t22,t23); // SELECT
t20=top();
S_DATA(t20)[0]=t21;
t21=pick(2);
save(t21);
X t24=allocate(2); // CONS
save(t24);
t21=pick(2);
X t26=TO_N(3); // ICONST
save(t26);
X t27=select1(TO_N(1),t21); // SELECT (simple)
t26=restore();
X t25=select1(t26,t27); // SELECT
t24=top();
S_DATA(t24)[0]=t25;
t25=pick(2);
X t28=select1(TO_N(3),t25); // SELECT (simple)
save(t28);
X t29=allocate(4); // CONS
save(t29);
t28=pick(2);
X t30=select1(TO_N(1),t28); // SELECT (simple)
t29=top();
S_DATA(t29)[0]=t30;
t30=pick(2);
X t31=select1(TO_N(2),t30); // SELECT (simple)
t29=top();
S_DATA(t29)[1]=t31;
t31=pick(2);
X t32=F; // ICONST
t29=top();
S_DATA(t29)[2]=t32;
t32=pick(2);
X t33=select1(TO_N(4),t32); // SELECT (simple)
t29=top();
S_DATA(t29)[3]=t33;
t29=restore();
restore();
t24=top();
S_DATA(t24)[1]=t29;
t24=restore();
restore();
X t34=___backend_3asave_5farg_5fif(t24); // REF: backend:save_arg_if
t20=top();
S_DATA(t20)[1]=t34;
t20=restore();
restore();
X t35=___backend_3at_5fexpr(t20); // REF: backend:t_expr
t13=top();
S_DATA(t13)[2]=t35;
t13=restore();
restore();
X t36=___backend_3arestore_5farg_5fif(t13); // REF: backend:restore_arg_if
t9=top();
S_DATA(t9)[1]=t36;
t9=restore();
restore();
X t37=___backend_3asave_5farg_5fif(t9); // REF: backend:save_arg_if
t6=top();
S_DATA(t6)[2]=t37;
t37=pick(2);
X t39=TO_N(1); // ICONST
save(t39);
X t40=select1(TO_N(3),t37); // SELECT (simple)
t39=restore();
X t38=select1(t39,t40); // SELECT
t6=top();
S_DATA(t6)[3]=t38;
t6=restore();
restore();
save(t6);
X t41=allocate(4); // CONS
save(t41);
t6=pick(2);
X t42=select1(TO_N(1),t6); // SELECT (simple)
t41=top();
S_DATA(t41)[0]=t42;
t42=pick(2);
X t43=select1(TO_N(2),t42); // SELECT (simple)
t41=top();
S_DATA(t41)[1]=t43;
t43=pick(2);
X t44=select1(TO_N(3),t43); // SELECT (simple)
t41=top();
S_DATA(t41)[2]=t44;
t44=pick(2);
save(t44);
X t45=allocate(3); // CONS
save(t45);
t44=pick(2);
X t47=TO_N(4); // ICONST
save(t47);
X t48=select1(TO_N(1),t44); // SELECT (simple)
t47=restore();
X t46=select1(t47,t48); // SELECT
t45=top();
S_DATA(t45)[0]=t46;
t46=pick(2);
X t50=TO_N(1); // ICONST
save(t50);
X t51=select1(TO_N(3),t46); // SELECT (simple)
t50=restore();
X t49=select1(t50,t51); // SELECT
t45=top();
S_DATA(t45)[1]=t49;
t49=pick(2);
save(t49);
X t52=allocate(2); // CONS
save(t52);
t49=pick(2);
X t54=TO_N(4); // ICONST
save(t54);
X t55=select1(TO_N(1),t49); // SELECT (simple)
t54=restore();
X t53=select1(t54,t55); // SELECT
t52=top();
S_DATA(t52)[0]=t53;
t53=pick(2);
save(t53);
X t56=allocate(2); // CONS
save(t56);
t53=pick(2);
X t57=select1(TO_N(4),t53); // SELECT (simple)
t56=top();
S_DATA(t56)[0]=t57;
t57=pick(2);
X t58=select1(TO_N(3),t57); // SELECT (simple)
save(t58);
X t59=allocate(4); // CONS
save(t59);
t58=pick(2);
X t60=select1(TO_N(1),t58); // SELECT (simple)
t59=top();
S_DATA(t59)[0]=t60;
t60=pick(2);
X t61=select1(TO_N(2),t60); // SELECT (simple)
t59=top();
S_DATA(t59)[1]=t61;
t61=pick(2);
X t62=F; // ICONST
t59=top();
S_DATA(t59)[2]=t62;
t62=pick(2);
X t63=select1(TO_N(4),t62); // SELECT (simple)
t59=top();
S_DATA(t59)[3]=t63;
t59=restore();
restore();
t56=top();
S_DATA(t56)[1]=t59;
t56=restore();
restore();
save(t56);
X t64=allocate(2); // CONS
save(t64);
t56=pick(2);
save(t56);
X t65=allocate(1); // CONS
save(t65);
t56=pick(2);
X t66=select1(TO_N(1),t56); // SELECT (simple)
t65=top();
S_DATA(t65)[0]=t66;
t65=restore();
restore();
t64=top();
S_DATA(t64)[0]=t65;
t65=pick(2);
X t67=select1(TO_N(2),t65); // SELECT (simple)
X t68=___tl(t67); // REF: tl
t64=top();
S_DATA(t64)[1]=t68;
t64=restore();
restore();
X t69=___cat(t64); // REF: cat
t52=top();
S_DATA(t52)[1]=t69;
t52=restore();
restore();
X t70=___backend_3at_5fexpr(t52); // REF: backend:t_expr
t45=top();
S_DATA(t45)[2]=t70;
t45=restore();
restore();
X t71=___backend_3arestore_5farg_5fif(t45); // REF: backend:restore_arg_if
t41=top();
S_DATA(t41)[3]=t71;
t41=restore();
restore();
RETURN(t41);}
//---------------------------------------- backend:unit_value (c-backend.fp:316)
DEFINE(___backend_3aunit_5fvalue){
ENTRY;
loop:;
tracecall("c-backend.fp:316:  backend:unit_value");
save(x); // COND
X t3=lf[293]; // CONST
save(t3);
X t4=select1(TO_N(1),x); // SELECT (simple)
t3=restore();
X t2=(t3==t4)||eq1(t3,t4)?T:F; // EQ
X t1;
x=restore();
if(t2!=F){
save(x); // OR
save(x);
X t6=allocate(2); // CONS
save(t6);
x=pick(2);
X t7=select1(TO_N(3),x); // SELECT (simple)
t6=top();
S_DATA(t6)[0]=t7;
t7=pick(2);
X t8=lf[269]; // CONST
t6=top();
S_DATA(t6)[1]=t8;
t6=restore();
restore();
X t9=___fetch(t6); // REF: fetch
X t5=t9;
x=restore();
if(t5==F){
X t10=lf[270]; // CONST
X t11=___cat(t10); // REF: cat
t5=t11;}
t1=t5;
}else{
X t12=lf[270]; // CONST
X t13=___cat(t12); // REF: cat
t1=t13;}
RETURN(t1);}
//---------------------------------------- backend:tail (c-backend.fp:343)
DEFINE(___backend_3atail){
ENTRY;
loop:;
tracecall("c-backend.fp:343:  backend:tail");
save(x);
X t1=allocate(4); // CONS
save(t1);
x=pick(2);
X t3=TO_N(1); // ICONST
save(t3);
X t4=select1(TO_N(2),x); // SELECT (simple)
t3=restore();
X t2=select1(t3,t4); // SELECT
t1=top();
S_DATA(t1)[0]=t2;
t2=pick(2);
X t6=TO_N(2); // ICONST
save(t6);
X t7=select1(TO_N(2),t2); // SELECT (simple)
t6=restore();
X t5=select1(t6,t7); // SELECT
t1=top();
S_DATA(t1)[1]=t5;
t5=pick(2);
X t8=select1(TO_N(1),t5); // SELECT (simple)
t1=top();
S_DATA(t1)[2]=t8;
t8=pick(2);
X t10=TO_N(4); // ICONST
save(t10);
X t11=select1(TO_N(2),t8); // SELECT (simple)
t10=restore();
X t9=select1(t10,t11); // SELECT
t1=top();
S_DATA(t1)[3]=t9;
t1=restore();
restore();
RETURN(t1);}
//---------------------------------------- backend:nontail (c-backend.fp:344)
DEFINE(___backend_3anontail){
ENTRY;
loop:;
tracecall("c-backend.fp:344:  backend:nontail");
save(x);
X t1=allocate(4); // CONS
save(t1);
x=pick(2);
X t2=select1(TO_N(1),x); // SELECT (simple)
t1=top();
S_DATA(t1)[0]=t2;
t2=pick(2);
X t3=select1(TO_N(2),t2); // SELECT (simple)
t1=top();
S_DATA(t1)[1]=t3;
t3=pick(2);
X t4=F; // ICONST
t1=top();
S_DATA(t1)[2]=t4;
t4=pick(2);
X t5=select1(TO_N(4),t4); // SELECT (simple)
t1=top();
S_DATA(t1)[3]=t5;
t1=restore();
restore();
RETURN(t1);}
//---------------------------------------- backend:conc (c-backend.fp:368)
DEFINE(___backend_3aconc){
ENTRY;
loop:;
tracecall("c-backend.fp:368:  backend:conc");
save(x);
X t1=allocate(4); // CONS
save(t1);
x=pick(2);
X t3=TO_N(1); // ICONST
save(t3);
X t4=select1(TO_N(2),x); // SELECT (simple)
t3=restore();
X t2=select1(t3,t4); // SELECT
t1=top();
S_DATA(t1)[0]=t2;
t2=pick(2);
X t6=TO_N(2); // ICONST
save(t6);
X t7=select1(TO_N(2),t2); // SELECT (simple)
t6=restore();
X t5=select1(t6,t7); // SELECT
t1=top();
S_DATA(t1)[1]=t5;
t5=pick(2);
X t9=TO_N(3); // ICONST
save(t9);
X t10=select1(TO_N(2),t5); // SELECT (simple)
t9=restore();
X t8=select1(t9,t10); // SELECT
t1=top();
S_DATA(t1)[2]=t8;
t8=pick(2);
save(t8);
X t11=allocate(2); // CONS
save(t11);
t8=pick(2);
X t13=TO_N(4); // ICONST
save(t13);
X t14=select1(TO_N(2),t8); // SELECT (simple)
t13=restore();
X t12=select1(t13,t14); // SELECT
t11=top();
S_DATA(t11)[0]=t12;
t12=pick(2);
X t15=select1(TO_N(1),t12); // SELECT (simple)
t11=top();
S_DATA(t11)[1]=t15;
t11=restore();
restore();
X t16=___cat(t11); // REF: cat
t1=top();
S_DATA(t1)[3]=t16;
t1=restore();
restore();
RETURN(t1);}
//---------------------------------------- backend:concx (c-backend.fp:369)
DEFINE(___backend_3aconcx){
ENTRY;
loop:;
tracecall("c-backend.fp:369:  backend:concx");
save(x);
X t1=allocate(4); // CONS
save(t1);
x=pick(2);
X t2=select1(TO_N(2),x); // SELECT (simple)
t1=top();
S_DATA(t1)[0]=t2;
t2=pick(2);
X t4=TO_N(2); // ICONST
save(t4);
X t5=select1(TO_N(3),t2); // SELECT (simple)
t4=restore();
X t3=select1(t4,t5); // SELECT
t1=top();
S_DATA(t1)[1]=t3;
t3=pick(2);
X t7=TO_N(3); // ICONST
save(t7);
X t8=select1(TO_N(3),t3); // SELECT (simple)
t7=restore();
X t6=select1(t7,t8); // SELECT
t1=top();
S_DATA(t1)[2]=t6;
t6=pick(2);
save(t6);
X t9=allocate(2); // CONS
save(t9);
t6=pick(2);
X t11=TO_N(4); // ICONST
save(t11);
X t12=select1(TO_N(3),t6); // SELECT (simple)
t11=restore();
X t10=select1(t11,t12); // SELECT
t9=top();
S_DATA(t9)[0]=t10;
t10=pick(2);
X t13=select1(TO_N(1),t10); // SELECT (simple)
t9=top();
S_DATA(t9)[1]=t13;
t9=restore();
restore();
X t14=___cat(t9); // REF: cat
t1=top();
S_DATA(t1)[3]=t14;
t1=restore();
restore();
RETURN(t1);}
//---------------------------------------- backend:save_arg_if (c-backend.fp:372)
DEFINE(___backend_3asave_5farg_5fif){
ENTRY;
loop:;
tracecall("c-backend.fp:372:  backend:save_arg_if");
save(x); // COND
X t2=select1(TO_N(1),x); // SELECT (simple)
X t3=___backend_3aconst_5farg(t2); // REF: backend:const_arg
X t1;
x=restore();
if(t3!=F){
X t4=select1(TO_N(2),x); // SELECT (simple)
t1=t4;
}else{
X t5=select1(TO_N(2),x); // SELECT (simple)
save(t5);
X t6=allocate(2); // CONS
save(t6);
t5=pick(2);
save(t5);
X t7=allocate(4); // CONS
save(t7);
t5=pick(2);
X t8=lf[280]; // CONST
t7=top();
S_DATA(t7)[0]=t8;
t8=pick(2);
X t9=lf[271]; // CONST
t7=top();
S_DATA(t7)[1]=t9;
t9=pick(2);
X t10=select1(TO_N(1),t9); // SELECT (simple)
t7=top();
S_DATA(t7)[2]=t10;
t10=pick(2);
X t11=lf[272]; // CONST
t7=top();
S_DATA(t7)[3]=t11;
t7=restore();
restore();
t6=top();
S_DATA(t6)[0]=t7;
t7=pick(2);
X t12=___id(t7); // REF: id
t6=top();
S_DATA(t6)[1]=t12;
t6=restore();
restore();
X t13=___backend_3aconc(t6); // REF: backend:conc
t1=t13;}
RETURN(t1);}
//---------------------------------------- backend:restore_arg_if (c-backend.fp:373)
DEFINE(___backend_3arestore_5farg_5fif){
ENTRY;
loop:;
tracecall("c-backend.fp:373:  backend:restore_arg_if");
save(x); // COND
X t2=select1(TO_N(1),x); // SELECT (simple)
X t3=___backend_3aconst_5farg(t2); // REF: backend:const_arg
X t1;
x=restore();
if(t3!=F){
X t4=select1(TO_N(3),x); // SELECT (simple)
t1=t4;
}else{
X t5=___tl(x); // REF: tl
save(t5);
X t6=allocate(2); // CONS
save(t6);
t5=pick(2);
save(t5);
X t7=allocate(3); // CONS
save(t7);
t5=pick(2);
X t8=lf[280]; // CONST
t7=top();
S_DATA(t7)[0]=t8;
t8=pick(2);
X t9=select1(TO_N(1),t8); // SELECT (simple)
t7=top();
S_DATA(t7)[1]=t9;
t9=pick(2);
X t10=lf[273]; // CONST
t7=top();
S_DATA(t7)[2]=t10;
t7=restore();
restore();
t6=top();
S_DATA(t6)[0]=t7;
t7=pick(2);
X t11=select1(TO_N(2),t7); // SELECT (simple)
t6=top();
S_DATA(t6)[1]=t11;
t6=restore();
restore();
X t12=___backend_3aconc(t6); // REF: backend:conc
t1=t12;}
RETURN(t1);}
//---------------------------------------- backend:const_arg (c-backend.fp:374)
DEFINE(___backend_3aconst_5farg){
ENTRY;
loop:;
tracecall("c-backend.fp:374:  backend:const_arg");
X t1=F;
save(x); // PCONSL
if(!IS_S(x)||S_LENGTH(x)<1) goto t2;
x=S_DATA(top())[0];
save(x); // OR
X t5=lf[290]; // CONST
save(t5);
X t6=___id(x); // REF: id
t5=restore();
X t4=(t5==t6)||eq1(t5,t6)?T:F; // EQ
X t3=t4;
x=restore();
if(t3==F){
X t8=lf[291]; // CONST
save(t8);
X t9=___id(x); // REF: id
t8=restore();
X t7=(t8==t9)||eq1(t8,t9)?T:F; // EQ
t3=t7;}
if(t3==F) goto t2;
t1=T;
t2:
restore();
RETURN(t1);}
//---------------------------------------- backend:gensym (c-backend.fp:379)
DEFINE(___backend_3agensym){
ENTRY;
loop:;
tracecall("c-backend.fp:379:  backend:gensym");
save(x);
X t1=allocate(2); // CONS
save(t1);
x=pick(2);
X t2=select1(TO_N(2),x); // SELECT (simple)
save(t2);
X t3=allocate(2); // CONS
save(t3);
t2=pick(2);
X t4=TO_N(116); // ICONST
t3=top();
S_DATA(t3)[0]=t4;
t4=pick(2);
X t5=___tos(t4); // REF: tos
t3=top();
S_DATA(t3)[1]=t5;
t3=restore();
restore();
X t6=___al(t3); // REF: al
t1=top();
S_DATA(t1)[0]=t6;
t6=pick(2);
save(t6);
X t7=allocate(4); // CONS
save(t7);
t6=pick(2);
X t8=select1(TO_N(1),t6); // SELECT (simple)
t7=top();
S_DATA(t7)[0]=t8;
t8=pick(2);
X t9=select1(TO_N(2),t8); // SELECT (simple)
save(t9);
X t10=allocate(2); // CONS
save(t10);
t9=pick(2);
X t11=___id(t9); // REF: id
t10=top();
S_DATA(t10)[0]=t11;
t11=pick(2);
X t12=TO_N(1); // ICONST
t10=top();
S_DATA(t10)[1]=t12;
t10=restore();
restore();
X t13=___add(t10); // REF: add
t7=top();
S_DATA(t7)[1]=t13;
t13=pick(2);
X t14=select1(TO_N(3),t13); // SELECT (simple)
t7=top();
S_DATA(t7)[2]=t14;
t14=pick(2);
X t15=select1(TO_N(4),t14); // SELECT (simple)
t7=top();
S_DATA(t7)[3]=t15;
t7=restore();
restore();
t1=top();
S_DATA(t1)[1]=t7;
t1=restore();
restore();
RETURN(t1);}
//---------------------------------------- backend:gensyms (c-backend.fp:380)
DEFINE(___backend_3agensyms){
ENTRY;
loop:;
tracecall("c-backend.fp:380:  backend:gensyms");
save(x);
X t1=allocate(2); // CONS
save(t1);
x=pick(2);
save(x);
X t2=allocate(2); // CONS
save(t2);
x=pick(2);
X t4=TO_N(2); // ICONST
save(t4);
X t5=select1(TO_N(2),x); // SELECT (simple)
t4=restore();
X t3=select1(t4,t5); // SELECT
t2=top();
S_DATA(t2)[0]=t3;
t3=pick(2);
X t6=select1(TO_N(1),t3); // SELECT (simple)
X t7=___iota(t6); // REF: iota
int t9; // ALPHA
check_S(t7,"@");
int t10=S_LENGTH(t7);
save(t7);
X t8=allocate(t10);
save(t8);
for(t9=0;t9<t10;++t9){
X t8=S_DATA(pick(2))[t9];
save(t8);
X t12=___id(t8); // REF: id
t8=restore();
X t13=TO_N(1); // ICONST
X t11=___sub_5fop2(t12,t13); // OP2: sub
S_DATA(top())[t9]=t11;}
t8=restore();
restore();
t2=top();
S_DATA(t2)[1]=t8;
t2=restore();
restore();
X t14=___dl(t2); // REF: dl
int t16; // ALPHA
check_S(t14,"@");
int t17=S_LENGTH(t14);
save(t14);
X t15=allocate(t17);
save(t15);
for(t16=0;t16<t17;++t16){
X t15=S_DATA(pick(2))[t16];
X t18=___add(t15); // REF: add
save(t18);
X t19=allocate(2); // CONS
save(t19);
t18=pick(2);
X t20=TO_N(116); // ICONST
t19=top();
S_DATA(t19)[0]=t20;
t20=pick(2);
X t21=___tos(t20); // REF: tos
t19=top();
S_DATA(t19)[1]=t21;
t19=restore();
restore();
X t22=___al(t19); // REF: al
S_DATA(top())[t16]=t22;}
t15=restore();
restore();
t1=top();
S_DATA(t1)[0]=t15;
t15=pick(2);
save(t15);
X t23=allocate(4); // CONS
save(t23);
t15=pick(2);
X t25=TO_N(1); // ICONST
save(t25);
X t26=select1(TO_N(2),t15); // SELECT (simple)
t25=restore();
X t24=select1(t25,t26); // SELECT
t23=top();
S_DATA(t23)[0]=t24;
t24=pick(2);
save(t24);
X t28=select1(TO_N(1),t24); // SELECT (simple)
t24=restore();
save(t28);
X t30=TO_N(2); // ICONST
save(t30);
X t31=select1(TO_N(2),t24); // SELECT (simple)
t30=restore();
X t29=select1(t30,t31); // SELECT
t28=restore();
X t27=___add_5fop2(t28,t29); // OP2: add
t23=top();
S_DATA(t23)[1]=t27;
t27=pick(2);
X t33=TO_N(3); // ICONST
save(t33);
X t34=select1(TO_N(2),t27); // SELECT (simple)
t33=restore();
X t32=select1(t33,t34); // SELECT
t23=top();
S_DATA(t23)[2]=t32;
t32=pick(2);
X t36=TO_N(4); // ICONST
save(t36);
X t37=select1(TO_N(2),t32); // SELECT (simple)
t36=restore();
X t35=select1(t36,t37); // SELECT
t23=top();
S_DATA(t23)[3]=t35;
t23=restore();
restore();
t1=top();
S_DATA(t1)[1]=t23;
t1=restore();
restore();
RETURN(t1);}
//---------------------------------------- backend:mangle (c-backend.fp:388)
DEFINE(___backend_3amangle){
ENTRY;
loop:;
tracecall("c-backend.fp:388:  backend:mangle");
save(x);
X t1=allocate(2); // CONS
save(t1);
x=pick(2);
X t2=lf[274]; // CONST
t1=top();
S_DATA(t1)[0]=t2;
t2=pick(2);
X t3=___tos(t2); // REF: tos
int t5; // ALPHA
check_S(t3,"@");
int t6=S_LENGTH(t3);
save(t3);
X t4=allocate(t6);
save(t4);
for(t5=0;t5<t6;++t5){
X t4=S_DATA(pick(2))[t5];
save(t4); // COND
save(t4);
X t8=allocate(2); // CONS
save(t8);
t4=pick(2);
X t9=___id(t4); // REF: id
t8=top();
S_DATA(t8)[0]=t9;
t9=pick(2);
X t10=lf[275]; // CONST
t8=top();
S_DATA(t8)[1]=t10;
t8=restore();
restore();
X t11=___index(t8); // REF: index
X t7;
t4=restore();
if(t11!=F){
save(t4);
X t12=allocate(1); // CONS
save(t12);
t4=pick(2);
X t13=___id(t4); // REF: id
t12=top();
S_DATA(t12)[0]=t13;
t12=restore();
restore();
t7=t12;
}else{
save(t4); // COND
save(t4);
X t15=allocate(2); // CONS
save(t15);
t4=pick(2);
X t16=___id(t4); // REF: id
t15=top();
S_DATA(t15)[0]=t16;
t16=pick(2);
X t17=TO_N(10); // ICONST
t15=top();
S_DATA(t15)[1]=t17;
t15=restore();
restore();
X t19=TO_N(-1); // ICONST
save(t19);
X t20=___cmp(t15); // REF: cmp
t19=restore();
X t18=(t19==t20)||eq1(t19,t20)?T:F; // EQ
X t14;
t4=restore();
if(t18!=F){
save(t4);
X t21=allocate(2); // CONS
save(t21);
t4=pick(2);
X t22=lf[276]; // CONST
t21=top();
S_DATA(t21)[0]=t22;
t22=pick(2);
X t23=___backend_3ahexdigit(t22); // REF: backend:hexdigit
t21=top();
S_DATA(t21)[1]=t23;
t21=restore();
restore();
X t24=___ar(t21); // REF: ar
t14=t24;
}else{
save(t4);
X t25=allocate(3); // CONS
save(t25);
t4=pick(2);
X t26=TO_N(95); // ICONST
t25=top();
S_DATA(t25)[0]=t26;
t26=pick(2);
save(t26);
X t28=___id(t26); // REF: id
t26=restore();
X t29=TO_N(16); // ICONST
X t27=___div_5fop2(t28,t29); // OP2: div
X t30=___backend_3ahexdigit(t27); // REF: backend:hexdigit
t25=top();
S_DATA(t25)[1]=t30;
t30=pick(2);
save(t30);
X t32=___id(t30); // REF: id
t30=restore();
X t33=TO_N(16); // ICONST
X t31=___mod_5fop2(t32,t33); // OP2: mod
X t34=___backend_3ahexdigit(t31); // REF: backend:hexdigit
t25=top();
S_DATA(t25)[2]=t34;
t25=restore();
restore();
t14=t25;}
t7=t14;}
S_DATA(top())[t5]=t7;}
t4=restore();
restore();
t1=top();
S_DATA(t1)[1]=t4;
t1=restore();
restore();
X t35=___al(t1); // REF: al
X t36=___cat(t35); // REF: cat
RETURN(t36);}
//---------------------------------------- backend:hexdigit (c-backend.fp:395)
DEFINE(___backend_3ahexdigit){
ENTRY;
loop:;
tracecall("c-backend.fp:395:  backend:hexdigit");
save(x);
save(x); // COND
save(x);
X t3=allocate(2); // CONS
save(t3);
x=pick(2);
X t4=___id(x); // REF: id
t3=top();
S_DATA(t3)[0]=t4;
t4=pick(2);
X t5=TO_N(10); // ICONST
t3=top();
S_DATA(t3)[1]=t5;
t3=restore();
restore();
X t7=TO_N(-1); // ICONST
save(t7);
X t8=___cmp(t3); // REF: cmp
t7=restore();
X t6=(t7==t8)||eq1(t7,t8)?T:F; // EQ
X t2;
x=restore();
if(t6!=F){
X t9=TO_N(48); // ICONST
t2=t9;
}else{
X t10=lf[277]; // CONST
X t11=___sub(t10); // REF: sub
t2=t11;}
x=restore();
save(t2);
X t12=___id(x); // REF: id
t2=restore();
X t1=___add_5fop2(t2,t12); // OP2: add
RETURN(t1);}
//---------------------------------------- unparse:unparse (unparse.fp:16)
DEFINE(___unparse_3aunparse){
ENTRY;
loop:;
tracecall("unparse.fp:16:  unparse:unparse");
save(x);
X t1=allocate(2); // CONS
save(t1);
x=pick(2);
X t2=TO_N(1); // ICONST
t1=top();
S_DATA(t1)[0]=t2;
t2=pick(2);
X t3=___id(t2); // REF: id
t1=top();
S_DATA(t1)[1]=t3;
t1=restore();
restore();
X t4=t1; // WHILE
for(;;){
save(t4);
save(t4);
X t5=allocate(2); // CONS
save(t5);
t4=pick(2);
X t6=select1(TO_N(1),t4); // SELECT (simple)
t5=top();
S_DATA(t5)[0]=t6;
t6=pick(2);
X t7=select1(TO_N(2),t6); // SELECT (simple)
X t8=___len(t7); // REF: len
t5=top();
S_DATA(t5)[1]=t8;
t5=restore();
restore();
X t9=___le(t5); // REF: le
t4=restore();
if(t9==F) break;
save(t4); // SEQ
save(t4);
X t11=select1(TO_N(1),t4); // SELECT (simple)
t4=restore();
save(t11);
X t12=select1(TO_N(2),t4); // SELECT (simple)
t11=restore();
X t10=select1(t11,t12); // SELECT
save(t10); // SEQ
save(t10);
X t13=allocate(4); // CONS
save(t13);
t10=pick(2);
X t14=lf[280]; // CONST
t13=top();
S_DATA(t13)[0]=t14;
t14=pick(2);
X t15=select1(TO_N(1),t14); // SELECT (simple)
X t16=___tos(t15); // REF: tos
t13=top();
S_DATA(t13)[1]=t16;
t16=pick(2);
X t17=lf[278]; // CONST
t13=top();
S_DATA(t13)[2]=t17;
t17=pick(2);
X t19=TO_N(2); // ICONST
save(t19);
X t20=select1(TO_N(2),t17); // SELECT (simple)
t19=restore();
X t18=select1(t19,t20); // SELECT
X t21=___tos(t18); // REF: tos
t13=top();
S_DATA(t13)[3]=t21;
t13=restore();
restore();
X t22=___cat(t13); // REF: cat
save(t22); // SEQ
X t23=____5femit(t22); // REF: _emit
t22=restore();
X t24=TO_N(10); // ICONST
X t25=____5femit(t24); // REF: _emit
t10=restore();
X t27=TO_N(3); // ICONST
save(t27);
save(t10);
X t28=allocate(2); // CONS
save(t28);
t10=pick(2);
X t29=TO_N(2); // ICONST
t28=top();
S_DATA(t28)[0]=t29;
t29=pick(2);
X t30=select1(TO_N(2),t29); // SELECT (simple)
t28=top();
S_DATA(t28)[1]=t30;
t28=restore();
restore();
save(t28);
X t31=allocate(2); // CONS
save(t31);
t28=pick(2);
X t32=select1(TO_N(2),t28); // SELECT (simple)
t31=top();
S_DATA(t31)[0]=t32;
t32=pick(2);
X t33=select1(TO_N(1),t32); // SELECT (simple)
save(t33);
X t34=allocate(2); // CONS
save(t34);
t33=pick(2);
X t35=___id(t33); // REF: id
t34=top();
S_DATA(t34)[0]=t35;
t35=pick(2);
X t36=lf[279]; // CONST
t34=top();
S_DATA(t34)[1]=t36;
t34=restore();
restore();
X t37=___unparse_3aindent(t34); // REF: unparse:indent
t31=top();
S_DATA(t31)[1]=t37;
t31=restore();
restore();
X t38=___unparse_3aup(t31); // REF: unparse:up
t27=restore();
X t26=select1(t27,t38); // SELECT
save(t26); // SEQ
X t39=____5femit(t26); // REF: _emit
t26=restore();
X t40=TO_N(10); // ICONST
X t41=____5femit(t40); // REF: _emit
t4=restore();
save(t4);
X t42=allocate(2); // CONS
save(t42);
t4=pick(2);
X t43=select1(TO_N(1),t4); // SELECT (simple)
save(t43);
X t44=allocate(2); // CONS
save(t44);
t43=pick(2);
X t45=___id(t43); // REF: id
t44=top();
S_DATA(t44)[0]=t45;
t45=pick(2);
X t46=TO_N(1); // ICONST
t44=top();
S_DATA(t44)[1]=t46;
t44=restore();
restore();
X t47=___add(t44); // REF: add
t42=top();
S_DATA(t42)[0]=t47;
t47=pick(2);
X t48=select1(TO_N(2),t47); // SELECT (simple)
t42=top();
S_DATA(t42)[1]=t48;
t42=restore();
restore();
t4=t42;}
RETURN(t4);}
//---------------------------------------- unparse:indent (unparse.fp:32)
DEFINE(___unparse_3aindent){
ENTRY;
loop:;
tracecall("unparse.fp:32:  unparse:indent");
save(x);
X t1=allocate(3); // CONS
save(t1);
x=pick(2);
X t3=TO_N(1); // ICONST
save(t3);
X t4=select1(TO_N(2),x); // SELECT (simple)
t3=restore();
X t2=select1(t3,t4); // SELECT
t1=top();
S_DATA(t1)[0]=t2;
t2=pick(2);
save(t2);
X t5=allocate(2); // CONS
save(t5);
t2=pick(2);
save(t2);
X t8=TO_N(2); // ICONST
save(t8);
X t9=select1(TO_N(2),t2); // SELECT (simple)
t8=restore();
X t7=select1(t8,t9); // SELECT
X t10=___len(t7); // REF: len
t2=restore();
save(t10);
X t11=select1(TO_N(1),t2); // SELECT (simple)
t10=restore();
X t6=___add_5fop2(t10,t11); // OP2: add
t5=top();
S_DATA(t5)[0]=t6;
t6=pick(2);
X t12=TO_N(32); // ICONST
t5=top();
S_DATA(t5)[1]=t12;
t5=restore();
restore();
X t13=___make(t5); // REF: make
t1=top();
S_DATA(t1)[1]=t13;
t13=pick(2);
X t15=TO_N(3); // ICONST
save(t15);
X t16=select1(TO_N(2),t13); // SELECT (simple)
t15=restore();
X t14=select1(t15,t16); // SELECT
t1=top();
S_DATA(t1)[2]=t14;
t1=restore();
restore();
RETURN(t1);}
//---------------------------------------- unparse:prec (unparse.fp:36)
DEFINE(___unparse_3aprec){
ENTRY;
loop:;
tracecall("unparse.fp:36:  unparse:prec");
save(x);
X t1=allocate(3); // CONS
save(t1);
x=pick(2);
X t2=select1(TO_N(1),x); // SELECT (simple)
t1=top();
S_DATA(t1)[0]=t2;
t2=pick(2);
X t4=TO_N(2); // ICONST
save(t4);
X t5=select1(TO_N(2),t2); // SELECT (simple)
t4=restore();
X t3=select1(t4,t5); // SELECT
t1=top();
S_DATA(t1)[1]=t3;
t3=pick(2);
X t7=TO_N(3); // ICONST
save(t7);
X t8=select1(TO_N(2),t3); // SELECT (simple)
t7=restore();
X t6=select1(t7,t8); // SELECT
t1=top();
S_DATA(t1)[2]=t6;
t1=restore();
restore();
RETURN(t1);}
//---------------------------------------- unparse:pr (unparse.fp:43)
DEFINE(___unparse_3apr){
ENTRY;
loop:;
tracecall("unparse.fp:43:  unparse:pr");
save(x);
X t1=allocate(3); // CONS
save(t1);
x=pick(2);
X t3=TO_N(1); // ICONST
save(t3);
X t4=select1(TO_N(2),x); // SELECT (simple)
t3=restore();
X t2=select1(t3,t4); // SELECT
t1=top();
S_DATA(t1)[0]=t2;
t2=pick(2);
X t6=TO_N(2); // ICONST
save(t6);
X t7=select1(TO_N(2),t2); // SELECT (simple)
t6=restore();
X t5=select1(t6,t7); // SELECT
t1=top();
S_DATA(t1)[1]=t5;
t5=pick(2);
save(t5);
X t8=allocate(2); // CONS
save(t8);
t5=pick(2);
X t10=TO_N(3); // ICONST
save(t10);
X t11=select1(TO_N(2),t5); // SELECT (simple)
t10=restore();
X t9=select1(t10,t11); // SELECT
t8=top();
S_DATA(t8)[0]=t9;
t9=pick(2);
X t12=select1(TO_N(1),t9); // SELECT (simple)
t8=top();
S_DATA(t8)[1]=t12;
t8=restore();
restore();
X t13=___cat(t8); // REF: cat
t1=top();
S_DATA(t1)[2]=t13;
t1=restore();
restore();
RETURN(t1);}
//---------------------------------------- unparse:prnl (unparse.fp:44)
DEFINE(___unparse_3aprnl){
ENTRY;
loop:;
tracecall("unparse.fp:44:  unparse:prnl");
save(x);
X t1=allocate(2); // CONS
save(t1);
x=pick(2);
save(x);
X t2=allocate(3); // CONS
save(t2);
x=pick(2);
X t3=select1(TO_N(1),x); // SELECT (simple)
t2=top();
S_DATA(t2)[0]=t3;
t3=pick(2);
X t4=lf[280]; // CONST
t2=top();
S_DATA(t2)[1]=t4;
t4=pick(2);
X t6=TO_N(2); // ICONST
save(t6);
X t7=select1(TO_N(2),t4); // SELECT (simple)
t6=restore();
X t5=select1(t6,t7); // SELECT
t2=top();
S_DATA(t2)[2]=t5;
t2=restore();
restore();
X t8=___cat(t2); // REF: cat
t1=top();
S_DATA(t1)[0]=t8;
t8=pick(2);
X t9=select1(TO_N(2),t8); // SELECT (simple)
t1=top();
S_DATA(t1)[1]=t9;
t1=restore();
restore();
X t10=___unparse_3apr(t1); // REF: unparse:pr
RETURN(t10);}
//---------------------------------------- unparse:prnl0 (unparse.fp:45)
DEFINE(___unparse_3aprnl0){
ENTRY;
loop:;
tracecall("unparse.fp:45:  unparse:prnl0");
save(x);
X t1=allocate(2); // CONS
save(t1);
x=pick(2);
save(x);
X t2=allocate(2); // CONS
save(t2);
x=pick(2);
X t3=lf[280]; // CONST
t2=top();
S_DATA(t2)[0]=t3;
t3=pick(2);
X t4=select1(TO_N(2),t3); // SELECT (simple)
t2=top();
S_DATA(t2)[1]=t4;
t2=restore();
restore();
X t5=___cat(t2); // REF: cat
t1=top();
S_DATA(t1)[0]=t5;
t5=pick(2);
X t6=___id(t5); // REF: id
t1=top();
S_DATA(t1)[1]=t6;
t1=restore();
restore();
X t7=___unparse_3apr(t1); // REF: unparse:pr
RETURN(t7);}
//---------------------------------------- unparse:up (unparse.fp:55)
DEFINE(___unparse_3aup){
ENTRY;
loop:;
tracecall("unparse.fp:55:  unparse:up");
save(x); // COND
X t2=select1(TO_N(1),x); // SELECT (simple)
X t3=F;
save(t2); // PCONS
if(!IS_S(t2)||S_LENGTH(t2)!=4) goto t4;
t2=S_DATA(top())[0];
X t6=lf[317]; // CONST
save(t6);
X t7=___id(t2); // REF: id
t6=restore();
X t5=(t6==t7)||eq1(t6,t7)?T:F; // EQ
if(t5==F) goto t4;
// skipped
t5=S_DATA(top())[2];
X t8=F;
save(t5); // PCONSL
if(!IS_S(t5)||S_LENGTH(t5)<1) goto t9;
t5=S_DATA(top())[0];
save(t5); // OR
X t12=lf[290]; // CONST
save(t12);
X t13=___id(t5); // REF: id
t12=restore();
X t11=(t12==t13)||eq1(t12,t13)?T:F; // EQ
X t10=t11;
t5=restore();
if(t10==F){
X t15=lf[291]; // CONST
save(t15);
X t16=___id(t5); // REF: id
t15=restore();
X t14=(t15==t16)||eq1(t15,t16)?T:F; // EQ
t10=t14;}
if(t10==F) goto t9;
t8=T;
t9:
restore();
if(t8==F) goto t4;
t8=S_DATA(top())[3];
X t17=F;
save(t8); // PCONS
if(!IS_S(t8)||S_LENGTH(t8)!=3) goto t18;
t8=S_DATA(top())[0];
X t20=lf[293]; // CONST
save(t20);
X t21=___id(t8); // REF: id
t20=restore();
X t19=(t20==t21)||eq1(t20,t21)?T:F; // EQ
if(t19==F) goto t18;
// skipped
t19=S_DATA(top())[2];
X t23=lf[282]; // CONST
save(t23);
X t24=___id(t19); // REF: id
t23=restore();
X t22=(t23==t24)||eq1(t23,t24)?T:F; // EQ
if(t22==F) goto t18;
t17=T;
t18:
restore();
if(t17==F) goto t4;
t3=T;
t4:
restore();
X t1;
x=restore();
if(t3!=F){
save(x);
X t25=allocate(2); // CONS
save(t25);
x=pick(2);
X t27=TO_N(3); // ICONST
save(t27);
X t29=TO_N(3); // ICONST
save(t29);
X t30=select1(TO_N(1),x); // SELECT (simple)
t29=restore();
X t28=select1(t29,t30); // SELECT
t27=restore();
X t26=select1(t27,t28); // SELECT
t25=top();
S_DATA(t25)[0]=t26;
t26=pick(2);
save(t26);
X t31=allocate(2); // CONS
save(t31);
t26=pick(2);
X t32=lf[281]; // CONST
t31=top();
S_DATA(t31)[0]=t32;
t32=pick(2);
X t33=select1(TO_N(2),t32); // SELECT (simple)
t31=top();
S_DATA(t31)[1]=t33;
t31=restore();
restore();
X t34=___unparse_3apr(t31); // REF: unparse:pr
t25=top();
S_DATA(t25)[1]=t34;
t25=restore();
restore();
X t35=___unparse_3aup_5fconst(t25); // REF: unparse:up_const
t1=t35;
}else{
save(x); // COND
X t37=select1(TO_N(1),x); // SELECT (simple)
X t38=F;
save(t37); // PCONS
if(!IS_S(t37)||S_LENGTH(t37)!=4) goto t39;
t37=S_DATA(top())[0];
X t41=lf[319]; // CONST
save(t41);
X t42=___id(t37); // REF: id
t41=restore();
X t40=(t41==t42)||eq1(t41,t42)?T:F; // EQ
if(t40==F) goto t39;
// skipped
t40=S_DATA(top())[2];
X t43=F;
save(t40); // PCONSL
if(!IS_S(t40)||S_LENGTH(t40)<1) goto t44;
t40=S_DATA(top())[0];
X t46=lf[291]; // CONST
save(t46);
X t47=___id(t40); // REF: id
t46=restore();
X t45=(t46==t47)||eq1(t46,t47)?T:F; // EQ
if(t45==F) goto t44;
t43=T;
t44:
restore();
if(t43==F) goto t39;
t43=S_DATA(top())[3];
X t48=F;
save(t43); // PCONS
if(!IS_S(t43)||S_LENGTH(t43)!=3) goto t49;
t43=S_DATA(top())[0];
X t51=lf[293]; // CONST
save(t51);
X t52=___id(t43); // REF: id
t51=restore();
X t50=(t51==t52)||eq1(t51,t52)?T:F; // EQ
if(t50==F) goto t49;
// skipped
t50=S_DATA(top())[2];
X t54=lf[282]; // CONST
save(t54);
X t55=___id(t50); // REF: id
t54=restore();
X t53=(t54==t55)||eq1(t54,t55)?T:F; // EQ
if(t53==F) goto t49;
t48=T;
t49:
restore();
if(t48==F) goto t39;
t38=T;
t39:
restore();
X t36;
x=restore();
if(t38!=F){
save(x);
X t56=allocate(2); // CONS
save(t56);
x=pick(2);
X t58=TO_N(3); // ICONST
save(t58);
X t60=TO_N(3); // ICONST
save(t60);
X t61=select1(TO_N(1),x); // SELECT (simple)
t60=restore();
X t59=select1(t60,t61); // SELECT
t58=restore();
X t57=select1(t58,t59); // SELECT
t56=top();
S_DATA(t56)[0]=t57;
t57=pick(2);
save(t57);
X t62=allocate(2); // CONS
save(t62);
t57=pick(2);
X t63=lf[283]; // CONST
t62=top();
S_DATA(t62)[0]=t63;
t63=pick(2);
X t64=select1(TO_N(2),t63); // SELECT (simple)
t62=top();
S_DATA(t62)[1]=t64;
t62=restore();
restore();
X t65=___unparse_3apr(t62); // REF: unparse:pr
t56=top();
S_DATA(t56)[1]=t65;
t56=restore();
restore();
X t66=___unparse_3aup_5fconst(t56); // REF: unparse:up_const
t36=t66;
}else{
save(x); // COND
save(x);
X t68=allocate(2); // CONS
save(t68);
x=pick(2);
X t69=select1(TO_N(1),x); // SELECT (simple)
X t70=___unparse_3anode_5fprec(t69); // REF: unparse:node_prec
t68=top();
S_DATA(t68)[0]=t70;
t70=pick(2);
X t71=select1(TO_N(2),t70); // SELECT (simple)
t68=top();
S_DATA(t68)[1]=t71;
t68=restore();
restore();
save(t68); // COND
X t73=select1(TO_N(1),t68); // SELECT (simple)
X t72;
t68=restore();
if(t73!=F){
save(t68);
X t74=allocate(2); // CONS
save(t74);
t68=pick(2);
X t75=select1(TO_N(1),t68); // SELECT (simple)
t74=top();
S_DATA(t74)[0]=t75;
t75=pick(2);
X t77=TO_N(1); // ICONST
save(t77);
X t78=select1(TO_N(2),t75); // SELECT (simple)
t77=restore();
X t76=select1(t77,t78); // SELECT
t74=top();
S_DATA(t74)[1]=t76;
t74=restore();
restore();
X t80=TO_N(-1); // ICONST
save(t80);
X t81=___cmp(t74); // REF: cmp
t80=restore();
X t79=(t80==t81)||eq1(t80,t81)?T:F; // EQ
t72=t79;
}else{
X t82=F; // ICONST
t72=t82;}
X t67;
x=restore();
if(t72!=F){
save(x);
X t83=allocate(2); // CONS
save(t83);
x=pick(2);
X t84=lf[284]; // CONST
t83=top();
S_DATA(t83)[0]=t84;
t84=pick(2);
save(t84);
X t85=allocate(2); // CONS
save(t85);
t84=pick(2);
X t86=select1(TO_N(1),t84); // SELECT (simple)
t85=top();
S_DATA(t85)[0]=t86;
t86=pick(2);
save(t86);
X t87=allocate(2); // CONS
save(t87);
t86=pick(2);
X t88=lf[285]; // CONST
t87=top();
S_DATA(t87)[0]=t88;
t88=pick(2);
X t89=select1(TO_N(2),t88); // SELECT (simple)
t87=top();
S_DATA(t87)[1]=t89;
t87=restore();
restore();
X t90=___unparse_3apr(t87); // REF: unparse:pr
t85=top();
S_DATA(t85)[1]=t90;
t85=restore();
restore();
X t91=___unparse_3aup1_27(t85); // REF: unparse:up1'
t83=top();
S_DATA(t83)[1]=t91;
t83=restore();
restore();
X t92=___unparse_3apr(t83); // REF: unparse:pr
t67=t92;
}else{
X t93=___unparse_3aup1_27(x); // REF: unparse:up1'
t67=t93;}
t36=t67;}
t1=t36;}
RETURN(t1);}
//---------------------------------------- tmp%1%314 (unparse.fp:63)
DEFINE(___tmp_251_25314){
ENTRY;
loop:;
tracecall("unparse.fp:63:  tmp%1%314");
save(x); // COND
X t2=select1(TO_N(1),x); // SELECT (simple)
X t3=F;
save(t2); // PCONSL
if(!IS_S(t2)||S_LENGTH(t2)<1) goto t4;
t2=S_DATA(top())[0];
X t6=lf[293]; // CONST
save(t6);
X t7=___id(t2); // REF: id
t6=restore();
X t5=(t6==t7)||eq1(t6,t7)?T:F; // EQ
if(t5==F) goto t4;
t3=T;
t4:
restore();
X t1;
x=restore();
if(t3!=F){
save(x);
X t8=allocate(2); // CONS
save(t8);
x=pick(2);
X t10=TO_N(3); // ICONST
save(t10);
X t11=select1(TO_N(1),x); // SELECT (simple)
t10=restore();
X t9=select1(t10,t11); // SELECT
X t12=___tos(t9); // REF: tos
t8=top();
S_DATA(t8)[0]=t12;
t12=pick(2);
X t13=select1(TO_N(2),t12); // SELECT (simple)
t8=top();
S_DATA(t8)[1]=t13;
t8=restore();
restore();
X t14=___unparse_3apr(t8); // REF: unparse:pr
t1=t14;
}else{
save(x); // COND
X t16=select1(TO_N(1),x); // SELECT (simple)
X t17=F;
save(t16); // PCONS
if(!IS_S(t16)||S_LENGTH(t16)!=5) goto t18;
t16=S_DATA(top())[0];
X t20=lf[287]; // CONST
save(t20);
X t21=___id(t16); // REF: id
t20=restore();
X t19=(t20==t21)||eq1(t20,t21)?T:F; // EQ
if(t19==F) goto t18;
// skipped
// skipped
// skipped
t19=S_DATA(top())[4];
X t22=F;
save(t19); // PCONS
if(!IS_S(t19)||S_LENGTH(t19)!=3) goto t23;
t19=S_DATA(top())[0];
X t25=lf[290]; // CONST
save(t25);
X t26=___id(t19); // REF: id
t25=restore();
X t24=(t25==t26)||eq1(t25,t26)?T:F; // EQ
if(t24==F) goto t23;
// skipped
t24=S_DATA(top())[2];
save(t24); // COND
X t28=___id(t24); // REF: id
X t27;
t24=restore();
if(t28!=F){
X t29=F; // ICONST
t27=t29;
}else{
X t30=T; // ICONST
t27=t30;}
if(t27==F) goto t23;
t22=T;
t23:
restore();
if(t22==F) goto t18;
t17=T;
t18:
restore();
X t15;
x=restore();
if(t17!=F){
save(x);
X t31=allocate(2); // CONS
save(t31);
x=pick(2);
X t33=TO_N(4); // ICONST
save(t33);
X t34=select1(TO_N(1),x); // SELECT (simple)
t33=restore();
X t32=select1(t33,t34); // SELECT
t31=top();
S_DATA(t31)[0]=t32;
t32=pick(2);
save(t32);
X t35=allocate(2); // CONS
save(t35);
t32=pick(2);
X t36=TO_N(3); // ICONST
t35=top();
S_DATA(t35)[0]=t36;
t36=pick(2);
save(t36);
X t37=allocate(2); // CONS
save(t37);
t36=pick(2);
X t38=lf[286]; // CONST
t37=top();
S_DATA(t37)[0]=t38;
t38=pick(2);
save(t38);
X t39=allocate(2); // CONS
save(t39);
t38=pick(2);
X t41=TO_N(3); // ICONST
save(t41);
X t42=select1(TO_N(1),t38); // SELECT (simple)
t41=restore();
X t40=select1(t41,t42); // SELECT
t39=top();
S_DATA(t39)[0]=t40;
t40=pick(2);
X t43=select1(TO_N(2),t40); // SELECT (simple)
t39=top();
S_DATA(t39)[1]=t43;
t39=restore();
restore();
X t44=___unparse_3aup(t39); // REF: unparse:up
t37=top();
S_DATA(t37)[1]=t44;
t37=restore();
restore();
X t45=___unparse_3apr(t37); // REF: unparse:pr
t35=top();
S_DATA(t35)[1]=t45;
t35=restore();
restore();
X t46=___unparse_3aprec(t35); // REF: unparse:prec
t31=top();
S_DATA(t31)[1]=t46;
t31=restore();
restore();
X t47=___unparse_3aup(t31); // REF: unparse:up
t15=t47;
}else{
save(x); // COND
X t49=select1(TO_N(1),x); // SELECT (simple)
X t50=F;
save(t49); // PCONSL
if(!IS_S(t49)||S_LENGTH(t49)<1) goto t51;
t49=S_DATA(top())[0];
X t53=lf[287]; // CONST
save(t53);
X t54=___id(t49); // REF: id
t53=restore();
X t52=(t53==t54)||eq1(t53,t54)?T:F; // EQ
if(t52==F) goto t51;
t50=T;
t51:
restore();
X t48;
x=restore();
if(t50!=F){
save(x);
X t55=allocate(2); // CONS
save(t55);
x=pick(2);
X t56=TO_N(-2); // ICONST
t55=top();
S_DATA(t55)[0]=t56;
t56=pick(2);
save(t56);
X t57=allocate(2); // CONS
save(t57);
t56=pick(2);
X t59=TO_N(5); // ICONST
save(t59);
X t60=select1(TO_N(1),t56); // SELECT (simple)
t59=restore();
X t58=select1(t59,t60); // SELECT
t57=top();
S_DATA(t57)[0]=t58;
t58=pick(2);
save(t58);
X t61=allocate(2); // CONS
save(t61);
t58=pick(2);
X t62=lf[288]; // CONST
t61=top();
S_DATA(t61)[0]=t62;
t62=pick(2);
save(t62);
X t63=allocate(2); // CONS
save(t63);
t62=pick(2);
X t65=TO_N(4); // ICONST
save(t65);
X t66=select1(TO_N(1),t62); // SELECT (simple)
t65=restore();
X t64=select1(t65,t66); // SELECT
t63=top();
S_DATA(t63)[0]=t64;
t64=pick(2);
save(t64);
X t67=allocate(2); // CONS
save(t67);
t64=pick(2);
X t68=lf[289]; // CONST
t67=top();
S_DATA(t67)[0]=t68;
t68=pick(2);
save(t68);
X t69=allocate(2); // CONS
save(t69);
t68=pick(2);
X t70=TO_N(2); // ICONST
t69=top();
S_DATA(t69)[0]=t70;
t70=pick(2);
save(t70);
X t71=allocate(2); // CONS
save(t71);
t70=pick(2);
X t73=TO_N(3); // ICONST
save(t73);
X t74=select1(TO_N(1),t70); // SELECT (simple)
t73=restore();
X t72=select1(t73,t74); // SELECT
t71=top();
S_DATA(t71)[0]=t72;
t72=pick(2);
X t75=select1(TO_N(2),t72); // SELECT (simple)
t71=top();
S_DATA(t71)[1]=t75;
t71=restore();
restore();
X t76=___unparse_3aup(t71); // REF: unparse:up
t69=top();
S_DATA(t69)[1]=t76;
t69=restore();
restore();
X t77=___unparse_3aindent(t69); // REF: unparse:indent
t67=top();
S_DATA(t67)[1]=t77;
t67=restore();
restore();
X t78=___unparse_3aprnl(t67); // REF: unparse:prnl
t63=top();
S_DATA(t63)[1]=t78;
t63=restore();
restore();
X t79=___unparse_3aup(t63); // REF: unparse:up
t61=top();
S_DATA(t61)[1]=t79;
t61=restore();
restore();
X t80=___unparse_3aprnl(t61); // REF: unparse:prnl
t57=top();
S_DATA(t57)[1]=t80;
t57=restore();
restore();
X t81=___unparse_3aup(t57); // REF: unparse:up
t55=top();
S_DATA(t55)[1]=t81;
t55=restore();
restore();
X t82=___unparse_3aindent(t55); // REF: unparse:indent
t48=t82;
}else{
save(x); // COND
X t84=select1(TO_N(1),x); // SELECT (simple)
X t85=F;
save(t84); // PCONSL
if(!IS_S(t84)||S_LENGTH(t84)<1) goto t86;
t84=S_DATA(top())[0];
save(t84); // OR
X t89=lf[290]; // CONST
save(t89);
X t90=___id(t84); // REF: id
t89=restore();
X t88=(t89==t90)||eq1(t89,t90)?T:F; // EQ
X t87=t88;
t84=restore();
if(t87==F){
X t92=lf[291]; // CONST
save(t92);
X t93=___id(t84); // REF: id
t92=restore();
X t91=(t92==t93)||eq1(t92,t93)?T:F; // EQ
t87=t91;}
if(t87==F) goto t86;
t85=T;
t86:
restore();
X t83;
x=restore();
if(t85!=F){
save(x);
X t94=allocate(2); // CONS
save(t94);
x=pick(2);
X t96=TO_N(3); // ICONST
save(t96);
X t97=select1(TO_N(1),x); // SELECT (simple)
t96=restore();
X t95=select1(t96,t97); // SELECT
t94=top();
S_DATA(t94)[0]=t95;
t95=pick(2);
save(t95);
X t98=allocate(2); // CONS
save(t98);
t95=pick(2);
X t99=lf[292]; // CONST
t98=top();
S_DATA(t98)[0]=t99;
t99=pick(2);
X t100=select1(TO_N(2),t99); // SELECT (simple)
t98=top();
S_DATA(t98)[1]=t100;
t98=restore();
restore();
X t101=___unparse_3apr(t98); // REF: unparse:pr
t94=top();
S_DATA(t94)[1]=t101;
t94=restore();
restore();
X t102=___unparse_3aup_5fconst(t94); // REF: unparse:up_const
t83=t102;
}else{
save(x); // COND
X t104=select1(TO_N(1),x); // SELECT (simple)
X t105=F;
save(t104); // PCONS
if(!IS_S(t104)||S_LENGTH(t104)!=4) goto t106;
t104=S_DATA(top())[0];
X t108=lf[320]; // CONST
save(t108);
X t109=___id(t104); // REF: id
t108=restore();
X t107=(t108==t109)||eq1(t108,t109)?T:F; // EQ
if(t107==F) goto t106;
// skipped
t107=S_DATA(top())[2];
X t110=F;
save(t107); // PCONS
if(!IS_S(t107)||S_LENGTH(t107)!=3) goto t111;
t107=S_DATA(top())[0];
X t113=lf[293]; // CONST
save(t113);
X t114=___id(t107); // REF: id
t113=restore();
X t112=(t113==t114)||eq1(t113,t114)?T:F; // EQ
if(t112==F) goto t111;
// skipped
t112=S_DATA(top())[2];
X t116=lf[294]; // CONST
save(t116);
X t117=___id(t112); // REF: id
t116=restore();
X t115=(t116==t117)||eq1(t116,t117)?T:F; // EQ
if(t115==F) goto t111;
t110=T;
t111:
restore();
if(t110==F) goto t106;
t110=S_DATA(top())[3];
X t118=F;
save(t110); // PCONS
if(!IS_S(t110)||S_LENGTH(t110)!=4) goto t119;
t110=S_DATA(top())[0];
X t121=lf[326]; // CONST
save(t121);
X t122=___id(t110); // REF: id
t121=restore();
X t120=(t121==t122)||eq1(t121,t122)?T:F; // EQ
if(t120==F) goto t119;
// skipped
// skipped
// skipped
t118=T;
t119:
restore();
if(t118==F) goto t106;
t105=T;
t106:
restore();
X t103;
x=restore();
if(t105!=F){
save(x);
X t123=allocate(2); // CONS
save(t123);
x=pick(2);
X t125=TO_N(4); // ICONST
save(t125);
X t127=TO_N(4); // ICONST
save(t127);
X t128=select1(TO_N(1),x); // SELECT (simple)
t127=restore();
X t126=select1(t127,t128); // SELECT
t125=restore();
X t124=select1(t125,t126); // SELECT
t123=top();
S_DATA(t123)[0]=t124;
t124=pick(2);
save(t124);
X t129=allocate(2); // CONS
save(t129);
t124=pick(2);
X t130=TO_N(5); // ICONST
t129=top();
S_DATA(t129)[0]=t130;
t130=pick(2);
save(t130);
X t131=allocate(2); // CONS
save(t131);
t130=pick(2);
X t132=lf[295]; // CONST
t131=top();
S_DATA(t131)[0]=t132;
t132=pick(2);
save(t132);
X t133=allocate(2); // CONS
save(t133);
t132=pick(2);
X t135=TO_N(3); // ICONST
save(t135);
X t137=TO_N(4); // ICONST
save(t137);
X t138=select1(TO_N(1),t132); // SELECT (simple)
t137=restore();
X t136=select1(t137,t138); // SELECT
t135=restore();
X t134=select1(t135,t136); // SELECT
t133=top();
S_DATA(t133)[0]=t134;
t134=pick(2);
save(t134);
X t139=allocate(2); // CONS
save(t139);
t134=pick(2);
X t140=TO_N(5); // ICONST
t139=top();
S_DATA(t139)[0]=t140;
t140=pick(2);
X t141=select1(TO_N(2),t140); // SELECT (simple)
t139=top();
S_DATA(t139)[1]=t141;
t139=restore();
restore();
X t142=___unparse_3aprec(t139); // REF: unparse:prec
t133=top();
S_DATA(t133)[1]=t142;
t133=restore();
restore();
X t143=___unparse_3aup(t133); // REF: unparse:up
t131=top();
S_DATA(t131)[1]=t143;
t131=restore();
restore();
X t144=___unparse_3apr(t131); // REF: unparse:pr
t129=top();
S_DATA(t129)[1]=t144;
t129=restore();
restore();
X t145=___unparse_3aprec(t129); // REF: unparse:prec
t123=top();
S_DATA(t123)[1]=t145;
t123=restore();
restore();
X t146=___unparse_3aup(t123); // REF: unparse:up
t103=t146;
}else{
save(x); // COND
X t148=select1(TO_N(1),x); // SELECT (simple)
X t149=F;
save(t148); // PCONSL
if(!IS_S(t148)||S_LENGTH(t148)<1) goto t150;
t148=S_DATA(top())[0];
X t152=lf[320]; // CONST
save(t152);
X t153=___id(t148); // REF: id
t152=restore();
X t151=(t152==t153)||eq1(t152,t153)?T:F; // EQ
if(t151==F) goto t150;
t149=T;
t150:
restore();
X t147;
x=restore();
if(t149!=F){
save(x);
X t154=allocate(3); // CONS
save(t154);
x=pick(2);
X t155=select1(TO_N(1),x); // SELECT (simple)
t154=top();
S_DATA(t154)[0]=t155;
t155=pick(2);
X t156=lf[384]; // CONST
t154=top();
S_DATA(t154)[1]=t156;
t156=pick(2);
X t157=select1(TO_N(2),t156); // SELECT (simple)
t154=top();
S_DATA(t154)[2]=t157;
t154=restore();
restore();
X t158=___unparse_3aup2(t154); // REF: unparse:up2
t147=t158;
}else{
save(x); // COND
X t160=select1(TO_N(1),x); // SELECT (simple)
X t161=F;
save(t160); // PCONSL
if(!IS_S(t160)||S_LENGTH(t160)<1) goto t162;
t160=S_DATA(top())[0];
X t164=lf[296]; // CONST
save(t164);
X t165=___id(t160); // REF: id
t164=restore();
X t163=(t164==t165)||eq1(t164,t165)?T:F; // EQ
if(t163==F) goto t162;
t161=T;
t162:
restore();
X t159;
x=restore();
if(t161!=F){
save(x);
X t166=allocate(2); // CONS
save(t166);
x=pick(2);
X t168=TO_N(3); // ICONST
save(t168);
X t169=select1(TO_N(1),x); // SELECT (simple)
t168=restore();
X t167=select1(t168,t169); // SELECT
t166=top();
S_DATA(t166)[0]=t167;
t167=pick(2);
save(t167);
X t170=allocate(2); // CONS
save(t170);
t167=pick(2);
X t171=lf[297]; // CONST
t170=top();
S_DATA(t170)[0]=t171;
t171=pick(2);
X t172=select1(TO_N(2),t171); // SELECT (simple)
t170=top();
S_DATA(t170)[1]=t172;
t170=restore();
restore();
X t173=___unparse_3apr(t170); // REF: unparse:pr
t166=top();
S_DATA(t166)[1]=t173;
t166=restore();
restore();
X t174=___unparse_3aup(t166); // REF: unparse:up
t159=t174;
}else{
save(x); // COND
X t176=select1(TO_N(1),x); // SELECT (simple)
X t177=F;
save(t176); // PCONSL
if(!IS_S(t176)||S_LENGTH(t176)<1) goto t178;
t176=S_DATA(top())[0];
X t180=lf[298]; // CONST
save(t180);
X t181=___id(t176); // REF: id
t180=restore();
X t179=(t180==t181)||eq1(t180,t181)?T:F; // EQ
if(t179==F) goto t178;
t177=T;
t178:
restore();
X t175;
x=restore();
if(t177!=F){
save(x);
X t182=allocate(2); // CONS
save(t182);
x=pick(2);
X t184=TO_N(3); // ICONST
save(t184);
X t185=select1(TO_N(1),x); // SELECT (simple)
t184=restore();
X t183=select1(t184,t185); // SELECT
t182=top();
S_DATA(t182)[0]=t183;
t183=pick(2);
save(t183);
X t186=allocate(2); // CONS
save(t186);
t183=pick(2);
X t187=lf[299]; // CONST
t186=top();
S_DATA(t186)[0]=t187;
t187=pick(2);
X t188=select1(TO_N(2),t187); // SELECT (simple)
t186=top();
S_DATA(t186)[1]=t188;
t186=restore();
restore();
X t189=___unparse_3apr(t186); // REF: unparse:pr
t182=top();
S_DATA(t182)[1]=t189;
t182=restore();
restore();
X t190=___unparse_3aup(t182); // REF: unparse:up
t175=t190;
}else{
save(x); // COND
X t192=select1(TO_N(1),x); // SELECT (simple)
X t193=F;
save(t192); // PCONSL
if(!IS_S(t192)||S_LENGTH(t192)<1) goto t194;
t192=S_DATA(top())[0];
X t196=lf[300]; // CONST
save(t196);
X t197=___id(t192); // REF: id
t196=restore();
X t195=(t196==t197)||eq1(t196,t197)?T:F; // EQ
if(t195==F) goto t194;
t193=T;
t194:
restore();
X t191;
x=restore();
if(t193!=F){
save(x);
X t198=allocate(2); // CONS
save(t198);
x=pick(2);
X t200=TO_N(3); // ICONST
save(t200);
X t201=select1(TO_N(1),x); // SELECT (simple)
t200=restore();
X t199=select1(t200,t201); // SELECT
t198=top();
S_DATA(t198)[0]=t199;
t199=pick(2);
save(t199);
X t202=allocate(2); // CONS
save(t202);
t199=pick(2);
X t203=lf[301]; // CONST
t202=top();
S_DATA(t202)[0]=t203;
t203=pick(2);
X t204=select1(TO_N(2),t203); // SELECT (simple)
t202=top();
S_DATA(t202)[1]=t204;
t202=restore();
restore();
X t205=___unparse_3apr(t202); // REF: unparse:pr
t198=top();
S_DATA(t198)[1]=t205;
t198=restore();
restore();
X t206=___unparse_3aup(t198); // REF: unparse:up
t191=t206;
}else{
save(x); // COND
X t208=select1(TO_N(1),x); // SELECT (simple)
X t209=F;
save(t208); // PCONSL
if(!IS_S(t208)||S_LENGTH(t208)<1) goto t210;
t208=S_DATA(top())[0];
X t212=lf[302]; // CONST
save(t212);
X t213=___id(t208); // REF: id
t212=restore();
X t211=(t212==t213)||eq1(t212,t213)?T:F; // EQ
if(t211==F) goto t210;
t209=T;
t210:
restore();
X t207;
x=restore();
if(t209!=F){
save(x);
X t214=allocate(2); // CONS
save(t214);
x=pick(2);
X t215=lf[303]; // CONST
t214=top();
S_DATA(t214)[0]=t215;
t215=pick(2);
save(t215);
X t216=allocate(2); // CONS
save(t216);
t215=pick(2);
X t218=TO_N(4); // ICONST
save(t218);
X t219=select1(TO_N(1),t215); // SELECT (simple)
t218=restore();
X t217=select1(t218,t219); // SELECT
t216=top();
S_DATA(t216)[0]=t217;
t217=pick(2);
save(t217);
X t220=allocate(2); // CONS
save(t220);
t217=pick(2);
X t221=lf[338]; // CONST
t220=top();
S_DATA(t220)[0]=t221;
t221=pick(2);
save(t221);
X t222=allocate(2); // CONS
save(t222);
t221=pick(2);
X t224=TO_N(3); // ICONST
save(t224);
X t225=select1(TO_N(1),t221); // SELECT (simple)
t224=restore();
X t223=select1(t224,t225); // SELECT
t222=top();
S_DATA(t222)[0]=t223;
t223=pick(2);
save(t223);
X t226=allocate(2); // CONS
save(t226);
t223=pick(2);
X t227=lf[304]; // CONST
t226=top();
S_DATA(t226)[0]=t227;
t227=pick(2);
X t228=select1(TO_N(2),t227); // SELECT (simple)
t226=top();
S_DATA(t226)[1]=t228;
t226=restore();
restore();
X t229=___unparse_3apr(t226); // REF: unparse:pr
t222=top();
S_DATA(t222)[1]=t229;
t222=restore();
restore();
X t230=___unparse_3aup(t222); // REF: unparse:up
t220=top();
S_DATA(t220)[1]=t230;
t220=restore();
restore();
X t231=___unparse_3apr(t220); // REF: unparse:pr
t216=top();
S_DATA(t216)[1]=t231;
t216=restore();
restore();
X t232=___unparse_3aup(t216); // REF: unparse:up
t214=top();
S_DATA(t214)[1]=t232;
t214=restore();
restore();
X t233=___unparse_3apr(t214); // REF: unparse:pr
t207=t233;
}else{
save(x); // COND
X t235=select1(TO_N(1),x); // SELECT (simple)
X t236=F;
save(t235); // PCONSL
if(!IS_S(t235)||S_LENGTH(t235)<1) goto t237;
t235=S_DATA(top())[0];
X t239=lf[305]; // CONST
save(t239);
X t240=___id(t235); // REF: id
t239=restore();
X t238=(t239==t240)||eq1(t239,t240)?T:F; // EQ
if(t238==F) goto t237;
t236=T;
t237:
restore();
X t234;
x=restore();
if(t236!=F){
save(x);
X t241=allocate(2); // CONS
save(t241);
x=pick(2);
X t242=TO_N(-2); // ICONST
t241=top();
S_DATA(t241)[0]=t242;
t242=pick(2);
save(t242);
X t243=allocate(2); // CONS
save(t243);
t242=pick(2);
X t245=TO_N(4); // ICONST
save(t245);
X t246=select1(TO_N(1),t242); // SELECT (simple)
t245=restore();
X t244=select1(t245,t246); // SELECT
t243=top();
S_DATA(t243)[0]=t244;
t244=pick(2);
save(t244);
X t247=allocate(2); // CONS
save(t247);
t244=pick(2);
X t248=TO_N(3); // ICONST
t247=top();
S_DATA(t247)[0]=t248;
t248=pick(2);
save(t248);
X t249=allocate(2); // CONS
save(t249);
t248=pick(2);
X t250=TO_N(2); // ICONST
t249=top();
S_DATA(t249)[0]=t250;
t250=pick(2);
save(t250);
X t251=allocate(2); // CONS
save(t251);
t250=pick(2);
X t253=TO_N(3); // ICONST
save(t253);
X t254=select1(TO_N(1),t250); // SELECT (simple)
t253=restore();
X t252=select1(t253,t254); // SELECT
t251=top();
S_DATA(t251)[0]=t252;
t252=pick(2);
save(t252);
X t255=allocate(2); // CONS
save(t255);
t252=pick(2);
X t256=lf[306]; // CONST
t255=top();
S_DATA(t255)[0]=t256;
t256=pick(2);
X t257=select1(TO_N(2),t256); // SELECT (simple)
t255=top();
S_DATA(t255)[1]=t257;
t255=restore();
restore();
X t258=___unparse_3apr(t255); // REF: unparse:pr
t251=top();
S_DATA(t251)[1]=t258;
t251=restore();
restore();
X t259=___unparse_3aup(t251); // REF: unparse:up
t249=top();
S_DATA(t249)[1]=t259;
t249=restore();
restore();
X t260=___unparse_3aindent(t249); // REF: unparse:indent
X t261=___unparse_3aprnl0(t260); // REF: unparse:prnl0
t247=top();
S_DATA(t247)[1]=t261;
t247=restore();
restore();
X t262=___unparse_3aprec(t247); // REF: unparse:prec
t243=top();
S_DATA(t243)[1]=t262;
t243=restore();
restore();
X t263=___unparse_3aup(t243); // REF: unparse:up
t241=top();
S_DATA(t241)[1]=t263;
t241=restore();
restore();
X t264=___unparse_3aindent(t241); // REF: unparse:indent
t234=t264;
}else{
save(x); // COND
X t266=select1(TO_N(1),x); // SELECT (simple)
X t267=F;
save(t266); // PCONSL
if(!IS_S(t266)||S_LENGTH(t266)<1) goto t268;
t266=S_DATA(top())[0];
X t270=lf[307]; // CONST
save(t270);
X t271=___id(t266); // REF: id
t270=restore();
X t269=(t270==t271)||eq1(t270,t271)?T:F; // EQ
if(t269==F) goto t268;
t267=T;
t268:
restore();
X t265;
x=restore();
if(t267!=F){
save(x);
X t272=allocate(2); // CONS
save(t272);
x=pick(2);
X t273=TO_N(-2); // ICONST
t272=top();
S_DATA(t272)[0]=t273;
t273=pick(2);
save(t273);
X t274=allocate(2); // CONS
save(t274);
t273=pick(2);
X t276=TO_N(4); // ICONST
save(t276);
X t277=select1(TO_N(1),t273); // SELECT (simple)
t276=restore();
X t275=select1(t276,t277); // SELECT
t274=top();
S_DATA(t274)[0]=t275;
t275=pick(2);
save(t275);
X t278=allocate(2); // CONS
save(t278);
t275=pick(2);
X t279=TO_N(3); // ICONST
t278=top();
S_DATA(t278)[0]=t279;
t279=pick(2);
save(t279);
X t280=allocate(2); // CONS
save(t280);
t279=pick(2);
X t281=TO_N(2); // ICONST
t280=top();
S_DATA(t280)[0]=t281;
t281=pick(2);
save(t281);
X t282=allocate(2); // CONS
save(t282);
t281=pick(2);
X t284=TO_N(3); // ICONST
save(t284);
X t285=select1(TO_N(1),t281); // SELECT (simple)
t284=restore();
X t283=select1(t284,t285); // SELECT
t282=top();
S_DATA(t282)[0]=t283;
t283=pick(2);
save(t283);
X t286=allocate(2); // CONS
save(t286);
t283=pick(2);
X t287=lf[308]; // CONST
t286=top();
S_DATA(t286)[0]=t287;
t287=pick(2);
X t288=select1(TO_N(2),t287); // SELECT (simple)
t286=top();
S_DATA(t286)[1]=t288;
t286=restore();
restore();
X t289=___unparse_3apr(t286); // REF: unparse:pr
t282=top();
S_DATA(t282)[1]=t289;
t282=restore();
restore();
X t290=___unparse_3aup(t282); // REF: unparse:up
X t291=___unparse_3aprnl0(t290); // REF: unparse:prnl0
t280=top();
S_DATA(t280)[1]=t291;
t280=restore();
restore();
X t292=___unparse_3aindent(t280); // REF: unparse:indent
t278=top();
S_DATA(t278)[1]=t292;
t278=restore();
restore();
X t293=___unparse_3aprec(t278); // REF: unparse:prec
t274=top();
S_DATA(t274)[1]=t293;
t274=restore();
restore();
X t294=___unparse_3aup(t274); // REF: unparse:up
t272=top();
S_DATA(t272)[1]=t294;
t272=restore();
restore();
X t295=___unparse_3aindent(t272); // REF: unparse:indent
t265=t295;
}else{
save(x); // COND
X t297=select1(TO_N(1),x); // SELECT (simple)
X t298=F;
save(t297); // PCONSL
if(!IS_S(t297)||S_LENGTH(t297)<1) goto t299;
t297=S_DATA(top())[0];
X t301=lf[309]; // CONST
save(t301);
X t302=___id(t297); // REF: id
t301=restore();
X t300=(t301==t302)||eq1(t301,t302)?T:F; // EQ
if(t300==F) goto t299;
t298=T;
t299:
restore();
X t296;
x=restore();
if(t298!=F){
save(x);
X t303=allocate(2); // CONS
save(t303);
x=pick(2);
X t305=TO_N(4); // ICONST
save(t305);
X t306=select1(TO_N(1),x); // SELECT (simple)
t305=restore();
X t304=select1(t305,t306); // SELECT
t303=top();
S_DATA(t303)[0]=t304;
t304=pick(2);
save(t304);
X t307=allocate(2); // CONS
save(t307);
t304=pick(2);
X t308=lf[310]; // CONST
t307=top();
S_DATA(t307)[0]=t308;
t308=pick(2);
save(t308);
X t309=allocate(2); // CONS
save(t309);
t308=pick(2);
X t311=TO_N(3); // ICONST
save(t311);
X t312=select1(TO_N(1),t308); // SELECT (simple)
t311=restore();
X t310=select1(t311,t312); // SELECT
t309=top();
S_DATA(t309)[0]=t310;
t310=pick(2);
X t313=select1(TO_N(2),t310); // SELECT (simple)
t309=top();
S_DATA(t309)[1]=t313;
t309=restore();
restore();
X t314=___unparse_3aup(t309); // REF: unparse:up
t307=top();
S_DATA(t307)[1]=t314;
t307=restore();
restore();
X t315=___unparse_3apr(t307); // REF: unparse:pr
t303=top();
S_DATA(t303)[1]=t315;
t303=restore();
restore();
X t316=___unparse_3aup(t303); // REF: unparse:up
t296=t316;
}else{
save(x); // COND
X t318=select1(TO_N(1),x); // SELECT (simple)
X t319=F;
save(t318); // PCONSL
if(!IS_S(t318)||S_LENGTH(t318)<1) goto t320;
t318=S_DATA(top())[0];
X t322=lf[311]; // CONST
save(t322);
X t323=___id(t318); // REF: id
t322=restore();
X t321=(t322==t323)||eq1(t322,t323)?T:F; // EQ
if(t321==F) goto t320;
t319=T;
t320:
restore();
X t317;
x=restore();
if(t319!=F){
save(x);
X t324=allocate(2); // CONS
save(t324);
x=pick(2);
X t326=TO_N(3); // ICONST
save(t326);
X t327=select1(TO_N(1),x); // SELECT (simple)
t326=restore();
X t325=select1(t326,t327); // SELECT
t324=top();
S_DATA(t324)[0]=t325;
t325=pick(2);
save(t325);
X t328=allocate(2); // CONS
save(t328);
t325=pick(2);
X t329=lf[312]; // CONST
t328=top();
S_DATA(t328)[0]=t329;
t329=pick(2);
X t330=select1(TO_N(2),t329); // SELECT (simple)
t328=top();
S_DATA(t328)[1]=t330;
t328=restore();
restore();
X t331=___unparse_3apr(t328); // REF: unparse:pr
t324=top();
S_DATA(t324)[1]=t331;
t324=restore();
restore();
X t332=___unparse_3aup_5fconst(t324); // REF: unparse:up_const
t317=t332;
}else{
save(x); // COND
X t334=select1(TO_N(1),x); // SELECT (simple)
X t335=F;
save(t334); // PCONSL
if(!IS_S(t334)||S_LENGTH(t334)<1) goto t336;
t334=S_DATA(top())[0];
X t338=lf[313]; // CONST
save(t338);
X t339=___id(t334); // REF: id
t338=restore();
X t337=(t338==t339)||eq1(t338,t339)?T:F; // EQ
if(t337==F) goto t336;
t335=T;
t336:
restore();
X t333;
x=restore();
if(t335!=F){
save(x);
X t340=allocate(2); // CONS
save(t340);
x=pick(2);
X t342=TO_N(3); // ICONST
save(t342);
X t343=select1(TO_N(1),x); // SELECT (simple)
t342=restore();
X t341=select1(t342,t343); // SELECT
t340=top();
S_DATA(t340)[0]=t341;
t341=pick(2);
save(t341);
X t344=allocate(2); // CONS
save(t344);
t341=pick(2);
X t345=lf[314]; // CONST
t344=top();
S_DATA(t344)[0]=t345;
t345=pick(2);
X t346=select1(TO_N(2),t345); // SELECT (simple)
t344=top();
S_DATA(t344)[1]=t346;
t344=restore();
restore();
X t347=___unparse_3apr(t344); // REF: unparse:pr
t340=top();
S_DATA(t340)[1]=t347;
t340=restore();
restore();
X t348=___unparse_3aup(t340); // REF: unparse:up
t333=t348;
}else{
save(x); // COND
X t350=select1(TO_N(1),x); // SELECT (simple)
X t351=F;
save(t350); // PCONSL
if(!IS_S(t350)||S_LENGTH(t350)<1) goto t352;
t350=S_DATA(top())[0];
X t354=lf[315]; // CONST
save(t354);
X t355=___id(t350); // REF: id
t354=restore();
X t353=(t354==t355)||eq1(t354,t355)?T:F; // EQ
if(t353==F) goto t352;
t351=T;
t352:
restore();
X t349;
x=restore();
if(t351!=F){
save(x);
X t356=allocate(3); // CONS
save(t356);
x=pick(2);
X t357=select1(TO_N(1),x); // SELECT (simple)
t356=top();
S_DATA(t356)[0]=t357;
t357=pick(2);
X t358=lf[316]; // CONST
t356=top();
S_DATA(t356)[1]=t358;
t358=pick(2);
X t359=select1(TO_N(2),t358); // SELECT (simple)
t356=top();
S_DATA(t356)[2]=t359;
t356=restore();
restore();
X t360=___unparse_3aup2(t356); // REF: unparse:up2
t349=t360;
}else{
save(x); // COND
X t362=select1(TO_N(1),x); // SELECT (simple)
X t363=F;
save(t362); // PCONSL
if(!IS_S(t362)||S_LENGTH(t362)<1) goto t364;
t362=S_DATA(top())[0];
X t366=lf[317]; // CONST
save(t366);
X t367=___id(t362); // REF: id
t366=restore();
X t365=(t366==t367)||eq1(t366,t367)?T:F; // EQ
if(t365==F) goto t364;
t363=T;
t364:
restore();
X t361;
x=restore();
if(t363!=F){
save(x);
X t368=allocate(2); // CONS
save(t368);
x=pick(2);
save(x);
X t369=allocate(4); // CONS
save(t369);
x=pick(2);
X t370=lf[320]; // CONST
t369=top();
S_DATA(t369)[0]=t370;
t370=pick(2);
X t371=T; // ICONST
t369=top();
S_DATA(t369)[1]=t371;
t371=pick(2);
X t372=lf[318]; // CONST
t369=top();
S_DATA(t369)[2]=t372;
t372=pick(2);
save(t372);
X t373=allocate(2); // CONS
save(t373);
t372=pick(2);
X t374=lf[322]; // CONST
t373=top();
S_DATA(t373)[0]=t374;
t374=pick(2);
X t375=select1(TO_N(1),t374); // SELECT (simple)
X t376=___tl(t375); // REF: tl
X t377=___tl(t376); // REF: tl
t373=top();
S_DATA(t373)[1]=t377;
t373=restore();
restore();
X t378=___cat(t373); // REF: cat
t369=top();
S_DATA(t369)[3]=t378;
t369=restore();
restore();
t368=top();
S_DATA(t368)[0]=t369;
t369=pick(2);
X t379=select1(TO_N(2),t369); // SELECT (simple)
t368=top();
S_DATA(t368)[1]=t379;
t368=restore();
restore();
X t380=___unparse_3aup(t368); // REF: unparse:up
t361=t380;
}else{
save(x); // COND
X t382=select1(TO_N(1),x); // SELECT (simple)
X t383=F;
save(t382); // PCONSL
if(!IS_S(t382)||S_LENGTH(t382)<1) goto t384;
t382=S_DATA(top())[0];
X t386=lf[319]; // CONST
save(t386);
X t387=___id(t382); // REF: id
t386=restore();
X t385=(t386==t387)||eq1(t386,t387)?T:F; // EQ
if(t385==F) goto t384;
t383=T;
t384:
restore();
X t381;
x=restore();
if(t383!=F){
save(x);
X t388=allocate(2); // CONS
save(t388);
x=pick(2);
save(x);
X t389=allocate(4); // CONS
save(t389);
x=pick(2);
X t390=lf[320]; // CONST
t389=top();
S_DATA(t389)[0]=t390;
t390=pick(2);
X t391=T; // ICONST
t389=top();
S_DATA(t389)[1]=t391;
t391=pick(2);
X t392=lf[321]; // CONST
t389=top();
S_DATA(t389)[2]=t392;
t392=pick(2);
save(t392);
X t393=allocate(2); // CONS
save(t393);
t392=pick(2);
X t394=lf[322]; // CONST
t393=top();
S_DATA(t393)[0]=t394;
t394=pick(2);
X t395=select1(TO_N(1),t394); // SELECT (simple)
X t396=___tl(t395); // REF: tl
X t397=___tl(t396); // REF: tl
t393=top();
S_DATA(t393)[1]=t397;
t393=restore();
restore();
X t398=___cat(t393); // REF: cat
t389=top();
S_DATA(t389)[3]=t398;
t389=restore();
restore();
t388=top();
S_DATA(t388)[0]=t389;
t389=pick(2);
X t399=select1(TO_N(2),t389); // SELECT (simple)
t388=top();
S_DATA(t388)[1]=t399;
t388=restore();
restore();
X t400=___unparse_3aup(t388); // REF: unparse:up
t381=t400;
}else{
save(x); // COND
X t402=select1(TO_N(1),x); // SELECT (simple)
X t403=F;
save(t402); // PCONSL
if(!IS_S(t402)||S_LENGTH(t402)<1) goto t404;
t402=S_DATA(top())[0];
X t406=lf[323]; // CONST
save(t406);
X t407=___id(t402); // REF: id
t406=restore();
X t405=(t406==t407)||eq1(t406,t407)?T:F; // EQ
if(t405==F) goto t404;
t403=T;
t404:
restore();
X t401;
x=restore();
if(t403!=F){
save(x);
X t408=allocate(2); // CONS
save(t408);
x=pick(2);
X t410=TO_N(3); // ICONST
save(t410);
X t411=select1(TO_N(1),x); // SELECT (simple)
t410=restore();
X t409=select1(t410,t411); // SELECT
t408=top();
S_DATA(t408)[0]=t409;
t409=pick(2);
save(t409);
X t412=allocate(2); // CONS
save(t412);
t409=pick(2);
X t413=lf[325]; // CONST
t412=top();
S_DATA(t412)[0]=t413;
t413=pick(2);
X t414=select1(TO_N(2),t413); // SELECT (simple)
t412=top();
S_DATA(t412)[1]=t414;
t412=restore();
restore();
X t415=___unparse_3apr(t412); // REF: unparse:pr
t408=top();
S_DATA(t408)[1]=t415;
t408=restore();
restore();
X t416=___unparse_3aup(t408); // REF: unparse:up
t401=t416;
}else{
save(x); // COND
X t418=select1(TO_N(1),x); // SELECT (simple)
X t419=F;
save(t418); // PCONSL
if(!IS_S(t418)||S_LENGTH(t418)<1) goto t420;
t418=S_DATA(top())[0];
X t422=lf[324]; // CONST
save(t422);
X t423=___id(t418); // REF: id
t422=restore();
X t421=(t422==t423)||eq1(t422,t423)?T:F; // EQ
if(t421==F) goto t420;
t419=T;
t420:
restore();
X t417;
x=restore();
if(t419!=F){
save(x);
X t424=allocate(2); // CONS
save(t424);
x=pick(2);
save(x);
X t425=allocate(2); // CONS
save(t425);
x=pick(2);
X t426=lf[325]; // CONST
t425=top();
S_DATA(t425)[0]=t426;
t426=pick(2);
X t428=TO_N(3); // ICONST
save(t428);
X t429=select1(TO_N(1),t426); // SELECT (simple)
t428=restore();
X t427=select1(t428,t429); // SELECT
X t430=___tos(t427); // REF: tos
t425=top();
S_DATA(t425)[1]=t430;
t425=restore();
restore();
X t431=___cat(t425); // REF: cat
t424=top();
S_DATA(t424)[0]=t431;
t431=pick(2);
X t432=select1(TO_N(2),t431); // SELECT (simple)
t424=top();
S_DATA(t424)[1]=t432;
t424=restore();
restore();
X t433=___unparse_3apr(t424); // REF: unparse:pr
t417=t433;
}else{
save(x); // COND
X t435=select1(TO_N(1),x); // SELECT (simple)
X t436=F;
save(t435); // PCONSL
if(!IS_S(t435)||S_LENGTH(t435)<1) goto t437;
t435=S_DATA(top())[0];
X t439=lf[326]; // CONST
save(t439);
X t440=___id(t435); // REF: id
t439=restore();
X t438=(t439==t440)||eq1(t439,t440)?T:F; // EQ
if(t438==F) goto t437;
t436=T;
t437:
restore();
X t434;
x=restore();
if(t436!=F){
save(x);
X t441=allocate(2); // CONS
save(t441);
x=pick(2);
X t442=lf[327]; // CONST
t441=top();
S_DATA(t441)[0]=t442;
t442=pick(2);
save(t442);
X t443=allocate(2); // CONS
save(t443);
t442=pick(2);
X t444=select1(TO_N(1),t442); // SELECT (simple)
X t445=___tl(t444); // REF: tl
X t446=___tl(t445); // REF: tl
t443=top();
S_DATA(t443)[0]=t446;
t446=pick(2);
save(t446);
X t447=allocate(2); // CONS
save(t447);
t446=pick(2);
X t448=lf[328]; // CONST
t447=top();
S_DATA(t447)[0]=t448;
t448=pick(2);
X t449=select1(TO_N(2),t448); // SELECT (simple)
t447=top();
S_DATA(t447)[1]=t449;
t447=restore();
restore();
X t450=___unparse_3apr(t447); // REF: unparse:pr
t443=top();
S_DATA(t443)[1]=t450;
t443=restore();
restore();
X t451=___unparse_3aup_5fseq(t443); // REF: unparse:up_seq
t441=top();
S_DATA(t441)[1]=t451;
t441=restore();
restore();
X t452=___unparse_3apr(t441); // REF: unparse:pr
t434=t452;
}else{
save(x); // COND
X t454=select1(TO_N(1),x); // SELECT (simple)
X t455=F;
save(t454); // PCONSL
if(!IS_S(t454)||S_LENGTH(t454)<1) goto t456;
t454=S_DATA(top())[0];
X t458=lf[329]; // CONST
save(t458);
X t459=___id(t454); // REF: id
t458=restore();
X t457=(t458==t459)||eq1(t458,t459)?T:F; // EQ
if(t457==F) goto t456;
t455=T;
t456:
restore();
X t453;
x=restore();
if(t455!=F){
save(x);
X t460=allocate(2); // CONS
save(t460);
x=pick(2);
X t461=lf[330]; // CONST
t460=top();
S_DATA(t460)[0]=t461;
t461=pick(2);
save(t461);
X t462=allocate(2); // CONS
save(t462);
t461=pick(2);
X t463=select1(TO_N(1),t461); // SELECT (simple)
X t464=___tl(t463); // REF: tl
X t465=___tl(t464); // REF: tl
t462=top();
S_DATA(t462)[0]=t465;
t465=pick(2);
save(t465);
X t466=allocate(2); // CONS
save(t466);
t465=pick(2);
X t467=lf[333]; // CONST
t466=top();
S_DATA(t466)[0]=t467;
t467=pick(2);
X t468=select1(TO_N(2),t467); // SELECT (simple)
t466=top();
S_DATA(t466)[1]=t468;
t466=restore();
restore();
X t469=___unparse_3apr(t466); // REF: unparse:pr
t462=top();
S_DATA(t462)[1]=t469;
t462=restore();
restore();
X t470=___unparse_3aup_5fseq(t462); // REF: unparse:up_seq
t460=top();
S_DATA(t460)[1]=t470;
t460=restore();
restore();
X t471=___unparse_3apr(t460); // REF: unparse:pr
t453=t471;
}else{
save(x); // COND
X t473=select1(TO_N(1),x); // SELECT (simple)
X t474=F;
save(t473); // PCONSL
if(!IS_S(t473)||S_LENGTH(t473)<1) goto t475;
t473=S_DATA(top())[0];
X t477=lf[331]; // CONST
save(t477);
X t478=___id(t473); // REF: id
t477=restore();
X t476=(t477==t478)||eq1(t477,t478)?T:F; // EQ
if(t476==F) goto t475;
t474=T;
t475:
restore();
X t472;
x=restore();
if(t474!=F){
save(x);
X t479=allocate(2); // CONS
save(t479);
x=pick(2);
X t480=lf[332]; // CONST
t479=top();
S_DATA(t479)[0]=t480;
t480=pick(2);
save(t480);
X t481=allocate(2); // CONS
save(t481);
t480=pick(2);
X t482=select1(TO_N(1),t480); // SELECT (simple)
X t483=___tl(t482); // REF: tl
X t484=___tl(t483); // REF: tl
t481=top();
S_DATA(t481)[0]=t484;
t484=pick(2);
save(t484);
X t485=allocate(2); // CONS
save(t485);
t484=pick(2);
X t486=lf[333]; // CONST
t485=top();
S_DATA(t485)[0]=t486;
t486=pick(2);
X t487=select1(TO_N(2),t486); // SELECT (simple)
t485=top();
S_DATA(t485)[1]=t487;
t485=restore();
restore();
X t488=___unparse_3apr(t485); // REF: unparse:pr
t481=top();
S_DATA(t481)[1]=t488;
t481=restore();
restore();
X t489=___unparse_3aup_5fseq(t481); // REF: unparse:up_seq
t479=top();
S_DATA(t479)[1]=t489;
t479=restore();
restore();
X t490=___unparse_3apr(t479); // REF: unparse:pr
t472=t490;
}else{
X t491=____5f(x); // REF: _
t472=t491;}
t453=t472;}
t434=t453;}
t417=t434;}
t401=t417;}
t381=t401;}
t361=t381;}
t349=t361;}
t333=t349;}
t317=t333;}
t296=t317;}
t265=t296;}
t234=t265;}
t207=t234;}
t191=t207;}
t175=t191;}
t159=t175;}
t147=t159;}
t103=t147;}
t83=t103;}
t48=t83;}
t15=t48;}
t1=t15;}
RETURN(t1);}
//---------------------------------------- unparse:up1' (unparse.fp:62)
DEFINE(___unparse_3aup1_27){
ENTRY;
loop:;
tracecall("unparse.fp:62:  unparse:up1'");
save(x);
X t1=allocate(3); // CONS
save(t1);
x=pick(2);
X t2=select1(TO_N(1),x); // SELECT (simple)
t1=top();
S_DATA(t1)[0]=t2;
t2=pick(2);
X t3=select1(TO_N(1),t2); // SELECT (simple)
X t4=___unparse_3anode_5fprec(t3); // REF: unparse:node_prec
t1=top();
S_DATA(t1)[1]=t4;
t4=pick(2);
X t5=select1(TO_N(2),t4); // SELECT (simple)
t1=top();
S_DATA(t1)[2]=t5;
t1=restore();
restore();
save(t1);
X t6=allocate(2); // CONS
save(t6);
t1=pick(2);
X t7=___id(t1); // REF: id
t6=top();
S_DATA(t6)[0]=t7;
t7=pick(2);
X ___tmp_251_25314(X);
X t8=fpointer((void*)___tmp_251_25314); // POINTER2: tmp%1%314
t6=top();
S_DATA(t6)[1]=t8;
t6=restore();
restore();
X t9=___unparse_3awith_5fprec(t6); // REF: unparse:with_prec
RETURN(t9);}
//---------------------------------------- unparse:up2 (unparse.fp:99)
DEFINE(___unparse_3aup2){
ENTRY;
loop:;
tracecall("unparse.fp:99:  unparse:up2");
save(x);
X t1=allocate(2); // CONS
save(t1);
x=pick(2);
X t3=TO_N(4); // ICONST
save(t3);
X t4=select1(TO_N(1),x); // SELECT (simple)
t3=restore();
X t2=select1(t3,t4); // SELECT
t1=top();
S_DATA(t1)[0]=t2;
t2=pick(2);
save(t2);
X t5=allocate(2); // CONS
save(t5);
t2=pick(2);
X t6=select1(TO_N(2),t2); // SELECT (simple)
t5=top();
S_DATA(t5)[0]=t6;
t6=pick(2);
save(t6);
X t7=allocate(2); // CONS
save(t7);
t6=pick(2);
X t9=TO_N(3); // ICONST
save(t9);
X t10=select1(TO_N(1),t6); // SELECT (simple)
t9=restore();
X t8=select1(t9,t10); // SELECT
t7=top();
S_DATA(t7)[0]=t8;
t8=pick(2);
X t11=select1(TO_N(3),t8); // SELECT (simple)
t7=top();
S_DATA(t7)[1]=t11;
t7=restore();
restore();
X t12=___unparse_3aup(t7); // REF: unparse:up
t5=top();
S_DATA(t5)[1]=t12;
t5=restore();
restore();
X t13=___unparse_3apr(t5); // REF: unparse:pr
t1=top();
S_DATA(t1)[1]=t13;
t1=restore();
restore();
X t14=___unparse_3aup(t1); // REF: unparse:up
RETURN(t14);}
//---------------------------------------- unparse:up_const (unparse.fp:103)
DEFINE(___unparse_3aup_5fconst){
ENTRY;
loop:;
tracecall("unparse.fp:103:  unparse:up_const");
save(x); // COND
X t2=select1(TO_N(1),x); // SELECT (simple)
X t3=___atom(t2); // REF: atom
X t1;
x=restore();
if(t3!=F){
save(x);
X t4=allocate(2); // CONS
save(t4);
x=pick(2);
save(x); // COND
X t6=select1(TO_N(1),x); // SELECT (simple)
X t7=___num(t6); // REF: num
X t5;
x=restore();
if(t7!=F){
X t8=select1(TO_N(1),x); // SELECT (simple)
X t9=___tos(t8); // REF: tos
t5=t9;
}else{
save(x);
X t10=allocate(3); // CONS
save(t10);
x=pick(2);
X t11=lf[334]; // CONST
t10=top();
S_DATA(t10)[0]=t11;
t11=pick(2);
X t12=select1(TO_N(1),t11); // SELECT (simple)
X t13=___tos(t12); // REF: tos
t10=top();
S_DATA(t10)[1]=t13;
t13=pick(2);
X t14=lf[334]; // CONST
t10=top();
S_DATA(t10)[2]=t14;
t10=restore();
restore();
X t15=___cat(t10); // REF: cat
t5=t15;}
t4=top();
S_DATA(t4)[0]=t5;
t5=pick(2);
X t16=select1(TO_N(2),t5); // SELECT (simple)
t4=top();
S_DATA(t4)[1]=t16;
t4=restore();
restore();
t1=t4;
}else{
save(x); // COND
X t19=TO_N(0); // ICONST
save(t19);
X t20=select1(TO_N(1),x); // SELECT (simple)
X t21=___len(t20); // REF: len
t19=restore();
X t18=(t19==t21)||eq1(t19,t21)?T:F; // EQ
X t17;
x=restore();
if(t18!=F){
save(x);
X t22=allocate(2); // CONS
save(t22);
x=pick(2);
X t23=lf[335]; // CONST
t22=top();
S_DATA(t22)[0]=t23;
t23=pick(2);
X t24=select1(TO_N(2),t23); // SELECT (simple)
t22=top();
S_DATA(t22)[1]=t24;
t22=restore();
restore();
t17=t22;
}else{
save(x);
X t25=allocate(2); // CONS
save(t25);
x=pick(2);
X t26=lf[336]; // CONST
t25=top();
S_DATA(t25)[0]=t26;
t26=pick(2);
X t28=TO_N(3); // ICONST
save(t28);
save(t26);
X t29=allocate(3); // CONS
save(t29);
t26=pick(2);
X t30=TO_N(2); // ICONST
t29=top();
S_DATA(t29)[0]=t30;
t30=pick(2);
X t31=select1(TO_N(1),t30); // SELECT (simple)
t29=top();
S_DATA(t29)[1]=t31;
t31=pick(2);
save(t31);
X t32=allocate(2); // CONS
save(t32);
t31=pick(2);
X t34=TO_N(1); // ICONST
save(t34);
X t35=select1(TO_N(1),t31); // SELECT (simple)
t34=restore();
X t33=select1(t34,t35); // SELECT
t32=top();
S_DATA(t32)[0]=t33;
t33=pick(2);
save(t33);
X t36=allocate(2); // CONS
save(t36);
t33=pick(2);
X t37=lf[337]; // CONST
t36=top();
S_DATA(t36)[0]=t37;
t37=pick(2);
X t38=select1(TO_N(2),t37); // SELECT (simple)
t36=top();
S_DATA(t36)[1]=t38;
t36=restore();
restore();
X t39=___unparse_3apr(t36); // REF: unparse:pr
t32=top();
S_DATA(t32)[1]=t39;
t32=restore();
restore();
X t40=___unparse_3aup_5fconst(t32); // REF: unparse:up_const
t29=top();
S_DATA(t29)[2]=t40;
t29=restore();
restore();
X t41=t29; // WHILE
for(;;){
save(t41);
save(t41);
X t42=allocate(2); // CONS
save(t42);
t41=pick(2);
X t43=select1(TO_N(1),t41); // SELECT (simple)
t42=top();
S_DATA(t42)[0]=t43;
t43=pick(2);
X t44=select1(TO_N(2),t43); // SELECT (simple)
X t45=___len(t44); // REF: len
t42=top();
S_DATA(t42)[1]=t45;
t42=restore();
restore();
X t46=___le(t42); // REF: le
t41=restore();
if(t46==F) break;
save(t41);
X t47=allocate(3); // CONS
save(t47);
t41=pick(2);
X t48=select1(TO_N(1),t41); // SELECT (simple)
save(t48);
X t49=allocate(2); // CONS
save(t49);
t48=pick(2);
X t50=___id(t48); // REF: id
t49=top();
S_DATA(t49)[0]=t50;
t50=pick(2);
X t51=TO_N(1); // ICONST
t49=top();
S_DATA(t49)[1]=t51;
t49=restore();
restore();
X t52=___add(t49); // REF: add
t47=top();
S_DATA(t47)[0]=t52;
t52=pick(2);
X t53=select1(TO_N(2),t52); // SELECT (simple)
t47=top();
S_DATA(t47)[1]=t53;
t53=pick(2);
save(t53);
X t54=allocate(2); // CONS
save(t54);
t53=pick(2);
save(t53);
X t56=select1(TO_N(1),t53); // SELECT (simple)
t53=restore();
save(t56);
X t57=select1(TO_N(2),t53); // SELECT (simple)
t56=restore();
X t55=select1(t56,t57); // SELECT
t54=top();
S_DATA(t54)[0]=t55;
t55=pick(2);
save(t55);
X t58=allocate(2); // CONS
save(t58);
t55=pick(2);
X t59=lf[338]; // CONST
t58=top();
S_DATA(t58)[0]=t59;
t59=pick(2);
X t60=select1(TO_N(3),t59); // SELECT (simple)
t58=top();
S_DATA(t58)[1]=t60;
t58=restore();
restore();
X t61=___unparse_3apr(t58); // REF: unparse:pr
t54=top();
S_DATA(t54)[1]=t61;
t54=restore();
restore();
X t62=___unparse_3aup_5fconst(t54); // REF: unparse:up_const
t47=top();
S_DATA(t47)[2]=t62;
t47=restore();
restore();
t41=t47;}
t28=restore();
X t27=select1(t28,t41); // SELECT
t25=top();
S_DATA(t25)[1]=t27;
t25=restore();
restore();
t17=t25;}
t1=t17;}
X t63=___unparse_3apr(t1); // REF: unparse:pr
RETURN(t63);}
//---------------------------------------- tmp%1%320 (unparse.fp:112)
DEFINE(___tmp_251_25320){
ENTRY;
loop:;
tracecall("unparse.fp:112:  tmp%1%320");
X t1=___unparse_3aup(x); // REF: unparse:up
RETURN(t1);}
//---------------------------------------- unparse:up_seq (unparse.fp:111)
DEFINE(___unparse_3aup_5fseq){
ENTRY;
loop:;
tracecall("unparse.fp:111:  unparse:up_seq");
save(x); // COND
X t3=lf[381]; // CONST
save(t3);
X t4=select1(TO_N(1),x); // SELECT (simple)
t3=restore();
X t2=(t3==t4)||eq1(t3,t4)?T:F; // EQ
X t1;
x=restore();
if(t2!=F){
X t5=select1(TO_N(2),x); // SELECT (simple)
t1=t5;
}else{
X t7=TO_N(2); // ICONST
save(t7);
save(x);
X t8=allocate(2); // CONS
save(t8);
x=pick(2);
X t9=select1(TO_N(1),x); // SELECT (simple)
X t10=___tl(t9); // REF: tl
t8=top();
S_DATA(t8)[0]=t10;
t10=pick(2);
save(t10);
X t11=allocate(3); // CONS
save(t11);
t10=pick(2);
X t13=TO_N(1); // ICONST
save(t13);
X t14=select1(TO_N(1),t10); // SELECT (simple)
t13=restore();
X t12=select1(t13,t14); // SELECT
t11=top();
S_DATA(t11)[0]=t12;
t12=pick(2);
X t15=TO_N(1); // ICONST
t11=top();
S_DATA(t11)[1]=t15;
t15=pick(2);
X t16=select1(TO_N(2),t15); // SELECT (simple)
t11=top();
S_DATA(t11)[2]=t16;
t11=restore();
restore();
save(t11);
X t17=allocate(2); // CONS
save(t17);
t11=pick(2);
X t18=___id(t11); // REF: id
t17=top();
S_DATA(t17)[0]=t18;
t18=pick(2);
X ___tmp_251_25320(X);
X t19=fpointer((void*)___tmp_251_25320); // POINTER2: tmp%1%320
t17=top();
S_DATA(t17)[1]=t19;
t17=restore();
restore();
X t20=___unparse_3awith_5fprec(t17); // REF: unparse:with_prec
t8=top();
S_DATA(t8)[1]=t20;
t8=restore();
restore();
X t21=t8; // WHILE
for(;;){
save(t21);
X t22=select1(TO_N(1),t21); // SELECT (simple)
save(t22);
X t23=allocate(2); // CONS
save(t23);
t22=pick(2);
X t24=lf[381]; // CONST
t23=top();
S_DATA(t23)[0]=t24;
t24=pick(2);
X t25=___id(t24); // REF: id
t23=top();
S_DATA(t23)[1]=t25;
t23=restore();
restore();
save(t23); // COND
X t27=___eq(t23); // REF: eq
X t26;
t23=restore();
if(t27!=F){
X t28=F; // ICONST
t26=t28;
}else{
X t29=T; // ICONST
t26=t29;}
t21=restore();
if(t26==F) break;
save(t21);
X t30=allocate(2); // CONS
save(t30);
t21=pick(2);
X t31=select1(TO_N(1),t21); // SELECT (simple)
X t32=___tl(t31); // REF: tl
t30=top();
S_DATA(t30)[0]=t32;
t32=pick(2);
save(t32);
X t33=allocate(3); // CONS
save(t33);
t32=pick(2);
X t35=TO_N(1); // ICONST
save(t35);
X t36=select1(TO_N(1),t32); // SELECT (simple)
t35=restore();
X t34=select1(t35,t36); // SELECT
t33=top();
S_DATA(t33)[0]=t34;
t34=pick(2);
X t37=TO_N(1); // ICONST
t33=top();
S_DATA(t33)[1]=t37;
t37=pick(2);
save(t37);
X t38=allocate(2); // CONS
save(t38);
t37=pick(2);
X t39=lf[338]; // CONST
t38=top();
S_DATA(t38)[0]=t39;
t39=pick(2);
X t40=select1(TO_N(2),t39); // SELECT (simple)
t38=top();
S_DATA(t38)[1]=t40;
t38=restore();
restore();
X t41=___unparse_3apr(t38); // REF: unparse:pr
t33=top();
S_DATA(t33)[2]=t41;
t33=restore();
restore();
save(t33);
X t42=allocate(2); // CONS
save(t42);
t33=pick(2);
X t43=___id(t33); // REF: id
t42=top();
S_DATA(t42)[0]=t43;
t43=pick(2);
X ___tmp_251_25322(X);
X t44=fpointer((void*)___tmp_251_25322); // POINTER2: tmp%1%322
t42=top();
S_DATA(t42)[1]=t44;
t42=restore();
restore();
X t45=___unparse_3awith_5fprec(t42); // REF: unparse:with_prec
t30=top();
S_DATA(t30)[1]=t45;
t30=restore();
restore();
t21=t30;}
t7=restore();
X t6=select1(t7,t21); // SELECT
t1=t6;}
RETURN(t1);}
//---------------------------------------- tmp%1%322 (unparse.fp:114)
DEFINE(___tmp_251_25322){
ENTRY;
loop:;
tracecall("unparse.fp:114:  tmp%1%322");
X t1=___unparse_3aup(x); // REF: unparse:up
RETURN(t1);}
//---------------------------------------- unparse:node_prec (unparse.fp:122)
DEFINE(___unparse_3anode_5fprec){
ENTRY;
loop:;
tracecall("unparse.fp:122:  unparse:node_prec");
save(x);
X t1=allocate(2); // CONS
save(t1);
x=pick(2);
X t2=select1(TO_N(1),x); // SELECT (simple)
t1=top();
S_DATA(t1)[0]=t2;
t2=pick(2);
X t3=lf[339]; // CONST
t1=top();
S_DATA(t1)[1]=t3;
t1=restore();
restore();
save(t1); // OR
X t5=___fetch(t1); // REF: fetch
X t4=t5;
t1=restore();
if(t4==F){
X t6=TO_N(7); // ICONST
t4=t6;}
RETURN(t4);}
//---------------------------------------- unparse:with_prec (unparse.fp:137)
DEFINE(___unparse_3awith_5fprec){
ENTRY;
loop:;
tracecall("unparse.fp:137:  unparse:with_prec");
save(x);
X t1=allocate(5); // CONS
save(t1);
x=pick(2);
X t3=TO_N(1); // ICONST
save(t3);
X t5=TO_N(3); // ICONST
save(t5);
X t6=select1(TO_N(1),x); // SELECT (simple)
t5=restore();
X t4=select1(t5,t6); // SELECT
t3=restore();
X t2=select1(t3,t4); // SELECT
t1=top();
S_DATA(t1)[0]=t2;
t2=pick(2);
X t8=TO_N(2); // ICONST
save(t8);
X t9=select1(TO_N(1),t2); // SELECT (simple)
t8=restore();
X t7=select1(t8,t9); // SELECT
t1=top();
S_DATA(t1)[1]=t7;
t7=pick(2);
X t11=TO_N(1); // ICONST
save(t11);
X t12=select1(TO_N(1),t7); // SELECT (simple)
t11=restore();
X t10=select1(t11,t12); // SELECT
t1=top();
S_DATA(t1)[2]=t10;
t10=pick(2);
X t14=TO_N(3); // ICONST
save(t14);
X t15=select1(TO_N(1),t10); // SELECT (simple)
t14=restore();
X t13=select1(t14,t15); // SELECT
t1=top();
S_DATA(t1)[3]=t13;
t13=pick(2);
X t16=select1(TO_N(2),t13); // SELECT (simple)
t1=top();
S_DATA(t1)[4]=t16;
t1=restore();
restore();
save(t1);
X t17=allocate(2); // CONS
save(t17);
t1=pick(2);
X t18=select1(TO_N(1),t1); // SELECT (simple)
t17=top();
S_DATA(t17)[0]=t18;
t18=pick(2);
save(t18);
X t19=allocate(2); // CONS
save(t19);
t18=pick(2);
X t20=select1(TO_N(5),t18); // SELECT (simple)
t19=top();
S_DATA(t19)[0]=t20;
t20=pick(2);
save(t20);
X t21=allocate(2); // CONS
save(t21);
t20=pick(2);
X t22=select1(TO_N(3),t20); // SELECT (simple)
t21=top();
S_DATA(t21)[0]=t22;
t22=pick(2);
save(t22);
X t23=allocate(2); // CONS
save(t23);
t22=pick(2);
X t24=select1(TO_N(2),t22); // SELECT (simple)
t23=top();
S_DATA(t23)[0]=t24;
t24=pick(2);
X t25=select1(TO_N(4),t24); // SELECT (simple)
t23=top();
S_DATA(t23)[1]=t25;
t23=restore();
restore();
X t26=___unparse_3aprec(t23); // REF: unparse:prec
t21=top();
S_DATA(t21)[1]=t26;
t21=restore();
restore();
t19=top();
S_DATA(t19)[1]=t21;
t19=restore();
restore();
X t27=___app(t19); // REF: app
t17=top();
S_DATA(t17)[1]=t27;
t17=restore();
restore();
save(t17);
X t28=allocate(2); // CONS
save(t28);
t17=pick(2);
X t29=select1(TO_N(1),t17); // SELECT (simple)
t28=top();
S_DATA(t28)[0]=t29;
t29=pick(2);
X t30=select1(TO_N(2),t29); // SELECT (simple)
t28=top();
S_DATA(t28)[1]=t30;
t28=restore();
restore();
X t31=___unparse_3aprec(t28); // REF: unparse:prec
RETURN(t31);}
//---------------------------------------- toplevel:loop' (fpclib.fp:66)
DEFINE(___toplevel_3aloop_27){
ENTRY;
loop:;
tracecall("fpclib.fp:66:  toplevel:loop'");
save(x); // SEQ
save(x); // COND
X t2=select1(TO_N(3),x); // SELECT (simple)
save(t2);
X t3=allocate(2); // CONS
save(t3);
t2=pick(2);
X t4=lf[361]; // CONST
t3=top();
S_DATA(t3)[0]=t4;
t4=pick(2);
X t5=___id(t4); // REF: id
t3=top();
S_DATA(t3)[1]=t5;
t3=restore();
restore();
X t6=___fetch(t3); // REF: fetch
X t1;
x=restore();
if(t6!=F){
save(x);
X t7=allocate(2); // CONS
save(t7);
x=pick(2);
X t8=lf[340]; // CONST
t7=top();
S_DATA(t7)[0]=t8;
t8=pick(2);
X t9=select1(TO_N(1),t8); // SELECT (simple)
X t10=___tos(t9); // REF: tos
t7=top();
S_DATA(t7)[1]=t10;
t7=restore();
restore();
X t11=___cat(t7); // REF: cat
save(t11); // SEQ
X t12=____5femit(t11); // REF: _emit
t11=restore();
X t13=TO_N(10); // ICONST
X t14=____5femit(t13); // REF: _emit
t1=t14;
}else{
X t15=___id(x); // REF: id
t1=t15;}
x=restore();
save(x);
X t16=allocate(3); // CONS
save(t16);
x=pick(2);
X t17=select1(TO_N(1),x); // SELECT (simple)
t16=top();
S_DATA(t16)[0]=t17;
t17=pick(2);
X t18=select1(TO_N(2),t17); // SELECT (simple)
int t20; // ALPHA
check_S(t18,"@");
int t21=S_LENGTH(t18);
save(t18);
X t19=allocate(t21);
save(t19);
for(t20=0;t20<t21;++t20){
X t19=S_DATA(pick(2))[t20];
save(t19);
X t22=allocate(4); // CONS
save(t22);
t19=pick(2);
X t23=select1(TO_N(1),t19); // SELECT (simple)
t22=top();
S_DATA(t22)[0]=t23;
t23=pick(2);
X t24=select1(TO_N(2),t23); // SELECT (simple)
t22=top();
S_DATA(t22)[1]=t24;
t24=pick(2);
X t25=select1(TO_N(2),t24); // SELECT (simple)
X t26=___analysis_3atreesize(t25); // REF: analysis:treesize
t22=top();
S_DATA(t22)[2]=t26;
t26=pick(2);
X t27=select1(TO_N(2),t26); // SELECT (simple)
X t28=___analysis_3acollectrefs(t27); // REF: analysis:collectrefs
t22=top();
S_DATA(t22)[3]=t28;
t22=restore();
restore();
S_DATA(top())[t20]=t22;}
t19=restore();
restore();
t16=top();
S_DATA(t16)[1]=t19;
t19=pick(2);
X t29=select1(TO_N(3),t19); // SELECT (simple)
t16=top();
S_DATA(t16)[2]=t29;
t16=restore();
restore();
save(t16); // SEQ
save(t16); // COND
X t31=select1(TO_N(3),t16); // SELECT (simple)
save(t31);
X t32=allocate(2); // CONS
save(t32);
t31=pick(2);
X t33=lf[361]; // CONST
t32=top();
S_DATA(t32)[0]=t33;
t33=pick(2);
X t34=___id(t33); // REF: id
t32=top();
S_DATA(t32)[1]=t34;
t32=restore();
restore();
X t35=___fetch(t32); // REF: fetch
X t30;
t16=restore();
if(t35!=F){
X t36=select1(TO_N(2),t16); // SELECT (simple)
X t37=___analysis_3areport_5fstats(t36); // REF: analysis:report_stats
t30=t37;
}else{
X t38=___id(t16); // REF: id
t30=t38;}
t16=restore();
save(t16); // COND
save(t16); // COND
X t42=TO_N(1); // ICONST
save(t42);
X t43=select1(TO_N(1),t16); // SELECT (simple)
t42=restore();
X t41=(t42==t43)||eq1(t42,t43)?T:F; // EQ
X t40;
t16=restore();
if(t41!=F){
save(t16);
X t44=allocate(2); // CONS
save(t44);
t16=pick(2);
X t45=lf[379]; // CONST
t44=top();
S_DATA(t44)[0]=t45;
t45=pick(2);
X t46=select1(TO_N(3),t45); // SELECT (simple)
t44=top();
S_DATA(t44)[1]=t46;
t44=restore();
restore();
X t47=___fetch(t44); // REF: fetch
save(t47); // COND
X t49=___id(t47); // REF: id
X t48;
t47=restore();
if(t49!=F){
X t50=F; // ICONST
t48=t50;
}else{
X t51=T; // ICONST
t48=t51;}
t40=t48;
}else{
X t52=F; // ICONST
t40=t52;}
X t39;
t16=restore();
if(t40!=F){
save(t16); // COND
X t54=select1(TO_N(2),t16); // SELECT (simple)
save(t54);
X t55=allocate(3); // CONS
save(t55);
t54=pick(2);
save(t54);
X t56=allocate(2); // CONS
save(t56);
t54=pick(2);
X t57=___id(t54); // REF: id
t56=top();
S_DATA(t56)[0]=t57;
t57=pick(2);
X t58=___id(t57); // REF: id
t56=top();
S_DATA(t56)[1]=t58;
t56=restore();
restore();
X t59=___dr(t56); // REF: dr
int t61; // ALPHA
check_S(t59,"@");
int t62=S_LENGTH(t59);
save(t59);
X t60=allocate(t62);
save(t60);
for(t61=0;t61<t62;++t61){
X t60=S_DATA(pick(2))[t61];
save(t60);
X t63=allocate(2); // CONS
save(t63);
t60=pick(2);
X t64=select1(TO_N(1),t60); // SELECT (simple)
t63=top();
S_DATA(t63)[0]=t64;
t64=pick(2);
save(t64);
X t65=allocate(2); // CONS
save(t65);
t64=pick(2);
X t67=TO_N(1); // ICONST
save(t67);
X t68=select1(TO_N(1),t64); // SELECT (simple)
t67=restore();
X t66=select1(t67,t68); // SELECT
t65=top();
S_DATA(t65)[0]=t66;
t66=pick(2);
X t69=select1(TO_N(2),t66); // SELECT (simple)
t65=top();
S_DATA(t65)[1]=t69;
t65=restore();
restore();
X t70=___fetchall(t65); // REF: fetchall
t63=top();
S_DATA(t63)[1]=t70;
t63=restore();
restore();
save(t63); // COND
save(t63);
X t72=allocate(2); // CONS
save(t72);
t63=pick(2);
X t73=select1(TO_N(2),t63); // SELECT (simple)
X t74=___len(t73); // REF: len
t72=top();
S_DATA(t72)[0]=t74;
t74=pick(2);
X t75=TO_N(1); // ICONST
t72=top();
S_DATA(t72)[1]=t75;
t72=restore();
restore();
X t77=TO_N(1); // ICONST
save(t77);
X t78=___cmp(t72); // REF: cmp
t77=restore();
X t76=(t77==t78)||eq1(t77,t78)?T:F; // EQ
X t71;
t63=restore();
if(t76!=F){
save(t63); // SEQ
save(t63);
X t79=allocate(4); // CONS
save(t79);
t63=pick(2);
X t81=TO_N(2); // ICONST
save(t81);
X t83=TO_N(2); // ICONST
save(t83);
X t84=select1(TO_N(1),t63); // SELECT (simple)
t83=restore();
X t82=select1(t83,t84); // SELECT
t81=restore();
X t80=select1(t81,t82); // SELECT
X t85=___tos(t80); // REF: tos
t79=top();
S_DATA(t79)[0]=t85;
t85=pick(2);
X t86=lf[347]; // CONST
t79=top();
S_DATA(t79)[1]=t86;
t86=pick(2);
X t88=TO_N(1); // ICONST
save(t88);
X t89=select1(TO_N(1),t86); // SELECT (simple)
t88=restore();
X t87=select1(t88,t89); // SELECT
X t90=___tos(t87); // REF: tos
t79=top();
S_DATA(t79)[2]=t90;
t90=pick(2);
X t91=lf[351]; // CONST
t79=top();
S_DATA(t79)[3]=t91;
t79=restore();
restore();
X t92=___cat(t79); // REF: cat
save(t92); // SEQ
X t93=____5femit(t92); // REF: _emit
t92=restore();
X t94=TO_N(10); // ICONST
X t95=____5femit(t94); // REF: _emit
t63=restore();
X t96=T; // ICONST
t71=t96;
}else{
X t97=F; // ICONST
t71=t97;}
S_DATA(top())[t61]=t71;}
t60=restore();
restore();
X t98=___any(t60); // REF: any
t55=top();
S_DATA(t55)[0]=t98;
t98=pick(2);
int t100; // ALPHA
check_S(t98,"@");
int t101=S_LENGTH(t98);
save(t98);
X t99=allocate(t101);
save(t99);
for(t100=0;t100<t101;++t100){
X t99=S_DATA(pick(2))[t100];
save(t99); // COND
save(t99);
X t103=allocate(2); // CONS
save(t103);
t99=pick(2);
X t104=select1(TO_N(1),t99); // SELECT (simple)
t103=top();
S_DATA(t103)[0]=t104;
t104=pick(2);
X t105=lf[349]; // CONST
t103=top();
S_DATA(t103)[1]=t105;
t103=restore();
restore();
X t106=___index(t103); // REF: index
X t102;
t99=restore();
if(t106!=F){
save(t99); // SEQ
save(t99);
X t107=allocate(4); // CONS
save(t107);
t99=pick(2);
X t109=TO_N(2); // ICONST
save(t109);
X t110=select1(TO_N(2),t99); // SELECT (simple)
t109=restore();
X t108=select1(t109,t110); // SELECT
X t111=___tos(t108); // REF: tos
t107=top();
S_DATA(t107)[0]=t111;
t111=pick(2);
X t112=lf[348]; // CONST
t107=top();
S_DATA(t107)[1]=t112;
t112=pick(2);
X t113=select1(TO_N(1),t112); // SELECT (simple)
X t114=___tos(t113); // REF: tos
t107=top();
S_DATA(t107)[2]=t114;
t114=pick(2);
X t115=lf[351]; // CONST
t107=top();
S_DATA(t107)[3]=t115;
t107=restore();
restore();
X t116=___cat(t107); // REF: cat
save(t116); // SEQ
X t117=____5femit(t116); // REF: _emit
t116=restore();
X t118=TO_N(10); // ICONST
X t119=____5femit(t118); // REF: _emit
t99=restore();
X t120=T; // ICONST
t102=t120;
}else{
X t121=F; // ICONST
t102=t121;}
S_DATA(top())[t100]=t102;}
t99=restore();
restore();
X t122=___any(t99); // REF: any
t55=top();
S_DATA(t55)[1]=t122;
t122=pick(2);
save(t122);
X t123=allocate(2); // CONS
save(t123);
t122=pick(2);
int t125; // ALPHA
check_S(t122,"@");
int t126=S_LENGTH(t122);
save(t122);
X t124=allocate(t126);
save(t124);
for(t125=0;t125<t126;++t125){
X t124=S_DATA(pick(2))[t125];
X t127=select1(TO_N(4),t124); // SELECT (simple)
S_DATA(top())[t125]=t127;}
t124=restore();
restore();
t123=top();
S_DATA(t123)[0]=t124;
t124=pick(2);
int t129; // ALPHA
check_S(t124,"@");
int t130=S_LENGTH(t124);
save(t124);
X t128=allocate(t130);
save(t128);
for(t129=0;t129<t130;++t129){
X t128=S_DATA(pick(2))[t129];
X t131=select1(TO_N(1),t128); // SELECT (simple)
S_DATA(top())[t129]=t131;}
t128=restore();
restore();
t123=top();
S_DATA(t123)[1]=t128;
t123=restore();
restore();
X t132=___dr(t123); // REF: dr
int t134; // ALPHA
check_S(t132,"@");
int t135=S_LENGTH(t132);
save(t132);
X t133=allocate(t135);
save(t133);
for(t134=0;t134<t135;++t134){
X t133=S_DATA(pick(2))[t134];
X t136=___dr(t133); // REF: dr
int t138; // ALPHA
check_S(t136,"@");
int t139=S_LENGTH(t136);
save(t136);
X t137=allocate(t139);
save(t137);
for(t138=0;t138<t139;++t138){
X t137=S_DATA(pick(2))[t138];
save(t137); // COND
save(t137);
X t141=allocate(2); // CONS
save(t141);
t137=pick(2);
X t143=TO_N(3); // ICONST
save(t143);
X t144=select1(TO_N(1),t137); // SELECT (simple)
t143=restore();
X t142=select1(t143,t144); // SELECT
t141=top();
S_DATA(t141)[0]=t142;
t142=pick(2);
X t145=select1(TO_N(2),t142); // SELECT (simple)
t141=top();
S_DATA(t141)[1]=t145;
t141=restore();
restore();
X t146=___index(t141); // REF: index
X t140;
t137=restore();
if(t146!=F){
X t147=F; // ICONST
t140=t147;
}else{
save(t137); // COND
save(t137);
X t149=allocate(2); // CONS
save(t149);
t137=pick(2);
X t151=TO_N(3); // ICONST
save(t151);
X t152=select1(TO_N(1),t137); // SELECT (simple)
t151=restore();
X t150=select1(t151,t152); // SELECT
t149=top();
S_DATA(t149)[0]=t150;
t150=pick(2);
X t153=lf[349]; // CONST
t149=top();
S_DATA(t149)[1]=t153;
t149=restore();
restore();
X t154=___index(t149); // REF: index
X t148;
t137=restore();
if(t154!=F){
X t155=F; // ICONST
t148=t155;
}else{
save(t137); // SEQ
save(t137);
X t156=allocate(4); // CONS
save(t156);
t137=pick(2);
X t158=TO_N(2); // ICONST
save(t158);
X t159=select1(TO_N(1),t137); // SELECT (simple)
t158=restore();
X t157=select1(t158,t159); // SELECT
X t160=___tos(t157); // REF: tos
t156=top();
S_DATA(t156)[0]=t160;
t160=pick(2);
X t161=lf[350]; // CONST
t156=top();
S_DATA(t156)[1]=t161;
t161=pick(2);
X t163=TO_N(3); // ICONST
save(t163);
X t164=select1(TO_N(1),t161); // SELECT (simple)
t163=restore();
X t162=select1(t163,t164); // SELECT
X t165=___tos(t162); // REF: tos
t156=top();
S_DATA(t156)[2]=t165;
t165=pick(2);
X t166=lf[351]; // CONST
t156=top();
S_DATA(t156)[3]=t166;
t156=restore();
restore();
X t167=___cat(t156); // REF: cat
save(t167); // SEQ
X t168=____5femit(t167); // REF: _emit
t167=restore();
X t169=TO_N(10); // ICONST
X t170=____5femit(t169); // REF: _emit
t137=restore();
X t171=T; // ICONST
t148=t171;}
t140=t148;}
S_DATA(top())[t138]=t140;}
t137=restore();
restore();
X t172=___any(t137); // REF: any
S_DATA(top())[t134]=t172;}
t133=restore();
restore();
X t173=___any(t133); // REF: any
t55=top();
S_DATA(t55)[2]=t173;
t55=restore();
restore();
X t174=___any(t55); // REF: any
X t53;
t16=restore();
if(t174!=F){
X t175=lf[352]; // CONST
X t176=___quit(t175); // REF: quit
t53=t176;
}else{
X t177=___id(t16); // REF: id
t53=t177;}
t39=t53;
}else{
X t178=___id(t16); // REF: id
t39=t178;}
save(t39);
X t179=allocate(3); // CONS
save(t179);
t39=pick(2);
X t180=select1(TO_N(1),t39); // SELECT (simple)
t179=top();
S_DATA(t179)[0]=t180;
t180=pick(2);
save(t180);
X t181=allocate(2); // CONS
save(t181);
t180=pick(2);
X t182=select1(TO_N(2),t180); // SELECT (simple)
t181=top();
S_DATA(t181)[0]=t182;
t182=pick(2);
X t183=select1(TO_N(3),t182); // SELECT (simple)
t181=top();
S_DATA(t181)[1]=t183;
t181=restore();
restore();
save(t181);
X t184=allocate(4); // CONS
save(t184);
t181=pick(2);
X t185=select1(TO_N(1),t181); // SELECT (simple)
t184=top();
S_DATA(t184)[0]=t185;
t185=pick(2);
save(t185);
X t186=allocate(2); // CONS
save(t186);
t185=pick(2);
X t187=lf[361]; // CONST
t186=top();
S_DATA(t186)[0]=t187;
t187=pick(2);
X t188=select1(TO_N(2),t187); // SELECT (simple)
t186=top();
S_DATA(t186)[1]=t188;
t186=restore();
restore();
X t189=___fetch(t186); // REF: fetch
t184=top();
S_DATA(t184)[1]=t189;
t189=pick(2);
save(t189);
X t190=allocate(2); // CONS
save(t190);
t189=pick(2);
X t191=lf[383]; // CONST
t190=top();
S_DATA(t190)[0]=t191;
t191=pick(2);
X t192=select1(TO_N(2),t191); // SELECT (simple)
t190=top();
S_DATA(t190)[1]=t192;
t190=restore();
restore();
save(t190); // OR
X t194=___fetch(t190); // REF: fetch
X t193=t194;
t190=restore();
if(t193==F){
X t195=lf[346]; // CONST
t193=t195;}
t184=top();
S_DATA(t184)[2]=t193;
t193=pick(2);
X t196=TO_N(1); // ICONST
t184=top();
S_DATA(t184)[3]=t196;
t184=restore();
restore();
X t197=___treeshake_3aloop(t184); // REF: treeshake:loop
t179=top();
S_DATA(t179)[1]=t197;
t197=pick(2);
X t198=select1(TO_N(3),t197); // SELECT (simple)
t179=top();
S_DATA(t179)[2]=t198;
t179=restore();
restore();
save(t179);
X t199=allocate(3); // CONS
save(t199);
t179=pick(2);
X t200=select1(TO_N(1),t179); // SELECT (simple)
t199=top();
S_DATA(t199)[0]=t200;
t200=pick(2);
X t201=select1(TO_N(2),t200); // SELECT (simple)
save(t201);
X t202=allocate(2); // CONS
save(t202);
t201=pick(2);
X t203=___id(t201); // REF: id
t202=top();
S_DATA(t202)[0]=t203;
t203=pick(2);
X t204=___id(t203); // REF: id
t202=top();
S_DATA(t202)[1]=t204;
t202=restore();
restore();
X t205=___dr(t202); // REF: dr
int t207; // ALPHA
check_S(t205,"@");
int t208=S_LENGTH(t205);
save(t205);
X t206=allocate(t208);
save(t206);
for(t207=0;t207<t208;++t207){
X t206=S_DATA(pick(2))[t207];
save(t206); // COND
X t211=TO_N(1); // ICONST
save(t211);
save(t206);
X t212=allocate(2); // CONS
save(t212);
t206=pick(2);
X t214=TO_N(1); // ICONST
save(t214);
X t215=select1(TO_N(1),t206); // SELECT (simple)
t214=restore();
X t213=select1(t214,t215); // SELECT
t212=top();
S_DATA(t212)[0]=t213;
t213=pick(2);
X t216=select1(TO_N(2),t213); // SELECT (simple)
int t218; // ALPHA
check_S(t216,"@");
int t219=S_LENGTH(t216);
save(t216);
X t217=allocate(t219);
save(t217);
for(t218=0;t218<t219;++t218){
X t217=S_DATA(pick(2))[t218];
X t220=select1(TO_N(4),t217); // SELECT (simple)
S_DATA(top())[t218]=t220;}
t217=restore();
restore();
t212=top();
S_DATA(t212)[1]=t217;
t212=restore();
restore();
X t221=___dl(t212); // REF: dl
int t223; // ALPHA
check_S(t221,"@");
int t224=S_LENGTH(t221);
save(t221);
X t222=allocate(t224);
save(t222);
for(t223=0;t223<t224;++t223){
X t222=S_DATA(pick(2))[t223];
save(t222);
X t225=allocate(2); // CONS
save(t225);
t222=pick(2);
X t226=select1(TO_N(1),t222); // SELECT (simple)
t225=top();
S_DATA(t225)[0]=t226;
t226=pick(2);
X t227=select1(TO_N(2),t226); // SELECT (simple)
int t229; // ALPHA
check_S(t227,"@");
int t230=S_LENGTH(t227);
save(t227);
X t228=allocate(t230);
save(t228);
for(t229=0;t229<t230;++t229){
X t228=S_DATA(pick(2))[t229];
X t231=select1(TO_N(3),t228); // SELECT (simple)
S_DATA(top())[t229]=t231;}
t228=restore();
restore();
t225=top();
S_DATA(t225)[1]=t228;
t225=restore();
restore();
save(t225); // COND
X t234=lf[381]; // CONST
save(t234);
X t235=select1(TO_N(2),t225); // SELECT (simple)
t234=restore();
X t233=(t234==t235)||eq1(t234,t235)?T:F; // EQ
X t232;
t225=restore();
if(t233!=F){
X t236=TO_N(0); // ICONST
t232=t236;
}else{
X t237=___dl(t225); // REF: dl
int t239; // ALPHA
check_S(t237,"@");
int t240=S_LENGTH(t237);
save(t237);
X t238=allocate(t240);
save(t238);
for(t239=0;t239<t240;++t239){
X t238=S_DATA(pick(2))[t239];
save(t238); // COND
X t242=___eq(t238); // REF: eq
X t241;
t238=restore();
if(t242!=F){
X t243=TO_N(1); // ICONST
t241=t243;
}else{
X t244=TO_N(0); // ICONST
t241=t244;}
S_DATA(top())[t239]=t241;}
t238=restore();
restore();
int t247; // INSERTR
check_S(t238,"/");
int t248=S_LENGTH(t238);
X t245=t248==0?TO_N(0):S_DATA(t238)[t248-1];
save(t238);
for(t247=t248-2;t247>=0;--t247){
X t246=sequence(2,S_DATA(pick(1))[t247],t245);
X t249=___add(t246); // REF: add
t245=t249;}
restore();
t232=t245;}
S_DATA(top())[t223]=t232;}
t222=restore();
restore();
int t252; // INSERTR
check_S(t222,"/");
int t253=S_LENGTH(t222);
X t250=t253==0?TO_N(0):S_DATA(t222)[t253-1];
save(t222);
for(t252=t253-2;t252>=0;--t252){
X t251=sequence(2,S_DATA(pick(1))[t252],t250);
X t254=___add(t251); // REF: add
t250=t254;}
restore();
t211=restore();
X t210=(t211==t250)||eq1(t211,t250)?T:F; // EQ
X t209;
t206=restore();
if(t210!=F){
save(t206);
X t255=allocate(4); // CONS
save(t255);
t206=pick(2);
X t257=TO_N(1); // ICONST
save(t257);
X t258=select1(TO_N(1),t206); // SELECT (simple)
t257=restore();
X t256=select1(t257,t258); // SELECT
t255=top();
S_DATA(t255)[0]=t256;
t256=pick(2);
X t260=TO_N(2); // ICONST
save(t260);
X t261=select1(TO_N(1),t256); // SELECT (simple)
t260=restore();
X t259=select1(t260,t261); // SELECT
t255=top();
S_DATA(t255)[1]=t259;
t259=pick(2);
X t262=TO_N(0); // ICONST
t255=top();
S_DATA(t255)[2]=t262;
t262=pick(2);
X t264=TO_N(4); // ICONST
save(t264);
X t265=select1(TO_N(1),t262); // SELECT (simple)
t264=restore();
X t263=select1(t264,t265); // SELECT
t255=top();
S_DATA(t255)[3]=t263;
t255=restore();
restore();
t209=t255;
}else{
X t266=select1(TO_N(1),t206); // SELECT (simple)
t209=t266;}
S_DATA(top())[t207]=t209;}
t206=restore();
restore();
t199=top();
S_DATA(t199)[1]=t206;
t206=pick(2);
X t267=select1(TO_N(3),t206); // SELECT (simple)
t199=top();
S_DATA(t199)[2]=t267;
t199=restore();
restore();
save(t199);
X t268=allocate(3); // CONS
save(t268);
t199=pick(2);
X t269=select1(TO_N(1),t199); // SELECT (simple)
t268=top();
S_DATA(t268)[0]=t269;
t269=pick(2);
X t270=select1(TO_N(2),t269); // SELECT (simple)
t268=top();
S_DATA(t268)[1]=t270;
t270=pick(2);
save(t270); // SEQ
save(t270); // COND
X t272=select1(TO_N(3),t270); // SELECT (simple)
save(t272);
X t273=allocate(2); // CONS
save(t273);
t272=pick(2);
X t274=lf[361]; // CONST
t273=top();
S_DATA(t273)[0]=t274;
t274=pick(2);
X t275=___id(t274); // REF: id
t273=top();
S_DATA(t273)[1]=t275;
t273=restore();
restore();
X t276=___fetch(t273); // REF: fetch
X t271;
t270=restore();
if(t276!=F){
save(t270); // SEQ
X t277=lf[342]; // CONST
save(t277); // SEQ
X t278=____5femit(t277); // REF: _emit
t277=restore();
X t279=TO_N(10); // ICONST
X t280=____5femit(t279); // REF: _emit
t270=restore();
X t281=select1(TO_N(2),t270); // SELECT (simple)
int t283; // ALPHA
check_S(t281,"@");
int t284=S_LENGTH(t281);
save(t281);
X t282=allocate(t284);
save(t282);
for(t283=0;t283<t284;++t283){
X t282=S_DATA(pick(2))[t283];
save(t282); // COND
X t287=TO_N(0); // ICONST
save(t287);
X t288=select1(TO_N(3),t282); // SELECT (simple)
t287=restore();
X t286=(t287==t288)||eq1(t287,t288)?T:F; // EQ
X t285;
t282=restore();
if(t286!=F){
save(t282);
X t289=allocate(5); // CONS
save(t289);
t282=pick(2);
X t290=lf[343]; // CONST
t289=top();
S_DATA(t289)[0]=t290;
t290=pick(2);
X t292=TO_N(2); // ICONST
save(t292);
X t293=select1(TO_N(2),t290); // SELECT (simple)
t292=restore();
X t291=select1(t292,t293); // SELECT
X t294=___tos(t291); // REF: tos
t289=top();
S_DATA(t289)[1]=t294;
t294=pick(2);
X t295=lf[344]; // CONST
t289=top();
S_DATA(t289)[2]=t295;
t295=pick(2);
X t296=select1(TO_N(1),t295); // SELECT (simple)
X t297=___tos(t296); // REF: tos
t289=top();
S_DATA(t289)[3]=t297;
t297=pick(2);
X t298=lf[345]; // CONST
t289=top();
S_DATA(t289)[4]=t298;
t289=restore();
restore();
X t299=___cat(t289); // REF: cat
save(t299); // SEQ
X t300=____5femit(t299); // REF: _emit
t299=restore();
X t301=TO_N(10); // ICONST
X t302=____5femit(t301); // REF: _emit
t285=t302;
}else{
X t303=___id(t282); // REF: id
t285=t303;}
S_DATA(top())[t283]=t285;}
t282=restore();
restore();
t271=t282;
}else{
X t304=___id(t270); // REF: id
t271=t304;}
t270=restore();
X t305=select1(TO_N(3),t270); // SELECT (simple)
t268=top();
S_DATA(t268)[2]=t305;
t268=restore();
restore();
save(t268);
X t306=allocate(3); // CONS
save(t306);
t268=pick(2);
X t307=select1(TO_N(1),t268); // SELECT (simple)
t306=top();
S_DATA(t306)[0]=t307;
t307=pick(2);
save(t307);
X t308=allocate(3); // CONS
save(t308);
t307=pick(2);
X t309=select1(TO_N(2),t307); // SELECT (simple)
t308=top();
S_DATA(t308)[0]=t309;
t309=pick(2);
save(t309);
X t310=allocate(2); // CONS
save(t310);
t309=pick(2);
X t311=lf[366]; // CONST
t310=top();
S_DATA(t310)[0]=t311;
t311=pick(2);
X t312=select1(TO_N(3),t311); // SELECT (simple)
t310=top();
S_DATA(t310)[1]=t312;
t310=restore();
restore();
save(t310); // OR
X t314=___fetch(t310); // REF: fetch
X t313=t314;
t310=restore();
if(t313==F){
X t315=TO_N(5); // ICONST
t313=t315;}
t308=top();
S_DATA(t308)[1]=t313;
t313=pick(2);
X t316=select1(TO_N(3),t313); // SELECT (simple)
save(t316);
X t317=allocate(2); // CONS
save(t317);
t316=pick(2);
X t318=lf[361]; // CONST
t317=top();
S_DATA(t317)[0]=t318;
t318=pick(2);
X t319=___id(t318); // REF: id
t317=top();
S_DATA(t317)[1]=t319;
t317=restore();
restore();
X t320=___fetch(t317); // REF: fetch
t308=top();
S_DATA(t308)[2]=t320;
t308=restore();
restore();
X t322=TO_N(3); // ICONST
save(t322);
save(t308);
X t323=allocate(5); // CONS
save(t323);
t308=pick(2);
X t324=TO_N(1); // ICONST
t323=top();
S_DATA(t323)[0]=t324;
t324=pick(2);
X t325=select1(TO_N(1),t324); // SELECT (simple)
t323=top();
S_DATA(t323)[1]=t325;
t325=pick(2);
X t326=EMPTY; // ICONST
t323=top();
S_DATA(t323)[2]=t326;
t326=pick(2);
X t327=select1(TO_N(2),t326); // SELECT (simple)
t323=top();
S_DATA(t323)[3]=t327;
t327=pick(2);
X t328=select1(TO_N(3),t327); // SELECT (simple)
t323=top();
S_DATA(t323)[4]=t328;
t323=restore();
restore();
X t329=t323; // WHILE
for(;;){
save(t329);
save(t329);
X t330=allocate(2); // CONS
save(t330);
t329=pick(2);
X t331=select1(TO_N(1),t329); // SELECT (simple)
t330=top();
S_DATA(t330)[0]=t331;
t331=pick(2);
X t332=select1(TO_N(2),t331); // SELECT (simple)
X t333=___len(t332); // REF: len
t330=top();
S_DATA(t330)[1]=t333;
t330=restore();
restore();
X t334=___le(t330); // REF: le
t329=restore();
if(t334==F) break;
save(t329);
X t335=allocate(2); // CONS
save(t335);
t329=pick(2);
X t336=___id(t329); // REF: id
t335=top();
S_DATA(t335)[0]=t336;
t336=pick(2);
save(t336);
X t337=allocate(2); // CONS
save(t337);
t336=pick(2);
save(t336);
X t339=select1(TO_N(1),t336); // SELECT (simple)
t336=restore();
save(t339);
X t340=select1(TO_N(2),t336); // SELECT (simple)
t339=restore();
X t338=select1(t339,t340); // SELECT
t337=top();
S_DATA(t337)[0]=t338;
t338=pick(2);
X t341=___tl(t338); // REF: tl
t337=top();
S_DATA(t337)[1]=t341;
t337=restore();
restore();
save(t337);
X t342=allocate(3); // CONS
save(t342);
t337=pick(2);
X t344=TO_N(1); // ICONST
save(t344);
X t345=select1(TO_N(1),t337); // SELECT (simple)
t344=restore();
X t343=select1(t344,t345); // SELECT
t342=top();
S_DATA(t342)[0]=t343;
t343=pick(2);
X t347=TO_N(2); // ICONST
save(t347);
X t348=select1(TO_N(2),t343); // SELECT (simple)
t347=restore();
X t346=select1(t347,t348); // SELECT
t342=top();
S_DATA(t342)[1]=t346;
t346=pick(2);
save(t346);
X t349=allocate(3); // CONS
save(t349);
t346=pick(2);
X t351=TO_N(2); // ICONST
save(t351);
X t352=select1(TO_N(1),t346); // SELECT (simple)
t351=restore();
X t350=select1(t351,t352); // SELECT
t349=top();
S_DATA(t349)[0]=t350;
t350=pick(2);
X t353=select1(TO_N(1),t350); // SELECT (simple)
t349=top();
S_DATA(t349)[1]=t353;
t353=pick(2);
X t354=select1(TO_N(2),t353); // SELECT (simple)
t349=top();
S_DATA(t349)[2]=t354;
t349=restore();
restore();
X t355=___inline_3awalkrec(t349); // REF: inline:walkrec
t342=top();
S_DATA(t342)[2]=t355;
t342=restore();
restore();
save(t342);
X t356=allocate(2); // CONS
save(t356);
t342=pick(2);
X t357=select1(TO_N(2),t342); // SELECT (simple)
t356=top();
S_DATA(t356)[0]=t357;
t357=pick(2);
save(t357);
X t358=allocate(2); // CONS
save(t358);
t357=pick(2);
save(t357);
X t359=allocate(2); // CONS
save(t359);
t357=pick(2);
X t360=select1(TO_N(1),t357); // SELECT (simple)
t359=top();
S_DATA(t359)[0]=t360;
t360=pick(2);
X t362=TO_N(1); // ICONST
save(t362);
X t363=select1(TO_N(3),t360); // SELECT (simple)
t362=restore();
X t361=select1(t362,t363); // SELECT
t359=top();
S_DATA(t359)[1]=t361;
t359=restore();
restore();
t358=top();
S_DATA(t358)[0]=t359;
t359=pick(2);
X t365=TO_N(2); // ICONST
save(t365);
X t366=select1(TO_N(3),t359); // SELECT (simple)
t365=restore();
X t364=select1(t365,t366); // SELECT
X t367=___tl(t364); // REF: tl
X t368=___tl(t367); // REF: tl
t358=top();
S_DATA(t358)[1]=t368;
t358=restore();
restore();
X t369=___cat(t358); // REF: cat
t356=top();
S_DATA(t356)[1]=t369;
t356=restore();
restore();
X t370=___ar(t356); // REF: ar
t335=top();
S_DATA(t335)[1]=t370;
t335=restore();
restore();
save(t335);
X t371=allocate(5); // CONS
save(t371);
t335=pick(2);
X t373=TO_N(1); // ICONST
save(t373);
X t374=select1(TO_N(1),t335); // SELECT (simple)
t373=restore();
X t372=select1(t373,t374); // SELECT
save(t372);
X t375=allocate(2); // CONS
save(t375);
t372=pick(2);
X t376=___id(t372); // REF: id
t375=top();
S_DATA(t375)[0]=t376;
t376=pick(2);
X t377=TO_N(1); // ICONST
t375=top();
S_DATA(t375)[1]=t377;
t375=restore();
restore();
X t378=___add(t375); // REF: add
t371=top();
S_DATA(t371)[0]=t378;
t378=pick(2);
X t380=TO_N(2); // ICONST
save(t380);
X t381=select1(TO_N(1),t378); // SELECT (simple)
t380=restore();
X t379=select1(t380,t381); // SELECT
t371=top();
S_DATA(t371)[1]=t379;
t379=pick(2);
X t382=select1(TO_N(2),t379); // SELECT (simple)
t371=top();
S_DATA(t371)[2]=t382;
t382=pick(2);
X t384=TO_N(4); // ICONST
save(t384);
X t385=select1(TO_N(1),t382); // SELECT (simple)
t384=restore();
X t383=select1(t384,t385); // SELECT
t371=top();
S_DATA(t371)[3]=t383;
t383=pick(2);
X t387=TO_N(5); // ICONST
save(t387);
X t388=select1(TO_N(1),t383); // SELECT (simple)
t387=restore();
X t386=select1(t387,t388); // SELECT
t371=top();
S_DATA(t371)[4]=t386;
t371=restore();
restore();
t329=t371;}
t322=restore();
X t321=select1(t322,t329); // SELECT
t306=top();
S_DATA(t306)[1]=t321;
t321=pick(2);
X t389=select1(TO_N(3),t321); // SELECT (simple)
t306=top();
S_DATA(t306)[2]=t389;
t306=restore();
restore();
save(t306);
X t390=allocate(3); // CONS
save(t390);
t306=pick(2);
X t391=select1(TO_N(1),t306); // SELECT (simple)
t390=top();
S_DATA(t390)[0]=t391;
t391=pick(2);
X t392=select1(TO_N(3),t391); // SELECT (simple)
t390=top();
S_DATA(t390)[1]=t392;
t392=pick(2);
X t393=select1(TO_N(2),t392); // SELECT (simple)
int t395; // ALPHA
check_S(t393,"@");
int t396=S_LENGTH(t393);
save(t393);
X t394=allocate(t396);
save(t394);
for(t395=0;t395<t396;++t395){
X t394=S_DATA(pick(2))[t395];
save(t394);
X t397=allocate(2); // CONS
save(t397);
t394=pick(2);
X t398=___id(t394); // REF: id
t397=top();
S_DATA(t397)[0]=t398;
t398=pick(2);
save(t398);
X t399=allocate(2); // CONS
save(t399);
t398=pick(2);
X t400=___len(t398); // REF: len
t399=top();
S_DATA(t399)[0]=t400;
t400=pick(2);
X t401=TO_N(5); // ICONST
t399=top();
S_DATA(t399)[1]=t401;
t399=restore();
restore();
X t402=___ge(t399); // REF: ge
t397=top();
S_DATA(t397)[1]=t402;
t397=restore();
restore();
save(t397); // COND
X t404=select1(TO_N(2),t397); // SELECT (simple)
X t403;
t397=restore();
if(t404!=F){
save(t397);
X t405=allocate(2); // CONS
save(t405);
t397=pick(2);
X t407=TO_N(5); // ICONST
save(t407);
X t408=select1(TO_N(1),t397); // SELECT (simple)
t407=restore();
X t406=select1(t407,t408); // SELECT
t405=top();
S_DATA(t405)[0]=t406;
t406=pick(2);
X t409=select1(TO_N(1),t406); // SELECT (simple)
X t410=___tlr(t409); // REF: tlr
t405=top();
S_DATA(t405)[1]=t410;
t405=restore();
restore();
t403=t405;
}else{
save(t397);
X t411=allocate(2); // CONS
save(t411);
t397=pick(2);
X t412=F; // ICONST
t411=top();
S_DATA(t411)[0]=t412;
t412=pick(2);
X t413=select1(TO_N(1),t412); // SELECT (simple)
t411=top();
S_DATA(t411)[1]=t413;
t411=restore();
restore();
t403=t411;}
S_DATA(top())[t395]=t403;}
t394=restore();
restore();
X t414=___trans(t394); // REF: trans
save(t414);
X t415=allocate(2); // CONS
save(t415);
t414=pick(2);
X t416=select1(TO_N(1),t414); // SELECT (simple)
X t417=___any(t416); // REF: any
t415=top();
S_DATA(t415)[0]=t417;
t417=pick(2);
X t418=select1(TO_N(2),t417); // SELECT (simple)
t415=top();
S_DATA(t415)[1]=t418;
t415=restore();
restore();
t390=top();
S_DATA(t390)[2]=t415;
t390=restore();
restore();
save(t390); // SEQ
save(t390); // COND
X t420=select1(TO_N(2),t390); // SELECT (simple)
save(t420);
X t421=allocate(2); // CONS
save(t421);
t420=pick(2);
X t422=lf[362]; // CONST
t421=top();
S_DATA(t421)[0]=t422;
t422=pick(2);
X t423=___id(t422); // REF: id
t421=top();
S_DATA(t421)[1]=t423;
t421=restore();
restore();
X t424=___fetch(t421); // REF: fetch
X t419;
t390=restore();
if(t424!=F){
save(t390); // SEQ
X t425=lf[341]; // CONST
save(t425); // SEQ
X t426=____5femit(t425); // REF: _emit
t425=restore();
X t427=TO_N(10); // ICONST
X t428=____5femit(t427); // REF: _emit
t390=restore();
save(t390); // SEQ
X t430=TO_N(2); // ICONST
save(t430);
X t431=select1(TO_N(3),t390); // SELECT (simple)
t430=restore();
X t429=select1(t430,t431); // SELECT
X t432=___unparse_3aunparse(t429); // REF: unparse:unparse
t390=restore();
X t434=TO_N(2); // ICONST
save(t434);
X t435=select1(TO_N(3),t390); // SELECT (simple)
t434=restore();
X t433=select1(t434,t435); // SELECT
t419=t433;
}else{
X t436=___id(t390); // REF: id
t419=t436;}
t390=restore();
save(t390); // COND
X t439=TO_N(1); // ICONST
save(t439);
X t440=select1(TO_N(3),t390); // SELECT (simple)
t439=restore();
X t438=select1(t439,t440); // SELECT
X t437;
t390=restore();
if(t438!=F){
save(t390);
X t441=allocate(3); // CONS
save(t441);
t390=pick(2);
X t442=select1(TO_N(1),t390); // SELECT (simple)
save(t442);
X t443=allocate(2); // CONS
save(t443);
t442=pick(2);
X t444=___id(t442); // REF: id
t443=top();
S_DATA(t443)[0]=t444;
t444=pick(2);
X t445=TO_N(1); // ICONST
t443=top();
S_DATA(t443)[1]=t445;
t443=restore();
restore();
X t446=___add(t443); // REF: add
t441=top();
S_DATA(t441)[0]=t446;
t446=pick(2);
X t448=TO_N(1); // ICONST
save(t448);
save(t446);
X t449=allocate(3); // CONS
save(t449);
t446=pick(2);
X t451=TO_N(2); // ICONST
save(t451);
X t452=select1(TO_N(3),t446); // SELECT (simple)
t451=restore();
X t450=select1(t451,t452); // SELECT
t449=top();
S_DATA(t449)[0]=t450;
t450=pick(2);
X t453=select1(TO_N(1),t450); // SELECT (simple)
t449=top();
S_DATA(t449)[1]=t453;
t453=pick(2);
X t454=select1(TO_N(2),t453); // SELECT (simple)
t449=top();
S_DATA(t449)[2]=t454;
t449=restore();
restore();
X t455=___rewrite_3arewrite(t449); // REF: rewrite:rewrite
t448=restore();
X t447=select1(t448,t455); // SELECT
t441=top();
S_DATA(t441)[1]=t447;
t447=pick(2);
X t456=select1(TO_N(2),t447); // SELECT (simple)
t441=top();
S_DATA(t441)[2]=t456;
t441=restore();
restore();
x=t441; // REF: toplevel:loop'
goto loop; // tail call: toplevel:loop'
X t457;
t437=t457;
}else{
save(t390); // SEQ
save(t390); // COND
X t459=select1(TO_N(2),t390); // SELECT (simple)
save(t459);
X t460=allocate(2); // CONS
save(t460);
t459=pick(2);
X t461=lf[361]; // CONST
t460=top();
S_DATA(t460)[0]=t461;
t461=pick(2);
X t462=___id(t461); // REF: id
t460=top();
S_DATA(t460)[1]=t462;
t460=restore();
restore();
X t463=___fetch(t460); // REF: fetch
X t458;
t390=restore();
if(t463!=F){
X t465=TO_N(2); // ICONST
save(t465);
X t466=select1(TO_N(3),t390); // SELECT (simple)
t465=restore();
X t464=select1(t465,t466); // SELECT
X t467=___analysis_3areport_5fstats(t464); // REF: analysis:report_stats
t458=t467;
}else{
X t468=___id(t390); // REF: id
t458=t468;}
t390=restore();
save(t390);
X t469=allocate(3); // CONS
save(t469);
t390=pick(2);
X t470=select1(TO_N(1),t390); // SELECT (simple)
t469=top();
S_DATA(t469)[0]=t470;
t470=pick(2);
save(t470);
X t471=allocate(3); // CONS
save(t471);
t470=pick(2);
X t473=TO_N(2); // ICONST
save(t473);
X t474=select1(TO_N(3),t470); // SELECT (simple)
t473=restore();
X t472=select1(t473,t474); // SELECT
t471=top();
S_DATA(t471)[0]=t472;
t472=pick(2);
X t475=select1(TO_N(1),t472); // SELECT (simple)
t471=top();
S_DATA(t471)[1]=t475;
t475=pick(2);
X t476=select1(TO_N(2),t475); // SELECT (simple)
t471=top();
S_DATA(t471)[2]=t476;
t471=restore();
restore();
X t477=___rewrite_3arewrite(t471); // REF: rewrite:rewrite
t469=top();
S_DATA(t469)[1]=t477;
t477=pick(2);
X t478=select1(TO_N(2),t477); // SELECT (simple)
t469=top();
S_DATA(t469)[2]=t478;
t469=restore();
restore();
save(t469); // COND
X t481=TO_N(2); // ICONST
save(t481);
X t482=select1(TO_N(2),t469); // SELECT (simple)
t481=restore();
X t480=select1(t481,t482); // SELECT
X t479;
t469=restore();
if(t480!=F){
save(t469);
X t483=allocate(3); // CONS
save(t483);
t469=pick(2);
X t484=select1(TO_N(1),t469); // SELECT (simple)
save(t484);
X t485=allocate(2); // CONS
save(t485);
t484=pick(2);
X t486=___id(t484); // REF: id
t485=top();
S_DATA(t485)[0]=t486;
t486=pick(2);
X t487=TO_N(1); // ICONST
t485=top();
S_DATA(t485)[1]=t487;
t485=restore();
restore();
X t488=___add(t485); // REF: add
t483=top();
S_DATA(t483)[0]=t488;
t488=pick(2);
X t490=TO_N(1); // ICONST
save(t490);
X t491=select1(TO_N(2),t488); // SELECT (simple)
t490=restore();
X t489=select1(t490,t491); // SELECT
t483=top();
S_DATA(t483)[1]=t489;
t489=pick(2);
X t492=select1(TO_N(3),t489); // SELECT (simple)
t483=top();
S_DATA(t483)[2]=t492;
t483=restore();
restore();
x=t483; // REF: toplevel:loop'
goto loop; // tail call: toplevel:loop'
X t493;
t479=t493;
}else{
X t495=TO_N(1); // ICONST
save(t495);
X t496=select1(TO_N(2),t469); // SELECT (simple)
t495=restore();
X t494=select1(t495,t496); // SELECT
t479=t494;}
t437=t479;}
RETURN(t437);}
//---------------------------------------- toplevel:options' (fpclib.fp:111)
DEFINE(___toplevel_3aoptions_27){
ENTRY;
loop:;
tracecall("fpclib.fp:111:  toplevel:options'");
save(x); // COND
X t3=lf[381]; // CONST
save(t3);
X t4=select1(TO_N(1),x); // SELECT (simple)
t3=restore();
X t2=(t3==t4)||eq1(t3,t4)?T:F; // EQ
X t1;
x=restore();
if(t2!=F){
X t5=select1(TO_N(2),x); // SELECT (simple)
save(t5); // COND
X t7=select1(TO_N(1),t5); // SELECT (simple)
save(t7); // COND
X t9=___id(t7); // REF: id
X t8;
t7=restore();
if(t9!=F){
X t10=F; // ICONST
t8=t10;
}else{
X t11=T; // ICONST
t8=t11;}
X t6;
t5=restore();
if(t8!=F){
X t12=lf[382]; // CONST
X t13=___quit(t12); // REF: quit
t6=t13;
}else{
save(t5); // COND
X t15=select1(TO_N(2),t5); // SELECT (simple)
save(t15); // COND
X t17=___id(t15); // REF: id
X t16;
t15=restore();
if(t17!=F){
X t18=F; // ICONST
t16=t18;
}else{
X t19=T; // ICONST
t16=t19;}
X t14;
t5=restore();
if(t16!=F){
save(t5);
X t20=allocate(4); // CONS
save(t20);
t5=pick(2);
X t21=select1(TO_N(1),t5); // SELECT (simple)
t20=top();
S_DATA(t20)[0]=t21;
t21=pick(2);
save(t21);
X t22=allocate(2); // CONS
save(t22);
t21=pick(2);
X t23=select1(TO_N(1),t21); // SELECT (simple)
t22=top();
S_DATA(t22)[0]=t23;
t23=pick(2);
X t24=lf[354]; // CONST
t22=top();
S_DATA(t22)[1]=t24;
t22=restore();
restore();
X t25=___toplevel_3areplace_5fext(t22); // REF: toplevel:replace_ext
t20=top();
S_DATA(t20)[1]=t25;
t25=pick(2);
X t26=select1(TO_N(3),t25); // SELECT (simple)
t20=top();
S_DATA(t20)[2]=t26;
t26=pick(2);
X t27=select1(TO_N(4),t26); // SELECT (simple)
t20=top();
S_DATA(t20)[3]=t27;
t20=restore();
restore();
t14=t20;
}else{
X t28=___id(t5); // REF: id
t14=t28;}
t6=t14;}
save(t6); // COND
save(t6);
X t30=allocate(2); // CONS
save(t30);
t6=pick(2);
X t31=lf[364]; // CONST
t30=top();
S_DATA(t30)[0]=t31;
t31=pick(2);
X t32=select1(TO_N(3),t31); // SELECT (simple)
t30=top();
S_DATA(t30)[1]=t32;
t30=restore();
restore();
X t33=___fetch(t30); // REF: fetch
X t29;
t6=restore();
if(t33!=F){
save(t6);
X t34=allocate(4); // CONS
save(t34);
t6=pick(2);
X t35=select1(TO_N(1),t6); // SELECT (simple)
t34=top();
S_DATA(t34)[0]=t35;
t35=pick(2);
save(t35);
X t36=allocate(2); // CONS
save(t36);
t35=pick(2);
X t37=select1(TO_N(2),t35); // SELECT (simple)
t36=top();
S_DATA(t36)[0]=t37;
t37=pick(2);
X t38=lf[354]; // CONST
t36=top();
S_DATA(t36)[1]=t38;
t36=restore();
restore();
X t39=___toplevel_3areplace_5fext(t36); // REF: toplevel:replace_ext
t34=top();
S_DATA(t34)[1]=t39;
t39=pick(2);
X t40=select1(TO_N(3),t39); // SELECT (simple)
t34=top();
S_DATA(t34)[2]=t40;
t40=pick(2);
save(t40);
X t41=allocate(2); // CONS
save(t41);
t40=pick(2);
X t42=select1(TO_N(4),t40); // SELECT (simple)
t41=top();
S_DATA(t41)[0]=t42;
t42=pick(2);
save(t42);
X t43=allocate(2); // CONS
save(t43);
t42=pick(2);
X t44=lf[369]; // CONST
t43=top();
S_DATA(t43)[0]=t44;
t44=pick(2);
X t45=select1(TO_N(2),t44); // SELECT (simple)
save(t45); // COND
X t48=lf[353]; // CONST
save(t48);
save(t45);
X t49=allocate(2); // CONS
save(t49);
t45=pick(2);
X t50=TO_N(2); // ICONST
t49=top();
S_DATA(t49)[0]=t50;
t50=pick(2);
X t51=___rev(t50); // REF: rev
t49=top();
S_DATA(t49)[1]=t51;
t49=restore();
restore();
X t52=___take(t49); // REF: take
t48=restore();
X t47=(t48==t52)||eq1(t48,t52)?T:F; // EQ
X t46;
t45=restore();
if(t47!=F){
save(t45);
X t53=allocate(2); // CONS
save(t53);
t45=pick(2);
save(t45);
X t55=___len(t45); // REF: len
t45=restore();
X t56=TO_N(2); // ICONST
X t54=___sub_5fop2(t55,t56); // OP2: sub
t53=top();
S_DATA(t53)[0]=t54;
t54=pick(2);
X t57=___id(t54); // REF: id
t53=top();
S_DATA(t53)[1]=t57;
t53=restore();
restore();
X t58=___take(t53); // REF: take
t46=t58;
}else{
X t59=___id(t45); // REF: id
t46=t59;}
t43=top();
S_DATA(t43)[1]=t46;
t43=restore();
restore();
t41=top();
S_DATA(t41)[1]=t43;
t41=restore();
restore();
X t60=___cat(t41); // REF: cat
t34=top();
S_DATA(t34)[3]=t60;
t34=restore();
restore();
t29=t34;
}else{
X t61=___id(t6); // REF: id
t29=t61;}
t1=t29;
}else{
save(x); // COND
X t64=lf[381]; // CONST
save(t64);
X t65=select1(TO_N(1),x); // SELECT (simple)
t64=restore();
X t63=(t64==t65)||eq1(t64,t65)?T:F; // EQ
X t62;
x=restore();
if(t63!=F){
X t66=lf[382]; // CONST
X t67=___quit(t66); // REF: quit
t62=t67;
}else{
save(x); // COND
X t70=lf[355]; // CONST
save(t70);
X t72=TO_N(1); // ICONST
save(t72);
X t73=select1(TO_N(1),x); // SELECT (simple)
t72=restore();
X t71=select1(t72,t73); // SELECT
t70=restore();
X t69=(t70==t71)||eq1(t70,t71)?T:F; // EQ
X t68;
x=restore();
if(t69!=F){
X t74=lf[382]; // CONST
X t75=___quit(t74); // REF: quit
t68=t75;
}else{
save(x); // COND
X t78=lf[356]; // CONST
save(t78);
X t80=TO_N(1); // ICONST
save(t80);
X t81=select1(TO_N(1),x); // SELECT (simple)
t80=restore();
X t79=select1(t80,t81); // SELECT
t78=restore();
X t77=(t78==t79)||eq1(t78,t79)?T:F; // EQ
X t76;
x=restore();
if(t77!=F){
save(x); // SEQ
X t82=lf[357]; // CONST
X t83=___cat(t82); // REF: cat
X t84=____5femit(t83); // REF: _emit
x=restore();
X t85=lf[358]; // CONST
X t86=____5fthrow(t85); // REF: _throw
t76=t86;
}else{
save(x); // COND
X t89=lf[359]; // CONST
save(t89);
X t91=TO_N(1); // ICONST
save(t91);
X t92=select1(TO_N(1),x); // SELECT (simple)
t91=restore();
X t90=select1(t91,t92); // SELECT
t89=restore();
X t88=(t89==t90)||eq1(t89,t90)?T:F; // EQ
X t87;
x=restore();
if(t88!=F){
save(x);
X t93=allocate(2); // CONS
save(t93);
x=pick(2);
X t94=select1(TO_N(1),x); // SELECT (simple)
X t95=___tl(t94); // REF: tl
t93=top();
S_DATA(t93)[0]=t95;
t95=pick(2);
save(t95);
X t96=allocate(2); // CONS
save(t96);
t95=pick(2);
X t97=lf[361]; // CONST
t96=top();
S_DATA(t96)[0]=t97;
t97=pick(2);
X t98=select1(TO_N(2),t97); // SELECT (simple)
t96=top();
S_DATA(t96)[1]=t98;
t96=restore();
restore();
X t99=___toplevel_3aflag(t96); // REF: toplevel:flag
t93=top();
S_DATA(t93)[1]=t99;
t93=restore();
restore();
x=t93; // REF: toplevel:options'
goto loop; // tail call: toplevel:options'
X t100;
t87=t100;
}else{
save(x); // COND
X t103=lf[360]; // CONST
save(t103);
X t105=TO_N(1); // ICONST
save(t105);
X t106=select1(TO_N(1),x); // SELECT (simple)
t105=restore();
X t104=select1(t105,t106); // SELECT
t103=restore();
X t102=(t103==t104)||eq1(t103,t104)?T:F; // EQ
X t101;
x=restore();
if(t102!=F){
save(x);
X t107=allocate(2); // CONS
save(t107);
x=pick(2);
X t108=select1(TO_N(1),x); // SELECT (simple)
X t109=___tl(t108); // REF: tl
t107=top();
S_DATA(t107)[0]=t109;
t109=pick(2);
save(t109);
X t110=allocate(2); // CONS
save(t110);
t109=pick(2);
X t111=lf[361]; // CONST
t110=top();
S_DATA(t110)[0]=t111;
t111=pick(2);
save(t111);
X t112=allocate(2); // CONS
save(t112);
t111=pick(2);
X t113=lf[362]; // CONST
t112=top();
S_DATA(t112)[0]=t113;
t113=pick(2);
X t114=select1(TO_N(2),t113); // SELECT (simple)
t112=top();
S_DATA(t112)[1]=t114;
t112=restore();
restore();
X t115=___toplevel_3aflag(t112); // REF: toplevel:flag
t110=top();
S_DATA(t110)[1]=t115;
t110=restore();
restore();
X t116=___toplevel_3aflag(t110); // REF: toplevel:flag
t107=top();
S_DATA(t107)[1]=t116;
t107=restore();
restore();
x=t107; // REF: toplevel:options'
goto loop; // tail call: toplevel:options'
X t117;
t101=t117;
}else{
save(x); // COND
X t120=lf[363]; // CONST
save(t120);
X t122=TO_N(1); // ICONST
save(t122);
X t123=select1(TO_N(1),x); // SELECT (simple)
t122=restore();
X t121=select1(t122,t123); // SELECT
t120=restore();
X t119=(t120==t121)||eq1(t120,t121)?T:F; // EQ
X t118;
x=restore();
if(t119!=F){
save(x);
X t124=allocate(2); // CONS
save(t124);
x=pick(2);
X t125=select1(TO_N(1),x); // SELECT (simple)
X t126=___tl(t125); // REF: tl
t124=top();
S_DATA(t124)[0]=t126;
t126=pick(2);
save(t126);
X t127=allocate(2); // CONS
save(t127);
t126=pick(2);
X t128=lf[364]; // CONST
t127=top();
S_DATA(t127)[0]=t128;
t128=pick(2);
X t129=select1(TO_N(2),t128); // SELECT (simple)
t127=top();
S_DATA(t127)[1]=t129;
t127=restore();
restore();
X t130=___toplevel_3aflag(t127); // REF: toplevel:flag
t124=top();
S_DATA(t124)[1]=t130;
t124=restore();
restore();
x=t124; // REF: toplevel:options'
goto loop; // tail call: toplevel:options'
X t131;
t118=t131;
}else{
save(x); // COND
X t134=lf[365]; // CONST
save(t134);
X t136=TO_N(1); // ICONST
save(t136);
X t137=select1(TO_N(1),x); // SELECT (simple)
t136=restore();
X t135=select1(t136,t137); // SELECT
t134=restore();
X t133=(t134==t135)||eq1(t134,t135)?T:F; // EQ
X t132;
x=restore();
if(t133!=F){
save(x);
X t138=allocate(2); // CONS
save(t138);
x=pick(2);
X t139=select1(TO_N(1),x); // SELECT (simple)
X t140=___tl(t139); // REF: tl
t138=top();
S_DATA(t138)[0]=t140;
t140=pick(2);
X t141=select1(TO_N(2),t140); // SELECT (simple)
t138=top();
S_DATA(t138)[1]=t141;
t138=restore();
restore();
save(t138); // COND
X t144=lf[381]; // CONST
save(t144);
X t145=select1(TO_N(1),t138); // SELECT (simple)
t144=restore();
X t143=(t144==t145)||eq1(t144,t145)?T:F; // EQ
X t142;
t138=restore();
if(t143!=F){
X t146=lf[382]; // CONST
X t147=___quit(t146); // REF: quit
t142=t147;
}else{
save(t138);
X t148=allocate(2); // CONS
save(t148);
t138=pick(2);
X t149=select1(TO_N(1),t138); // SELECT (simple)
X t150=___tl(t149); // REF: tl
t148=top();
S_DATA(t148)[0]=t150;
t150=pick(2);
save(t150);
X t151=allocate(3); // CONS
save(t151);
t150=pick(2);
X t152=lf[366]; // CONST
t151=top();
S_DATA(t151)[0]=t152;
t152=pick(2);
X t154=TO_N(1); // ICONST
save(t154);
X t155=select1(TO_N(1),t152); // SELECT (simple)
t154=restore();
X t153=select1(t154,t155); // SELECT
X t156=___toplevel_3aoption_5fvalue(t153); // REF: toplevel:option_value
t151=top();
S_DATA(t151)[1]=t156;
t156=pick(2);
X t157=select1(TO_N(2),t156); // SELECT (simple)
t151=top();
S_DATA(t151)[2]=t157;
t151=restore();
restore();
X t158=___toplevel_3aflag2(t151); // REF: toplevel:flag2
t148=top();
S_DATA(t148)[1]=t158;
t148=restore();
restore();
x=t148; // REF: toplevel:options'
goto loop; // tail call: toplevel:options'
X t159;
t142=t159;}
t132=t142;
}else{
save(x); // COND
X t162=lf[367]; // CONST
save(t162);
X t164=TO_N(1); // ICONST
save(t164);
X t165=select1(TO_N(1),x); // SELECT (simple)
t164=restore();
X t163=select1(t164,t165); // SELECT
t162=restore();
X t161=(t162==t163)||eq1(t162,t163)?T:F; // EQ
X t160;
x=restore();
if(t161!=F){
save(x);
X t166=allocate(2); // CONS
save(t166);
x=pick(2);
X t167=select1(TO_N(1),x); // SELECT (simple)
X t168=___tl(t167); // REF: tl
t166=top();
S_DATA(t166)[0]=t168;
t168=pick(2);
X t169=select1(TO_N(2),t168); // SELECT (simple)
t166=top();
S_DATA(t166)[1]=t169;
t166=restore();
restore();
save(t166); // COND
X t172=lf[381]; // CONST
save(t172);
X t173=select1(TO_N(1),t166); // SELECT (simple)
t172=restore();
X t171=(t172==t173)||eq1(t172,t173)?T:F; // EQ
X t170;
t166=restore();
if(t171!=F){
X t174=lf[382]; // CONST
X t175=___quit(t174); // REF: quit
t170=t175;
}else{
save(t166);
X t176=allocate(2); // CONS
save(t176);
t166=pick(2);
X t177=select1(TO_N(1),t166); // SELECT (simple)
X t178=___tl(t177); // REF: tl
t176=top();
S_DATA(t176)[0]=t178;
t178=pick(2);
save(t178);
X t179=allocate(4); // CONS
save(t179);
t178=pick(2);
X t181=TO_N(1); // ICONST
save(t181);
X t182=select1(TO_N(2),t178); // SELECT (simple)
t181=restore();
X t180=select1(t181,t182); // SELECT
t179=top();
S_DATA(t179)[0]=t180;
t180=pick(2);
X t184=TO_N(2); // ICONST
save(t184);
X t185=select1(TO_N(2),t180); // SELECT (simple)
t184=restore();
X t183=select1(t184,t185); // SELECT
t179=top();
S_DATA(t179)[1]=t183;
t183=pick(2);
X t187=TO_N(3); // ICONST
save(t187);
X t188=select1(TO_N(2),t183); // SELECT (simple)
t187=restore();
X t186=select1(t187,t188); // SELECT
t179=top();
S_DATA(t179)[2]=t186;
t186=pick(2);
save(t186);
X t189=allocate(2); // CONS
save(t189);
t186=pick(2);
X t191=TO_N(4); // ICONST
save(t191);
X t192=select1(TO_N(2),t186); // SELECT (simple)
t191=restore();
X t190=select1(t191,t192); // SELECT
t189=top();
S_DATA(t189)[0]=t190;
t190=pick(2);
save(t190);
X t193=allocate(2); // CONS
save(t193);
t190=pick(2);
X t194=lf[368]; // CONST
t193=top();
S_DATA(t193)[0]=t194;
t194=pick(2);
X t196=TO_N(1); // ICONST
save(t196);
X t197=select1(TO_N(1),t194); // SELECT (simple)
t196=restore();
X t195=select1(t196,t197); // SELECT
X t198=___toplevel_3aoption_5fvalue(t195); // REF: toplevel:option_value
X t199=___tos(t198); // REF: tos
t193=top();
S_DATA(t193)[1]=t199;
t193=restore();
restore();
X t200=___cat(t193); // REF: cat
t189=top();
S_DATA(t189)[1]=t200;
t189=restore();
restore();
X t201=___ar(t189); // REF: ar
t179=top();
S_DATA(t179)[3]=t201;
t179=restore();
restore();
t176=top();
S_DATA(t176)[1]=t179;
t176=restore();
restore();
x=t176; // REF: toplevel:options'
goto loop; // tail call: toplevel:options'
X t202;
t170=t202;}
t160=t170;
}else{
save(x); // COND
X t205=lf[369]; // CONST
save(t205);
X t207=TO_N(1); // ICONST
save(t207);
X t208=select1(TO_N(1),x); // SELECT (simple)
t207=restore();
X t206=select1(t207,t208); // SELECT
t205=restore();
X t204=(t205==t206)||eq1(t205,t206)?T:F; // EQ
X t203;
x=restore();
if(t204!=F){
save(x);
X t209=allocate(2); // CONS
save(t209);
x=pick(2);
X t210=select1(TO_N(1),x); // SELECT (simple)
X t211=___tl(t210); // REF: tl
t209=top();
S_DATA(t209)[0]=t211;
t211=pick(2);
X t212=select1(TO_N(2),t211); // SELECT (simple)
t209=top();
S_DATA(t209)[1]=t212;
t209=restore();
restore();
save(t209); // COND
X t215=lf[381]; // CONST
save(t215);
X t216=select1(TO_N(1),t209); // SELECT (simple)
t215=restore();
X t214=(t215==t216)||eq1(t215,t216)?T:F; // EQ
X t213;
t209=restore();
if(t214!=F){
X t217=lf[382]; // CONST
X t218=___quit(t217); // REF: quit
t213=t218;
}else{
save(t209);
X t219=allocate(2); // CONS
save(t219);
t209=pick(2);
X t220=select1(TO_N(1),t209); // SELECT (simple)
X t221=___tl(t220); // REF: tl
t219=top();
S_DATA(t219)[0]=t221;
t221=pick(2);
save(t221);
X t222=allocate(4); // CONS
save(t222);
t221=pick(2);
X t224=TO_N(1); // ICONST
save(t224);
X t225=select1(TO_N(2),t221); // SELECT (simple)
t224=restore();
X t223=select1(t224,t225); // SELECT
t222=top();
S_DATA(t222)[0]=t223;
t223=pick(2);
X t227=TO_N(1); // ICONST
save(t227);
X t228=select1(TO_N(1),t223); // SELECT (simple)
t227=restore();
X t226=select1(t227,t228); // SELECT
t222=top();
S_DATA(t222)[1]=t226;
t226=pick(2);
X t230=TO_N(3); // ICONST
save(t230);
X t231=select1(TO_N(2),t226); // SELECT (simple)
t230=restore();
X t229=select1(t230,t231); // SELECT
t222=top();
S_DATA(t222)[2]=t229;
t229=pick(2);
X t233=TO_N(4); // ICONST
save(t233);
X t234=select1(TO_N(2),t229); // SELECT (simple)
t233=restore();
X t232=select1(t233,t234); // SELECT
t222=top();
S_DATA(t222)[3]=t232;
t222=restore();
restore();
t219=top();
S_DATA(t219)[1]=t222;
t219=restore();
restore();
x=t219; // REF: toplevel:options'
goto loop; // tail call: toplevel:options'
X t235;
t213=t235;}
t203=t213;
}else{
save(x); // COND
X t238=lf[370]; // CONST
save(t238);
X t240=TO_N(1); // ICONST
save(t240);
X t241=select1(TO_N(1),x); // SELECT (simple)
t240=restore();
X t239=select1(t240,t241); // SELECT
t238=restore();
X t237=(t238==t239)||eq1(t238,t239)?T:F; // EQ
X t236;
x=restore();
if(t237!=F){
save(x);
X t242=allocate(2); // CONS
save(t242);
x=pick(2);
X t243=select1(TO_N(1),x); // SELECT (simple)
X t244=___tl(t243); // REF: tl
t242=top();
S_DATA(t242)[0]=t244;
t244=pick(2);
X t245=select1(TO_N(2),t244); // SELECT (simple)
t242=top();
S_DATA(t242)[1]=t245;
t242=restore();
restore();
save(t242); // COND
X t248=lf[381]; // CONST
save(t248);
X t249=select1(TO_N(1),t242); // SELECT (simple)
t248=restore();
X t247=(t248==t249)||eq1(t248,t249)?T:F; // EQ
X t246;
t242=restore();
if(t247!=F){
X t250=lf[382]; // CONST
X t251=___quit(t250); // REF: quit
t246=t251;
}else{
save(t242);
X t252=allocate(2); // CONS
save(t252);
t242=pick(2);
X t253=select1(TO_N(1),t242); // SELECT (simple)
X t254=___tl(t253); // REF: tl
t252=top();
S_DATA(t252)[0]=t254;
t254=pick(2);
save(t254);
X t255=allocate(3); // CONS
save(t255);
t254=pick(2);
X t256=lf[371]; // CONST
t255=top();
S_DATA(t255)[0]=t256;
t256=pick(2);
X t258=TO_N(1); // ICONST
save(t258);
X t259=select1(TO_N(1),t256); // SELECT (simple)
t258=restore();
X t257=select1(t258,t259); // SELECT
t255=top();
S_DATA(t255)[1]=t257;
t257=pick(2);
X t260=select1(TO_N(2),t257); // SELECT (simple)
t255=top();
S_DATA(t255)[2]=t260;
t255=restore();
restore();
X t261=___toplevel_3aflag2(t255); // REF: toplevel:flag2
t252=top();
S_DATA(t252)[1]=t261;
t252=restore();
restore();
x=t252; // REF: toplevel:options'
goto loop; // tail call: toplevel:options'
X t262;
t246=t262;}
t236=t246;
}else{
save(x); // COND
X t265=lf[372]; // CONST
save(t265);
X t267=TO_N(1); // ICONST
save(t267);
X t268=select1(TO_N(1),x); // SELECT (simple)
t267=restore();
X t266=select1(t267,t268); // SELECT
t265=restore();
X t264=(t265==t266)||eq1(t265,t266)?T:F; // EQ
X t263;
x=restore();
if(t264!=F){
save(x);
X t269=allocate(2); // CONS
save(t269);
x=pick(2);
X t270=select1(TO_N(1),x); // SELECT (simple)
X t271=___tl(t270); // REF: tl
t269=top();
S_DATA(t269)[0]=t271;
t271=pick(2);
X t272=select1(TO_N(2),t271); // SELECT (simple)
t269=top();
S_DATA(t269)[1]=t272;
t269=restore();
restore();
save(t269); // COND
X t275=lf[381]; // CONST
save(t275);
X t276=select1(TO_N(1),t269); // SELECT (simple)
t275=restore();
X t274=(t275==t276)||eq1(t275,t276)?T:F; // EQ
X t273;
t269=restore();
if(t274!=F){
X t277=lf[382]; // CONST
X t278=___quit(t277); // REF: quit
t273=t278;
}else{
save(t269);
X t279=allocate(2); // CONS
save(t279);
t269=pick(2);
X t280=select1(TO_N(1),t269); // SELECT (simple)
X t281=___tl(t280); // REF: tl
t279=top();
S_DATA(t279)[0]=t281;
t281=pick(2);
save(t281);
X t282=allocate(3); // CONS
save(t282);
t281=pick(2);
X t283=lf[373]; // CONST
t282=top();
S_DATA(t282)[0]=t283;
t283=pick(2);
X t285=TO_N(1); // ICONST
save(t285);
X t286=select1(TO_N(1),t283); // SELECT (simple)
t285=restore();
X t284=select1(t285,t286); // SELECT
t282=top();
S_DATA(t282)[1]=t284;
t284=pick(2);
X t287=select1(TO_N(2),t284); // SELECT (simple)
t282=top();
S_DATA(t282)[2]=t287;
t282=restore();
restore();
X t288=___toplevel_3aflag2(t282); // REF: toplevel:flag2
t279=top();
S_DATA(t279)[1]=t288;
t279=restore();
restore();
x=t279; // REF: toplevel:options'
goto loop; // tail call: toplevel:options'
X t289;
t273=t289;}
t263=t273;
}else{
save(x); // COND
X t292=lf[374]; // CONST
save(t292);
X t294=TO_N(1); // ICONST
save(t294);
X t295=select1(TO_N(1),x); // SELECT (simple)
t294=restore();
X t293=select1(t294,t295); // SELECT
t292=restore();
X t291=(t292==t293)||eq1(t292,t293)?T:F; // EQ
X t290;
x=restore();
if(t291!=F){
save(x);
X t296=allocate(2); // CONS
save(t296);
x=pick(2);
X t297=select1(TO_N(1),x); // SELECT (simple)
X t298=___tl(t297); // REF: tl
t296=top();
S_DATA(t296)[0]=t298;
t298=pick(2);
save(t298);
X t299=allocate(2); // CONS
save(t299);
t298=pick(2);
X t300=lf[375]; // CONST
t299=top();
S_DATA(t299)[0]=t300;
t300=pick(2);
X t301=select1(TO_N(2),t300); // SELECT (simple)
t299=top();
S_DATA(t299)[1]=t301;
t299=restore();
restore();
X t302=___toplevel_3aflag(t299); // REF: toplevel:flag
t296=top();
S_DATA(t296)[1]=t302;
t296=restore();
restore();
x=t296; // REF: toplevel:options'
goto loop; // tail call: toplevel:options'
X t303;
t290=t303;
}else{
save(x); // COND
X t306=lf[376]; // CONST
save(t306);
X t308=TO_N(1); // ICONST
save(t308);
X t309=select1(TO_N(1),x); // SELECT (simple)
t308=restore();
X t307=select1(t308,t309); // SELECT
t306=restore();
X t305=(t306==t307)||eq1(t306,t307)?T:F; // EQ
X t304;
x=restore();
if(t305!=F){
save(x);
X t310=allocate(2); // CONS
save(t310);
x=pick(2);
X t311=select1(TO_N(1),x); // SELECT (simple)
X t312=___tl(t311); // REF: tl
t310=top();
S_DATA(t310)[0]=t312;
t312=pick(2);
save(t312);
X t313=allocate(2); // CONS
save(t313);
t312=pick(2);
X t314=lf[377]; // CONST
t313=top();
S_DATA(t313)[0]=t314;
t314=pick(2);
X t315=select1(TO_N(2),t314); // SELECT (simple)
t313=top();
S_DATA(t313)[1]=t315;
t313=restore();
restore();
X t316=___toplevel_3aflag(t313); // REF: toplevel:flag
t310=top();
S_DATA(t310)[1]=t316;
t310=restore();
restore();
x=t310; // REF: toplevel:options'
goto loop; // tail call: toplevel:options'
X t317;
t304=t317;
}else{
save(x); // COND
X t320=lf[378]; // CONST
save(t320);
X t322=TO_N(1); // ICONST
save(t322);
X t323=select1(TO_N(1),x); // SELECT (simple)
t322=restore();
X t321=select1(t322,t323); // SELECT
t320=restore();
X t319=(t320==t321)||eq1(t320,t321)?T:F; // EQ
X t318;
x=restore();
if(t319!=F){
save(x);
X t324=allocate(2); // CONS
save(t324);
x=pick(2);
X t325=select1(TO_N(1),x); // SELECT (simple)
X t326=___tl(t325); // REF: tl
t324=top();
S_DATA(t324)[0]=t326;
t326=pick(2);
save(t326);
X t327=allocate(2); // CONS
save(t327);
t326=pick(2);
X t328=lf[379]; // CONST
t327=top();
S_DATA(t327)[0]=t328;
t328=pick(2);
X t329=select1(TO_N(2),t328); // SELECT (simple)
t327=top();
S_DATA(t327)[1]=t329;
t327=restore();
restore();
X t330=___toplevel_3aflag(t327); // REF: toplevel:flag
t324=top();
S_DATA(t324)[1]=t330;
t324=restore();
restore();
x=t324; // REF: toplevel:options'
goto loop; // tail call: toplevel:options'
X t331;
t318=t331;
}else{
save(x); // COND
X t334=lf[380]; // CONST
save(t334);
X t336=TO_N(1); // ICONST
save(t336);
X t337=select1(TO_N(1),x); // SELECT (simple)
t336=restore();
X t335=select1(t336,t337); // SELECT
t334=restore();
X t333=(t334==t335)||eq1(t334,t335)?T:F; // EQ
X t332;
x=restore();
if(t333!=F){
save(x);
X t338=allocate(2); // CONS
save(t338);
x=pick(2);
X t339=select1(TO_N(1),x); // SELECT (simple)
X t340=___tl(t339); // REF: tl
t338=top();
S_DATA(t338)[0]=t340;
t340=pick(2);
X t341=select1(TO_N(2),t340); // SELECT (simple)
t338=top();
S_DATA(t338)[1]=t341;
t338=restore();
restore();
save(t338); // COND
X t344=lf[381]; // CONST
save(t344);
X t345=select1(TO_N(1),t338); // SELECT (simple)
t344=restore();
X t343=(t344==t345)||eq1(t344,t345)?T:F; // EQ
X t342;
t338=restore();
if(t343!=F){
X t346=lf[382]; // CONST
X t347=___quit(t346); // REF: quit
t342=t347;
}else{
save(t338);
X t348=allocate(2); // CONS
save(t348);
t338=pick(2);
X t349=select1(TO_N(1),t338); // SELECT (simple)
X t350=___tl(t349); // REF: tl
t348=top();
S_DATA(t348)[0]=t350;
t350=pick(2);
save(t350);
X t351=allocate(3); // CONS
save(t351);
t350=pick(2);
X t352=lf[383]; // CONST
t351=top();
S_DATA(t351)[0]=t352;
t352=pick(2);
X t354=TO_N(1); // ICONST
save(t354);
X t355=select1(TO_N(1),t352); // SELECT (simple)
t354=restore();
X t353=select1(t354,t355); // SELECT
X t356=___toa(t353); // REF: toa
t351=top();
S_DATA(t351)[1]=t356;
t356=pick(2);
X t357=select1(TO_N(2),t356); // SELECT (simple)
t351=top();
S_DATA(t351)[2]=t357;
t351=restore();
restore();
X t358=___toplevel_3aflag2(t351); // REF: toplevel:flag2
t348=top();
S_DATA(t348)[1]=t358;
t348=restore();
restore();
x=t348; // REF: toplevel:options'
goto loop; // tail call: toplevel:options'
X t359;
t342=t359;}
t332=t342;
}else{
save(x); // COND
X t362=TO_N(45); // ICONST
save(t362);
X t364=TO_N(1); // ICONST
save(t364);
X t366=TO_N(1); // ICONST
save(t366);
X t367=select1(TO_N(1),x); // SELECT (simple)
t366=restore();
X t365=select1(t366,t367); // SELECT
t364=restore();
X t363=select1(t364,t365); // SELECT
t362=restore();
X t361=(t362==t363)||eq1(t362,t363)?T:F; // EQ
X t360;
x=restore();
if(t361!=F){
save(x);
X t368=allocate(2); // CONS
save(t368);
x=pick(2);
X t369=select1(TO_N(1),x); // SELECT (simple)
X t370=___tl(t369); // REF: tl
t368=top();
S_DATA(t368)[0]=t370;
t370=pick(2);
save(t370);
X t371=allocate(4); // CONS
save(t371);
t370=pick(2);
X t373=TO_N(1); // ICONST
save(t373);
X t374=select1(TO_N(2),t370); // SELECT (simple)
t373=restore();
X t372=select1(t373,t374); // SELECT
t371=top();
S_DATA(t371)[0]=t372;
t372=pick(2);
X t376=TO_N(2); // ICONST
save(t376);
X t377=select1(TO_N(2),t372); // SELECT (simple)
t376=restore();
X t375=select1(t376,t377); // SELECT
t371=top();
S_DATA(t371)[1]=t375;
t375=pick(2);
X t379=TO_N(3); // ICONST
save(t379);
X t380=select1(TO_N(2),t375); // SELECT (simple)
t379=restore();
X t378=select1(t379,t380); // SELECT
t371=top();
S_DATA(t371)[2]=t378;
t378=pick(2);
save(t378);
X t381=allocate(2); // CONS
save(t381);
t378=pick(2);
X t383=TO_N(4); // ICONST
save(t383);
X t384=select1(TO_N(2),t378); // SELECT (simple)
t383=restore();
X t382=select1(t383,t384); // SELECT
t381=top();
S_DATA(t381)[0]=t382;
t382=pick(2);
X t386=TO_N(1); // ICONST
save(t386);
X t387=select1(TO_N(1),t382); // SELECT (simple)
t386=restore();
X t385=select1(t386,t387); // SELECT
t381=top();
S_DATA(t381)[1]=t385;
t381=restore();
restore();
X t388=___ar(t381); // REF: ar
t371=top();
S_DATA(t371)[3]=t388;
t371=restore();
restore();
t368=top();
S_DATA(t368)[1]=t371;
t368=restore();
restore();
t360=t368;
}else{
save(x);
X t389=allocate(2); // CONS
save(t389);
x=pick(2);
X t390=select1(TO_N(1),x); // SELECT (simple)
X t391=___tl(t390); // REF: tl
t389=top();
S_DATA(t389)[0]=t391;
t391=pick(2);
save(t391);
X t392=allocate(4); // CONS
save(t392);
t391=pick(2);
X t394=TO_N(1); // ICONST
save(t394);
X t395=select1(TO_N(1),t391); // SELECT (simple)
t394=restore();
X t393=select1(t394,t395); // SELECT
t392=top();
S_DATA(t392)[0]=t393;
t393=pick(2);
X t397=TO_N(2); // ICONST
save(t397);
X t398=select1(TO_N(2),t393); // SELECT (simple)
t397=restore();
X t396=select1(t397,t398); // SELECT
t392=top();
S_DATA(t392)[1]=t396;
t396=pick(2);
X t400=TO_N(3); // ICONST
save(t400);
X t401=select1(TO_N(2),t396); // SELECT (simple)
t400=restore();
X t399=select1(t400,t401); // SELECT
t392=top();
S_DATA(t392)[2]=t399;
t399=pick(2);
X t403=TO_N(4); // ICONST
save(t403);
X t404=select1(TO_N(2),t399); // SELECT (simple)
t403=restore();
X t402=select1(t403,t404); // SELECT
t392=top();
S_DATA(t392)[3]=t402;
t392=restore();
restore();
t389=top();
S_DATA(t389)[1]=t392;
t389=restore();
restore();
t360=t389;}
x=t360; // REF: toplevel:options'
goto loop; // tail call: toplevel:options'
X t405;
t332=t405;}
t318=t332;}
t304=t318;}
t290=t304;}
t263=t290;}
t236=t263;}
t203=t236;}
t160=t203;}
t132=t160;}
t118=t132;}
t101=t118;}
t87=t101;}
t76=t87;}
t68=t76;}
t62=t68;}
t1=t62;}
RETURN(t1);}
//---------------------------------------- toplevel:replace_ext (fpclib.fp:140)
DEFINE(___toplevel_3areplace_5fext){
ENTRY;
loop:;
tracecall("fpclib.fp:140:  toplevel:replace_ext");
save(x);
X t1=allocate(3); // CONS
save(t1);
x=pick(2);
X t2=select1(TO_N(1),x); // SELECT (simple)
t1=top();
S_DATA(t1)[0]=t2;
t2=pick(2);
X t3=select1(TO_N(2),t2); // SELECT (simple)
t1=top();
S_DATA(t1)[1]=t3;
t3=pick(2);
save(t3);
X t4=allocate(2); // CONS
save(t4);
t3=pick(2);
X t5=TO_N(46); // ICONST
t4=top();
S_DATA(t4)[0]=t5;
t5=pick(2);
X t6=select1(TO_N(1),t5); // SELECT (simple)
t4=top();
S_DATA(t4)[1]=t6;
t4=restore();
restore();
save(t4);
X t7=allocate(3); // CONS
save(t7);
t4=pick(2);
X t8=select1(TO_N(1),t4); // SELECT (simple)
t7=top();
S_DATA(t7)[0]=t8;
t8=pick(2);
X t9=select1(TO_N(2),t8); // SELECT (simple)
t7=top();
S_DATA(t7)[1]=t9;
t9=pick(2);
X t10=select1(TO_N(2),t9); // SELECT (simple)
X t11=___len(t10); // REF: len
t7=top();
S_DATA(t7)[2]=t11;
t7=restore();
restore();
X t12=___indexr_27(t7); // REF: indexr'
t1=top();
S_DATA(t1)[2]=t12;
t1=restore();
restore();
save(t1); // COND
X t14=select1(TO_N(3),t1); // SELECT (simple)
X t13;
t1=restore();
if(t14!=F){
save(t1);
X t15=allocate(2); // CONS
save(t15);
t1=pick(2);
save(t1);
X t16=allocate(2); // CONS
save(t16);
t1=pick(2);
X t17=select1(TO_N(3),t1); // SELECT (simple)
t16=top();
S_DATA(t16)[0]=t17;
t17=pick(2);
X t18=select1(TO_N(1),t17); // SELECT (simple)
t16=top();
S_DATA(t16)[1]=t18;
t16=restore();
restore();
X t19=___take(t16); // REF: take
t15=top();
S_DATA(t15)[0]=t19;
t19=pick(2);
X t20=select1(TO_N(2),t19); // SELECT (simple)
t15=top();
S_DATA(t15)[1]=t20;
t15=restore();
restore();
t13=t15;
}else{
save(t1);
X t21=allocate(3); // CONS
save(t21);
t1=pick(2);
X t22=select1(TO_N(1),t1); // SELECT (simple)
t21=top();
S_DATA(t21)[0]=t22;
t22=pick(2);
X t23=lf[384]; // CONST
t21=top();
S_DATA(t21)[1]=t23;
t23=pick(2);
X t24=select1(TO_N(2),t23); // SELECT (simple)
t21=top();
S_DATA(t21)[2]=t24;
t21=restore();
restore();
t13=t21;}
X t25=___cat(t13); // REF: cat
RETURN(t25);}
//---------------------------------------- toplevel:flag (fpclib.fp:146)
DEFINE(___toplevel_3aflag){
ENTRY;
loop:;
tracecall("fpclib.fp:146:  toplevel:flag");
save(x);
X t1=allocate(4); // CONS
save(t1);
x=pick(2);
X t3=TO_N(1); // ICONST
save(t3);
X t4=select1(TO_N(2),x); // SELECT (simple)
t3=restore();
X t2=select1(t3,t4); // SELECT
t1=top();
S_DATA(t1)[0]=t2;
t2=pick(2);
X t6=TO_N(2); // ICONST
save(t6);
X t7=select1(TO_N(2),t2); // SELECT (simple)
t6=restore();
X t5=select1(t6,t7); // SELECT
t1=top();
S_DATA(t1)[1]=t5;
t5=pick(2);
save(t5);
X t8=allocate(2); // CONS
save(t8);
t5=pick(2);
X t10=TO_N(3); // ICONST
save(t10);
X t11=select1(TO_N(2),t5); // SELECT (simple)
t10=restore();
X t9=select1(t10,t11); // SELECT
t8=top();
S_DATA(t8)[0]=t9;
t9=pick(2);
save(t9);
X t12=allocate(2); // CONS
save(t12);
t9=pick(2);
X t13=select1(TO_N(1),t9); // SELECT (simple)
t12=top();
S_DATA(t12)[0]=t13;
t13=pick(2);
X t14=T; // ICONST
t12=top();
S_DATA(t12)[1]=t14;
t12=restore();
restore();
t8=top();
S_DATA(t8)[1]=t12;
t8=restore();
restore();
X t15=___ar(t8); // REF: ar
t1=top();
S_DATA(t1)[2]=t15;
t15=pick(2);
X t17=TO_N(4); // ICONST
save(t17);
X t18=select1(TO_N(2),t15); // SELECT (simple)
t17=restore();
X t16=select1(t17,t18); // SELECT
t1=top();
S_DATA(t1)[3]=t16;
t1=restore();
restore();
RETURN(t1);}
//---------------------------------------- toplevel:flag2 (fpclib.fp:147)
DEFINE(___toplevel_3aflag2){
ENTRY;
loop:;
tracecall("fpclib.fp:147:  toplevel:flag2");
save(x);
X t1=allocate(4); // CONS
save(t1);
x=pick(2);
X t3=TO_N(1); // ICONST
save(t3);
X t4=select1(TO_N(3),x); // SELECT (simple)
t3=restore();
X t2=select1(t3,t4); // SELECT
t1=top();
S_DATA(t1)[0]=t2;
t2=pick(2);
X t6=TO_N(2); // ICONST
save(t6);
X t7=select1(TO_N(3),t2); // SELECT (simple)
t6=restore();
X t5=select1(t6,t7); // SELECT
t1=top();
S_DATA(t1)[1]=t5;
t5=pick(2);
save(t5);
X t8=allocate(2); // CONS
save(t8);
t5=pick(2);
X t10=TO_N(3); // ICONST
save(t10);
X t11=select1(TO_N(3),t5); // SELECT (simple)
t10=restore();
X t9=select1(t10,t11); // SELECT
t8=top();
S_DATA(t8)[0]=t9;
t9=pick(2);
save(t9);
X t12=allocate(2); // CONS
save(t12);
t9=pick(2);
X t13=select1(TO_N(1),t9); // SELECT (simple)
t12=top();
S_DATA(t12)[0]=t13;
t13=pick(2);
X t14=select1(TO_N(2),t13); // SELECT (simple)
t12=top();
S_DATA(t12)[1]=t14;
t12=restore();
restore();
t8=top();
S_DATA(t8)[1]=t12;
t8=restore();
restore();
X t15=___ar(t8); // REF: ar
t1=top();
S_DATA(t1)[2]=t15;
t15=pick(2);
X t17=TO_N(4); // ICONST
save(t17);
X t18=select1(TO_N(3),t15); // SELECT (simple)
t17=restore();
X t16=select1(t17,t18); // SELECT
t1=top();
S_DATA(t1)[3]=t16;
t1=restore();
restore();
RETURN(t1);}
//---------------------------------------- toplevel:option_value (fpclib.fp:166)
DEFINE(___toplevel_3aoption_5fvalue){
ENTRY;
loop:;
tracecall("fpclib.fp:166:  toplevel:option_value");
save(x);
X t1=allocate(2); // CONS
save(t1);
x=pick(2);
X t2=___id(x); // REF: id
t1=top();
S_DATA(t1)[0]=t2;
t2=pick(2);
save(t2); // COND
X t5=TO_N(0); // ICONST
save(t5);
X t6=___len(t2); // REF: len
t5=restore();
X t4=(t5==t6)||eq1(t5,t6)?T:F; // EQ
save(t4); // COND
X t8=___id(t4); // REF: id
X t7;
t4=restore();
if(t8!=F){
X t9=F; // ICONST
t7=t9;
}else{
X t10=T; // ICONST
t7=t10;}
X t3;
t2=restore();
if(t7!=F){
save(t2);
X t12=___len(t2); // REF: len
t2=restore();
save(t12);
X t13=___id(t2); // REF: id
t12=restore();
X t11=select1(t12,t13); // SELECT
t3=t11;
}else{
X t14=F; // ICONST
t3=t14;}
t1=top();
S_DATA(t1)[1]=t3;
t1=restore();
restore();
save(t1); // COND
save(t1);
X t16=allocate(2); // CONS
save(t16);
t1=pick(2);
X t17=select1(TO_N(2),t1); // SELECT (simple)
t16=top();
S_DATA(t16)[0]=t17;
t17=pick(2);
X t18=lf[385]; // CONST
t16=top();
S_DATA(t16)[1]=t18;
t16=restore();
restore();
X t19=___index(t16); // REF: index
X t15;
t1=restore();
if(t19!=F){
X t21=TO_N(1024); // ICONST
save(t21);
X t22=select1(TO_N(1),t1); // SELECT (simple)
X t23=___tlr(t22); // REF: tlr
X t24=___ton(t23); // REF: ton
t21=restore();
X t20=___mul_5fop2(t21,t24); // OP2: mul
t15=t20;
}else{
save(t1); // COND
save(t1);
X t26=allocate(2); // CONS
save(t26);
t1=pick(2);
X t27=select1(TO_N(2),t1); // SELECT (simple)
t26=top();
S_DATA(t26)[0]=t27;
t27=pick(2);
X t28=lf[386]; // CONST
t26=top();
S_DATA(t26)[1]=t28;
t26=restore();
restore();
X t29=___index(t26); // REF: index
X t25;
t1=restore();
if(t29!=F){
X t31=TO_N(1048576); // ICONST
save(t31);
X t32=select1(TO_N(1),t1); // SELECT (simple)
X t33=___tlr(t32); // REF: tlr
X t34=___ton(t33); // REF: ton
t31=restore();
X t30=___mul_5fop2(t31,t34); // OP2: mul
t25=t30;
}else{
save(t1); // COND
save(t1);
X t36=allocate(2); // CONS
save(t36);
t1=pick(2);
X t37=select1(TO_N(2),t1); // SELECT (simple)
t36=top();
S_DATA(t36)[0]=t37;
t37=pick(2);
X t38=lf[387]; // CONST
t36=top();
S_DATA(t36)[1]=t38;
t36=restore();
restore();
X t39=___index(t36); // REF: index
X t35;
t1=restore();
if(t39!=F){
X t41=TO_N(1073741824); // ICONST
save(t41);
X t42=select1(TO_N(1),t1); // SELECT (simple)
X t43=___tlr(t42); // REF: tlr
X t44=___ton(t43); // REF: ton
t41=restore();
X t40=___mul_5fop2(t41,t44); // OP2: mul
t35=t40;
}else{
X t45=select1(TO_N(1),t1); // SELECT (simple)
X t46=___ton(t45); // REF: ton
t35=t46;}
t25=t35;}
t15=t25;}
RETURN(t15);}
int main(int argc,char *argv[]){
return init(argc,argv,____5fstart,gen_lf);}
/* END OF FILE */
