/* 
 * Extra defs needed by certain OSes.
 */
#if defined(SOLARIS)
typedef	unsigned int u_int32_t;
typedef	unsigned char u_int8_t;
typedef unsigned short u_int16_t;
typedef unsigned long long u_int64_t;
#endif /* SOLARIS */

#if defined(SOLARIS) || defined(SUNOS4) || defined(AIX)
char *strsep(char **stringp, const char *delim);
#endif

#if defined(SUNOS4)
#if !defined(__BIT_TYPES_DEFINED__) /* Real resolver installed */
typedef char int8_t;
typedef short int16_t;
typedef int int32_t;
typedef unsigned char u_int8_t;
typedef unsigned short u_int16_t;
typedef unsigned int u_int32_t;
#endif /* __BIT_TYPES_DEFINED__ */
typedef unsigned long long u_int64_t;
typedef long long int64_t;
typedef int ssize_t;

/*
 * Many prototypes are missing...
 */
int socket(int domain, int type, int protocol);
void bcopy(const void *src, void *dst, int len);
struct sockaddr;
int connect(int s, const struct sockaddr *name, int namelen);
void bzero(void *b, int len);
int bcmp(const void *b1, const void *b2, int len);
int printf (const char *, ...);
struct pollfd;
int poll(struct pollfd *fds, unsigned int nfds, int timeout);
int _flsbuf(int, void *);
int _filbuf(void *);
struct iovec;
ssize_t writev(int d, const struct iovec *iov, int iovcnt);
int fclose(void *);
int fflush(void *);
int fprintf(void *, ...);
long strtol(char *, char **, int);
int fputc(char, void *);
int fputs(const char *, void *);
int getopt(int, char **, char *);
extern char *optarg;
extern int optind, opterr;
struct timezone;
struct timeval;
int gettimeofday(struct timeval *, struct timezone *);
int ioctl(int, int, char *);
char *strerror(int);
int snprintf(char *str, int count, const char *fmt, ...);
int puts(char *);
int mkstemp(char *template);
int strncasecmp(char *, char *, int);
int strcasecmp(char *, char *);
int system(char *);
char *getpass(char *);
char *strdup(const char *str);
#endif
