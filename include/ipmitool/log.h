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

#ifndef IPMITOOL_LOG_H
#define IPMITOOL_LOG_H

#include <syslog.h>

/* sys/syslog.h:
 * LOG_EMERG       0       system is unusable
 * LOG_ALERT       1       action must be taken immediately
 * LOG_CRIT        2       critical conditions
 * LOG_ERR         3       error conditions
 * LOG_WARNING     4       warning conditions
 * LOG_NOTICE      5       normal but significant condition
 * LOG_INFO        6       informational
 * LOG_DEBUG       7       debug-level messages
 */

#define LOG_ERROR		LOG_ERR
#define LOG_WARN		LOG_WARNING

#define LOG_NAME_DEFAULT	"ipmitool"
#define LOG_MSG_LENGTH		1024

void log_init(const char * name, int isdaemon, int verbose);
void log_halt(void);
void log_level_set(int level);
int log_level_get(void);
void lprintf(int level, const char * format, ...);
void lperror(int level, const char * format, ...);

void ipmi_log(int level,const char* file, int lineno, const char* fmt,...);
void ipmi_buffer_log(int level, const char* file, int lineno, void* pbuf, int bufsize,const char* fmt,...);

#define IPMI_DEBUG(...)       do{ipmi_log(LOG_DEBUG,__FILE__,__LINE__,__VA_ARGS__);}while(0)
#define IPMI_INFO(...)        do{ipmi_log(LOG_INFO,__FILE__,__LINE__,__VA_ARGS__);}while(0)
#define IPMI_ERR(...)         do{ipmi_log(LOG_ERR,__FILE__,__LINE__,__VA_ARGS__);}while(0)
#define IPMI_EMERG(...)       do{ipmi_log(LOG_EMERG,__FILE__,__LINE__,__VA_ARGS__);}while(0)

#define IPMI_BUFFER_DEBUG(ptr,size,...)       do{ipmi_buffer_log(LOG_DEBUG,__FILE__,__LINE__,(void*)(ptr), (int)(size),__VA_ARGS__);}while(0)
#define IPMI_BUFFER_ERR(ptr,size,...)         do{ipmi_buffer_log(LOG_ERR,__FILE__,__LINE__,(void*)(ptr), (int)(size),__VA_ARGS__);}while(0)
#define IPMI_BUFFER_EMERG(ptr,size,...)       do{ipmi_buffer_log(LOG_EMERG,__FILE__,__LINE__,(void*)(ptr), (int)(size),__VA_ARGS__);}while(0)

#endif /*IPMITOOL_LOG_H*/

