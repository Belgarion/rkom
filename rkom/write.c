
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <paths.h>
#include <errno.h>
#include <fcntl.h>

#include "rkom_proto.h"

#include "rkom.h"
#include "write.h"
#include "set.h"
#include "next.h"

static char *get_text(char *);
static char *input_string(void);
static void parse_text(char *);
static int extedit(char *);
static char *show_format(void);
static void wfotnot(char *str);

static struct rk_misc_info *mi;
static int nmi = 0;
static char *ctext = 0;

#define	TW if (!is_writing) {rprintf("Du skriver ingen text just nu.\n");return;}
#define IW if (is_writing) {rprintf("Du håller redan på att skriva en text.\n");return;}
#define	LF if (myuid == 0) {rprintf("Du måste logga in först.\n");return;}
#define	NC if (curconf == 0) {rprintf("Du måste gå till ett möte först.\n");return;}

static void
doedit(char *s)
{
	char *txt;

a:      if (use_editor) {
		if (extedit(s)) {
			use_editor = 0;
			goto a;
		}
	} else {
		txt = get_text(s);
		parse_text(txt);
		free(txt);
	}
}

void
write_brev(char *str)
{
	struct rk_confinfo_retval *retval;

	IW;
	LF;
	if (str == 0) {
		rprintf("Du måste ange mottagare.\n");
		return;
	}
	if ((retval = match_complain(str, MATCHCONF_PERSON)) == 0)
		return;

	rprintf("(Skicka) brev (till) %s\n",
	    retval->rcr_ci.rcr_ci_val[0].rc_name);
	is_writing = 1;
	mi = calloc(sizeof(struct rk_misc_info), 2);
	mi[0].rmi_type = recpt;
	mi[0].rmi_numeric = retval->rcr_ci.rcr_ci_val[0].rc_conf_no;
	mi[1].rmi_type = recpt;
	mi[1].rmi_numeric = myuid;
	nmi = 2;
	doedit(0);
}

void
write_private(int textno)
{
	struct rk_text_stat *ts;
	char *s, *t;

	rprintf("Personligt (svar till text %d)\n", textno);
	is_writing = 1;
	ts = rk_textstat(textno);
	if (ts->rt_retval) {
		rprintf("Kunde inte svara personligt på texten: %s\n",
		    error(ts->rt_retval));
		free(ts);
		return;
	}
	mi = calloc(sizeof(struct rk_misc_info), 2);
	mi[0].rmi_type = recpt;
	mi[0].rmi_numeric = ts->rt_author;
	mi[1].rmi_type = recpt;
	mi[1].rmi_numeric = myuid;
	nmi = 2;
	free(ts);

	/* Get the subject line from commented text */
	s = rk_gettext(textno);
	t = index(s, '\n');
	if (t)
		t[1] = 0;

	doedit(s);
	free(s);
}

void
write_new(char *str)
{
	IW;
	LF;
	NC;

	is_writing = 1;
	mi = calloc(sizeof(struct rk_misc_info), 1);
	mi[0].rmi_type = recpt;
	mi[0].rmi_numeric = curconf;
	nmi = 1;
	doedit(0);
}

void
parse_text(char *txt)
{
	char *cmd, *arg;

	if (ctext)
		free(ctext);
	ctext = 0;

	for (;;) {
		if (*txt != '!') {
			ctext = strdup(txt);
			return;
		}
		txt++;
		arg = strsep(&txt, "\n");
		cmd = strsep(&arg, " ");
		if (txt == NULL || arg == NULL)
			return;
		if (strncasecmp(cmd, "Mottagare:", strlen(cmd)) == 0)
			write_rcpt(arg);
		else if (strncasecmp(cmd, "Ärende:", strlen(cmd)) == 0) {
			arg[strlen(arg)] = '\n';
			ctext = strdup(arg);
			return;
		} else if (strncasecmp(cmd, "Kommentar", strlen(cmd)) == 0) {
			if (strncasecmp(arg, "till:", 5) == 0)
				cmd = strsep(&arg, " ");
			write_comment(arg);
		} else if (strncasecmp(cmd, "Fotnot", strlen(cmd)) == 0) {
			wfotnot(arg);
		} else
			rprintf("%s förstods inte.\n", cmd);
	}
}

void
write_put(char *str)
{
	struct rk_text_stat *ts;
	struct rk_text_info *rti;
	struct rk_text_retval *rtr;
	struct rk_misc_info *imi;
	int cnt, i, conf;

	TW;

	rti = malloc(sizeof(struct rk_text_info));
	rti->rti_misc.rti_misc_len = nmi;
	rti->rti_misc.rti_misc_val = mi;
	rti->rti_text = ctext;
	rtr = rk_create_text(rti);
	if (rtr->rtr_status)
		rprintf("write_new: %s\n", error(rtr->rtr_status));
	else
		rprintf("Text %d har skapats.\n", rtr->rtr_textnr);
	if (isneq("created-texts-are-read", "0") && (rtr->rtr_status == 0)) {
		ts = rk_textstat(rtr->rtr_textnr);
		imi = ts->rt_misc_info.rt_misc_info_val;
		cnt = ts->rt_misc_info.rt_misc_info_len;
		for (i = 0; i < cnt; i++) {
			if (imi[i].rmi_type == recpt ||
			    imi[i].rmi_type == cc_recpt ||
			    imi[i].rmi_type == bcc_recpt)
				conf = imi[i].rmi_numeric;
			if (imi[i].rmi_type == loc_no)
				rk_mark_read(conf, imi[i].rmi_numeric);
		}
	}
	free(ctext);
	free(mi);
	free(rtr);
	free(rti);
	nmi = 0;
	is_writing = 0;
	ctext = 0;
}

char *
get_text(char *sub)
{
	char *str, *base;

	if (sub == 0)
		base = calloc(10, 1);
	else
		base = strdup(sub);
	rprintf("\nÄrende: ");
	if (sub)
		rprintf("%s\n", base);
	fflush(stdout);

	for (;;) {
		str = input_string();
		base = realloc(base, strlen(base) + strlen(str) + 1);
		strcat(base, str);
		if (strlen(str) == 0 || str[strlen(str) - 1] != '\n') {
			free(str);
			return base;
		}
		free(str);
	}
}

char *
input_string()
{
        int i = 0, len;
        char *buf = 0;

        do {
                i++;
                buf = realloc(buf, 80 * i);
                len = read(0, &buf[80 * (i - 1)], 80);
        } while (len == 80);
        buf[80 * (i - 1) + len] = 0;

        return buf;
}

void
write_forget(char *str)
{
	if (is_writing)
		rprintf("Texten du höll på att skriva är nu bortkastad.\n");
	else
		rprintf("Du håller inte på att skriva någon text.\n");
	is_writing = 0;
	if (ctext)
		free(ctext);
	if (nmi)
		free(mi);
	ctext = 0;
	nmi = 0;
}

void
write_editor(char *str)
{
	TW;
	extedit(0);
}

void
write_rcpt(char *str)
{
	struct rk_confinfo_retval *cr;
	int conf, i;

	TW;
	cr = match_complain(str, MATCHCONF_PERSON|MATCHCONF_CONF);
	if (cr == NULL)
		return;
	conf = cr->rcr_ci.rcr_ci_val[0].rc_conf_no;
	free(cr);
	for (i = 0; i < nmi; i++)
		if (mi[i].rmi_numeric == conf && mi[i].rmi_type == recpt)
			return;
	nmi++;
	mi = realloc(mi, sizeof(struct rk_misc_info) * nmi);
	mi[nmi-1].rmi_type = recpt;
	mi[nmi-1].rmi_numeric = conf;
}

static void
wfotnot(char *str)
{
	int nr, i;
	char *p;

	TW;
	p = index(str, ' ');
	if (p)
		nr = atoi(p);
	if (p == 0 || nr == 0) {
		rprintf("Det var ett hemskt dåligt inläggsnummer.\n");
		return;
	}
	for (i = 0; i < nmi; i++)
		if (mi[i].rmi_numeric == nr && mi[i].rmi_type == footn_to)
			return;

	nmi++;
	mi = realloc(mi, sizeof(struct rk_misc_info) * nmi);
	mi[nmi-1].rmi_type = footn_to;
	mi[nmi-1].rmi_numeric = nr;
}

void
write_comment(char *str)
{
	int nr, i;

	TW;
	nr = atoi(str);
	if (nr == 0) {
		rprintf("Det var ett hemskt dåligt inläggsnummer.\n");
		return;
	}
	for (i = 0; i < nmi; i++)
		if (mi[i].rmi_numeric == nr && mi[i].rmi_type == comm_to)
			return;

	nmi++;
	mi = realloc(mi, sizeof(struct rk_misc_info) * nmi);
	mi[nmi-1].rmi_type = comm_to;
	mi[nmi-1].rmi_numeric = nr;
}

void    
write_whole(char *str)  
{
	char *txt;

	TW;

	txt = show_format();
	puts(txt);
	free(txt);
}

char *
show_format()
{
	struct rk_conference *conf;
	char *ret = calloc(10, 1), *r, buf[40];
	int i;

	for (i = 0; i < nmi; i++) {
		switch(mi[i].rmi_type) {
		case recpt:
			conf = rk_confinfo(mi[i].rmi_numeric);
			r = conf->rc_name;
			ret = realloc(ret, strlen(ret) + strlen(r) + 30);
			strcat(ret, "!Mottagare: ");
			strcat(ret, r);
			strcat(ret, "\n");
			free(conf);
			break;
		case comm_to:
			ret = realloc(ret, strlen(ret) + 40);
			sprintf(buf, "!Kommentar till: %d\n", mi[i].rmi_numeric);
			strcat(ret, buf);
			break;
		case footn_to:
			ret = realloc(ret, strlen(ret) + 40);
			sprintf(buf, "!Fotnot till: %d\n", mi[i].rmi_numeric);
			strcat(ret, buf);
			break;
		default:
			rprintf("Unknown text type %d.\n", mi[i].rmi_type);
			break;
		}
	}
	ret = realloc(ret, strlen(ret) + (ctext?strlen(ctext):0) + 25);
	strcat(ret, "!Ärende: ");
	if (ctext)
		strcat(ret, ctext);
	return ret;
}

int
extedit(char *sub)
{
	struct stat sb;
	extern char **environ;
	char *editor = getenv("EDITOR");
	char *txt, fil[30], *args[4];
	int f;

	if (editor == 0)
		editor = _PATH_VI;

	strcpy(fil, "/tmp/raggkom.XXXXX");
	f = mkstemp(fil);
	if (f == -1) {
		rprintf("Det gick inte: %s\n", strerror(errno));
		return 0;
	}
	if (sub)
		ctext = strdup(sub);
	txt = show_format();
	write(f, txt, strlen(txt));
	close(f);
	free(txt);
	args[0] = editor;
	args[1] = "+2"; /* XXX */
	args[2] = fil;
	args[3] = 0;
	if (fork() == 0) {
		execve(editor, args, environ);
		exit(errno); /* Only if exec failed */
	}
	wait(&f);
	if (WEXITSTATUS(f)) {
		unlink(fil);
		rprintf("Kunde inte anropa %s: %s\n", editor, 
		    strerror(WEXITSTATUS(f)));
		return 1;
	}
	stat(fil, &sb);
	txt = calloc(sb.st_size + 5, 1);
	f = open(fil, O_RDONLY);
	read(f, txt, sb.st_size);
	close(f);
	unlink(fil);
	parse_text(txt);
	free(txt);
	return 0;
}

static void
write_internal(int text, int ktyp)
{
	struct rk_text_stat *ts;
	struct rk_misc_info *mf;
	char *s, *t;
	int i, num;

	ts = rk_textstat(text);
	if (ts->rt_retval) {
		rprintf("Text %d är inte läsbar, tyvärr...\n", text);
		free(ts);
		return;
	}

	if (ktyp == footn_to && ts->rt_author != myuid) {
		rprintf("Du kan bara skriva fotnötter till dina egna inlägg.\n");
		free(ts);
		return;
	}
	mf = ts->rt_misc_info.rt_misc_info_val;
	num = ts->rt_misc_info.rt_misc_info_len;
	mi = calloc(sizeof(struct rk_misc_info), num + 1); /* Max size */
	nmi = 0;
	for (i = 0; i < num; i++)
		if (mf[i].rmi_type == recpt) {
			mi[nmi].rmi_type = recpt;
			mi[nmi].rmi_numeric = mf[i].rmi_numeric;
			nmi++;
		}
	mi[nmi].rmi_type = ktyp;
	mi[nmi].rmi_numeric = text;
	nmi++;

	/* Get the subject line from commented text */
	s = rk_gettext(text);
	t = index(s, '\n');
	if (t)
		t[1] = 0;

	is_writing = 1;
	doedit(s);
	free(s);
}

void
write_cmnt_last()
{
	if (lastlasttext == 0) {
		rprintf("Det finns inget föregående läst inlägg.\n");
		return;
	}
	rprintf("Kommentera föregående (inlägg) (%d)\n", lastlasttext);
	write_internal(lastlasttext, comm_to);
}

void
write_cmnt_no(nr)
{
	rprintf("Kommentera (inlägg %d)\n", nr);
	write_internal(nr, comm_to);
}

void
write_cmnt()
{
	if (lasttext == 0) {
		rprintf("Du har inte läst något inlägg.\n");
		return;
	}
	rprintf("Kommentera (inlägg %d)\n", lasttext);
	write_internal(lasttext, comm_to);
}

void
write_footnote()
{
	if (lasttext == 0) {
		rprintf("Du har inte läst något inlägg.\n");
		return;
	}
	rprintf("Fotnot (till inlägg %d)\n", lasttext);
	write_internal(lasttext, footn_to);
}

void
write_footnote_no(int num)
{
	rprintf("Fotnot (till inlägg %d)\n", num);
	write_internal(num, footn_to);
}
