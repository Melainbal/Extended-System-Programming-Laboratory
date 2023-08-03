global infile
global outfile
section .data
    stdout: equ 1
    newline: db 10  ; define newline character
    extern strlen
    infile dd 0
    outfile dd 1
    buffer db 1
    null_byte db 0

section .text
    CLOSE EQU 6
    OPEN EQU 5
    READ EQU 3
    EXIT EQU 1 
    global _start
    
_start:
    call main
    call encoder

main:
    push ebp
    mov ebp, esp
    mov esi, [ebp+16] ; Get 2nd argument av
    mov edi, 1

    ; loop through all arguments
    start_loop:
        ; check if we've reached the end of arguments
        cmp edi, [ebp+8] 
        je end_loop

        ; check for flags
        mov edx, esi
        cmp byte[edx],'-'
        jne start_print
        inc edx 
        cmp byte[edx], 'i'
        je infile_change
        cmp byte[edx], 'o'
        je outfile_change

        infile_change:
            inc edx
            mov eax, OPEN
            mov ebx, edx
            mov ecx, 0
            mov edx, 0644
            int 0x80            ; call kernel
            mov [infile], eax
            jmp start_print
        
        outfile_change:
            inc edx
            mov eax, 0x5
            mov ebx, edx
            mov ecx, 0x41
            mov edx, 0777
            int 0x80            ; call kernel
            mov [outfile], eax
            jmp start_print

        start_print: ; write argument to stdout
            mov ebx, stdout     ; file descriptor
            push esi          ;
            call strlen          ;
            mov edx, eax          ;
            mov ecx, esi
            mov eax, 4             ;
            int 0x80            ; call kernel

            add esp, 4            ; restore stack pointer

            ;Print newline character
            mov ebx, stdout   ;
            mov ecx, newline  ;
            mov edx, 1         ;
            mov eax, 4        ;
            int 0x80            ;

            ; increment pointer to next argument
            mov esi, [ebp+ 16 + 4*edi]
            inc edi 

            ; jump back to start of loop
            jmp start_loop

    end_loop:
        pop ebp
        ret

encoder:
    encoder_loop:
        ; read input and encode
        mov eax, READ
        mov ebx, [infile]   ; input file descriptor
        mov ecx, buffer    ; input buffer
        mov edx, 1         ; read 1 byte
        int 0x80
        cmp eax, 0         ; check for end of file
        je done

        ; encode character
        cmp byte [buffer], 'A'
        jl write
        cmp byte [buffer], 'z'
        jg write
        add byte [buffer], 1

        ; write encoded character to output
        write:
            mov eax, 4
            mov ebx, [outfile]  ; output file descriptor
            mov ecx, buffer    ; output buffer
            mov edx, 1         ; write 1 byte
            int 0x80

            ; loop until end of input
            jmp encoder_loop

        done:
            ; exit program with status code 0

            ;close inflie
            mov eax, 6
            mov ebx, [infile]
            int 0x80

            ;close outfile
            mov eax, 6
            mov ebx, [outfile]
            int 0x80

            mov eax, 1
            xor ebx, ebx
            int 0x80
            