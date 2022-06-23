#ifndef COPT_H
#define COPT_H

#define COPT_OPTS_LENGTH(X) (sizeof(X) / sizeof(copt_opt))
#define COPT_BUFFER_M_L 1024

enum
{
    copt_res_ok,
    copt_res_parse_fail,
    copt_res_length_fail,
    copt_res_req_fail,
    copt_res_unrecognized
};

enum
{
    copt_opt_flag,
    copt_opt_int,
    copt_opt_str,
    copt_opt_help,
};

struct copt_opt
{
    const char s;  // short
    const char *l; // long
    const char *d; // description
    char t;        // type
    int r;         // required

    int ro_f; // read_only access - found

    union
    {
        int ival;     // int val
        char *strval; // str_val
    };
};
typedef struct copt_opt copt_opt;

int copt_getopt(int argc, char **argv, copt_opt *opts, int s);
void copt_deinit(copt_opt *opts, int s);
void copt_printf_help(copt_opt *opts, int s);

#endif // COPT_H