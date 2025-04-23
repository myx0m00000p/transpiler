%macro len 0
    push rdx
    push rsi

    xor rax, rax

%%loop_start:
    mov dl, [rsi]

    cmp dl, 0
    jz %%loop_end

    inc rsi
    inc rax

    jmp %%loop_start

%%loop_end:

    pop rsi
    pop rdx

%endmacro


%macro print 0
    push rax
    push rdi
    push rdx
    push rcx
    push rsi

    len

    pop rsi

    mov rdx, rax

    mov rdi, 1

    mov rax, 1

    syscall

    pop rcx
    pop rdx
    pop rdi
    pop rax
%endmacro


%macro clear 0
    push rax

    mov al, 21

%%loop_clear:
    mov byte [rsi], 0
    inc rsi
    dec al

    cmp al, 0
    jnz %%loop_clear

    pop rax
%endmacro


%macro toStr 0
    push rax
    push rdi
    push rdx
    push rbx
    push rcx

    mov rsi, buf
    clear

    mov rsi, buf2
    clear

    mov rbx, buf2

    cmp rax, 0
    jge %%sign_check_end

    neg rax

    mov byte [rbx], '-'
    inc rbx

%%sign_check_end:

    mov r8, 10

    cmp rax, 0
    jnz %%start_division_loop
        
    mov rsi, buf
    mov byte [rsi], '0'
        
    mov rdi, 1
    jmp %%end_division_loop

%%start_division_loop:
    mov rsi, buf

    xor rdi, rdi

%%division_loop:
    xor rdx, rdx
    div r8

    add dl, 48

    mov [rsi], dl

    inc rsi
    inc rdi

    cmp rax, 0
    jnz %%division_loop

%%end_division_loop:
    mov rcx, buf
    add rcx, rdi
    dec rcx

    mov rdx, rdi

%%copy_loop:
    mov al, [rcx]
    mov [rbx], al

    dec rcx
    inc rbx

    dec rdx
    jnz %%copy_loop

    mov byte [rbx], 0

    pop rcx
    pop rbx
    pop rdx
    pop rdi
    pop rax
    
%endmacro