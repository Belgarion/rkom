/*	$Id: backend.h,v 1.14 2003/09/17 10:51:34 ragge Exp $	*/
/*
 * Prototypes for the rkom backend internal functions.
 */

/* Forward declarations */
struct conference;
struct person;
struct membership;
struct tm;
struct rk_aux_item;
struct rk_text_stat;
struct rk_text_info;
struct rk_aux_item_input;
struct rk_modifyconfinfo;

/* Lyskom protocol helper functions, in rkom_lyskom.c */
int	send_reply(char *msg);
char	get_char(void);
int	get_int(void);
void	get_eat(char c);
void	put_char(char c);
void	put_string(char *str);
void	get_accept(char ch);
char	*get_string(void);
void	send_callback(char *msg, int arg, void(*)(int, int));
int	get_bitfield(void);

/* Main loop, in rkom_subr.c */
struct	rk_server *rkom_connect(char *, char *, char *, char *);
int	rkom_loop(void);
void	rkom_command(void);
void	rkom_logout(void);

/* Communication fe/be, in rkom_be.c */
int	fgrw(int cmd, void *arg, int arglen, void **reply, int *replylen);
void	bgreceive(void);
void	bgsend(int retval, int len, void *reply);
struct iovec;
void	bgsendv(int retval, int elem, struct iovec *iov);

/* Parsing of messages, in rkom_beparse.c */
void	rkom_beparse(int cmd, void *svar, int len);
void	reread_text_stat_bg(int text);
void	readin_textstat(struct rk_text_stat *ts);
struct	rk_time *rk_time(void);
void	rk_alive(void);
struct	rk_dynamic_session_info_retval *rk_vilka(u_int32_t, u_int32_t);
struct	rk_conference *rk_confinfo(u_int32_t);
struct	rk_person *rk_persinfo(u_int32_t);
struct	rk_confinfo_retval *rk_matchconf(char *name, u_int8_t flags);
int32_t rk_login(u_int32_t userid, char *passwd);
int32_t rk_whatido(char *args);
struct	rk_unreadconfval *rk_unreadconf(u_int32_t uid);
struct	rk_uconference *rk_uconfinfo(u_int32_t mid);
struct	rk_membership *rk_membership(u_int32_t uid, u_int32_t mid);
char *	rk_client_name(u_int32_t vers);
char *	rk_client_version(u_int32_t vers);
char *	rk_gettext(u_int32_t nr);
struct	rk_text_stat *rk_textstat(u_int32_t nr);
struct	rk_text_retval *rk_create_text(struct rk_text_info *rti);
struct	rk_mark_retval *rk_getmarks(void);
int32_t rk_setmark(u_int32_t text, u_int8_t type);
int32_t rk_unmark(u_int32_t text);
int32_t rk_send_msg(u_int32_t dest, char *string);
int32_t rk_setpass(u_int32_t uid, char *oldpass, char *newpass);
int32_t rk_change_name(u_int32_t uid, char *newname);
int32_t rk_set_presentation(u_int32_t conf, struct rk_text_info *rti);
int32_t rk_add_rcpt(u_int32_t text, u_int32_t conf, u_int32_t type);
int32_t rk_sub_rcpt(u_int32_t text, u_int32_t conf);
int32_t rk_delete_text(u_int32_t text);
int32_t rk_set_motd(u_int32_t conf, struct rk_text_info *rti);
int32_t rk_add_text_info(u_int32_t textno, struct rk_aux_item_input *raii);


/* conference requests, in rkom_conf.c */
int	get_conf_stat(int confno, struct rk_conference **confer);
int	get_pers_stat(int persno, struct rk_person **pers);
int	get_membership(int uid, int conf, struct rk_membership **member);
void	conf_set_high_local(int conf, int local, int global);
void	reread_conf_stat_bg(int conf);
void	newname(int);
void	invalidate_local(struct rk_text_stat *ts);
int32_t rk_change_conference(u_int32_t conf);
int32_t rk_is_read(u_int32_t nr);
u_int32_t rk_next_unread(u_int32_t conf, u_int32_t uid);
u_int32_t rk_local_to_global(u_int32_t conf, u_int32_t local);
int32_t rk_mark_read(u_int32_t conf, u_int32_t local);
int32_t rk_set_last_read(u_int32_t conf, u_int32_t local);
int32_t rk_add_member(u_int32_t, u_int32_t, u_int8_t, u_int16_t, u_int32_t);
int32_t rk_sub_member(u_int32_t conf, u_int32_t uid);
struct rk_memberconflist * rk_memberconf(u_int32_t uid);
void rk_sync(void);
int32_t rk_create_conf(char *name, u_int32_t btype);
int32_t rk_create_person(char *name, char *passwd, u_int32_t btype);
int32_t rk_delete_conf(u_int32_t conf);
int32_t rk_modify_conf_info(struct rk_modifyconfinfo *rkm);


/* Helper functions, in rkom_helpers.c */
void	read_in_time(struct rk_time *t);
void	read_in_aux_item(struct rk_aux_item *a);
char *	bitfield2str(int bf);

/* Async functions, in rkom_async.c */
void	async(int);
void	async_collect(void);
struct	rk_async *rk_async(void);

/* rkom_uarea.c */
struct rk_uarea *rk_get_uarea(char *str);
int32_t rk_set_uarea(char *str, struct rk_uarea *u);

/* variables */
extern	int readfd;	/* Get messages from frontend */
extern	int writefd;	/* Write to frontend */
extern	int myuid;	/* Current active uid */

enum Misc_Info_types {
	recpt, cc_recpt, comm_to, comm_in, footn_to, footn_in,
	loc_no, rec_time, sent_by, sentat, bcc_recpt = 15
};
#define	RAI_TAG_CONTENT_TYPE	1
#define RAI_TAG_FAST_REPLY	2
#define RAI_TAG_CROSS_REF	3
#define RAI_TAG_NO_COMMENTS	4
#define RAI_TAG_PERS_COMMENTS	5
#define RAI_TAG_REQ_CONF	6
#define RAI_TAG_READ_CONFIRM	7
#define RAI_TAG_REDIRECT	8
#define RAI_TAG_X_FACE		9
#define RAI_TAG_ALTNAME		10
#define RAI_TAG_PGP_SIGN	11
#define RAI_TAG_PGP_PUBLIC_KEY	12
#define RAI_TAG_EMAIL_ADDRSSS	13
#define RAI_TAG_FAQ_TEXT	14
#define RAI_TAG_CREATING_SW	15
#define	RK_CONF_TYPE_RD_PROT	0x80
#define RK_CONF_TYPE_ORIGINAL	0x40
#define RK_CONF_TYPE_SECRET	0x20
#define RK_CONF_TYPE_LETTERBOX	0x10
#define RK_CONF_TYPE_ALLOW_ANON	0x08
#define RK_CONF_TYPE_NO_SECRET	0x04
#define MATCHCONF_PERSON	1
#define MATCHCONF_CONF		2
#define WHO_VISIBLE	1
#define WHO_INVISIBLE	2

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
	struct {
		u_int32_t	rc_aux_item_len;
		struct rk_aux_item	*rc_aux_item_val;
	} rc_aux_item;
};

struct rk_uconference {
		int32_t	ru_retval;
		char *	ru_name;
		u_int32_t	ru_type;
		u_int32_t	ru_highest_local_no;
		u_int32_t	ru_nice;
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

struct rk_modifyconfinfo {
		u_int32_t	rkm_conf;
	struct {
		u_int32_t	rkm_delete_len;
		u_int32_t	*rkm_delete_val;
	} rkm_delete;
	struct {
		u_int32_t	rkm_add_len;
		struct rk_aux_item_input	*rkm_add_val;
	} rkm_add;
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

struct rk_server {
		int32_t	rs_retval;
		int32_t	rs_proto;
		char *	rs_servtype;
		char *	rs_version;
};
