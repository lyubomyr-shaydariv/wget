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

#include <config.h>
#ifdef HAVE_SSL
#include <openssl/bio.h>
#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <assert.h>
#define SSL_ERR_CTX_CREATION -2
#include <sys/time.h>
#include "wget.h"
#include "connect.h"
int verify_callback(int ok, X509_STORE_CTX *ctx);

/* Creates a SSL Context and sets some defaults for it */
int init_ssl(SSL_CTX **ctx)
{
SSL_METHOD *meth=NULL;
int verify=SSL_VERIFY_NONE;
  SSL_library_init();
  SSL_load_error_strings();
  SSLeay_add_all_algorithms();
  SSLeay_add_ssl_algorithms();
  meth = SSLv23_client_method();
  *ctx = SSL_CTX_new(meth);
  SSL_CTX_set_verify(*ctx,verify,verify_callback);
  if (*ctx==NULL) return SSL_ERR_CTX_CREATION;
return 0; /* Succeded */
}

/* Sets up a SSL structure and performs the handshake on fd 
   Returns 0 if everything went right
	Returns 1 if something went wrong ----- TODO: More exit codes
*/
int connect_ssl (SSL **con, SSL_CTX *ctx, int fd) 
{
  *con=(SSL *)SSL_new(ctx);
  SSL_set_fd(*con,fd);
  SSL_set_connect_state(*con); 
  SSL_connect(*con);  
  if ((*con)->state !=SSL_ST_OK)
    return 1;
return 0;
}

void shutdown_ssl (SSL* con)
{
  SSL_shutdown(con);
  if (con != NULL) SSL_free(con);
}

void free_ssl_ctx (SSL_CTX * ctx) {
  SSL_CTX_free(ctx);
}

int verify_callback(int ok, X509_STORE_CTX *ctx){
char *s,buf[256];
  s=X509_NAME_oneline(X509_get_subject_name(ctx->current_cert),buf,256);
  if (ok == 0) {
    switch (ctx->error) {
   	case X509_V_ERR_CERT_NOT_YET_VALID:
   	case X509_V_ERR_CERT_HAS_EXPIRED:
   	case X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT:
         	ok=1;
    }
  }
  return(ok);
}

/* SSL version of iread. Only exchanged read for SSL_read 
   Read at most LEN bytes from FD, storing them to BUF.  This is
   virtually the same as read(), but takes care of EINTR braindamage
   and uses select() to timeout the stale connections (a connection is
   stale if more than OPT.TIMEOUT time is spent in select() or
   read()).  */
int
ssl_iread (SSL *con, char *buf, int len)
{
  int res;
  int fd;
  BIO_get_fd(con->rbio,&fd);
  do
    {
#ifdef HAVE_SELECT
      if (opt.timeout)
	{
	  
	  do
	    {
	      res = select_fd (fd, opt.timeout, 0);
	    }
	  while (res == -1 && errno == EINTR);
	  if (res <= 0)
	    {
	      /* Set errno to ETIMEDOUT on timeout.  */
	      if (res == 0)
		/* #### Potentially evil!  */
		errno = ETIMEDOUT;
	      return -1;
	    }
	}
#endif
      res = SSL_read (con, buf, len);
    }
  while (res == -1 && errno == EINTR);

  return res;
}

/* SSL version of iwrite. Only exchanged write for SSL_write 
   Write LEN bytes from BUF to FD.  This is similar to iread(), but
   doesn't bother with select().  Unlike iread(), it makes sure that
   all of BUF is actually written to FD, so callers needn't bother
   with checking that the return value equals to LEN.  Instead, you
   should simply check for -1.  */
int
ssl_iwrite (SSL *con, char *buf, int len)
{
  int res = 0;
  int fd;
  BIO_get_fd(con->rbio,&fd);
  /* `write' may write less than LEN bytes, thus the outward loop
     keeps trying it until all was written, or an error occurred.  The
     inner loop is reserved for the usual EINTR f*kage, and the
     innermost loop deals with the same during select().  */
  while (len > 0)
    {
      do
	{
#ifdef HAVE_SELECT
	  if (opt.timeout)
	    {
	      do
		{
		  res = select_fd (fd, opt.timeout, 1);
		}
	      while (res == -1 && errno == EINTR);
	      if (res <= 0)
		{
		  /* Set errno to ETIMEDOUT on timeout.  */
		  if (res == 0)
		    /* #### Potentially evil!  */
		    errno = ETIMEDOUT;
		  return -1;
		}
	    }
#endif
	  res = SSL_write (con, buf, len);
	}
      while (res == -1 && errno == EINTR);
      if (res <= 0)
	break;
      buf += res;
      len -= res;
    }
  return res;
}
#endif /* HAVE_SSL */
