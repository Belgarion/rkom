/* $Id: articles.h,v 1.1 2000/10/15 11:59:39 jens Exp $ */
#ifndef ARTICLES_H
#define ARTICLES_H

#include "container.h"

#define	ART_UNREAD	((u_int32_t)1)
#define	ART_MARKED	((u_int32_t)1<<1)
#define	ART_FOOTN	((u_int32_t)1<<2)

typedef struct article art_t;
struct article {
	u_int32_t	art_id;
	u_int32_t	art_flags;
	u_int32_t	art_no_of_lines;
	char		art_subj[80];
	char		art_real_subj[80];
	char		art_from[80];
	int			art_comm_depth;
	u_int32_t	art_date;
	art_t		*art_parent;
};


typedef struct cl_art_que artl_t;
CL_TYPE(art, art_t)

typedef struct conf_texts conft_t;
struct conf_texts {
	artl_t		*ct_al;
	char		ct_name[80];
	u_int32_t	ct_uid;
	u_int32_t	ct_cid;
	u_int32_t	ct_first_text;
	u_int32_t	ct_last_text;
};


conft_t *art_get_arts(u_int32_t uid, u_int32_t conf_id);

#endif /* ARTICLES_H */
