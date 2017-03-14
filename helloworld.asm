[bits 32]
global _start
_start:mov byte [0xb8000],'H'
mov byte[0xb8002],'e'
mov byte[0xb8004],'l'
mov byte[0xb8006],'l'
mov byte[0xb8008],'o'
hng:jmp hng
