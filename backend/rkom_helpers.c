
#include <sys/types.h>

#include <time.h>

#include "rkomsupport.h"
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

void
read_in_aux_item(struct rk_aux_item *a)
{
	a->rai_aux_no = get_int();
	a->rai_tag = get_int();
	a->rai_creator = get_int();
	read_in_time(&a->rai_created_at);
	a->rai_flags = get_int();
	a->inherit_limit = get_int();
	a->rai_data = get_string();
}
