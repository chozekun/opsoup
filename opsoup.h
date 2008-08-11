#ifndef OPSOUP_H
#define OPSOUP_H 1

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "nasmlib.h"
#include "disasm.h"
#include "sync.h"

/* segment types */
typedef enum {
    seg_NONE = 0,
    seg_CODE,
    seg_DATA,
    seg_BSS,
    seg_RELOC
} segment_type_t;

/* segment data */
typedef struct segment_st {
    char            *name;
    segment_type_t   type;
    uint8_t         *start;
    uint8_t         *end;
    uint32_t         size;
    void            *info;
} segment_t;

typedef struct _image_st {
    int              fd;
    uint8_t         *core;
    uint32_t         size;
    segment_t       *segment;
} image_t;

typedef struct _reloc_st {
    uint8_t         *mem;
    uint8_t         *target;
} reloc_t;

typedef enum {
    label_NONE          = 0x0,

    label_VTABLE        = 0x1,      /* vtable mask */

    label_NAME          = 0x10,     /* named label only, no semantic value */
    
    label_RELOC         = 0x20,     /* relocation target */

    label_DATA          = 0x40,     /* data item */
    label_DATA_VTABLE   = 0x41,     /* jump/call vector table */

    label_BSS           = 0x80,     /* uninitialised data item */
    label_BSS_VTABLE    = 0x81,     /* uninitialised jump/call vector table */

    label_CODE          = 0x100,    /* code item */
    label_CODE_JUMP     = 0x102,    /* jump point ("jmp" instruction target) */
    label_CODE_CALL     = 0x104,    /* call point ("call" instruction target) */

    label_EXTERN        = 0x200     /* external (eg linked library) symbol */
} label_type_t;

typedef struct label_st {
    uint8_t         *target;         /* where this label is located */
    label_type_t    type;           /* its type */
    segment_t       *seg;           /* segment the label is in, for convenience */

    int             count;          /* use count */

    char            *name;          /* name from object file or generated */
} label_t;

#define MAX_REF_TARGET (4)

typedef struct ref_st {
    uint8_t        *mem;                    /* location of the reference */
    uint8_t        *target[MAX_REF_TARGET]; /* location of target labels */
    int             ntarget;                /* number of targets */
} ref_t;

typedef struct _opsoup_st {
    image_t          image;

    reloc_t         *reloc;
    int              nreloc;

    label_t         *label;
    int             nlabel, slabel;
    int             added, upgraded;

    ref_t           *ref;
    int              nref, sref;

    int              verbose;
} opsoup_t;

extern opsoup_t *o;


int         image_load(void);
segment_t   *image_seg_find(uint8_t *mem);

label_t         *label_find(uint8_t *target);
label_t         *label_insert(uint8_t *target, label_type_t type, segment_t *s);
void            label_remove(uint8_t *target);
void            label_reloc_upgrade(void);
int             label_print_upgraded(char *str);
void            label_gen_names(void);
void            label_sort(void);
void            label_print_count(void);
void            label_print_unused(void);
void            label_extern_output(FILE *f);

ref_t           *ref_insert(uint8_t *source, uint8_t *target);


/* disassembly */
void    dis_pass1(void);
int     dis_pass2(int n);
void    dis_pass3(FILE *f);


/* data "disassembly" */
void    data_output(FILE *f);
void    data_bss_output(FILE *f);


/* elf stuff */
int elf_make_segment_table(image_t *image);
void elf_load_labels(opsoup_t *o);
int elf_relocate(opsoup_t *o);

#endif
