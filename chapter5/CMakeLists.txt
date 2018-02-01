cmake_minimum_required(VERSION 3.9)
project(chapter5)

set(CMAKE_CXX_STANDARD 11)

add_executable(chapter5
        absyn.c
        absyn.h
        errormsg.c
        errormsg.h
        lex.yy.c
        parse.c
        parse.h
        prabsyn.c
        prabsyn.h
        semant.c
        semant.h
        symbol.c
        symbol.h
        table.c
        table.h
        types.c
        types.h
        util.c
        util.h
        venv.c
        venv.h)
