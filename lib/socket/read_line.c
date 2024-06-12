#include <unistd.h>
#include <errno.h>
#include "read_line.h"                  /* Declaration of readLine() */

/* Read characters from 'fd' until a newline is encountered. If a newline
  character is not encountered in the first (n - 1) bytes, then the excess
  characters are discarded. The returned string placed in 'buf' is
  null-terminated and includes the newline character if it was read in the
  first (n - 1) bytes. The function return value is the number of bytes
  placed in buffer (which includes the newline character if encountered,
  but excludes the terminating null byte). */

ssize_t readLine(int fd, void *buffer, size_t n)
{
    ssize_t numRead;                    /* # of bytes fetched by last read() */
    size_t totRead;                     /* Total bytes read so far */
    char *buf;
    char ch;

    if (n <= 0 || buffer == NULL) {
        errno = EINVAL;
        return -1;
    }

    buf = buffer;                       /* No pointer arithmetic on "void *" */

    totRead = 0;
    for (;;) {
        numRead = read(fd, &ch, 1);

        if (numRead == -1) {
            if (errno == EINTR) {         /* Interrupted --> restart read() */
                continue;
            } else {
                return -1;              /* Some other error */
            }
        } else if (numRead == 0) {      /* EOF */
            if (totRead == 0) {          /* No bytes read; return 0 */
                return 0;
            } else {                        /* Some bytes read; add '\0' */
                break;
            }
        } else {                        /* 'numRead' must be 1 if we get here */
            if (totRead < n - 1) {      /* Discard > (n - 1) bytes */
                totRead++;
                *buf++ = ch;
            }

            if (ch == '\n') {
                break;
            }
        }
    }

    *buf = '\0';
    return totRead;
}

void readLineBufInit(int fd, struct readLineBuf *rlbuf)
{
    rlbuf->fd = fd;
    rlbuf->len = 0;
    rlbuf->next = 0;
}

ssize_t readLineBuf(struct readLineBuf *rlbuf, char *buffer, size_t n)
{
    size_t totRead;   
    char ch;                  

    if (n <= 0 || buffer == NULL) {
        errno = EINVAL;
        return -1;
    }

    totRead = 0;
    for (;;) {
        if (rlbuf->next >= rlbuf->len) {
            rlbuf->len = read(rlbuf->fd, rlbuf->buf, RL_MAX_BUF);
            if (rlbuf->len == -1) {
                if (errno == EINTR) {         
                    continue;
                } else {
                    return -1;              
                }
            }
            
            if (rlbuf->len == 0) {      
                break;
            }

            rlbuf->next = 0;
        }

        ch = rlbuf->buf[rlbuf->next];
        rlbuf->next++;

        if (totRead < n) {
            buffer[totRead++] = ch;
        }

        if (ch == '\n') {
            break;
        }
    }

    buffer[totRead] = '\0';

    return totRead;
}