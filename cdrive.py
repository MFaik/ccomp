#! /bin/python
import argparse
import os
import subprocess

parser = argparse.ArgumentParser(prog='cdrive', description='Compiler Driver')

parser.add_argument('ccomp', help='The compiler')
parser.add_argument('filename', help='The file to be compiled')
parser.add_argument('--lex', action='store_true',
                    help='Directs the compiler to run the lexer, but stop\
                            before parsing')
parser.add_argument('--parse', action='store_true',
                    help='Directs the compiler to run the lexer and parser, but\
                            stop before tacky generation')
parser.add_argument('--tacky', action='store_true',
                    help='Directs the compiler to run the lexer parser and tacky\
                            generation, but stop before assembly generation')
parser.add_argument('--codegen', action='store_true',
                    help='Directs the compiler to perform lexing, parsing, tacky\
                            generation and assembly generation, but stop before\
                            code emission')
parser.add_argument('--output', '-o', help='Output file')

args = parser.parse_args()

filename, file_extention = os.path.splitext(args.filename)

option = 'c'
if(args.lex):
    option = 'l'
elif(args.parse):
    option = 'p'
elif(args.tacky):
    option = 't'
elif(args.codegen):
    option = 'c'

result = subprocess.run(
    [args.ccomp, option, args.filename, args.output if args.output else ''],
    stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True
)
print(result.stdout)
exit(result.returncode)

#normal gcc pipeline
# os.system(f"gcc -E -P {filename+file_extention} -o {filename}.i")
#
# os.system(f"gcc -S {filename}.i -o {filename}.s")
# os.system(f"rm -f {filename}.i")
#
# os.system(f"gcc {filename}.s -o {filename}")
# os.system(f"rm -f {filename}.s")
