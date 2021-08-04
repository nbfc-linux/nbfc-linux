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
  {"-q|--quiet",               -'q',         0},
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

static struct {
  bool        A, B, C;
  bool        F;
  unsigned    u;
  int         i;
  float       f;
  double      d;
  long double D;
  char        c;
  bool        b;
  const char* s;
  const char* o;
  unsigned    r;
  bool        quiet;
  bool        print_flags;
  const char* command;
  const char* pos1;
} options;

static void parse_opts(int argc, char* const argv[]) {
  cli99 p;
  cli99_Init(&p, argc, argv, cli_options, cli99_options_python);

  int o;
  while ((o = cli99_GetOpt(&p))) {
    if (0)
      printf("Handling: %d (%c)\n", o, abs(o));

    switch (o) {
    case 1:     options.pos1 = p.optarg; break;
    case -'F':  options.F = 1;           break;
    case -'c':  options.c = p.optval.c;  break;
    case -'f':  options.f = p.optval.f;  break;
    case -'d':  options.d = p.optval.d;  break;
    case -'D':  options.D = p.optval.d;  break;
    case -'i':  options.i = p.optval.i;  break;
    case -'u':  options.u = p.optval.u;  break;
    case -'b':  options.b = p.optval.b;  break;
    case -'s':  options.s = p.optarg;    break;
    case -'o':  options.o = p.optarg;    break;
    case -'A':  options.A = 1;           break;
    case -'B':  options.B = 1;           break;
    case -'C':  options.C = 1;           break;
    case -'q':  options.quiet = 1;       break;
    case -'P':  cli99_PrintFlagMask(&p); break;
    case -'r':  options.r = p.optval.u;  break;
    case  'C':  options.command = p.optarg;
                if (! strcmp(p.optarg, "positionals"))
                  cli99_SetOptions(&p, cli_test_positionals, false);
                else
                  printf("Unknown command %s\n", p.optarg);
                break;
    default:    cli99_ExplainError(&p);  goto end;
    }
  }
end:

  if (options.quiet)
    return;

  if (! cli99_CheckRequired(&p))
    cli99_ExplainError(&p);

#define print_opt(O, FMT) printf("-" #O ": " FMT "\n", options.O)
  print_opt(A, "%d");
  print_opt(B, "%d");
  print_opt(C, "%d");
  print_opt(F, "%d");
  print_opt(c, "%c");
  print_opt(f, "%f");
  print_opt(i, "%d");
  print_opt(u, "%u");
  print_opt(b, "%d");
  print_opt(s, "%s");
  print_opt(o, "%s");
  print_opt(command, "%s");
  print_opt(pos1, "%s");

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
  for (unsigned i = 0; i < options.r; ++i)
    parse_opts(argc, argv);
  return 0;
}

