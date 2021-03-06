// OtoPerl - live sound programming environment with Perl.
// OtoPerl::otoperld - sound processing server for OtoPerl.
// Otoperl::otoperld::codeserver - mini http server for otoperld.
/*
    OtoPerl - live sound programming environment with Perl.
    Copyright (C) 2011- Haruka Kataoka

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "otoperld.h"
#include "codeserver.h"

#define BUFFERSIZE 8192

// -------------------------------------------------------- private function

void *codeserver__error(codeserver *self, char *err);
bool codeserver__decode_ipaddr(char *str, unsigned char *addr, unsigned char *mask);
bool codeserver__check_client_ip( codeserver *self, struct sockaddr_in *client);
void codeserver__write_port_file( codeserver *self, char *path, int port );

// ------------------------------------------------- codeserver_text decraration
struct codeserver_textnode_s {
	char *text;
	int size;
	struct codeserver_textnode_s *next;
};
typedef struct codeserver_textnode_s codeserver_textnode;

typedef struct {
	codeserver_textnode *root;
	codeserver_textnode *cur;
	int size;
} codeserver_text;

codeserver_text *codeserver_text_new();
void  codeserver_text_push(codeserver_text *self, char *buf, int size);
char *codeserver_text_join(codeserver_text *self);
void  codeserver_text_destroy(codeserver_text *self);

// --------------------------------------------------- codeserver implimentation

codeserver *codeserver_init(int port, bool findfreeport, const char *c_allow, bool verbose, char *(*callback)(char *code)) {
	codeserver *self = (codeserver *)malloc(sizeof(codeserver));
	self->callback = callback;
	self->port = port;
	self->findfreeport = findfreeport;
	self->verbose = verbose;
	
	char *allow = (char *)malloc(strlen(c_allow));
	if (!allow) return codeserver__error(self, "malloc failed(codeserver_init).");
	strcpy(allow, c_allow);
	unsigned char addr_c4[4]={0,0,0,0}, mask_c4[4]={0,0,0,0};
	char *mask = strchr(allow, '/');
	if (mask) {
		*mask = '\0';
		mask++;
		if (! codeserver__decode_ipaddr(allow, addr_c4, NULL) )
			return codeserver__error(self, "invalid format: allowed address.");
		if (! codeserver__decode_ipaddr(mask , mask_c4, NULL) )
			return codeserver__error(self, "invalid format: allowed mask.");
	}else{
		if (! codeserver__decode_ipaddr(allow, addr_c4, mask_c4) )
			return codeserver__error(self, "invalid format: allowed address.");
	}
	self->allow_mask.s_addr = (*(uint32_t *)mask_c4);
	self->allow_addr.s_addr = (*(uint32_t *)addr_c4) & (self->allow_mask.s_addr);
	free(allow);
	
	return self;
}

void codeserver_start(codeserver *self) {
	struct sockaddr_in saddr;
	unsigned int sockaddr_in_size = sizeof(struct sockaddr_in);
	
	if ((self->listen_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		codeserver__error(self, "socket failed."); return;
	}
	
	bzero((char *)&saddr, sizeof(saddr));
	saddr.sin_family        = PF_INET;
	saddr.sin_addr.s_addr   = INADDR_ANY;
	//if ( ! inet_aton("192.168.0.1", &(saddr.sin_addr) ) ) exit(1);
	saddr.sin_port          = htons(self->port);
	if (self->findfreeport) {
		while ( bind(self->listen_fd, (struct sockaddr *)&saddr, sockaddr_in_size) < 0 ) {
			self->port++;
			saddr.sin_port = htons(self->port);
		}
		codeserver__write_port_file(self, PORTFILENAME, self->port);
	} else {
		if (bind(self->listen_fd, (struct sockaddr *)&saddr, sockaddr_in_size) < 0) {
			codeserver__error(self, "bind failed."); return;
		}
	}

	if (listen(self->listen_fd, SOMAXCONN) < 0) {
		codeserver__error(self, "listen failed."); return;
	}
	printf("codeserver start listening port %d.\n" , self->port);
}

void codeserver_stop(codeserver *self) {
	close(self->listen_fd);
	printf("codeserver stop.\n");
}

bool codeserver_run(codeserver *self) {
	struct sockaddr_in caddr;
	int conn_fd;
	unsigned int sockaddr_in_size = sizeof(struct sockaddr_in);
	char buf[BUFFERSIZE];
	
	struct timeval accept_timeout = {0, 0};
	fd_set accept_fds;
	FD_ZERO(&accept_fds);
	FD_SET(self->listen_fd, &accept_fds);
	int selected = select(self->listen_fd+1, &accept_fds, (fd_set *)NULL, (fd_set *)NULL, &accept_timeout);
	if (selected < 0) {
		codeserver__error(self, "select accept failed."); return false;
	} else if (selected == 0) {
		return true;
	}

	if ((conn_fd = accept(self->listen_fd, (struct sockaddr *)&caddr, &sockaddr_in_size)) < 0) {
		codeserver__error(self, "accept failed."); return false;
	}

	printf("client %s: ", inet_ntoa(caddr.sin_addr) );
	if ( ! codeserver__check_client_ip( self, &caddr ) ) {
		printf("accessed denied.\n");
		write(conn_fd, "HTTP/1.1 403 Forbidden\r\n", 24);
		write(conn_fd, "Content-Type: text/plain;\r\n\r\n", 29);
		write(conn_fd, "access from forbidden address.", 30);
		if ( close(conn_fd) < 0) {
			codeserver__error(self, "close failed."); return false;
		}
		return true;
	}

	codeserver_text *cstext = codeserver_text_new();
	struct timeval recv_timeout = {0, 100000};
	fd_set recv_fds;
	FD_ZERO(&recv_fds);
	FD_SET(conn_fd, &recv_fds);
	while (1) {
		int selected = select(conn_fd+1, &recv_fds, (fd_set *)NULL, (fd_set *)NULL, &recv_timeout);
		if (selected < 0) {
			codeserver__error(self, "select recv failed."); return false;
		} else if (selected == 0) {
			break;
		}

		int rsize = recv(conn_fd, buf, BUFFERSIZE, 0);
		if (rsize == 0) {
			break;
		} else if (rsize == -1) {
			codeserver__error(self, "recv failed."); return false;
		} else {
			codeserver_text_push(cstext, buf, rsize);
			//if (rsize < BUFFERSIZE) break;
		}
	};

	if (cstext->size == 0) {
		printf("no code received ?.\n");
		write(conn_fd, "HTTP/1.1 400 no code received\r\n", 31);
		write(conn_fd, "Content-Type: text/plain;\r\n\r\n", 29);
		write(conn_fd, "no code received.", 17);
		codeserver_text_destroy(cstext);
	}else{
		char *code = codeserver_text_join(cstext);
		printf("%d bytes code to eval.\n", cstext->size);
		codeserver_text_destroy(cstext);

		char *ret;
		bool ret_allocated;
		char *codestart = strstr(code, "\r\n\r\n");
		if (codestart != NULL) {
			codestart += 4;
			if ( self->verbose )
				printf("[CODE START]\n%s\n[CODE END]\n", codestart);
			ret = self->callback(codestart);
			ret_allocated = true;
		}else{
			ret = "failed to find code.";
			ret_allocated = false;
		}

		if (ret == NULL) {
			write(conn_fd, "HTTP/1.1 200 OK\r\n", 17);
			write(conn_fd, "Content-Type: text/plain;\r\n\r\n", 29);
		}else{
			write(conn_fd, "HTTP/1.1 400 eval failed\r\n", 26);
			write(conn_fd, "Content-Type: text/plain;\r\n\r\n", 29);
			write(conn_fd, ret, strlen(ret));
		}

		if (ret_allocated) free(ret);
		free(code);
	}

	if ( close(conn_fd) < 0) {
		codeserver__error(self, "close failed.");
		return false;
	}
	return true;
}

void *codeserver__error(codeserver *self, char *err) {
	(void)self;
	perror(err);
	return NULL;
}

// decode string address *str to binary address *addr.
// if *mask is not null, auto generated netmask will be set.
// example: '192.168' is decoded as '192.168.0.0/255.255.0.0'
bool codeserver__decode_ipaddr(char *str, unsigned char *addr, unsigned char *mask){
	char *err, *tp;
	for ( tp = strtok(str,"."); tp != NULL; tp = strtok(NULL,".") ){
		long number = strtol(tp, &err, 0);
		if (*err != '\0') return false;
		if (number < 0 || number > 255) return false;
		*addr++ = number;
		if (mask) {
			*mask++ = 255;
		}
	}
	return true;
}

bool codeserver__check_client_ip( codeserver *self, struct sockaddr_in *client) {
	return (self->allow_addr.s_addr) == 
	       (self->allow_mask.s_addr & client->sin_addr.s_addr);
}

void codeserver__write_port_file( codeserver *self, char *path, int port ) {
	FILE *portfile = fopen(path, "w");
	if (portfile == NULL) {
		codeserver__error(self, "opening portfile failed."); return;
	}
	if (fprintf(portfile, "%d", port) < 0) {
		codeserver__error(self, "writing portfile failed."); return;
	}
	fclose(portfile);
}

// -------------------------------------------- codeserver_text implimentation
codeserver_text *codeserver_text_new() {
	codeserver_text *self = (codeserver_text *)malloc(sizeof(codeserver_text));
	self->root = NULL;
	self->cur = NULL;
	self->size = 0;
	return self;
}
void codeserver_text_push(codeserver_text *self, char *buf, int size) {
	codeserver_textnode *newnode = (codeserver_textnode *)malloc(sizeof(codeserver_textnode));
	newnode->text = (char *)malloc(size);
	memcpy(newnode->text, buf, size);
	newnode->size = size;
	newnode->next = NULL;

	if ( self->root == NULL ) {
		self->root = newnode;
		self->cur  = newnode;
	}else{
		self->cur->next = newnode;
		self->cur       = newnode;
	}
	self->size += size;
}
char *codeserver_text_join(codeserver_text *self) {
	codeserver_textnode *cur = self->root;
	char *text = (char *)malloc(self->size + 1);
	char *textcur = text;
	while( cur != NULL ) {
		memcpy(textcur, cur->text, cur->size);
		textcur += cur->size;
		cur = cur->next;
	}
	*textcur = 0;
	return text;
}
void codeserver_text_destroy(codeserver_text *self) {
	codeserver_textnode *cur = self->root;
	while( cur != NULL ) {
		codeserver_textnode *next = cur->next;
		free(cur->text);
		free(cur);
		cur = next;
	}
	free(self);
}

