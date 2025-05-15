if [ "$1" == "-c" ]; then
    echo "Cleaning build..."
    make clean
    rm -rf scratch/output
    mkdir -p scratch/output
fi
make
make dump > scratch/output/dump.txt
make symbols > scratch/output/symbols.txt
make sections > scratch/output/sections.txt

mkdir -p scratch/output

# Dumping symbols using nm and objdump
riscv32-unknown-elf-nm -n -S build/bin/main.elf > scratch/output/nm.txt
echo "" >> scratch/output/nm.txt
echo "Dumping symbols section (with objdump)..." >> scratch/output/nm.txt
riscv32-unknown-elf-objdump -t build/bin/main.elf | sort >> scratch/output/nm.txt
riscv32-unknown-elf-objdump -D build/bin/main.elf > scratch/output/disas.txt

# Dumping sections and symbols of main.o using nm and objdump
riscv32-unknown-elf-nm -n -S build/obj/src/main.o > scratch/output/nm_main.txt
echo "" >> scratch/output/nm_main.txt
echo "Dumping sections..." >> scratch/output/nm_main.txt
riscv32-unknown-elf-readelf -S build/obj/src/main.o >> scratch/output/nm_main.txt
echo "" >> scratch/output/nm_main.txt
echo "Dumping symbols section (with objdump)..." >> scratch/output/nm_main.txt
riscv32-unknown-elf-objdump -t build/obj/src/main.o | sort >> scratch/output/nm_main.txt

# Creating programs files
executePython build/bin/main.elf build/bin/testLib.h testLib
# executePython --format sv build/bin/main.elf build/bin/testLib.svh testLib
# executePython --format coe build/bin/main.elf build/bin/instruction.coe build/bin/RAM.coe

executePython build/bin/main.elf ~/workspace/TestCPU/src/programs/testLibProgram.h LibProgram