#include <stdio.h>

#define WARRANTY_TEXT \
"NBFC-Linux is provided \"as is\", without any warranty of any kind.\n"        \
"\n"                                                                           \
"This software allows direct control over the embedded controller (EC) and\n"  \
"fan system of your device. Misconfiguration or incorrect usage may lead\n"    \
"to hardware damage, overheating, or unpredictable system behavior. In\n"      \
"some cases, modifying EC settings can cause changes that persist beyond\n"    \
"software usage and may affect your device's stability or usability.\n"        \
"\n"                                                                           \
"By using NBFC-Linux, you acknowledge that:\n"                                 \
"\n"                                                                           \
"- You are using the software at your own risk.\n"                             \
"- The developers and contributors are not responsible for any damage to\n"    \
"  hardware, data loss, or other consequences resulting from its use.\n"       \
"- No guarantees are given regarding the suitability, reliability, or\n"       \
"  accuracy of the software.\n"                                                \
"\n"                                                                           \
"NBFC-Linux is licensed under the GNU General Public License v3.0. For\n"      \
"more information, please refer to the full license text included with\n"      \
"the source code."

int Warranty() {
  puts(WARRANTY_TEXT);
  return NBFC_EXIT_SUCCESS;
}
