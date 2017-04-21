#include "../lib/mongoose/mongoose.h"
#include "http_client.h"
#include "log.h"

int s_exit_flag = 0;

void ev_handler(struct mg_connection *nc, int ev, void *ev_data) {
	 struct http_message *hm = (struct http_message *) ev_data;
	  switch (ev) {
	    case MG_EV_CONNECT:
	      if (* (int *) ev_data != 0) {
	        s_exit_flag = 1;
	      }
	      break;
	    case MG_EV_HTTP_REPLY: {

	      struct mbuf *buffer  = (struct mbuf *) nc->mgr->user_data;
	      LOG(E_DEBUG, "append %.*s to buffer\n", hm->body.len, hm->body.p);
	      mbuf_append(buffer, hm->body.p, hm->body.len);
	      s_exit_flag = 1;
	      nc->flags |= MG_F_CLOSE_IMMEDIATELY;
	      break;
	    }
	    default:
	      break;
	  }
}

struct mbuf *http_client_download(char *url) {
	struct mg_mgr mgr;
	struct mbuf *buffer;

	buffer = malloc(sizeof(struct mbuf*));
	mbuf_init(buffer, 1024);

	LOG(E_DEBUG, "*** http client: %s\n", url);

	mg_mgr_init(&mgr, buffer);
	mg_connect_http(&mgr, ev_handler, url, NULL, NULL);

	s_exit_flag = 0;
	LOG(E_DEBUG, "connection to http server established\n");
	while(s_exit_flag == 0) {
		mg_mgr_poll(&mgr, 1000);
	}
	LOG(E_DEBUG, "return received chunk\n");
	mg_mgr_free(&mgr);
	return buffer;
}
