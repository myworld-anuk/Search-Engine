#include <list>
#include <string>
#include <vector>
#include <set>

/*

*/
using namespace std;
class Document
{
    private:
        string title;
        string description;
        string url;
        list<string> links;
        string body;
        double page_score = 0.0;
        set<Document> back_links;
        double back_link_score = 0.0;
        int doc_size = 0;

    public:
        Document();
        Document(string title, string description, string url, list<string> links, string body);
        string getTitle() const { return title; }
        string getDescription() const { return description; }
        string getUrl() const { return url; }
        list<string> getLinks() const { return links; }
        string getBody() const { return body; }
        void set_back_links(set<Document> back_links){ this-> back_links = back_links;}
        void setBackLinkScore(double backScore){this-> back_link_score = backScore;}
        set<Document> get_back_links() const{return back_links;}
        bool operator<(const Document& other) const;
        void find_docSize(const string& full_file);
        int getDocSize()const {return this->doc_size;}
        void setPage_Score(double page_score){ this-> page_score = page_score;}

};