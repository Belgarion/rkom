/* $Id: keys.h,v 1.1 2000/10/15 21:39:15 jens Exp $ */
#ifndef KEYS_H
#define KEYS_H

#define CNTRL(x) ((int)(0xf&(x)))

typedef struct key_def key_def_t;
struct key_def {
	int		kd_key;
	int		kd_id;
	char	*kd_name;
};

#define KD_UNBOUND 0

typedef struct skip_kdef_list keybind_t;

keybind_t *keybind_init(void);
void keybind_free(keybind_t *);
void keybind_add(keybind_t *, key_def_t *);
int keybind_lookup(keybind_t *, int);


#endif /*KEYS_H*/
