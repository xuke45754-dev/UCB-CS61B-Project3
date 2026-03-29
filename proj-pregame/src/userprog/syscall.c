#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "userprog/process.h"
#include "threads/vaddr.h"    
#include "userprog/pagedir.h" 
#include <stdint.h> 

static void syscall_handler(struct intr_frame*);
static uint32_t get_user_u32(const void *uaddr);
static const uint8_t* get_user_ptr(const void *uaddr);

void syscall_init(void) { intr_register_int(0x30, 3, INTR_ON, syscall_handler, "syscall"); }

static void syscall_handler(struct intr_frame* f UNUSED) {
  uint32_t* args = ((uint32_t*)f->esp);

  /*
   * The following print statement, if uncommented, will print out the syscall
   * number whenever a process enters a system call. You might find it useful
   * when debugging. It will cause tests to fail, however, so you should not
   * include it in your final submission.
   */

  /* printf("System call number: %d\n", args[0]); */
    uint32_t sys_num = get_user_u32(f->esp);

    if (sys_num == SYS_EXIT) {
        uint32_t status = get_user_u32(f->esp + 4);
        printf("%s: exit(%d)\n", thread_current()->pcb->process_name, status);
        process_exit();
    }

    if (sys_num == SYS_PRACTICE) {
        uint32_t arg = get_user_u32(f->esp + 4);
        f->eax = arg + 1;
        return;
    }
}

static uint32_t get_user_u32(const void *uaddr) {
    if (!is_user_vaddr(uaddr)) process_exit();
    uint32_t *kaddr = pagedir_get_page(thread_current()->pcb->pagedir, uaddr);
    if (!kaddr) process_exit();
    return *kaddr;
}

static const uint8_t* get_user_ptr(const void *uaddr) {
    if (!is_user_vaddr(uaddr)) process_exit();
    return uaddr;
}
