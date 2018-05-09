all: performance

gcc: competition.c
	$@ -mtune=generic -O0 $^ -o a.out 

clang: competition.c
	$@ -mtune=generic -O0 $^ -o a.out 

performance:
	/usr/bin/time -f'%E' ./a.out ; sleep 1 ;

clean:
	rm -f a.out competition *.txt ;
