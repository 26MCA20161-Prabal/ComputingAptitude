#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cctype>
#include <map>
#include <limits>
#include <vector>

using namespace std;

// -----------------------------------------------------------
// Class: Email
// Holds the data for a single email.
// -----------------------------------------------------------
class Email {
private:
    string sender;
    string subject;
    string body;
    int spamScore;   // computed by SpamDetector
    bool spam;        // final verdict

public:
    Email(const string& s, const string& subj, const string& b)
        : sender(s), subject(subj), body(b), spamScore(0), spam(false) {}

    string getSender()  const { return sender; }
    string getSubject() const { return subject; }
    string getBody()    const { return body; }
    string getFullText() const { return subject + " " + body; }

    int  getScore() const { return spamScore; }
    bool isSpam()   const { return spam; }

    // Called by SpamDetector through a pointer to this object.
    void setResult(int score, bool spamVerdict) {
        spamScore = score;
        spam = spamVerdict;
    }

    void display() const {
        cout << "From    : " << sender << "\n";
        cout << "Subject : " << subject << "\n";
        cout << "Verdict : " << (spam ? "SPAM" : "HAM")
             << "  (score = " << spamScore << ")\n";
    }
};

// -----------------------------------------------------------
// Class: SpamDetector
// Scores an email's text against a weighted keyword list and
// decides SPAM / HAM against a threshold.
// -----------------------------------------------------------
class SpamDetector {
private:
    map<string, int>* keywordWeights;   // pointer to a dynamically allocated map
    int spamThreshold;

    static string toLower(const string& text) {
        string result = text;
        for (char& c : result) c = static_cast<char>(tolower(static_cast<unsigned char>(c)));
        return result;
    }

public:
    SpamDetector(int threshold = 5) : spamThreshold(threshold) {
        // Allocate the keyword table on the heap to demonstrate
        // pointer-managed dynamic memory inside a class.
        keywordWeights = new map<string, int>();

        (*keywordWeights)["free"]        = 3;
        (*keywordWeights)["win"]         = 3;
        (*keywordWeights)["winner"]      = 3;
        (*keywordWeights)["lottery"]     = 4;
        (*keywordWeights)["prize"]       = 3;
        (*keywordWeights)["urgent"]      = 2;
        (*keywordWeights)["click"]       = 2;
        (*keywordWeights)["click here"]  = 4;
        (*keywordWeights)["cash"]        = 3;
        (*keywordWeights)["credit"]      = 2;
        (*keywordWeights)["loan"]        = 2;
        (*keywordWeights)["offer"]       = 2;
        (*keywordWeights)["discount"]    = 2;
        (*keywordWeights)["guarantee"]   = 2;
        (*keywordWeights)["congratulations"] = 3;
        (*keywordWeights)["password"]    = 2;
        (*keywordWeights)["verify"]      = 2;
        (*keywordWeights)["account"]     = 1;
        (*keywordWeights)["suspended"]   = 3;
        (*keywordWeights)["million"]     = 3;
        (*keywordWeights)["money"]       = 2;
        (*keywordWeights)["act now"]     = 3;
        (*keywordWeights)["limited time"]= 2;
        (*keywordWeights)["buy now"]     = 2;
    }

    ~SpamDetector() {
        delete keywordWeights;   // free heap memory in the destructor
        keywordWeights = nullptr;
    }

    // Computes a weighted keyword score for a block of text.
    int computeScore(const string& text) const {
        string lowerText = toLower(text);
        int score = 0;

        for (const auto& entry : *keywordWeights) {
            const string& keyword = entry.first;
            int weight = entry.second;

            size_t pos = lowerText.find(keyword);
            while (pos != string::npos) {
                score += weight;
                pos = lowerText.find(keyword, pos + keyword.size());
            }
        }

        // Extra signal: too many exclamation marks is spammy.
        int exclaims = 0;
        for (char c : text) if (c == '!') exclaims++;
        if (exclaims >= 3) score += 2;

        return score;
    }

    // Classifies the email in place via its pointer.
    void classify(Email* mail) const {
        int score = computeScore(mail->getFullText());
        bool verdict = (score >= spamThreshold);
        mail->setResult(score, verdict);
    }
};

// -----------------------------------------------------------
// Linked list node holding a pointer to a dynamically
// allocated Email object.
// -----------------------------------------------------------
struct EmailNode {
    Email* data;
    EmailNode* next;
    EmailNode(Email* e) : data(e), next(nullptr) {}
};

// -----------------------------------------------------------
// Class: EmailList
// A minimal singly linked list built with raw pointers.
// Owns the Email* objects it stores and frees them on destruction.
// -----------------------------------------------------------
class EmailList {
private:
    EmailNode* head;
    EmailNode* tail;
    int count;

public:
    EmailList() : head(nullptr), tail(nullptr), count(0) {}

    ~EmailList() {
        EmailNode* current = head;
        while (current != nullptr) {
            EmailNode* toDelete = current;
            current = current->next;
            delete toDelete->data;   // free the Email object
            delete toDelete;         // free the node itself
        }
        head = tail = nullptr;
    }

    void append(Email* e) {
        EmailNode* node = new EmailNode(e);
        if (head == nullptr) {
            head = tail = node;
        } else {
            tail->next = node;
            tail = node;
        }
        count++;
    }

    int size() const { return count; }
    EmailNode* getHead() const { return head; }
};

// -----------------------------------------------------------
// Class: MailFetcher
// Simulates fetching emails by reading a local mailbox file.
// Creates a demo mailbox file automatically if none exists.
// -----------------------------------------------------------
class MailFetcher {
private:
    string filename;

    void createSampleMailbox() const {
        ofstream out(filename);
        out << "lottery@unknown.com|You have WON a free prize!!!|"
               "Congratulations winner! Click here now to claim your free cash prize before it expires.\n";
        out << "alerts@bank-secure.com|URGENT: verify your account|"
               "Your account has been suspended. Verify your password immediately to avoid loss of access.\n";
        out << "hr@college.edu|Meeting reschedule|"
               "Hi team, can we move tomorrow's meeting to 3 PM? Let me know if that works.\n";
        out << "friend@gmail.com|Notes from class|"
               "Hey, here are the notes from today's data structures lecture, let me know if anything is missing.\n";
        out << "deals@shopfast.com|Limited time offer just for you|"
               "Buy now and get huge discount, guarantee lowest price, act now before offer ends!!!\n";
        out << "prof.sharma@college.edu|Assignment deadline reminder|"
               "This is a reminder that your assignment submission deadline is next Monday.\n";
        out.close();
    }

public:
    MailFetcher(const string& file) : filename(file) {}

    // Reads mailbox.txt and returns a dynamically built EmailList.
    // Each line format: sender|subject|body
    EmailList* fetchEmails() {
        ifstream in(filename);
        if (!in.is_open()) {
            cout << "[MailFetcher] " << filename << " not found. Creating a sample mailbox...\n";
            createSampleMailbox();
            in.open(filename);
        }

        EmailList* list = new EmailList();   // caller owns this pointer
        string line;

        while (getline(in, line)) {
            if (line.empty()) continue;

            size_t firstBar = line.find('|');
            size_t secondBar = line.find('|', firstBar + 1);
            if (firstBar == string::npos || secondBar == string::npos) continue;

            string sender  = line.substr(0, firstBar);
            string subject = line.substr(firstBar + 1, secondBar - firstBar - 1);
            string body    = line.substr(secondBar + 1);

            Email* mail = new Email(sender, subject, body);  // heap allocation
            list->append(mail);
        }
        in.close();
        return list;
    }
};

// -----------------------------------------------------------
// Class: EmailFilterSystem
// Orchestrates fetching, classification, reporting, and
// writing filtered output files.
// -----------------------------------------------------------
class EmailFilterSystem {
private:
    SpamDetector detector;
    EmailList* allEmails;   // owns the fetched emails

public:
    EmailFilterSystem(int threshold = 5)
        : detector(threshold), allEmails(nullptr) {}

    ~EmailFilterSystem() {
        delete allEmails;   // EmailList destructor frees every Email* too
    }

    void run(const string& mailboxFile) {
        if (allEmails != nullptr) {
            delete allEmails;   // avoid leaking a previous batch if run() is called again
            allEmails = nullptr;
        }

        MailFetcher fetcher(mailboxFile);
        allEmails = fetcher.fetchEmails();

        cout << "\nFetched " << allEmails->size() << " email(s) from " << mailboxFile << "\n\n";

        classifyAll();
        printReport();
        saveFilteredOutput();
    }

    // Lets the user type a single email at the keyboard and classifies
    // it immediately, without touching mailbox.txt (unless they opt to save it).
    void classifyManualEntry() {
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        string sender, subject, line, body;

        cout << "\nEnter sender email: ";
        getline(cin, sender);

        cout << "Enter subject: ";
        getline(cin, subject);

        cout << "Enter body (type END on its own line to finish):\n";
        while (getline(cin, line)) {
            if (line == "END") break;
            body += line + " ";
        }

        Email* mail = new Email(sender, subject, body);   // heap allocation
        detector.classify(mail);                          // classify via pointer

        cout << "\n--- Classification Result ---\n";
        mail->display();

        cout << "\nSave this email to mailbox.txt for next time? (y/n): ";
        char choice;
        cin >> choice;

        if (choice == 'y' || choice == 'Y') {
            ofstream out("mailbox.txt", ios::app);
            out << sender << "|" << subject << "|" << body << "\n";
            out.close();
            cout << "Saved to mailbox.txt.\n";
        }

        delete mail;   // this email was never added to allEmails, so free it here
    }

    // Reads ONE email from a file and classifies just that email.
    // Supported formats:
    //   1) With headers:
    //        From: someone@example.com
    //        Subject: Free prize for you
    //        <blank line>
    //        body text...
    //   2) Headerless: the whole file is treated as the body, and
    //      sender/subject default to placeholders.
    void classifyFromFile(const string& filepath) {
        ifstream in(filepath);
        if (!in.is_open()) {
            cout << "Could not open file: " << filepath << "\n";
            return;
        }

        vector<string> lines;
        string raw;
        while (getline(in, raw)) {
            if (!raw.empty() && raw.back() == '\r') raw.pop_back();  // strip CRLF
            lines.push_back(raw);
        }
        in.close();

        string sender, subject, body;
        parseEmailLines(lines, sender, subject, body);

        Email* mail = new Email(sender, subject, body);   // heap allocation
        detector.classify(mail);                          // classify via pointer

        cout << "\n--- Classification Result for \"" << filepath << "\" ---\n";
        mail->display();

        cout << "\nSave this email to mailbox.txt for next time? (y/n): ";
        char choice;
        cin >> choice;

        if (choice == 'y' || choice == 'Y') {
            ofstream out("mailbox.txt", ios::app);
            string flatBody = body;
            for (char& c : flatBody) if (c == '\n') c = ' ';
            out << sender << "|" << subject << "|" << flatBody << "\n";
            out.close();
            cout << "Saved to mailbox.txt.\n";
        }

        delete mail;
    }

private:
    // Splits a small set of header lines (From:, Subject:) away from the
    // body. Falls back to treating the whole file as body if no headers
    // are present, so plain unformatted .txt files work too.
    static void parseEmailLines(const vector<string>& lines,
                                 string& sender, string& subject, string& body) {
        size_t i = 0;
        for (; i < lines.size(); ++i) {
            const string& line = lines[i];

            if (line.empty()) { i++; break; }   // blank line ends the header block

            size_t colon = line.find(':');
            if (colon == string::npos) break;    // doesn't look like a header -> body starts here

            string key = line.substr(0, colon);
            string value = line.substr(colon + 1);
            size_t start = value.find_first_not_of(" \t");
            value = (start == string::npos) ? "" : value.substr(start);

            for (char& c : key) c = static_cast<char>(tolower(static_cast<unsigned char>(c)));

            if (key == "from")    sender = value;
            else if (key == "subject") subject = value;
            // any other "Key: value" line is treated as a header and skipped
        }

        for (; i < lines.size(); ++i) body += lines[i] + " ";

        if (sender.empty())  sender = "unknown@unknown.com";
        if (subject.empty()) subject = "(no subject)";
    }

public:

    void classifyAll() {
        EmailNode* current = allEmails->getHead();
        while (current != nullptr) {
            Email* mailPtr = current->data;   // pointer to the email object
            detector.classify(mailPtr);       // classifier writes result via pointer
            current = current->next;
        }
    }

    void printReport() const {
        cout << "================ CLASSIFICATION REPORT ================\n";
        EmailNode* current = allEmails->getHead();
        int index = 1;
        while (current != nullptr) {
            cout << "[" << index++ << "] ";
            current->data->display();
            cout << "---------------------------------------------------------\n";
            current = current->next;
        }
    }

    void saveFilteredOutput() const {
        ofstream inboxOut("inbox_output.txt");
        ofstream spamOut("spam_output.txt");

        int spamCount = 0, hamCount = 0;
        EmailNode* current = allEmails->getHead();

        while (current != nullptr) {
            Email* mail = current->data;
            ofstream& target = mail->isSpam() ? spamOut : inboxOut;

            target << "From: "    << mail->getSender()  << "\n";
            target << "Subject: " << mail->getSubject()  << "\n";
            target << "Score: "   << mail->getScore()    << "\n";
            target << "Body: "    << mail->getBody()      << "\n";
            target << "------------------------------------\n";

            mail->isSpam() ? spamCount++ : hamCount++;
            current = current->next;
        }

        inboxOut.close();
        spamOut.close();

        cout << "Summary: " << spamCount << " spam, " << hamCount << " ham.\n";
        cout << "Results written to inbox_output.txt and spam_output.txt\n";
    }
};

// -----------------------------------------------------------
// main
// -----------------------------------------------------------
int main() {
    cout << "=========================================\n";
    cout << "  Email Content Classification & Filter\n";
    cout << "=========================================\n";

    EmailFilterSystem* system = new EmailFilterSystem(5);  // spam score threshold = 5
    int choice = 0;

    while (choice != 4) {
        cout << "\n----------------- MENU -----------------\n";
        cout << "1. Fetch & classify all emails in mailbox.txt\n";
        cout << "2. Type in a single email to classify\n";
        cout << "3. Classify a single email from a file\n";
        cout << "4. Exit\n";
        cout << "Choice: ";

        cin >> choice;
        if (cin.fail()) {               // handle non-numeric input gracefully
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Please enter 1, 2, 3, or 4.\n";
            continue;
        }

        string path;
        switch (choice) {
            case 1:
                system->run("mailbox.txt");
                break;
            case 2:
                system->classifyManualEntry();
                break;
            case 3:
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << "Enter filename (e.g. email.txt): ";
                getline(cin, path);
                system->classifyFromFile(path);
                break;
            case 4:
                cout << "Exiting.\n";
                break;
            default:
                cout << "Please enter 1, 2, 3, or 4.\n";
                break;
        }
    }

    delete system;
    return 0;
}
