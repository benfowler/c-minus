Overview
========

[c-minus][1] is a ridiculously trivial compiler project, which was originally 
written for a university subject (Queensland University of Technology's course
ITB464 Modern Compiler Construction).  It implements the C-Minus language, 
which is the simplest possible conventional imperative language that does
something interesting (recursion, arrays, iteration, decision -- that's it)

Here's what a C-Minus program might look like:

    /*
     * A program to perform Euclid's algorithm to computer
     *  the greatest common denominator (GCD) of a pair of integers. 
     */
    
    int gcd(int u, int v)
    {
        if (v == 0) return u ;
        else return gcd(v,u-u/v*v);
        /* u-u/v*v == u mod v */
    }
    
    void main(void)
    {   int x; int y;
        x = input(); y = input();
        output(gcd(x,y));
    }
    

Presently, it implements a single DCODE back end, which emits the intermediate
representation used by QUT's proprietary Gardens Point Modula suite of 
compilers.


To Do
=====

* Implement a different back end so it can generate executables on 
  contemporary machines; e.g. LLVM, MSIL.


Support
=======

Supplied as-is.  If it breaks, you get to keep both pieces.


Example Usage
=============

A few examples are below, but you my also find example usage in the 
'/example' and '/test' directories.


[1]: https://github.com/benfowler/c-minus

