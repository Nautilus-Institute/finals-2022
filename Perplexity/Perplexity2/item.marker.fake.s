.intel_syntax noprefix
.text

.globl ItemMarkerInternalFakeFunc00
.type ItemMarkerInternalFakeFunc00, @function
.globl ItemMarkerInternalFakeFunc01
.type ItemMarkerInternalFakeFunc01, @function
.globl ItemMarkerInternalFakeFunc02
.type ItemMarkerInternalFakeFunc02, @function
.globl ItemMarkerInternalFakeFunc03
.type ItemMarkerInternalFakeFunc03, @function
.globl ItemMarkerInternalFakeFunc04
.type ItemMarkerInternalFakeFunc04, @function
.globl ItemMarkerInternalFakeFunc05
.type ItemMarkerInternalFakeFunc05, @function
.globl ItemMarkerInternalFakeFunc06
.type ItemMarkerInternalFakeFunc06, @function
.globl ItemMarkerInternalFakeFunc07
.type ItemMarkerInternalFakeFunc07, @function
.globl ItemMarkerInternalFakeFunc08
.type ItemMarkerInternalFakeFunc08, @function
.globl ItemMarkerInternalFakeFunc09
.type ItemMarkerInternalFakeFunc09, @function
.globl ItemMarkerInternalFakeFunc10
.type ItemMarkerInternalFakeFunc10, @function
.globl ItemMarkerInternalFakeFunc11
.type ItemMarkerInternalFakeFunc11, @function
.globl ItemMarkerInternalFakeFunc12
.type ItemMarkerInternalFakeFunc12, @function
.globl ItemMarkerInternalFakeFunc13
.type ItemMarkerInternalFakeFunc13, @function
.globl ItemMarkerInternalFakeFunc14
.type ItemMarkerInternalFakeFunc14, @function
.globl ItemMarkerInternalFakeFunc15
.type ItemMarkerInternalFakeFunc15, @function
.globl ItemMarkerInternalFakeFunc16
.type ItemMarkerInternalFakeFunc16, @function
.globl ItemMarkerInternalFakeFunc17
.type ItemMarkerInternalFakeFunc17, @function
.globl ItemMarkerInternalFakeFunc18
.type ItemMarkerInternalFakeFunc18, @function
.globl ItemMarkerInternalFakeFunc19
.type ItemMarkerInternalFakeFunc19, @function

ItemMarkerInternalFakeFunc00:
	mov eax, 0x00
	jmp Item_Marker_FakeFunc

ItemMarkerInternalFakeFunc01:
	mov eax, 0x04
	jmp Item_Marker_FakeFunc

ItemMarkerInternalFakeFunc02:
	mov eax, 0x08
	jmp Item_Marker_FakeFunc

ItemMarkerInternalFakeFunc03:
	mov eax, 0x0c
	jmp Item_Marker_FakeFunc

ItemMarkerInternalFakeFunc04:
	mov eax, 0x10
	jmp Item_Marker_FakeFunc

ItemMarkerInternalFakeFunc05:
	mov eax, 0x14
	jmp Item_Marker_FakeFunc

ItemMarkerInternalFakeFunc06:
	mov eax, 0x18
	jmp Item_Marker_FakeFunc

ItemMarkerInternalFakeFunc07:
	mov eax, 0x1c
	jmp Item_Marker_FakeFunc

ItemMarkerInternalFakeFunc08:
	mov eax, 0x20
	jmp Item_Marker_FakeFunc

ItemMarkerInternalFakeFunc09:
	mov eax, 0x24
	jmp Item_Marker_FakeFunc

ItemMarkerInternalFakeFunc10:
	mov eax, 0x28
	jmp Item_Marker_FakeFunc

ItemMarkerInternalFakeFunc11:
	mov eax, 0x2c
	jmp Item_Marker_FakeFunc

ItemMarkerInternalFakeFunc12:
	mov eax, 0x30
	jmp Item_Marker_FakeFunc

ItemMarkerInternalFakeFunc13:
	mov eax, 0x34
	jmp Item_Marker_FakeFunc

ItemMarkerInternalFakeFunc14:
	mov eax, 0x38
	jmp Item_Marker_FakeFunc

ItemMarkerInternalFakeFunc15:
	mov eax, 0x3c
	jmp Item_Marker_FakeFunc

ItemMarkerInternalFakeFunc16:
	mov eax, 0x40
	jmp Item_Marker_FakeFunc

ItemMarkerInternalFakeFunc17:
	mov eax, 0x44
	jmp Item_Marker_FakeFunc

ItemMarkerInternalFakeFunc18:
	mov eax, 0x48
	jmp Item_Marker_FakeFunc

ItemMarkerInternalFakeFunc19:
	mov eax, 0x4c
	jmp Item_Marker_FakeFunc

Item_Marker_FakeFunc:
//edi - this pointer to ItemMarkerInternal
//eax - which function to call by offset

//load up eax with pointer to Item

//get pointer to object
push ebx
mov ebx, [esp+8]
mov ebx, [ebx+0x0c]

//get object vtable pointer
mov ebx, [ebx]
add eax, ebx

//restore ebx
pop ebx

//jump to the proper function
jmp [eax]

Item_Marker_Fake_VTable:
.section .data.rel.ro.Item_Marker_Fake_VTable,"awG",@progbits,Item_Marker_Fake_VTable,comdat
.align 4
.type Item_Marker_Fake_VTable, @object
.size Item_Marker_Fake_VTable, 0x50
.long ItemMarkerInternalFakeFunc00
.long ItemMarkerInternalFakeFunc01
.long ItemMarkerInternalFakeFunc02
.long ItemMarkerInternalFakeFunc03
.long ItemMarkerInternalFakeFunc04
.long ItemMarkerInternalFakeFunc05
.long ItemMarkerInternalFakeFunc06
.long ItemMarkerInternalFakeFunc07
.long ItemMarkerInternalFakeFunc08
.long ItemMarkerInternalFakeFunc09
.long ItemMarkerInternalFakeFunc10
.long ItemMarkerInternalFakeFunc11
.long _ZN18ItemMarkerInternal8DescribeEv
.long ItemMarkerInternalFakeFunc13
.long ItemMarkerInternalFakeFunc14
.long ItemMarkerInternalFakeFunc15
.long ItemMarkerInternalFakeFunc16
.long ItemMarkerInternalFakeFunc17
.long ItemMarkerInternalFakeFunc18
.long ItemMarkerInternalFakeFunc19
