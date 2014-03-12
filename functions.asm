
 ;//Exports

GLOBAL bsf64

;//Data

SEGMENT .data align=16

;//Code

SEGMENT .text

ALIGN 16

bsf64:
	bsf eax, [esp+4]
	jnz finish

	bsf eax, [esp+8]
	add eax, 32
	jnz finish

	mov eax, 0
	finish:
ret


bsr64:
	bsr eax, [esp+8]
	add eax, 32
	jnz finish2

	bsr eax, [esp+4]
	jnz finish2

	mov eax, 0
	finish2:
ret
end
