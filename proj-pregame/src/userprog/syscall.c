#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "userprog/process.h"
#include "threads/vaddr.h"
#include "userprog/pagedir.h"
#include <stdint.h>
#include <string.h>
#include "lib/kernel/stdio.h" /* putbuf */

static void syscall_handler(struct intr_frame* f);
static bool check_user_buffer(const void *uaddr, size_t size);
static bool get_user_uint32(const void *uaddr, uint32_t *dst, struct intr_frame *f);

void syscall_init(void) { intr_register_int(0x30, 3, INTR_ON, syscall_handler, "syscall"); }

/* Check that each byte in [uaddr, uaddr + size) is a valid user address
     mapped in the current process page directory. */
static bool
check_user_buffer(const void *uaddr, size_t size)
{
    const uint8_t *up = (const uint8_t *) uaddr;
    if (!is_user_vaddr(uaddr))
        return false;

    struct thread *cur = thread_current();
    if (cur->pcb == NULL)
        return false;
    uint32_t *pagedir = cur->pcb->pagedir;
    if (pagedir == NULL)
        return false;

    for (size_t i = 0; i < size; i++) {
        void *page = pagedir_get_page(pagedir, (const void *)(up + i));
        if (page == NULL)
            return false;
    }
    return true;
}

/* Safely read a uint32_t from user address uaddr into *dst. On failure,
     terminate the user process. */
static bool
get_user_uint32(const void *uaddr, uint32_t *dst, struct intr_frame *f UNUSED)
{
    if (!check_user_buffer(uaddr, sizeof(uint32_t))) {
        printf("%s: exit(%d)\n", thread_current()->name, -1);
        process_exit();
        return false;
    }
    memcpy(dst, uaddr, sizeof(uint32_t));
    return true;
}

static void
syscall_handler(struct intr_frame* f)
{
    uint32_t num;
    if (!get_user_uint32(f->esp, &num, f))
        return;

    if (num == SYS_EXIT) {
        uint32_t status;
        if (!get_user_uint32(f->esp + 4, &status, f))
            return;
        printf("%s: exit(%d)\n", thread_current()->pcb ? thread_current()->pcb->process_name : thread_current()->name, (int)status);
        process_exit();
        return;
    }

    if (num == SYS_PRACTICE) {
        uint32_t arg;
        if (!get_user_uint32(f->esp + 4, &arg, f))
            return;
        f->eax = arg + 1;
        return;
    }

    if (num == SYS_WRITE) {
        uint32_t fd, buffer_uaddr, size;
        if (!get_user_uint32(f->esp + 4, &fd, f)) return;
        if (!get_user_uint32(f->esp + 8, &buffer_uaddr, f)) return;
        if (!get_user_uint32(f->esp + 12, &size, f)) return;

        /* Only support writes to stdout (fd == 1) here: copy user buffer and
             send to console. */
        if (fd == 1) {
            if (size > 0) {
                if (!check_user_buffer((const void *)buffer_uaddr, size)) {
                    printf("%s: exit(%d)\n", thread_current()->name, -1);
                    process_exit();
                    return;
                }
                /* buffer_uaddr is in user space; putbuf expects a kernel pointer.
                     Since we verified the bytes are mapped, we can safely use the
                     user pointer directly for putbuf. */
                putbuf((const char *)buffer_uaddr, (size_t)size);
            }
            f->eax = size;
            return;
        }

        /* Other fds not implemented yet. Return -1. */
        f->eax = (uint32_t)-1;
        return;
    }
    
}
