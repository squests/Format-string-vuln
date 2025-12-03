# Format-String-Vuln
ROP Chaining and bypassing ASLR, PIE, Canaries, NX.
## Intro
Buffer overflow into ROP chaining is extremely powerful. However, in these days, they are stopped by ASLR, PIE, Canaries, and NX.
## ASLR
ASLR (Address Space Layout Randomisation) is a method used to randomise the base addresses of procedures in libc. Instead of eg printf starting from a fixed address (0x400600 for example), ASLR would change that address everytime the program ran using a formula as so: [Random Base Offset] + [Actual Address].
## PIE
Similarly, PIE does the same thing but for your own procedures in the program (main(), func1(), func2()). It follows the exact same formula, but the random base offset is different.
## Canaries
Canaries are a mechanism specifically designed to prevent stack smashing and overwriting the return value or any other values. After the procedure is done, before it pops rbp or returns, it checks the value of the canary to see if it is the same. This canary value is fetched from something called TLS(Thread Local Storage). Just remember that it's practically impossible to fetch before running the program. The mechanism works as so: It pushes the value of the canary onto the stack before rbp, and if you try and overwrite the return address, it will smash into the canary value and change it, therefore the program stops.
## NX
NX is quite simply that bytes on the stack aren't able to be executed, so no shellcode can be injected. Format-String-Vuln is able to bypass this but requires a lot of tedious work to write shellcode with this methodology.

# What is Format String Vuln?
