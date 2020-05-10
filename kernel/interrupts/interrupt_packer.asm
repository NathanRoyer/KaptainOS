[extern isr_handler]
[extern irq_handler]

isr_packer:
	; fonction qui monte la structure registers_state_t pour l'isr_handler
	
	pusha ; pousse edi, esi, abp, esp, ebx, edx, ecx, eax
	xor eax, eax ; eax = 0
	mov ax, ds ; eax = segment data
	push eax ; résultat : pousse ds sur la pile
	
	;mov ax, 0x10 ; vu sur un tutoriel, pas compris l'utilité de cette partie.
	;mov ds, ax
	;mov es, ax
	;mov fs, ax
	;mov gs, ax
	
	call isr_handler ; fonction en C qui prends pour paramètre le registers_state_t construit précédemment
	
	; restoration
	pop eax
	
	; voir ligne 11 :
	;mov ds, ax
	;mov es, ax
	;mov fs, ax
	;mov gs, ax
	
	popa
	add esp, 8 ; les isr_[n] push 2 fois 32 bits = 8 octets de plus sur la pile, on nettoie ça ici
	sti
	iret

irq_packer:
	; fonction qui monte la structure registers_state_t pour l'irq_handler
	
	pusha ; pousse edi, esi, abp, esp, ebx, edx, ecx, eax
	xor eax, eax ; eax = 0
	mov ax, ds ; eax = segment data
	push eax ; résultat : pousse ds sur la pile
	
	;mov ax, 0x10 ; vu sur un tutoriel, pas compris l'utilité de cette partie.
	;mov ds, ax
	;mov es, ax
	;mov fs, ax
	;mov gs, ax
	
	call irq_handler ; fonction en C qui prends pour paramètre le registers_state_t construit précédemment
	
	; restoration
	pop ebx
	
	; voir ligne 11 :
	;mov ds, ax
	;mov es, ax
	;mov fs, ax
	;mov gs, ax
	
	popa
	add esp, 8 ; les isr_[n] push 2 fois 32 bits = 8 octets de plus sur la pile, on nettoie ça ici
	sti
	iret

%macro isr_version 1
global isr_%1
isr_%1:
	cli
	push 0
	push %1
	jmp isr_packer
%endmacro

%macro irq_version 1
global irq_%1
irq_%1:
	cli
	push %1
	push 32+%1
	jmp irq_packer
%endmacro

; nothing after this

isr_version 0
isr_version 1
isr_version 2
isr_version 3
isr_version 4
isr_version 5
isr_version 6
isr_version 7
isr_version 8
isr_version 9
isr_version 10
isr_version 11
isr_version 12
isr_version 13
isr_version 14
isr_version 15
isr_version 16
isr_version 17
isr_version 18
isr_version 19
isr_version 20
isr_version 21
isr_version 22
isr_version 23
isr_version 24
isr_version 25
isr_version 26
isr_version 27
isr_version 28
isr_version 29
isr_version 30
isr_version 31
isr_version 32
isr_version 33
isr_version 34
isr_version 35
isr_version 36
isr_version 37
isr_version 38
isr_version 39
isr_version 40
isr_version 41
isr_version 42
isr_version 43
isr_version 44
isr_version 45
isr_version 46
isr_version 47
isr_version 48
isr_version 49
isr_version 50
isr_version 51
isr_version 52
isr_version 53
isr_version 54
isr_version 55
isr_version 56
isr_version 57
isr_version 58
isr_version 59
isr_version 60
isr_version 61
isr_version 62
isr_version 63
isr_version 64
isr_version 65
isr_version 66
isr_version 67
isr_version 68
isr_version 69
isr_version 70
isr_version 71
isr_version 72
isr_version 73
isr_version 74
isr_version 75
isr_version 76
isr_version 77
isr_version 78
isr_version 79
isr_version 80
isr_version 81
isr_version 82
isr_version 83
isr_version 84
isr_version 85
isr_version 86
isr_version 87
isr_version 88
isr_version 89
isr_version 90
isr_version 91
isr_version 92
isr_version 93
isr_version 94
isr_version 95
isr_version 96
isr_version 97
isr_version 98
isr_version 99
isr_version 100
isr_version 101
isr_version 102
isr_version 103
isr_version 104
isr_version 105
isr_version 106
isr_version 107
isr_version 108
isr_version 109
isr_version 110
isr_version 111
isr_version 112
isr_version 113
isr_version 114
isr_version 115
isr_version 116
isr_version 117
isr_version 118
isr_version 119
isr_version 120
isr_version 121
isr_version 122
isr_version 123
isr_version 124
isr_version 125
isr_version 126
isr_version 127
isr_version 128
isr_version 129
isr_version 130
isr_version 131
isr_version 132
isr_version 133
isr_version 134
isr_version 135
isr_version 136
isr_version 137
isr_version 138
isr_version 139
isr_version 140
isr_version 141
isr_version 142
isr_version 143
isr_version 144
isr_version 145
isr_version 146
isr_version 147
isr_version 148
isr_version 149
isr_version 150
isr_version 151
isr_version 152
isr_version 153
isr_version 154
isr_version 155
isr_version 156
isr_version 157
isr_version 158
isr_version 159
isr_version 160
isr_version 161
isr_version 162
isr_version 163
isr_version 164
isr_version 165
isr_version 166
isr_version 167
isr_version 168
isr_version 169
isr_version 170
isr_version 171
isr_version 172
isr_version 173
isr_version 174
isr_version 175
isr_version 176
isr_version 177
isr_version 178
isr_version 179
isr_version 180
isr_version 181
isr_version 182
isr_version 183
isr_version 184
isr_version 185
isr_version 186
isr_version 187
isr_version 188
isr_version 189
isr_version 190
isr_version 191
isr_version 192
isr_version 193
isr_version 194
isr_version 195
isr_version 196
isr_version 197
isr_version 198
isr_version 199
isr_version 200
isr_version 201
isr_version 202
isr_version 203
isr_version 204
isr_version 205
isr_version 206
isr_version 207
isr_version 208
isr_version 209
isr_version 210
isr_version 211
isr_version 212
isr_version 213
isr_version 214
isr_version 215
isr_version 216
isr_version 217
isr_version 218
isr_version 219
isr_version 220
isr_version 221
isr_version 222
isr_version 223
isr_version 224
isr_version 225
isr_version 226
isr_version 227
isr_version 228
isr_version 229
isr_version 230
isr_version 231
isr_version 232
isr_version 233
isr_version 234
isr_version 235
isr_version 236
isr_version 237
isr_version 238
isr_version 239
isr_version 240
isr_version 241
isr_version 242
isr_version 243
isr_version 244
isr_version 245
isr_version 246
isr_version 247
isr_version 248
isr_version 249
isr_version 250
isr_version 251
isr_version 252
isr_version 253
isr_version 254
isr_version 255

irq_version 0
irq_version 1
irq_version 2
irq_version 3
irq_version 4
irq_version 5
irq_version 6
irq_version 7
irq_version 8
irq_version 9
irq_version 10
irq_version 11
irq_version 12
irq_version 13
irq_version 14
irq_version 15
