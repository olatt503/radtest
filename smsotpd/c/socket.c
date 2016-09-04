#include "common.h"

int
server_listen_unix_socket(const char *name)
{
        int fd, len, ret;
        struct sockaddr_un unix_addr;

        fd = socket(AF_UNIX, SOCK_STREAM, 0);
        if (fd < 0) {
                die("%s", "failed to create socket");
        }

        unlink(name);

        memset(&unix_addr, 0, sizeof(unix_addr));

        unix_addr.sun_family = AF_UNIX;

        strcpy(unix_addr.sun_path, name);

        len = sizeof (unix_addr.sun_family) + strlen(unix_addr.sun_path);

        ret = bind(fd, (struct sockaddr *) &unix_addr, len);
        if (ret < 0) {
                die("%s", "failed to bind socket");
        }

        ret = listen(fd, BACKLOG);
        if (ret < 0) {
                die("%s", "failed to listen");
        }

        return fd;
}
