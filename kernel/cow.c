// Code to manage the Copy-on-Write mechanism
// Everything works on PTEs

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

struct spinlock lock;  // Better lock this to prevent concurrent edit of rcs
uint32 cow_rc[512] = {0};  // Cow Reference Counting for every page

pte_t cow_spawn(pte_t * pte) {
    uint32 page_n = PGROUNDDOWN(PTE2PA(*pte))/PGSIZE;
    printf("cow spawn page %d pa %p\n", page_n, PTE2PA(*pte));
    if (!(*pte & PTE_W) && !(*pte & PTE_PW)) {
        // Original PTE is already CoW: just return it as it is
        // Original PTE is non writable: just return it as it is
    }
    if (*pte & PTE_W) {
        // Original PTE is Write: Create new PTE that reference same PA and flag all of them as CoW
        *pte = (*pte | PTE_PW) & ~PTE_W;
    }
    acquire(&lock);
    cow_rc[page_n]++;
    release(&lock);
    return *pte;
}

pte_t cow_upgrade(pte_t * pte) {
    // Transform OPTE in a writable page IF NEEDED
    uint flags;
    char* mem;
    uint32 page_n = PGROUNDDOWN(PTE2PA(*pte))/PGSIZE;
    printf("cow upgrade page %d pa %p\n", page_n, PTE2PA(*pte));
    if (*pte & PTE_W) {
        // printf("cow_upgrade: PTE %p is already writable\n", *pte);
        return *pte;
    } else if (!(*pte & PTE_PW)) {
        printf("cow_upgrade: PTE %p is not writable or CoW\n", *pte);
        return 0;
    }
    if ((mem = kalloc()) == 0) {
        printf("cow_upgrade: unexpected kalloc failure\n");
        return 0;
    }
    flags = (PTE_FLAGS(*pte) | PTE_W) & ~PTE_PW;
    acquire(&lock);
    if(cow_rc[page_n] == 1) {
        // Only one person (the caller) is referencing this page, you can upgrade in-place
        cow_rc[page_n] = 0;
        release(&lock);
        return PA2PTE(PTE2PA(*pte)) | flags;
    } else {
        // More than one person are referencing this page, duplicate the page
        memmove(mem, (char*)PTE2PA(*pte), PGSIZE);
        cow_rc[page_n]--;
        release(&lock);
        flags = (PTE_FLAGS(*pte) | PTE_W) & ~PTE_PW;
        return PA2PTE(mem) | flags;
    }
}

int cow_refcount(void *pa) {
    // pa is already the address of the start of the page
    // for a successfull free if this func returns 1 you need to upgrade and then free the page to reset the counter
    uint32 page_n = (uint64)pa/PGSIZE;
    return cow_rc[page_n];
}