// i.e. Tianocore EDK II (UEFI) include files [a]
#include "include/Uefi.h"
#include "include/LoadedImage.h" // Provides EFI_LOADED_IMAGE_PROTOCOL
#include "include/DeviceGUID.h" // Provides EFI_DEVICE_PATH_PROTOCOL_GUID

void EfiInitpCreateApplicationEntry(
	EFI_SYSTEM_TABLE table
) {
	return;
}

__uint128_t EfiInitCreateInputParametersEx (
	EFI_HANDLE handle,
	EFI_SYSTEM_TABLE *table
) {
	__uint128_t EfiInitScratch = 0x50504120544f4f42;

	EFI_BOOT_SERVICES *bootServices = table->BootServices; // Points to the boot services table from the base image system table [v. 1 § 4.4.]
	EFI_GUID EfiLoadedImageProtocol = EFI_LOADED_IMAGE_PROTOCOL_GUID; // Identifies an interface of an UEFI image loaded into memory [v. 9.1 & 1.9.2.]

	EFI_LOADED_IMAGE_PROTOCOL *loadedImage; // Denotes a protocol over such interface [v. 1.9.2.]

	/* Checks if the handle can return information about the image, such as its source and memory location — if it has been loaded [v. 4.1.1 & 9.1.1.]
	 * If it does, it points to an structure over the information of the loaded image, as defined by its protocol [v. 7.3.7.] */
	EFI_STATUS status = bootServices->HandleProtocol(handle, &EfiLoadedImageProtocol, (void **) loadedImage);

	if (status != EFI_SUCCESS) { // i.e. 0
		return 0;
	}

	/* Likewise, checks if the loaded image can return information about the paths found on a device [v. 10.2.] */
	EFI_GUID EfiDevicePathProtocol = EFI_DEVICE_PATH_PROTOCOL_GUID;
	EFI_DEVICE_PATH_PROTOCOL *devicePath;

	status = bootServices->HandleProtocol(loadedImage->DeviceHandle, &EfiDevicePathProtocol, (void **) devicePath);

	if (status != EFI_SUCCESS) {
		return 0;
	}

	/* Defines the number of 4 KB pages held by the image, and the image base page location */

	#define PAGE_SHIFT 0xc // i.e. 12
	#define PAGE_SIZE 0x1000 // i.e. 4096

	UINT64 *imageBase = loadedImage->ImageBase;
	UINT32 imageSize = loadedImage->ImageSize;

	UINT64 basePage = *imageBase >> PAGE_SHIFT; // Converts the image base to the base page

	UINT32 *endOfFirstPage = &imageSize + (PAGE_SIZE - 1);
	UINT32 pageCount = (*endOfFirstPage & -PAGE_SIZE) >> PAGE_SHIFT; // Converts the count to the number of pages

	EfiInitpCreateApplicationEntry(*table);

	return EfiInitScratch;
}

EFI_STATUS EFIAPI EfiEntry(
	EFI_HANDLE handle, // RCX
	EFI_SYSTEM_TABLE *table // RDX
) {
	__uint128_t EfiParameters = EfiInitCreateInputParametersEx(handle, table);

	UINT64 toStatusCode;

	if (EfiParameters == 0) {
		toStatusCode = 0xc000000d;
	} else {
		// toStatusCode = BmMain(EfiParameters);
	}

	return 0; // RSP
}
