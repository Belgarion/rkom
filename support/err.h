/*
 * Protos for the err() routines. Will be included if system misses err.h.
 */
void	err(int, const char *, ...);
void	errx(int, const char *, ...);
void	warn(const char *, ...);
void	warnx(const char *, ...);
