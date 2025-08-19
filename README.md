# Search-Engine

Objectives:
1) Write recursive programs
2) Use std::map and std::set

Project Capabilities:
This program mimics the three core functions of a search engine, but operates on a local folder of HTML files instead of the live internet.

1) Web Crawling: Starting from a single "seed" HTML file, your program recursively navigated through hyperlinks to discover all other linked local HTML files. During this process, it built an inverted index—a map where each key is a word (e.g., "Tom") and its value is a list of all the documents containing that word.

2) Query Searching: The program reads search queries from an input file and can perform two types of searches:
     - Regular Search (e.g., Tom Cruise): It found documents that contained all the specified words, regardless of their order or proximity.
     - Phrase Search (e.g., "Tom Cruise"): It found documents containing the exact phrase, with the words in the specified order.

3) Page Ranking: For the documents that matched a query, your program ranked them to decide which was most relevant. The ranking was based on a page score calculated from two factors:
     - Keywords Density Score: Measured how frequently the search terms appeared in a specific document compared to their frequency across all crawled documents.
     - Backlinks Score: Gave a higher score to pages that were linked to by other pages, especially if those other pages had few outgoing links themselves.

Finally, it generated a formatted output file for each query, listing the results in descending order of their page score. Each result included the document's title, URL, description, and a 120-character snippet of text relevant to the search query.

Skills Gained: 

1) Recursion: Required to use recursion to implement the web crawler. This involved writing a function that could call itself to follow links from one page to the next, building a complete map of the local site structure.
2) STL Data Structures: The project heavily relied on the ability to choose and use appropriate C++ Standard Template Library containers.
     - std::map: to create the inverted index, mapping string keywords to a list or set of documents. Maps were also useful for storing metadata about each crawled page (like its content, title, or backlink score).
     - std::set: You probably used std::set to keep track of which URLs had already been visited by your crawler, preventing it from getting stuck in an infinite loop.
3) String Manipulation: I performed extensive string processing, including:
     - Searching for words and phrases within the text of HTML files.
     - Enforcing search rules like case-sensitivity and word boundaries (ensuring "Tom" doesn't match "Tomato").
     - Parsing file content to extract specific information like the title, description, and text snippets using functions like find, rfind, and substr.
4) Algorithm Design: You designed and implemented a multi-step algorithm: crawl, then search, then rank. This involved calculating complex scores based on the specific formulas provided for keyword density and backlinks.
5) File I/O: Your program handled reading from multiple files—the query list and numerous HTML files—and writing formatted output to a new set of result files.
