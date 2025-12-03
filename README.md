# Format-String-Vuln
ROP Chaining and bypassing ASLR, PIE, Canaries, NX.
## Intro
Buffer overflow into ROP chaining is extremely powerful. However, in these days, they are stopped by ASLR, PIE, Canaries, and NX.
## ASLR
ASLR (Address Space Layout Randomisation) is a method used to randomise the base addresses of procedures in libc. Instead of eg printf starting from a fixed address (0x400600 for example), ASLR would change that address everytime the program ran using a formula as so: [Random Base Offset] + [Actual Address].
## PIE
Similarly, PIE (Position Indepdendent Executable) does the same thing but for your own procedures in the program (main(), func1(), func2()). It follows the exact same formula, but the random base offset is different.
## Canaries
Canaries are a mechanism specifically designed to prevent stack smashing and overwriting the return value or any other values. After the procedure is done, before it pops rbp or returns, it checks the value of the canary to see if it is the same. This canary value is fetched from something called TLS(Thread Local Storage). Just remember that it's practically impossible to fetch before running the program. The mechanism works as so: It pushes the value of the canary onto the stack before rbp, and if you try and overwrite the return address, it will smash into the canary value and change it, therefore the program stops.
## NX
NX is quite simply that bytes on the stack aren't able to be executed, so no shellcode can be injected. Format-String-Vuln is able to bypass this but requires a lot of tedious work to write shellcode with this methodology.

# What is Format String Vuln?
Format String Vuln to put simply is the act of trying to access parameters that aren't there through format specifiers (%s, %n, %p, %x, etc). What's special about these is that if there is code where you are able to input something in printf and it points to the start of your buffer input, then you are able to inject these format specifiers.
char buffer[300];
fgets(buffer, sizeof(buffer), stdin)
printf(buffer);

The code above is inherently insecure. You're able to inject %x, %p, etc into the buffer assuming buffer points to the start of your inputted string. Even using fgets vs gets doesn't really matter here, using gets only makes exploitation easier. By injecting %x, it expects (printf(buffer, &somewhere)) but there isn't a &somewhere, so in assembly, it accesses the %rsi register.
## About printf
Branching off of that, printf in x86-64 machines is a variadic function, so when you pass in parameters into it, it kind of looks like this in assembly. 
printf(%rdi, %rsi, %rdx, %rcx, %r8, %r9); 
After r9, it starts printing stuff off of the **stack**.
Also %rdi is your string pointer.

# Usability
Okay great! Cool bug! How do we use this to bypass ASLR and all of those stack securities?

# The stack visualisation
In vuln, if you compile using gcc vuln.c -o vuln -fstack-protector-all, you're able to go through and map out the stack from main to the end of vuln.

The stack looks like so:
[High Memory]
old %rbp (new rbp now has the address of this location after mov %rsp,%rbp)
1st canary
8 bytes padding
ret to main from vuln
old %rbp (which holds the address of the rbp in main's stack frame)
2nd canary (of vuln)
312 bytes buffer size
[Low Memory]

**I highly encourage you draw this out or whatever helps you visualise this stack, as this will be integral to Format String Vuln**

# Putting it together
Okay so now that we have the stack visualisation and also the format string vulnerability in mind, how can we bypass ASLR and PIE?

## Defeating ASLR/PIE
Both of these methods of protection are simple on paper to bypass, but extremely hard to do in real practice. Simply leaking an address of a procedure or any instructions in the procedure defeats PIE/ASLR(If you leak a procedure address in libc, you defeat ASLR, if you leak a procedure address in the binary procedures, then you defeat PIE). 

If you are able to leak an address of a procedure, then calculating the base randomised offset is very simple. Since the address of a procedure is [Random Base Offset] + [Actual Address], you can subtract the actual address from this and get the random base offset, which allows you to know where any other procedure lies.

## Example
In this specific example, looking at the stack, we see that an address that we can try to leak is the "ret to main from vuln" which points to the next instruction in main after the vuln call.

## Leaking stack values with Format String Vuln
Remembering that after printf runs out of registers to use as inputs, it uses the stack, we can print values **directly off of the stack**. So after 
print(%x %x %x %x %x %x);
The 6th %x is actually printing 4 byte value off of the stack, where the top of the stack is the **start of your buffer**, looking at the stack frame. Ineed since adding %x's is redundant, we can shorten it to
print(%6$x);
Which prints the 6th "ghost" parameter which is off the stack.
### If you try inputting gggggggg%6$p into the program, what do you think will happen?
gggggggg0x676767676767676767
This is because since gggggggg is at the beginning of the buffer and also is 8 bytes long, %6$p will try to look at the 6th parameter, which is past all of the rsi, rdi, rdx, rcx, r8, r9 registers, so it starts from the stack, which is the start of your buffer, and then print the g's as hexadecimal interpreted as an 8 byte address.

## Usage
Using this knowledge, we are now able to leak the ret to main from vuln. Looking at the stack, the start of the buffer is pointed to by %6$p, in order to print ret to main from vuln, we have to keep going past 8 byte parameters to print out the ret to main. This is calculated by %[6 + (312 bytes buffer / 8 bytes) + 1 (to go past the 2nd canary) + 1 (to go past the old %rbp)]$p which is %47$p. To ensure that this is the address, we can look at the canary value at %45$p. Canary values are very distinct from stack values, as they end in 00 and look different than other values on the stack. %46$p prints the old %rbp, which is something we will come back to, and %47$p prints the ret addr to main. 

GDBing vuln, we see that that address of that without the randomised offset is 0x12ad. So then the adderss when running the actual %47$p is given by this equation: Output of %47$p = [randomised offset] + 0x12ad. So now in order to get the randomised offset, you just subtract 0x12ad from that address. Now with the knowledge of the base address offset, we are able to know EXACTLY where every single procedure in this program are. PIE defeated!

## To be continued maybe


