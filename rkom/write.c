/*	$Id: write.c,v 1.24 2001/01/14 11:12:43 ragge Exp $	*/

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
#include <histedit.h>

#include "rkom_proto.h"

#include "rkom.h"
#include "write.h"
#include "set.h"
#include "next.h"

static char *get_text(char *);
static char *input_string(char *);
static void parse_text(char *);
static int extedit(char *);
static char *show_format(void);
static void wfotnot(char *str);

static struct rk_misc_info *mi;
static int nmi = 0, ispres = 0;
static char *ctext = 0;

#define	TW if (!is_writing) {rprintf("Du skriver ingen text just nu.\n");return;}
#define IW if (is_writing) {rprintf("Du h�ller redan p� att skriva en text.\n");return;}
#define	LF if (myuid == 0) {rprintf("Du m�ste logga in f�rst.\n");return;}
#define	NC if (curconf == 0) {rprintf("Du m�ste g� till ett m�te f�rst.\n");return;}

static void
doedit(char *s)
{
	char *txt;

a:      if (isneq("use-editor", "0")) {
		if (extedit(s)) {
			set_setflag("use-editor", "0");
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
		rprintf("Du m�ste ange mottagare.\n");
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
		rprintf("Kunde inte svara personligt p� texten: %s\n",
		    error(ts->rt_retval));
		free(ts);
		return;
	}
	mi = calloc(sizeof(struct rk_misc_info), 3);
	mi[0].rmi_type = recpt;
	mi[0].rmi_numeric = ts->rt_author;
	mi[1].rmi_type = recpt;
	mi[1].rmi_numeric = myuid;
	mi[2].rmi_type = comm_to;
	mi[2].rmi_numeric = textno;
	nmi = 3;
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
			while (ctext[strlen(ctext)-1] == '\n' &&
			    ctext[strlen(ctext)-2] == '\n')
				ctext[strlen(ctext)-1] = 0;
			return;
		}
		txt++;
		arg = strsep(&txt, "\n");
		cmd = strsep(&arg, " ");
		if (txt == NULL || arg == NULL)
			return;
		if (strncasecmp(cmd, "Mottagare:", strlen(cmd)) == 0)
			write_rcpt(arg);
		else if (strncasecmp(cmd, "�rende:", strlen(cmd)) == 0) {
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
			rprintf("%s f�rstods inte.\n", cmd);
	}
}

void
write_put(char *str)
{
	struct rk_text_info *rti;
	struct rk_text_retval *rtr;
	struct rk_aux_item_input *rtii;

	TW;

	rti = alloca(sizeof(struct rk_text_info));
	rti->rti_misc.rti_misc_len = nmi;
	rti->rti_misc.rti_misc_val = mi;
	rti->rti_text = ctext;

	rtii = alloca(sizeof(*rtii));
	rtii->raii_tag = RAI_TAG_CREATING_SW;
	rtii->inherit_limit = 0;
	rtii->raii_data = "raggkom ett.ett.beta"; /* XXX */
	rti->rti_input.rti_input_len = 1;
	rti->rti_input.rti_input_val = rtii;

	if (ispres) {
		if (rk_set_presentation(ispres, rti))
			printf("Det gick inte.\n");
		else
			printf("Presentationen �ndrad.\n");
		ispres = 0;
	} else {
		rtr = rk_create_text(rti);
		if (rtr->rtr_status)
			rprintf("write_new: %s\n", error(rtr->rtr_status));
		else
			rprintf("Text %d har skapats.\n", rtr->rtr_textnr);
		lastlasttext = lasttext;
		lasttext = rtr->rtr_textnr;
		free(rtr);
	}
	free(ctext);
	free(mi);
	nmi = 0;
	is_writing = 0;
	ctext = 0;
}

char *
get_text(char *sub)
{
	char *str, *base;

	rprintf("\n");
	if (sub) {
		base = strdup(sub);
		rprintf("�rende: %s", base);
	} else
		base = input_string("�rende: ");

	for (;;) {
		str = input_string("");
		if (str == NULL)
			return base;
		base = realloc(base, strlen(base) + strlen(str) + 1);
		strcat(base, str);
		free(str);
	}
}

static char *msg;

static char *
prompt_fun(EditLine *el)
{
	return msg;
}

char *
input_string(char *m)
{
	EditLine *el;
	const char *get;
	char *ret;
	int len;

	msg = m;
#if defined(__FreeBSD__)
	el = el_init("rkom", stdin, stdout);
#else
	el = el_init("rkom", stdin, stdout, stderr);
#endif
	el_set(el, EL_EDITOR, "emacs");
	el_set(el, EL_PROMPT, prompt_fun);
	get = el_gets(el, &len);
	if (get)
		ret = strdup(get);
	else
		ret = NULL;
	el_end(el);
	return ret;
}

void
write_forget(char *str)
{
	if (is_writing)
		rprintf("Texten du h�ll p� att skriva �r nu bortkastad.\n");
	else
		rprintf("Du h�ller inte p� att skriva n�gon text.\n");
	ispres = is_writing = 0;
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
		rprintf("Det var ett hemskt d�ligt inl�ggsnummer.\n");
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
		rprintf("Det var ett hemskt d�ligt inl�ggsnummer.\n");
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
	strcat(ret, "!�rende: ");
	if (ctext)
		strcat(ret, ctext);
	return ret;
}

int
extedit(char *sub)
{
	struct stat sb;
	extern char **environ;
	char *editor;
	char *txt, fil[30], *args[4], buf[10];
	int f;

	editor = getenv("RKOM_EDITOR");
	if (editor == NULL)
		editor = getenv("EDITOR");
	if (editor == NULL)
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
	sprintf(buf, "+%d", nmi + 1);
	args[1] = buf;
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
		rprintf("Text %d �r inte l�sbar, tyv�rr...\n", text);
		free(ts);
		return;
	}

	if (ktyp == footn_to && ts->rt_author != myuid) {
		rprintf("Du kan bara skriva fotn�tter till dina egna inl�gg.\n");
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
		rprintf("Det finns inget f�reg�ende l�st inl�gg.\n");
		return;
	}
	rprintf("Kommentera f�reg�ende (inl�gg) (%d)\n", lastlasttext);
	write_internal(lastlasttext, comm_to);
}

void
write_cmnt_no(nr)
{
	rprintf("Kommentera (inl�gg %d)\n", nr);
	write_internal(nr, comm_to);
}

void
write_cmnt()
{
	if (lasttext == 0) {
		rprintf("Du har inte l�st n�got inl�gg.\n");
		return;
	}
	rprintf("Kommentera (inl�gg %d)\n", lasttext);
	write_internal(lasttext, comm_to);
}

void
write_footnote()
{
	if (lasttext == 0) {
		rprintf("Du har inte l�st n�got inl�gg.\n");
		return;
	}
	rprintf("Fotnot (till inl�gg %d)\n", lasttext);
	write_internal(lasttext, footn_to);
}

void
write_footnote_no(int num)
{
	rprintf("Fotnot (till inl�gg %d)\n", num);
	write_internal(num, footn_to);
}

void
write_change_presentation(char *str)
{
        struct rk_confinfo_retval *retval;

        if ((retval = match_complain(str, MATCHCONF_PERSON|MATCHCONF_CONF)) == 0)
                return;

	ispres = retval->rcr_ci.rcr_ci_val[0].rc_conf_no;
	rprintf("�ndra presentation (f�r) %s\n",
	    retval->rcr_ci.rcr_ci_val[0].rc_name);
	is_writing = 1;
	mi = calloc(sizeof(struct rk_misc_info), 2);
	nmi = 0;
	doedit(retval->rcr_ci.rcr_ci_val[0].rc_name);
	free(retval);
}

