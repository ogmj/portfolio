.code

break_point proc
int 3
ret
break_point endp

getRDTSC proc
rdtsc
ret
getRDTSC endp

interlocked_increment proc
lock xadd   dword ptr [rcx],edx 
mov eax, edx
ret
interlocked_increment endp

interlocked_decrement proc
neg edx
lock xadd   dword ptr [rcx],edx 
mov eax, edx
ret
interlocked_decrement endp

interlocked_exchange proc
lock xchg   dword ptr [rcx],edx 
mov eax, edx
ret
interlocked_exchange endp

interlocked_increment64 proc
lock xadd   qword ptr [rcx],rdx 
mov rax, rdx
ret
interlocked_increment64 endp

interlocked_decrement64 proc
neg rdx
lock xadd   qword ptr [rcx],rdx 
mov rax, rdx
ret
interlocked_decrement64 endp

interlocked_exchange64 proc
lock xchg   qword ptr [rcx],rdx 
mov rax, rdx
ret
interlocked_exchange64 endp

End
