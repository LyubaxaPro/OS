#include <stdio.h>
#include <unistd.h>

int main(int argc, char* argv[])
{

    // while(1){
    //     printf("Hello\n");
    // }
    FILE *f = fopen("test_file.txt", "r");
    int n = 0;
    char mc[10];
   
    while (n < 800)
    {
        scanf("%s", mc);
        n++;
        printf("%s\n", mc);
        //sleep(1);
    }

    fclose(f);

    return 0;
}