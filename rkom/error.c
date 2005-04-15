
#include <sys/types.h>

#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>
#include <err.h>

#include "rkom.h"
#include "rkomsupport.h"

static char *fel[] = {
	"Inget fel. ??? (borde inte intr�ffa", /* 0 */
	"Feltyp #1",
	"Anropet finns inte (�nnu).",
	"Anropet har utg�tt ur specen.",
	"Felaktigt l�senord.",
	"Str�ngen alldeles f�r l�ng.",
	"Var god logga in f�rst.",
	"KOMservern �r tyv�rr nere f�r tillf�llet.",
	"Internt fel: F�rs�ker anv�nda m�testyp noll.",
	"Det g�r inte att komma �t icke existerande m�ten.",
	"Det g�r inte att komma �t icke existerande personer.",
	"Du saknar r�ttigheter f�r denna operation (11).",
	"Du saknar r�ttigheter f�r denna operation (12).",
	"Du �r inte medlem i m�tet.",
	"Den h�r texten finns inte.",
	"Internt fel: F�rs�ker komma �t text nummer noll.",
	"Det lokala textnumret finns inte.",
	"Det lokala textnumret 0 f�r inte anv�ndas.",
	"Det d�r namnet �r inte alls bra.",
	"Numret du f�rs�kt anv�nda �r utanf�r gr�nserna.",
	"Namnet du angett finns redan.",
	"Namnet du angett finns redan (21).",
	"Du f�rs�ker skapa ett m�te tvetydigt, men det g�r inte.",
	"Nu f�rs�kte du �ndra brevl�deflaggor.",
	"Servern har fel: databasen �r s�nderskriven.", /* 24 */
	NULL, NULL, NULL, NULL, NULL, NULL, /* 30 */
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, /* 40 */
	NULL, NULL, NULL, NULL, NULL, NULL,
	"Anonyma texter �r inte till�tna i detta m�te.",
	NULL, NULL, NULL, /* 50 */
};

static int nfel = sizeof(fel)/sizeof(char *);

char *
error(int code)
{
	static char buf[30];

	if (code > nfel || fel[code] == NULL) {
		sprintf(buf, "Fel nummer %d", code);
		return buf;
	}
	return fel[code];
}

#if defined(SOLARIS) || defined(SUNOS4) || defined(AIX)
int vfprintf(FILE *, const char *, va_list);
extern char * __progname;

void
err(int eval, const char *fmt, ...)
{
	va_list ap;
	int	sverrno;

	sverrno = errno;
	(void)fprintf(stderr, "%s: ", __progname);
	va_start(ap, fmt);
	if (fmt != NULL) {
		(void)vfprintf(stderr, fmt, ap);
		(void)fprintf(stderr, ": ");
	}
	va_end(ap);
	(void)fprintf(stderr, "%s\n", strerror(sverrno));
	exit(eval);
}
void
errx(int eval, const char *fmt, ...)
{
	va_list ap;

	(void)fprintf(stderr, "%s: ", __progname);
	va_start(ap, fmt);
	if (fmt != NULL)
		(void)vfprintf(stderr, fmt, ap);
	va_end(ap);
	(void)fprintf(stderr, "\n");
	exit(eval);
}

void
warn(const char *fmt, ...)
{
	va_list ap;
	int	sverrno;

	sverrno = errno;
	(void)fprintf(stderr, "%s: ", __progname);
	va_start(ap, fmt);
	if (fmt != NULL) {
		(void)vfprintf(stderr, fmt, ap);
		(void)fprintf(stderr, ": ");
	}
	va_end(ap);
	(void)fprintf(stderr, "%s\n", strerror(sverrno));
}
void
warnx(const char *fmt, ...)
{
	va_list ap;

	(void)fprintf(stderr, "%s: ", __progname);
	va_start(ap, fmt);
	if (fmt != NULL)
		(void)vfprintf(stderr, fmt, ap);
	va_end(ap);
	(void)fprintf(stderr, "\n");
}
#endif
