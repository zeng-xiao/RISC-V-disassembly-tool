#ifndef ELF_H
#define ELF_H

/* Standard ELF types.  */

#include <stdint.h>

/* Type for a 16-bit quantity.  */
typedef uint16_t Elf32_Half;
typedef uint16_t Elf64_Half;

/* Types for signed and unsigned 32-bit quantities.  */
typedef uint32_t Elf32_Word;
typedef int32_t Elf32_Sword;
typedef uint32_t Elf64_Word;
typedef int32_t Elf64_Sword;

/* Types for signed and unsigned 64-bit quantities.  */
typedef uint64_t Elf32_Xword;
typedef int64_t Elf32_Sxword;
typedef uint64_t Elf64_Xword;
typedef int64_t Elf64_Sxword;

/* Type of addresses.  */
typedef uint32_t Elf32_Addr;
typedef uint64_t Elf64_Addr;

/* Type of file offsets.  */
typedef uint32_t Elf32_Off;
typedef uint64_t Elf64_Off;

/* Type for section indices, which are 16-bit quantities.  */
typedef uint16_t Elf32_Section;
typedef uint16_t Elf64_Section;

/* Type for version symbol information.  */
typedef Elf32_Half Elf32_Versym;
typedef Elf64_Half Elf64_Versym;

/* Fields in e_ident[].  */

#define EI_MAG0 0    /* File identification byte 0 index */
#define ELFMAG0 0x7F /* Magic number byte 0 */

#define EI_MAG1 1   /* File identification byte 1 index */
#define ELFMAG1 'E' /* Magic number byte 1 */

#define EI_MAG2 2   /* File identification byte 2 index */
#define ELFMAG2 'L' /* Magic number byte 2 */

#define EI_MAG3 3   /* File identification byte 3 index */
#define ELFMAG3 'F' /* Magic number byte 3 */

#define EI_CLASS 4     /* File class */
#define ELFCLASSNONE 0 /* Invalid class */
#define ELFCLASS32 1   /* 32-bit objects */
#define ELFCLASS64 2   /* 64-bit objects */

#define EI_DATA 5     /* Data encoding */
#define ELFDATANONE 0 /* Invalid data encoding */
#define ELFDATA2LSB 1 /* 2's complement, little endian */
#define ELFDATA2MSB 2 /* 2's complement, big endian */

#define EI_VERSION 6 /* File version */

#define EI_OSABI 7           /* Operating System/ABI indication */
#define ELFOSABI_NONE 0      /* UNIX System V ABI */
#define ELFOSABI_HPUX 1      /* HP-UX operating system */
#define ELFOSABI_NETBSD 2    /* NetBSD */
#define ELFOSABI_GNU 3       /* GNU */
#define ELFOSABI_LINUX 3     /* Alias for ELFOSABI_GNU */
#define ELFOSABI_SOLARIS 6   /* Solaris */
#define ELFOSABI_AIX 7       /* AIX */
#define ELFOSABI_IRIX 8      /* IRIX */
#define ELFOSABI_FREEBSD 9   /* FreeBSD */
#define ELFOSABI_TRU64 10    /* TRU64 UNIX */
#define ELFOSABI_MODESTO 11  /* Novell Modesto */
#define ELFOSABI_OPENBSD 12  /* OpenBSD */
#define ELFOSABI_OPENVMS 13  /* OpenVMS */
#define ELFOSABI_NSK 14      /* Hewlett-Packard Non-Stop Kernel */
#define ELFOSABI_AROS 15     /* AROS */
#define ELFOSABI_FENIXOS 16  /* FenixOS */
#define ELFOSABI_CLOUDABI 17 /* Nuxi CloudABI */
#define ELFOSABI_OPENVOS 18  /* Stratus Technologies OpenVOS */

#define ELFOSABI_C6000_ELFABI 64 /* Bare-metal TMS320C6000 */
#define ELFOSABI_C6000_LINUX 65  /* Linux TMS320C6000 */
#define ELFOSABI_ARM_FDPIC 65    /* ARM FDPIC */
#define ELFOSABI_ARM 97          /* ARM */
#define ELFOSABI_STANDALONE 255  /* Standalone (embedded) application */

#define EI_ABIVERSION 8 /* ABI version */

#define EI_PAD 9 /* Start of padding bytes */

/* Values for e_type, which identifies the object file type.  */

#define ET_NONE 0 /* No file type */
#define ET_REL 1  /* Relocatable file */
#define ET_EXEC 2 /* Position-dependent executable file */
#define ET_DYN 3  /* Position-independent executable or shared object file */
#define ET_CORE 4 /* Core file */
#define ET_LOOS 0xFE00   /* Operating system-specific */
#define ET_HIOS 0xFEFF   /* Operating system-specific */
#define ET_LOPROC 0xFF00 /* Processor-specific */
#define ET_HIPROC 0xFFFF /* Processor-specific */

/* Values for e_machine, which identifies the architecture.  These numbers
   are officially assigned by registry@sco.com.  See below for a list of
   ad-hoc numbers used during initial development.  */
#define EM_RISCV 243 /* RISC-V */

/* File may contain compressed instructions.  */
#define EF_RISCV_RVC 0x0001

/* File uses the 32E base integer instruction.  */
#define EF_RISCV_RVE 0x0008

/* Which floating-point ABI a file uses.  */
#define EF_RISCV_FLOAT_ABI 0x0006

/* File uses the soft-float ABI.  */
#define EF_RISCV_FLOAT_ABI_SOFT 0x0000

/* File uses the single-float ABI.  */
#define EF_RISCV_FLOAT_ABI_SINGLE 0x0002

/* File uses the double-float ABI.  */
#define EF_RISCV_FLOAT_ABI_DOUBLE 0x0004

/* File uses the quad-float ABI.  */
#define EF_RISCV_FLOAT_ABI_QUAD 0x0006

/* The ELF file header.  This appears at the start of every ELF file.  */

#define EI_NIDENT (16)

typedef struct {
  unsigned char e_ident[EI_NIDENT]; /* Magic number and other info */
  Elf32_Half e_type;                /* Object file type */
  Elf32_Half e_machine;             /* Architecture */
  Elf32_Word e_version;             /* Object file version */
  Elf32_Addr e_entry;               /* Entry point virtual address */
  Elf32_Off e_phoff;                /* Program header table file offset */
  Elf32_Off e_shoff;                /* Section header table file offset */
  Elf32_Word e_flags;               /* Processor-specific flags */
  Elf32_Half e_ehsize;              /* ELF header size in bytes */
  Elf32_Half e_phentsize;           /* Program header table entry size */
  Elf32_Half e_phnum;               /* Program header table entry count */
  Elf32_Half e_shentsize;           /* Section header table entry size */
  Elf32_Half e_shnum;               /* Section header table entry count */
  Elf32_Half e_shstrndx;            /* Section header string table index */
} Elf32_Ehdr;

typedef struct {
  unsigned char e_ident[EI_NIDENT]; /* Magic number and other info */
  Elf64_Half e_type;                /* Object file type */
  Elf64_Half e_machine;             /* Architecture */
  Elf64_Word e_version;             /* Object file version */
  Elf64_Addr e_entry;               /* Entry point virtual address */
  Elf64_Off e_phoff;                /* Program header table file offset */
  Elf64_Off e_shoff;                /* Section header table file offset */
  Elf64_Word e_flags;               /* Processor-specific flags */
  Elf64_Half e_ehsize;              /* ELF header size in bytes */
  Elf64_Half e_phentsize;           /* Program header table entry size */
  Elf64_Half e_phnum;               /* Program header table entry count */
  Elf64_Half e_shentsize;           /* Section header table entry size */
  Elf64_Half e_shnum;               /* Section header table entry count */
  Elf64_Half e_shstrndx;            /* Section header string table index */
} Elf64_Ehdr;

/* Section header.  */

typedef struct {
  Elf32_Word sh_name;      /* Section name (string tbl index) */
  Elf32_Word sh_type;      /* Section type */
  Elf32_Word sh_flags;     /* Section flags */
  Elf32_Addr sh_addr;      /* Section virtual addr at execution */
  Elf32_Off sh_offset;     /* Section file offset */
  Elf32_Word sh_size;      /* Section size in bytes */
  Elf32_Word sh_link;      /* Link to another section */
  Elf32_Word sh_info;      /* Additional section information */
  Elf32_Word sh_addralign; /* Section alignment */
  Elf32_Word sh_entsize;   /* Entry size if section holds table */
} Elf32_Shdr;

typedef struct {
  Elf64_Word sh_name;       /* Section name (string tbl index) */
  Elf64_Word sh_type;       /* Section type */
  Elf64_Xword sh_flags;     /* Section flags */
  Elf64_Addr sh_addr;       /* Section virtual addr at execution */
  Elf64_Off sh_offset;      /* Section file offset */
  Elf64_Xword sh_size;      /* Section size in bytes */
  Elf64_Word sh_link;       /* Link to another section */
  Elf64_Word sh_info;       /* Additional section information */
  Elf64_Xword sh_addralign; /* Section alignment */
  Elf64_Xword sh_entsize;   /* Entry size if section holds table */
} Elf64_Shdr;

/* Values for section header, sh_type field.  */

#define SHT_NULL 0     /* Section header table entry unused */
#define SHT_PROGBITS 1 /* Program specific (private) data */
#define SHT_SYMTAB 2   /* Link editing symbol table */
#define SHT_STRTAB 3   /* A string table */
#define SHT_RELA 4     /* Relocation entries with addends */
#define SHT_HASH 5     /* A symbol hash table */
#define SHT_DYNAMIC 6  /* Information for dynamic linking */
#define SHT_NOTE 7     /* Information that marks file */
#define SHT_NOBITS 8   /* Section occupies no space in file */
#define SHT_REL 9      /* Relocation entries, no addends */
#define SHT_SHLIB 10   /* Reserved, unspecified semantics */
#define SHT_DYNSYM 11  /* Dynamic linking symbol table */

#define SHT_INIT_ARRAY 14    /* Array of ptrs to init functions */
#define SHT_FINI_ARRAY 15    /* Array of ptrs to finish functions */
#define SHT_PREINIT_ARRAY 16 /* Array of ptrs to pre-init funcs */
#define SHT_GROUP 17         /* Section contains a section group */
#define SHT_SYMTAB_SHNDX 18  /* Indices for SHN_XINDEX entries */
#define SHT_RELR 19          /* RELR relative relocations */

#define SHT_LOOS 0x60000000 /* First of OS specific semantics */
#define SHT_HIOS 0x6fffffff /* Last of OS specific semantics */

#define SHT_GNU_INCREMENTAL_INPUTS 0x6fff4700 /* incremental build data */
#define SHT_GNU_ATTRIBUTES 0x6ffffff5         /* Object attributes */
#define SHT_GNU_HASH 0x6ffffff6               /* GNU style symbol hash table */
#define SHT_GNU_LIBLIST 0x6ffffff7            /* List of prelink dependencies */

/* Additional section types.  */
#define SHT_RISCV_ATTRIBUTES 0x70000003 /* Section holds attributes.  */

/* Values for section header, sh_flags field.  */

#define SHF_WRITE (1 << 0)      /* Writable data during execution */
#define SHF_ALLOC (1 << 1)      /* Occupies memory during execution */
#define SHF_EXECINSTR (1 << 2)  /* Executable machine instructions */
#define SHF_MERGE (1 << 4)      /* Data in this section can be merged */
#define SHF_STRINGS (1 << 5)    /* Contains null terminated character strings */
#define SHF_INFO_LINK (1 << 6)  /* sh_info holds section header table index */
#define SHF_LINK_ORDER (1 << 7) /* Preserve section ordering when linking */
#define SHF_OS_NONCONFORMING (1 << 8) /* OS specific processing required */
#define SHF_GROUP (1 << 9)            /* Member of a section group */
#define SHF_TLS (1 << 10)             /* Thread local storage section */
#define SHF_COMPRESSED (1 << 11)      /* Section with compressed data */

/* #define SHF_MASKOS	0x0F000000    */ /* OS-specific semantics */
#define SHF_MASKOS 0x0FF00000            /* New value, Oct 4, 1999 Draft */
#define SHF_GNU_RETAIN                                                         \
  (1 << 21) /* Section should not be garbage collected by the linker.  */
#define SHF_MASKPROC 0xF0000000 /* Processor-specific semantics */

/* This used to be implemented as a processor specific section flag.
   We just make it generic.  */
#define SHF_EXCLUDE                                                            \
  0x80000000 /* Link editor is to exclude                                      \
                this section from executable                                   \
                and shared library that it                                     \
                builds when those objects                                      \
                are not to be further                                          \
                relocated.  */

#define SHF_GNU_MBIND 0x01000000 /* Mbind section.  */

#define ENOENT 2 /* No such file or directory */

#endif
