/* $Id: articles.c,v 1.1 2000/10/15 11:59:39 jens Exp $ */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <err.h>

#include <rkom_proto.h>
#include <exported.h>

#include "articles.h"
#include "container.h"

static int art_add(conft_t *ct, u_int32_t local_id);
static int art_format_subjects(conft_t *ct);

conft_t *
art_get_arts(u_int32_t uid, u_int32_t conf_id)
{
	struct rk_conference *rc = NULL;
	struct rk_membership *rm = NULL;
	conft_t		*ct;
	u_int32_t	first_text, last_text;
	int			i;

	if ((ct = calloc(1, sizeof(conft_t))) == NULL)
		err(1, "calloc");
	ct->ct_uid = uid;
	ct->ct_cid = conf_id;

	rm = rk_membership(uid, conf_id);
	if (rm->rm_retval != 0) {
		warnx("rk_membership: failed");
		free(rm);
		free(ct);
		return NULL;
	}
	first_text = rm->rm_last_text_read + 1;
	free(rm);

	rc = rk_confinfo(conf_id);
	if (rc->rc_retval != 0) {
		warnx("rk_confinfo: failed");
		free(rc);
		free(ct);
		return NULL;
	}
	last_text = rc->rc_first_local_no + rc->rc_no_of_texts - 1;
	strlcpy(ct->ct_name, rc->rc_name, sizeof(ct->ct_name));
	free(rc);

	for (i = first_text; i <= last_text; i++)
		art_add(ct, i);

	art_format_subjects(ct);
	
	return ct;
}

int
art_add(conft_t *ct, u_int32_t local_id)
{
	struct rk_text_stat *rt;
	struct rk_conference *rc;
	art_t		*art, *p_art;
	char		*tb;
	char		buf[80];
	artl_t		*al = NULL, *al_pos, *oal_pos;
	u_int32_t	global_text;
	int			i;

	if (ct->ct_al == NULL) {
		if ((ct->ct_al = cl_art_init()) == NULL) {
			warn("cl_art_init");
			return NULL;
		}
		ct->ct_first_text = local_id;
	}
	al = ct->ct_al;


	global_text = rk_local_to_global(ct->ct_cid, local_id);

	/* get some info of text */
	rt = rk_textstat(global_text);
	if (rt->rt_retval != 0) {
		free(rt);
		return -1;
	}

	ct->ct_last_text = local_id;

	if ((art = calloc(1, sizeof(art_t))) == NULL)
		err(1, "calloc");
	art->art_id = global_text;
	art->art_no_of_lines = rt->rt_no_of_lines;

	/* get subject */
	tb = rk_gettext(global_text);
	for (i = 0; tb[i] != '\0' && tb[i] != '\n' && i < sizeof(buf) - 1; i++);
	tb[i] = '\0';
	strcpy(art->art_real_subj, tb);
	free(tb);

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
art_format_subjects(conft_t *ct)
{
	artl_t	*al, *al_pos;
	art_t	*art, *p_art, *oart;
	int		comm_depth, i;
	size_t	subj_size;

	
	subj_size = sizeof(art->art_subj);
	al = ct->ct_al;
	oart = NULL;
	for (al_pos = NULL; cl_art_walk(al, &al_pos, &art) == 0; oart = art) {

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
			continue;
		}

		for (i = 0; i < (comm_depth - 1) * 2 && i < subj_size - 1; i++)
			art->art_subj[i] = ' ';
		art->art_subj[i] = '\0';
		strlcat(art->art_subj, "`->", subj_size);
		if (oart == NULL || art->art_parent != oart->art_parent)
			continue;
		oart->art_subj[i] = '|';
	}

	return 0;
}
