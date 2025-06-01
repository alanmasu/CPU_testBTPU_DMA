import argparse
import os
from elftools.elf.elffile import ELFFile
from elftools.elf.constants import SHN_INDICES

def is_data_symbol(symbol):
    return symbol['st_info']['type'] == 'STT_OBJECT'

def is_valid_symbol(symbol):
    return (
        symbol['st_shndx'] != 'SHN_UNDEF' and
        isinstance(symbol['st_value'], int) and
        symbol['st_value'] != 0
    )

def extract_data_symbols(elf_path):
    symbols = []
    with open(elf_path, 'rb') as f:
        elf = ELFFile(f)
        symtab = elf.get_section_by_name('.symtab')
        if symtab is None:
            print("Nessuna sezione .symtab trovata.")
            return symbols

        for symbol in symtab.iter_symbols():
            if is_data_symbol(symbol) and is_valid_symbol(symbol):
                symbols.append((symbol.name.upper(), symbol['st_value']))
    return symbols

def read_existing_defines(file_path, fmt):
    if not os.path.exists(file_path):
        return set()

    existing = set()
    with open(file_path, 'r') as f:
        for line in f:
            line = line.strip()
            if fmt == 'c' and line.startswith('#define'):
                parts = line.split()
                if len(parts) >= 2:
                    existing.add(parts[1])
            elif fmt == 'sv' and line.startswith('`define'):
                parts = line.split()
                if len(parts) >= 2:
                    existing.add(parts[1])
    return existing

def generate_definitions(symbols, fmt, existing):
    lines = []
    for name, addr in symbols:
        if name in existing:
            continue
        if fmt == 'c':
            lines.append(f"#define {name} 0x{addr:08X}")
        elif fmt == 'sv':
            lines.append(f"`define {name} 32'h{addr:08X}")
    return lines

def main():
    parser = argparse.ArgumentParser(description="Estrai indirizzi di variabili da un file ELF.")
    parser.add_argument("elf_file", help="Path al file ELF")
    parser.add_argument("-f", "--format", choices=['c', 'sv'], default='c',
                        help="Formato di output: c (.h) o sv (.svh)")
    parser.add_argument("-o", "--output", required=True, help="File di output (.h o .svh)")
    parser.add_argument("-a", "--append", action="store_true", help="Aggiungi all'output invece di sovrascrivere")
    args = parser.parse_args()

    symbols = extract_data_symbols(args.elf_file)
    if not symbols:
        print("Nessuna variabile trovata.")
        return

    existing = read_existing_defines(args.output, args.format) if args.append else set()
    lines = generate_definitions(symbols, args.format, existing)

    if not lines:
        print("Nessun nuovo simbolo da scrivere.")
        return

    mode = 'a' if args.append else 'w'
    with open(args.output, mode) as out_file:
        out_file.write('\n'.join(lines) + '\n')

    print(f"{len(lines)} nuovi simboli scritti in '{args.output}' come formato {args.format.upper()}.")

if __name__ == "__main__":
    main()
