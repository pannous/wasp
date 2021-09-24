extern int SERVER_PORT;
#pragma once

void start_server(int port);

int handle(const char *q, int conn = -1);
