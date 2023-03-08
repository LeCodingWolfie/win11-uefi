---
header-includes: |
	<link rel="stylesheet" type="text/css" href="css/css.css"/>
	<style>
	/* header {
		margin-bottom: 0em;
		padding: 0.5em;
		padding-left: 1.5em;
		background: #3e4b66;
		border-radius: 0.4em;
		text-align: left;
	} */

	body h1 + h2 {
		margin-top: 0.5em;
		margin-bottom: 0;
	}

	blockquote + hr {
		height: 2px;
		background-color: #333;
	}

	div + hr {
		height: 4px;
		margin: 2em 0em 1.5em 0em;
		background-color: #ddd;
	}
	</style>
fontsize: 14px
title: Dissasembly
lang: en
---
<!-- markdownlint-disable -->

# EfiInitCreateInputParametersEx

## HandleProtocol

```c
	EFI_BOOT_SERVICES *bootServices = table->BootServices;
	EFI_STATUS status = bootServices->HandleProtocol(handle, &EfiLoadedImageProtocol, /**/);
```

### *To find the method*

In Ghidra, the HandleProtocol call is rather difficult to grasp on the decompilation:

```
	lVar2 = (**(code **)(*(longlong *)(param_2 + 0x60) + 0x98))(param_1,&EfiLoadedImageProtocol,&local_res10);
```

Therein, the code represented as assembly language follows:

```
	MOV RAX, qword ptr [param_2 + 0x60]
	LEA RDX, [EfiLoadedImageProtocol]
	MOV RAX, qword ptr [RAX + 0x98]
	CALL RAX
```

where param_2 (RDX) is the system table.

In fact, pointer aritmetic is performed on the table address, by moving the address forward about 96 bytes, and then 152 B, and then calling on that address; like a function pointer.
This suggests that a method is being accessed from an structure inside the table.

Thus, is matter of checking which members are pointed to in each structure as defined in UefiSpec.h, by counting how much bytes each member occupies — up to down:

```
	0x60:
	  * EFI_TABLE_HEADER (-> 24 B)
	  * FirmwareVendor * (-> 8 B) # should be 6 B for 7 times per! (but nevermind)
	  * FirmwareRevision (-> 4 B)
	  * ConsoleInHandle to RuntimeServices (-> 8 B per pointer, i.e. 7 times / 56 B)

	--- Points to EFI_BOOT_SERVICES *

	0x98:
	  * EFI_TABLE_HEADER (-> 24 B)
	  * RaiseTPL to UninstallProtocolInterface (-> 8 B per, i.e. 16 times / 128 B)

	--- Points to HandleProtocol
```

considering each pointer size is of 8 bytes (i.e. 64-bit) <sup>(see [2], [3])</sup>, specially since these might be near pointers and respectively change of size accordingly with the bit architecture <sup>([4]: § 4.3.).

That means EFI_BOOT_SERVICES struct is accessed from the table to point to the HandleProtocol method — so it gets called.

### *To discover the parameters*

*Besides the handle*, the LEA instruction shows that EfiLoadedImageProtocol address is used, as it stores its address on RDX <sup>(see [5])</sup> – so `&EfiLoadedImageProtocol` is used.

#### `local_res10`

Defined in Ghidra as:

```asm
	LEA param_3 => local_res10, [RBP + 0x38]
	----------------------------------------
	LEA R8, [RBP + 0x38]
```

Accordingly with the UEFI specification, the final argument is an interface variable to be added a value to (`(VOID **)Interface`), which will return the loaded image if already loaded <sup>(see [6]: § 1.8.2. & p. 188-9)</sup>. Thus, as we try to get the loaded image, one should use `EFI_LOADED_IMAGE_PROTOCOL` type on the Interface variable <sup>(see *id.* § 2.4)</sup>.


## *Next* HandleProtocol

Again, another similar call, that in assembly language follows:

```asm
	MOV RDI, param_2 ; table is passed to RDI
	--------
	MOV RCX, qword ptr [RBP + 0x38] ; i.e. &local_res10
	LEA RDX, [EfiDevicePathProtocol] ; as prev. i.e. &EfiDevicePathProtocol
	--------
	MOV RAX, qword ptr [RDI + 0x60] ; like prev. moves 96 bytes forward from the table
	MOV RCX, qword ptr [RCX + 0x18] ; moves up 24 B from the loaded image
	MOV RAX, qword ptr [RAX + 0x98] ; id. moves up 156 B from table
```

Now, matter is to find what the loaded image points to:

```
	0x18:
	  * Revision (-> 4 B)
	  * ParentHandle & SystemTable (-> 8 B per, i.e. 16 B)

	--- Points to DeviceHandle
```

Like the last time, for the Interface variable, the type is `EFI_DEVICE_PATH_PROTOCOL` from EfiDevicePathProtocol variable, and is same way casted to `VOID **`.


## Another call on BootServices

Just as the previous calls, BootServices is pointed from the system table, which then points to required function to be called:

```asm
	MOV RAX, qword ptr [RDI + 0x60] ; Again, RDI (i.e. system table) is used to point to BootServices
	--------
	MOV RAX, qword ptr [RAX + 0x28] ; Moves up 40 B from BootServices
	CALL RAX ; Calls on that address
```

Yet, it calls on AllocatePages, whereas in ReactOS, BootServices is not pointed to a function anymore <sup>(see [1])</sup>, since:

```
	0x28:
	  * EFI_TABLE_HEADER (-> 24 B)
	  * RaiseTPL & RestoreTPL (-> 8 B per, i.e. 16 B)

	--- Points to AllocateAddress
```

By analysing the whole assembly, it follows:

```asm
	MOV EBX, 0x2  ; Moves 0x2 to EBX
	--------
	MOV RAX, qword ptr [RDI + 0x60]
	LEA R9, [RBP + 0x48]					; Stores an in-stack address to R9
    MOV qword ptr [RBP + 0x48], 0x102000	; Moves 0x102000 to the address
    LEA R8D, [RBX + -0x1]					; "Should" store the address of RBX by one byte minus (e.g. param_3)
    MOV EDX, EBX							; Moves 0x2 to EDX
    MOV ECX, EBX							; id. to ECX
	MOV RAX, qword ptr [RAX + 0x28]
	CALL RAX
```

Thence, the function is called as:

```c
	AllocatePages(0x2, 0x2, -0x1, &0x102000); // Here, though, -0x1 is just used as a placeholder, the exact value is indeterminate
```

According to the function definition in UefiSpec.h <sup>(see [7])</sup>, it requires two enum-valued parameters (i.e. types), where 0x2 actually corresponds properly as:

```
	AllocatePages(AllocateAddress, EfiLoaderData, ?, &0x102000);
```

In fact, the EfiLoaderData type is used in a wrap-up method for AllocatePages in the ReactOS code, whereas AllocateAddress is used elsewhere. <sup>(see [8] & [9])</sup>

Two issues arise:

> (1) is to find the value of `RBX + -0x1` or `RBX - 1` (in Cutter/Rizin), which "should" be behind the register address in memory.
> (2) is the reason of which a pointer to 0x102000 is used as the address, number which throws many variate results on a single search, such as the kernel32.dll size.

These issues represent (somehow) a challenge that requires dynamic analysis, and so debugging UEFI images is. ~~Thus, this line is omitted.~~

Not so fast, let's dig deeper why.

### `LEA` *magic* 🪄

It's possible you might've noticed a flaw on explaining the LEA logic, and it's in fact.

## *Pages*?

In fact, *pages*. But let's see why.

The following assembly code follows:

```asm
	MOV RCX, qword ptr [RBP + 0x38] ; Moves local_res10 (or the loaded image) to RCX
	--------
	MOV RAX, qword ptr [RCX + 0x40]  ; Moves up 64 B from the loaded image
	MOV qword ptr [DAT_1020f8f8], RAX  ; Stores the value to some address
	MOV EAX, dword ptr [RCX + 0x48]
	MOV dword ptr [DAT_1020f900], EAX  ; Moves up 72 B and stores it
	--------
	MOV ECX, dword ptr [DAT_1020f900]  ; Takes the "72 B" value to ECX
	MOV RAX, qword ptr [DAT_1020f8f8]  ; Stores the "64 B" address in RAX
    SHR RAX, 0xc						; Moves the value 12 bits right (>>) – see [10, 11]
    LEA EDX, [ECX + 0xfff]  ; Adds 0xfff to "72 B" and stores it in EDX
    AND EDX, -0x1000  ; Performs the bitwise AND as: EDX & -0x1000 (i.e. 0xfffff000)
	--------
	CMP EDX, ECX  ; "Compares" EDX & ECX ("72 B") where if EDX is bigger, the carry flag is set – thus, ECX should be bigger; so it acts as part of a conditional – see [12, 13]
	JC LAB_1003f906  ; As a conditional jump, if the carry flag is set, jumps to an address – see [14]
	--------
	# LAB_1003f906
	XOR EAX, EAX  ; Simply means that EAX is set to 0 since it is equal to itself – see [15]
				  ; Thus, it does not jump inside the conditional (which would've, if the condition was met), and runs up to the end of the function, as shown by Cutter.
	--------
	SHR EDX, 0xc  ; Moves EDX 12 bits right
```
<sup>For reference: ([10], [11], [12], [13], [14], [15])</sup>

Bigger and more complex than the previous assemblies, in fact.

In summary, from what we know, what it does is just some back-and-forth with byte operations.

Anyways, it's worth looking at what the "64 B" & "72 B" values point to:

```
	0x40:
	  * Revision (-> 4 B)
	  * ParentHandle to LoadOptions (-> 8 B per, i.e. 56 B / 7 times)

	--- Points to ImageBase

	0x48:
	  * ImageBase (-> 8 B)

	--- Points to ImageSize
```

Thus, code follows:

```c
	UINT64 *imageBase = loadedImage->ImageBase; // i.e. qword ptr
	UINT32 imageSize = loadedImage->ImageSize; // i.e. dword ptr

	UINT64 baseSHR = *imageBase >> 0xc;

	UINT32 *sizeFFF = &imageSize + 0xfff;
	UINT32 sizeAND = *sizeFFF & 0xfffff000; // i.e. -0x1000 (due to two's complement)

	if (sizeAND < imageSize) {
		return 0;
	}

	UINT32 sizeSHR = sizeAND >> 0xc;
```

Due to the otherwise odd programming, guided from ReactOS code, all has to do with *paging*.

To understand so, one step is to simplify common values into variables – i.e. every hexadecimal number:

> **0xfff and -0x1000:** 0xfff is 4095, whereas 0x1000 is 4096. Thus, 0xfff is 0x1000 subtracted by one.

Next is to understand and then name each value:

> **4096:** In AllocatePages each page is defined as about 4 KiB <sup>([6]: p. 166)</sup>, where 4096 B denotes 4 KB; thus, 4096 is the page size in bytes.
>
> **0xc (12):** Can be taken as a SHIFT variable, so can be named as PAGE_SHIFT.

______

> Based on ReactOS code: <sup>(see [1])</sup>
> sizeFFF (1) is then the end of the first page in the image, and sizeAND (2) the count of the pages that the image holds.
> For baseSHR (3), the shift seems to convert bytes to pages. <sup>(see [16])</sup> In that case, it can be named to be the basePage.

What about the conditional? The code in ReactOS doesn't include one, so why does the assembly do? Considering it should, in any case, the number of pages shouldn't be bigger than the image size – since the amount of pages was calculated from the image size, so that line of code is omitted.

Then, the code would follow as:

```c
	#define PAGE_SHIFT 0xc;
	#define PAGE_SIZE 0x1000; // i.e. 4096 B

	UINT64 basePage = *imageBase >> PAGE_SHIFT

	UINT32 *endOfFirstPage = &imageSize + (PAGE_SIZE - 1)
	UINT32 pageCount = (*endOfFirstPage + -PAGE_SIZE) >> PAGE_SHIFT;
```

______

As a side note, the analysis and code syntax were based on ReactOS code with the help of Doxygen [1].

[1]: https://github.com/reactos/reactos/blob/fe777bb52f67921b26bf5791b06a5c712f5be3f6/boot/environ/app/bootmgr/efiemu.c#L862
[2]: https://stackoverflow.com/a/6751809
[3]: https://stackoverflow.com/a/56209070
[4]: https://cdrdv2.intel.com/v1/dl/getContent/671436
[5]: https://www.felixcloutier.com/x86/lea
[6]: https://uefi.org/sites/default/files/resources/UEFI_Spec_2_9_2021_03_18.pdf
[7]: https://github.com/tianocore/edk2/blob/fca5de51e1fd2f3c5ddbf5974d785f0f6b2f6c38/MdePkg/Include/Uefi/UefiSpec.h#L187
[8]: https://github.com/reactos/reactos/blob/23ecbb3ed5f1b2881e28e54cccec16586e8241fb/boot/environ/lib/firmware/efi/firmware.c#L1591
[9]: https://github.com/reactos/reactos/blob/23ecbb3ed5f1b2881e28e54cccec16586e8241fb/boot/environ/lib/firmware/efi/firmware.c#L1936
[10]: https://c9x.me/x86/html/file_module_x86_id_285.html
[11]: https://www.ibm.com/docs/en/i/7.2?topic=expressions-bitwise-left-right-shift-operators
[12]: https://stackoverflow.com/a/45898850
[13]: https://stackoverflow.com/a/61568237
[14]: https://www.keil.com/support/man/docs/is51/is51_jc.htm
[15]: https://stackoverflow.com/a/4749620
[16]: https://github.com/mirror/reactos/blob/c6d2b35ffc91e09f50dfb214ea58237509329d6b/reactos/ntoskrnl/mm/ARM3/mminit.c#L514
