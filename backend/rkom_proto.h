/* Generated by spcgen, do not edit */

#ifndef RKOM_PROTO_H
#define RKOM_PROTO_H

#include <sys/types.h>

/* Begin verbatim code */

int rkom_connect(char *server, char *frontend, char *os_username);
void rkom_logout(void);
enum Misc_Info_types {
	recpt, cc_recpt, comm_to, comm_in, footn_to, footn_in,
	loc_no, rec_time, sent_by, sentat, bcc_recpt = 15
};
#define	RAI_TAG_CONTENT_TYPE	1
#define RAI_TAG_FAST_REPLY	2
#define RAI_TAG_CREATING_SW	15
#define	RK_CONF_TYPE_RD_PROT	1000
#define RK_CONF_TYPE_ORIGINAL	100
#define RK_CONF_TYPE_SECRET	10
#define RK_CONF_TYPE_LETTERBOX	1
#define MATCHCONF_PERSON	1
#define MATCHCONF_CONF		2
#define WHO_VISIBLE	1
#define WHO_INVISIBLE	2

/* End verbatim code */

void spc_set_read_fd(int fd);
int spc_get_read_fd(void);
void spc_set_write_fd(int fd);
int spc_get_write_fd(void);
void spc_read_msg(void *buf, size_t nbytes);
u_int32_t spc_read_msglen(void);
u_int32_t spc_read_fun_num(void);
void spc_write_msg(void *buf, size_t nbytes);
void spc_write_fun_call(u_int32_t fun_num,
						void *buf, size_t nbytes);



/* Struct definitions */

struct rk_time {
		u_int8_t	rt_seconds;
		u_int8_t	rt_minutes;
		u_int8_t	rt_hours;
		u_int8_t	rt_day;
		u_int8_t	rt_month;
		u_int16_t	rt_year;
		u_int8_t	rt_day_of_week;
		u_int16_t	rt_day_of_year;
		u_int8_t	rt_is_dst;
};

struct rk_misc_info {
		u_int32_t	rmi_type;
		u_int32_t	rmi_numeric;
		struct rk_time	rmi_time;
};

struct rk_aux_item {
		u_int32_t	rai_aux_no;
		u_int32_t	rai_tag;
		u_int32_t	rai_creator;
		struct rk_time	rai_created_at;
		u_int32_t	rai_flags;
		u_int32_t	inherit_limit;
		char *	rai_data;
};

struct rk_aux_item_input {
		u_int32_t	raii_tag;
		u_int32_t	raii_flags;
		u_int32_t	inherit_limit;
		char *	raii_data;
};

struct rk_text_stat {
		int32_t	rt_retval;
		struct rk_time	rt_time;
		u_int32_t	rt_author;
		u_int32_t	rt_no_of_lines;
		u_int32_t	rt_no_of_chars;
		u_int32_t	rt_no_of_marks;
	struct {
		u_int32_t	rt_misc_info_len;
		struct rk_misc_info	*rt_misc_info_val;
	} rt_misc_info;
	struct {
		u_int32_t	rt_aux_item_len;
		struct rk_aux_item	*rt_aux_item_val;
	} rt_aux_item;
};

struct rk_membership {
		int32_t	rm_retval;
		u_int32_t	rm_position;
		struct rk_time	rm_last_time_read;
		u_int32_t	rm_conference;
		u_int32_t	rm_priority;
		u_int32_t	rm_last_text_read;
	struct {
		u_int32_t	rm_read_texts_len;
		u_int32_t	*rm_read_texts_val;
	} rm_read_texts;
		u_int32_t	rm_added_by;
		struct rk_time	rm_added_at;
		u_int32_t	rm_type;
};

struct rk_conference {
		int32_t	rc_retval;
		char *	rc_name;
		u_int32_t	rc_type;
		struct rk_time	rc_creation_time;
		struct rk_time	rc_last_written;
		u_int32_t	rc_creator;
		u_int32_t	rc_presentation;
		u_int32_t	rc_supervisor;
		u_int32_t	rc_permitted_submitters;
		u_int32_t	rc_super_conf;
		u_int32_t	rc_msg_of_day;
		u_int32_t	rc_nice;
		u_int32_t	rc_keep_commented;
		u_int32_t	rc_no_of_members;
		u_int32_t	rc_first_local_no;
		u_int32_t	rc_no_of_texts;
		u_int32_t	rc_expire;
};

struct rk_person {
		int32_t	rp_retval;
		char *	rp_username;
		u_int32_t	rp_privileges;
		u_int32_t	rp_flags;
		struct rk_time	rp_last_login;
		u_int32_t	rp_user_area;
		u_int32_t	rp_total_time_present;
		u_int32_t	rp_sessions;
		u_int32_t	rp_created_lines;
		u_int32_t	rp_created_bytes;
		u_int32_t	rp_read_texts;
		u_int32_t	rp_no_of_text_fetches;
		u_int32_t	rp_created_persons;
		u_int32_t	rp_created_confs;
		u_int32_t	rp_first_created_local_no;
		u_int32_t	rp_no_of_created_texts;
		u_int32_t	rp_no_of_marks;
		u_int32_t	rp_no_of_confs;
};

struct rk_confinfo {
		char *	rc_name;
		u_int32_t	rc_type;
		u_int32_t	rc_conf_no;
};

struct rk_confinfo_retval {
	struct {
		u_int32_t	rcr_ci_len;
		struct rk_confinfo	*rcr_ci_val;
	} rcr_ci;
};

struct rk_dynamic_session_info {
		u_int32_t	rds_session;
		u_int32_t	rds_person;
		u_int32_t	rds_conf;
		u_int32_t	rds_idletime;
		u_int32_t	rds_flags;
		char *	rds_doing;
};

struct rk_dynamic_session_info_retval {
	struct {
		u_int32_t	rdv_rds_len;
		struct rk_dynamic_session_info	*rdv_rds_val;
	} rdv_rds;
};

struct rk_unreadconfval {
		int32_t	ru_retval;
	struct {
		u_int32_t	ru_confs_len;
		u_int32_t	*ru_confs_val;
	} ru_confs;
};

struct rk_memberconflist {
		int32_t	rm_retval;
	struct {
		u_int32_t	rm_confs_len;
		u_int32_t	*rm_confs_val;
	} rm_confs;
};

struct rk_text_retval {
		int32_t	rtr_status;
		u_int32_t	rtr_textnr;
};

struct rk_text_info {
		char *	rti_text;
	struct {
		u_int32_t	rti_misc_len;
		struct rk_misc_info	*rti_misc_val;
	} rti_misc;
	struct {
		u_int32_t	rti_input_len;
		struct rk_aux_item_input	*rti_input_val;
	} rti_input;
};

struct rk_async {
		u_int32_t	ra_type;
		u_int32_t	ra_conf;
		u_int32_t	ra_pers;
		u_int32_t	ra_text;
		char *	ra_message;
		char *	ra_message2;
};

struct rk_marks {
		u_int32_t	rm_text;
		u_int8_t	rm_type;
};

struct rk_mark_retval {
		int32_t	rmr_retval;
	struct {
		u_int32_t	rmr_marks_len;
		struct rk_marks	*rmr_marks_val;
	} rmr_marks;
};

struct rk_val {
		char *	rv_var;
		char *	rv_val;
};

struct rk_uarea {
		int32_t	ru_retval;
	struct {
		u_int32_t	ru_val_len;
		struct rk_val	*ru_val_val;
	} ru_val;
};



/* Declaration of get_size_encoded_XXX functions */

size_t get_size_encoded_string(char ** var);
size_t get_size_encoded_int8_t(int8_t* var);
size_t get_size_encoded_u_int8_t(u_int8_t* var);
size_t get_size_encoded_int16_t(int16_t* var);
size_t get_size_encoded_u_int16_t(u_int16_t* var);
size_t get_size_encoded_int32_t(int32_t* var);
size_t get_size_encoded_u_int32_t(u_int32_t* var);
size_t get_size_encoded_int64_t(int64_t* var);
size_t get_size_encoded_u_int64_t(u_int64_t* var);
size_t get_size_encoded_float(float* var);
size_t get_size_encoded_double(double* var);
size_t get_size_encoded_rk_time(struct rk_time* var);
size_t get_size_encoded_rk_misc_info(struct rk_misc_info* var);
size_t get_size_encoded_rk_aux_item(struct rk_aux_item* var);
size_t get_size_encoded_rk_aux_item_input(struct rk_aux_item_input* var);
size_t get_size_encoded_rk_text_stat(struct rk_text_stat* var);
size_t get_size_encoded_rk_membership(struct rk_membership* var);
size_t get_size_encoded_rk_conference(struct rk_conference* var);
size_t get_size_encoded_rk_person(struct rk_person* var);
size_t get_size_encoded_rk_confinfo(struct rk_confinfo* var);
size_t get_size_encoded_rk_confinfo_retval(struct rk_confinfo_retval* var);
size_t get_size_encoded_rk_dynamic_session_info(struct rk_dynamic_session_info* var);
size_t get_size_encoded_rk_dynamic_session_info_retval(struct rk_dynamic_session_info_retval* var);
size_t get_size_encoded_rk_unreadconfval(struct rk_unreadconfval* var);
size_t get_size_encoded_rk_memberconflist(struct rk_memberconflist* var);
size_t get_size_encoded_rk_text_retval(struct rk_text_retval* var);
size_t get_size_encoded_rk_text_info(struct rk_text_info* var);
size_t get_size_encoded_rk_async(struct rk_async* var);
size_t get_size_encoded_rk_marks(struct rk_marks* var);
size_t get_size_encoded_rk_mark_retval(struct rk_mark_retval* var);
size_t get_size_encoded_rk_val(struct rk_val* var);
size_t get_size_encoded_rk_uarea(struct rk_uarea* var);


/* Declaration of get_size_decoded_XXX functions */

size_t get_size_decoded_string(char **enc_buf,
			size_t *dyn_len, size_t *stat_len);
size_t get_size_decoded_int8_t(char **enc_buf,
			size_t *dyn_len, size_t *stat_len);
size_t get_size_decoded_u_int8_t(char **enc_buf,
			size_t *dyn_len, size_t *stat_len);
size_t get_size_decoded_int16_t(char **enc_buf,
			size_t *dyn_len, size_t *stat_len);
size_t get_size_decoded_u_int16_t(char **enc_buf,
			size_t *dyn_len, size_t *stat_len);
size_t get_size_decoded_int32_t(char **enc_buf,
			size_t *dyn_len, size_t *stat_len);
size_t get_size_decoded_u_int32_t(char **enc_buf,
			size_t *dyn_len, size_t *stat_len);
size_t get_size_decoded_int64_t(char **enc_buf,
			size_t *dyn_len, size_t *stat_len);
size_t get_size_decoded_u_int64_t(char **enc_buf,
			size_t *dyn_len, size_t *stat_len);
size_t get_size_decoded_float(char **enc_buf,
			size_t *dyn_len, size_t *stat_len);
size_t get_size_decoded_double(char **enc_buf,
			size_t *dyn_len, size_t *stat_len);
size_t get_size_decoded_rk_time(char **enc_buf,
			size_t *dyn_len, size_t *stat_len);
size_t get_size_decoded_rk_misc_info(char **enc_buf,
			size_t *dyn_len, size_t *stat_len);
size_t get_size_decoded_rk_aux_item(char **enc_buf,
			size_t *dyn_len, size_t *stat_len);
size_t get_size_decoded_rk_aux_item_input(char **enc_buf,
			size_t *dyn_len, size_t *stat_len);
size_t get_size_decoded_rk_text_stat(char **enc_buf,
			size_t *dyn_len, size_t *stat_len);
size_t get_size_decoded_rk_membership(char **enc_buf,
			size_t *dyn_len, size_t *stat_len);
size_t get_size_decoded_rk_conference(char **enc_buf,
			size_t *dyn_len, size_t *stat_len);
size_t get_size_decoded_rk_person(char **enc_buf,
			size_t *dyn_len, size_t *stat_len);
size_t get_size_decoded_rk_confinfo(char **enc_buf,
			size_t *dyn_len, size_t *stat_len);
size_t get_size_decoded_rk_confinfo_retval(char **enc_buf,
			size_t *dyn_len, size_t *stat_len);
size_t get_size_decoded_rk_dynamic_session_info(char **enc_buf,
			size_t *dyn_len, size_t *stat_len);
size_t get_size_decoded_rk_dynamic_session_info_retval(char **enc_buf,
			size_t *dyn_len, size_t *stat_len);
size_t get_size_decoded_rk_unreadconfval(char **enc_buf,
			size_t *dyn_len, size_t *stat_len);
size_t get_size_decoded_rk_memberconflist(char **enc_buf,
			size_t *dyn_len, size_t *stat_len);
size_t get_size_decoded_rk_text_retval(char **enc_buf,
			size_t *dyn_len, size_t *stat_len);
size_t get_size_decoded_rk_text_info(char **enc_buf,
			size_t *dyn_len, size_t *stat_len);
size_t get_size_decoded_rk_async(char **enc_buf,
			size_t *dyn_len, size_t *stat_len);
size_t get_size_decoded_rk_marks(char **enc_buf,
			size_t *dyn_len, size_t *stat_len);
size_t get_size_decoded_rk_mark_retval(char **enc_buf,
			size_t *dyn_len, size_t *stat_len);
size_t get_size_decoded_rk_val(char **enc_buf,
			size_t *dyn_len, size_t *stat_len);
size_t get_size_decoded_rk_uarea(char **enc_buf,
			size_t *dyn_len, size_t *stat_len);


/* Declaration of encode_XXX functions */

size_t encode_string(char ** var, char ** enc_buf);
size_t encode_int8_t(int8_t* var, char ** enc_buf);
size_t encode_u_int8_t(u_int8_t* var, char ** enc_buf);
size_t encode_int16_t(int16_t* var, char ** enc_buf);
size_t encode_u_int16_t(u_int16_t* var, char ** enc_buf);
size_t encode_int32_t(int32_t* var, char ** enc_buf);
size_t encode_u_int32_t(u_int32_t* var, char ** enc_buf);
size_t encode_int64_t(int64_t* var, char ** enc_buf);
size_t encode_u_int64_t(u_int64_t* var, char ** enc_buf);
size_t encode_float(float* var, char ** enc_buf);
size_t encode_double(double* var, char ** enc_buf);
size_t encode_rk_time(struct rk_time* var, char ** enc_buf);
size_t encode_rk_misc_info(struct rk_misc_info* var, char ** enc_buf);
size_t encode_rk_aux_item(struct rk_aux_item* var, char ** enc_buf);
size_t encode_rk_aux_item_input(struct rk_aux_item_input* var, char ** enc_buf);
size_t encode_rk_text_stat(struct rk_text_stat* var, char ** enc_buf);
size_t encode_rk_membership(struct rk_membership* var, char ** enc_buf);
size_t encode_rk_conference(struct rk_conference* var, char ** enc_buf);
size_t encode_rk_person(struct rk_person* var, char ** enc_buf);
size_t encode_rk_confinfo(struct rk_confinfo* var, char ** enc_buf);
size_t encode_rk_confinfo_retval(struct rk_confinfo_retval* var, char ** enc_buf);
size_t encode_rk_dynamic_session_info(struct rk_dynamic_session_info* var, char ** enc_buf);
size_t encode_rk_dynamic_session_info_retval(struct rk_dynamic_session_info_retval* var, char ** enc_buf);
size_t encode_rk_unreadconfval(struct rk_unreadconfval* var, char ** enc_buf);
size_t encode_rk_memberconflist(struct rk_memberconflist* var, char ** enc_buf);
size_t encode_rk_text_retval(struct rk_text_retval* var, char ** enc_buf);
size_t encode_rk_text_info(struct rk_text_info* var, char ** enc_buf);
size_t encode_rk_async(struct rk_async* var, char ** enc_buf);
size_t encode_rk_marks(struct rk_marks* var, char ** enc_buf);
size_t encode_rk_mark_retval(struct rk_mark_retval* var, char ** enc_buf);
size_t encode_rk_val(struct rk_val* var, char ** enc_buf);
size_t encode_rk_uarea(struct rk_uarea* var, char ** enc_buf);


/* Declaration of decode_XXX functions */

size_t decode_string(char * * var, char **dyn_buf, char **enc_buf,
			size_t *dyn_len, size_t *stat_len);
size_t decode_int8_t(int8_t * var, char **dyn_buf, char **enc_buf,
			size_t *dyn_len, size_t *stat_len);
size_t decode_u_int8_t(u_int8_t * var, char **dyn_buf, char **enc_buf,
			size_t *dyn_len, size_t *stat_len);
size_t decode_int16_t(int16_t * var, char **dyn_buf, char **enc_buf,
			size_t *dyn_len, size_t *stat_len);
size_t decode_u_int16_t(u_int16_t * var, char **dyn_buf, char **enc_buf,
			size_t *dyn_len, size_t *stat_len);
size_t decode_int32_t(int32_t * var, char **dyn_buf, char **enc_buf,
			size_t *dyn_len, size_t *stat_len);
size_t decode_u_int32_t(u_int32_t * var, char **dyn_buf, char **enc_buf,
			size_t *dyn_len, size_t *stat_len);
size_t decode_int64_t(int64_t * var, char **dyn_buf, char **enc_buf,
			size_t *dyn_len, size_t *stat_len);
size_t decode_u_int64_t(u_int64_t * var, char **dyn_buf, char **enc_buf,
			size_t *dyn_len, size_t *stat_len);
size_t decode_float(float * var, char **dyn_buf, char **enc_buf,
			size_t *dyn_len, size_t *stat_len);
size_t decode_double(double * var, char **dyn_buf, char **enc_buf,
			size_t *dyn_len, size_t *stat_len);
size_t decode_rk_time(struct rk_time * var, char **dyn_buf, char **enc_buf,
			size_t *dyn_len, size_t *stat_len);
size_t decode_rk_misc_info(struct rk_misc_info * var, char **dyn_buf, char **enc_buf,
			size_t *dyn_len, size_t *stat_len);
size_t decode_rk_aux_item(struct rk_aux_item * var, char **dyn_buf, char **enc_buf,
			size_t *dyn_len, size_t *stat_len);
size_t decode_rk_aux_item_input(struct rk_aux_item_input * var, char **dyn_buf, char **enc_buf,
			size_t *dyn_len, size_t *stat_len);
size_t decode_rk_text_stat(struct rk_text_stat * var, char **dyn_buf, char **enc_buf,
			size_t *dyn_len, size_t *stat_len);
size_t decode_rk_membership(struct rk_membership * var, char **dyn_buf, char **enc_buf,
			size_t *dyn_len, size_t *stat_len);
size_t decode_rk_conference(struct rk_conference * var, char **dyn_buf, char **enc_buf,
			size_t *dyn_len, size_t *stat_len);
size_t decode_rk_person(struct rk_person * var, char **dyn_buf, char **enc_buf,
			size_t *dyn_len, size_t *stat_len);
size_t decode_rk_confinfo(struct rk_confinfo * var, char **dyn_buf, char **enc_buf,
			size_t *dyn_len, size_t *stat_len);
size_t decode_rk_confinfo_retval(struct rk_confinfo_retval * var, char **dyn_buf, char **enc_buf,
			size_t *dyn_len, size_t *stat_len);
size_t decode_rk_dynamic_session_info(struct rk_dynamic_session_info * var, char **dyn_buf, char **enc_buf,
			size_t *dyn_len, size_t *stat_len);
size_t decode_rk_dynamic_session_info_retval(struct rk_dynamic_session_info_retval * var, char **dyn_buf, char **enc_buf,
			size_t *dyn_len, size_t *stat_len);
size_t decode_rk_unreadconfval(struct rk_unreadconfval * var, char **dyn_buf, char **enc_buf,
			size_t *dyn_len, size_t *stat_len);
size_t decode_rk_memberconflist(struct rk_memberconflist * var, char **dyn_buf, char **enc_buf,
			size_t *dyn_len, size_t *stat_len);
size_t decode_rk_text_retval(struct rk_text_retval * var, char **dyn_buf, char **enc_buf,
			size_t *dyn_len, size_t *stat_len);
size_t decode_rk_text_info(struct rk_text_info * var, char **dyn_buf, char **enc_buf,
			size_t *dyn_len, size_t *stat_len);
size_t decode_rk_async(struct rk_async * var, char **dyn_buf, char **enc_buf,
			size_t *dyn_len, size_t *stat_len);
size_t decode_rk_marks(struct rk_marks * var, char **dyn_buf, char **enc_buf,
			size_t *dyn_len, size_t *stat_len);
size_t decode_rk_mark_retval(struct rk_mark_retval * var, char **dyn_buf, char **enc_buf,
			size_t *dyn_len, size_t *stat_len);
size_t decode_rk_val(struct rk_val * var, char **dyn_buf, char **enc_buf,
			size_t *dyn_len, size_t *stat_len);
size_t decode_rk_uarea(struct rk_uarea * var, char **dyn_buf, char **enc_buf,
			size_t *dyn_len, size_t *stat_len);


/* Declaration of client and server functions */

int32_t rk_login(u_int32_t arg0, char * arg1);
int32_t rk_login_server(u_int32_t arg0, char * arg1);
int32_t rk_whatido(char * arg0);
int32_t rk_whatido_server(char * arg0);
struct rk_unreadconfval * rk_unreadconf(u_int32_t arg0);
struct rk_unreadconfval * rk_unreadconf_server(u_int32_t arg0);
struct rk_memberconflist * rk_memberconf(u_int32_t arg0);
struct rk_memberconflist * rk_memberconf_server(u_int32_t arg0);
struct rk_confinfo_retval * rk_matchconf(char * arg0, u_int8_t arg1);
struct rk_confinfo_retval * rk_matchconf_server(char * arg0, u_int8_t arg1);
struct rk_conference * rk_confinfo(u_int32_t arg0);
struct rk_conference * rk_confinfo_server(u_int32_t arg0);
struct rk_person * rk_persinfo(u_int32_t arg0);
struct rk_person * rk_persinfo_server(u_int32_t arg0);
void rk_alive(void);
void rk_alive_server(void);
struct rk_membership * rk_membership(u_int32_t arg0, u_int32_t arg1);
struct rk_membership * rk_membership_server(u_int32_t arg0, u_int32_t arg1);
struct rk_dynamic_session_info_retval * rk_vilka(u_int32_t arg0, u_int32_t arg1);
struct rk_dynamic_session_info_retval * rk_vilka_server(u_int32_t arg0, u_int32_t arg1);
char * rk_client_version(u_int32_t arg0);
char * rk_client_version_server(u_int32_t arg0);
char * rk_client_name(u_int32_t arg0);
char * rk_client_name_server(u_int32_t arg0);
struct rk_time * rk_time(void);
struct rk_time * rk_time_server(void);
u_int32_t rk_next_unread(u_int32_t arg0, u_int32_t arg1);
u_int32_t rk_next_unread_server(u_int32_t arg0, u_int32_t arg1);
u_int32_t rk_local_to_global(u_int32_t arg0, u_int32_t arg1);
u_int32_t rk_local_to_global_server(u_int32_t arg0, u_int32_t arg1);
int32_t rk_mark_read(u_int32_t arg0, u_int32_t arg1);
int32_t rk_mark_read_server(u_int32_t arg0, u_int32_t arg1);
int32_t rk_is_read(u_int32_t arg0);
int32_t rk_is_read_server(u_int32_t arg0);
struct rk_text_stat * rk_textstat(u_int32_t arg0);
struct rk_text_stat * rk_textstat_server(u_int32_t arg0);
char * rk_gettext(u_int32_t arg0);
char * rk_gettext_server(u_int32_t arg0);
int32_t rk_set_last_read(u_int32_t arg0, u_int32_t arg1);
int32_t rk_set_last_read_server(u_int32_t arg0, u_int32_t arg1);
int32_t rk_change_conference(u_int32_t arg0);
int32_t rk_change_conference_server(u_int32_t arg0);
int32_t rk_add_member(u_int32_t arg0, u_int32_t arg1, u_int8_t arg2, u_int16_t arg3, u_int32_t arg4);
int32_t rk_add_member_server(u_int32_t arg0, u_int32_t arg1, u_int8_t arg2, u_int16_t arg3, u_int32_t arg4);
int32_t rk_sub_member(u_int32_t arg0, u_int32_t arg1);
int32_t rk_sub_member_server(u_int32_t arg0, u_int32_t arg1);
int32_t rk_add_rcpt(u_int32_t arg0, u_int32_t arg1, u_int32_t arg2);
int32_t rk_add_rcpt_server(u_int32_t arg0, u_int32_t arg1, u_int32_t arg2);
int32_t rk_sub_rcpt(u_int32_t arg0, u_int32_t arg1);
int32_t rk_sub_rcpt_server(u_int32_t arg0, u_int32_t arg1);
struct rk_text_retval * rk_create_text(struct rk_text_info * arg0);
struct rk_text_retval * rk_create_text_server(struct rk_text_info * arg0);
struct rk_async * rk_async(void);
struct rk_async * rk_async_server(void);
int32_t rk_send_msg(u_int32_t arg0, char * arg1);
int32_t rk_send_msg_server(u_int32_t arg0, char * arg1);
struct rk_mark_retval * rk_getmarks(void);
struct rk_mark_retval * rk_getmarks_server(void);
int32_t rk_setmark(u_int32_t arg0, u_int8_t arg1);
int32_t rk_setmark_server(u_int32_t arg0, u_int8_t arg1);
int32_t rk_unmark(u_int32_t arg0);
int32_t rk_unmark_server(u_int32_t arg0);
struct rk_uarea * rk_get_uarea(char * arg0);
struct rk_uarea * rk_get_uarea_server(char * arg0);
int32_t rk_set_uarea(char * arg0, struct rk_uarea * arg1);
int32_t rk_set_uarea_server(char * arg0, struct rk_uarea * arg1);
int32_t rk_setpass(u_int32_t arg0, char * arg1, char * arg2);
int32_t rk_setpass_server(u_int32_t arg0, char * arg1, char * arg2);
int32_t rk_change_name(u_int32_t arg0, char * arg1);
int32_t rk_change_name_server(u_int32_t arg0, char * arg1);
int32_t rk_set_presentation(u_int32_t arg0, struct rk_text_info * arg1);
int32_t rk_set_presentation_server(u_int32_t arg0, struct rk_text_info * arg1);
int32_t rk_delete_text(u_int32_t arg0);
int32_t rk_delete_text_server(u_int32_t arg0);
void rk_sync(void);
void rk_sync_server(void);
int32_t rk_create_conf(char * arg0, u_int32_t arg1);
int32_t rk_create_conf_server(char * arg0, u_int32_t arg1);
int32_t rk_set_motd(u_int32_t arg0, struct rk_text_info * arg1);
int32_t rk_set_motd_server(u_int32_t arg0, struct rk_text_info * arg1);


/* Declaration of server functions */

void spc_process_request(void);



#endif /* RKOM_PROTO_H */