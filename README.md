# HTTPd16

Yea, a new 16bit webserver for Windows 3.11, so what?!

I've always liked retro-development, I think because of the challenges. The resource constraints, lack of digital documentation (you can’t google much), the modern expectations of UI and configurability (it’s got to support JSON, right?).
Plus I get to push the limits of these older operating systems using resources they never though they’d see, and that’s fun! Last time I did this it lost me a few hundred dollars to throw 8MB of RAM at the problem.

One thing that I don’t think many people know or remember is the memory allocation restraints. Even if you have 16MB free, its not conventionally available via typical memory management APIs such as malloc() and free(). You can use GlobalAlloc and lock memory, but that is slooooow as Christmas.
In this rendition, I have baked in my own memory manager that gobbles up a configurable amount of RAM and doles it out in paged chunks. That alone was pretty fun!

Anyway, likely more to come.

![image](https://user-images.githubusercontent.com/11428567/172197547-ef3dac79-c197-4922-8b35-37132caa0ac5.png)
