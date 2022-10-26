#include "elfHeader.h"

#include <libintl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "elf.h"

uint64_t byte_get_little_endian(const unsigned char *field, int size) {
  switch (size) {
  case 1:
    return *field;

  case 2:
    return ((unsigned int)(field[0])) | (((unsigned int)(field[1])) << 8);

  case 3:
    return ((uint64_t)(field[0])) | (((uint64_t)(field[1])) << 8) |
           (((uint64_t)(field[2])) << 16);

  case 4:
    return ((uint64_t)(field[0])) | (((uint64_t)(field[1])) << 8) |
           (((uint64_t)(field[2])) << 16) | (((uint64_t)(field[3])) << 24);

  case 5:
    if (sizeof(uint64_t) == 8)
      return ((uint64_t)(field[0])) | (((uint64_t)(field[1])) << 8) |
             (((uint64_t)(field[2])) << 16) | (((uint64_t)(field[3])) << 24) |
             (((uint64_t)(field[4])) << 32);
    else if (sizeof(uint64_t) == 4)
      /* We want to extract data from an 8 byte wide field and
         place it into a 4 byte wide field.  Since this is a little
         endian source we can just use the 4 byte extraction code.  */
      return ((uint64_t)(field[0])) | (((uint64_t)(field[1])) << 8) |
             (((uint64_t)(field[2])) << 16) | (((uint64_t)(field[3])) << 24);
    /* Fall through.  */

  case 6:
    if (sizeof(uint64_t) == 8)
      return ((uint64_t)(field[0])) | (((uint64_t)(field[1])) << 8) |
             (((uint64_t)(field[2])) << 16) | (((uint64_t)(field[3])) << 24) |
             (((uint64_t)(field[4])) << 32) | (((uint64_t)(field[5])) << 40);
    else if (sizeof(uint64_t) == 4)
      /* We want to extract data from an 8 byte wide field and
         place it into a 4 byte wide field.  Since this is a little
         endian source we can just use the 4 byte extraction code.  */
      return ((uint64_t)(field[0])) | (((uint64_t)(field[1])) << 8) |
             (((uint64_t)(field[2])) << 16) | (((uint64_t)(field[3])) << 24);
    /* Fall through.  */

  case 7:
    if (sizeof(uint64_t) == 8)
      return ((uint64_t)(field[0])) | (((uint64_t)(field[1])) << 8) |
             (((uint64_t)(field[2])) << 16) | (((uint64_t)(field[3])) << 24) |
             (((uint64_t)(field[4])) << 32) | (((uint64_t)(field[5])) << 40) |
             (((uint64_t)(field[6])) << 48);
    else if (sizeof(uint64_t) == 4)
      /* We want to extract data from an 8 byte wide field and
         place it into a 4 byte wide field.  Since this is a little
         endian source we can just use the 4 byte extraction code.  */
      return ((uint64_t)(field[0])) | (((uint64_t)(field[1])) << 8) |
             (((uint64_t)(field[2])) << 16) | (((uint64_t)(field[3])) << 24);
    /* Fall through.  */

  case 8:
    if (sizeof(uint64_t) == 8)
      return ((uint64_t)(field[0])) | (((uint64_t)(field[1])) << 8) |
             (((uint64_t)(field[2])) << 16) | (((uint64_t)(field[3])) << 24) |
             (((uint64_t)(field[4])) << 32) | (((uint64_t)(field[5])) << 40) |
             (((uint64_t)(field[6])) << 48) | (((uint64_t)(field[7])) << 56);
    else if (sizeof(uint64_t) == 4)
      /* We want to extract data from an 8 byte wide field and
         place it into a 4 byte wide field.  Since this is a little
         endian source we can just use the 4 byte extraction code.  */
      return ((uint64_t)(field[0])) | (((uint64_t)(field[1])) << 8) |
             (((uint64_t)(field[2])) << 16) | (((uint64_t)(field[3])) << 24);
    /* Fall through.  */

  default:
    printf("Unhandled data length: %d\n", size);
    abort();
  }
}

void byte_put_little_endian(unsigned char *field, uint64_t value, int size) {
  switch (size) {
  case 8:
    field[7] = (((value >> 24) >> 24) >> 8) & 0xff;
    field[6] = ((value >> 24) >> 24) & 0xff;
    field[5] = ((value >> 24) >> 16) & 0xff;
    field[4] = ((value >> 24) >> 8) & 0xff;
    /* Fall through.  */
  case 4:
    field[3] = (value >> 24) & 0xff;
    /* Fall through.  */
  case 3:
    field[2] = (value >> 16) & 0xff;
    /* Fall through.  */
  case 2:
    field[1] = (value >> 8) & 0xff;
    /* Fall through.  */
  case 1:
    field[0] = value & 0xff;
    break;

  default:
    printf("Unhandled data length: %d\n", size);
    abort();
  }
}

int elfHdrIdent(Elf64_Ehdr *elfHdr, Elf64_Info_Ehdr *elfInfo) {
  if (elfHdr->e_ident[EI_MAG0] != ELFMAG0 ||
      elfHdr->e_ident[EI_MAG1] != ELFMAG1 ||
      elfHdr->e_ident[EI_MAG2] != ELFMAG2 ||
      elfHdr->e_ident[EI_MAG3] != ELFMAG3)
    return false;

  if (elfHdr->e_ident[EI_CLASS] == ELFCLASS32)
    elfInfo->i_class = "ELF32";
  else if (elfHdr->e_ident[EI_CLASS] == ELFCLASS64)
    elfInfo->i_class = "ELF64";
  else
    return false;

  if (elfHdr->e_ident[EI_DATA] == ELFDATA2LSB)
    elfInfo->i_dataEncodingForm = "2's complement, little endian";
  else if (elfHdr->e_ident[EI_DATA] == ELFDATA2MSB)
    elfInfo->i_dataEncodingForm = "2's complement, big endian";
  else
    return false;
  return 0;
}

void elfHdrType(Elf64_Ehdr *elfHdr, Elf64_Info_Ehdr *elfInfo) {
  switch (elfHdr->e_type) {
  case ET_NONE:
    elfInfo->i_type = "NONE (None)";
    break;
  case ET_REL:
    elfInfo->i_type = "REL (Relocatable file)";
    break;
  case ET_EXEC:
    elfInfo->i_type = "EXEC (Executable file)";
    break;
  case ET_DYN:
    elfInfo->i_type = "DYN (Shared object file)";
    break;
  case ET_CORE:
    elfInfo->i_type = "CORE (Core file)";
    break;

  default:
    printf("Unsupported Elf Type\n");
    abort();
  }
}

void elfHdrMachine(Elf64_Ehdr *elfHdr, Elf64_Info_Ehdr *elfInfo) {
  switch (elfHdr->e_machine) {
  case EM_RISCV:
    elfInfo->i_machine = "RISC-V";
    break;

  default:
    printf("Unsupported Machine\n");
    abort();
  }
}

int elfHdrVersion(Elf64_Ehdr *elfHdr, Elf64_Info_Ehdr *elfInfo) {
  if (elfHdr->e_version != elfHdr->e_ident[EI_VERSION])
    return false;
  elfInfo->i_version = elfHdr->e_version;
  return 0;
}

int elfHdrFlags(Elf64_Ehdr *elfHdr, Elf64_Info_Ehdr *elfInfo) {
  if (elfHdr->e_version != elfHdr->e_ident[EI_VERSION])
    return false;

  if (elfHdr->e_machine == EM_RISCV) {
    if (elfHdr->e_flags & EF_RISCV_RVC)
      elfInfo->i_flagStatement = ", RVC";

    if (elfHdr->e_flags & EF_RISCV_RVE)
      elfInfo->i_flagStatement = ", RVE";

    switch (elfHdr->e_flags & EF_RISCV_FLOAT_ABI) {
    case EF_RISCV_FLOAT_ABI_SOFT:
      elfInfo->i_flagStatement = ", soft-float ABI";
      break;

    case EF_RISCV_FLOAT_ABI_SINGLE:
      elfInfo->i_flagStatement = ", single-float ABI";
      break;

    case EF_RISCV_FLOAT_ABI_DOUBLE:
      elfInfo->i_flagStatement = ", double-float ABI";
      break;

    case EF_RISCV_FLOAT_ABI_QUAD:
      elfInfo->i_flagStatement = ", quad-float ABI";
      break;

    default:
      printf("Unsupported RISC-V flag\n");
      abort();
    }
  }
  return 0;
}

void elfHdrEntry(Elf64_Ehdr *elfHdr, Elf64_Info_Ehdr *elfInfo) {
  elfInfo->i_entry = elfHdr->e_entry;
}

void elfHdrPhoff(Elf64_Ehdr *elfHdr, Elf64_Info_Ehdr *elfInfo) {
  elfInfo->i_phoff = elfHdr->e_phoff;
}

void elfHdrShoff(Elf64_Ehdr *elfHdr, Elf64_Info_Ehdr *elfInfo) {
  elfInfo->i_shoff = elfHdr->e_shoff;
}

void elfHdrFlag(Elf64_Ehdr *elfHdr, Elf64_Info_Ehdr *elfInfo) {
  elfInfo->i_flags = elfHdr->e_flags;
}

void elfHdrFlag(Elf64_Ehdr *elfHdr, Elf64_Info_Ehdr *elfInfo) {
  elfInfo->i_flags = elfHdr->e_flags;
}

void elfHdrEhsize(Elf64_Ehdr *elfHdr, Elf64_Info_Ehdr *elfInfo) {
  elfInfo->i_ehsize = elfHdr->e_ehsize;
}

void elfHdrPhentsize(Elf64_Ehdr *elfHdr, Elf64_Info_Ehdr *elfInfo) {
  elfInfo->i_phentsize = elfHdr->e_phentsize;
}

void elfHdrPhnum(Elf64_Ehdr *elfHdr, Elf64_Info_Ehdr *elfInfo) {
  elfInfo->i_phnum = elfHdr->e_phnum;
}

void elfHdrShentsize(Elf64_Ehdr *elfHdr, Elf64_Info_Ehdr *elfInfo) {
  elfInfo->i_shentsize = elfHdr->e_shentsize;
}

void elfHdrShnum(Elf64_Ehdr *elfHdr, Elf64_Info_Ehdr *elfInfo) {
  elfInfo->i_shnum = elfHdr->e_shnum;
}
void elfHdrFlag(Elf64_Ehdr *elfHdr, Elf64_Info_Ehdr *elfInfo) {
  elfInfo->i_shstrndx = elfHdr->e_shstrndx;
}

void printfElf64Header(Elf64_Info_Ehdr *elfInfo) {
  fprintf(stderr, "\n\n");
  fprintf(stderr, "ELF Header:\n");

  fprintf(stderr, "  Magic Number:                      ");
  for (int i = 0; i < EI_NIDENT; i++)
    fprintf(stderr, "%2.2x ", elfInfo->i_ident[i]);
  fprintf(stderr, "\n");

  fprintf(stderr, "  Binary File Class:                 %s\n",
          elfInfo->i_class);
  fprintf(stderr, "  Data Encoding Form:                %s\n",
          elfInfo->i_dataEncodingForm);
  fprintf(stderr, "  Elf Type:                          %s\n", elfInfo->i_type);
  fprintf(stderr, "  Machine:                           %s\n",
          elfInfo->i_machine);
  fprintf(stderr, "  Version:                           %u\n",
          elfInfo->i_version);
  fprintf(stderr, "  Entry point address:               0x%lx\n",
          elfInfo->i_entry);
  fprintf(stderr,
          "  Start of program headers:          0x%lx (bytes into file)\n",
          elfInfo->i_phoff);
  fprintf(stderr,
          "  Start of section headers:          0x%lx (bytes into file)\n",
          elfInfo->i_shoff);
  fprintf(stderr, "  Flags:                             %u%s\n",
          elfInfo->i_flags, elfInfo->i_flagStatement);
  fprintf(stderr, "  Size of this header:               %u (bytes)\n",
          elfInfo->i_ehsize);
  fprintf(stderr, "  Size of program headers:           %u (bytes)\n",
          elfInfo->i_phentsize);
  fprintf(stderr, "  Number of program headers:         %u\n",
          elfInfo->i_phnum);
  fprintf(stderr, "  Size of section headers:           %u (bytes)\n",
          elfInfo->i_shentsize);
  fprintf(stderr, "  Number of section headers:         %u\n",
          elfInfo->i_shstrndx + 1);
  fprintf(stderr, "\n\n");
}

int processElfHeader(const char *inputFileName) {
  Elf64_Ehdr elfHdr;
  Elf64_External_Ehdr externalElf64Hdr;
  Elf64_Info_Ehdr infoElf64Hdr;

  FILE *fileHandle = fopen(inputFileName, "rb");

  fread(externalElf64Hdr.e_ident, EI_NIDENT, 1, fileHandle);
  fread(externalElf64Hdr.e_type, sizeof(externalElf64Hdr) - EI_NIDENT, 1,
        fileHandle);
  strcpy(infoElf64Hdr.i_ident, externalElf64Hdr.e_ident);
  strcpy(elfHdr.e_ident, externalElf64Hdr.e_ident);
  elfHdr.e_type =
      byte_get_little_endian(externalElf64Hdr.e_type, sizeof(elfHdr.e_type));
  elfHdr.e_machine = byte_get_little_endian(externalElf64Hdr.e_machine,
                                            sizeof(elfHdr.e_machine));
  elfHdr.e_version = byte_get_little_endian(externalElf64Hdr.e_version,
                                            sizeof(elfHdr.e_version));
  elfHdr.e_entry =
      byte_get_little_endian(externalElf64Hdr.e_entry, sizeof(elfHdr.e_entry));
  elfHdr.e_phoff =
      byte_get_little_endian(externalElf64Hdr.e_phoff, sizeof(elfHdr.e_phoff));
  elfHdr.e_shoff =
      byte_get_little_endian(externalElf64Hdr.e_shoff, sizeof(elfHdr.e_shoff));
  elfHdr.e_flags =
      byte_get_little_endian(externalElf64Hdr.e_flags, sizeof(elfHdr.e_flags));
  elfHdr.e_ehsize = byte_get_little_endian(externalElf64Hdr.e_ehsize,
                                           sizeof(elfHdr.e_ehsize));
  elfHdr.e_phentsize = byte_get_little_endian(externalElf64Hdr.e_phentsize,
                                              sizeof(elfHdr.e_phentsize));
  elfHdr.e_phnum =
      byte_get_little_endian(externalElf64Hdr.e_phnum, sizeof(elfHdr.e_phnum));
  elfHdr.e_shentsize = byte_get_little_endian(externalElf64Hdr.e_shentsize,
                                              sizeof(elfHdr.e_shentsize));
  elfHdr.e_shnum =
      byte_get_little_endian(externalElf64Hdr.e_shnum, sizeof(elfHdr.e_shnum));
  elfHdr.e_shstrndx = byte_get_little_endian(externalElf64Hdr.e_shstrndx,
                                             sizeof(elfHdr.e_shstrndx));
  elfHdrIdent(&elfHdr, &infoElf64Hdr);
  elfHdrType(&elfHdr, &infoElf64Hdr);
  elfHdrMachine(&elfHdr, &infoElf64Hdr);
  elfHdrVersion(&elfHdr, &infoElf64Hdr);
  elfHdrFlags(&elfHdr, &infoElf64Hdr);

  printfElf64Header(&infoElf64Hdr);

  return 0;
}
