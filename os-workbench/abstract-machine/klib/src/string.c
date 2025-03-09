#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  assert(s != NULL);
  int count = 0;
  while (*s != '\0')
  {
    count++;
    s++;
  }
  return count;
}

char *strcpy(char *dst, const char *src) {
  char *ptr = dst; // 保存目标字符串的起始地址
    while (*src != '\0') { // 遍历源字符串直到遇到结束符
        *dst = *src; // 复制字符
        dst++; // 移动目标字符串指针
        src++; // 移动源字符串指针
    }
    *dst = '\0'; // 在目标字符串末尾添加结束符
    return ptr; // 返回目标字符串的起始地址
}

char *strncpy(char *dst, const char *src, size_t n) {
  panic("Not implemented");
}

char *strcat(char *dst, const char *src) {
  char *p = dst;

    // 找到dst字符串的末尾
    while (*p != '\0') {
        p++;
    }

    // 将src字符串复制到dst字符串的末尾
    while (*src != '\0') {
        *p = *src;
        p++;
        src++;
    }

    // 添加字符串结束符
    *p = '\0';

    return dst;
}

int strcmp(const char *s1, const char *s2) {
  if (strlen(s1) != strlen(s2))
    return 1;
  int l = strlen(s1);
  for (int i = 0; i < l; i++)
  {
    if (s1[i] != s2[i])
      return 1;
  }
  return 0;
}

int strncmp(const char *s1, const char *s2, size_t n) {
  panic("Not implemented");
}

void *memset(void *s, int c, size_t n) {
  unsigned char *p = s;
  unsigned char uc = c;
  while (n-- > 0)
  {
    *p++ = uc;
  }
  return s;
}

void *memmove(void *dst, const void *src, size_t n) {
  panic("Not implemented");
}

void *memcpy(void *out, const void *in, size_t n) {
  panic("Not implemented");
}

int memcmp(const void *s1, const void *s2, size_t n) {
  panic("Not implemented");
}

#endif
