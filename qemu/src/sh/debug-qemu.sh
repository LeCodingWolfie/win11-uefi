LOG=debug.log
OVMFBASE=edk2/Build/OvmfX64/DEBUG_GCC5/
OVMFCODE="$OVMFBASE/FV/OVMF_CODE.fd"
OVMFVARS="$OVMFBASE/FV/OVMF_VARS.fd"
qemu-system-x86_64 -drive format=raw,file=fat:rw:image \
          -drive if=pflash,format=raw,readonly=on,file=$OVMFCODE \
          -drive if=pflash,format=raw,file=$OVMFVARS \
          -debugcon file:$LOG -global isa-debugcon.iobase=0x402 \
          -serial stdio \
          -nographic \
          -nodefaults -s #-S
