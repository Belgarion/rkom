/* $Id: rkom_proto.spc,v 1.15 2000/11/18 10:35:58 ragge Exp $ */

/* Exported prototypes */
%hfile
int rkom_connect(char *server, char *frontend, char *os_username);
void rkom_logout(void);
%end
/*
 * Time as defined in the lyskom protocol. Variables are kept
 * in as small variables as possible.
 */
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

/*
 * Additional text information.
 */
%hfile
enum Misc_Info_types {
	recpt, cc_recpt, comm_to, comm_in, footn_to, footn_in,
	loc_no, rec_time, sent_by, sentat, bcc_recpt = 15
};
%end
struct rk_misc_info {
	u_int32_t	rmi_type;	/* Type of misc field */
	u_int32_t	rmi_numeric;	/* Used if type is numeric */
	struct rk_time	rmi_time;	/* Used if type is time */
};

/* Gurka */
struct rk_aux_item {
	u_int32_t	rai_aux_no;
	u_int32_t	rai_tag;
	u_int32_t	rai_creator;
	u_int32_t	rai_created_at;
	u_int32_t	rai_flags;
	u_int32_t	inherit_limit;
	string		rai_data;
};

/*
 * Information about a text.
 */
struct rk_text_stat {
	int32_t		rt_retval;
	struct rk_time	rt_time;
	u_int32_t	rt_author;
	u_int32_t	rt_no_of_lines;
	u_int32_t	rt_no_of_chars;
	u_int32_t	rt_no_of_marks;
	struct rk_misc_info rt_misc_info<>;
	struct rk_aux_item rt_aux_item<>;
};

/*
 * Information about a member in a conference.
 */
struct rk_membership {
	int32_t		rm_retval;
	u_int32_t	rm_position;
	struct rk_time	rm_last_time_read;
	u_int32_t	rm_conference;
	u_int32_t	rm_priority;
	u_int32_t	rm_last_text_read;
	u_int32_t	rm_read_texts<>;
	u_int32_t	rm_added_by;
	struct rk_time	rm_added_at;
	u_int32_t	rm_type;
};

/*
 * Description of a conference.
 */
struct rk_conference {
	int32_t		rc_retval;
	string		rc_name;
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

/*
 * Struct describing a person.
 */
struct rk_person {
	int32_t		rp_retval;
	string		rp_username;
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
	string		rc_name;
	u_int32_t	rc_type;
	u_int32_t	rc_conf_no;
};

struct rk_confinfo_retval {
	struct rk_confinfo rcr_ci<>;
};

struct rk_dynamic_session_info {
	u_int32_t	rds_session;
	u_int32_t	rds_person;
	u_int32_t	rds_conf;
	u_int32_t	rds_idletime;
	u_int32_t	rds_flags;
	string		rds_doing;
};

struct rk_dynamic_session_info_retval {
	struct rk_dynamic_session_info rdv_rds<>;
};

struct rk_unreadconfval {
	int32_t		ru_retval;
	u_int32_t	ru_confs<>;
};

struct rk_memberconflist {
	int32_t		rm_retval;
	u_int32_t	rm_confs<>;
};

struct rk_text_retval {
	int32_t		rtr_status;
	u_int32_t	rtr_textnr;
};

struct rk_text_info {
	string		rti_text;
	struct rk_misc_info rti_misc<>;
};

/* Async message info struct */
struct rk_async {
	u_int32_t	ra_type;
	u_int32_t	ra_conf;
	u_int32_t	ra_sender;
	string		ra_message;
	string		ra_message2;
};

struct rk_marks {
	u_int32_t	rm_text;
	u_int8_t	rm_type;
};

struct rk_mark_retval {
	int32_t		rmr_retval;
	struct rk_marks	rmr_marks<>;
};

struct rk_val {
	string		rv_var;
	string		rv_val;
};

struct rk_uarea {
	int32_t		ru_retval;
	struct rk_val	ru_val<>;
};

/*
 * Login a user to the system.
 * Arguments are (userid, password).
 */
int32_t rk_login(u_int32_t, string);

/*
 * Set the "what-I-am-doing" string.
 */
int32_t rk_whatido(string);

/*
 * Get conferences with unread texts.
 */
struct rk_unreadconfval rk_unreadconf(u_int32_t);

/*
 * Get the conferences where the user is member.
 */
struct rk_memberconflist rk_memberconf(u_int32_t);

/*
 * Do a conference/user name match.
 * Return an array of matched confinfo structs.
 */
%hfile
#define MATCHCONF_PERSON	1
#define MATCHCONF_CONF		2
%end
struct rk_confinfo_retval rk_matchconf(string, u_int8_t);

/*
 * Get the conference struct based on the conference number.
 */
struct rk_conference	rk_confinfo(u_int32_t);

/*
 * Get the person struct based on the person number.
 */
struct rk_person	rk_persinfo(u_int32_t);

/*
 * Tell the server that the user is alive.
 * XXX - should have void arguments.
 */
int32_t rk_alive(int32_t);

/*
 * Get membership information about a user in a conference.
 * Args are uid, mid.
 */
struct rk_membership rk_membership(u_int32_t, u_int32_t);

/*
 * Return the users that are logged on to the system.
 * Args are (idlesecs, flags).
 */
%hfile
#define WHO_VISIBLE	1
#define WHO_INVISIBLE	2
%end
struct rk_dynamic_session_info_retval rk_vilka(u_int32_t, u_int32_t);

/*
 * Get the client version. This call directly corresponds to Lyskom call #71.
 */
string rk_client_version(u_int32_t);

/*
 * Get the client name. This call directly corresponds to Lyskom call #70.
 */
string rk_client_name(u_int32_t);

/*
 * Ask for the server time.
 * XXX - should have void argument.
 */
struct rk_time rk_time(int32_t);

/*
 * Get next unread message in a conference.
 * Args are (conference, uid) and returns the conference local text number.
 */
u_int32_t rk_next_unread(u_int32_t, u_int32_t);

/*
 * Converts a local text number to a global number.
 * Args are (conference, textnumber) and returns the global text number.
 */
u_int32_t rk_local_to_global(u_int32_t, u_int32_t);

/*
 * Marks a text as read in a conference.
 * Args are (conference, textnumber) and returns a lyskom error number.
 */
int32_t rk_mark_read(u_int32_t, u_int32_t);

/*
 * Returns 1 if the given global number is read, otherwise 0.
 */
int32_t rk_is_read(u_int32_t);

/*
 * Get statistics about a text.
 */
struct rk_text_stat rk_textstat(u_int32_t);

/*
 * Fetches the text body from server.
 */
string rk_gettext(u_int32_t);

/*
 * Set last read read local text in a conference.
 * Args are (conference, local textnumber) and returns a lyskom error number.
 */
int32_t rk_set_last_read(u_int32_t, u_int32_t);

/*
 * Change to new working conference.
 * Arg is (conference), returns a lyskom error number.
 */
int32_t rk_change_conference(u_int32_t);

/*
 * Adds/changes a member in a conference.
 * Args are (conf, uid, prio, where, flags) and returns a lyskom error number.
 */
int32_t rk_add_member(u_int32_t, u_int32_t, u_int8_t, u_int16_t, u_int32_t);

/*
 * Remove a member from a conference. Corresponds to call #15.
 * Args are (conf, pers) and returns a lyskom error number.
 */
int32_t rk_sub_member(u_int32_t, u_int32_t);

/*
 * Puts a text in the conferences described in the misc_info struct.
 * Arg in rk_text_info and returns a status struct.
 */
struct	rk_text_retval rk_create_text(struct rk_text_info);

/*
 * Returns an async message if there is one to get.
 * XXX - no argument to this one.
 */
struct rk_async rk_async(u_int32_t);

/*
 * Sends a message to a user, a conference or to all logged in.
 * Args are (user, string). Returns null.
 */
int32_t rk_send_msg(u_int32_t, string);

/*
 * Get marked texts for the current user.
 * XXX - no argument to this one.
 */
struct rk_mark_retval rk_getmarks(u_int32_t);

/*
 * Mark a text for the current user.
 * Args are (text, markno). Returns a normal Lyskom error code.
 */
int32_t rk_setmark(u_int32_t, u_int8_t);

/*
 * Unmark a text for the current user.
 * Arg is (text). Returns a normal Lyskom error code.
 */
int32_t rk_unmark(u_int32_t);

/*
 * Get the user area specified by the string.
 * Arg is (identifier). Returns the uarea struct or a normal Lyskom error code.
 */
struct rk_uarea rk_get_uarea(string);

/*
 * Set the user area specified by the string.
 * Returns a normal Lyskom error code.
 */
int32_t rk_set_uarea(string, struct rk_uarea);
