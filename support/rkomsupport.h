/* 
 * Extra defs needed by certain OSes.
 */
#if defined(SOLARIS)
typedef	unsigned int u_int32_t;
typedef	unsigned char u_int8_t;
typedef unsigned short u_int16_t;
typedef unsigned long long u_int64_t;
#endif /* SOLARIS */

#if defined(SOLARIS)
char *strsep(char **stringp, const char *delim);
#endif
