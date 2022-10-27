#ifndef ELF_HEADER_H
#define ELF_HEADER_H

#include <stdint.h>

int processElfHeader(const char *inputFileName);

uint64_t byte_get_little_endian(const unsigned char *field, int size);
void byte_put_little_endian(unsigned char *field, uint64_t value, int size);

// This is an auxiliary data structure created by converting bytes
// read from the outside into corresponding data sizes

/* ELF Header (32-bit implementations) */
typedef struct {
  unsigned char e_ident[16];    /* ELF "magic number" */
  unsigned char e_type[2];      /* Identifies object file type */
  unsigned char e_machine[2];   /* Specifies required architecture */
  unsigned char e_version[4];   /* Identifies object file version */
  unsigned char e_entry[4];     /* Entry point virtual address */
  unsigned char e_phoff[4];     /* Program header table file offset */
  unsigned char e_shoff[4];     /* Section header table file offset */
  unsigned char e_flags[4];     /* Processor-specific flags */
  unsigned char e_ehsize[2];    /* ELF header size in bytes */
  unsigned char e_phentsize[2]; /* Program header table entry size */
  unsigned char e_phnum[2];     /* Program header table entry count */
  unsigned char e_shentsize[2]; /* Section header table entry size */
  unsigned char e_shnum[2];     /* Section header table entry count */
  unsigned char e_shstrndx[2];  /* Section header string table index */
} Elf32_External_Ehdr;

/* ELF Header (64-bit implementations) */
typedef struct {
  unsigned char e_ident[16];    /* ELF "magic number" */
  unsigned char e_type[2];      /* Identifies object file type */
  unsigned char e_machine[2];   /* Specifies required architecture */
  unsigned char e_version[4];   /* Identifies object file version */
  unsigned char e_entry[8];     /* Entry point virtual address */
  unsigned char e_phoff[8];     /* Program header table file offset */
  unsigned char e_shoff[8];     /* Section header table file offset */
  unsigned char e_flags[4];     /* Processor-specific flags */
  unsigned char e_ehsize[2];    /* ELF header size in bytes */
  unsigned char e_phentsize[2]; /* Program header table entry size */
  unsigned char e_phnum[2];     /* Program header table entry count */
  unsigned char e_shentsize[2]; /* Section header table entry size */
  unsigned char e_shnum[2];     /* Section header table entry count */
  unsigned char e_shstrndx[2];  /* Section header string table index */
} Elf64_External_Ehdr;

#define EI_NIDENT (16)

typedef struct {
  unsigned char i_ident[EI_NIDENT]; /* Magic number and other info */

  const char *i_class;
  const char *i_dataEncodingForm;
  uint32_t i_fileVersion; /* File version */
  const char *i_osAbi;
  uint32_t i_abiVersion; /* ABI version */

  const char *i_type;        /* Object file type */
  const char *i_machine;     /* Architecture */
  uint32_t i_objectVersion;  /* Object file version */
  uint64_t i_entry;          /* Entry point virtual address */
  uint64_t i_phoff;          /* Program header table file offset */
  uint64_t i_shoff;          /* Section header table file offset */
  uint32_t i_flags;          /* Processor-specific flags */
  char i_flagStatement[100]; /* Architecture */
  uint16_t i_ehsize;         /* ELF header size in bytes */
  uint16_t i_phentsize;      /* Program header table entry size */
  uint16_t i_phnum;          /* Program header table entry count */
  uint16_t i_shentsize;      /* Section header table entry size */
  uint16_t i_shnum;          /* Section header table entry count */
  uint16_t i_shstrndx;       /* Section header string table index */
} Elf64_Info_Ehdr;

/* File uses the 32E base integer instruction.  */
//#define EF_RISCV_RVE 0x0008

#endif
