#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "copt.h"

static int errval = 0;
static int c_chr = 0;
static int c_arg = 1;
static int p_help = 0;

static char copt_buf[COPT_BUFFER_M_L];

static const char *copt_type_str[] = {
    "",
    "INT",
    "STRING",
    "",
};

int copt_get_intv(int argc, char **argv)
{
    if (argv[c_arg][c_chr] == '\0')
    {
        c_chr = 0;
        c_arg++;
    }
    if (c_arg == argc)
        return errval = copt_res_parse_fail;

    int val = 0;
    while (argv[c_arg][c_chr] >= '0' && argv[c_arg][c_chr] <= '9')
    {
        val *= 10;
        val += (argv[c_arg][c_chr++] - '0');
    }

    if (argv[c_arg][c_chr] != '\0')
    {
        return errval = copt_res_parse_fail;
    }

    c_arg++;
    c_chr = 0;
    errval = copt_res_ok;
    return val;
}

int copt_get_strv(int argc, char **argv, copt_opt *opt)
{
    if (argv[c_arg][c_chr] == '\0')
    {
        c_chr = 0;
        c_arg++;
    }
    if (c_arg == argc)
        return errval = copt_res_parse_fail;

    if (strlen(argv[c_arg] + c_chr) > COPT_BUFFER_M_L)
        return errval = copt_res_length_fail;

    strcpy(copt_buf, argv[c_arg] + c_chr);
    int l = strlen(copt_buf);
    opt->strval = malloc((sizeof(char) * l) + 1);
    strcpy(opt->strval, copt_buf);

    c_arg++;
    c_chr = 0;

    return copt_res_ok;
}

int copt_get_short(int argc, char **argv, copt_opt *opts, int s, char c)
{
    int idx = -1;
    for (int i = 0; i < s; ++i)
    {
        if (opts[i].s == c)
        {
            idx = i;
            break;
        }
    }

    if (idx == -1)
        return copt_res_unrecognized;

    switch (opts[idx].t)
    {
    case copt_opt_flag:
        opts[idx].ival = 1;
        c_arg++;
        c_chr = 0;
        return copt_res_ok;
    case copt_opt_int:
        opts[idx].ival = copt_get_intv(argc, argv);
        opts[idx].ro_f = 1;
        return errval;
    case copt_opt_str:
        copt_get_strv(argc, argv, &opts[idx]);
        opts[idx].ro_f = 1;
        return errval;
    case copt_opt_help:
        p_help = 1;
        opts[idx].ro_f = 1;
        c_arg++;
        c_chr = 0;
        return copt_res_ok;
    default:
        return copt_res_parse_fail;
    }
}

int copt_get_long(int argc, char **argv, copt_opt *opts, int s)
{
    if (argv[c_arg][c_chr] == '\0')
    {
        c_chr = 0;
        c_arg++;
    }
    if (c_arg == argc)
        return errval = copt_res_parse_fail;

    if (strlen(argv[c_arg] + c_chr) > COPT_BUFFER_M_L)
        return errval = copt_res_length_fail;

    strcpy(copt_buf, argv[c_arg] + c_chr);

    c_arg++;
    c_chr = 0;

    int idx = -1;
    for (int i = 0; i < s; ++i)
    {
        if (!strcmp(opts[i].l, copt_buf))
        {
            idx = i;
            break;
        }
    }

    switch (opts[idx].t)
    {
    case copt_opt_flag:
        opts[idx].ival = 1;
        c_arg++;
        c_chr = 0;
        return copt_res_ok;
    case copt_opt_int:
        opts[idx].ival = copt_get_intv(argc, argv);
        opts[idx].ro_f = 1;
        return errval;
    case copt_opt_str:
        copt_get_strv(argc, argv, &opts[idx]);
        opts[idx].ro_f = 1;
        return errval;
    case copt_opt_help:
        p_help = 1;
        opts[idx].ro_f = 1;
        c_arg++;
        c_chr = 0;
        return errval;
    default:
        return copt_res_parse_fail;
    }
}

int copt_getopt(int argc, char **argv, copt_opt *opts, int s)
{
    int r;
    while (c_arg < argc)
    {
        if (argv[c_arg][c_chr++] != '-')
            return copt_res_parse_fail;
        char c = argv[c_arg][c_chr++];
        if ((c <= 'z' && c >= 'a') || (c <= 'Z' && c >= 'A'))
        {
            r = copt_get_short(argc, argv, opts, s, c);
        }
        else if (c == '-')
        {
            r = copt_get_long(argc, argv, opts, s);
        }
        else
            return copt_res_parse_fail;

        if (r != copt_res_ok)
        {
            return r;
        }
    }

    if (p_help == 1)
    {
        return copt_res_ok;
    }

    for (int i = 0; i < s; ++i)
    {
        if (opts[i].r == 1 && opts[i].ro_f == 0)
            return copt_res_req_fail;
    }

    return copt_res_ok;
}

void copt_deinit(copt_opt *opts, int s)
{
    for (int i = 0; i < s; ++i)
        free(opts[i].strval);
}

void copt_printf_help(copt_opt *opts, int s)
{
    // first printf help
    for (int i = 0; i < s; ++i)
    {
        switch (opts[i].t)
        {
        case copt_opt_help:
            if (strlen(opts[i].l) > 0 && opts[i].s != 0)
            {
                printf("\t-%c,--%-29s %s\n", opts[i].s, opts[i].l, opts[i].d);
            }
            else if (opts[i].s != 0)
            {
                printf("\t-%c%-34s %s\n", opts[i].s, "", opts[i].d);
            }
            else
            {
                printf("\t--%-36s %s\n", opts[i].l, opts[i].d);
            }
            break;
        default:
            break;
        }
    }
    printf("\n");
    for (int i = 0; i < s; ++i)
    {
        switch (opts[i].t)
        {
        case copt_opt_help:
            break;
        default:

            if (strlen(opts[i].l) && opts[i].s)
            {
                sprintf(copt_buf, "-%c, --%s", opts[i].s, opts[i].l);
            }
            else if (opts[i].s != 0)
            {
                sprintf(copt_buf, "-%c", opts[i].s);
            }
            else
            {
                sprintf(copt_buf, "--%s", opts[i].l);
            }
            printf("\t-%-16s %-6s%-10s %s\n", copt_buf, copt_type_str[opts[i].t], (opts[i].r == 1 ? "(required)" : ""), opts[i].d);
            break;
        }
    }
}