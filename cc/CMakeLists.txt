cmake_minimum_required(VERSION 3.9)
project(cc)

set(CMAKE_CXX_STANDARD 11)

add_executable(cc
        absyn.c
        absyn.h
        errormsg.c
        errormsg.h
        frame.c
        frame.h
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
        temp.c
        temp.h
        types.c
        types.h
        util.c
        util.h
        canon.c
        canon.h
        venv.c
        printtree.c
        printtree.h
        tree.c
        tree.h
        main.c
        assem.c
        assem.h
        graph.c
        graph.h
        flowgraph.c
        flowgraph.h
        liveness.c
        liveness.h
        codegen.c
        codegen.h
        venv.h escape.h escape.c translate.c translate.h)
