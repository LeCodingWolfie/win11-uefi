---
header-includes: |
	<link rel="stylesheet" type="text/css" href="css/css.css"/>
	<style>
	body {
		max-width: 75%;
	}

	h1, h2, h3, h4, h5, h6 {
		margin-top: 0em;
		margin-bottom: 0;
	}

	p {
		line-height: 20px;
	}
	</style>
fontsize: 16px
title: Documentation
lang: en
---

<!-- markdownlint-disable MD013 MD025 MD033 MD053 -->

In x64 systems, bootx64.efi represents the UEFI image file booted from removable media, <sup>([1] § 3.5.1.1)</sup> which borrows from bootmgfw.efi code, used to initialize Windows on an installed system. <sup>(see [2], p. 242)</sup>

EfiEntry (adapted from [1] <sup>§ 4.1.1</sup>) represents the entry point of the image, once loaded into memory, after the UEFI platform has been initialized <sup>([1] § 2.1 / [2], *id.*)</sup>, which takes two parameters, a handle that points to the image and a pointer to the EFI system table (reg. RCX & RDX) <sup>(see [1] § 2.3.2.1) (a)</sup>, which to unload the image, returns an EFI exit code depending on the flow of execution (reg. RSP) <sup>(see [1] § 2.1.1 / [3])</sup>.

To that end, EfiInitCreateInputParametersEx is triggered to convert the arguments passed to the image entry point into EFI parameters useful to boot Windows <sup>([2], p. 247)</sup>, and if successful, calls BmMain to return a value the EFI code is inferred of. <sup>(see [3])</sup>

# Footnotes

<sup>(a)</sup> As a side note, the EFI system table points to the devices, services, and configuration available to UEFI. <sup>(see [1] § 4.1)</sup>

<!-- # References -->

[1]: https://uefi.org/sites/default/files/resources/UEFI_Spec_2_10_Aug29.pdf
[2]: https://www.kea.nu/files/textbooks/humblesec/rootkitsandbootkits.pdf
[3]: https://github.com/reactos/reactos/blob/d15f12614397d14264e676aed343d1b56937efcd/boot/environ/app/bootmgr/efiemu.c#L1020

<!-- ______ -->

# Code

<sup>(a)</sup> Provides the UEFI definitions included within ReactOS code <sup>([C.1])</sup>, which were taken from the Module Development Environment (MDE) Package from TianoCore EDK II. <sup>(see [C.2] & [C.3])</sup>

<!-- ## Sources -->

[C.1]: https://github.com/reactos/reactos/tree/master/boot/environ/include/efi
[C.2]: https://github.com/tianocore/tianocore.github.io/wiki/MdePkg
[C.3]: https://github.com/tianocore/edk2/blob/master/MdePkg/MdePkg.uni
