# HTTPd16

Yeah, it’s a *new* 16-bit web server for Windows 3.11... so what?!

Look, I didn’t put a ton of effort into this, so please don’t go digging through the code looking for elegance. This project exists because I genuinely enjoy retro-development. It’s the constraints that make it interesting: limited memory, practically no modern documentation (good luck Googling anything), and yet we still expect things like JSON support in 2025.

There’s something fun about pushing old operating systems to do things they were never meant to. The last time I went down this road, I spent a few hundred bucks just to throw 8MB of RAM at a problem. Totally worth it.

One often-forgotten challenge is memory allocation. Even if you have 16MB free, good luck accessing it conventionally—malloc() and free() just don’t cut it. You can use GlobalAlloc and lock memory, but that’s about as fast as Christmas morning when you're 80. So in this build, I included my own memory manager that grabs a configurable chunk of RAM and hands it out in paged segments. Honestly, that part was a blast.

Anyway, more may come. Or not. But hey—enjoy the weirdness.

![image](https://user-images.githubusercontent.com/11428567/172197547-ef3dac79-c197-4922-8b35-37132caa0ac5.png)
