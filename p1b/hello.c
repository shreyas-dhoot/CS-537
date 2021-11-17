#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"

int
main(int argc, char *argv[])
{
	//printf(1, "hello my name is %d\n", getiocount());
	//exit();
    int x1 = getiocount();

    int rc = fork();

    int total = 0;
    int i;
    for (i = 0; i < 100000; i++) {
	char buf[100];
	(void) read(4, buf, 1);
    }
    // https://wiki.osdev.org/Shutdown
    // (void) shutdown();
    printf(1, "XV6_TEST_OUTPUT %d\n", total);

    if (rc > 0) {
	(void) wait();
	int x2 = getiocount();
	total += (x2 - x1);
	printf(1, "XV6_TEST_OUTPUT %d\n", total);
    }
    exit();

}
