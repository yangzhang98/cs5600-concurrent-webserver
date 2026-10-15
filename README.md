# AdaptivePool: Concurrent Web Server with an Adaptive Thread Pool

CS 5600 term project, OSTEP Project G.5 (Concurrent Web Server).

We are building a multi-threaded web server in C, using a master thread, a bounded request buffer and a pool of worker threads (producer/consumer), with FIFO and Smallest-File-First scheduling. Our research extension, **AdaptivePool**, resizes the worker pool at runtime based on queue length. We will evaluate it against fixed-size pools under steady and bursty synthetic traffic.

## Contribution Tracker

**[Contribution tracker (Excel / Google Sheet)](PASTE-SHAREABLE-LINK-HERE)**. Shared with the professor and TAs. A starting template is in `docs/contribution_tracker.xlsx`.

## Sprint 1 Deliverables

| Deliverable | Location |
|---|---|
| Team bio | [`docs/team_bio.pdf`](docs/team_bio.pdf) |
| Executive summary & research proposal | [`docs/proposal.pdf`](docs/proposal.pdf) |
| Exploration report (3 existing systems) | [`docs/exploration.md`](docs/exploration.md), screenshots in `docs/images/` |
| System architecture (3 UML sequence diagrams) | [`docs/architecture/sequence_diagrams.pdf`](docs/architecture/sequence_diagrams.pdf), PNGs + Mermaid sources in `docs/architecture/` (extension sketches in `docs/architecture/extension_cases/`) |

## Repository Layout

```
README.md
docs/
  team_bio.pdf
  proposal.pdf
  exploration.md
  images/                 screenshots from our VM
  architecture/           3 sequence diagrams (.mmd, .png) + combined PDF
    extension_cases/      diagrams for possible extensions
  src/                    Markdown sources for the PDFs
  contribution_tracker.xlsx
exploration/              test apps + load script used in the exploration report
  flask_app/  express_app/  springboot_app/  run_load.sh
src/                      our web server (Sprint 2 onward)
```

## Team

- Yang (Selena) Zhang
- [Teammate 2]
- [Teammate 3]
