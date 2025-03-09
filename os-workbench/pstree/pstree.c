#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h> 

#define NUM_1 10000
#define NUM_2 500

struct Ps
{
  int prev;
  char node[256];
  int next[NUM_2];
};

struct Ps pses[NUM_1];

void Print(int j,int depth, int type);

int main(int argc, char *argv[]) {
  //pstree -V or --version
  if (argc == 2 && (strcmp(argv[1], "-V") == 0 || strcmp(argv[1], "--version") == 0))
  {
    fprintf(stderr, "pstree (PSmisc) 23.4\nCopyright (C) 1993-2020 Werner Almesberger and Craig Small\n\nPSmisc 不提供任何保证。\n该程序为自由软件，欢迎你在 GNU 通用公共许可证 (GPL) 下重新发布。\n详情可参阅 COPYING 文件。");
    return 0;
  }
  
  for (int i = 0; i < NUM_1; i++)
  {
    pses[i].prev = -1;
    for (int j = 0; j < NUM_2; j++)
    {
      pses[i].next[j] = -1;
    }
  }
  int mark = -1;
  char rub[256];//存放文件中第三个，即没用数据
  //fire
  struct stat fileStat;
  struct dirent* stp;
  DIR* dp;
  if ((dp = opendir("/proc")) == NULL)
  {
    fprintf(stderr, "fetchdir error /proc\n");
    return 1;
  }
  while ((stp = readdir(dp)) != NULL)
  {
    char name[512];
    sprintf(name, "%s/%s", "/proc", stp->d_name);
    if (stat(name, &fileStat) < 0)
      continue;
    if (*(stp->d_name) >= 48 && *(stp->d_name) <= 57)
    {
      strcat(name, "/stat");
      FILE *fp = fopen(name, "r");
      if (fp)
      {
        fscanf(fp, "%d", &mark);
        if (mark < NUM_1)
          fscanf(fp, "%s%s%d", pses[mark].node, rub, &pses[mark].prev);
      }
        fclose(fp);
    }
  }
  closedir(dp);

  //order
  for (int j = 0; j < NUM_1; j++)
  {
    if (pses[j].prev > 0 && pses[j].prev < NUM_1)
    {
      int k = 0;
      for (; pses[pses[j].prev].next[k] >= 0 && k < NUM_2 - 1; k++);
      pses[pses[j].prev].next[k] = j;
    }
  }
  
  //pstree -p or --show-pids
  if (argc >= 2)
  {
    for (int i = 0; i < argc; i++)
    {
      if ((strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--show-pids") == 0))
      {
        for (int j = 0; j < NUM_1; j++)
        {
          if (pses[j].prev == 0)
          {
            Print(j, 0, 1);
          }
        }
        return 0;
      }
    }
  }
  
  //pstree -n or --numeric-sort
  for (int j = 0; j < NUM_1; j++)
  {
    if (pses[j].prev == 0)
    {
      Print(j, 0, 0);
    }
  }

}

void Print(int j, int depth, int type)
{
  for (int k = 0; k < depth; k++)
  {
    printf("    ");
  }
  if (type == 1)//-p
    printf("%s(%d)\n", pses[j].node, j);
  else//-n
    printf("%s\n", pses[j].node);
  for (int k = 0; pses[j].next[k] >= 0 && k < NUM_2 - 1; k++)
  {
    Print(pses[j].next[k], depth + 1, type);
  }
  return;
}