
#include <sys/types.h>

#include <time.h>

#include "rkom_proto.h"
#include "backend.h"

void
read_in_time(struct rk_time *t)
{
	t->rt_seconds = get_int();
	t->rt_minutes = get_int();
	t->rt_hours = get_int();
	t->rt_day = get_int();
	t->rt_month = get_int();
	t->rt_year = get_int();
	t->rt_day_of_week = get_int();
	t->rt_day_of_year = get_int();
	t->rt_is_dst = get_int();
}

#if 0
void
read_in_aux_item(struct aux_item *a)
{
	a->aux_no = get_int();
	a->tag = get_int();
	a->creator = get_int();
	read_in_time(&a->created_at);
	a->flags = get_int();
	a->inherit_limit = get_int();
	a->data = get_string();
}
#endif
