tcc:  parse.o y.tab.o lex.yy.o errormsg.o util.o symbol.o table.o absyn.o prabsyn.o venv.o semant.o types.o frame.o escape.o translate.o temp.o tree.o printtree.o canon.o main.o codegen.o assem.o graph.o flowgraph.o
	cc -g parse.o y.tab.o lex.yy.o errormsg.o util.o symbol.o table.o absyn.o prabsyn.o venv.o semant.o types.o frame.o escape.o translate.o temp.o tree.o printtree.o canon.o main.o codegen.o assem.o graph.o flowgraph.o -o tcc

parse.o: parse.c errormsg.h util.h
	cc -g -c parse.c

y.tab.o: y.tab.c
	cc -g -c y.tab.c

y.tab.c: tiger.grm
	yacc -dv tiger.grm

y.tab.h: y.tab.c
	echo "y.tab.h was created at the same time as y.tab.c"

errormsg.o: errormsg.c errormsg.h util.h 
	cc -g -c errormsg.c

lex.yy.o: lex.yy.c y.tab.h errormsg.h util.h
	cc -g -c lex.yy.c

symbol.o: symbol.c util.h symbol.h table.h
	cc -g -c symbol.c

table.o: table.c util.h table.h
	cc -g -c table.c

absyn.o: absyn.c util.h symbol.h absyn.h
	cc -g -c absyn.c

prabsyn.o: prabsyn.c util.h symbol.h absyn.h
	cc -g -c prabsyn.c

venv.o: venv.c venv.h
	cc -g -c venv.c

semant.o: semant.c
	cc -g -c semant.c

types.o: types.c
	cc -g -c types.c

frame.o: frame.c
	cc -g -c frame.c

escape.o: escape.c
	cc -g -c escape.c

translate.o: translate.c
	cc -g -c translate.c

temp.o: temp.c
	cc -g -c temp.c

printtree.o: printtree.c
	cc -g -c printtree.c

tree.o: tree.c
	cc -g -c tree.c

#lex.yy.c: tiger.lex
#	lex tiger.lex

util.o: util.c util.h
	cc -g -c util.c

canon.o: canon.c
	cc -g -c canon.c

main.o: main.c
	cc -g -c main.c

codegen.o: codegen.c
	cc -g -c codegen.c

assem.o: assem.c
	cc -g -c assem.c

flowgraph.o: flowgraph.c
	cc -g -c flowgraph.c

graph.o: graph.c
	cc -g -c graph.c

clean:
	rm -f tcc  util.o parsetest.o lex.yy.o errormsg.o y.tab.c y.tab.h y.tab.o absyn.o parse.o symbol.o table.o prabsyn.o venv.o semant.o types.o frame.o escape.o translate.o temp.o tree.o printtree.o canon.o main.o codegen.o assem.o graph.o flowgraph.o
