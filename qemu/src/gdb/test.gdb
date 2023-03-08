########
# Setup
########

# Prints out large amounts of information without asking the user [1]
set pagination off

# Skips confirmation [5], which allows to break the entry point even if not defined
set breakpoint pending on

# Adds debugging symbols over the most relevant functions and types at UEFI [2]
add-symbol-file edk2/Build/OvmfX64/DEBUG_GCC5/X64/DxeCore.debug 0x7e88240
add-symbol-file edk2/Build/OvmfX64/DEBUG_GCC5/X64/Shell.debug 0x60ae240

# Connects to QEMU gdbserver
target remote localhost:1234

##############
# Breakpoints
##############

# Breaks AllocatePages if both allocate and memory type arguments are AllocateAddress and EfiLoaderData
break CoreAllocatePages if $rcx == 0x2 && $rdx == 0x2
commands
	# continue
end

# Breaks at InternalShellExecuteDevicePath after the loaded image is returned (at L. 1522), which is executed when running an image from path; see [3 & 4]
# In order to infer the image entry point address (i.e. EfiEntry)
break "edk2/ShellPkg/Application/Shell/ShellProtocol.c":1531
commands
	# Prints command
	x/11s CommandLine
	# Extracts the image base; calculates its entry point; and dissasembles
	p/x LoadedImage->ImageSize
	p/x LoadedImage->ImageBase
	set $entry = $+0x2d2a0
	disas $entry,+16
	break *$entry
		# EfiInitCreateInputParametersEx (AllocatePages setup)
	break *$entry+0x124c1
		# id. call
	break *$entry+0x124a5
	continue
end

continue

#############
# References
#############

# [1]: https://sourceware.org/gdb/onlinedocs/gdb/Screen-Size.html
# [2]: https://retrage.github.io/2019/12/05/debugging-ovmf-en.html
# [3]: https://github.com/tianocore/edk2/blob/6ae2b6648eb4b42b5a133f3cde567f9765467bf6/ShellPkg/Application/Shell/Shell.c#L2647
# [4]: https://github.com/tianocore/edk2/blob/6ae2b6648eb4b42b5a133f3cde567f9765467bf6/ShellPkg/Application/Shell/ShellProtocol.c#L1522
# [5]: https://stackoverflow.com/a/11356376
