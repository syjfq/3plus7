#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>


#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

unsigned long m_pow_n(unsigned long m, unsigned long n)
{
    unsigned long i = 0, ret = 1;
    if (n < 0)
        return 0;
    for (i = 0; i < n; i++)
    {
        ret *= m;
    }
    return ret;
}
int printf(const char *fmt, ...)
{
    if (fmt == NULL)
        return -1;
    int ret_num = 0;
    char *pStr = (char *)fmt; // 指向str
    int ArgIntVal = 0;        // 接收整型
    unsigned long ArgHexVal = 0;// 接十六进制
    char *ArgStrVal = NULL; // 接收字符型
    // double ArgFloVal = 0.0; // 接受浮点型
    unsigned long val_seg = 0; // 数据切分
    // unsigned long val_temp = 0;  // 临时保存数据
    int cnt = 0;          // 数据长度计数
    va_list pArgs;        // 定义va_list类型指针，用于存储参数的地址
    va_start(pArgs, fmt); // 初始化pArgs
    while (*pStr != '\0')
    {
        switch (*pStr)
        {
        case '\t':
            putch(*pStr);
            ret_num += 4;
            break;
        case '\r':
            putch(*pStr);
            ret_num++;
            break;
        case '\n':
            putch(*pStr);
            ret_num++;
            break;
        case '%':
            pStr++;
            switch (*pStr)
            {
            case '%':
                putch(*pStr);
                ret_num++;
                pStr++;
                break;
            case 'c':
                ArgIntVal = va_arg(pArgs, int);
                putch((char)ArgIntVal);
                pStr++;
                ret_num++;
                break;
            case 'x':
                // 接收16进制
                ArgHexVal = va_arg(pArgs, unsigned long);
                val_seg = ArgHexVal;
                // 计算ArgIntVal长度
                if (ArgHexVal)
                {
                    while (val_seg)
                    {
                        cnt++;
                        val_seg /= 16;
                    }
                }
                else
                    cnt = 1; // 数字0的长度为1

                ret_num += cnt; // 字符个数加上整数的长度
                // 将整数转为单个字符打印
                while (cnt)
                {
                    val_seg = ArgHexVal / m_pow_n(16, cnt - 1);
                    ArgHexVal %= m_pow_n(16, cnt - 1);
                    if (val_seg <= 9)
                       putch((char)val_seg + '0');
                    else
                    {
                        putch((char)val_seg - 10 + 'A');
                    }
                    cnt--;
                }
                pStr++;
                break;
            case 'b':
                // 接收整型
                ArgIntVal = va_arg(pArgs, int);
                val_seg = ArgIntVal;
                // 计算ArgIntVal长度
                if (ArgIntVal)
                {
                    while (val_seg)
                    {
                        cnt++;
                        val_seg /= 2;
                    }
                }
                else
                    cnt = 1; // 数字0的长度为1

                ret_num += cnt; // 字符个数加上整数的长度
                // 将整数转为单个字符打印
                while (cnt)
                {
                    val_seg = ArgIntVal / m_pow_n(2, cnt - 1);
                    ArgIntVal %= m_pow_n(2, cnt - 1);
                    putch((char)val_seg + '0');
                    cnt--;
                }
                pStr++;
                break;
            case 'd':
                ArgIntVal = va_arg(pArgs, int);
                if (ArgIntVal < 0) // 如果为负数打印，负号
                {
                    ArgIntVal = -ArgIntVal; // 取相反数

                    putch('-');
                    ret_num++;
                }
                val_seg = ArgIntVal; // 赋值给 val_seg处理数据
                // 计算ArgIntVal长度
                if (ArgIntVal)
                {
                    while (val_seg)
                    {
                        cnt++;
                        val_seg /= 10;
                    }
                }
                else
                    cnt = 1;    // 数字0的长度为1
                ret_num += cnt; // 字符个数加上整数的长度
                // 将整数转为单个字符打印
                while (cnt)
                {
                    val_seg = ArgIntVal / m_pow_n(10, cnt - 1);
                    ArgIntVal %= m_pow_n(10, cnt - 1);
                    putch((char)val_seg + '0');
                    cnt--;
                }
                pStr++;
                break;
            case 's':
                ArgStrVal = va_arg(pArgs, char *);
                ret_num += strlen(ArgStrVal);
                for (; *ArgStrVal != '\0'; ArgStrVal++)
                {
                    putch(*ArgStrVal);
                }
                pStr++;
                break;
            default:
                break;
            }
        default:
            putch(*pStr);
            ret_num++;
            break;
        }
        pStr++;
    }
    va_end(pArgs);
    return ret_num;
}


int vsprintf(char *out, const char *fmt, va_list ap) {
  panic("Not implemented");
}

int sprintf(char *out, const char *fmt, ...) {
  panic("Not implemented");
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
