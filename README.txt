polynom calculator.
input: 
	string (max length 510)  
	input: polynom, x (string)
accepted symbol: '+'(addition), '^'(power), digits(integer only), ','(to delimit polynom from x), x(the desired value) 

output: 
	<the polynom> = <result>.
list of file:
	ex4a.c
	ex4b.c
	README.txt
	
compilation: 
		gcc -pthread -o threadCalc threadCalc.c
		gcc procCalc.c -o procCalc
run: 
		./threadCalc
		./procCalc

by author Elya Athlan.
