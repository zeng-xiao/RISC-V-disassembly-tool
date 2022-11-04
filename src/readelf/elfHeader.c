#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dataOperate.h"
#include "elf.h"

#include "elfHeader.h"

static int elfHdrIdent(Elf64_Ehdr *elfHdr, Elf64_Info_Ehdr *elfInfo) {
  strcpy(elfInfo->i_ident, elfHdr->e_ident);

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

  elfInfo->i_fileVersion = elfHdr->e_ident[EI_VERSION];

  switch (elfHdr->e_ident[EI_OSABI]) {
  case ELFOSABI_NONE:
    elfInfo->i_osAbi = "UNIX - System V";
    break;
  case ELFOSABI_HPUX:
    elfInfo->i_osAbi = "UNIX - HP-UX";
    break;
  case ELFOSABI_NETBSD:
    elfInfo->i_osAbi = "UNIX - NetBSD";
    break;
  case ELFOSABI_GNU:
    elfInfo->i_osAbi = "UNIX - GNU";
    break;
  case ELFOSABI_SOLARIS:
    elfInfo->i_osAbi = "UNIX - Solaris";
    break;
  default:
    printf("Unsupported OS ABI\n");
    abort();
  }
  elfInfo->i_abiVersion = elfHdr->e_ident[EI_ABIVERSION];
  return 0;
}

static void elfHdrType(Elf64_Ehdr *elfHdr, Elf64_Info_Ehdr *elfInfo) {
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

static void elfHdrMachine(Elf64_Ehdr *elfHdr, Elf64_Info_Ehdr *elfInfo) {
  switch (elfHdr->e_machine) {
  case EM_RISCV:
    elfInfo->i_machine = "RISC-V";
    break;

  default:
    printf("Unsupported Machine\n");
    abort();
  }
}

static int elfHdrObjectVersion(Elf64_Ehdr *elfHdr, Elf64_Info_Ehdr *elfInfo) {
  if (elfHdr->e_version != elfHdr->e_ident[EI_VERSION])
    return false;
  elfInfo->i_objectVersion = elfHdr->e_version;
  return 0;
}

static void elfHdrEntry(Elf64_Ehdr *elfHdr, Elf64_Info_Ehdr *elfInfo) {
  elfInfo->i_entry = elfHdr->e_entry;
}

static void elfHdrPhoff(Elf64_Ehdr *elfHdr, Elf64_Info_Ehdr *elfInfo) {
  elfInfo->i_phoff = elfHdr->e_phoff;
}

static void elfHdrShoff(Elf64_Ehdr *elfHdr, Elf64_Info_Ehdr *elfInfo) {
  elfInfo->i_shoff = elfHdr->e_shoff;
  sectionHeadersAddress = elfHdr->e_shoff;
}

static void elfHdrFlag(Elf64_Ehdr *elfHdr, Elf64_Info_Ehdr *elfInfo) {
  elfInfo->i_flags = elfHdr->e_flags;

  if (elfHdr->e_machine == EM_RISCV) {
    if (elfHdr->e_flags & EF_RISCV_RVC)
      strcat(elfInfo->i_flagStatement, ", RVC");

    if (elfHdr->e_flags & EF_RISCV_RVE)
      strcat(elfInfo->i_flagStatement, ", RVE");

    switch (elfHdr->e_flags & EF_RISCV_FLOAT_ABI) {
    case EF_RISCV_FLOAT_ABI_SOFT:
      strcat(elfInfo->i_flagStatement, ", soft-float ABI");
      break;

    case EF_RISCV_FLOAT_ABI_SINGLE:
      strcat(elfInfo->i_flagStatement, ", single-float ABI");
      break;

    case EF_RISCV_FLOAT_ABI_DOUBLE:
      strcat(elfInfo->i_flagStatement, ", double-float ABI");
      break;

    case EF_RISCV_FLOAT_ABI_QUAD:
      strcat(elfInfo->i_flagStatement, ", quad-float ABI");
      break;

    default:
      printf("Unsupported RISC-V flag\n");
      abort();
    }
  }
}

static void elfHdrEhsize(Elf64_Ehdr *elfHdr, Elf64_Info_Ehdr *elfInfo) {
  elfInfo->i_ehsize = elfHdr->e_ehsize;
}

static void elfHdrPhentsize(Elf64_Ehdr *elfHdr, Elf64_Info_Ehdr *elfInfo) {
  elfInfo->i_phentsize = elfHdr->e_phentsize;
}

static void elfHdrPhnum(Elf64_Ehdr *elfHdr, Elf64_Info_Ehdr *elfInfo) {
  elfInfo->i_phnum = elfHdr->e_phnum;
}

static void elfHdrShentsize(Elf64_Ehdr *elfHdr, Elf64_Info_Ehdr *elfInfo) {
  elfInfo->i_shentsize = elfHdr->e_shentsize;
  sectionSize = elfHdr->e_shentsize;
}

static void elfHdrShnum(Elf64_Ehdr *elfHdr, Elf64_Info_Ehdr *elfInfo) {
  elfInfo->i_shnum = elfHdr->e_shnum;
  sectionNumber = elfHdr->e_shnum;
}

static void elfHdrShstrndx(Elf64_Ehdr *elfHdr, Elf64_Info_Ehdr *elfInfo) {
  elfInfo->i_shstrndx = elfHdr->e_shstrndx;
}

static void printfElf64Header(Elf64_Info_Ehdr *elfInfo) {
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
  fprintf(stderr, "  File Version:                      %u (current)\n",
          elfInfo->i_fileVersion);
  fprintf(stderr, "  Elf Type:                          %s\n", elfInfo->i_type);
  fprintf(stderr, "  Machine:                           %s\n",
          elfInfo->i_machine);
  fprintf(stderr, "  Object Version:                    %u\n",
          elfInfo->i_objectVersion);
  fprintf(stderr, "  OS/ABI:                            %s\n",
          elfInfo->i_osAbi);
  fprintf(stderr, "  ABI Version:                       %u\n",
          elfInfo->i_abiVersion);
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

  closeFile(fileHandle);

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
  elfHdrObjectVersion(&elfHdr, &infoElf64Hdr);
  elfHdrEntry(&elfHdr, &infoElf64Hdr);
  elfHdrPhoff(&elfHdr, &infoElf64Hdr);
  elfHdrShoff(&elfHdr, &infoElf64Hdr);
  elfHdrFlag(&elfHdr, &infoElf64Hdr);
  elfHdrEhsize(&elfHdr, &infoElf64Hdr);
  elfHdrPhentsize(&elfHdr, &infoElf64Hdr);
  elfHdrPhnum(&elfHdr, &infoElf64Hdr);
  elfHdrShentsize(&elfHdr, &infoElf64Hdr);
  elfHdrShnum(&elfHdr, &infoElf64Hdr);
  elfHdrShstrndx(&elfHdr, &infoElf64Hdr);

  printfElf64Header(&infoElf64Hdr);

  return 0;
}
