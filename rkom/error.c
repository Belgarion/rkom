
#include <sys/types.h>

#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>
#include <err.h>

#include "rkom.h"
#include "rkomsupport.h"

static char *fel[] = {
	"Inget fel. ??? (borde inte inträffa", /* 0 */
	"Feltyp #1",
	"Anropet finns inte (ännu).",
	"Anropet har utgått ur specen.",
	"Felaktigt lösenord.",
	"Strängen alldeles för lång.",
	"Var god logga in först.",
	"KOMservern är tyvärr nere för tillfället.",
	"Internt fel: Försöker använda mötestyp noll.",
	"Det går inte att komma åt icke existerande möten.",
	"Det går inte att komma åt icke existerande personer.",
	"Du saknar rättigheter för denna operation (11).",
	"Du saknar rättigheter för denna operation (12).",
	"Du är inte medlem i mötet.",
	"Den här texten finns inte.",
	"Internt fel: Försöker komma åt text nummer noll.",
	"Det lokala textnumret finns inte.",
	"Det lokala textnumret 0 får inte användas.",
	"Det där namnet är inte alls bra.",
	"Numret du försökt använda är utanför gränserna.",
	"Namnet du angett finns redan.",
	"Namnet du angett finns redan (21).",
	"Du försöker skapa ett möte tvetydigt, men det går inte.",
	"Nu försökte du ändra brevlådeflaggor.",
	"Servern har fel: databasen är sönderskriven.", /* 24 */
	NULL, NULL, NULL, NULL, NULL, NULL, /* 30 */
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, /* 40 */
	NULL, NULL, NULL, NULL, NULL, NULL,
	"Anonyma texter är inte tillåtna i detta möte.",
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
