
#include <sys/types.h>
#include <sys/uio.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "rkomsupport.h"
#include "backend.h"
#include "rtype.h"


struct uarea {
	char *key;
	char *elem;
} *uarea;

/*
 * Return an asciz string based on a hollerith argument.
 * Returns NULL if something failed.
 */
static char *
getstr(char *str)
{
	int i, num = atoi(str);

	while (isspace((int)*str) || isdigit((int)*str))
		str++;
	if (*str++ != 'H')
		return NULL;
	for (i = 0; i < num; i++)
		if (str[i] == 0)
			return NULL;
	str[num] = 0;
	return str;
}

static int
cnth(char *str)
{
	int i, j;

	for (j = i = 0; str[i]; i++)
		if (str[i] == 'H')
			j++;
	return j;
}

/*
 * Take a asciz string consisting of many hollerith strings and
 * split it up in an array of uarea (of asciz strings).
 */
static struct uarea *
splitup(char *str, int *cnt, int n)
{
	int i, j, mlen;
	struct uarea *spole;
	char *c;

	j = cnth(str);
	mlen = strlen(str);
	spole = calloc(sizeof(struct uarea), j + 1);
	for (j = i = 0; i < mlen && str[i]; j++) {
		c = getstr(&str[i]);
		if (c == NULL) {
			free(spole);
			return 0;
		}
		spole[j].key = c;
		i = (c - str) + strlen(c) + 1;
		if (n) {
			c = getstr(&str[i]);
			if (c == NULL) {
				free(spole);
				return 0;
			}
			spole[j].elem = c;
			i = (c - str) + strlen(c) + 1;
		}
	}
	if (cnt)
		*cnt = j;
	return spole;
}

/*
 * Take a asciz string consisting of a complete user area and
 * split it up in an array of uarea (of asciz strings).
 */
static struct uarea *
sort_uarea(char *str)
{
	struct uarea *p, *key, *elem;
	int cnt, cnt2, i;
	char *c, *d;

	c = getstr(str);
	if (c == NULL)
		return 0;
	d = c + strlen(c) + 1;
	key = splitup(c, &cnt, 0);
	if (key == NULL)
		return 0;
	elem = splitup(d, &cnt2, 0);
	if (cnt != cnt2)
		return 0;
	p = calloc(sizeof(struct uarea), cnt + 3);
	for (i = 0; i < cnt; i++) {
		p[i].key = key[i].key;
		p[i].elem = elem[i].key;
	}
	free(key);
	free(elem);
	return p;
		
}


static struct uarea *
get_uarea(int uid)
{
	static char *upole;
	struct rk_person *p;

	if (upole)
		free(upole), upole = NULL;
	p = rk_persinfo(myuid);
	if (p->rp_user_area == 0)
		return NULL;
	if (send_reply("25 %d 0 2000000\n", p->rp_user_area)) {
		get_int();
		get_eat('\n');
		return 0;
	}
	upole = get_string();
	get_accept('\n');
	return sort_uarea(upole);
}


struct rk_uarea *
rk_get_uarea(char *str)
{
	static struct rk_uarea rku;
	static struct rk_val *rv;
	struct uarea *vec;
	int i, j;

	if (uarea)
		free(uarea);
	if (rv)
		free(rv);
	rv = NULL;
	uarea = NULL;

	if (myuid == 0) {
		komerr = 6;
		return NULL;
	}
	if ((uarea = get_uarea(myuid)) == NULL) {
		komerr = 11;
		return NULL;
	}

	/* Check if there is any entry in the uarea matching our string */
	for (i = 0; uarea[i].key; i++)
		if (strcmp(uarea[i].key, str) == 0)
			break;
	if (uarea[i].key == NULL) {
		komerr = 12;
		return NULL;
	}

	vec = splitup(uarea[i].elem, &i, 1);
	if (vec == NULL) {
		komerr = 12;
		return NULL;
	}

	rv = calloc(sizeof(struct rk_val), i + 1);
	rku.ru_val.ru_val_len = i;
	rku.ru_val.ru_val_val = rv;
	for (j = 0; j < i; j++) {
		rv[j].rv_var = vec[j].key;
		rv[j].rv_val = vec[j].elem;
	}
	free(vec);
	return &rku;
}


int32_t
rk_set_uarea(char *str, struct rk_uarea *u)
{
	struct rk_person *p;
	struct uarea *ua;
	struct rk_val *v;
	char *udata, *narea, *hdr, *tmp;
	int i, tot, len, no;

	/* First make an uarea data string */
	v = u->ru_val.ru_val_val;
	len = u->ru_val.ru_val_len;
	for (i = tot = 0; i < len; i++)
		tot += (strlen(v->rv_var) + strlen(v->rv_val));
	tot += (len * 20);
	udata = alloca(tot);
	*udata = 0;
	narea = alloca(tot + 20);
	*narea = 0;
	for (i = 0; i < len; i++) {
		sprintf(udata, " %ldH%s %ldH%s\n", (long)strlen(v[i].rv_var),
		    v[i].rv_var, (long)strlen(v[i].rv_val), v[i].rv_val);
		strcat(narea, udata);
	}

	ua = get_uarea(myuid);
	if (ua == NULL) /* No uarea */
		ua = calloc(sizeof(struct uarea), 2);

	for (i = 0; ua[i].key; i++)
		if (strcmp(ua[i].key, str) == 0)
			break;
	if (ua[i].key == NULL) /* Must add our uarea */
		ua[i].key = str;

	/* Ok, just switch uarea info */
	ua[i].elem = narea;

	/* Make header */
	for (i = len = 0; ua[i].key; i++)
		len += strlen(ua[i].key) + 10;
	tmp = alloca(len);
	*tmp = 0;
	for (i = 0; ua[i].key; i++) {
		sprintf(udata, " %ldH%s", (long)strlen(ua[i].key), ua[i].key);
		strcat(tmp, udata);
	}
	hdr = alloca(len);
	sprintf(hdr, " %ldH%s", (long)strlen(tmp), tmp);

	/* Make data area */
	for (i = len = 0; ua[i].key; i++)
		len += strlen(ua[i].elem) + 10;
	udata = alloca(len);
	tmp = alloca(len);
	*udata = 0;
	for (i = 0; ua[i].key; i++) {
		sprintf(tmp, " %ldH%s", (long)strlen(ua[i].elem), ua[i].elem);
		strcat(udata, tmp);
	}

	/* Make the long string */
	if (send_reply("86 %ldH%s %s 0 { } 0 { }\n",
	    (long)(strlen(udata) + strlen(hdr) + 1), hdr, udata)) {
		i = get_int();
		get_eat('\n');
		return i;
	}
	no = get_int();
	get_accept('\n');
	if (send_reply("57 %d %d\n", myuid, no)) {
		i = get_int();
		get_eat('\n');
		return i;
	}
	p = rk_persinfo(myuid);
	p->rp_user_area = no;
	get_accept('\n');
	return 0;
}
