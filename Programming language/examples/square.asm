fpmon
push ebp
mov ebp, esp
push 0xADD
call func_5AF9AADE
pop ebp
hlt
func_4D7A9E9D:
pop [ebp]
add ebp, 4
add esp, 4
push 0.010000

push ebp
add ebp, 0
mov esi, ebp
pop ebp
push [esi]

pop eax
abs eax
push eax
pop eax
pop ebx
cmp eax, ebx
jb T_jb_0
push 0.0
jmp end_jb_0
T_jb_0:
push 1.0
end_jb_0:
pop eax
cmp eax, 0
jne if_0
je else_0
if_0:

push 1.000000
pop eax
mov esp, ebp
ret

jmp endif_0
else_0:
endif_0:

push 0.000000
pop eax
mov esp, ebp
ret

mov esp, ebp
ret
func_5AF9AADE:
pop [ebp]
add ebp, 4
add esp, 12
in eax
push eax

push ebp
add ebp, 0
mov esi, ebp
pop ebp
pop [esi]

in eax
push eax

push ebp
add ebp, 4
mov esi, ebp
pop ebp
pop [esi]

in eax
push eax

push ebp
add ebp, 8
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


push ebp
add ebp, 4
mov esi, ebp
pop ebp
push [esi]


push ebp
add ebp, 8
mov esi, ebp
pop ebp
push [esi]

mov ebp, edi
call func_C7198C63
pop ebp
push eax

push 0.000000
pop eax
mov esp, ebp
ret

mov esp, ebp
ret
func_962EE3F1:
pop [ebp]
add ebp, 4
add esp, 8


push ebp
add ebp, 0
mov esi, ebp
pop ebp
push [esi]


push ebp
add ebp, 4
mov esi, ebp
pop ebp
push [esi]

pop eax
mul eax, -1.0
push eax
pop eax
pop ebx
div eax, ebx
push eax
pop eax
mov esp, ebp
ret

mov esp, ebp
ret
func_C7198C63:
pop [ebp]
add ebp, 4
add esp, 20
push ebp
mov edi, esp
push 0xADD

push ebp
add ebp, 8
mov esi, ebp
pop ebp
push [esi]

mov ebp, edi
call func_4D7A9E9D
pop ebp
push eax
push ebp
mov edi, esp
push 0xADD

push ebp
add ebp, 4
mov esi, ebp
pop ebp
push [esi]

mov ebp, edi
call func_4D7A9E9D
pop ebp
push eax
pop eax
pop ebx
and eax, ebx
push eax
push ebp
mov edi, esp
push 0xADD

push ebp
add ebp, 0
mov esi, ebp
pop ebp
push [esi]

mov ebp, edi
call func_4D7A9E9D
pop ebp
push eax
pop eax
pop ebx
and eax, ebx
push eax
pop eax
cmp eax, 0
jne if_1
je else_1
if_1:

push 0.000000
pop eax
mov esp, ebp
ret

jmp endif_1
else_1:
endif_1:
push ebp
mov edi, esp
push 0xADD

push ebp
add ebp, 0
mov esi, ebp
pop ebp
push [esi]

mov ebp, edi
call func_4D7A9E9D
pop ebp
push eax
pop eax
cmp eax, 0
jne if_2
je else_2
if_2:
push ebp
mov edi, esp
push 0xADD

push ebp
add ebp, 4
mov esi, ebp
pop ebp
push [esi]


push ebp
add ebp, 8
mov esi, ebp
pop ebp
push [esi]

mov ebp, edi
call func_962EE3F1
pop ebp
push eax
pop eax
out eax

push 0.000000
pop eax
mov esp, ebp
ret

jmp endif_2
else_2:
endif_2:
push ebp
mov edi, esp
push 0xADD

push ebp
add ebp, 8
mov esi, ebp
pop ebp
push [esi]

mov ebp, edi
call func_4D7A9E9D
pop ebp
push eax
pop eax
cmp eax, 0
jne if_3
je else_3
if_3:
push ebp
mov edi, esp
push 0xADD

push ebp
add ebp, 0
mov esi, ebp
pop ebp
push [esi]


push ebp
add ebp, 4
mov esi, ebp
pop ebp
push [esi]

mov ebp, edi
call func_962EE3F1
pop ebp
push eax
pop eax
out eax
push 0.000000
pop eax
out eax

push 0.000000
pop eax
mov esp, ebp
ret

jmp endif_3
else_3:
endif_3:

push ebp
add ebp, 8
mov esi, ebp
pop ebp
push [esi]


push ebp
add ebp, 0
mov esi, ebp
pop ebp
push [esi]

pop eax
pop ebx
mul eax, ebx
push eax
push 4.000000
pop eax
pop ebx
mul eax, ebx
push eax
push 2.000000

push ebp
add ebp, 4
mov esi, ebp
pop ebp
push [esi]

pop eax
pop ebx
pow eax, ebx
push eax
pop eax
pop ebx
sub eax, ebx
push eax

push ebp
add ebp, 12
mov esi, ebp
pop ebp
pop [esi]


push ebp
add ebp, 0
mov esi, ebp
pop ebp
push [esi]

push 2.000000
pop eax
pop ebx
mul eax, ebx
push eax

push ebp
add ebp, 4
mov esi, ebp
pop ebp
push [esi]

pop eax
mul eax, -1.0
push eax
pop eax
pop ebx
div eax, ebx
push eax

push ebp
add ebp, 16
mov esi, ebp
pop ebp
pop [esi]

push 0.000000

push ebp
add ebp, 12
mov esi, ebp
pop ebp
push [esi]

pop eax
pop ebx
cmp eax, ebx
ja T_ja_1
push 0.0
jmp end_ja_1
T_ja_1:
push 1.0
end_ja_1:
pop eax
cmp eax, 0
jne if_4
je else_4
if_4:

push ebp
add ebp, 12
mov esi, ebp
pop ebp
push [esi]

pop eax
sqrt eax
push eax

push ebp
add ebp, 12
mov esi, ebp
pop ebp
pop [esi]


push ebp
add ebp, 0
mov esi, ebp
pop ebp
push [esi]

push 2.000000
pop eax
pop ebx
mul eax, ebx
push eax

push ebp
add ebp, 12
mov esi, ebp
pop ebp
push [esi]

pop eax
pop ebx
div eax, ebx
push eax

push ebp
add ebp, 12
mov esi, ebp
pop ebp
pop [esi]


push ebp
add ebp, 12
mov esi, ebp
pop ebp
push [esi]


push ebp
add ebp, 16
mov esi, ebp
pop ebp
push [esi]

pop eax
pop ebx
add eax, ebx
push eax
pop eax
out eax

push ebp
add ebp, 12
mov esi, ebp
pop ebp
push [esi]


push ebp
add ebp, 16
mov esi, ebp
pop ebp
push [esi]

pop eax
pop ebx
sub eax, ebx
push eax
pop eax
out eax

push 0.000000
pop eax
mov esp, ebp
ret

jmp endif_4
else_4:
endif_4:
push ebp
mov edi, esp
push 0xADD

push ebp
add ebp, 12
mov esi, ebp
pop ebp
push [esi]

mov ebp, edi
call func_4D7A9E9D
pop ebp
push eax
pop eax
cmp eax, 0
jne if_5
je else_5
if_5:

push ebp
add ebp, 16
mov esi, ebp
pop ebp
push [esi]

pop eax
out eax

push 0.000000
pop eax
mov esp, ebp
ret

jmp endif_5
else_5:
endif_5:

push 0.000000
pop eax
mov esp, ebp
ret

mov esp, ebp
ret
