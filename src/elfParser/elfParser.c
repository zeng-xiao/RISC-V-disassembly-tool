#include "elfHeader.h"
#include "programHeader.h"
#include "sectionHeader.h"

#include "GCC.command.line.h"
#include "comment.h"
#include "debug_str.h"
#include "riscv_attributes.h"

#include "debug_frame.h"

#include "dumpTextSection.h"

void static dumpStr(const uint8_t *inputFileName) {
  dumpDebug_str(inputFileName);
  dumpComment(inputFileName);
  dumpRiscv_attributes(inputFileName);
  dumpGCC_command_line(inputFileName);
}

int main(int argc, char **argv) {
  const char *inputFileName = argv[1];

  processElfHeader(inputFileName);
  processSectionHeader(inputFileName);
  processProgramHeader(inputFileName);

  dumpStr(inputFileName);

  analysisDebugFrame(inputFileName);

  disassembleTextSection(inputFileName);

  return 0;
}
