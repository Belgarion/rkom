
#include <sys/types.h>
#include <sys/stat.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "rkomsupport.h"
#include "rkom.h"
#include "set.h"
#include "next.h"
#include "backend.h"

char *supstr;

void
show_superhoppa(char *arg)
{
	char *c, *ch;

	if (lasttext == 0) {
		rprintf("Du m�ste l�sa ett inl�gg f�rst.\n");
		return;
	}
	ch = rk_gettext(lasttext);
	c = index(ch, '\n');
	if (c != NULL) {
		supstr = calloc(c - ch + 1, 1);
		bcopy(ch, supstr, c - ch);
	} else
		supstr = strdup(ch);
	rprintf("Superhoppar �ver alla inl�gg med �renderad '%s'\n", supstr);
}

char *
vem(int num)
{
	struct rk_conference *conf;
	static char *ret, buf[60];

	if (ret && ret != buf)
		free(ret);
	ret = 0;

	if ((conf = rk_confinfo(num)) == NULL) {
		sprintf(buf, "person %d (hemlig)", num);
		ret = buf;
	} else
		ret = strdup(conf->rc_name);
	return ret;
}

static void
printcmnt(struct rk_misc_info *mi, int len, int p)
{
	int i;

	for (i = 0; i < len; i++) {
		if (mi[i].rmi_type == footn_in ||
		    mi[i].rmi_type == comm_in) {
			struct rk_text_stat *tt;

			rprintf("%s%s i text %d", p ? "(" : "",
			    (mi[i].rmi_type == footn_in ?
			    "Fotnot" : "Kommentar"), mi[i].rmi_numeric);
			if ((tt = rk_textstat(mi[i].rmi_numeric)) != NULL)
				rprintf(" av %s", vem(tt->rt_author));
			rprintf("%s\n", p ? ")" : "");
		}
	}
}

static char *
rxindex(char *s, int n, int c)
{
	while (n && s[n] != c)
		n--;
	return (n ? s+n : NULL);
}

static void
show_formatted(char *cc)
{
	char *ncc, *nncc;

	/* Remove trailing \n */
	while (strlen(cc) && cc[strlen(cc)-1] == '\n')
		cc[strlen(cc)-1] = 0;
	if (strlen(cc) == 0)
		return;

	/* Print lines, break them if they are longer than the screen width */
	while ((ncc = index(cc, '\n'))) {
		if ((ncc - cc) > wcols) {
			nncc = rxindex(cc, wcols-8, ' ');
			if (nncc == NULL) {
				*ncc++ = 0;
				rprintf("%s\n", cc);
				cc = ncc;
			} else {
				*nncc++ = 0;
				rprintf("%s\n", cc);
				cc = nncc;
			}
		} else {
			*ncc++ = 0;
			rprintf("%s\n", cc);
			cc = ncc;
		}
	}
	if (*cc) {
		while (strlen(cc) > wcols) {
			nncc = rxindex(cc, wcols-8, ' ');
			if (nncc == NULL)
				break;
			*nncc++ = 0;
			rprintf("%s\n", cc);
			cc = nncc;
		}
		if (*cc)
			rprintf("%s", cc);
	}
}

int
show_text(int nr, int format)
{
	struct rk_conference *conf;
	struct rk_text_stat *ts;
	struct rk_misc_info *mi;
	int i, len, p;
	char *c, *cc, *namn, buf[100];

	if ((ts = rk_textstat(nr)) == NULL) {
		rprintf("F�r din del s� finns inte text %d\n", nr);
		return 0;
	}
	c = strdup(rk_gettext(nr));
	if (supstr) {
		int l = strlen(supstr);
		if (strncmp(c, supstr, l) == 0 && (c[l] == 0 || c[l] == '\n')) {
			return 1;
		}
	}

	if ((conf = rk_confinfo(ts->rt_author)) == NULL) {
		namn = malloc(100);
		sprintf(namn, "Person %d (hemlig)", ts->rt_author);
	} else
		namn = strdup(conf->rc_name);
	rprintf("\n(%d) %s /%d rad%s/ %s", nr,
	    get_date_string(&ts->rt_time), ts->rt_no_of_lines,
	    ts->rt_no_of_lines > 1 ? "er" : "", namn);

	mi = ts->rt_misc_info.rt_misc_info_val;
	len = ts->rt_misc_info.rt_misc_info_len;

	for (i = 0; i < len; i++) {
		struct rk_text_stat *tt;

		switch(mi[i].rmi_type) {
		case recpt:
			rprintf("\n");
			rprintf("Mottagare: %s", vem(mi[i].rmi_numeric));
			break;
		case loc_no:
			rprintf(" <%d>", mi[i].rmi_numeric);
			break;

		case cc_recpt:
			rprintf("\n");
			rprintf("Extra kopia: %s", vem(mi[i].rmi_numeric));
			break;

		case sent_by:
			rprintf("\n");
			rprintf("  S�ndare: %s", vem(mi[i].rmi_numeric));
			break;
		
		case sentat:
			rprintf(" -- S�nt: %s",
				get_date_string(&mi[i].rmi_time));
			break;

		case rec_time:
			rprintf(" -- Mottaget: %s", 
				get_date_string(&mi[i].rmi_time));
			break;

		case comm_to:
			rprintf("\n");
			rprintf("Kommentar till text %d", mi[i].rmi_numeric);
			if ((tt = rk_textstat(mi[i].rmi_numeric)) != NULL)
				rprintf(" av %s", vem(tt->rt_author));
			break;

		case footn_to:
			rprintf("\n");
			rprintf("Fotnot till text %d", mi[i].rmi_numeric);
			if ((tt = rk_textstat(mi[i].rmi_numeric)) != NULL)
				rprintf(" av %s", vem(tt->rt_author));
			break;
		default:
			break;
		}
	}
	rprintf("\n");
	p = iseql("reading-puts-comments-in-pointers-last", "1");
	if (p == 0)
		printcmnt(mi, len, p);

	if (ts->rt_no_of_marks)
		rprintf("Texten markerad av %d person%s.\n", ts->rt_no_of_marks,
		    ts->rt_no_of_marks == 1 ? "" : "er");
	rprintf("�rende: ");
	cc = c;
	while (*cc && (*cc != '\n'))
		rprintf("%c", *cc++);
	if (isneq("dashed-lines", "0"))
		rprintf("\n------------------------------------------------------------");
	if (*cc) {
		if (format) {
			show_formatted(cc);
			rprintf("\n");
		} else {
			rprintf("%s", cc);
			if (cc[strlen(cc) - 1] != '\n')
				rprintf("\n");
		}
	} else
		rprintf("\n");
	if (iseql("show-writer-after-text", "1"))
		sprintf(buf, "(%d) /%s/ ", nr, namn);
	else
		sprintf(buf, "(%d) ", nr);
	if (isneq("dashed-lines", "0") && strlen(buf) < 60)
		rprintf("%s%s", buf, &"------------------------------------------------------------\n"[strlen(buf)]);
	else
		rprintf("%s\n", buf);
	free(namn);
	/* Check for "Anm�rkningar" */
	for (i = 0; i < ts->rt_aux_item.rt_aux_item_len; i++) {
		if (ts->rt_aux_item.rt_aux_item_val[i].rai_tag ==
		    RAI_TAG_FAST_REPLY) {
			rprintf("Anm�rkning av %s:\n",
			        vem(ts->rt_aux_item.rt_aux_item_val[i].rai_creator));
			rprintf("  \"%s\"\n", 
			    ts->rt_aux_item.rt_aux_item_val[i].rai_data);
		}
	}
	if (p)
		printcmnt(mi, len, p);

	return 0;
}

char *
get_date_string(struct rk_time *t)
{
	static char dstr[100];

	/* XXX use asctime() */
	sprintf(dstr, "%d-%02d-%02d %02d.%02d", t->rt_year+1900,
	    t->rt_month + 1, t->rt_day, t->rt_hours, t->rt_minutes);
	return dstr;
}

void
show_savetext(char *str)
{
	int fd, outfd, olines, orows;

	if (str == 0) {
		rprintf("Du m�ste ange filnamn att spara till.\n");
		return;
	}
	fd = open(str, O_WRONLY|O_CREAT|O_APPEND, S_IRUSR|S_IWUSR);
	if (fd < 0) {
		rprintf("Det sket sej att skapa filen: %s\n", strerror(errno));
		return;
	}
	/* This is a quite strange way to save the message... */
	outfd = dup(1);
	dup2(fd, 1);
	close(fd);
	orows = wrows;
	olines = outlines;
	wrows = 2000000;
	show_text(lasttext, 0);
	wrows = orows;
	outlines = olines;
	close(1);
	dup2(outfd, 1);
	close(outfd);
}
