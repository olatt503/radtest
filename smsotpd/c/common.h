#include <arpa/inet.h>
#include <assert.h>
#include <ctype.h>
#include <db.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <syslog.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

/* Tunables */

#define BACKLOG 10
#define SOCKET "/var/run/smsotp_socket"
#define DATABASE "/home/sithglan/work/smsotpd/users"
#define SEND_SMS_BINARY "/home/sithglan/work/smsotpd/c/send_email.sh"
#define OTP_TTL (60 * 10)
#define OTP_MODULO 99999

#if 1
#define dprintf(...) printf(__VA_ARGS__)
#else
#define dprintf(...)
#endif

/* Macros */
#define die(format, ...) \
{ \
        char buf[BUFSIZ]; \
        snprintf(buf, sizeof(buf), \
                        "%s(%d): %d: %s: " format, \
                        __FILE__, getpid(), (int) __LINE__, __FUNCTION__,  __VA_ARGS__); \
        perror(buf); \
        exit(EXIT_FAILURE); \
}

/* Function prototypes */

int
server_listen_unix_socket(const char *name);

void
handle_client(int fd);

void
db_open(char *filename, int flags);

void
db_close(void);

char *
db_fetch (char *key);

void
db_put(char *key, char *value);

void
create_berkley_db(char *file);

/* Global Datastructure Prototypes */

extern int debug;
