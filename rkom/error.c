
#include <sys/types.h>

#include <stdio.h>

#include "rkom.h"

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
	"Det d�r namnet �r inte alls bra.",
	"Numret du f�rs�kt anv�nda �r utanf�r gr�nserna.",
	"Namnet du angett finns redan.",
	"Namnet du angett finns redan (21).",
	"Du f�rs�ker skapa ett m�te tvetydigt, men det g�r inte.",
	"Nu f�rs�kte du �ndra brevl�deflaggor.",
	"Servern har fel: databasen �r s�nderskriven.",
};

static int nfel = sizeof(fel)/sizeof(char *);

char *
error(int code)
{
	static char buf[30];

	if (code > nfel) {
		sprintf(buf, "Fel nummer %d", code);
		return buf;
	}
	return fel[code];
}
