#pragma once

extern int SERVER_PORT;

void start_server(int port);

int handle(const char *q, int connection_id = -1);

int Parse_HTTP_Header(char *buffer, struct ReqInfo *reqinfo);

int Get_Request(int connection_id, struct ReqInfo *reqinfo);

void InitReqInfo(struct ReqInfo *reqinfo);

void FreeReqInfo(struct ReqInfo *reqinfo);

int Return_Resource(int conn, int resource, struct ReqInfo *reqinfo);

int Check_Resource(struct ReqInfo *reqinfo);

int Return_Error_Msg(int conn, struct ReqInfo *reqinfo);

int Output_HTTP_Headers(int conn, struct ReqInfo *reqinfo);

int Service_Request(int connectionId);

void Serve_Resource(ReqInfo reqinfo, int connection_id);// local file

void Error_Quit(char const *msg);

int Trim(char *buffer);

int StrUpper(char *buffer);

void CleanURL(char *buffer);

int Readline(int sockd, void *vptr, int maxlen);

//ssize_t Writeline (int sockd, const void *vptr, size_t n=-1);
void Writeline(const char *s);// debug to server
//void Writeline(std::string s);
//ssize_t Writeline(int sockd, std::string s);
int Writeline(int sockd, const char *vptr, int n = -1);

