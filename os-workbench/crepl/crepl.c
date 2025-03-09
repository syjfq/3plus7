#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dlfcn.h>

char *function_definitions[105];
int function_count = 0;

int func(const char *line, int is_function)
{
    char ctemplate[] = "/tmp/mytempfileXXXXXX.c";
    int fd = mkstemps(ctemplate, 2);
    if (fd == -1)
    {
        perror("Failed to create temporary file");
        return -1;
    }
    close(fd);
        
    char sotemplate[] = "/tmp/mytempfileXXXXXX.so";

    FILE *cfile = fopen(ctemplate, "w");
    if (!cfile)
    {
        perror("Failed to open file");
        remove(ctemplate);
        return -1;
    }
    for (int i = 0; i < function_count; i++)
    {
        fprintf(cfile, "%s\n", function_definitions[i]);
    }
    fprintf(cfile, "%s\n", line);
    fclose(cfile);

    pid_t pid = fork();
    if (pid == 0)
    {
        if (sizeof(void *) == 8)
        {
            execlp("gcc", "gcc", "-shared", "-fPIC", "-m64", ctemplate, "-o", sotemplate, (char *)NULL);
        }
        else
        {
            execlp("gcc", "gcc", "-shared", "-fPIC", "-m32", ctemplate, "-o", sotemplate, (char *)NULL);
        }
        perror("Compile Failure");
        return -1;
    }
    else if (pid > 0)
    {
        int status;
        waitpid(pid, &status, 0);
        if (status != 0)
        {
            printf("Compile Failure\n");
            remove(ctemplate);
            remove(sotemplate);
            return -1;
        }
    }
    else
    {
        perror("Fork Failure\n");
        remove(ctemplate);
        remove(sotemplate);
        return -1;
    }
    void *handle = dlopen(sotemplate, RTLD_LAZY);
    if (!handle)
    {
        fprintf(stderr, "%s\n", dlerror());
        remove(ctemplate);
        remove(sotemplate);
        return -1;
    }
    if (is_function == 1)
    {
        function_definitions[function_count++] = strdup(line);
    }
    else
    {
        int (*expr_func)(void);
        expr_func = dlsym(handle, "__expr_wrapper");
        if (!expr_func)
        {
            fprintf(stderr, "%s\n", dlerror());
            dlclose(handle);
            remove(ctemplate);
            remove(sotemplate);
            return -1;
        }
        printf("= %d\n", expr_func());
    }
    dlclose(handle);
    remove(ctemplate);
    remove(sotemplate);
    return 0;
}

int main(int argc, char *argv[]) {
    static char line[4096];

    while (1) {
        printf("crepl> ");
        fflush(stdout);

        if (!fgets(line, sizeof(line), stdin)) {
            break;
        }

        // To be implemented.
        if (line[0] == 'i' && line[1] == 'n' && line[2] == 't' && line[3] == ' ')
        {
            func(line, 1);
        }
        else
        {
            char wrapped_expr[4150];
            snprintf(wrapped_expr, sizeof(wrapped_expr), "int __expr_wrapper() { return %s; }", line);
            func(wrapped_expr, 0);
        }
    }
    return 0;
}
