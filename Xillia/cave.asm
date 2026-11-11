; nasm -f bin cave.asm -o cave.bin
bits 64
default rel
org 0x182a20ee0
; Per frame in CUnit.PreExecState (inlined UpdateCombo), replacing
; "mov rax,[rbx+0x40]; test rax,rax": seconds spent in hitstun during the current semi combo
; (CUnit.m_SemiDamageComboHit, the hit count that enemy scripts compare to the counter threshold).
combo_timer:
    cmp dword [rbx+0x3dc], 0        ; m_DamageComboHit: in hitstun now?
    je .idle
    cmp byte [rel $$ + (0x1837d09ae - 0x182a20ee0)], 0  ; BTL.CTask type info resolved?
    je .done
    mov rax, [rel $$ + (0x1835e6a18 - 0x182a20ee0)]     ; BTL.CTask TypeInfo
    mov rax, [rax+0xb8]             ; static_fields
    test rax, rax
    je .done
    sub rsp, 16
    movdqu [rsp], xmm0
    movss xmm0, [rax+0x2c]          ; s_FrameTime (1/FPS)
    addss xmm0, [rbx+0x2f4]         ; timer in unused CUnit padding
    movss [rbx+0x2f4], xmm0
    movdqu xmm0, [rsp]
    add rsp, 16
    jmp .done
.idle:
    cmp dword [rbx+0x3e4], 0        ; m_SemiDamageComboHit: semi combo over?
    jne .done
    mov dword [rbx+0x2f4], 0
.done:
    mov rax, [rbx+0x40]
    test rax, rax
    jmp 0x180a5e04d

; Script_GetParam 0x7007/0x7008 (UNIT_STATUS.emergencyDamageMin/Max), replacing
; "mov eax,[rax+0x1b0/0x1b4]": 0 (no counter) until the timer reaches THRESHOLD seconds, then 1, so
; the script counters on the next hit. (Returning the current hit count never triggers a counter.)
emergency_min:
    mov eax, [rax+0x1b0]
    jmp emergency
emergency_max:
    mov eax, [rax+0x1b4]
emergency:
    test eax, eax                   ; this enemy never counters
    je .back
    cmp dword [rbx+0x58], 0         ; m_Kind 0: party member
    je .back
    cmp dword [rbx+0x2f4], 0x41700000   ; THRESHOLD (float bits, 15.0); positive floats compare as ints
    mov eax, 0
    jb .back
    mov eax, 1
.back:
    jmp 0x180a292a5
