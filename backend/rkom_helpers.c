
#include <sys/types.h>

#include <time.h>

#include "exported.h"
#include "backend.h"

void
read_in_time(struct tm *t)
{
	t->tm_sec = get_int();
	t->tm_min = get_int();
	t->tm_hour = get_int();
	t->tm_mday = get_int();
	t->tm_mon = get_int();
	t->tm_year = get_int();
	t->tm_wday = get_int();
	t->tm_yday = get_int();
	t->tm_isdst = get_int();
}

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
