#include<stdio.h>
#include<fcntl.h>
#include<errno.h>
#include<unistd.h>

#include<sys/time.h>

#include<string.h>

int main()
{

        struct timeval tv_start, tv_end;
        char bufAppend[100] = "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb";

        gettimeofday(&tv_start, NULL);
        char bufFileName[30];

        for (int i = 0; i < 20000; i++)
        {
                sprintf(bufFileName, "/root/append/%d%s", i, ".txt");
                FILE* fd = fopen(bufFileName, "rb+");
                fseek(fd, 0, SEEK_END);
                fwrite(bufAppend, 100, 1, fd);

                fclose(fd);
        }

        gettimeofday(&tv_end, NULL); //s us

        printf("It used %lu us to write test \n", (tv_end.tv_sec - tv_start.tv_sec) * 1000000 + tv_end.tv_usec - tv_start.tv_usec);
        printf("%lu \n", (tv_end.tv_sec - tv_start.tv_sec) * 1000000 + tv_end.tv_usec - tv_start.tv_usec);

        return 0;
}

