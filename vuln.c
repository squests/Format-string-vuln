#include <stdio.h>

void win(){
	printf("Successfully won!\n");
	fflush(stdout);
}

int vuln() {
    char buffer[300];
    while(fgets(buffer, sizeof(buffer), stdin)) {

        printf(buffer);
    }
return 0;
}

int main() {
    vuln();

    return 0;
}
