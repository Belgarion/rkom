
#include <sys/types.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "rkom_proto.h"
#include "exported.h"
#include "rkom.h"
#include "set.h"
//#include "read.h"

static char *
vem(int num)
{
	struct rk_conference *conf;
	static char *ret, buf[30];

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

			printf("%s%s i text %d", p ? "(" : "",
			    (mi[i].rmi_type == footn_in ?
			    "Fotnot" : "Kommentar"), mi[i].rmi_numeric);
			tt = rk_textstat(mi[i].rmi_numeric);
			if (tt->rt_retval == 0)
				printf(" av %s", vem(tt->rt_author));
			free(tt);
			printf("%s\n", p ? ")" : "");
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
	char *c, *cc, *namn, buf[20];

	ts = rk_textstat(nr);
	if (ts->rt_retval) {
		printf("För din del så finns inte text %d\n", nr);
		free(ts);
		return;
	}

	conf = rk_confinfo(ts->rt_author);
	if (conf->rc_retval) {
		namn = alloca(100);
		sprintf(namn, "Person %d (hemlig)", ts->rt_author);
	} else
		namn = conf->rc_name;
	printf("\n(%d) %s /%d rad%s/ %s\n", nr,
	    get_date_string(&ts->rt_time), ts->rt_no_of_lines,
	    ts->rt_no_of_lines == 2 ? "er" : "", namn);
	free(conf);

	mi = ts->rt_misc_info.rt_misc_info_val;
	len = ts->rt_misc_info.rt_misc_info_len;
	for (i = 0; i < len; i++) {
		if (mi[i].rmi_type == recpt)
			printf("Mottagare: %s\n", vem(mi[i].rmi_numeric));

		if (mi[i].rmi_type == cc_recpt)
			printf("Extra kopia: %s\n", vem(mi[i].rmi_numeric));
		if (mi[i].rmi_type == comm_to) {
			struct rk_text_stat *tt;

			printf("Kommentar till text %d", mi[i].rmi_numeric);
			tt = rk_textstat(mi[i].rmi_numeric);
			if (tt->rt_retval == 0)
				printf(" av %s", vem(tt->rt_author));
			free(tt);
			printf("\n");
		}

		if (mi[i].rmi_type == footn_to) {
			struct rk_text_stat *tt;

			printf("Fotnot till text %d", mi[i].rmi_numeric);
			tt = rk_textstat(mi[i].rmi_numeric);
			if (tt->rt_retval == 0)
					printf(" av %s", vem(tt->rt_author));
			free(tt);
			printf("\n");
		}
	}
	p = iseql("reading-puts-comments-in-pointers-last", "1");
	if (p == 0)
		printcmnt(mi, len, p);

	if (ts->rt_no_of_marks)
		printf("Texten markerad av %d person%s.\n", ts->rt_no_of_marks,
		    ts->rt_no_of_marks == 1 ? "" : "er");
	c = rk_gettext(nr);
	printf("Ärende: ");
	cc = c;
	while (*cc && (*cc != '\n'))
		printf("%c", *cc++);
	if (isneq("dashed-lines", "0"))
		printf("\n------------------------------------------------------------");
	if (*cc) {
		printf("%s", cc);
		if (cc[strlen(cc) - 1] != '\n')
			printf("\n");
	} else
		printf("\n");
	sprintf(buf, "(%d)", nr);
	if (isneq("dashed-lines", "0"))
		printf("%s%s", buf, &"------------------------------------------------------------\n"[strlen(buf)]);
	else
		printf("\n");
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
