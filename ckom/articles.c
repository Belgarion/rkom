/* $Id: articles.c,v 1.4 2000/10/15 23:30:42 jens Exp $ */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <err.h>

#include <rkom_proto.h>
#include <exported.h>

#include "articles.h"
#include "container.h"

#include <assert.h>

typedef struct cl_art_que artl_t;
CL_TYPE(art, art_t)

struct conf_texts {
	artl_t		*ct_al;
	char		ct_name[80];
	u_int32_t	ct_uid;
	u_int32_t	ct_cid;
	u_int32_t	ct_first_text;
	u_int32_t	ct_last_text;
};


static int art_add(u_int32_t local_id);
static int art_format_subjects(void);
static char **art_get_text(char *);
static char **art_get_header(struct rk_text_stat *rt);
static char **art_get_footer(struct rk_text_stat *rt);
static time_t art_get_time(struct rk_time *rt);
static void art_free(art_t *);

static int art_inited = 0;
static struct conf_texts ct;

int
art_open_conf(u_int32_t uid, u_int32_t conf_id)
{
	struct rk_conference *rc = NULL;
	struct rk_membership *rm = NULL;
	u_int32_t	first_text, last_text;
	int			i;

	if (!art_inited) {
		memset(&ct, 0, sizeof(ct));
		art_inited = 1;
	}
	if (ct.ct_al != NULL)
		cl_art_free(ct.ct_al, art_free);
	ct.ct_al = NULL;

	ct.ct_uid = uid;
	ct.ct_cid = conf_id;

	rm = rk_membership(uid, conf_id);
	if (rm->rm_retval != 0) {
		warnx("rk_membership: failed");
		free(rm);
		return -1;
	}
	first_text = rm->rm_last_text_read + 1;
	free(rm);

	rc = rk_confinfo(conf_id);
	if (rc->rc_retval != 0) {
		warnx("rk_confinfo: failed");
		free(rc);
		return -1;
	}
	last_text = rc->rc_first_local_no + rc->rc_no_of_texts - 1;
	strlcpy(ct.ct_name, rc->rc_name, sizeof(ct.ct_name));
	free(rc);

	for (i = first_text; i <= last_text; i++)
		art_add(i);

	art_format_subjects();
	
	return 0;
}

int
art_add(u_int32_t local_id)
{
	struct rk_text_stat *rt;
	struct rk_conference *rc;
	art_t		*art, *p_art;
	char		*tb;
	char		buf[80];
	artl_t		*al = NULL, *al_pos, *oal_pos;
	u_int32_t	global_text;
	int			i;

	if (ct.ct_al == NULL) {
		if ((ct.ct_al = cl_art_init()) == NULL) {
			warn("cl_art_init");
			return NULL;
		}
		ct.ct_first_text = local_id;
	}
	al = ct.ct_al;


	global_text = rk_local_to_global(ct.ct_cid, local_id);

	/* get some info of text */
	rt = rk_textstat(global_text);
	if (rt->rt_retval != 0) {
		free(rt);
		return -1;
	}
	/* rt is freed after the label out_find_parent: */

	ct.ct_last_text = local_id;

	if ((art = calloc(1, sizeof(art_t))) == NULL)
		err(1, "calloc");
	art->art_id = global_text;
	art->art_no_of_lines = rt->rt_no_of_lines;

	/* get subject */
	tb = rk_gettext(global_text);
	for (i = 0; tb[i] != '\0' && tb[i] != '\n' && i < sizeof(buf) - 1; i++);
	tb[i] = '\0';
	strcpy(art->art_real_subj, tb);

	/* store the text */
	art->art_text = art_get_text(&tb[i + 1]);
	free(tb);

	/* generate headers */
	art->art_header = art_get_header(rt);

	/* generate footers */
	art->art_footer = art_get_footer(rt);

	art->art_time = art_get_time(&rt->rt_time);

	/* get name of author */
	rc = rk_confinfo(rt->rt_author);
	if (rc->rc_retval != 0)
	snprintf(art->art_from, sizeof(art->art_from),
			"Person %d (hemlig)", rt->rt_author);
	 else
		strlcpy(art->art_from, rc->rc_name, sizeof(art->art_from));
	free(rc);

	art->art_flags |= ART_UNREAD;

	/* find eventual parent */
	for (al_pos = NULL; cl_art_walk(al, &al_pos, &p_art) == 0;) {
		for (i = 0; i < rt->rt_misc_info.rt_misc_info_len; i++) {
			u_int32_t rmi_type =
				rt->rt_misc_info.rt_misc_info_val[i].rmi_type;
			if (rmi_type != comm_to && rmi_type != footn_to)
				continue;
			if (p_art->art_id ==
					rt->rt_misc_info.rt_misc_info_val[i].rmi_numeric) {
				/* found a parent */
				art->art_parent = p_art;
				if (rmi_type == footn_to)
					art->art_flags |= ART_FOOTN;
				goto out_find_parent;
			}
		}
	}
out_find_parent:
	free(rt);

	/* if the text didn't have any author put it at the end */
	if (art->art_parent == NULL) {
		if (cl_art_tail_push(al, art) < 0)
			err(1, "cl_art_tail_push");
		return 0;
	}

	/*
	 * The text has a parent find the position after the last
	 * child of the parent
	 */
	for (oal_pos = al_pos; cl_art_walk(al, &al_pos, &p_art); ) {
		if (p_art->art_parent == NULL)
			break;
		if (p_art->art_parent->art_id != art->art_id)
			break;
		oal_pos = al_pos;
	}
	cl_art_ins_after_pos(oal_pos, NULL, art);
	
	return 0;
}

int
art_format_subjects()
{
	artl_t	*al, *al_pos, *t_al_pos;
	art_t	*art, *p_art, *t_art;
	int		comm_depth, i;
	size_t	subj_size;

	
	subj_size = sizeof(art->art_subj);
	al = ct.ct_al;
	for (al_pos = NULL; cl_art_walk(al, &al_pos, &art) == 0; ) {

		/* calculate how deep the thread of comments is*/
		p_art = art->art_parent;
		comm_depth = 0;
		while (p_art != NULL) {
			p_art = p_art->art_parent;
			comm_depth++;
		}
		art->art_comm_depth = comm_depth;

		if (comm_depth == 0) {
			strlcpy(art->art_subj, art->art_real_subj, subj_size);
			strlcpy(art->art_alt_subj, art->art_real_subj, subj_size);
			continue;
		}

		for (i = 0; i < (comm_depth - 1) * 2 && i < subj_size - 1; i++)
			art->art_subj[i] = ' ';
		art->art_subj[i] = '\0';
		strlcat(art->art_subj, "`->", subj_size);
		strlcpy(art->art_alt_subj, art->art_subj, subj_size);
		strlcat(art->art_alt_subj, art->art_real_subj, subj_size);
		if (strcmp(art->art_real_subj, art->art_parent->art_real_subj) != 0)
			strlcat(art->art_subj, "art->art_real_subj", subj_size);
		
		for (t_al_pos = al_pos;; ) {
			assert(cl_art_walk_back(al, &t_al_pos, &t_art) == 0);
			if (t_art == art->art_parent)
				break;
			t_art->art_subj[i] = '|';
			t_art->art_alt_subj[i] = '|';
		}
	}

	return 0;
}

int
art_count(void)
{
	return cl_art_count(ct.ct_al);
}

art_t *
art_get_num(int art_num)
{
	artl_t	*al, *al_pos;
	art_t	*art;
	int		i;

	al = ct.ct_al;
	al_pos = NULL;
	for (i = 0; i <= art_num; i++)
		assert(cl_art_walk(al, &al_pos, &art) == 0);
	return art;
}

char *
art_get_conf_name(void)
{
	return ct.ct_name;
}

static void
art_free(art_t *art)
{
	int	i;

	for (i = 0; art->art_text[i] != NULL; i++)
		free(art->art_text[i]);
	free(art->art_text);
	for (i = 0; art->art_header[i] != NULL; i++)
		free(art->art_header[i]);
	free(art->art_header);
	for (i = 0; art->art_footer[i] != NULL; i++)
		free(art->art_footer[i]);
	free(art->art_footer);
	free(art);
}

static char **
art_get_text(char *str)
{
	int		line_no, num_lines;
	char	**art_text, *p;

	p = str;
	num_lines = 0;
	for (p = str; *p != '\0'; p++) {
		if (*p == '\n')
			num_lines++;
	}
	num_lines++;
	if ((art_text = calloc(num_lines + 1, sizeof(char *))) == NULL)
		err(1, "calloc");

	line_no = 0;
	for (p = str;; p++) {
		if (*p == '\0') {
			if ((art_text[line_no] = strdup(str)) == NULL)
				err(1, "strdup");
			break;
		}
		if (*p == '\n') {
			*p = '\0';
			if ((art_text[line_no] = strdup(str)) == NULL)
				err(1, "strdup");
			line_no++;
			str = p + 1;
		}
	}

	return art_text;
}

static char **
art_get_header(struct rk_text_stat *rt)
{
	struct rk_conference *rc;
	struct rk_text_stat *rt2;
	char		**h_text;
	char		name[70];
	int			h_num, i;
	u_int32_t	rmi_type, rmi_num;

	/* XXX wasting a little bit of memory */
	if ((h_text =
		calloc(rt->rt_misc_info.rt_misc_info_len + 1, sizeof(char *))) == NULL)
		err(1, "calloc");

	h_num = 0;
	for (i = 0; i < rt->rt_misc_info.rt_misc_info_len; i++) {
		rmi_type = rt->rt_misc_info.rt_misc_info_val[i].rmi_type;
		if (rmi_type != recpt && rmi_type != cc_recpt &&
			rmi_type != comm_to && rmi_type != footn_to)
			continue;

		if ((h_text[h_num] = calloc(81, sizeof(char))) == NULL)
			err(1, "calloc");
		rmi_num = rt->rt_misc_info.rt_misc_info_val[i].rmi_numeric;

		if (rmi_type == recpt || rmi_type == cc_recpt) {
			rc = rk_confinfo(rmi_num);
			if (rc->rc_retval != 0)
				snprintf(name, sizeof(name), "Person %d (hemlig)", rmi_num);
			else
				snprintf(name, sizeof(name), "%s <%d>", rc->rc_name, rmi_num);
			free(rc);
			if (rmi_type == recpt)
				snprintf(h_text[h_num], 81, "Mottagare: %s", name);
			else
				snprintf(h_text[h_num], 81, "Extra kopia: %s", name);
		} else {
			/* rmi_type == comm_to || rmi_type == footn_to */
			rt2 = rk_textstat(rmi_num);
			if (rt2->rt_retval == 0) {
				rc = rk_confinfo(rt2->rt_author);
				if (rc->rc_retval != 0)
					snprintf(name, sizeof(name),
						"av Person %d (hemlig)", rt2->rt_author);
				else
					snprintf(name, sizeof(name),
						"av %s <%d>", rc->rc_name, rt2->rt_author);
				free(rc);
			} else
				name[0] = '\0';
			free(rt2);

			if (rmi_type == footn_to)
				snprintf(h_text[h_num], 81, "Fotnot till text %d %s",
					rmi_num, name);
			else
				snprintf(h_text[h_num], 81, "Kommentar till text %d %s",
					rmi_num, name);
		}
		h_num++;
	}
	return h_text;
}

static char **
art_get_footer(struct rk_text_stat *rt)
{
	struct rk_conference *rc;
	struct rk_text_stat *rt2;
	char		**f_text;
	char		name[70];
	int			f_num, i;
	u_int32_t	rmi_type, rmi_num;

	/* XXX wasting a little bit of memory */
	if ((f_text =
		calloc(rt->rt_misc_info.rt_misc_info_len + 1, sizeof(char *))) == NULL)
		err(1, "calloc");

	f_num = 0;
	for (i = 0; i < rt->rt_misc_info.rt_misc_info_len; i++) {
		rmi_type = rt->rt_misc_info.rt_misc_info_val[i].rmi_type;
		if (rmi_type != comm_in && rmi_type != footn_in)
			continue;

		if ((f_text[f_num] = calloc(81, sizeof(char))) == NULL)
			err(1, "calloc");
		rmi_num = rt->rt_misc_info.rt_misc_info_val[i].rmi_numeric;

		rt2 = rk_textstat(rmi_num);
		if (rt2->rt_retval == 0) {
			rc = rk_confinfo(rt2->rt_author);
			if (rc->rc_retval != 0)
				snprintf(name, sizeof(name),
					"av Person %d (hemlig)", rt2->rt_author);
			else
				snprintf(name, sizeof(name),
					"av %s <%d>", rc->rc_name, rt2->rt_author);
			free(rc);
		} else
			name[0] = '\0';
		free(rt2);

		if (rmi_type == footn_to)
			snprintf(f_text[f_num], 81, "Fotnot i text %d %s",
				rmi_num, name);
		else
			snprintf(f_text[f_num], 81, "Kommentar i text %d %s",
				rmi_num, name);
		f_num++;
	}
	return f_text;
}

time_t
art_get_time(struct rk_time *rt)
{
	struct tm tm;
	
	tm.tm_sec = rt->rt_seconds;
	tm.tm_min = rt->rt_minutes;
	tm.tm_hour = rt->rt_hours;
	tm.tm_mday = rt->rt_day;
	tm.tm_mon = rt->rt_month;
	tm.tm_year = rt->rt_year;
	tm.tm_isdst = rt->rt_is_dst;
	return mktime(&tm);
}
