
#include <sys/types.h>
#include <sys/stat.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "rkom_proto.h"
#include "rkom.h"
#include "set.h"
#include "next.h"

char *
vem(int num)
{
	struct rk_conference *conf;
	static char *ret, buf[60];

	if (ret && ret != buf)
		free(ret);
	ret = 0;

	conf = rk_confinfo(num);
	if (conf->rc_retval) {
		sprintf(buf, "person %d (hemlig)", num);
		ret = buf;
	} else
		ret = strdup(conf->rc_name);
	free(conf);
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
			tt = rk_textstat(mi[i].rmi_numeric);
			if (tt->rt_retval == 0)
				rprintf(" av %s", vem(tt->rt_author));
			free(tt);
			rprintf("%s\n", p ? ")" : "");
		}
	}
}

void
show_text(int nr)
{
	struct rk_conference *conf;
	struct rk_text_stat *ts;
	struct rk_misc_info *mi;
	int i, len, p;
	char *c, *cc, *namn, buf[100];
	char namnbuf[100];

	ts = rk_textstat(nr);
	if (ts->rt_retval) {
		rprintf("För din del så finns inte text %d\n", nr);
		free(ts);
		return;
	}

	conf = rk_confinfo(ts->rt_author);
	if (conf->rc_retval) {
		namn = namnbuf;
		sprintf(namn, "Person %d (hemlig)", ts->rt_author);
	} else
		namn = conf->rc_name;
	rprintf("\n(%d) %s /%d rad%s/ %s\n", nr,
	    get_date_string(&ts->rt_time), ts->rt_no_of_lines,
	    ts->rt_no_of_lines > 1 ? "er" : "", namn);
	free(conf);

	mi = ts->rt_misc_info.rt_misc_info_val;
	len = ts->rt_misc_info.rt_misc_info_len;
	for (i = 0; i < len; i++) {
		if (mi[i].rmi_type == recpt)
			rprintf("Mottagare: %s\n", vem(mi[i].rmi_numeric));

		if (mi[i].rmi_type == cc_recpt)
			rprintf("Extra kopia: %s\n", vem(mi[i].rmi_numeric));
		if (mi[i].rmi_type == comm_to) {
			struct rk_text_stat *tt;

			rprintf("Kommentar till text %d", mi[i].rmi_numeric);
			tt = rk_textstat(mi[i].rmi_numeric);
			if (tt->rt_retval == 0)
				rprintf(" av %s", vem(tt->rt_author));
			free(tt);
			rprintf("\n");
		}

		if (mi[i].rmi_type == footn_to) {
			struct rk_text_stat *tt;

			rprintf("Fotnot till text %d", mi[i].rmi_numeric);
			tt = rk_textstat(mi[i].rmi_numeric);
			if (tt->rt_retval == 0)
					rprintf(" av %s", vem(tt->rt_author));
			free(tt);
			rprintf("\n");
		}
	}
	p = iseql("reading-puts-comments-in-pointers-last", "1");
	if (p == 0)
		printcmnt(mi, len, p);

	if (ts->rt_no_of_marks)
		rprintf("Texten markerad av %d person%s.\n", ts->rt_no_of_marks,
		    ts->rt_no_of_marks == 1 ? "" : "er");
	c = rk_gettext(nr);
	rprintf("Ärende: ");
	cc = c;
	while (*cc && (*cc != '\n'))
		rprintf("%c", *cc++);
	if (isneq("dashed-lines", "0"))
		rprintf("\n------------------------------------------------------------");
	if (*cc) {
		rprintf("%s", cc);
		if (cc[strlen(cc) - 1] != '\n')
			rprintf("\n");
	} else
		rprintf("\n");
	sprintf(buf, "(%d) %s", nr, namn);
	if (isneq("dashed-lines", "0") && strlen(buf) < 60)
		rprintf("%s%s", buf, &"------------------------------------------------------------\n"[strlen(buf)]);
	else
		rprintf("%s\n", buf);
	free(c);
	if (p)
		printcmnt(mi, len, p);

	free(ts);
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
		rprintf("Du måste ange filnamn att spara till.\n");
		return;
	}
	fd = open(str, O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR);
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
	show_text(lasttext);
	wrows = orows;
	outlines = olines;
	close(1);
	dup2(outfd, 1);
	close(outfd);
}
