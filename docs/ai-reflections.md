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

I used ChatGPT and Claude this sprint, mainly to understand the concurrency ideas behind our server, like the producer-consumer model, bounded queues, and how worker threads get scheduled, and to think through how we should evaluate AdaptivePool.

What helped most was talking through how different pool sizes might behave under different workloads. It pushed me to look beyond throughput and see why p99 latency, queueing delay, and average thread count matter for an adaptive pool. It also helped me understand why our pool grows fast under queue pressure but shrinks slowly when idle.

Where it went wrong: AI sometimes made performance explanations sound more convincing than our evidence supported. In our Sprint 1 exploration, our CPU workload used wall-clock busy waiting, which looked reasonable but counted time spent waiting for the CPU as real work, so the results were misleading. We had to go back to the code and redefine the workload ourselves. My takeaway is that AI is good for exploring ideas, but it can’t replace carefully designed experiments and our own reasoning about the results.
