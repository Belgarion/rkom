
#include <sys/types.h>

#include <stdio.h>

#include "rkom.h"

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
	"Det där namnet är inte alls bra.",
	"Numret du försökt använda är utanför gränserna.",
	"Namnet du angett finns redan.",
	"Namnet du angett finns redan (21).",
	"Du försöker skapa ett möte tvetydigt, men det går inte.",
	"Nu försökte du ändra brevlådeflaggor.",
	"Servern har fel: databasen är sönderskriven.",
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
