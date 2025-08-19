#include "search.h"
#include <dirent.h>  // Include for directory traversal
#include <sys/types.h>
#include <sys/stat.h>    // for stat()
#include <dirent.h>      // for opendir(), readdir(), closedir()
#include <cstring> // Include for string manipulation
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <regex>
#include <cmath>

Search::Search() : totalDocumentLength(0) {}

// ===================================================
// UTILITY FUNCTIONS
// ===================================================

// Check if a word exists at a specific position in text with word boundaries
bool wordExistsAtPosition(const std::string& text, size_t pos, const std::string& word) 
{
    size_t wordLen = word.length();
    // Check if we're at a valid position
    if (pos + wordLen > text.length()) return false;
    
    // Check if the word matches
    if (text.substr(pos, wordLen) != word) return false;
    
    // Check if the character before is a word boundary
    bool validBefore = (pos == 0) || !std::isalnum(text[pos - 1]);
    
    // Check if the character after is a word boundary
    bool validAfter = (pos + wordLen == text.length()) || !std::isalnum(text[pos + wordLen]);
    
    return validBefore && validAfter;
}

// Returns the position of the first occurrence of 'word' in 'text' (case-insensitive)
// such that both the character before and after 'word' are NOT alphanumeric (or are
// the beginning/end of the string). Returns std::string::npos if not found.
size_t findWord(const std::string& text, const std::string& word, size_t startPos = 0)
{
    // Convert both text and word to lowercase for case-insensitive matching
    std::string lowerText = text;
    std::string lowerWord = word;
    std::transform(lowerText.begin(), lowerText.end(), lowerText.begin(), ::tolower);
    std::transform(lowerWord.begin(), lowerWord.end(), lowerWord.begin(), ::tolower);

    // Search for occurrences of lowerWord in lowerText
    size_t pos = startPos;
    while (true) {
        pos = lowerText.find(lowerWord, pos);
        if (pos == std::string::npos) {
            // No more occurrences
            return std::string::npos;
        }

        // Check the character before pos
        bool validBefore = (pos == 0) ||
            !std::isalnum(static_cast<unsigned char>(lowerText[pos - 1]));

        // Check the character after pos + word.length() - 1
        size_t afterPos = pos + lowerWord.size();
        bool validAfter = (afterPos >= lowerText.size()) ||
            !std::isalnum(static_cast<unsigned char>(lowerText[afterPos]));

        if (validBefore && validAfter) {
            // Found a standalone occurrence
            return pos;
        }

        // Otherwise, continue searching from the next position
        pos += 1;
    }
}

// Find a phrase in text with word boundaries
size_t findPhrase(const std::string& text, const std::string& phrase, size_t startPos = 0)
{
    // Convert both text and phrase to lowercase for case-insensitive matching
    std::string lowerText = text;
    std::string lowerPhrase = phrase;
    std::transform(lowerText.begin(), lowerText.end(), lowerText.begin(), ::tolower);
    std::transform(lowerPhrase.begin(), lowerPhrase.end(), lowerPhrase.begin(), ::tolower);

    // Search for occurrences of lowerPhrase in lowerText
    size_t pos = startPos;
    while (true) {
        pos = lowerText.find(lowerPhrase, pos);
        if (pos == std::string::npos) {
            // No more occurrences
            return std::string::npos;
        }

        // Check the character before pos
        bool validBefore = (pos == 0) ||
            !std::isalnum(static_cast<unsigned char>(lowerText[pos - 1]));

        // Check the character after pos + phrase.length() - 1
        size_t afterPos = pos + lowerPhrase.size();
        bool validAfter = (afterPos >= lowerText.size()) ||
            !std::isalnum(static_cast<unsigned char>(lowerText[afterPos]));

        if (validBefore && validAfter) {
            // Found a standalone phrase occurrence
            return pos;
        }

        // Otherwise, continue searching from the next position
        pos += 1;
    }
}

// ===================================================
// WEB CRAWLER COMPONENT
// ===================================================

std::list<std::string> Search::extractLinksFromHTML(const std::string& fileContent) 
{
    std::list<std::string> links;
    
    // Regular expression to match href attributes in anchor tags
    std::regex linkRegex("<a\\s+[^>]*href\\s*=\\s*['\"]([^'\"]+)['\"][^>]*>");
    std::smatch match;

    // Search for links in the HTML content
    std::string::const_iterator start = fileContent.cbegin();
    while (std::regex_search(start, fileContent.cend(), match, linkRegex)) 
    {
        if (match.size() > 1) 
        {
            std::string link = match[1].str();
            links.push_back(link);
        }
        start = match.suffix().first;
    }

    return links;
}

int extractFileNumber(const std::string& url) {
    std::regex fileNumberRegex("file(\\d+)\\.html");
    std::smatch match;
    if (std::regex_search(url, match, fileNumberRegex) && match.size() > 1) {
        return std::stoi(match[1].str());
    }
    return -1; // Return -1 if no file number is found
}

// Function to extract title from HTML content
std::string Search::extractTitle(const std::string& fileContent) 
{
    size_t start = fileContent.find("<title>");
    size_t end = fileContent.find("</title>");
    std::smatch match;
    if (start != std::string::npos && end != std::string::npos) 
    {
        return fileContent.substr(start + 7, end - (start + 7));
    }
    return "No Title";
}

// Function to extract description from HTML content
std::string Search::extractDescription(const std::string& fileContent) 
{
    std::regex metaRegex("<meta\\s+name=\"description\"\\s+content=\"([^\"]+)\"");
    std::smatch match;
    if (std::regex_search(fileContent, match, metaRegex))
    {
        return match[1];
    }
    return "No Description";
}


// Function to extract body content from HTML
std::string Search::extractBodyContent(const std::string& fileContent) 
{
    // Locate the <body> and </body> tags.
    size_t bodyStart = fileContent.find("<body");
    if (bodyStart != std::string::npos) {
        bodyStart = fileContent.find(">", bodyStart);
        if (bodyStart != std::string::npos) {
            bodyStart++;  // move past the ">"
        }
    }
    size_t bodyEnd = fileContent.find("</body>");
    
    // If both tags are found, use the content in between; otherwise, use the entire file.
    std::string bodyContent;
    if (bodyStart != std::string::npos && bodyEnd != std::string::npos) {
        bodyContent = fileContent.substr(bodyStart, bodyEnd - bodyStart);
    } else {
        bodyContent = fileContent;
    }

    // Manually remove HTML tags while preserving all whitespace and line breaks.
    std::string result;
    bool inTag = false;
    for (char c : bodyContent) {
        if (c == '<') {
            inTag = true;
            continue; // Do not include the tag-start character.
        } else if (c == '>') {
            inTag = false;
            continue; // Do not include the tag-end character.
        }
        if (!inTag) {
            result.push_back(c);  // Append every character as-is.
        }
    }

    // Return the result without any further whitespace collapsing.
    return result;
}

std::string Search::createSnippet(
    const std::unordered_map<std::string, std::string>& documentContents,
    const std::string& url,
    const std::string& query,
    bool isPhraseSearch)
{
    // 1) Use only the already‐extracted <body> text.
    if (documentContents.find(url) == documentContents.end()) {
        return "URL not found in document contents.";
    }
    const std::string& bodyText = documentContents.at(url);

    // 2) Determine the target string (handling phrase vs. single/multiple words).
    std::string target;
    if (isPhraseSearch) {
        size_t startQuote = query.find('"');
        size_t endQuote   = query.rfind('"');
        if (startQuote != std::string::npos && endQuote != std::string::npos && startQuote < endQuote) {
            target = query.substr(startQuote + 1, endQuote - (startQuote + 1));
        } else {
            target = query;
        }
    } else {
        target = query;
    }

    // 3) Find a candidate sentence that contains an exact match to 'target'.
    //    (Essentially the same logic you have now.)
    size_t searchPos = 0;
    std::string candidateSentence;
    bool foundCandidate = false;

    while (true) {
        size_t pos = bodyText.find(target, searchPos);
        if (pos == std::string::npos) {
            break;  // No more occurrences
        }

        // Find the beginning of the sentence by looking for the preceding period.
        size_t sentenceStart = 0;
        if (pos != 0) {
            sentenceStart = bodyText.rfind('.', pos);
            if (sentenceStart == std::string::npos) {
                sentenceStart = 0;
            } else {
                sentenceStart++;  // move past the period
            }
        }

        // Skip any whitespace right after the period.
        while (sentenceStart < bodyText.size() &&
               std::isspace(static_cast<unsigned char>(bodyText[sentenceStart])))
        {
            sentenceStart++;
        }

        // Find the end of the sentence by looking for the next period.
        size_t sentenceEnd = bodyText.find('.', pos);
        if (sentenceEnd == std::string::npos) {
            sentenceEnd = bodyText.size();
        } else {
            sentenceEnd++;  // include the period
        }

        candidateSentence = bodyText.substr(sentenceStart, sentenceEnd - sentenceStart);

        // Reject candidate if it appears to be a file URL (starts with "html_files/").
        if (candidateSentence.rfind("html_files/", 0) == 0) {
            searchPos = pos + 1;
            continue;
        }

        // Check that the candidate sentence contains an exact match (valid word boundaries).
        bool exactMatchFound = false;
        size_t matchIndex = candidateSentence.find(target);
        while (matchIndex != std::string::npos) {
            bool validBefore = (matchIndex == 0) ||
                               !std::isalnum(static_cast<unsigned char>(candidateSentence[matchIndex - 1]));
            size_t endPos = matchIndex + target.size();
            bool validAfter = (endPos == candidateSentence.size()) ||
                              !std::isalnum(static_cast<unsigned char>(candidateSentence[endPos]));
            if (validBefore && validAfter) {
                exactMatchFound = true;
                break;
            }
            matchIndex = candidateSentence.find(target, matchIndex + 1);
        }

        if (exactMatchFound) {
            foundCandidate = true;
            break;
        }
        searchPos = pos + 1;
    }

    // 4) If no valid candidate sentence was found, fallback to the first 200 chars.
    std::string snippet;
    if (!foundCandidate) {
        snippet = bodyText.substr(0, std::min<size_t>(200, bodyText.size()));
    } else {
        snippet = candidateSentence;
    }

    // 5) If snippet is longer than 120, truncate.
    if (snippet.size() > 120) {
        snippet = snippet.substr(0, 120);
    }
    // 6) If snippet is shorter than 120, try to append more characters
    //    from where that snippet ended in the original body text.
    else if (snippet.size() < 120) 
    {
        // Where does this snippet begin in the body?
        size_t snippetStartPos = bodyText.find(snippet);
        if (snippetStartPos != std::string::npos) {
            // How many extra characters do we need?
            size_t needed = 120 - snippet.size();
            // We want to add from bodyText starting at snippetStartPos + snippet.size().
            size_t nextPos = snippetStartPos + snippet.size();
            if (nextPos < bodyText.size()) {
                // Append up to 'needed' characters, or until the end of bodyText.
                size_t canAppend = std::min(needed, bodyText.size() - nextPos);
                snippet += bodyText.substr(nextPos, canAppend);
            }
        }
    }

    // 7) Now we have a snippet that may or may not be exactly 120. 
    //    If it's still too long, truncate again; if it's still too short, 
    //    we can’t do much more except pad with spaces (or just leave it).
    if (snippet.size() > 120) 
    {
        snippet = snippet.substr(0, 120);
    } 
    else if (snippet.size() < 120) 
    {
        // If you want to pad at the very end, do so:
        while (snippet.size() < 120) 
        {
            snippet.push_back(' ');
        }
    }

    // 8) Remove only leading whitespace (not trailing).
    //    We'll find the first non-whitespace character and slice from there.
    {
        size_t idx = 0;
        while (idx < snippet.size() && std::isspace(static_cast<unsigned char>(snippet[idx]))) 
        {
            idx++;
        }
        if (idx > 0 && idx < snippet.size()) 
        {
            snippet = snippet.substr(idx);
        }
        // If removing leading spaces caused snippet to become shorter than 120,
        // and you want to preserve exactly 120, you can optionally re-append
        // some text from the body or pad with spaces again.
        // For simplicity, let's do a quick re-check:
        if (snippet.size() < 120) 
        {
            size_t needed = 120 - snippet.size();
            snippet.append(needed, ' ');
        }
    }

    return snippet;
}

// Helper: determine directory depth after "html_files/"
int directoryDepth(const std::string& url) 
{
    size_t pos = url.find("html_files/");
    if (pos == std::string::npos) return 0;
    std::string remainder = url.substr(pos + std::string("html_files/").length());
    // Count the number of '/' characters in remainder
    int depth = 0;
    for (char c : remainder) {
        if (c == '/') depth++;
    }
    return depth;
}

// Update the normalizePath function to handle absolute paths relative to the seed directory:
std::string Search::normalizePath(const std::string& basePath, const std::string& relativePath) {
    // Make a copy of the relative path and trim leading/trailing whitespace.
    std::string rel = relativePath;
    while (!rel.empty() && std::isspace(rel.front())) {
        rel.erase(rel.begin());
    }
    while (!rel.empty() && std::isspace(rel.back())) {
        rel.pop_back();
    }
    
    // Convert to lowercase for case-insensitive file systems.
    std::transform(rel.begin(), rel.end(), rel.begin(), ::tolower);
    
    // If the relative path is actually absolute (starts with '/'),
    // treat it as relative to the seed directory.
    if (!rel.empty() && rel[0] == '/') {
        std::string absPath = seedDirectory + rel;
        // Now remove the seedDirectory prefix so that rel becomes relative.
        rel = absPath.substr(seedDirectory.length());
    }
    
    // Extract the base directory from basePath.
    std::string baseDir = basePath.substr(0, basePath.find_last_of('/') + 1);
    
    // Split the (now relative) path into components using '/' as delimiter.
    std::vector<std::string> components;
    std::istringstream iss(rel);
    std::string token;
    while (std::getline(iss, token, '/')) {
        // Trim whitespace from each token.
        token.erase(token.begin(), std::find_if(token.begin(), token.end(), [](int ch) {
            return !std::isspace(ch);
        }));
        token.erase(std::find_if(token.rbegin(), token.rend(), [](int ch) {
            return !std::isspace(ch);
        }).base(), token.end());
        
        if (token.empty() || token == ".")
            continue;
        if (token == "..") {
            if (!components.empty())
                components.pop_back();
        } else {
            components.push_back(token);
        }
    }
    
    // Rebuild the final path from the base directory and the components.
    std::string result = baseDir;
    for (const std::string &comp : components) {
        if (result.empty() || result.back() != '/')
            result += '/';
        result += comp;
    }
    
    // Collapse any multiple consecutive slashes into one.
    result = std::regex_replace(result, std::regex("/{2,}"), "/");
    
    // If the result ends with a slash, treat it as a directory and append "index.html".
    if (!result.empty() && result.back() == '/') {
        result += "index.html";
    } else {
        // Otherwise, check if there's an extension.
        size_t lastDot = result.find_last_of('.');
        size_t lastSlash = result.find_last_of('/');
        if (lastDot == std::string::npos || (lastSlash != std::string::npos && lastDot < lastSlash)) {
            // No extension found. Check if it's a directory.
            struct stat st;
            if (stat(result.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) {
                if (result.back() != '/')
                    result += '/';
                result += "index.html";
            } else {
                // Not a directory so assume it's an HTML file.
                result += ".html";
            }
        } else {
            // If there is an extension but it isn’t ".html", try to use an HTML version if it exists.
            std::string extension = result.substr(lastDot);
            std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
            if (extension != ".html") {
                std::string htmlVersion = result.substr(0, lastDot) + ".html";
                struct stat st;
                if (stat(htmlVersion.c_str(), &st) == 0) {
                    result = htmlVersion;
                }
            }
        }
    }
    
    return result;
}

// Helper to do a simple case-insensitive ends-with(".html")
bool Search::endsWithHtml(const std::string& path) {
    // Trim trailing spaces just in case
    std::string trimmed = path;
    while (!trimmed.empty() && std::isspace((unsigned char)trimmed.back())) {
        trimmed.pop_back();
    }
    // Now check if it ends with .html or .HTML, ignoring case
    if (trimmed.size() < 5) return false;
    std::string ext = trimmed.substr(trimmed.size() - 5);
    // Convert to lowercase
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    return (ext == ".html");
}


// Main web crawling function - entry point for crawler
void Search::crawl(const std::string& seedURL) 
{
    
    // Clear any previous crawl data
    crawledURLs.clear();
    documentTitles.clear();
    documentDescriptions.clear();
    documentContents.clear();
    documentContentLengths.clear();
    documentOutgoingLinks.clear();
    incomingLinks.clear();
    wordCounts.clear();
    totalDocumentLength = 0;
    
    // Start crawling from the seed URL
    crawlURL(seedURL, 0);
    
    // After crawling, build word counts using documentFullContent
    for (std::set<std::string>::iterator it = crawledURLs.begin(); it != crawledURLs.end(); ++it) 
    {
        const std::string& url = *it;
        if (documentFullContent.find(url) != documentFullContent.end()) 
        {
            std::string content = documentFullContent[url];
            std::stringstream ss(content);
            std::string word;
            while (ss >> word) {
                wordCounts[word]++;
            }
        }
    }
}

// Recursive crawler function that follows links
void Search::crawlURL(const std::string& url, int depth) {
    // Check if we've already crawled this URL
    if (crawledURLs.find(url) != crawledURLs.end()) {
        return;
    }
    
    // Try to open the file
    std::ifstream file(url);
    if (!file.is_open()) 
    {
        return;
    }
    
    // Mark this URL as crawled
    crawledURLs.insert(url);
    
    // Read the file content
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
    
    // Extract and store document information
    std::string title = extractTitle(content);
    std::string description = extractDescription(content);
    std::string bodyContent = extractBodyContent(content);
    size_t contentLength = content.length();
    
    documentFullContent[url] = content; // Add this line
    documentTitles[url] = title;
    documentDescriptions[url] = description;
    documentContents[url] = bodyContent;
    documentContentLengths[url] = contentLength;
    totalDocumentLength += contentLength;
    
    // Extract links from the document
    std::list<std::string> links = extractLinksFromHTML(content);
    
    // Initialize outgoing links set if needed
    if (documentOutgoingLinks.find(url) == documentOutgoingLinks.end()) {
        documentOutgoingLinks[url] = std::set<std::string>();
    }
    
    // Process each link
    for (std::list<std::string>::iterator it = links.begin(); it != links.end(); ++it) {
        std::string link = *it;
        
        // Skip external links.
        if (link.find("://") != std::string::npos) {
            continue;
        }
        
        // Normalize the link.
        std::string newPath = normalizePath(url, link);
        
        // std::cerr << "DEBUG: Normalizing path." << std::endl
        //           << "DEBUG: Base Path: " << url << std::endl
        //           << "DEBUG: Relative Path: " << link << std::endl
        //           << "DEBUG: Normalized Path: " << newPath << std::endl;
        
        // Add to outgoing links only if not linking to self.
        if (newPath != url) {
            documentOutgoingLinks[url].insert(newPath);
        }
        
        // Add to incoming links for the target.
        if (incomingLinks.find(newPath) == incomingLinks.end()) {
            incomingLinks[newPath] = std::set<std::string>();
        }
        incomingLinks[newPath].insert(url);
        
        // Crawl the linked file if it's HTML.
        if (newPath.size() > 5 && newPath.substr(newPath.size() - 5) == ".html") {
            crawlURL(newPath, depth + 1);
        }
    }
    
    // After processing links, print the final outgoing links for this file.
    // std::cerr << "DEBUG: Outgoing links for " << url << ":" << std::endl;
    // for (const auto& outLink : documentOutgoingLinks[url]) {
    //     std::cerr << "    " << outLink << std::endl;
    // }    
    
}

// ===================================================
// QUERY PROCESSING COMPONENT
// ===================================================

// Process queries from input file and generate output files
// Process queries from input file and generate output files
void Search::processQueries(const std::string& inputFilePath) 
{
    std::ifstream inputFile(inputFilePath);
    if (!inputFile.is_open()) 
    {
        return;
    }
    
    std::string query;
    int queryIndex = 1;
    while (std::getline(inputFile, query)) 
    {
        bool isPhraseSearch = query.find('"') != std::string::npos;
        std::vector<std::pair<std::string, double>> results = search(query, isPhraseSearch);
        
        std::ofstream outputFile("out" + std::to_string(queryIndex) + ".txt");
        if (!outputFile.is_open()) 
        {
            continue;
        }
        
        if (results.empty()) {
            outputFile << "Your search - " << query << " - did not match any documents.\n";
        } 
        else 
        {
            outputFile << "Matching documents: " << std::endl << std::endl;
            for (std::vector<std::pair<std::string, double>>::const_iterator it = results.begin(); it != results.end(); ++it) 
            {
                const std::string& url = it->first;
                
                //double score = it->second;
                
                std::string title = documentTitles[url];
                std::string description = documentDescriptions[url];
                
                // Create the snippet using our new snippet function that follows the assignment rules.
                std::string snippet = createSnippet(documentContents, url, query, isPhraseSearch);
                // Force the snippet to exactly 120 characters.
                if (snippet.length() > 120) {
                    snippet = snippet.substr(0, 120);
                }
                
                outputFile << "Title: " << title << std::endl;
                outputFile << "URL: " << url << std::endl;
                //outputFile << "Score: " << score << std::endl; // Debug: print score
                outputFile << "Description: " << description << std::endl;
                if(it != results.end() - 1) 
                {
                    outputFile << "Snippet: " << snippet << std::endl << std::endl;
                } 
                else 
                {
                    outputFile << "Snippet: " << snippet << std::endl;
                }
            }
        }
        
        outputFile.close();
        queryIndex++;
    }
    
    inputFile.close();
}

// ===================================================
// PAGE RANKING COMPONENT
// ===================================================

size_t Search::countAllCharactersInHTML(const std::string& htmlContent)
{
    // Simply return the total number of characters in the raw HTML string.
    // This counts every character, including whitespace, tags, newlines, etc.
    return htmlContent.size();
}


// Helper: Count the number of exact occurrences (case sensitive)
// of 'keyword' in 'text' with valid word boundaries.
int Search::countExactOccurrences(const std::string& text, const std::string& keyword) 
{
    if (keyword.empty()) return 0; // avoid infinite loop on empty keyword
    int count = 0;
    size_t pos = 0;
    while ((pos = text.find(keyword, pos)) != std::string::npos) 
    {
        // Check for a valid word boundary before the keyword.
        bool validBefore = (pos == 0) || !std::isalnum(static_cast<unsigned char>(text[pos - 1]));
        // Check for a valid word boundary after the keyword.
        size_t endPos = pos + keyword.size();
        bool validAfter = (endPos == text.size()) || !std::isalnum(static_cast<unsigned char>(text[endPos]));
        
        if (validBefore && validAfter) 
        {
            count++;
            pos += keyword.size(); // Move past this valid occurrence.
        } 
        else 
        {
            pos++; // Not a valid match; move one character forward.
        }
    }
    return count;
}

double Search::calculateGlobalKeywordDensity(const std::string& keyword, bool isPhraseSearch) 
{
    // Split the keyword into words if this is a phrase search.
    std::vector<std::string> words;
    if (isPhraseSearch) {
        std::istringstream iss(keyword);
        std::string w;
        while (iss >> w) {
            words.push_back(w);
        }
    } else {
        words.push_back(keyword);
    }
    
    double combinedDensity = 0.0;
    // For each word, iterate over all documents (using raw HTML from documentFullContent)
    for (const std::string& w : words) {
        int totalOccurrences = 0;
        size_t totalLength = 0;
        for (std::unordered_map<std::string, std::string>::const_iterator it = documentContents.begin(); it != documentContents.end(); ++it) {
            const std::string& rawHTML = it->second;
            totalOccurrences += countExactOccurrences(rawHTML, w);
            totalLength += countAllCharactersInHTML(rawHTML);
        }
        double density = 0.0;
        if (totalLength != 0) {
            density = static_cast<double>(totalOccurrences) / totalLength;
        }
        combinedDensity += density;
    }
    return combinedDensity;
}

double Search::calculateKeywordDensityScore(const std::string& url, const std::vector<std::string>& keywords, bool isPhraseSearch) {
    // Use raw HTML for density calculations.
    if (documentFullContent.find(url) == documentFullContent.end()) {
        return 0.0;
    }
    
    const std::string& rawHTML = documentFullContent.at(url);
    size_t docLength = countAllCharactersInHTML(rawHTML);
    double score = 0.0;
    
    // Build a vector of words to score.
    std::vector<std::string> words;
    if (isPhraseSearch && keywords.size() == 1) {
        // Split the single phrase into individual words.
        std::istringstream iss(keywords[0]);
        std::string w;
        while (iss >> w) {
            words.push_back(w);
        }
    } else {
        // Otherwise, each keyword is used as given.
        words = keywords;
    }
    
    for (const std::string& w : words) 
    {
        int occurrences = countExactOccurrences(rawHTML, w);
        double globalDensity = calculateGlobalKeywordDensity(w, false);  
        // Here, for each individual word we set isPhraseSearch to false because we already split.
        double component = 0.0;
        if (globalDensity > 0 && docLength > 0) {
            component = static_cast<double>(occurrences) / (docLength * globalDensity);
        }
        score += component;
        
        // Debug print for each word component.
        // std::cerr << "Document: " << url << std::endl
        //           << "Length: " << docLength << std::endl
        //           << "Occurrences: " << occurrences << std::endl
        //           << occurrences << " / (" << docLength << " * " << globalDensity << ") = " 
        //           << component << std::endl
        //           << std::endl;
    }
    
    return score;
}

double Search::calculateBacklinksScore(const std::string& url) 
{
    double score = 0.0;
    if (incomingLinks.find(url) != incomingLinks.end()) {
        for (const std::string& backlink : incomingLinks.at(url)) {
            size_t outgoingLinkCount = 0;
            if (documentOutgoingLinks.find(backlink) != documentOutgoingLinks.end()) {
                outgoingLinkCount = documentOutgoingLinks.at(backlink).size();
            }
            // Use the actual outgoing link count (do not cap it)
            score += 1.0 / (1.0 + outgoingLinkCount);
        }
    }
    return score;
}


double Search::calculateScore(const std::string& url, const std::vector<std::string>& keywords, bool isPhraseSearch) 
{
    double keywordDensityScore = calculateKeywordDensityScore(url, keywords, isPhraseSearch);
    double backlinksScore = calculateBacklinksScore(url);
    double finalScore = 0.5 * keywordDensityScore + 0.5 * backlinksScore;
    
    // Build a joined query string for snippet display.
    std::string joinedQuery;
    if (isPhraseSearch && keywords.size() == 1) 
    {
        joinedQuery = keywords[0];
    } 
    else 
    {
        for (size_t i = 0; i < keywords.size(); i++) 
        {
            if (i > 0) joinedQuery += " ";
            joinedQuery += keywords[i];
        }
    }
    std::string snippet = createSnippet(documentContents, url, joinedQuery, isPhraseSearch);
    
    return finalScore;
}

// ===================================================
// SEARCH FUNCTION
// ===================================================

// Search for documents matching the query containing ANY of the keywords
std::vector<std::pair<std::string, double>> Search::search(const std::string& query, bool isPhraseSearch) 
{
    std::vector<std::pair<std::string, double>> results;
    
    if (isPhraseSearch) {
        // Extract phrase from between quotes
        size_t startQuote = query.find('"');
        size_t endQuote = query.rfind('"');
        if (startQuote != std::string::npos && endQuote != std::string::npos && startQuote != endQuote) 
        {
            std::string phrase = query.substr(startQuote + 1, endQuote - startQuote - 1);

            // For phrase search, only include documents that contain the exact phrase
            for (const std::string& url : crawledURLs) {
                if (documentFullContent.find(url) == documentFullContent.end())
                    continue;
                std::string fullContent = documentFullContent[url];
                if (findPhrase(fullContent, phrase) != std::string::npos) {
                    // Treat the entire phrase as a single keyword for scoring purposes
                    std::vector<std::string> phraseAsKeyword;
                    phraseAsKeyword.push_back(phrase);
                    double score = calculateScore(url, phraseAsKeyword, isPhraseSearch);
                    results.push_back({url, score});
                }
            }
        }
    } 
    else 
    {
        // For regular (non-phrase) search, split query into individual keywords.
        std::vector<std::string> keywords;
        std::stringstream ss(query);
        std::string word;
        while (ss >> word) 
        {
            keywords.push_back(word);
        }
        
        // Only include documents where ALL keywords (as standalone words) are found.
        for (const std::string& url : crawledURLs) {
            if (documentFullContent.find(url) == documentFullContent.end()) continue;
            std::string fullContent = documentFullContent[url];
            bool allFound = true;
            for (const std::string& keyword : keywords) {
                if (findWord(fullContent, keyword) == std::string::npos) {
                    allFound = false;
                    break;
                }
            }
            if (allFound) {
                double score = calculateScore(url, keywords, isPhraseSearch);
                results.push_back({url, score});
            }
        }
    }
    
    // Sort the results (the comparator code remains unchanged)

    std::sort(results.begin(), results.end(),
    [](const std::pair<std::string, double>& a,
       const std::pair<std::string, double>& b) 
    {
        return a.second > b.second;
    });

    
    return results;
}

// ===================================================
// MAIN FUNCTION
// ===================================================
int main(int argc, char** argv) 
{
    // Command line format: ./nysearch.exe html_files/index.html input.txt
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <seed_file> <query_file>" << std::endl;
        return 1;
    }
    
    try {
        std::string seedFile = argv[1]; // e.g. "html_files/index.html"
        std::string inputFilePath = argv[2];
        
        // Extract the directory from the seed file.
        size_t pos = seedFile.rfind('/');
        std::string seedDir;
        if (pos != std::string::npos) {
            seedDir = seedFile.substr(0, pos);
        } else {
            seedDir = ".";
        }
        
        // Create search engine instance.
        Search searchEngine;
        
        // Crawl all .html files in the seed directory.
        searchEngine.crawl(seedFile);
        
        // Process search queries.
        searchEngine.processQueries(inputFilePath);
        
    } catch (const std::bad_alloc& e) {
        std::cerr << "Memory allocation error: " << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}