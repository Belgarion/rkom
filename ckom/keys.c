/* $Id: keys.c,v 1.1 2000/10/15 21:39:15 jens Exp $ */

#include <sys/param.h>

#include "keys.h"
#include "container.h"

#include <assert.h>

SKIP_TYPE(kdef, key_def_t)

keybind_t *
keybind_init(void)
{
	return skl_kdef_init();
}

void
keybind_free(keybind_t *kdh)
{
	skl_kdef_free(kdh, NULL);
}

void
keybind_add(keybind_t *kdh, key_def_t *kd)
{
	assert(kd->kd_id != KD_UNBOUND);
	/* in case we are overriding a key */
	skl_kdef_rem(kdh, kd->kd_key, NULL);

	assert(skl_kdef_ins(kdh, kd->kd_key, kd) == 0);
}

int
keybind_lookup(keybind_t *kdh, int key)
{
	key_def_t	*kd;

	if (skl_kdef_find(kdh, key, &kd) < 0)
		return KD_UNBOUND;
	return kd->kd_id;
}
