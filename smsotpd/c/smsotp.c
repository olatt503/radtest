#include "common.h"

#define GEN_OTP_FOR "generate otp for "
#define CHECK_OTP_FOR "check otp for "
#define USER_OTP_IS "user otp is "
#define OTP_ID_IS "otp id is "
#define QUIT "quit"
#define GET_CHECK_RESULT "get check result"

#define OKAY "OK"
#define FAIL "FAILED"

struct smsotp
{
        struct smsotp *next;
        char *username;
        int id;
        int otp;
        time_t time;
};

struct smsotp *first = NULL;

struct smsotp *
otp_alocate(char *username)
{
        time_t now = time(NULL);

        struct smsotp *p = first;
        while (p) {
                if (! strcmp(p->username, username)) {
                        dprintf("recycling same username\n");
                        return p;
                }

                if (OTP_TTL < (now - p->time)) {
                        dprintf("recycling expired token\n");
                        if (p->username) {
                                free(p->username);
                        }
                        p->username = strdup(username);
                        return p;
                }

                p = p->next;
        }

        dprintf("Allocating new slot\n");

        p = malloc(sizeof(struct smsotp));
        p->username = strdup(username);
        p->next = NULL;

        if (! first) {
                first = p;
        }

        return p;
}

void
send_otp(char *mobile, int otp)
{
        int pid;
        char otp_char[BUFSIZ];

        dprintf("%05d => %s\n", otp, mobile);

        snprintf(otp_char, BUFSIZ, "%05d", otp);

        pid = fork();
        if (pid == 0) {
                dprintf("Before exec\n");
                int ret = execl(SEND_SMS_BINARY, SEND_SMS_BINARY, mobile, otp_char, NULL);
                exit(ret);
                dprintf("I am here exec failed\n");

        } else if (pid < 0) {
                die("%s", "fork failed");
        }
}

int
generate_otp(char *username)
{
        int i;
        char *mobile;

        for (i = 0; username[i]; i++) {
                username[i] = tolower(username[i]);
        }

        dprintf("username = '%s'\n", username);

        mobile = db_fetch(username);
        if (mobile) {
                struct smsotp *smsotp_p = otp_alocate(username);
                smsotp_p->id = rand();
                smsotp_p->otp = rand() % OTP_MODULO;
                smsotp_p->time = time(NULL);

                dprintf("id = %d; otp = %d\n", smsotp_p->id, smsotp_p->otp);

                send_otp(mobile, smsotp_p->otp);

                return smsotp_p->id;
        }

        return -1;
}

int
check_otp(char *username, int otp, int id)
{
        time_t now = time(NULL);
        struct smsotp *p = first;
        int ret = -1;

        if (! username) {
                return ret;
        }

        while (p) {
                if (! strcmp(p->username, username)) {
                        if ( otp == p->otp
                           && id == p->id
                           && (now - p->time) <= OTP_TTL) {
                                ret = 0;
                        }
                        p->time = 0;
                }

                p = p->next;
        }

        return ret;
}

char *
find_char_in_buf(char *buf, int size, char c)
{
        int i ;
        for (i = 0; i < size; i++) {
                if (buf[i] == c) {
                        return buf + i;
                }
        }

        return NULL;
}

void
handle_client(int fd)
{
        char buf[BUFSIZ];
        int len, ret;

        char *pos = NULL;

        char *username = NULL;
        int otp = 0;
        int id = 0;

        ret = write(fd, OKAY, strlen(OKAY) + 1);
        if (ret < 0) {
                goto out;
        }


        while (1) {

                int offset = 0;
                bzero(buf, BUFSIZ);
again:
                len = read(fd, buf + offset, BUFSIZ - offset - 1);
                if (! len) {
                        break;
                }

                /* XXX rlm_smsotp sends username followed by zeroes and than a
                 * newline character */

                /*
                buf 0x0000: 67 65 6e 65 72 61 74 65 20 6f 74 70 20 66 6f 72   generate otp for
                buf 0x0010: 20 41 64 6d 69 6e 69 73 74 72 61 74 6f 72 40 44    Administrator@D
                buf 0x0020: 49 52 45 43 54 4f 52 59 2e 47 4d 56 4c 2e 44 45   IRECTORY.GMVL.DE
                buf 0x0030: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00   ................
                ...
                buf 0x0100: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 0a   ................
                */

                pos = find_char_in_buf(buf, BUFSIZ, '\n');
                if (pos) {
                        *pos = '\0';
                } else {
                        /* wait until newline */
                        offset += len;
                        goto again;
                }

                dprintf("Line complete <%s>\n", buf);

                pos = strstr((const char *) buf, GEN_OTP_FOR);
                if (pos) {
                        pos += strlen(GEN_OTP_FOR);
                        ret = generate_otp(pos);
                        if (ret < 0) {
                                ret = write(fd, FAIL, strlen(FAIL) + 1);
                                if (ret < 0) {
                                        goto out;
                                }
                                continue;
                        }
                        len = snprintf(buf, BUFSIZ, "%d", ret);
                        dprintf("id = '%s' len = %d\n", buf, len);
                        ret = write(fd, buf, len + 1);
                        if (ret < 0) {
                                goto out;
                        }
                        continue;
                }

                pos = strstr((const char *) buf, QUIT);
                if (pos) {
                        ret = write(fd, OKAY, strlen(OKAY) + 1);
                        if (ret < 0) {
                                goto out;
                        }
                        break;
                }

                pos = strstr((const char *) buf, CHECK_OTP_FOR);
                if (pos) {
                        pos += strlen(CHECK_OTP_FOR);
                        username = strdup(pos);
                        int i;
                        for (i = 0; username[i]; i++) {
                                username[i] = tolower(username[i]);
                        }
                        ret = write(fd, OKAY, strlen(OKAY) + 1);
                        if (ret < 0) {
                                goto out;
                        }
                        continue;
                }

                pos = strstr((const char *) buf, USER_OTP_IS);
                if (pos) {
                        pos += strlen(USER_OTP_IS);
                        otp = atoi(pos);
                        ret = write(fd, OKAY, strlen(OKAY) + 1);
                        if (ret < 0) {
                                goto out;
                        }
                        continue;
                }

                pos = strstr((const char *) buf, OTP_ID_IS);
                if (pos) {
                        pos += strlen(OTP_ID_IS);
                        id = atoi(pos);
                        ret = write(fd, OKAY, strlen(OKAY) + 1);
                        if (ret < 0) {
                                goto out;
                        }
                        continue;
                }

                pos = strstr((const char *) buf, GET_CHECK_RESULT);
                if (pos) {
                        ret = check_otp(username, otp, id);
                        if (! ret) {
                                ret = write(fd, OKAY, strlen(OKAY) + 1);
                                if (ret < 0) {
                                        goto out;
                                }
                                continue;
                        }
                }

                ret = write(fd, FAIL, strlen(FAIL) + 1);
                if (ret < 0) {
                        goto out;
                }
        }

out:

        close(fd);
}
