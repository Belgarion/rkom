/* $Id: rkom_proto.spc,v 1.1 2000/10/07 11:36:56 jens Exp $ */

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

struct rk_conf {
	u_int32_t	rc_id;
	string		rc_name;
	string		rc_creater;
	/* plus lite annat stuff */
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

/* get article with the specific id */
struct rk_article rk_get_article(u_int32_t);

/*
 * Get a list of id numers of articles that are unread in
 * the conference with the specific id.
 */
struct rk_id_list rk_get_unread(u_int32_t);

/* Get the name of the specific id. */
string rk_get_name(u_int32_t);
