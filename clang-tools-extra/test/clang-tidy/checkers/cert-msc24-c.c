// RUN: %check_clang_tidy -check-suffix=WITH-ANNEX-K    %s cert-msc24-c %t -- -- -D__STDC_LIB_EXT1__=1 -D__STDC_WANT_LIB_EXT1__=1
// RUN: %check_clang_tidy -check-suffix=WITHOUT-ANNEX-K %s cert-msc24-c %t -- -- -U__STDC_LIB_EXT1__   -U__STDC_WANT_LIB_EXT1__
// RUN: %check_clang_tidy -check-suffix=WITHOUT-ANNEX-K %s cert-msc24-c %t -- -- -D__STDC_LIB_EXT1__=1 -U__STDC_WANT_LIB_EXT1__
// RUN: %check_clang_tidy -check-suffix=WITHOUT-ANNEX-K %s cert-msc24-c %t -- -- -U__STDC_LIB_EXT1__   -D__STDC_WANT_LIB_EXT1__=1

typedef void *FILE;
char *gets(char *s);
void rewind(FILE *stream);
void setbuf(FILE *stream, char *buf);

void f1(char *s, FILE *f) {
  gets(s);
  // CHECK-MESSAGES-WITH-ANNEX-K:    :[[@LINE-1]]:3: warning: function 'gets' is deprecated as of C99, removed from C11.
  // CHECK-MESSAGES-WITHOUT-ANNEX-K: :[[@LINE-2]]:3: warning: function 'gets' is deprecated as of C99, removed from C11.

  rewind(f);
  // CHECK-MESSAGES-WITH-ANNEX-K:    :[[@LINE-1]]:3: warning: function 'rewind' has no error detection; 'fseek' should be used instead.
  // CHECK-MESSAGES-WITHOUT-ANNEX-K: :[[@LINE-2]]:3: warning: function 'rewind' has no error detection; 'fseek' should be used instead.

  setbuf(f, s);
  // CHECK-MESSAGES-WITH-ANNEX-K:    :[[@LINE-1]]:3: warning: function 'setbuf' has no error detection; 'setvbuf' should be used instead.
  // CHECK-MESSAGES-WITHOUT-ANNEX-K: :[[@LINE-2]]:3: warning: function 'setbuf' has no error detection; 'setvbuf' should be used instead.
}

struct tm;
char *asctime(const struct tm *timeptr);

void f2(const struct tm *timeptr) {
  asctime(timeptr);
  // CHECK-MESSAGES-WITH-ANNEX-K: :[[@LINE-1]]:3: warning: function 'asctime' is non-reentrant; 'asctime_s' should be used instead.
  // no-warning WITHOUT-ANNEX-K

  char *(*f_ptr1)(const struct tm *) = asctime;
  // CHECK-MESSAGES-WITH-ANNEX-K: :[[@LINE-1]]:40: warning: function 'asctime' is non-reentrant; 'asctime_s' should be used instead.
  // no-warning WITHOUT-ANNEX-K

  char *(*f_ptr2)(const struct tm *) = &asctime;
  // CHECK-MESSAGES-WITH-ANNEX-K: :[[@LINE-1]]:41: warning: function 'asctime' is non-reentrant; 'asctime_s' should be used instead.
  // no-warning WITHOUT-ANNEX-K
}

FILE *fopen(const char *filename, const char *mode);
FILE *freopen(const char *filename, const char *mode, FILE *stream);
int fscanf(FILE *stream, const char *format, ...);

void f3(char *s, FILE *f) {
  fopen(s, s);
  // CHECK-MESSAGES-WITH-ANNEX-K: :[[@LINE-1]]:3: warning: function 'fopen' has no exclusive access to file; 'fopen_s' should be used instead.
  // no-warning WITHOUT-ANNEX-K

  freopen(s, s, f);
  // CHECK-MESSAGES-WITH-ANNEX-K: :[[@LINE-1]]:3: warning: function 'freopen' has no exclusive access to file; 'freopen_s' should be used instead.
  // no-warning WITHOUT-ANNEX-K

  int i;
  fscanf(f, "%d", &i);
  // CHECK-MESSAGES-WITH-ANNEX-K: :[[@LINE-1]]:3: warning: function 'fscanf' is obsolescent; 'fscanf_s' should be used instead.
  // no-warning WITHOUT-ANNEX-K
}

typedef int time_t;
char *ctime(const time_t *timer);

void f4(const time_t *timer) {
  ctime(timer);
  // CHECK-MESSAGES-WITH-ANNEX-K: :[[@LINE-1]]:3: warning: function 'ctime' is non-reentrant; 'ctime_s' should be used instead.
  // no-warning WITHOUT-ANNEX-K
}

typedef int errno_t;
typedef size_t rsize_t;
errno_t asctime_s(char *s, rsize_t maxsize, const struct tm *timeptr);
errno_t strcat_s(char *s1, rsize_t s1max, const char *s2);
int fseek(FILE *stream, long int offset, int whence);
int setvbuf(FILE *stream, char *buf, int mode, size_t size);

void fUsingSafeFunctions(const struct tm *timeptr, FILE *f) {
  const size_t BUFFSIZE = 32;
  char buf[BUFFSIZE] = {0};

  // no-warning, safe function from annex K is used
  if (asctime_s(buf, BUFFSIZE, timeptr) != 0)
    return;

  // no-warning, safe function from annex K is used
  if (strcat_s(buf, BUFFSIZE, "something") != 0)
    return;

  // no-warning, fseeks supports error checking
  if (fseek(f, 0, 0) != 0)
    return;

  // no-warning, setvbuf supports error checking
  if (setvbuf(f, buf, 0, BUFFSIZE) != 0)
    return;
}
