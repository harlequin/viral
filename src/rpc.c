/**
 * Copyright (c) 2015 harlequin
 * https://github.com/harlequin/viral
 *
 * This file is part of viral.
 *
 * viral is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdarg.h>
#include <stdlib.h>


#include "rpc.h"
#include "log.h"
#include "options.h"
#include "../lib/mongoose/mongoose.h"
#include "portable.h"
#include "utlist.h"
#include "networks.h"
#include "viral.h"
#include "utils.h"
#include "utlist.h"
#include "../version.h"
#include "http_client.h"


#include "../lib/frozen/frozen.h"

//typedef int (*mg_rpc_handler_t)(char *buf, int len, struct mg_rpc_request *);
typedef int (*mg_rpc_handler_t)(struct mbuf *dest, struct mg_rpc_request *);

/* JSON-RPC standard error codes */
#define JSON_RPC_PARSE_ERROR (-32700)
#define JSON_RPC_INVALID_REQUEST_ERROR (-32600)
#define JSON_RPC_METHOD_NOT_FOUND_ERROR (-32601)
#define JSON_RPC_INVALID_PARAMS_ERROR (-32602)
#define JSON_RPC_INTERNAL_ERROR (-32603)
#define JSON_RPC_SERVER_ERROR (-32000)



struct mg_rpc_request {
	struct json_token *id;
	struct json_token method;
	struct json_token *params;
	//char **params;
	int params_count;
};

///* JSON-RPC request */
//struct mg_rpc_request {
//  struct json_token *message; /* Whole RPC message */
//  struct json_token *id;      /* Message ID */
//  struct json_token *method;  /* Method name */
//  struct json_token *params;  /* Method params */
//};

/* JSON-RPC response */
struct mg_rpc_reply {
  struct json_token *message; /* Whole RPC message */
  struct json_token *id;      /* Message ID */
  struct json_token *result;  /* Remote call result */
};

/* JSON-RPC error */
struct mg_rpc_error {
  struct json_token *message;       /* Whole RPC message */
  struct json_token *id;            /* Message ID */
  struct json_token *error_code;    /* error.code */
  struct json_token *error_message; /* error.message */
  struct json_token *error_data;    /* error.data, can be NULL */
};

int json_emit_unquoted_str2(struct mbuf *dest, const char *str, int len) {
	mbuf_append(dest, str, len);
	//mbuf_append(dest, '\0', 1);
  return len;
}


int json_emit_quoted_str(char *s, int s_len, const char *str, int len) {
  const char *begin = s, *end = s + s_len, *str_end = str + len;
  char ch;

#define EMIT(x) do { if (s < end) *s = x; s++; } while (0)

  EMIT('"');
  while (str < str_end) {
    ch = *str++;
    switch (ch) {
      case '"':  EMIT('\\'); EMIT('"'); break;
      case '\\': EMIT('\\'); EMIT('\\'); break;
      case '\b': EMIT('\\'); EMIT('b'); break;
      case '\f': EMIT('\\'); EMIT('f'); break;
      case '\n': EMIT('\\'); EMIT('n'); break;
      case '\r': EMIT('\\'); EMIT('r'); break;
      case '\t': EMIT('\\'); EMIT('t'); break;
      default: EMIT(ch);
    }
  }
  EMIT('"');
  if (s < end) {
    *s = '\0';
  }

  return s - begin;
}

//struct json_token *find_json_token(struct json_token *toks, const char *path) {
////  while (path != 0 && path[0] != '\0') {
////    int i, ind2 = 0, ind = -1, skip = 2, n = path_part_len(path);
////    if (path[0] == '[') {
////      if (toks->type != JSON_TYPE_ARRAY_START || !is_digit(path[1])) return 0;
////      for (ind = 0, n = 1; path[n] != ']' && path[n] != '\0'; n++) {
////        if (!is_digit(path[n])) return 0;
////        ind *= 10;
////        ind += path[n] - '0';
////      }
////      if (path[n++] != ']') return 0;
////      skip = 1;  /* In objects, we skip 2 elems while iterating, in arrays 1. */
////    } else if (toks->type != JSON_TYPE_OBJECT_START) return 0;
////    toks++;
////    for (i = 0; i < toks[-1].num_desc; i += skip, ind2++) {
////      /* ind == -1 indicated that we're iterating an array, not object */
////      if (ind == -1 && toks[i].type != JSON_TYPE_STRING) return 0;
////      if (ind2 == ind ||
////          (ind == -1 && toks[i].len == n && compare(path, toks[i].ptr, n))) {
////        i += skip - 1;
////        break;
////      };
////      if (toks[i - 1 + skip].type == JSON_TYPE_ARRAY_START ||
////          toks[i - 1 + skip].type == JSON_TYPE_OBJECT_START) {
////        i += toks[i - 1 + skip].num_desc;
////      }
////    }
////    if (i == toks[-1].num_desc) return 0;
////    path += n;
////    if (path[0] == '.') path++;
////    if (path[0] == '\0') return &toks[i];
////    toks += i;
////  }
////  return 0;
//	return 0;
//}

int json_emit_long(char *buf, int buf_len, long int value) {
  char tmp[20];
  int n = snprintf(tmp, sizeof(tmp), "%ld", value);
  strncpy(buf, tmp, buf_len > 0 ? buf_len : 0);
  return n;
}

int json_emit_long2(struct mbuf *dest, long int value) {
  char tmp[20];
  int n = snprintf(tmp, sizeof(tmp), "%ld", value);
  mbuf_append(dest, tmp, n);
  return n;
}

int json_emit_double(char *buf, int buf_len, double value) {
  char tmp[20];
  int n = snprintf(tmp, sizeof(tmp), "%g", value);
  strncpy(buf, tmp, buf_len > 0 ? buf_len : 0);
  return n;
}

int json_emit_double2(struct mbuf *dest, double value) {
  char tmp[20];
  int n = snprintf(tmp, sizeof(tmp), "%g", value);
  mbuf_append(dest, tmp, n);
  return n;
}

int json_emit_unquoted_str(char *buf, int buf_len, const char *str, int len) {
  if (buf_len > 0 && len > 0) {
    int n = len < buf_len ? len : buf_len;
    memcpy(buf, str, n);
    if (n < buf_len) {
      buf[n] = '\0';
    }
  }
  return len;
}

int json_emit_quoted_str2(struct mbuf *dest, const char *str, int len) {
	//char *s;
	//int s_len = strlen(str) * 2; //if all chars have to be emitted
	//s = malloc(s_len);
  //const char *begin = s;
  //const char *end = s + s_len;
  const char *str_end = str + len;
  size_t size = 0;
  char ch;

//#define EMIT(x) do { if (s < end) *s = x; s++; } while (0)

#define EMIT2(x) do { mbuf_append( dest,  x, 1 ); size++; } while(0)



  EMIT2("\"");
  while (str < str_end) {
    ch = *str++;
    switch (ch) {
      case '"':  EMIT2("\\"); EMIT2("\""); break;
      case '\\': EMIT2("\\"); EMIT2("\\"); break;
      case '\b': EMIT2("\\"); EMIT2("b"); break;
      case '\f': EMIT2("\\"); EMIT2("f"); break;
      case '\n': EMIT2("\\"); EMIT2("n"); break;
      case '\r': EMIT2("\\"); EMIT2("r"); break;
      case '\t': EMIT2("\\"); EMIT2("t"); break;
      default:
    	  if ( isprint(ch)) {
    		  EMIT2(&ch);
    	  }
    	  break;
    }
  }
  EMIT2("\"");
  //if (s < end) {
  //  *s = '\0';
  //}
  //EMIT2('\0');

  return size;
}

int json_emit_va2(struct mbuf *dest, const char *fmt, va_list ap) {
  //const char *end = s + s_len, *str, *orig = s;
  size_t len;
  size_t s;
  const char *str;

  while (*fmt != '\0') {
    switch (*fmt) {
      case '[': case ']': case '{': case '}': case ',': case ':':
      case ' ': case '\r': case '\n': case '\t':
//        if (s < end) {
//          *s = *fmt;
//        }
    	mbuf_append(dest,&(*fmt), 1);
        s++;
        break;
      case 'i':
        s += json_emit_long2(dest, va_arg(ap, long));
        break;
      case 'f':
        s += json_emit_double2(dest, va_arg(ap, double));
        break;
      case 'v':
        str = va_arg(ap, char *);
        len = va_arg(ap, size_t);
        s += json_emit_quoted_str2(dest, str, len);
        break;
      case 'V':
        str = va_arg(ap, char *);
        len = va_arg(ap, size_t);
        s += json_emit_unquoted_str2(dest, str, len);
        break;
      case 's':
        str = va_arg(ap, char *);
        s += json_emit_quoted_str2(dest, str, strlen(str));
        break;
      case 'S':
        str = va_arg(ap, char *);
        s += json_emit_unquoted_str2(dest, str, strlen(str));
        break;
      case 'T':
        s += json_emit_unquoted_str2(dest, "true", 4);
        break;
      case 'F':
        s += json_emit_unquoted_str2(dest, "false", 5);
        break;
      case 'N':
        s += json_emit_unquoted_str2(dest, "null", 4);
        break;
      default:
        return 0;
    }
    fmt++;
  }

  /* Best-effort to 0-terminate generated string */
//  if (s < end) {
//    *s = '\0';
//  }
  //mbuf_append(dest, '\0', 1);
  return dest->len;
//return s - orig;
}

int json_emit2(struct mbuf *dest, const char *fmt, ...) {
  int len;
  va_list ap;

  va_start(ap, fmt);
  len = json_emit_va2(dest, fmt, ap);
  va_end(ap);

  return len;
}

int json_emit(char *buf, int buf_len, const char *fmt, ...) {
  int len;
  va_list ap;

  va_start(ap, fmt);
  len = json_emit_va(buf, buf_len, fmt, ap);
  va_end(ap);

  return len;
}

int json_emit_va(char *s, int s_len, const char *fmt, va_list ap) {
  const char *end = s + s_len, *str, *orig = s;
  size_t len;

  while (*fmt != '\0') {
    switch (*fmt) {
      case '[': case ']': case '{': case '}': case ',': case ':':
      case ' ': case '\r': case '\n': case '\t':
        if (s < end) {
          *s = *fmt;
        }
        s++;
        break;
      case 'i':
        s += json_emit_long(s, end - s, va_arg(ap, long));
        break;
      case 'f':
        s += json_emit_double(s, end - s, va_arg(ap, double));
        break;
      case 'v':
        str = va_arg(ap, char *);
        len = va_arg(ap, size_t);
        s += json_emit_quoted_str(s, end - s, str, len);
        break;
      case 'V':
        str = va_arg(ap, char *);
        len = va_arg(ap, size_t);
        s += json_emit_unquoted_str(s, end - s, str, len);
        break;
      case 's':
        str = va_arg(ap, char *);
        s += json_emit_quoted_str(s, end - s, str, strlen(str));
        break;
      case 'S':
        str = va_arg(ap, char *);
        s += json_emit_unquoted_str(s, end - s, str, strlen(str));
        break;
      case 'T':
        s += json_emit_unquoted_str(s, end - s, "true", 4);
        break;
      case 'F':
        s += json_emit_unquoted_str(s, end - s, "false", 5);
        break;
      case 'N':
        s += json_emit_unquoted_str(s, end - s, "null", 4);
        break;
      default:
        return 0;
    }
    fmt++;
  }

  /* Best-effort to 0-terminate generated string */
  if (s < end) {
    *s = '\0';
  }

  return s - orig;
}



/*
 * Create JSON-RPC reply in a given buffer.
 *
 * Return length of the reply, which
 * can be larger then `len` that indicates an overflow.
 * `result_fmt` format string should conform to `json_emit()` API,
 * see https://github.com/cesanta/frozen
 */
//int mg_rpc_create_reply(char *buf, int len, const struct mg_rpc_request *req,
//                        const char *result_fmt, ...);
int mg_rpc_create_reply2(struct mbuf *dest, const struct mg_rpc_request *req,
                        const char *result_fmt, ...);






int mg_rpc_create_reply2(struct mbuf *dest, const struct mg_rpc_request *req, const char *result_fmt, ...) {
  //static const struct json_token null_tok = {"null", 4, 0, JSON_TYPE_NULL};
  //const struct json_token *id = req->id == NULL ? &null_tok : req->id;
  va_list ap;
  int n = 0;

  n += json_emit2(dest, "{s:s,s:s", "jsonrpc", "2.0", "id","1");
//  if (id->type == JSON_TYPE_STRING) {
//    n += json_emit_quoted_str2(dest, id->ptr, id->len);
//  } else {
//    n += json_emit_unquoted_str2(dest, id->ptr, id->len);
//  }
  n += json_emit2(dest, ",s:", "result");

  va_start(ap, result_fmt);
  n += json_emit_va2(dest, result_fmt, ap);
  va_end(ap);

  n += json_emit2(dest, "}");

  return n;
}

/*
 * Create JSON-RPC error reply in a given buffer.
 *
 * Return length of the error, which
 * can be larger then `len` that indicates an overflow.
 * `fmt` format string should conform to `json_emit()` API,
 * see https://github.com/cesanta/frozen
 */
//int mg_rpc_create_error(char *buf, int len, struct mg_rpc_request *req,
//                        int code, const char *message, const char *fmt, ...);
int mg_rpc_create_std_error2(struct mbuf *dest, struct mg_rpc_request *req,
                            int code);
int mg_rpc_create_error2(struct mbuf *dest, struct mg_rpc_request *req,
                        int code, const char *message, const char *fmt, ...);


int mg_rpc_create_error2(struct mbuf *dest, struct mg_rpc_request *req,
                        int code, const char *message, const char *fmt, ...) {
  va_list ap;
  int n = 0;

  n += json_emit2(dest, "{s:s,s:V,s:{s:i,s:s,s:", "jsonrpc", "2.0",
                 "id", req->id == NULL ? "null" : req->id->ptr,
                 req->id == NULL ? 4 : req->id->len, "error", "code", code,
                 "message", message, "data");
  va_start(ap, fmt);
  n += json_emit_va2(dest, fmt, ap);
  va_end(ap);

  n += json_emit2(dest, "}}");

  return n;
}



static void scan_params(const char *str, int len, void *data) {
	struct mg_rpc_request *r = (struct rpc_request*) data;
	struct json_token t;
	int i = 0;

	r->params = malloc(sizeof(r->params) * 1);
	//r->params = malloc(sizeof(struct json_token) * 1);

	r->params_count = 0;

	for (i = 0; json_scanf_array_elem(str, len, "", i, &t) > 0; i++) {

		r->params = realloc(r->params, sizeof(r->params[0]) * (i+1));
		//r->tokens[i] =(struct json_token) malloc(sizeof(struct json_token));
		r->params[i].len = t.len;
		r->params[i].ptr = t.ptr;
		r->params_count ++;
		//r->params = (char **) realloc (r->params, sizeof(r->params) * (i + 1));
		//((char**) r->params)[i] = (char*) malloc(t.len + 1);
		//((char**) r->params)[i][t.len] = '\0';
		//memcpy(((char**) r->params)[i],t.ptr,t.len);
		//LOG(E_DEBUG, "param %.*s\n", r->params[i].len, r->params[i].ptr);
	}
	//r->params_count = i;
}

int mg_rpc_dispatch2(const char *buf, int len, struct mbuf *dest, const char **methods, mg_rpc_handler_t *handlers) {

	struct mg_rpc_request req;

	int i, n;

	//LOG(E_DEBUG, "==> %.*s\n", len, buf);
	n = json_scanf(buf, len, "{id: %T, method: %T, params: %M}", &req.id, &req.method, scan_params, &req);
	//n = json_scanf(buf, len, "{id: %T, method: %T, params: %Q}", &req.id, &req.method, &req.params);
	if (n < 0) {
		int err_code = (n == JSON_STRING_INVALID) ?	JSON_RPC_PARSE_ERROR : JSON_RPC_SERVER_ERROR;
		return mg_rpc_create_std_error2(dest, &req, err_code);
	}

	//LOG(E_DEBUG, "param %.*s\n", req.params->len, req.params->ptr);
//
//	 for( struct json_token *tok = req.params; tok; tok = ++req.params) {
//		 LOG(E_DEBUG, "xxx %.*s\n", tok->len, tok->ptr);
//	 }

	//if (req.id == 0 || req.method == NULL) {
	//  return mg_rpc_create_std_error2(dest, &req, JSON_RPC_INVALID_REQUEST_ERROR);
	//}

	for (i = 0; methods[i] != NULL; i++) {
		int mlen = strlen(methods[i]);

		if (mlen == req.method.len && memcmp(methods[i], req.method.ptr, mlen) == 0)
			break;
	}

	if (methods[i] == NULL) {
		LOG(E_DEBUG, "Seems not\n");
		return mg_rpc_create_std_error2(dest, &req,	JSON_RPC_METHOD_NOT_FOUND_ERROR);
	}

	//return handlers[i](dst, dst_len, &req);

	//LOG(E_DEBUG, "call handler %d\n", i);
	return handlers[i](dest, &req);
}







int mg_rpc_create_std_error2(struct mbuf *dest, struct mg_rpc_request *req,
                            int code) {
  const char *message = NULL;

  switch (code) {
    case JSON_RPC_PARSE_ERROR:
      message = "parse error";
      break;
    case JSON_RPC_INVALID_REQUEST_ERROR:
      message = "invalid request";
      break;
    case JSON_RPC_METHOD_NOT_FOUND_ERROR:
      message = "method not found";
      break;
    case JSON_RPC_INVALID_PARAMS_ERROR:
      message = "invalid parameters";
      break;
    case JSON_RPC_SERVER_ERROR:
      message = "server error";
      break;
    default:
      message = "unspecified error";
      break;
  }

  return mg_rpc_create_error2(dest, req, code, message, "N");
}
/*
 * Create JSON-RPC error in a given buffer.
 *
 * Return length of the error, which
 * can be larger then `len` that indicates an overflow. See
 * JSON_RPC_*_ERROR definitions for standard error values:
 *
 * - #define JSON_RPC_PARSE_ERROR (-32700)
 * - #define JSON_RPC_INVALID_REQUEST_ERROR (-32600)
 * - #define JSON_RPC_METHOD_NOT_FOUND_ERROR (-32601)
 * - #define JSON_RPC_INVALID_PARAMS_ERROR (-32602)
 * - #define JSON_RPC_INTERNAL_ERROR (-32603)
 * - #define JSON_RPC_SERVER_ERROR (-32000)
 */
int mg_rpc_create_std_error(char *, int, struct mg_rpc_request *, int code);




int mg_rpc_dispatch2(const char *buf, int len, struct mbuf *dest, const char **methods, mg_rpc_handler_t *handlers) ;





#define JSON_TYPE_ARRAY JSON_TYPE_ARRAY_START







extern log_message_t *log_messages;

static struct mg_serve_http_opts s_http_server_opts;

#define JSON_CALLBACK(funcname)					int funcname (struct mbuf *buf, struct mg_rpc_request *req)
#define JSON_CALLBACK_NOT_IMPLEMENTED(funcname)	int funcname (struct mbuf *buf, struct mg_rpc_request *req) {return mg_rpc_create_std_error2(buf, req, JSON_RPC_SERVER_ERROR );}






char *join(char *result,  char **array, uint16_t size, const char *sep){
	uint16_t i,len=0;
	LOG(E_DEBUG, "Function called with count of %d items \n", size);
    for(i=0;i<size;++i){
    	LOG(E_DEBUG, "Add %s\n", array[i]);
        len+=sprintf(result+len, "\"%s\"", array[i]);
        if(i < size - 1)
            len+=sprintf(result+len, "%s", sep);
    }
    return result;
}


char *list_join(irc_channel_t *list, const char *sep) {
	irc_channel_t *c, *tmp;
	struct mbuf buf;
	char *result;

	mbuf_init(&buf, 1024);

	LL_FOREACH_SAFE(list,c, tmp ) {
		mbuf_append(&buf, "\"", 1);
		mbuf_append(&buf, c->name, strlen(c->name));
		mbuf_append(&buf, "\"", 1);

		if (c->next)
			mbuf_append(&buf, sep, strlen(sep));
	}

	result = malloc(buf.len);
	sprintf(result, "%.*s", buf.len, buf.buf);

	mbuf_free(&buf);

	return result;
}


JSON_CALLBACK(json_rpc_servers) {
	size_t sz = 0;
	char buffer[1024] = "";
	irc_server_t *item = NULL, *tmp = NULL;

	LL_FOREACH_SAFE(viral_options.servers, item, tmp) {
		sz += json_emit( buffer+sz, sizeof(buffer) - sz, "{s:s,s:i,s:i,s:i,s:[S]}",
				"name", item->host,
				"port", item->port,
				"ssl", item->encryption,
				"state", 0/*irc_is_connected(server_list->connections[i]->irc_session)*/,
				"channels", list_join(item->channels, ",")
		);
		if ( item->next != NULL ) {
			sz += json_emit(buffer+sz, sizeof(buffer)-sz, ",");
		}
	}

	LOG(E_DEBUG, "Reply %.*s\n", sz, buffer);


//	struct json_out out;
//	json_printf(&out, "{jsonrpc:2.0, id:1, result:[%Q]}", buffer);
//
//	LOG(E_DEBUG, "Reply %.*s\n", out.);
//

	//return mg_rpc_create_reply(buf, len, req, "[S]", buffer);
	return mg_rpc_create_reply2(buf, req, "[S]", buffer);
}


JSON_CALLBACK(json_rpc_log){

	//./viral.log
//	size_t sz;
//	char buffer[4096*16] = "";
//	char *id;
//	char *num;
//	int count;


	//return mg_rpc_create_std_error2(buf, req, JSON_RPC_INVALID_PARAMS_ERROR);

	log_message_t *tmp, *item;
	struct mbuf b;
	mbuf_init(&b,128);

	DL_FOREACH_SAFE(log_messages, item, tmp) {
		json_emit2(&b, "{s:s, s:i}", "data", item->data, "level", item->level );
		if(item->next) {
			mbuf_append(&b, ",", 1);
		}
	}

	char *buffer;
	buffer = malloc(b.len + 1);
	sprintf(buffer, "%.*s", b.len, b.buf);

	return mg_rpc_create_reply2(buf, req, "[S]", buffer);

//	if ( req->params[0].type != JSON_TYPE_ARRAY) {
//		LOG(E_WARN, "Invalid type array\n");
//		return mg_rpc_create_std_error(buf, len, req, JSON_RPC_INVALID_PARAMS_ERROR);
//	}

//	id = malloc( req->params[1].len );
//	sprintf(id, "%.*s", req->params[1].len, req->params[1].ptr);
//
//	num = malloc( req->params[2].len );
//	sprintf(num, "%.*s", req->params[2].len, req->params[2].ptr);
//

//
//	sz = 0;
//	count = 0;
//	DL_FOREACH_SAFE(log_messages, item, tmp) {
//		count++;
//
//		sz += json_emit( buffer+sz, sizeof(buffer) - sz, "{s:s, s:i},",
//				"data", item->data, "level", item->level);
//
//		if ( count == 5 ) {
//			break;
//		}
//	}
//	if ( sz > 0 ) {
//		if(buffer[sz-1] == ',') {
//			buffer[sz-1] = ' ';
//		}
//	}
//
//	return mg_rpc_create_reply(buf, len, req, "[S]", buffer);
}


JSON_CALLBACK(json_rpc_download_delete) {
	//list_item *el;
	uint16_t id;

//	if ( req->params[0].type != JSON_TYPE_ARRAY) {
//		LOG(E_WARN, "Invalid type array\n");
//		return mg_rpc_create_std_error2(buf,  req, JSON_RPC_INVALID_PARAMS_ERROR);
//		//return mg_rpc_create_std_error(buf, len, req, JSON_RPC_INVALID_PARAMS_ERROR);
//	}
//
//	if ( req->params[1].type != JSON_TYPE_NUMBER)  {
//		LOG(E_WARN, "Type not a string\n");
//		return mg_rpc_create_std_error2(buf,  req, JSON_RPC_INVALID_PARAMS_ERROR);
//		//return mg_rpc_create_std_error(buf, len, req, JSON_RPC_INVALID_PARAMS_ERROR);
//	}

	id = strtol(req->params[0].ptr, NULL,10);

	LOG(E_INFO, "Delete download with id %d\n", id);

	irc_download_t *el, *tmp;
	LL_FOREACH_SAFE(download_queue, el, tmp) {
		if ( el->id == id ) {
			/*TODO: Send xdcc abort if it's ongoing ... also delete local files if there?*/
			LL_DELETE(download_queue, el);
			downloads_queue_save();
			break;
		}
	}

	//return mg_rpc_create_reply(buf, len, req, "[s]", "Download deleted");
	return mg_rpc_create_reply2(buf, req, "[s]", "Download deleted");
}

void strip(char *s) {
    char *p2 = s;
    while(*s != '\0') {
    	if(*s != '\t' && *s != '\n' && *s != '\r') {
    		*p2++ = *s++;
    	} else {
    		++s;
    	}
    }
    *p2 = '\0';
}

JSON_CALLBACK(json_rpc_downloads) {
	size_t sz;
	char buffer[4096*16] = "";
	char *t_sz;
	char *c_sz;
	irc_download_t *j, *tmp;
	double time_diff = 0.0;
	uint32_t speed;

	//if ( download_queue == NULL) {
	//	return mg_rpc_create_std_error(buf, len, req, JSON_RPC_SERVER_ERROR);
	//}

	//pthread_mutex_lock(&(download_queue->mutex));
	sz = 0;
	LL_FOREACH_SAFE(download_queue, j, tmp) {

		t_sz = malloc(32);
		c_sz = malloc(32);
		sprintf(c_sz, "%"INT64_FMT, j->current_size);
		sprintf(t_sz, "%"INT64_FMT, j->total_size);

		//strip(j->error_text);


		//bytes_downloaded / (now - start_time)
		if ( j->start_time != 0 ) {
			time_diff = difftime( time(0), j->start_time );
			speed = j->current_size / time_diff;
		} else {
			speed = 0;
		}

		sz += json_emit( buffer+sz, sizeof(buffer) - sz, "{s:i,s:s,s:s,s:s,s:s,s:S,s:S,s:i,s:i,s:i,s:i},",
			"id",(int) j->id,
			"server", j->server,
			"channel", j->channel,
			"bot", j->bot,
			"name", j->name,
			"total_size", t_sz,// j->total_size,
			"current_size", c_sz,//  j->current_size,
			"package",(int) j->packet,
			"status",(int) j->status,
			"error", j->error,
			"speed", speed
		);
	}
	//}
	//pthread_mutex_unlock(&(download_queue->mutex));
	if ( sz > 0 ) {
		if(buffer[sz-1] == ',') {
			buffer[sz-1] = ' ';
		}
	}

	//return mg_rpc_create_reply(buf, len, req, "[S]", buffer);
	return mg_rpc_create_reply2(buf, req, "[S]", buffer);
}

/**
 * Add a new download to uirc
 * Parameters:
 * - message string
 * 		can be a xdcc uri or a /msg command
 * - category
 * - server - optional only when a /msg command was set
 * - channel - optional only when a /msg command was set
 */
JSON_CALLBACK(json_rpc_download_add) {
	char *message;
	//char *category;
	//char *server;
	//char *channel;
	irc_download_t *item;

LOG(E_DEBUG, "Add download arg count %d\n", req->params_count);

//	if ( req->params_count == 0) {
//		LOG(E_WARN, "Invalid type array\n");
		//return mg_rpc_create_std_error(buf, len, req, JSON_RPC_INVALID_PARAMS_ERROR);
//		return mg_rpc_create_std_error2(buf, req, JSON_RPC_INVALID_PARAMS_ERROR);
//	}

	/* TODO: check for valid parameter count */
	/* Possible values */
	/* 1 - only xdcc */
	/* 2 - only xdcc + categorie */
	/* 4 - all for a msg format and an xdcc uri */
	/*TODO: REACTIVATE */
	//LOG(E_DEBUG, "Number of request parameters %d\n", req->params->num_desc);

	message = malloc(req->params[0].len + 1);
	sprintf(message, "%.*s", req->params[0].len, req->params[0].ptr);

	/*TODO: REACTIVATE */
	//if ( req->params->num_desc == 4) {
		item = download_create();
		sprintf(item->channel, "%.*s", req->params[2].len, req->params[2].ptr);
		sprintf(item->server, "%.*s", req->params[1].len, req->params[1].ptr);

		/* check for msg format or an xdcc uri */
		//server = malloc(req->params[3].len);
		//channel = malloc(req->params[4].len);

		//sprintf(server, "%.*s", req->params[3].len, req->params[3].ptr);
		//sprintf(channel, "%.*s", req->params[4].len, req->params[4].ptr);

		//LOG(E_DEBUG, "Message %s\tCategory %s\tServer %s\tChannel %s\n", message, "" /*category*/, server, channel);
		if (! strncmp(message, "/msg", 4 )) {
			//item = download_create();
			if (parse_irc_msg(message, item) == 0) {
				//sprintf(item->channel, "%.*s", req->params[4].len, req->params[4].ptr);
				//sprintf(item->server, "%.*s", req->params[3].len, req->params[3].ptr);
				downloads_add(item->server, item->channel, item->bot, item->packet, "");
				LOG(E_DEBUG, "Download item added\n");
				//return mg_rpc_create_reply(buf, len, req, "[s]", "Download added");
				if(message){ free(message); }
				return mg_rpc_create_reply2(buf, req, "[s]", "Download added");
			} else {
				LOG(E_WARN, "Message format is not well formated\n");
			}
		/* else if xdcc */
		} else {
			LOG(E_WARN, "Format not supported\n");
		}
	//}
	if(message){ free(message); }
	//return mg_rpc_create_reply(buf, len, req, "[s]", "Download added");
	return mg_rpc_create_reply2(buf, req, "[s]", "Download added");
}

JSON_CALLBACK(json_rpc_shutdown) {
	exit_flag = 1;
	return mg_rpc_create_reply2(buf,  req, "[s]", "Server shutdown");
	//return mg_rpc_create_reply(buf, len, req, "[s]", "Server shutdown");
}

JSON_CALLBACK(json_rpc_version) {
	size_t sz = 0;
	char buffer[4096*16] = "";
	json_emit( buffer+sz, sizeof(buffer) - sz, "{s:s, s:s}", "version", BUILD_GIT_SHA, "date", BUILD_GIT_TIME);
	return mg_rpc_create_reply2(buf,  req, "[S]", buffer);
	//return mg_rpc_create_reply(buf, len, req, "[S]", buffer);
}

const char *methods[] = {
	"servers",
	"downloads",
	"download_add",
	"download_delete",
	"shutdown",
	"log",
	"version",
	NULL
};

mg_rpc_handler_t handlers[] = {
	json_rpc_servers,
	json_rpc_downloads,
	json_rpc_download_add,
	json_rpc_download_delete,
	json_rpc_shutdown,
	json_rpc_log,
	json_rpc_version,
	NULL
};

/* Server handler */
static void ev_handler(struct mg_connection *nc, int ev, void *ev_data) {
	struct http_message *hm = (struct http_message *) ev_data;
	struct mbuf buffer;
	char l[33];

	switch (ev) {
		case MG_EV_HTTP_REQUEST:
			if( mg_vcmp(&hm->uri, "/jsonrpc") == 0) {
				mbuf_init(&buffer, 1024);
				mg_rpc_dispatch2(hm->body.p, hm->body.len, &buffer, methods, handlers);

				snprintf(l, 33,"%d",buffer.len);

				mg_send(nc, "HTTP/1.0 200 OK\r\nContent-Length: ", 33);
				mg_send(nc, l, strlen(l));
				mg_send(nc, "\r\nContent-Type: application/json\r\n\r\n", 36);
				mg_send(nc, buffer.buf, buffer.len);
				nc->flags |= MG_F_SEND_AND_CLOSE;
			} else {
				mg_serve_http(nc, ev_data, s_http_server_opts);  /* Serve static content */
			}
		break;
	default:
		break;
	}
}

void rpc_init (struct mg_mgr *mgr) {
	struct mg_connection *nc;
	const char *ssl_result;
	uint8_t use_ssl;

	use_ssl = !strcmp(viral_options._encryption,"yes") ? 1 : 0;
	nc = mg_bind(mgr, viral_options._listening_port, ev_handler);

	if(!nc) {
		LOG(E_ERROR, "Can't bind rpc interface to %s\n", viral_options._listening_port);
		return;
	}

	if (use_ssl) {

#ifdef VIRAL_ENABLE_SSL
		ssl_result = mg_set_ssl(nc, "viral.pem", NULL);
		if(ssl_result != NULL) {
			LOG(E_FATAL, "%s\n", ssl_result);
		}
#else
			LOG(E_FATAL, "SSL not support ... abort now\n");
#endif


	}

	mg_set_protocol_http_websocket(nc);
	s_http_server_opts.document_root = viral_options._webui;
	s_http_server_opts.enable_directory_listing = "no";

	LOG(E_INFO, "Starting server on port %s %s, serving %s\n", viral_options._listening_port, use_ssl ? "(ssl secured)" : "", s_http_server_opts.document_root);
}
