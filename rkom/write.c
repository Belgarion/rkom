/*	$Id: write.c,v 1.49 2002/09/01 10:22:56 ragge Exp $	*/

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <unistd.h>
#ifdef STAR_BSD
#include <paths.h>
#else
#define	_PATH_VI	"/usr/bin/vi"
#endif
#include <errno.h>
#include <fcntl.h>

#include "rkomsupport.h"
#include "rkom_proto.h"

#include "rkom.h"
#include "write.h"
#include "set.h"
#include "next.h"
#include "rhistedit.h"

static char *get_text(char *);
static void parse_text(char *);
static int extedit(char *);
static char *show_format(void);
static void wfotnot(char *str);

static struct rk_misc_info *mi;
static int nmi = 0, ispres = 0, islapp = 0, isfaq = 0;
static char *ctext = 0;

static char *input_string(char *);

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
	nmi = (retval->rcr_ci.rcr_ci_val[0].rc_conf_no == myuid ? 1 : 2);
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
			write_rcpt(arg, recpt);
		else if (strncasecmp(cmd, "Extrakopia:", strlen(cmd)) == 0)
			write_rcpt(arg, cc_recpt);
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

static int
change_faq(int confno, struct rk_text_info *rti)
{
	struct rk_text_retval *rv;
	struct rk_modifyconfinfo *rkm;
	struct rk_conference *rcp;
	struct rk_aux_item *rai;
	struct rk_aux_item_input *raii;
	u_int32_t rmtext = 0;
	char buf[20];
	int i, nrai, top;

	rv = rk_create_text(rti);
	if (rv->rtr_status)
		return rv->rtr_status;
	rkm = alloca(sizeof(struct rk_modifyconfinfo));
	rkm->rkm_conf = confno;
	rkm->rkm_add.rkm_add_len = 0;
	rcp = rk_confinfo(confno);
	top = 0;
	rai = rcp->rc_aux_item.rc_aux_item_val;
	nrai = rcp->rc_aux_item.rc_aux_item_len;
	
	/*
	 * XXX what if more than one FAQ?
	 */
	for (i = 0; i < nrai; i++) {
		if (rai[i].rai_tag == RAI_TAG_FAQ_TEXT) {
			rmtext = rai[i].rai_aux_no;
		}
	}

	rkm->rkm_delete.rkm_delete_len = 0;
	if (rmtext) {
		rkm->rkm_delete.rkm_delete_len = 1;
		rkm->rkm_delete.rkm_delete_val = &rmtext;
	}
	raii = alloca(sizeof(struct rk_aux_item_input));
	rkm->rkm_add.rkm_add_len = 1;
	rkm->rkm_add.rkm_add_val = raii;
	raii->raii_tag = RAI_TAG_FAQ_TEXT;
	raii->raii_flags = 0;
	raii->inherit_limit = 0;
	sprintf(buf, "%d", rv->rtr_textnr);
	raii->raii_data = buf;
	return rk_modify_conf_info(rkm);
}

void
write_put(char *str)
{
	struct rk_text_info *rti;
	struct rk_text_retval *rtr;
	struct rk_aux_item_input *rtii;

	if (isneq("write-convert-from-swascii", "0"))
		convert_from_swascii();

	/* Remove extra '\n' after txt */
	while (strlen(ctext) && ctext[strlen(ctext) - 1] == '\n')
		ctext[strlen(ctext) - 1] = 0;

	rti = alloca(sizeof(struct rk_text_info));
	rti->rti_misc.rti_misc_len = nmi;
	rti->rti_misc.rti_misc_val = mi;
	rti->rti_text = ctext;

	/*
	 * Set misc info. This should be done more intelligent.
	 */
	/* Set creating software */
	rtii = alloca(sizeof(*rtii) * 5);	/* XXX */
	rtii[0].raii_tag = RAI_TAG_CREATING_SW;
	rtii[0].inherit_limit = 0;
	rtii[0].raii_data = alloca(strlen(client_version) + 10);
	sprintf(rtii[0].raii_data, "raggkom %s", client_version);

	/* Set content type */
	rtii[1].raii_tag = RAI_TAG_CONTENT_TYPE;
	rtii[1].inherit_limit = 1; /* ??? */
#define	DEF_CONTENT_TYPE	"x-kom/text;charset=iso-8859-1"
	rtii[1].raii_data = DEF_CONTENT_TYPE;

	rti->rti_input.rti_input_len = 2;
	rti->rti_input.rti_input_val = rtii;

	if (isfaq) {
		int i;
		if ((i = change_faq(isfaq, rti)))
			rprintf("Det gick inte: %s\n", error(i));
		else
			rprintf("FAQn ändrad.\n");
		isfaq = 0;
	} else if (ispres) {
		if (rk_set_presentation(ispres, rti))
			rprintf("Det gick inte.\n");
		else
			rprintf("Presentationen ändrad.\n");
		ispres = 0;
	} else if (islapp) {
		if (rk_set_motd(islapp, rti))
			rprintf("Det gick inte.\n");
		else
			rprintf("Lappen ditsatt.\n");
		islapp = 0;
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
		rprintf("Ärende: %s", base);
	} else
		base = input_string("Ärende: ");

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
	static EditLine *el =  NULL;
	const char *get;
	char *ret;
	int len;

	if (el == NULL) {
		el = el_init("rkom", stdin, stdout, stderr);
		el_set(el, EL_EDITOR, getval("editor-mode"));
		el_set(el, EL_PROMPT, prompt_fun);
	}

	msg = m;
	get = el_gets(el, &len);
	if (get)
		ret = strdup(get);
	else
		ret = NULL;
	return ret;
}

void
write_forget(char *str)
{
	if (is_writing)
		rprintf("Texten du höll på att skriva är nu bortkastad.\n");
	else
		rprintf("Du håller inte på att skriva någon text.\n");
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
	extedit(0);
}

void
write_rcpt(char *str, int type)
{
	struct rk_confinfo_retval *cr;
	int conf, i;

	cr = match_complain(str, MATCHCONF_PERSON|MATCHCONF_CONF);
	if (cr == NULL)
		return;
	conf = cr->rcr_ci.rcr_ci_val[0].rc_conf_no;
	free(cr);
	for (i = 0; i < nmi; i++)
		if (mi[i].rmi_numeric == conf && mi[i].rmi_type == type)
			return;
	nmi++;
	mi = realloc(mi, sizeof(struct rk_misc_info) * nmi);
	mi[nmi-1].rmi_type = type;
	mi[nmi-1].rmi_numeric = conf;
}

static void
wfotnot(char *str)
{
	int nr = 0, i;
	char *p;

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
		case cc_recpt:
		case recpt:
			conf = rk_confinfo(mi[i].rmi_numeric);
			r = conf->rc_name;
			ret = realloc(ret, strlen(ret) + strlen(r) + 30);
			if (mi[i].rmi_type == cc_recpt)
				strcat(ret, "!Extrakopia: ");
			else
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
	char *editor;
	char *txt, fil[30], *buf;
	int f;

	editor = getenv("RKOM_EDITOR");
	if (editor == NULL)
		editor = getenv("EDITOR");
	if (editor == NULL)
		editor = _PATH_VI;

	strcpy(fil, "/tmp/raggkom.XXXXXX");
	if ((f = mkstemp(fil)) < 0) {
		rprintf("mkstemp() gick inte: %s\n", strerror(errno));
		return 0;
	}
	if (sub)
		ctext = strdup(sub);
	txt = show_format();
	write(f, txt, strlen(txt));
	close(f);
	free(txt);

	buf = alloca(strlen(editor) + 40);
	sprintf(buf, "%s +%d %s", editor, nmi + 1, fil);
	if (system(buf)) {
		unlink(fil);
		rprintf("Kunde inte anropa %s: %s\n", editor, strerror(errno));
		return 1;
	}

	stat(fil, &sb);
	txt = calloc(sb.st_size + 5, 1);
	f = open(fil, O_RDONLY);
	read(f, txt, sb.st_size);
	close(f);
	unlink(fil);
	if(mi) {
		free(mi);
		nmi = 0;
		mi = NULL;
	}
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
write_fastcmnt_no(nr)
{
	char *buf;
	struct rk_aux_item_input *rtii;
	int retval;

	rprintf("Snabbkommentera (inlägg %d)\n", nr);
	buf = getstr("Kommentarstext: ");
	if(strlen(buf)) {
		rtii = alloca(sizeof(*rtii));
		rtii->raii_tag = RAI_TAG_FAST_REPLY;
		rtii->inherit_limit = 1;
		rtii->raii_data = buf;

		retval = rk_add_text_info(nr, rtii);
	
		if(retval) {
			rprintf("Kunde inte addera kommentaren till texten: %s\n",
			    error(retval));
		}
		free(rtii);
	} else
		rprintf("Nehepp.");

	free(buf);
}

void
write_fastcmnt()
{
	if (lasttext == 0) {
		rprintf("Du har inte läst något inlägg.\n");
		return;
	}
	write_fastcmnt_no(lasttext);
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

void
write_change_faq(char *str)
{
        struct rk_confinfo_retval *retval;
	char *c = NULL;

        if ((retval = match_complain(str, MATCHCONF_CONF)) == 0)
                return;

	isfaq = retval->rcr_ci.rcr_ci_val[0].rc_conf_no;
	rprintf("Ändra FAQ (för) %s\n",
	    retval->rcr_ci.rcr_ci_val[0].rc_name);
	is_writing = 1;
	mi = calloc(sizeof(struct rk_misc_info), 2);
	nmi = 0;
	write_rcpt(str, recpt);
	if (isneq("use-editor", "0")) { /* Extern editor, edit old text */
		struct rk_conference *rc;

		rc = rk_confinfo(retval->rcr_ci.rcr_ci_val[0].rc_conf_no);
		if (rc->rc_retval == 0 && rc->rc_presentation)
			c = rk_gettext(rc->rc_presentation);
		free(rc);
	}
	if (c == NULL)
		c = strdup(retval->rcr_ci.rcr_ci_val[0].rc_name);
	doedit(c);
	free(c);
	free(retval);
}

void
write_change_presentation(char *str)
{
        struct rk_confinfo_retval *retval;
	struct rk_conference *rc;
	char *c = NULL;

        if ((retval = match_complain(str, MATCHCONF_PERSON|MATCHCONF_CONF)) == 0)
                return;

	ispres = retval->rcr_ci.rcr_ci_val[0].rc_conf_no;
	rprintf("Ändra presentation (för) %s\n",
	    retval->rcr_ci.rcr_ci_val[0].rc_name);
	rc = rk_confinfo(retval->rcr_ci.rcr_ci_val[0].rc_conf_no);
	is_writing = 1;
	mi = calloc(sizeof(struct rk_misc_info), 3);
	nmi = 0;
	if (rc->rc_presentation) {
		mi[0].rmi_type = comm_to;
		mi[0].rmi_numeric = rc->rc_presentation;
		nmi++;
	}

	if (isneq("use-editor", "0")) { /* Extern editor, edit old text */
		struct rk_conference *rc;

		rc = rk_confinfo(retval->rcr_ci.rcr_ci_val[0].rc_conf_no);
		if (rc->rc_retval == 0 && rc->rc_presentation)
			c = rk_gettext(rc->rc_presentation);
		free(rc);
	}
	if (c == NULL)
		c = strdup(retval->rcr_ci.rcr_ci_val[0].rc_name);
	doedit(c);
	free(c);
	free(retval);
}

void
write_set_motd(char *str)
{
        struct rk_confinfo_retval *rv;

        if ((rv = match_complain(str, MATCHCONF_PERSON|MATCHCONF_CONF)) == 0)
                return;

	islapp = rv->rcr_ci.rcr_ci_val[0].rc_conf_no;
	rprintf("(Sätt) lapp (på dörren för) %s\n",
	    rv->rcr_ci.rcr_ci_val[0].rc_name);
	free(rv);
	is_writing = 1;
	mi = calloc(sizeof(struct rk_misc_info), 2);
	nmi = 0;
	doedit(0);
}

void
write_remove_motd(char *str)
{
	struct rk_confinfo_retval *rv;
	struct rk_text_info rti;

	if ((rv = match_complain(str, MATCHCONF_PERSON|MATCHCONF_CONF)) == 0)
		return; 

	bzero(&rti, sizeof(struct rk_text_info));
	rti.rti_text = "";
	if (rk_set_motd(rv->rcr_ci.rcr_ci_val[0].rc_conf_no, &rti))
		rprintf("Det gick inte.\n");
	else
		rprintf("Lappen nu borttagen.\n");
}

void convert_from_swascii(void)
{
	chrconvert(ctext);
	rprintf("Texten har konverterats från swascii till ISO 8859-1.\n");
}
