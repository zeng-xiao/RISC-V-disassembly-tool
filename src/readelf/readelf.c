#include "elfHeader.h"
#include "programHeader.h"
#include "sectionHeader.h"

#include "GCC.command.line.h"
#include "comment.h"
#include "debug_str.h"
#include "riscv_attributes.h"

#include "text.h"

int main(int argc, char **argv) {
  const char *inputFileName = argv[1];

  processElfHeader(inputFileName);
  processSectionHeader(inputFileName);
  processProgramHeader(inputFileName);

  dumpText(inputFileName);
  dumpDebug_str(inputFileName);
  dumpComment(inputFileName);
  dumpRiscv_attributes(inputFileName);
  dumpGCCcommandline(inputFileName);

  return 0;
}
