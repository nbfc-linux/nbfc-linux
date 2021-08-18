#define cli99_EnabledFlags 0x000000000201FC3FuLL

#include "optparse.h"
#include "optparse.c"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <locale.h>
#include <assert.h>

static const cli99_option cli_options[] = {
  cli99_exclusive_group("Exclusive group 1", 0),
  {"-A|--exclusive-A",         -'A',         0},
  {"-B|--exclusive-B",         -'B',         0},
  cli99_end_exclusive_group(),
  {"-F|--flag",                -'F',         0},
  {"-u|--unsigned",            -'u',         1|cli99_type(unsigned)},
  {"-i|--int",                 -'i',         1|cli99_type(int)},
  {"-f|--float",               -'f',         1|cli99_type(float)},
  {"-d|--double",              -'d',         1|cli99_type(double)},
  {"-D|--long-double",         -'D',         1|cli99_type(long double)},
  {"-c|--char",                -'c',         1|cli99_type(char)},
  {"-b|--bool",                -'b',         1|cli99_type(bool)},
  {"-s|--string",              -'s',         1},
  {"-o|--optional",            -'o',        '?'},
  {"--print-flags",            -'P',         0},
  {"-r|--repeat",              -'r',         1|cli99_type(unsigned)},
  {"command",                   'C',         1|cli99_required_option},
  cli99_options_end(),
};

static const cli99_option cli_test_positionals[] = {
  cli99_include_options(cli_options),
  {"pos1",                       1 ,         1},
  cli99_options_end(),
};

static void parse_opts(int argc, char* const argv[]) {
  cli99 p;
  cli99_Init(&p, argc, argv, cli_options, cli99_options_python);

  int o;
  while ((o = cli99_GetOpt(&p))) {
    if (0)
      printf("Handling: %d (%c)\n", o, abs(o));

    switch (o) {
    case 1:     printf("--pos1=%s ", p.optarg); break;
    case -'F':  printf("-F=True ");             break;
    case -'c':  printf("-c=%c ",  p.optval.c);  break;
    case -'f':  printf("-f=%f ",  p.optval.f);  break;
    case -'d':  printf("-d=%f ", p.optval.d);   break;
    case -'D':  printf("-D=%f ", p.optval.d);   break;
    case -'i':  printf("-i=%d ", p.optval.i);   break;
    case -'u':  printf("-u=%u ", p.optval.u);   break;
    case -'b':  printf("-b=True ");             break;
    case -'s':  printf("-s=%s ", p.optarg);     break;
    case -'o':  printf("-o=%s ", p.optarg);     break;
    case -'A':  printf("-A=True ");             break;
    case -'B':  printf("-B True ");             break;
    case -'C':  printf("-C True ");             break;
//  case -'P':  cli99_PrintFlagMask(&p); break;
//  case -'r':  options.r = p.optval.u;  break;
    case  'C':  printf("command=%s ", p.optarg);
                if (! strcmp(p.optarg, "positionals"))
                  cli99_SetOptions(&p, cli_test_positionals, false);
                else if (! strcmp(p.optarg, "test"))
                    ;
                else
                  printf("Unknown command %s\n", p.optarg);
                break;
    default:    cli99_ExplainError(&p);  goto end;
    }
  }
end:

/*
  if (options.quiet)
    return;
*/

  if (! cli99_CheckRequired(&p))
    cli99_ExplainError(&p);

  if (!cli99_End(&p)) {
    fprintf(stderr, "Too much arguments\n");
    while (cli99_GetArg(&p, 1))
      printf("%s ", p.optarg);
    exit(1);
  }
}

int main(int argc, char* const argv[])
{
  setlocale(LC_NUMERIC, "C");
  parse_opts(argc, argv);
/*
  for (unsigned i = 0; i < options.r; ++i)
    parse_opts(argc, argv);
*/
  return 0;
}

