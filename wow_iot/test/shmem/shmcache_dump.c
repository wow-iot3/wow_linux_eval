#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <errno.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>

#include "shmcache/shm_hashtable.h"
#include "shmcache/wow_shmcache.h"

static char *config_filename = "/etc/libshmcache.conf";
static char *key_value_seperator = "=";
static char *row_seperator = "\n";
static int start_offset = 0;
static int row_count = 0;
static char *search_key  = NULL;
static bool show_usage = false;
static char *output_filename = NULL;

static void usage(char *program)
{
    fprintf(stderr, "Usage: %s options, the options as:\n"
            "\t -h help\n"
            "\t -c <config_filename>, default: /etc/libshmcache.conf\n"
            "\t -F <key_value_seperator>, default: =\n"
            "\t -R <row_seperator>, default: \\n\n"
            "\t -k <search_key>, SQL like style such as "
            "MYKEY%% or %%MYKEY or %%MYKEY%%\n"
            "\t -s <start> start offset, default: 0\n"
            "\t -n <row_count>, 0 means NO limit, default: 0\n"
            "\t -o <filename> output filename, default: output to sdtout\n"
            "\n"
            "\t the seperators can use escape characters, such as: \\\\ for \\, \n"
            "\t \\t for tab, \\r for carriage return, \\n for new line\n"
            "\n", program);
}

static char *unescape_string(const char *s)
{
    char *output;
    char *dest;
    const char *p;
    const char *end;
    const char *start;
    int src_len;
    int num_len;
    char buff[16];

    src_len = strlen(s);
    output = (char *)malloc(src_len + 1);
    if (output == NULL) {
        fprintf(stderr, "malloc %d bytes fail", src_len + 1);
        return NULL;
    }

    dest = output;
    end = s + src_len;
    p = s;
    while (p < end) {
        if (*p == '\\' && p + 1 < end) {
            switch (*(p+1)) {
                case 'r':
                    *dest++ = '\r';
                    break;
                case 'n':
                    *dest++ = '\n';
                    break;
                case 't':
                    *dest++ = '\t';
                    break;
                case 'f':
                    *dest++ = '\f';
                    break;
                case '\\':
                    *dest++ = '\\';
                    break;
                default:
                    if (*(p+1) >= '0' && *(p+1) <= '9') {
                        start = p + 1;
                        p += 2;
                        while ((p < end) && (*p >= '0' && *p <= '9')) {
                            p++;
                            if (p - start == 3) {
                                break;
                            }
                        }
                        num_len = p - start;
                        memcpy(buff, start, num_len);
                        *(buff + num_len) = '\0';
                        *dest++ = atoi(buff);
                    } else {
                        *dest++ = *p++;
                        *dest++ = *p++;
                    }
                    continue;
            }
            p += 2;
        } else {
            *dest++ = *p++;
        }
    }

    *dest = '\0';
    return output;
}

static void parse_args(int argc, char **argv)
{
    int ch;

    while ((ch = getopt(argc, argv, "hc:F:R:k:s:n:o:")) != -1) {
        switch (ch) {
            case 'c':
                config_filename = optarg;
                break;
            case 'F':
                key_value_seperator = unescape_string(optarg);
                break;
            case 'R':
                row_seperator = unescape_string(optarg);
                break;
            case 'k':
                search_key = optarg;
                break;
            case 's':
                start_offset = atoi(optarg);
                break;
            case 'n':
                row_count = atoi(optarg);
                break;
            case 'o':
                output_filename = optarg;
                break;
            case 'h':
            default:
                show_usage = true;
                usage(argv[0]);
                return;
        }
    }

    if (optind != argc) {
        show_usage = true;
        usage(argv[0]);
    }
}

int main(int argc, char **argv)
{
    Shmcache_T* context;

    parse_args(argc, argv);
    if (show_usage) {
        return 0;
    }

    context = wow_shmcache_init_from_file(config_filename,false);
    if(context == NULL) return -1;

    wow_shmcache_keyval_dump(context);

    wow_shmcache_destroy(context);

    return 0;
}
