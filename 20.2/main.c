#include <signal.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    signal(SIGQUIT, SIG_IGN);

    pause();

    return 0;
}