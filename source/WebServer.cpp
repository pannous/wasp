//#pragma once
// All structured data from the main and property namespace is available under the Creative Commons CC0 License;

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>    /* socket definitions    */
#include <sys/types.h>    /* socket types       */
#include <sys/wait.h>     /* for waitpid()       */
#include <arpa/inet.h>    /* inet (3) funtions     */
#include <unistd.h>      /* misc. UNIX functions   */

#include <stdio.h>
#include <stdlib.h>

#include <sys/time.h>       /* For select() */
#include <unistd.h>
#include <fcntl.h>
#include <thread>

#include "WebServer.hpp"
#include "String.h"
#include "Angle.h"
#include "Paint.h"

int SERVER_PORT; // set by start_server(port)

int Parse_HTTP_Header(char *buffer, struct ReqInfo *reqinfo);

int Get_Request(int conn, struct ReqInfo *reqinfo);

void InitReqInfo(struct ReqInfo *reqinfo);

void FreeReqInfo(struct ReqInfo *reqinfo);

int Return_Resource(int conn, int resource, struct ReqInfo *reqinfo);

int Check_Resource(struct ReqInfo *reqinfo);

int Return_Error_Msg(int conn, struct ReqInfo *reqinfo);

int Output_HTTP_Headers(int conn, struct ReqInfo *reqinfo);

int Service_Request(int conn);

void Serve_Resource(ReqInfo reqinfo, int conn);// local file

void Error_Quit(char const *msg);

int Trim(char *buffer);

int StrUpper(char *buffer);

void CleanURL(char *buffer);

ssize_t Readline(int sockd, void *vptr, size_t maxlen);

//ssize_t Writeline (int sockd, const void *vptr, size_t n=-1);
void Writeline(const char *s);// debug to server
//void Writeline(std::string s);
//ssize_t Writeline(int sockd, std::string s);
ssize_t Writeline(int sockd, const char *vptr, size_t n = -1);


/* Global macros/variables */

#define LISTENQ     (4096)
#define MAX_REQ_LINE     (4096)
enum Req_Method {
	GET, HEAD, UNSUPPORTED
};
enum Req_Type {
	SIMPLE, FULL
};

struct ReqInfo {
	enum Req_Method method;
	enum Req_Type type;
	char *referer;
	char *useragent;
	char *resource;
	int status;
};

int MAX_QUERY_LENGTH = 1000;
static char index_html[100] = "wasp.html";
static char wasp_js[100] = "wasp.js";// wax
static char wasp_css[100] = "wasp.css";
static char favicon_ico[100] = "favicon.ico";

pid_t pid;
int listener, conn, closing = 0;
struct sockaddr_in servaddr;

char const *query0;

void handle_emit() {
	emit(query0);
}

int handle(char const *query, int conn) {
//	const char *html_block = "<!DOCTYPE html><html><head><META HTTP-EQUIV='CONTENT-TYPE' CONTENT='text/html; charset=UTF-8'/></head>\n"\
                            "<link rel='stylesheet' href='wasp.css'>\n"\
                            "<body class='wasp'><div id='wasp_results'></div>\n<script>var results=";
//	char buff[10000];
//		const char *html_block = "<!DOCTYPE html><html><head><META HTTP-EQUIV='CONTENT-TYPE' CONTENT='text/html; charset=UTF-8'/></head>\n"
	print("\nGOT WEB REQUEST "s + query + '\n');
//	Writeline(conn, "OK");
	if (conn <= 0)printf("NO Connection");
	if (conn <= 0)return 0;
	char buffer[200];
	sprintf(buffer, "<HTML>\n<HEAD>\n<TITLE>REQUEST %s</TITLE>\n</HEAD><BODY>JA</BODY></HTML>\n\n", query);
	Writeline(conn, buffer, strlen(buffer));

	// when calling FROM or TO the Webview:
// The process has forked and you cannot use this CoreFoundation functionality safely. You MUST exec().
//	exec(), which replaces the program in the current process with a brand new program.

//	init_graphics();//  +[NSCell initialize] may have been in progress in another thread when fork() was called.
	emit(query);

//	query0 = query;
//	std::thread emitte(handle_emit);
//	emitte.detach();
//	const char *html_end = ";\n</script>\n<script src='wasp.js'></script></body></html>\n";
//	if (format == html)Writeline(conn, html_end);
	return 0;// 0K
}


// WORKS FINE, but not when debugging
int Service_Request(int conn) {
	int ok = 0;
	struct ReqInfo reqinfo;
	InitReqInfo(&reqinfo);
	/* Get HTTP request */
	if (Get_Request(conn, &reqinfo) < 0)
		return -1;
	else if (reqinfo.type == FULL)
		Output_HTTP_Headers(conn, &reqinfo);

	CleanURL(reqinfo.resource);
//	initSharedMemory(); // for each forked process!
	if (strlen(reqinfo.resource) > 1000)return 0;// safety
//	char *q = substr(reqinfo.resource, 1, -1);
	char *q = String(reqinfo.resource).substring(1, -1);
	// ::::::::::::::::::::::::::::::
	printf("%s\n", reqinfo.resource);
	if (not q or strlen(q) == 0 or q[0] == '?' or q[strlen(q) - 1] == '/'
	    or contains(q, wasp_js) or contains(q, wasp_css) or contains(q, favicon_ico) or contains(q, index_html) or
	    contains(q, "index.html"))
		Serve_Resource(reqinfo, conn);
	else
		handle(q, conn); // <<<<<<< CENTRAL CALL
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	FreeReqInfo(&reqinfo);
	return ok;
}


/* Prints an error message and quits */
void Error_Quit(char const *msg) {
	fprintf(stderr, "WEBSERV: %s\n", msg);
	exit(EXIT_FAILURE);
}

/* Read a line from a socket */
ssize_t Readline(int sockd, void *vptr, size_t maxlen) {
	ssize_t n, rc;
	char c, *buffer;

	buffer = (char *) vptr;

	for (n = 1; n < maxlen; n++) {

		if ((rc = read(sockd, &c, 1)) == 1) {
			*buffer++ = c;
			if (c == '\n')
				break;
		} else if (rc == 0) {
			if (n == 1)
				return 0;
			else
				break;
		} else {
			if (errno == EINTR)
				continue;
			Error_Quit("Error in Readline()");
		}
	}

	*buffer = 0;
	return n;
}

int lastSockd = -1;

void Writeline(const char *s) {
	Writeline(lastSockd, s, 0);
}

void Writeline(String s) {
	Writeline(lastSockd, s.data, 0);
}

/* Write a line to a socket */
ssize_t Writeline(int sockd, String &s) {
	return Writeline(sockd, s.data, s.length);
}

ssize_t Writeline(int sockd, const char *vptr, size_t n) {
	size_t nleft;
	ssize_t nwritten;
	const char *buffer;
	if (sockd == -1) {// debug
		print(vptr);
		return 0;
	}
	if (sockd == 0)sockd = lastSockd; //not thread safe!
	lastSockd = sockd;

	buffer = vptr;
	if (n == 0 or n == -1)
		n = strlen(buffer);
	//	printf("%d:%s\n",n,buffer);
	nleft = n;

	while (nleft > 0) {
		if ((nwritten = write(sockd, buffer, nleft)) <= 0) {
			if (errno == EINTR)
				nwritten = 0;
			else
				Error_Quit("Error in Writeline()");
		}
		nleft -= nwritten;
		buffer += nwritten;
	}

	return n;
}

int Trim(char *buffer) {
	long n = strlen(buffer) - 1;

	while (!isalnum(buffer[n]) and n >= 0)
		buffer[n--] = '\0';

	return 0;
}

int StrUpper(char *buffer) {
	while (*buffer) {
		*buffer = toupper(*buffer);
		++buffer;
	}
	return 0;
}

/* Cleans up url-encoded string */
void CleanURL(char *buffer) {
	char asciinum[3] = {0};
	int i = 0, c;

	while (buffer[i]) {
		if (buffer[i] == '+')
			buffer[i] = ' ';
		else if (buffer[i] == '%') {
			asciinum[0] = buffer[i + 1];
			asciinum[1] = buffer[i + 2];
			buffer[i] = strtol(asciinum, NULL, 16);
			c = i + 1;
			do {
				buffer[c] = buffer[c + 2];
			} while (buffer[2 + (c++)]);
		}
		++i;
	}
}

/* Parses a string and updates a request information structure if necessary.  */
int Parse_HTTP_Header(char *buffer, struct ReqInfo *reqinfo) {

	static int first_header = 1;
	char *temp;
	char *endptr;
	size_t len;
	if (first_header == 1) {
		/* If first_header is 0, this is the first line of
     the HTTP request, so this should be the request line. */
		/* Get the request method, which is case-sensitive. This
     version of the server only supports the GET and HEAD
     request methods.                    */
		if (!strncmp(buffer, "GET ", 4)) {
			reqinfo->method = GET;
			buffer += 4;
		} else if (!strncmp(buffer, "HEAD ", 5)) {
			reqinfo->method = HEAD;
			buffer += 5;
		} else {
			reqinfo->method = UNSUPPORTED;
			//			reqinfo->status = 501;
			//			return -1;
		}
		/* Skip to start of resource */
		while (*buffer and isspace(*buffer))
			buffer++;
		/* Calculate string length of resource... */
		endptr = strchr(buffer, ' ');
		if (endptr == NULL)
			len = strlen(buffer);
		else
			len = endptr - buffer;
		if (len == 0) {
			//			reqinfo->status = 400;
			//			return -1;
		}
		/* ...and store it in the request information structure. */
		reqinfo->resource = (char *) calloc(len + 1, sizeof(char));
		strncpy(reqinfo->resource, buffer, len);
		/* Test to see if we have any HTTP version information.
     If there isn't, this is a simple HTTP request, and we
     should not try to read any more headers. For simplicity,
     we don't bother checking the validity of the HTTP version
     information supplied - we just assume that if it is
     supplied, then it's a full request.            */
		if (contains(buffer, "HTTP/") or contains(buffer, "http/"))
			reqinfo->type = FULL;
		else
			reqinfo->type = SIMPLE;
		first_header = 0;
		return 0;
	}
	/* If we get here, we have further headers aside from the
   request line to parse, so this is a "full" HTTP request. */
	/* HTTP field names are case-insensitive, so make an
   upper-case copy of the field name to aid comparison.
   We need to make a copy of the header up until the colon.
   If there is no colon, we return a status code of 400
   (bad request) and terminate the connection. Note that
   HTTP/1.0 allows (but discourages) headers to span multiple
   lines if the following lines start with a space or a
   tab. For simplicity, we do not allow this here.       */
	endptr = strchr(buffer, ':');
	if (endptr == NULL) {
		//		reqinfo->status = 400;
		return -1;//ok
	}
	temp = (char *) calloc((endptr - buffer) + 1, sizeof(char));
	strncpy(temp, buffer, (endptr - buffer));
	StrUpper(temp);
	/* Increment buffer so that it now points to the value.
   If there is no value, just return.          */
	buffer = endptr + 1;
	while (*buffer and isspace(*buffer))
		++buffer;
	if (*buffer == '\0')
		return 0;
	/* Now update the request information structure with the
   appropriate field value. This version only supports the
   "Referer:" and "User-Agent:" headers, ignoring all others. */
	if (!strcmp(temp, "USER-AGENT")) {
		reqinfo->useragent = (char *) malloc((strlen(buffer) + 1) * sizeof(char));
		strcpy(reqinfo->useragent, buffer);
	} else if (!strcmp(temp, "REFERER")) {
		reqinfo->referer = (char *) malloc((strlen(buffer) + 1) * sizeof(char));
		strcpy(reqinfo->referer, buffer);
	}
	free(temp);
	return 0;
}

/* Gets request headers. A CRLF terminates a HTTP header line,
 but if one is never sent we would wait forever. Therefore,
 we use select() to set a maximum length of time we will
 wait for the next complete header. If we timeout before
 this is received, we terminate the connection.        */
int Get_Request(int conn, struct ReqInfo *reqinfo) {
	char buffer[MAX_REQ_LINE] = {0};
	int rval;
	fd_set fds;
	struct timeval tv;
	/* Set timeout to 5 seconds */
	tv.tv_sec = 5;
	tv.tv_usec = 0;
	/* Loop through request headers. If we have a simple request,
   then we will loop only once. Otherwise, we will loop until
   we receive a blank line which signifies the end of the headers,
   or until select() times out, whichever is sooner.        */
	do {
		/* Reset file descriptor set */
		FD_ZERO(&fds);
		FD_SET(conn, &fds);
		/* Wait until the timeout to see if input is ready */
		rval = select(conn + 1, &fds, NULL, NULL, &tv);
		/* Take appropriate action based on return from select() */
		if (rval < 0) {
			Error_Quit("Error calling select() in get_request()");
		} else if (rval == 0) {
//			p(" input not ready after timeout "); seems non-severe? happened after 2017 why??
			return -1;
		} else {
			/* We have an input line waiting, so retrieve it */
			Readline(conn, buffer, MAX_REQ_LINE - 1);
			//	  Trim(buffer);
			if (buffer[0] == '\0')
				break;
			if (Parse_HTTP_Header(buffer, reqinfo))
				break;
		}
	} while (reqinfo->type != SIMPLE);
	return 0;
}

void InitReqInfo(struct ReqInfo *reqinfo) {
	reqinfo->useragent = NULL;
	reqinfo->referer = NULL;
	reqinfo->resource = NULL;
	reqinfo->method = UNSUPPORTED;
	reqinfo->status = 200;
	reqinfo->type = (Req_Type) FULL;// SIMPLE, FULL
}

void FreeReqInfo(struct ReqInfo *reqinfo) {
	if (reqinfo->useragent)
		free(reqinfo->useragent);
	if (reqinfo->referer)
		free(reqinfo->referer);
	if (reqinfo->resource)
		free(reqinfo->resource);
}

int Return_Resource(int conn, int resource, struct ReqInfo *reqinfo) {
	char c;
	size_t i;
	while ((i = read(resource, &c, 1))) {
		if (i < 0)
			Error_Quit("Error reading from file.");
		if (write(conn, &c, 1) < 1)
			Error_Quit("Error sending file.");
	}
	return 0;
}

/* Tries to open a resource. The calling function can use
 the return value to check for success, and then examine
 errno to determine the cause of failure if neceesary.  */
int Check_Resource(struct ReqInfo *reqinfo) {
	/* Resource name can contain urlencoded
   data, so clean it up just in case.  */
	print("Serve_Resource "s + reqinfo->resource);
	CleanURL(reqinfo->resource);
	/* Concatenate resource name to server root, and try to open */
//	strcat(server_root, reqinfo->resource);// DONT allow arbitrary files
	if (contains(reqinfo->resource, "wasp.js"))
		return open(wasp_js, O_RDONLY);
	if (contains(reqinfo->resource, "wasp.css"))
		return open(wasp_css, O_RDONLY);
	if (contains(reqinfo->resource, "favicon.ico"))
		return open(favicon_ico, O_RDONLY);

//	return open(string("./")+reqinfo->resource, O_RDONLY);
	else
		return open(index_html, O_RDONLY);
}

int Return_Error_Msg(int conn, struct ReqInfo *reqinfo) {
	char buffer[100];
	sprintf(buffer, "<HTML>\n<HEAD>\n<TITLE>Server Error %d</TITLE>\n</HEAD>\n\n", reqinfo->status);
	Writeline(conn, buffer, strlen(buffer));
	sprintf(buffer, "<BODY>\n<H1>Server Error %d</H1>\n", reqinfo->status);
	Writeline(conn, buffer, strlen(buffer));
	sprintf(buffer, "<P>The request could not be completed.</P>\n"
	                "</BODY>\n</HTML>\n");
	Writeline(conn, buffer, strlen(buffer));
	return 0;
}

int Output_HTTP_Headers(int conn, struct ReqInfo *reqinfo) {
	char buffer[100];
	sprintf(buffer, "HTTP/1.1 %d OK\r\n", reqinfo->status);
	Writeline(conn, buffer, strlen(buffer));
	if (contains(reqinfo->resource, "text/") or contains(reqinfo->resource, "txt/") or
	    contains(reqinfo->resource, "plain/"))
		Writeline(conn, "Content-Type: text/plain; charset=utf-8\r\n");
	else if (contains(reqinfo->resource, "json/") or contains(reqinfo->resource, "learn") or
	         contains(reqinfo->resource, "delete")) {
		Writeline(conn, "Content-Type: application/json; charset=utf-8\r\n");
		Writeline(conn, "Access-Control-Allow-Origin: *\r\n");// http://quasiris.com
	} else if (contains(reqinfo->resource, "csv/"))
		Writeline(conn, "Content-Type: text/plain; charset=utf-8\r\n");
	else if (contains(reqinfo->resource, "tsv/"))
		Writeline(conn, "Content-Type: text/plain; charset=utf-8\r\n");
	else if (contains(reqinfo->resource, "xml/"))
		Writeline(conn, "Content-Type: text/plain; charset=utf-8\r\n");// till entities are fixed
	else if (contains(reqinfo->resource, ".css"))
		Writeline(conn, "Content-Type: text/css; charset=utf-8\r\n");
	else if (contains(reqinfo->resource, ".ico"))
		Writeline(conn, "Content-Type: image/x-icon\r\n");
//		Writeline(conn, "Content-Type: application/xml; charset=utf-8\r\n");
	else
		Writeline(conn, "Content-Type: text/html; charset=utf-8\r\n");
//	Writeline(conn, "Cache-Control:max-age=10000, public\r\n");
	Writeline(conn, "Connection: close\r\n");
	Writeline(conn, "Server: Wasp\r\n");
	Writeline(conn, "\r\n", 2);
//	Writeline(conn, "\r\n", 2);
	return 0;
}

void Serve_Resource(ReqInfo reqinfo, int conn) {
	int resource = 0;
	printf("Serve_Resource!!\n");
	printf("%s", reqinfo.resource);
	printf("%s", reqinfo.referer);
	printf("%s", reqinfo.useragent);
	/* Check whether resource exists, whether we have permission
   to access it, and update status code accordingly.     */
	if (reqinfo.status == 200)
		if ((resource = Check_Resource(&reqinfo)) < 0) {
			if (errno == EACCES)
				reqinfo.status = 401;
			else
				reqinfo.status = 404;
		}
	/* Output HTTP response headers if we have a full request */
//	if (reqinfo.type == FULL) done
//		Output_HTTP_Headers(conn, &reqinfo);
	/* Service the HTTP request */
	if (Return_Resource(conn, resource, &reqinfo))
		Error_Quit("Something wrong returning resource.");
//		Return_Error_Msg(conn, &reqinfo);
//	  else // all good!
//
	if (resource > 0)
		if (close(resource) < 0)
			Error_Quit("Error closing resource.");
//	FreeReqInfo(&reqinfo);
}

void start_server(int port = SERVER_PORT) {
	signal(SIGPIPE, SIG_IGN);
	//	https://stackoverflow.com/questions/108183/how-to-prevent-sigpipes-or-handle-them-properly

	printf("STARTING SERVER!\n http://localhost:%d\n", port);
	if (port < 1024)print("sudo wasp if port < 1024!");
	fflush(stdout);
	/* Create socket */
	if ((listener = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		Error_Quit("Couldn't create listening socket.");

	int flag = 1;// allow you to bind a local port that is in TIME_WAIT.
	//	This is very useful to ensure you don't have to wait 4 minutes after killing a server before restarting it.
	setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));

	/* Populate socket address structure */
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(port);

	/* Assign socket address to socket */
	//	__bind<int&,sockaddr *,unsigned long> x=
	bind(listener, (struct sockaddr *) &servaddr, sizeof(servaddr));
	if (listener < 0)
		Error_Quit("Couldn't bind listening socket.");

	/* Make socket a listening socket */
	//  	if (listen(listener, BACKLOG) < 0)
	if (listen(listener, LISTENQ) < 0)
		Error_Quit("Call to listen failed.");

	printf("listening on %d port %d\n", INADDR_ANY, port);
//	printf(" [debugging server doesn't work with xcode, use ./compile.sh ]");

	/* Loop infinitely to accept and service connections */
	while (1) {
		/* Wait for connection */
		// NOT with XCODE -> WEBSERV
		conn = accept(listener, NULL, NULL);
		if (conn < 0)
			Error_Quit("Error calling accept()! debugging not supported, are you debugging?");
		else print("connection accept OK");
		// WORKS FINE, but not when debugging
		/* Fork child process to service connection */
		pid = fork();
		if (pid == 0) {
			/* This is now the forked child process, so close listening socket and service request  */
			if (close(listener) < 0)
				Error_Quit("Error closing listening socket in child.");
			Service_Request(conn);
			/* Close connected socket and exit forked process */
			if (close(conn) < 0)
				Error_Quit("Error closing connection socket.");
			exit(EXIT_SUCCESS);
		}
		/* If we get here, we are still in the parent process,
     so close the connected socket, clean up child processes,
     and go back to accept a new connection.
     */
		waitpid(-1, NULL, WNOHANG);
		if (close(conn) < 0)
			Error_Quit("Error closing connection socket in parent.");
	}
	Error_Quit("FORK web server failed");
	return; // EXIT_FAILURE;  /* We shouldn't get here */
}