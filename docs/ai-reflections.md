# AI Reflection Log

Each team member writes a short personal entry (~100–200 words) every sprint.

---

## Sprint 2

### Selena (Yang) Zhang

I mainly used Claude (in the claude.ai app) this sprint. I used it to plan how to split the skeleton into files, to generate the first version of the C skeleton (`net.c`, `http.c`, `main.c`, the thread pool stubs), and to help check our bibliography citations.

What helped most: it mapped each source file to a role in our sequence diagrams, which made the code easier for me to explain. It also pulled up the real nginx and httpd source, and while doing that we noticed something we'd missed: nginx does *not* block when its thread pool queue is full, it just returns an error. That ended up in our Research Extension Update.

Where it went wrong: its first summary of httpd said the pool "doubles threads" each round, but after reading the actual source it turned out httpd doubles *child processes*, not threads. Also, its own test script had a bug at first (it read an old log file and fired requests before the server was up), so it looked like the server was broken when it wasn't. And it couldn't verify one of our book citations, so we're replacing that source. Main lesson: I still have to check everything against the real source myself.

### Minyuan Wang

<!-- 100–200 words: which tools, for what; where AI helped; where it was wrong or didn't help. If you didn't use AI, explain why. -->

### Yao Qin

<!-- 100–200 words: which tools, for what; where AI helped; where it was wrong or didn't help. If you didn't use AI, explain why. -->
