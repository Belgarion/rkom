/* $Id: rkom_proto.spc,v 1.2 2000/10/08 14:26:29 ragge Exp $ */

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
 * Information of an article of imediate interest.
 */
struct rk_article {
	u_int32_t	ra_id;
	u_int32_t	ra_recpt<>;
	u_int32_t	ra_cc_recpt<>;
	u_int32_t	ra_bcc_recpt<>;
	u_int32_t	ra_sent_by;
	struct rk_time ra_sent_at;
	u_int32_t	ra_comm_to<>;
	u_int32_t	ra_comm_in<>;
	u_int32_t	ra_footn_to<>;
	u_int32_t	ra_footn_in<>;
	string		ra_subject;
	string		ra_text;
};

struct rk_id_list {
	u_int32_t	ri_id<>;
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

struct rk_membership_args {
	u_int32_t	rma_uid;
	u_int32_t	rma_mid;
};

/*
 * Get membership information about a user in a conference.
 */
struct rk_membership rk_membership(struct rk_membership_args);

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
 * Get the conference struct based on the conference number.
 */
struct rk_conference	rk_confinfo(u_int32_t);

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

/*
 * Get the person struct based on the person number.
 */
struct rk_person	rk_persinfo(u_int32_t);
/*
 * Do a conference name match.
 * Return an array of matched confinfo structs.
 */
struct rk_matchconfargs {
	string		rm_name;
	int8_t		rm_flags;
};

struct rk_confinfo {
	string		rc_name;
	u_int32_t	rc_type;
	u_int32_t	rc_conf_no;
};

struct rk_confinfo_retval {
	struct rk_confinfo rcr_ci<>;
};

struct rk_confinfo_retval rk_matchconf(struct rk_matchconfargs);

/*
 * Check who is logged on to the server.
 */
struct rk_vilka_args {
	u_int32_t	rva_secs;
	u_int32_t	rva_flags;
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

struct rk_dynamic_session_info_retval rk_vilka(struct rk_vilka_args);


/*
 * Login a user to the system.
 */
struct rk_loginargs {
	u_int32_t	rk_userid;
	string		rk_passwd;
};

int32_t rk_login(struct rk_loginargs);

/*
 * Set the "what-i-am-doing" string.
 */
struct rk_whatidoargs {
	string		rw_whatido;
};

int32_t rk_whatido(struct rk_whatidoargs);

/*
 * Get conferences with unread texts.
 */
struct rk_unreadconfval {
	u_int32_t	ru_confs<>;
};

struct rk_unreadconfval rk_unreadconf(u_int32_t);

/*
 * Tell the server that the user is alive.
 * XXX - should have void arguments.
 */
int32_t rk_alive(int32_t);

/*
 * Ask for the server time.
 * XXX - should have void argument.
 */
struct rk_time rk_time(int32_t);



/* get article with the specific id */
struct rk_article rk_get_article(u_int32_t);

/*
 * Get a list of id numers of articles that are unread in
 * the conference with the specific id.
 */
struct rk_id_list rk_get_unread(u_int32_t);

/* Get the name of the specific id. */
string rk_get_name(u_int32_t);
