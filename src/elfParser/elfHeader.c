#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dataOperate.h"
#include "elf.h"

#include "elfHeader.h"

uint64_t shdrAddress;     /* Section header table file offset */
uint16_t shdrNumber;      /* Section header table entry count */
uint16_t shdrSize;        /* Section header table entry size */
uint16_t shdrStrtabIndex; /* Section header table entry size */

static int ehdrIdent(Elf64_Ehdr *ehdr, Elf64_Info_Ehdr *elfInfo) {
  strcpy(elfInfo->i_ident, ehdr->e_ident);

  if (ehdr->e_ident[EI_MAG0] != ELFMAG0 || ehdr->e_ident[EI_MAG1] != ELFMAG1 ||
      ehdr->e_ident[EI_MAG2] != ELFMAG2 || ehdr->e_ident[EI_MAG3] != ELFMAG3)
    return false;

  if (ehdr->e_ident[EI_CLASS] == ELFCLASS32)
    elfInfo->i_class = "ELF32";
  else if (ehdr->e_ident[EI_CLASS] == ELFCLASS64)
    elfInfo->i_class = "ELF64";
  else
    return false;

  if (ehdr->e_ident[EI_DATA] == ELFDATA2LSB)
    elfInfo->i_dataEncodingForm = "2's complement, little endian";
  else if (ehdr->e_ident[EI_DATA] == ELFDATA2MSB)
    elfInfo->i_dataEncodingForm = "2's complement, big endian";
  else
    return false;

  elfInfo->i_fileVersion = ehdr->e_ident[EI_VERSION];

  switch (ehdr->e_ident[EI_OSABI]) {
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
  elfInfo->i_abiVersion = ehdr->e_ident[EI_ABIVERSION];
  return 0;
}

static void ehdrType(Elf64_Ehdr *ehdr, Elf64_Info_Ehdr *elfInfo) {
  switch (ehdr->e_type) {
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

static void ehdrMachine(Elf64_Ehdr *ehdr, Elf64_Info_Ehdr *elfInfo) {
  switch (ehdr->e_machine) {
  case EM_RISCV:
    elfInfo->i_machine = "RISC-V";
    break;

  default:
    printf("Unsupported Machine\n");
    abort();
  }
}

static int ehdrObjectVersion(Elf64_Ehdr *ehdr, Elf64_Info_Ehdr *elfInfo) {
  if (ehdr->e_version != ehdr->e_ident[EI_VERSION])
    return false;
  elfInfo->i_objectVersion = ehdr->e_version;
  return 0;
}

static void ehdrEntry(Elf64_Ehdr *ehdr, Elf64_Info_Ehdr *elfInfo) {
  elfInfo->i_entry = ehdr->e_entry;
}

static void ehdrPhoff(Elf64_Ehdr *ehdr, Elf64_Info_Ehdr *elfInfo) {
  elfInfo->i_phoff = ehdr->e_phoff;
}

static void ehdrShoff(Elf64_Ehdr *ehdr, Elf64_Info_Ehdr *elfInfo) {
  elfInfo->i_shoff = ehdr->e_shoff;
  shdrAddress = ehdr->e_shoff;
}

static void ehdrFlag(Elf64_Ehdr *ehdr, Elf64_Info_Ehdr *elfInfo) {
  static uint8_t buff[1024];

  elfInfo->i_flags = ehdr->e_flags;

  if (ehdr->e_machine == EM_RISCV) {
    if (ehdr->e_flags & EF_RISCV_RVC)
      strcat(buff, ", RVC");

    if (ehdr->e_flags & EF_RISCV_RVE)
      strcat(buff, ", RVE");

    switch (ehdr->e_flags & EF_RISCV_FLOAT_ABI) {
    case EF_RISCV_FLOAT_ABI_SOFT:
      strcat(buff, ", soft-float ABI");
      break;

    case EF_RISCV_FLOAT_ABI_SINGLE:
      strcat(buff, ", single-float ABI");
      break;

    case EF_RISCV_FLOAT_ABI_DOUBLE:
      strcat(buff, ", double-float ABI");
      break;

    case EF_RISCV_FLOAT_ABI_QUAD:
      strcat(buff, ", quad-float ABI");
      break;

    default:
      printf("Unsupported RISC-V flag\n");
      abort();
    }
    elfInfo->i_flagStatement = buff;
  }
}

static void ehdrEhsize(Elf64_Ehdr *ehdr, Elf64_Info_Ehdr *elfInfo) {
  elfInfo->i_ehsize = ehdr->e_ehsize;
}

static void ehdrPhentsize(Elf64_Ehdr *ehdr, Elf64_Info_Ehdr *elfInfo) {
  elfInfo->i_phentsize = ehdr->e_phentsize;
}

static void ehdrPhnum(Elf64_Ehdr *ehdr, Elf64_Info_Ehdr *elfInfo) {
  elfInfo->i_phnum = ehdr->e_phnum;
}

static void ehdrShentsize(Elf64_Ehdr *ehdr, Elf64_Info_Ehdr *elfInfo) {
  elfInfo->i_shentsize = ehdr->e_shentsize;
  shdrSize = ehdr->e_shentsize;
}

static void ehdrShnum(Elf64_Ehdr *ehdr, Elf64_Info_Ehdr *elfInfo) {
  elfInfo->i_shnum = ehdr->e_shnum;
  shdrNumber = ehdr->e_shnum;
}

static void ehdrShstrndx(Elf64_Ehdr *ehdr, Elf64_Info_Ehdr *elfInfo) {
  elfInfo->i_shstrndx = ehdr->e_shstrndx;
  shdrStrtabIndex = ehdr->e_shstrndx;
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
  fprintf(stderr, "  Size of this ELF header:           %u (bytes)\n",
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

int processElfHeader(const uint8_t *inputFileName) {
  Elf64_Ehdr ehdr;
  Elf64_Info_Ehdr i_ehdr;
  Elf64_Auxiliary_Ehdr a_ehdr;

  FILE *fileHandle = fopen(inputFileName, "rb");
  fread(a_ehdr.a_e_ident, 1, EI_NIDENT, fileHandle);
  fread(a_ehdr.a_e_type, 1, sizeof(a_ehdr) - EI_NIDENT, fileHandle);

  closeFile(fileHandle);

  strcpy(ehdr.e_ident, a_ehdr.a_e_ident);

  ehdr.e_type = byte_get_little_endian(a_ehdr.a_e_type, sizeof(ehdr.e_type));
  ehdr.e_machine =
      byte_get_little_endian(a_ehdr.a_e_machine, sizeof(ehdr.e_machine));
  ehdr.e_version =
      byte_get_little_endian(a_ehdr.a_e_version, sizeof(ehdr.e_version));
  ehdr.e_entry = byte_get_little_endian(a_ehdr.a_e_entry, sizeof(ehdr.e_entry));
  ehdr.e_phoff = byte_get_little_endian(a_ehdr.a_e_phoff, sizeof(ehdr.e_phoff));
  ehdr.e_shoff = byte_get_little_endian(a_ehdr.a_e_shoff, sizeof(ehdr.e_shoff));
  ehdr.e_flags = byte_get_little_endian(a_ehdr.a_e_flags, sizeof(ehdr.e_flags));
  ehdr.e_ehsize =
      byte_get_little_endian(a_ehdr.a_e_ehsize, sizeof(ehdr.e_ehsize));
  ehdr.e_phentsize =
      byte_get_little_endian(a_ehdr.a_e_phentsize, sizeof(ehdr.e_phentsize));
  ehdr.e_phnum = byte_get_little_endian(a_ehdr.a_e_phnum, sizeof(ehdr.e_phnum));
  ehdr.e_shentsize =
      byte_get_little_endian(a_ehdr.a_e_shentsize, sizeof(ehdr.e_shentsize));
  ehdr.e_shnum = byte_get_little_endian(a_ehdr.a_e_shnum, sizeof(ehdr.e_shnum));
  ehdr.e_shstrndx =
      byte_get_little_endian(a_ehdr.a_e_shstrndx, sizeof(ehdr.e_shstrndx));

  ehdrIdent(&ehdr, &i_ehdr);

  ehdrType(&ehdr, &i_ehdr);
  ehdrMachine(&ehdr, &i_ehdr);
  ehdrObjectVersion(&ehdr, &i_ehdr);
  ehdrEntry(&ehdr, &i_ehdr);
  ehdrPhoff(&ehdr, &i_ehdr);
  ehdrShoff(&ehdr, &i_ehdr);
  ehdrFlag(&ehdr, &i_ehdr);
  ehdrEhsize(&ehdr, &i_ehdr);
  ehdrPhentsize(&ehdr, &i_ehdr);
  ehdrPhnum(&ehdr, &i_ehdr);
  ehdrShentsize(&ehdr, &i_ehdr);
  ehdrShnum(&ehdr, &i_ehdr);
  ehdrShstrndx(&ehdr, &i_ehdr);

  printfElf64Header(&i_ehdr);

  return 0;
}
