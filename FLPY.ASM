.model large
.code

		public _FloppyStatus

_FloppyStatus	proc near

		push	bp
		mov	bp, sp

		push	dx

		mov	ah, 00h
		mov	dl, [bp+6]
		int	13h

		jnc	drive_ok

		xor	al, al
		xchg	ah, al

		jmp	end_rtn

drive_ok:

		mov	ax, 0

end_rtn:

		pop	dx
		pop	bp
		
		retf

_FloppyStatus	endp
		end
