/* SSL support.
   Copyright (C) 2000 Christian Fraenkel.

This file is part of Wget.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

#ifdef HAVE_SSL
# include <openssl/ssl.h>
#endif

int init_ssl(SSL_CTX **ctx);
int connect_ssl (SSL **con, SSL_CTX *ctx, int fd);
void shutdown_ssl (SSL* con);
void free_ssl_ctx (SSL_CTX * ctx);
int verify_callback(int ok, X509_STORE_CTX *ctx);
int ssl_iread (SSL *con, char *buf, int len);
int ssl_iwrite (SSL *con, char *buf, int len);

