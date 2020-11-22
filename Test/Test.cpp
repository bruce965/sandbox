#include <cstdlib>
#include <cstdio>
#include <cstdint>
#include <cerrno>

int main()
{
    printf("Starting...\n");

    FILE* file;
    errno_t openStatus = fopen_s(&file, "test.txt", "w");
    if (openStatus != 0)
    {
        printf("Failed to open file: %d.\n", openStatus);
        return -1;
    }

    int status = fprintf(file, "Hello World!\n");
    if (status < 0 || status == EILSEQ)
    {
        printf("Failed to write to file.\n");
    }
    else
    {
        if (fflush(file) != 0)
        {
            printf("Failed to flush to file.\n");
        }
    }

    if (fclose(file) != 0)
    {
        printf("Failed to close file.\n");
        return -1;
    }

    printf("File written successfully.\n");

    return 0;
}
