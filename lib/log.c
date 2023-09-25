/*
 * Copyright (c) 2003 Sun Microsystems, Inc.  All Rights Reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * Redistribution of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 
 * Redistribution in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 
 * Neither the name of Sun Microsystems, Inc. or the names of
 * contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * 
 * This software is provided "AS IS," without a warranty of any kind.
 * ALL EXPRESS OR IMPLIED CONDITIONS, REPRESENTATIONS AND WARRANTIES,
 * INCLUDING ANY IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE OR NON-INFRINGEMENT, ARE HEREBY EXCLUDED.
 * SUN MICROSYSTEMS, INC. ("SUN") AND ITS LICENSORS SHALL NOT BE LIABLE
 * FOR ANY DAMAGES SUFFERED BY LICENSEE AS A RESULT OF USING, MODIFYING
 * OR DISTRIBUTING THIS SOFTWARE OR ITS DERIVATIVES.  IN NO EVENT WILL
 * SUN OR ITS LICENSORS BE LIABLE FOR ANY LOST REVENUE, PROFIT OR DATA,
 * OR FOR DIRECT, INDIRECT, SPECIAL, CONSEQUENTIAL, INCIDENTAL OR
 * PUNITIVE DAMAGES, HOWEVER CAUSED AND REGARDLESS OF THE THEORY OF
 * LIABILITY, ARISING OUT OF THE USE OF OR INABILITY TO USE THIS SOFTWARE,
 * EVEN IF SUN HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <errno.h>
#include <stdarg.h>
#include <string.h>

#include <ipmitool/log.h>

struct logpriv_s {
	char * name;
	int daemon;
	int level;
};
struct logpriv_s *logpriv;

static void log_reinit(void)
{
	log_init(NULL, 0, 0);
}

void lprintf_inner(int level, const char* file,int lineno, const char * format, ...)
{
	static char logmsg[LOG_MSG_LENGTH];
	char* sptr=NULL;
	int leftlen = LOG_MSG_LENGTH;
	int ret;
	va_list vptr;

	if (!logpriv)
		log_reinit();

	if (logpriv->level < level)
		return;

	sptr = logmsg;
	ret = snprintf(sptr,leftlen,"[%s:%d] ",file,lineno);
	sptr += ret;
	leftlen -= ret;
	va_start(vptr, format);
	ret = vsnprintf(sptr, leftlen, format, vptr);
	va_end(vptr);
	sptr += ret;
	leftlen -= ret;

	if (logpriv->daemon)
		syslog(level, "%s", logmsg);
	else
		fprintf(stderr, "%s\n", logmsg);
	return;
}

void lperror(int level, const char * format, ...)
{
	static char logmsg[LOG_MSG_LENGTH];
	va_list vptr;

	if (!logpriv)
		log_reinit();

	if (logpriv->level < level)
		return;

	va_start(vptr, format);
	vsnprintf(logmsg, LOG_MSG_LENGTH, format, vptr);
	va_end(vptr);

	if (logpriv->daemon)
		syslog(level, "%s: %s", logmsg, strerror(errno));
	else
		fprintf(stderr, "%s: %s\n", logmsg, strerror(errno));
	return;
}

/*
 * open connection to syslog if daemon
 */
void log_init(const char * name, int isdaemon, int verbose)
{
	if (logpriv)
		return;

	logpriv = malloc(sizeof(struct logpriv_s));
	if (!logpriv)
		return;

	if (name)
		logpriv->name = strdup(name);
	else
		logpriv->name = strdup(LOG_NAME_DEFAULT);

	if (!logpriv->name)
		fprintf(stderr, "ipmitool: malloc failure\n");

	logpriv->daemon = isdaemon;
	logpriv->level = verbose + LOG_NOTICE;

	if (logpriv->daemon)
		openlog(logpriv->name, LOG_CONS, LOG_LOCAL4);
}

/*
 * stop syslog logging if daemon mode,
 * free used memory that stored log service
 */
void log_halt(void)
{
	if (!logpriv)
		return;

	if (logpriv->name) {
		free(logpriv->name);
		logpriv->name = NULL;
	}

	if (logpriv->daemon)
		closelog();

	free(logpriv);
	logpriv = NULL;
}

int log_level_get(void)
{
	return logpriv->level;
}

void log_level_set(int level)
{
	logpriv->level = level;
}

static int st_ipmi_loglevel= -1;
static FILE* st_ipmi_logfp = NULL;

const char* get_level_str(int level)
{
	if (level <= LOG_EMERG) {
		return "EMERGENCY";
	} else if (level <= LOG_ERR) {
		return "ERROR";
	} else if (level <= LOG_WARNING) {
		return "WARN";
	} else if (level <= LOG_INFO) {
		return "INFO";
	}
	return "DEBUG";
}

int get_ipmi_level() 
{
	static int st_ipmi_inited = 0;
	if (st_ipmi_inited == 0) {
		int level=0;
		char* levelstr = NULL;
		char* file = NULL;
		levelstr = getenv("IPMI_LOGLEVEL");
		if (levelstr != NULL) {
			level = atoi(levelstr);
		}

		file = getenv("IPMI_LOGFILE");
		if (file != NULL) {
			st_ipmi_logfp = fopen(file,"w+");
		}
		if (st_ipmi_logfp == NULL) {
			st_ipmi_logfp = stderr;
		}
		st_ipmi_loglevel = level;

	}
	return st_ipmi_loglevel;
}

int check_ipmi_loglevel(int level)
{
	int getlevel = get_ipmi_level();
	if (getlevel >= level) {
		return 1;
	}
	return 0;
}


void ipmi_log(int level,const char* file, int lineno, const char* fmt,...)
{
	va_list ap;

	if (check_ipmi_loglevel(level) == 0) {
		return ;
	}

	va_start(ap,fmt);
	fprintf(st_ipmi_logfp,"[%s:%d]<%s> ", file, lineno,get_level_str(level));
	vfprintf(st_ipmi_logfp,fmt,ap);
	fprintf(st_ipmi_logfp,"\n");
	fflush(st_ipmi_logfp);
	return;
}


void ipmi_buffer_log(int level, const char* file, int lineno, void* pbuf, int bufsize,const char* fmt,...)
{
	va_list ap;
	unsigned char* pptr= (unsigned char*)pbuf;
	int i;
	unsigned char* plast = pptr;

	if (check_ipmi_loglevel(level) == 0) {
		return ;
	}


	fprintf(st_ipmi_logfp,"[%s:%d]<%s> ",file , lineno,get_level_str(level));
	fprintf(st_ipmi_logfp,"[%p] size[0x%x:%d]", pbuf, bufsize,bufsize);
	if (fmt != NULL) {
		va_start(ap, fmt);
		vfprintf(st_ipmi_logfp,fmt, ap);
	}
	for (i=0;i<bufsize;i++) {
		if ((i % 16) == 0) {
			if (i > 0) {
				fprintf(st_ipmi_logfp,"    ");
				while(plast != pptr) {
					if (*plast >= ' ' && *plast <= '~') {
						fprintf(st_ipmi_logfp,"%c", *plast);
					} else {
						fprintf(st_ipmi_logfp, ".");
					}
					plast ++;
				}
			}
			fprintf(st_ipmi_logfp,"\n0x%08x", i);
		}
		fprintf(st_ipmi_logfp," 0x%02x", *pptr);
		pptr ++;
	}

	if (pptr != plast) {
		while((i % 16) !=0 ) {
			fprintf(st_ipmi_logfp, "     ");
			i ++;
		}
		fprintf(st_ipmi_logfp,"    ");
		while(plast != pptr) {
			if (*plast >= ' ' && *plast <= '~') {
				fprintf(st_ipmi_logfp, "%c", *plast);
			} else {
				fprintf(st_ipmi_logfp, ".");
			}
			plast ++;
		}
	}
	fprintf(st_ipmi_logfp,"\n");
	fflush(st_ipmi_logfp);
	return;
}