push ebp
mov edi, esp
push 0xADD
mov ebp, edi
call func_5AF9AADE
pop ebp
hlt
func_1943C662:
pop [ebp]
add ebp, 4
add esp, 0
push 1.000000
push ebp
add ebp, 0
mov esi, ebp
pop ebp
push [esi]
pop eax
pop ebx
fcomp eax, ebx
ja T_ja_0
push 0.0
jmp end_ja_0
T_ja_0:
push 1.0
end_ja_0:
pop eax
fcomp eax, 0.0
jne if_0
je else_0
if_0:
push ebp
mov edi, esp
push 0xADD
push 1.000000
push ebp
add ebp, 0
mov esi, ebp
pop ebp
push [esi]
pop eax
pop ebx
fsub eax, ebx
push eax
mov ebp, edi
call func_1943C662
pop ebp
push eax
push ebp
add ebp, 0
mov esi, ebp
pop ebp
push [esi]
pop eax
pop ebx
fmul eax, ebx
push eax
pop eax
mov esp, ebp
ret
jmp endif_0
else_0:
push 1.000000
pop eax
mov esp, ebp
ret
endif_0:
mov esp, ebp
ret
func_5AF9AADE:
pop [ebp]
add ebp, 4
add esp, 4
fin eax
push eax
push ebp
add ebp, 0
mov esi, ebp
pop ebp
pop [esi]
push ebp
mov edi, esp
push 0xADD
push ebp
add ebp, 0
mov esi, ebp
pop ebp
push [esi]
mov ebp, edi
call func_1943C662
pop ebp
push eax
push ebp
add ebp, 0
mov esi, ebp
pop ebp
pop [esi]
push ebp
add ebp, 0
mov esi, ebp
pop ebp
push [esi]
pop eax
out eax
mov esp, ebp
ret
