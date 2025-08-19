#include "document.h"
#include <list>
#include <string>

Document::Document(){
    this->title = "No Title";
    this->description = "No Description";
    this->url = "No Url";
    this->links = list<string>();
    this->body = "No Body";

}
Document::Document(string title, string description, string url, list<string> links, string body){
    this->title = title;
    this->description = description;
    this->url = url;
    this->links = links;
    this->body = body;
    find_docSize(body);
}
bool Document::operator<(const Document& other) const {
        return this->url < other.url;  // Use URL as a unique identifier for ordering
    }

void Document::find_docSize(const string& full_file){
    for(char c: full_file){
        if(!isspace(c)){
            this->doc_size++;
        }
    }
}
string Create_snippet(const string& body){
    
}