#ifndef SEARCH_H
#define SEARCH_H

#include <string>
#include <unordered_map>
#include <map>
#include <set>
#include <list>
#include <vector>
#include <algorithm>

class Search 
{
public:
    // Constructor
    Search();
    
    // ===== WEB CRAWLER COMPONENT =====
    void crawl(const std::string& seedURL);
    std::list<std::string> extractLinksFromHTML(const std::string& fileContent);
    std::string extractTitle(const std::string& fileContent);
    std::string extractDescription(const std::string& fileContent);
    std::string extractSnippet(const std::string& filePath, const std::string& query);
    std::string extractBodyContent(const std::string& fileContent);
    bool endsWithHtml(const std::string& path);
    void crawlAllDirectories(const std::string& directoryPath);
    
    // ===== QUERY PROCESSING COMPONENT =====
    void processQueries(const std::string& inputFilePath);
    std::vector<std::pair<std::string, double> > search(const std::string& query, bool isPhraseSearch);
    
    // ===== PAGE RANKING COMPONENT =====
    size_t countAllCharactersInHTML(const std::string& htmlContent);
    int countExactOccurrences(const std::string& text, const std::string& keyword);
    double calculateGlobalKeywordDensity(const std::string& keyword, bool isPhraseSearch);
    double calculateKeywordDensityScore(const std::string& url, const std::vector<std::string>& keywords, bool isPhraseSearch);
    double calculateBacklinksScore(const std::string& url);
    double calculateScore(const std::string& url, const std::vector<std::string>& keywords, bool isPhraseSearch);
    
    std::string createSnippet(const std::unordered_map<std::string, std::string>& documentContents, const std::string& url, const std::string& query, bool isPhraseSearch);
    
private:
    // New member to track seed directory for absolute path resolution
    std::string seedDirectory;
    
    // Helper methods
    void crawlURL(const std::string& url, int depth);
    std::string normalizePath(const std::string& basePath, const std::string& relativePath);
    
    // Document data
    std::map<std::string, std::string> documentFullContent;
    std::unordered_map<std::string, std::string> documentTitles; 
    std::unordered_map<std::string, std::string> documentDescriptions;
    std::unordered_map<std::string, std::string> documentContents;
    std::unordered_map<std::string, std::set<std::string>> documentOutgoingLinks;
    std::unordered_map<std::string, size_t> documentContentLengths;
    
    // Link graph data
    std::unordered_map<std::string, std::set<std::string>> incomingLinks;
    std::set<std::string> crawledURLs;
    
    // Word statistics
    std::unordered_map<std::string, int> wordCounts;
    size_t totalDocumentLength;
};

#endif // SEARCH_H