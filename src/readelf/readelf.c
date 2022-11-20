#include "elfHeader.h"
#include "programHeader.h"
#include "sectionHeader.h"

#include "GCC.command.line.h"
#include "comment.h"
#include "debug_str.h"
#include "riscv_attributes.h"

#include "debug_frame.h"

#include "dumpTextSection.h"

int main(int argc, char **argv) {
  const char *inputFileName = argv[1];

  processElfHeader(inputFileName);
  processSectionHeader(inputFileName);
  processProgramHeader(inputFileName);

  dumpDebug_str(inputFileName);
  dumpComment(inputFileName);
  dumpRiscv_attributes(inputFileName);
  dumpGCC_command_line(inputFileName);

  analysisDebug_frame(inputFileName);

  disassembleText(inputFileName);

  return 0;
}
