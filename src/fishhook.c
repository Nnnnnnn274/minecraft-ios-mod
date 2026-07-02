#include "fishhook.h"
#include <mach-o/dyld.h>
#include <mach-o/loader.h>
#include <mach-o/nlist.h>
#include <mach/mach.h>
#include <mach/vm_map.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/mman.h>

struct rebindings_entry {
    struct rebinding *rebindings;
    size_t rebindings_nel;
    struct rebindings_entry *next;
};
static struct rebindings_entry *_rebindings_head;
static int _dyld_func_lookup(const char *name, void **address) {
    for (uint32_t i = 0; i < _dyld_image_count(); i++) {
        const struct mach_header *header = _dyld_get_image_header(i);
        intptr_t slide = _dyld_get_image_vmaddr_slide(i);
        const struct load_command *cmds = (void *)header + sizeof(struct mach_header_64);
        for (uint32_t j = 0; j < header->ncmds; j++) {
            if (cmds->cmd == LC_SYMTAB) {
                const struct symtab_command *symtab = (void *)cmds;
                const char *strtab = (void *)header + symtab->stroff;
                const struct nlist_64 *sym = (void *)header + symtab->symoff;
                for (uint32_t k = 0; k < symtab->nsyms; k++) {
                    if (strcmp(strtab + sym[k].n_un.n_strx, name) == 0) {
                        *address = (void *)((uintptr_t)header + slide + sym[k].n_value);
                        return 0;
                    }
                }
            }
            cmds = (void *)cmds + cmds->cmdsize;
        }
    }
    return -1;
}
static void perform_rebinding_with_section(struct rebindings_entry *rebindings, uintptr_t section, uint32_t section_size, uintptr_t slide, const struct nlist_64 *symtab, const char *strtab) {
    const uint32_t *indirect = (void *)section;
    for (uint32_t i = 0; i < section_size / sizeof(uint32_t); i++) {
        uint32_t symoff = indirect[i];
        if (symoff == INDIRECT_SYMBOL_ABS || symoff == INDIRECT_SYMBOL_LOCAL) continue;
        const char *symname = strtab + symtab[symoff].n_un.n_strx;
        struct rebindings_entry *cur = rebindings;
        while (cur) {
            for (size_t j = 0; j < cur->rebindings_nel; j++) {
                if (strcmp(symname, cur->rebindings[j].name) == 0) {
                    uintptr_t *ptr = (uintptr_t *)(slide + (uintptr_t)(void *)&indirect[i]);
                    vm_protect(mach_task_self(), (vm_address_t)ptr, sizeof(void *), 0, VM_PROT_READ | VM_PROT_WRITE);
                    *cur->rebindings[j].replaced = (void *)*ptr;
                    *ptr = (uintptr_t)cur->rebindings[j].replacement;
                    vm_protect(mach_task_self(), (vm_address_t)ptr, sizeof(void *), 0, VM_PROT_READ);
                    break;
                }
            }
            cur = cur->next;
        }
    }
}
static void rebind_symbols_for_image(const struct mach_header *header, intptr_t slide) {
    const struct load_command *cmds = (void *)header + sizeof(struct mach_header_64);
    for (uint32_t i = 0; i < header->ncmds; i++) {
        if (cmds->cmd == LC_SEGMENT_64) {
            const struct segment_command_64 *seg = (void *)cmds;
            const struct section_64 *sect = (void *)seg + sizeof(struct segment_command_64);
            for (uint32_t j = 0; j < seg->nsects; j++) {
                if ((sect[j].flags & SECTION_TYPE) == S_LAZY_SYMBOL_POINTERS) {
                    const struct symtab_command *symtab = NULL;
                    const struct dysymtab_command *dysymtab = NULL;
                    const struct load_command *c2 = (void *)header + sizeof(struct mach_header_64);
                    for (uint32_t k = 0; k < header->ncmds; k++) {
                        if (c2->cmd == LC_SYMTAB) symtab = (void *)c2;
                        if (c2->cmd == LC_DYSYMTAB) dysymtab = (void *)c2;
                        c2 = (void *)c2 + c2->cmdsize;
                    }
                    if (symtab && dysymtab) {
                        const struct nlist_64 *sym = (void *)header + symtab->symoff;
                        const char *str = (void *)header + symtab->stroff;
                        perform_rebinding_with_section(_rebindings_head, sect[j].addr, sect[j].size, slide, sym, str);
                    }
                }
                if ((sect[j].flags & SECTION_TYPE) == S_NON_LAZY_SYMBOL_POINTERS) {
                    const struct symtab_command *symtab = NULL;
                    const struct dysymtab_command *dysymtab = NULL;
                    const struct load_command *c2 = (void *)header + sizeof(struct mach_header_64);
                    for (uint32_t k = 0; k < header->ncmds; k++) {
                        if (c2->cmd == LC_SYMTAB) symtab = (void *)c2;
                        if (c2->cmd == LC_DYSYMTAB) dysymtab = (void *)c2;
                        c2 = (void *)c2 + c2->cmdsize;
                    }
                    if (symtab && dysymtab) {
                        const struct nlist_64 *sym = (void *)header + symtab->symoff;
                        const char *str = (void *)header + symtab->stroff;
                        perform_rebinding_with_section(_rebindings_head, sect[j].addr, sect[j].size, slide, sym, str);
                    }
                }
            }
        }
        cmds = (void *)cmds + cmds->cmdsize;
    }
}
static void _rebind_symbols_for_image(const struct mach_header *header, intptr_t slide) {
    rebind_symbols_for_image(header, slide);
}
int rebind_symbols(struct rebinding rebindings[], size_t rebindings_nel) {
    struct rebindings_entry *entry = malloc(sizeof(struct rebindings_entry));
    entry->rebindings = malloc(sizeof(struct rebinding) * rebindings_nel);
    memcpy(entry->rebindings, rebindings, sizeof(struct rebinding) * rebindings_nel);
    entry->rebindings_nel = rebindings_nel;
    entry->next = _rebindings_head;
    _rebindings_head = entry;
    _dyld_register_func_for_add_image(_rebind_symbols_for_image);
    return 0;
}
