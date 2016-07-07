#include <argp.h>
#include "fbs_argp.h"
#include "db_object.h"

static error_t global_parser(int key, char *arg, struct argp_state *state){
	switch(key){
		case 'c':
			/*TODO: Length checking */
			CONFIG_FILE = arg;
			break;
		default:
			return ARGP_ERR_UNKNOWN;
	}
	return 0;
}

static struct argp_option options[] = {
	{"config", 'c', "file", 0, 0 },
	{0}
};

struct argp global_argp = {options, global_parser, 0, 0, 0, 0};

struct argp_child global_child[2] = {
	{&global_argp, 0, 0, 0},
	{0}
};
