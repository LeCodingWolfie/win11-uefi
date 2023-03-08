#!/usr/bin/env make

SHELL=/bin/bash

LOG=debug.log
OVMFBASE=edk2/Build/OvmfX64/DEBUG_GCC5/
OVMFCODE=$(OVMFBASE)/FV/OVMF_CODE.fd
OVMFVARS=$(OVMFBASE)/FV/OVMF_VARS.fd
QEMU=qemu-system-x86_64
QEMUFLAGS=-drive format=raw,file=fat:rw:image \
          -drive if=pflash,format=raw,readonly,file=$(OVMFCODE) \
          -drive if=pflash,format=raw,file=$(OVMFVARS) \
          -debugcon file:$(LOG) -global isa-debugcon.iobase=0x402 \
          -serial stdio \
          -nographic \
          -nodefaults

run:
    $(QEMU) $(QEMUFLAGS)

debug:
    $(QEMU) $(QEMUFLAGS) -s -S

.PHONY: run debug
