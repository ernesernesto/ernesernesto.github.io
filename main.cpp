#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include <vector>

#include <curl/curl.h>
#include "pugixml/pugixml.cpp"

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

typedef struct 
{
    char* data;
    size_t size;
} Buffer;

typedef struct 
{
    const char* domain;
    const char* title;
    const char* url;
    long dateSecs;

    size_t processedEntryLength;
} PostEntry;

#define TEST 0
#define WRITE_TO_FILE 0
#define MAX_PARALLEL 32 

#if ENABLE_DEBUG
#define LOG(...) printf(__VA_ARGS__)
#else
#define LOG(...)
#endif

// Note: only use post from 3 months prior
#define DURATION 24*3600*90
#define ENTRIES_COUNT 512

static char* urls[] = 
{
#if 0
#endif
    // working
#if 1
    "http://deplinenoise.wordpress.com/feed/",
    "http://feeds.feedburner.com/C0de517e/",
    "http://randomascii.wordpress.com/feed/",
    "http://mollyrocket.com/casey/stream_atom.rss",
    "http://feeds.feedburner.com/codinghorror/",
    "http://danluu.com/atom.xml",
    "http://fgiesen.wordpress.com/feed/",
    "http://fabiensanglard.net/rss.xml",
    "http://feeds.lia-sae.net/main.rss.xml",
    "http://molecularmusings.wordpress.com/feed/",
    "http://reedbeta.com/feed/",
    "http://blog.demofox.org/feed/",
    "http://tomforsyth1000.github.io/blog.wiki.xml",
    "http://www.codersnotes.com/rss/",
    "https://floooh.github.io/feed.xml",
    "https://aras-p.info/atom.xml",
    "https://www.jendrikillner.com/index.xml",
    "https://bartwronski.com/feed/",
    "https://ourmachinery.com/index.xml",
    "http://code4k.blogspot.com/feeds/posts/default",
    "http://casual-effects.blogspot.com/feeds/posts/default",
    "http://cbloomrants.blogspot.com/feeds/posts/default",
    "http://codecapsule.com/feed/",
    "http://realtimecollisiondetection.net/blog/?feed=rss2",
    "http://blog.yiningkarlli.com/feeds/posts/default/",
    "https://flashypixels.wordpress.com/feed/",
    "http://blog.regehr.org/feed",
    "http://eev.ee/feeds/atom.xml",
    "http://donw.io/index.xml",
    "http://www.decarpentier.nl/feed",
    "http://www.iryoku.com/feed",
    "http://blogs.msdn.com/marcelolr/rss.xml",
    "https://tuxedolabs.blogspot.com/feeds/posts/default?alt=rss",
    "http://sonnati.wordpress.com/feed/",
    "http://www.gijskaerts.com/wordpress/?feed=rss2",
    "http://kayru.org/feed.xml",
    "http://zeuxcg.org/feed/",
    "https://halisavakis.com/category/technically-art/feed/",
    "https://0fps.net/feed/",
    "https://anteru.net/rss.xml",
    "https://www.tobias-franke.eu/log/rss/index.xml",
    "https://bitbashing.io/feed.xml",
    "https://www.forceflow.be/feed/",
    "http://playtechs.blogspot.com/feeds/posts/default",
    "https://www.wihlidal.com/feed.xml",
    "http://www.forwardscattering.org/rss",
    "http://filmicworlds.com/feed.xml",
    "https://computingandrecording.wordpress.com/feed/",
    "http://www.thetenthplanet.de/feed",
    "https://arvid.io/rss/",
    "http://www.sebastiansylvan.com/index.xml",
    "https://agraphicsguynotes.com/posts/index.xml",
    "http://accidentallyquadratic.tumblr.com/rss",
    "https://devblogs.microsoft.com/oldnewthing/feed",
    "https://www.realtimerendering.com/blog/feed/",
    "https://interplayoflight.wordpress.com/feed/",
    "https://preshing.com/feed",
#endif
};

char* htmlPrefix = 
"<!DOCTYPE html>\n"
"<html>\n"
"   <head>\n"
"       <meta charset=\"utf-8\" />\n"
"       <meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge,chrome=1\" />\n"
"       <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\" />\n"
"       <link rel=\"stylesheet\" type=\"text/css\" href=\"styles.css\" media=\"screen\" />\n"
"       <style>\n"
"       @import url(\"https://fonts.googleapis.com/css2?family=Nanum+Myeongjo&display=swap\");\n"
"       body\n"
"       {\n"
"           font-family: \"Nanum Myeongjo\", serif;\n"
"           line-height: 1.7;\n"
"           max-width: 600px;\n"
"           margin: 0px 0px 0px 50px;\n"
"           padding: 0 12px 0;\n"
"           height: 100%;\n"
"       }\n"
"       @media (prefers-color-scheme: dark)\n"
"       {\n"
"           body\n"
"           {\n"
"               background-color: hsl(0, 0%, 27%);\n"
"               color: hsl(0, 0%, 86.7%);\n"
"           }\n"
"           a { color: hsl(0, 0%, 86.7%); }\n"
"           a:visited { color: hsl(212, 22%, 46%); }\n"
"       }\n"
"       li { padding-bottom: 16px; }\n"
"       </style>\n"
"       <title>Simple RSS dump page</title>\n"
"   </head>\n"
"   <body>\n"
"       <h1>Read</h1>\n"
"       <ol class=\"postlist\">)";

char* htmlPostfix = 
"       </ol>\n"
"       <footer>\n"
"           <p><a href=\"https://github.com/ernesernesto/ernesernesto.github.io\">source</a></p>\n"
"       </footer>\n"
"   </body>\n"
"</html>";

static PostEntry* entries;
static Buffer buffers[ENTRIES_COUNT];
static pugi::xml_document documents[ENTRIES_COUNT];

size_t OnWriteData(void* data, size_t size, size_t nmemb, void* userData)
{
    size_t fullSize = size * nmemb;
    Buffer* buffer = (Buffer*)userData;

    char* ptr = (char*)realloc(buffer->data, buffer->size + fullSize + 1);
    if(ptr)
    {
        buffer->data = ptr;
        memcpy(&(buffer->data[buffer->size]), data, fullSize);
        buffer->size += fullSize;
        buffer->data[buffer->size] = 0;
    }
    else
    {
        printf("Failed to allocate memory\n");
    }

    return fullSize;
}

void CurlAdd(CURLM* multiHandle, char* url, int index)
{
    Buffer* buffer = &buffers[index];

    CURL* handle = curl_easy_init();
    //curl_easy_setopt(handle, CURLOPT_VERBOSE, 1);
    curl_easy_setopt(handle, CURLOPT_URL, url);
    curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, OnWriteData);
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, buffer);
    curl_easy_setopt(handle, CURLOPT_PRIVATE, index);
    curl_multi_add_handle(multiHandle, handle);
}

void BeginReadFile(FILE* file, Buffer* buffer)
{
    fseek(file, 0, SEEK_END);
    size_t fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    buffer->size = fileSize;
    buffer->data = (char*)malloc(buffer->size + 1);

    fread(buffer->data, buffer->size, 1, file);
    buffer->data[buffer->size] = 0;
}

void EndReadFile(FILE* file, Buffer* buffer)
{
    free(buffer->data);
    fclose(file);
}

time_t GetTimeFromString(bool isFeed, const char* str)
{
    // TODO: this is wonky, needs to be fixed to handle different format
    struct tm tm;
    char buf[256];

    memset(&tm, 0, sizeof(struct tm));
    if(isFeed)
    {
        //Note: 2020-12-27T14:03:00.010-08:00
        strptime(str, "%Y-%m-%dT%H:%M:%S", &tm);
        strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S", &tm);
    }
    else
    {
        //Note: Wed, 29 Aug 2018 07:00:00 +0000
        strptime(str, "%a, %d %b %Y %H:%M:%S", &tm);
        strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S", &tm);
    }
    time_t result = mktime(&tm);
    LOG("%ld\n", result);
    return result;
}

void StripWhitespace(char* src, char* dest)
{
    char* begin = src;
    while(*begin == '\n' || *begin == '\t')
    {
        begin++; 
    }

    char* tmp = begin;
    int test = 0;
    while(*tmp != '\n' && *tmp != 0)
    {
        tmp++; 
    }

    size_t count = tmp - begin;
    strncpy(dest, begin, count);
    dest[count] = 0;
}

void ParseBufferToRSS(pugi::xml_document* doc, Buffer* buffer, long currentTime)
{
    pugi::xml_parse_result parseResult = doc->load_buffer(buffer->data, buffer->size);
    if(parseResult)
    {
        pugi::xml_node root = doc->child("rss");
        if(root)
        {
            pugi::xml_node channel = root.child("channel");
            const char* domain = channel.child_value("link");
            LOG("domain %s\n", domain);

            for(pugi::xml_node item = channel.child("item"); item; item = item.next_sibling("item"))
            {
                const char* title = item.child_value("title");
                const char* link = item.child_value("link");
                const char* pubDate = item.child_value("pubDate");

                LOG("%s %s %s\n", title, link, pubDate);

                char time[128];
                StripWhitespace(const_cast<char*>(pubDate), time);
                time_t secEpoch = GetTimeFromString(false, time);

                long diffTime = currentTime - secEpoch;
                LOG("%ld\n", diffTime);
                if(diffTime <= DURATION)
                {
                    PostEntry postEntry = {};
                    postEntry.domain = domain;
                    postEntry.title = title;
                    postEntry.url = link;
                    postEntry.dateSecs = secEpoch;
                    arrput(entries, postEntry);
                    
                    LOG("added\n");
                }
            }
        }
        else
        {
            root = doc->child("feed");

            const char* domain = 0;
            for(pugi::xml_node link = root.child("link"); link; link = link.next_sibling("link"))
            {
                if(strcmp(link.attribute("rel").value(), "alternate") == 0)
                {
                    domain = link.attribute("href").value();
                    LOG("domain %s\n", domain);
                    break;
                }
            }

            if(!domain)
            {
                //! Note: iterate one more, find any link on the feed for domain link
                for(pugi::xml_node link = root.child("link"); link; link = link.next_sibling("link"))
                {
                    if(strcmp(link.attribute("rel").value(), "self") != 0)
                    {
                        domain = link.attribute("href").value();
                        break;
                    }
                }
            }

            LOG("domain %s\n", domain);

            //printf("%s\n", root.child_value("title"));
            for(pugi::xml_node entry = root.child("entry"); entry; entry = entry.next_sibling("entry"))
            {
                const char* title = entry.child_value("title");
                const char* pubDate = entry.child_value("published");

                if(strlen(pubDate) == 0)
                {
                    pubDate = entry.child_value("updated");
                    LOG("published %s\n", pubDate);
                }

                pugi::xml_node findLink;
                for(pugi::xml_node link = entry.child("link"); link; link = link.next_sibling("link"))
                {
                    if(strcmp(link.attribute("rel").value(), "alternate") == 0)
                    {
                        findLink = link;
                        break;
                    }
                }
                
                const char* link = 0;
                if(findLink)
                {
                    link = findLink.attribute("href").value();
                }
                else
                {
                    link = entry.child("link").attribute("href").value();
                }

                LOG("%s %s %s\n", title, link, pubDate);

                char time[128];
                StripWhitespace(const_cast<char*>(pubDate), time);
                time_t secEpoch = GetTimeFromString(true, time);

                long diffTime = currentTime - secEpoch;
                LOG("%ld\n", diffTime);
                if(diffTime <= DURATION)
                {
                    PostEntry postEntry = {};
                    postEntry.domain = domain;
                    postEntry.title = title;
                    postEntry.url = link;
                    postEntry.dateSecs = secEpoch;
                    arrput(entries, postEntry);

                    LOG("added\n");
                }
            }
        }
    }
}

void SwapPostEntry(int a, int b)
{
    const char* domainTmp = entries[a].domain;
    const char* titleTmp = entries[a].title;
    const char* urlTmp = entries[a].url;
    long dateSecsTmp = entries[a].dateSecs;

    entries[a].domain = entries[b].domain;
    entries[a].title = entries[b].title;
    entries[a].url = entries[b].url;
    entries[a].dateSecs = entries[b].dateSecs;

    entries[b].domain = domainTmp;
    entries[b].title = titleTmp;
    entries[b].url = urlTmp;
    entries[b].dateSecs = dateSecsTmp;
}

void GetFilePath(char* src, char* dest, int prefixCount)
{
    size_t count = strlen(src);
    char* tmp = src;

    char buf[128];
    strncpy(buf, src, count);
    buf[count] = 0;

    tmp = buf;
    while(*tmp != 0)
    {
        if(*tmp == '.' || *tmp == ':' || *tmp == '/')
        {
            *tmp = '_';
        }
        tmp++; 
    }
    sprintf(dest, "./test/%s.xml", buf);
}

int main(int, char**)
{
    struct timeval start;
    gettimeofday(&start, NULL);

    long currentTime = start.tv_sec;
    printf("%ld time now secs\n", currentTime);
    curl_global_init(CURL_GLOBAL_ALL);

    CURLM* multiHandle = curl_multi_init();
    curl_multi_setopt(multiHandle, CURLMOPT_MAXCONNECTS, (long)MAX_PARALLEL);

    int entryCount = 0;
    int urlsCount = sizeof(urls)/sizeof(urls[0]);
    for(int index = 0; index < urlsCount; ++index)
    {
        char* url = urls[index];
        pugi::xml_document* doc = &documents[entryCount];

#if TEST
        Buffer* buffer = &buffers[entryCount];

        char filePath[128];
        GetFilePath(url, filePath, 7);
        FILE* file = fopen(filePath, "r");

        if(file)
        {
            BeginReadFile(file, buffer);
            ParseBufferToRSS(doc, buffer, currentTime);
            //printf("%s\n", buffer.data);
            EndReadFile(file, buffer);
        }
#else
        CurlAdd(multiHandle, url, entryCount);
#endif

        ++entryCount;
    }

    int alive = 1;
    int msgsLeft = -1;

    CURLMsg* response;
    do 
    {
        curl_multi_perform(multiHandle, &alive);

        while((response = curl_multi_info_read(multiHandle, &msgsLeft))) 
        {
            if(response->msg == CURLMSG_DONE) 
            {
                int index;
                CURL* handle = response->easy_handle;
                curl_easy_getinfo(handle, CURLINFO_PRIVATE, &index);

                char* url = urls[index];
                printf("%s\n", url);

                pugi::xml_document* doc = &documents[index];
                Buffer* buffer = &buffers[index];
                ParseBufferToRSS(doc, buffer, currentTime);

                //printf("%s\n", buffer->data);
#if WRITE_TO_FILE
                char filePath[128];
                GetFilePath(url, filePath, 7);
                FILE* file = fopen(filePath, "w" );
                fwrite(buffer->data, 1, buffer->size, file);
                fclose(file);
#endif
        
                curl_multi_remove_handle(multiHandle, handle);
                curl_easy_cleanup(handle);
            }
            else 
            {
                fprintf(stderr, "E: CURLMsg (%d)\n", response->msg);
            }
        }

        if(alive)
        {
            curl_multi_wait(multiHandle, NULL, 0, 500, NULL);
        }
    } 
    while(alive);

    curl_multi_cleanup(multiHandle);
    curl_global_cleanup();

    int count = arrlen(entries);
    printf("Registered entries %d\n", count);
    for(int i = 0 ; i < count - 1; ++i)
    {
        for(int j = 0 ; j < count - i - 1; j++)
        {
            if(entries[j].dateSecs < entries[j+1].dateSecs)
            {
                SwapPostEntry(j, j+1);
            }
        }
    }

    size_t prefixLength = strlen(htmlPrefix);
    size_t postFixLength = strlen(htmlPostfix);

    size_t entryLength = 0;

    char* linkFormat = "<li><a href=\"%s\" target=\"_blank\" rel=\"noopener noreferrer\">%s</a> (%s)</li>\n";
    for(int index = 0; index < count; ++index)
    {
        PostEntry* entry = &entries[index];
        size_t currentEntryLength = snprintf(NULL, 0, linkFormat, entry->url, entry->title, entry->domain);
        entry->processedEntryLength = currentEntryLength;

        entryLength += currentEntryLength; 
    }

    size_t totalLength = prefixLength + entryLength + postFixLength;
    char* output = (char*)malloc(totalLength + 1);
    memmove(output, htmlPrefix, prefixLength);

    entryLength = 0;
    for(int index = 0; index < count; ++index)
    {
        PostEntry* entry = &entries[index];
        size_t currentEntryLength = snprintf(output + prefixLength + entryLength, entry->processedEntryLength, linkFormat, entry->url, entry->title, entry->domain);
        output[prefixLength + entryLength - 1] = '\n';

        entryLength += currentEntryLength;
    }

    memmove(output + prefixLength + entryLength - 1, htmlPostfix, postFixLength);
    output[totalLength - 1] = 0;
    LOG("%s\n", output);

    FILE* file = fopen("./read/index.html", "w" );
    fwrite(output, 1, totalLength, file);
    fclose(file);

    struct timeval end;
    gettimeofday(&end, NULL);
    double timeSpent = end.tv_sec + end.tv_usec / 1e6 - start.tv_sec - start.tv_usec / 1e6; // in seconds
    printf("Time taken %f seconds\n", timeSpent);

    return 0;
}
