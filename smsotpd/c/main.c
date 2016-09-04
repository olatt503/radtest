#include "common.h"

int debug = 0;

int
main(int argc, char **argv)
{
        int listen_socket, client_socket, client_len;
        struct sockaddr_un client_addr;

        char *input_file = NULL;
        int c;

        opterr = 0;

        while ((c = getopt(argc, argv, "df:")) != -1) {
                switch (c) {
                case 'd':
                        debug = 1;
                        break;
                case 'f':
                        input_file = optarg;
                        break;
                case '?':
                        if (optopt == 'f') {
                                fprintf (stderr, "Option -%c requires an argument. Aborting\n", optopt);
                                exit(-1);
                        }
                        fprintf(stderr, "Syntax: %s [-d] [-f <file>]\n", argv[0]);
                        fprintf(stderr, "\t-d        - Debug\n");
                        fprintf(stderr, "\t-f <file> - Generate Berkeley database (username<tab>mobile)\n");
                        return 1;
                default:
                        abort();
                }
        }

        if (input_file) {
                create_berkley_db(input_file);
                return 0;
        }

        db_open(DATABASE, DB_RDONLY);

        signal(SIGPIPE, SIG_IGN);
        signal(SIGCHLD, SIG_IGN);

        listen_socket = server_listen_unix_socket(SOCKET);

        srand(time(NULL));

        client_len = sizeof(client_addr);

        while (1) {
                memset(&client_addr, 0, sizeof(struct sockaddr_un));
                client_socket = accept(listen_socket,
                                (struct sockaddr *) &client_addr, (socklen_t *) &client_len);
                if (client_socket < 0) {
                        die("%s", "accept failed");
                }

                dprintf("Client connected\n");

                handle_client(client_socket);

                dprintf("Client disconnected\n");
        }

        close(listen_socket);
        db_close();

        return 0;
}
