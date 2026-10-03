# Email Content Classification & Filtering System
### MCA Semester 1 — C++ Mini Project

A single-file C++ console program that scores emails as **SPAM** or **HAM**
using a weighted keyword classifier, built with classes and pointers
(a hand-rolled linked list, dynamic memory via `new`/`delete`).

No external libraries, no internet connection, no setup required beyond
a C++ compiler.

## Files in this folder

```
EmailClassifier/
├── EmailClassifier.cpp       <- the entire program (one file)
├── README.md                  <- this file
└── sample_emails/
    ├── spam_example.txt       <- sample email with headers (SPAM)
    └── ham_example.txt        <- sample headerless email (HAM)
```

## How to build

```bash
g++ -std=c++17 -Wall EmailClassifier.cpp -o EmailClassifier
```

(Any C++17-capable compiler works — g++, clang++, or MSVC on Windows.)

## How to run

```bash
./EmailClassifier
```

You'll see a menu:

```
1. Fetch & classify all emails in mailbox.txt
2. Type in a single email to classify
3. Classify a single email from a file
4. Exit
```

### Option 1 — Batch mode
Reads every email from `mailbox.txt` and classifies all of them at once.
If `mailbox.txt` doesn't exist yet, the program creates one automatically
with 6 sample emails (3 spam-like, 3 legitimate), so this works out of
the box with no setup. Results print to the console and are also written
to `inbox_output.txt` and `spam_output.txt`.

`mailbox.txt` format — one email per line:
```
sender|subject|body
```

### Option 2 — Type one email in directly
Prompts you for sender, subject, and body (type `END` on its own line to
finish the body), then classifies just that one email immediately.

### Option 3 — Classify one email from a file
Point it at a file containing a single email (try the two examples in
`sample_emails/`) and it scores just that email. Two formats are
supported:

**With headers:**
```
From: promo@dealsite.com
Subject: Congratulations! You WON a free prize

Click here now to claim your free cash prize, act now!!!
```

**Headerless** (the whole file is treated as the body):
```
Hi, just following up on the report you sent yesterday.
```

Try it with the included samples:
```
Choice: 3
Enter filename (e.g. email.txt): sample_emails/spam_example.txt
```

After classifying, you'll be asked if you want to save that email into
`mailbox.txt` for future batch runs.

## Notes

- Everything runs locally — no real email account, credentials, or
  internet access is needed or used.
- To reset the demo mailbox, just delete `mailbox.txt` and run the
  program again; it will be recreated with the default sample emails.
- Spam threshold is set in `main()`: `new EmailFilterSystem(5)`. Raise
  or lower the `5` to make the classifier stricter or more lenient.
