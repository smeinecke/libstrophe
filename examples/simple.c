/* simple.c
** libstrophe XMPP client library -- basic usage example
**
** Copyright (C) 2005-2009 Collecta, Inc.
**
**  This software is provided AS-IS with no warranty, either express
**  or implied.
**
**  This software is distributed under license and may not be copied,
**  modified or distributed except as expressly authorized under the
**  terms of the license contained in the file LICENSE.txt in this
**  distribution.
*/

/* simple message example
**
** This example was provided by Elmo Todurov <todurov@gmail.com>
**
** This progam just sends a simple message to the given Jabber ID.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <strophe.h>

#define BUF_SIZE 1024
//#define DEBUG 0
char buffer[BUF_SIZE];
size_t contentSize = 1; // includes NULL

struct msgdata {
	char* to;
	char* msg;
};

/* define a handler for connection events */
void conn_handler(xmpp_conn_t * const conn, const xmpp_conn_event_t status,
		  const int error, xmpp_stream_error_t * const stream_error,
		  void * const userdata)
{
    xmpp_ctx_t *ctx = xmpp_conn_get_context(conn);
    struct msgdata* msg = (struct msgdata*)userdata;

    if (status == XMPP_CONN_CONNECT) {
        xmpp_stanza_t* pres;
#ifdef DEBUG
        fprintf(stderr, "DEBUG: connected\n");
#endif

        /* Send initial <presence/> so that we appear online to contacts */
        //pres = xmpp_stanza_new(ctx);
        //xmpp_stanza_set_name(pres, "presence");
        //xmpp_send(conn, pres);
        //xmpp_stanza_release(pres);

        /* Send a message */
        xmpp_send_simple_message(conn, msg->to, msg->msg);
        free(msg);

        /* Causes the conn_handler to be executed, where the event loop gets shut down */
        xmpp_disconnect(conn);
    }
    else {
#ifdef DEBUG
    	fprintf(stderr, "DEBUG: disconnected\n");
#endif
	    xmpp_stop(ctx);
    }
}

int main(int argc, char **argv)
{
    xmpp_ctx_t *ctx;
    xmpp_conn_t *conn;
    xmpp_log_t *log;
    char *jid, *pass;

    /* take a jid and password on the command line */
    if (argc != 4) {
    	fprintf(stderr, "Usage: ./simple <jid> <pass> <to>\n\n");
	    return 1;
    }

    jid = argv[1];
    pass = argv[2];

    char *content = malloc(sizeof(char) * BUF_SIZE);
    if(content == NULL){
    	fprintf(stderr, "Failed to allocate content\n");
        exit(1);
    }
    content[0] = '\0'; // make null-terminated

    while (fgets(buffer, BUF_SIZE, stdin))
    {
        char *old = content;
        contentSize += strlen(buffer);
        content = realloc(content, contentSize);
        if(content == NULL)
        {
            fprintf(stderr, "Failed to reallocate content\n");
            free(old);
            exit(2);
        }
        strcat(content, buffer);
    }

    if(ferror(stdin))
    {
        free(content);
        fprintf(stderr, "Error reading from stdin.\n");
        exit(3);
    }

    if (content[contentSize-2] == '\n') {
        content[contentSize-2] = '\0';
        contentSize -= 1;
    }

    /* init library */
    xmpp_initialize();

    /* create a context */
#ifdef DEBUG
    log = xmpp_get_default_logger(XMPP_LEVEL_DEBUG); /* pass XMPP_LEVEL_ERROR instead to silence output */
#else
    log = xmpp_get_default_logger(XMPP_LEVEL_INFO);
#endif
    ctx = xmpp_ctx_new(NULL, log);

    /* create a connection */
    conn = xmpp_conn_new(ctx);

    /* setup authentication information */
    xmpp_conn_set_jid(conn, jid);
    xmpp_conn_set_pass(conn, pass);

    /* Prepare the message for sending */
    struct msgdata* msg = (struct msgdata*)malloc(sizeof(struct msgdata));
    msg->to = argv[3];
    msg->msg = content;

    /* initiate connection */
    xmpp_connect_client(conn, NULL, 0, conn_handler, msg);

    /* enter the event loop -
       our connect handler will trigger an exit */
    xmpp_run(ctx);

    free(content);

    /* release our connection and context */
    xmpp_conn_release(conn);
    xmpp_ctx_free(ctx);

    /* final shutdown of the library */
    xmpp_shutdown();

    return 0;
}
