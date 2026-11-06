# AI Reflection Log

Each team member writes a short personal entry (~100–200 words) every sprint.

---

## Sprint 2

### Selena (Yang) Zhang

I mainly used Claude (in the claude.ai app) this sprint. I used it to plan how to split the skeleton into files, to generate the first version of the C skeleton (`net.c`, `http.c`, `main.c`, the thread pool stubs), and to help check our bibliography citations.

What helped most: it mapped each source file to a role in our sequence diagrams, which made the code easier for me to explain. It also pulled up the real nginx and httpd source, and while doing that we noticed something we'd missed: nginx does *not* block when its thread pool queue is full, it just returns an error. That ended up in our Research Extension Update.

Where it went wrong: its first summary of httpd said the pool "doubles threads" each round, but after reading the actual source it turned out httpd doubles *child processes*, not threads. Also, its own test script had a bug at first (it read an old log file and fired requests before the server was up), so it looked like the server was broken when it wasn't. And it couldn't verify one of our book citations at first, so we went back to the actual PDF and checked the edition and chapter ourselves. Main lesson: I still have to check everything against the real source myself.

### Minyuan Wang

   I used ChatGPT and Claude this sprint, mostly to look up papers for our bibliography, to think through how our code should be structured, and to help write some of the code.

   Where it helped most was designing the main function for our skeleton. Talking it through with AI gave me a starting point for how the server should be laid out, which saved time compared to starting from a blank file.

   Where it went wrong: when we were designing the main function, some of what AI suggested didn't match the structure our team had already agreed on, so we ended up not using it. AI doesn't know how our team wants the pieces to fit together, so I couldn't just take its code as is and had to bring in my own thinking for each function. In the end, AI was a helper, but the main logic and design ideas came from the three of us working together.

### Yao Qin

<!-- 100–200 words: which tools, for what; where AI helped; where it was wrong or didn't help. If you didn't use AI, explain why. -->
