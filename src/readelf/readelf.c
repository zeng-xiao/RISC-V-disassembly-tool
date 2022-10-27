#include "elfHeader.h"
#include "programHeader.h"
#include "sectionHeader.h"

int main(int argc, char **argv) {
  const char *inputFileName = argv[1];

  processElfHeader(inputFileName);
  processSectionHeader(inputFileName);
  processProgramHeader(inputFileName);

  return 0;
}
