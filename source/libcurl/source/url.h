#ifndef __URL_H
#define __URL_H
/***************************************************************************
 *                                  _   _ ____  _
 *  Project                     ___| | | |  _ \| |
 *                             / __| | | | |_) | |
 *                            | (__| |_| |  _ <| |___
 *                             \___|\___/|_| \_\_____|
 *
 * Copyright (C) 1998 - 2005, Daniel Stenberg, <daniel@haxx.se>, et al.
 *
 * This software is licensed as described in the file COPYING, which
 * you should have received as part of this distribution. The terms
 * are also available at http://curl.haxx.se/docs/copyright.html.
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 * $Id: url.h,v 1.24 2006-04-10 15:00:54 bagder Exp $
 ***************************************************************************/

#include <stdarg.h> /* to make sure we have ap_list */

/*
 * Prototypes for library-wide functions provided by url.c
 */

CURLcode Curl_open(struct SessionHandle **curl);
CURLcode Curl_setopt(struct SessionHandle *data, CURLoption option,
                     va_list arg);
CURLcode Curl_close(struct SessionHandle *data); /* opposite of curl_open() */
CURLcode Curl_connect(struct SessionHandle *, struct connectdata **,
                      bool *async, bool *protocol_connect);
CURLcode Curl_async_resolved(struct connectdata *conn,
                             bool *protocol_connect);
CURLcode Curl_do(struct connectdata **, bool *done);
CURLcode Curl_do_more(struct connectdata *);
CURLcode Curl_done(struct connectdata **, CURLcode);
CURLcode Curl_disconnect(struct connectdata *);
CURLcode Curl_protocol_connect(struct connectdata *conn, bool *done);
CURLcode Curl_protocol_connecting(struct connectdata *conn, bool *done);
CURLcode Curl_protocol_doing(struct connectdata *conn, bool *done);
void Curl_safefree(void *ptr);


int Curl_protocol_getsock(struct connectdata *conn,
                          curl_socket_t *socks,
                          int numsocks);
int Curl_doing_getsock(struct connectdata *conn,
                       curl_socket_t *socks,
                       int numsocks);

#if 0
CURLcode Curl_protocol_fdset(struct connectdata *conn,
                             fd_set *read_fd_set,
                             fd_set *write_fd_set,
                             int *max_fdp);
CURLcode Curl_doing_fdset(struct connectdata *conn,
                          fd_set *read_fd_set,
                          fd_set *write_fd_set,
                          int *max_fdp);
#endif

#endif
