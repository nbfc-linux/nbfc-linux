struct ShellArgs {
  const char* args[64];
  ssize_t count;
};

static void ShellRead(const struct ShellArgs* args) {
  Initialize_EC();

  int word = 0;
  const char* register_arg = NULL;
  const char* err;

  for (int i = 1; i < args->count; ++i) {
    const char* arg = args->args[i];

    if (arg[0] == '-') {
      if (!strcmp(arg, "-w") || !strcmp(arg, "--word"))
        word = 1;
      else {
        printf("ERR: Invalid option: %s\n", arg);
        return;
      }
    }
    else if (! register_arg)
      register_arg = arg;
    else {
      printf("ERR: Too much arguments\n");
      return;
    }
  }

  if (! register_arg) {
    printf("ERR: Missing argument (REGISTER)\n");
    return;
  }

  uint8_t register_ = (uint8_t) parse_number(register_arg, 0, word ? 254 : 255, &err);
  if (err) {
    printf("ERR: Argument (REGISTER): %s\n", err);
    return;
  }

  if (word) {
    uint16_t value;
    Error e = ec->ReadWord(register_, &value);
    if (e) {
      printf("ERR: %s\n", err_print_all(e));
      return;
    }

    printf("%d\n", value);
  }
  else {
    uint8_t value;
    Error e = ec->ReadByte(register_, &value);
    if (e) {
      printf("ERR: %s\n", err_print_all(e));
      return;
    }

    printf("%d\n", value);
  }
}

static void ShellWrite(const struct ShellArgs* args) {
  Initialize_EC();

  int word = 0;
  const char* register_arg = NULL;
  const char* value_arg = NULL;
  const char* err;

  for (int i = 1; i < args->count; ++i) {
    const char* arg = args->args[i];

    if (arg[0] == '-') {
      if (!strcmp(arg, "-w") || !strcmp(arg, "--word"))
        word = 1;
      else {
        printf("ERR: Invalid option: %s\n", arg);
        return;
      }
    }
    else if (! register_arg)
      register_arg = arg;
    else if (! value_arg)
      value_arg = arg;
    else {
      printf("ERR: Too much arguments\n");
      return;
    }
  }

  if (! register_arg) {
    printf("ERR: Missing argument (REGISTER)\n");
    return;
  }

  if (! value_arg) {
    printf("ERR: Missing argument (VALUE)\n");
    return;
  }

  uint8_t register_ = (uint8_t) parse_number(register_arg, 0, word ? 254 : 255, &err);
  if (err) {
    printf("ERR: Argument (REGISTER): %s\n", err);
    return;
  }

  uint16_t value = (uint16_t) parse_number(value_arg, 0, word ? UINT16_MAX : UINT8_MAX, &err);
  if (err) {
    printf("ERR: Argument (VALUE): %s\n", err);
    return;
  }

  if (word) {
    Error e = ec->WriteWord(register_, value);
    if (e) {
      printf("ERR: %s\n", err_print_all(e));
      return;
    }
  }
  else {
    Error e = ec->WriteByte(register_, (uint8_t) value);
    if (e) {
      printf("ERR: %s\n", err_print_all(e));
      return;
    }
  }

  printf("OK\n");
}

static void ShellReadAll(struct ShellArgs*) {
  Initialize_EC();

  uint8_t values[256];

  for (int register_ = 0; register_ <= 255; ++register_) {
    Error e = ec->ReadByte((uint8_t) register_, &values[register_]);
    if (e) {
      printf("ERR: %s\n", err_print_all(e));
      return;
    }
  }

  printf("%d", values[0]);
  for (int register_ = 1; register_ <= 255; ++register_)
    printf(" %d", values[register_]);
  printf("\n");
}

static void ShellHelp(void) {
  printf(
    "Available commands: \n"
    "  read [-w|--word] REGISTER\n"
    "  write [-w|--word] REGISTER VALUE\n"
    "  read_all\n"
    "  exit | quit\n"
  );
}

static const char* read_arg(char** line) {
  while (**line == ' ' || **line == '\t')
    ++*line;

  if (!**line)
    return NULL;

  const char* arg = *line;

  ++*line;

  while (**line && **line != ' ' && **line != '\t')
    ++*line;

  if (**line) {
    **line = '\0';
    ++*line;
  }

  return arg;
}

static void read_args(struct ShellArgs* args, char** line) {
  args->count = 0;

  while (args->count < ARRAY_SSIZE(args->args)) {
    if (! (args->args[args->count] = read_arg(line)))
      break;

    args->count++;
  }
}

static int Shell(void) {
  check_root();

  char buffer[8192];
  char* line;
  struct ShellArgs args;

  signal(SIGINT, SIG_DFL);
  signal(SIGTERM, SIG_DFL);

  while (fgets(buffer, sizeof(buffer), stdin)) {
    size_t len = strlen(buffer);

    if (buffer[len - 1] != '\n') {
      printf("ERR: Line too long\n");
      continue;
    }

    buffer[len - 1] = '\0';
    line = buffer;

    read_args(&args, &line);

    if (args.count == 0);
    else if (!strcmp(args.args[0], "read"))     ShellRead(&args);
    else if (!strcmp(args.args[0], "write"))    ShellWrite(&args);
    else if (!strcmp(args.args[0], "read_all")) ShellReadAll(&args);
    else if (!strcmp(args.args[0], "help"))     ShellHelp();
    else if (!strcmp(args.args[0], "exit"))     return NBFC_EXIT_SUCCESS;
    else if (!strcmp(args.args[0], "quit"))     return NBFC_EXIT_SUCCESS;
    else
      printf("ERR: No such command: %s (type `help` for a list of commands)\n", args.args[0]);

    fflush(stdout);
  }

  return NBFC_EXIT_SUCCESS;
}
