;Hello World for bad6502
BC = $10
DB = $11
S1 = $1000

	LDA #<TEXT1  ; Get Lowbyte of Address
	STA $FFF8   ; Store in pointer
	LDA #>TEXT1  ; Get Hibyte of Address
	STA $FFF9   ; Store in pointer+1

	LDA #<TEXT2  ; Get Lowbyte of Address
	STA $FFF8   ; Store in pointer
	LDA #>TEXT2  ; Get Hibyte of Address
	STA $FFF9   ; Store in pointer+1

STOP
	JMP STOP
	
;Assist callback request <INT Request>,<DATA 1...n>
TEXT1:
	.BYTE "Test program running !",10,00
TEXT2:
	.BYTE "...hello world...",10,00

