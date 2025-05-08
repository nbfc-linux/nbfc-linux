#include <stdio.h>
#include <unistd.h>
#include <termios.h>

#define DONATE_PAYPAL_LINK "https://paypal.me/BenjaminAbendroth"

#define DONATE_GITHUB_LINK "https://github.com/nbfc-linux/nbfc-linux"

#define DONATE_TEXT                                                            \
  "Thank you for using NBFC-Linux!\n"                                          \
  "\n"                                                                         \
  "If you'd like to support the project, you can:\n"                           \
  "\n"                                                                         \
  " - Send a donation via PayPal:\n"                                           \
  "     " DONATE_PAYPAL_LINK "\n"                                              \
  "\n"                                                                         \
  " - Or simply star the project on GitHub:\n"                                 \
  "     " DONATE_GITHUB_LINK "\n"

static void AskAndOpen(const char* url) {
  char yn = 'n';
  printf("Would you like to open '%s' in your default browser? [y/N]\n", url);

  read(STDIN_FILENO, &yn, 1);
  if (yn == 'y' || yn == 'Y') {
    char command[1024];
    snprintf(command, sizeof(command), "xdg-open '%s' >/dev/null 2>/dev/null &", url);
    system(command);
  }
}

int Donate() {
  struct termios oldt, newt;

  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;

  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);

  puts(DONATE_TEXT);
  AskAndOpen(DONATE_PAYPAL_LINK);
  AskAndOpen(DONATE_GITHUB_LINK);

  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

  return NBFC_EXIT_SUCCESS;
}
