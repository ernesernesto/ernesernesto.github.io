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
    "https://www.realtimerendering.com/blog/feed/",
    "https://interplayoflight.wordpress.com/feed/",
    "https://preshing.com/feed",
    "https://howtomarketagame.com/blog/feed/",
#endif
};

char* readmePrefix = 
"+++\n"
"title = \"Reads\"\n"
"+++\n"
"\n"
"This page list a collection of [rss feeds from other dev]({{< relref \"reads.md#other-dev-rsss\" >}}) and conference [talks]({{< relref \"reads.md#talks\" >}}) that I've gathered for a while\n"
"\n"
"## Other Dev RSS's\n"
"Collection of good reads (a.k.a Dirt Cheap Poor Man's RSS Feed), this list is updated automatically everyday with github workflows\n"
"\n";

char* readmePostfix = 
"\n"
"\n"
"## Talks\n"
"Curation of talks that I've collected for years which I found useful and worth to watch in no particular order. \n"
"\n"
"### Art\n"
"- [A Live Art Demonstration of Creating Worlds through Design Thinking](https://gdcvault.com/play/1021752/)\n"
"- [Animation Style and Process for Broken Age](https://www.youtube.com/watch?v=iWEVY4ujyI4)\n"
"- [GuiltyGearXrd's Art Style - The X Factor Between 2D and 3D](https://www.youtube.com/watch?v=yhGjCzxJV3E)\n"
"- [Making Fluid and Powerful Animations For Skullgirls](https://www.youtube.com/watch?v=Mw0h9WmBlsw)\n"
"- [Maximizing Artistic Critique - Improving Communication for Everyone Involved in Critical Feedback](https://www.youtube.com/watch?v=5ZDls8L5ONE)\n"
"- [The Animation Process Of Ori & The Blind Forest](https://www.youtube.com/watch?v=m8lOwrWNbEY)\n"
"- [The Art of Reanimating Plants vs. Zombies 2](https://www.youtube.com/watch?v=aqPQepp60L0)\n"
"\n"
"### Game Design\n"
"- [Board Game Design Day - Board Game Design and the Psychology of Loss Aversion](https://www.youtube.com/watch?v=F_1YcCcBVfY)\n"
"- [Breaking Conventions with The Legend of Zelda - Breath of the Wild](https://www.youtube.com/watch?v=QyMsF31NdNc)\n"
"- [Building Games That Can Be Understood at a Glance](https://www.youtube.com/watch?v=YISKcRDcDJg)\n"
"- [Clash of Clans - Designing Games That People Will Play For Years](https://www.youtube.com/watch?v=X7MRK0R0qAU)\n"
"- [Doubling Day 7 Retention _ Layton HAWKES](https://www.youtube.com/watch?v=dnSmV-nOK2A)\n"
"- [Hearthstone - 10 Bits Of Design Wisdom](https://www.youtube.com/watch?v=pyjDMPTgxxk)\n"
"- [Idle Games - The Mechanics and Monetization of Self-Playing Games-Lu](https://www.youtube.com/watch?v=Lu-RjxeDpU8)\n"
"- [Level Design Workshop - Designing Celeste](https://www.youtube.com/watch?v=4RlpMhBKNr0)\n"
"- [Dynamics - The State of the Art](https://www.gdcvault.com/play/1014597/Dynamics-The-State-of-the)\n"
"- [Ten principles for good level design](https://www.youtube.com/watch?v=iNEe3KhMvXM)\n"
"- [Making a Standard (and Trying to Stick to it!) - Blizzard Design Philosophies](https://www.youtube.com/watch?v=FhC0NaB6ock)\n"
"- [Nier's Taro Yoko  'Making Weird Games For Weird People'](https://www.youtube.com/watch?v=OO_d3fwTNPo)\n"
"- [Platinum Games - Action Without Borders](https://www.youtube.com/watch?v=wrw2IP6pZu0)\n"
"- [Poly Bridge and Social Media - A Love Story](https://www.youtube.com/watch?v=HsmYmUD3VoI)\n"
"- [Returners and Retention - How to Win Back Lapsed Players](https://www.youtube.com/watch?v=3LnrzbqX_Xc)\n"
"- [Magic: the Gathering: Twenty Years, Twenty Lessons Learned](https://www.youtube.com/watch?v=QHHg99hwQGY)\n"
"- [Cultist Simulator: Designing an Experimental Game for Commercial Success](https://www.youtube.com/watch?v=0pBvMIUk1nQ)\n"
"- [Designing Games for Game Designers](https://www.gdcvault.com/play/1015305/Designing-Games-for-Game)\n"
"- [One Page Designs](https://www.gdcvault.com/play/1012356/One-Page)\n"
"- [Of Choice and Breaking New Ground - Designing Mark of The Ninja](https://www.gdcvault.com/play/1017791/Of-Choice-and-Breaking-New)\n"
"- [Prototyping Your Monetization: Evaluating Monetization Potential Early](https://www.gdcvault.com/play/1023951/Prototyping-Your-Monetization-Evaluating-Monetization)\n"
"- [Jamie Gilbertson & David Petry - Designing and Implementing an Engaging PvP Multiplayer Experience](https://www.youtube.com/watch?v=P_T_Hl9H3l4)\n"
"- [Combat Design of 'God of War: Ragnarök' - Rob Meyer](https://www.youtube.com/watch?v=6iTBqcBv5QA)\n"
"- [Designing 'MARVEL SNAP'- Ben Brode](https://www.gdcvault.com/play/1029024/Designing-MARVEL-SNAP)\n"
"\n"
"### Narrative\n"
"- [Creating Strong Video Game Characters](https://www.youtube.com/watch?v=4mgK2hL33Vw)\n"
"- [Dynamic Storytelling in The Novelist](https://www.youtube.com/watch?v=4ie1NrENMaI)\n"
"- [Mata Haggis – Non-verbal Storytelling Tools for Narrative Designers [Game Happens! 2016]](https://www.youtube.com/watch?v=Q2ibl55vzIg)\n"
"- [Writing 'Nothing' - Storytelling with Unsaid Words and Unreliable Narrators](https://www.youtube.com/watch?v=LPkbAMj-xVA)\n"
"- [Storytelling Tools to Boost Your Indie Game's Narrative and Gameplay](https://www.youtube.com/watch?v=8fXE-E1hjKk)\n"
"- [The Future of Storytelling - How Medium Shapes Story-BjrO](https://www.youtube.com/watch?v=BjrO-di22v8)\n"
"- [Konsoll 2018: Charlene Putney - Delightful Dilemmas: Writing For Choice-Based Narratives](https://www.youtube.com/watch?v=sX0cuILjcRE)\n"
"\n"
"### Production\n"
"- [Stop Shouting! Collaboration Through Candid Conversation](https://www.youtube.com/watch?v=q7bdDUk7UBM)\n"
"- [The Art of Pre-Production](https://www.youtube.com/watch?v=s2u4jhpZkTQ)\n"
"- [You Suck at Showcasing Your Game](https://www.youtube.com/watch?v=gB8CvsOjDvg)\n"
"- [SIEGE 2013: You're Responsible](https://www.youtube.com/watch?v=xNjfruknpaQ)\n"
"- [Why Dark Souls Is The 'Ikea' Of Games](https://www.youtube.com/watch?v=vid5yZRKzs0)\n"
"- [Being glue - Tanya Reilly](https://www.youtube.com/watch?v=KClAPipnKqw)\n"
"- [Get Over Yourself: Making Someone Else's Game](https://www.gdcvault.com/play/1015649/Get-Over-Yourself-Making-Someone)\n"
"- [Low Level Emotional Programming](https://guide.handmade-seattle.com/c/2021/low-level-emotional-programming/)\n"
"\n"
"### UX\n"
"- [Game UX Summit ’17 _ Alex Neonakis Naughty Dog _ How UX Can Spearhead your Accessibility Push](https://www.youtube.com/watch?v=HFcRwY2gj-8)\n"
"- [Game UX Summit ’17 _ David Candland Bungie _ We’ll fix it in UI](https://www.youtube.com/watch?v=LnxZxQpDsJg)\n"
"- [Hearthstone - How to Create an Immersive User Interface](https://www.youtube.com/watch?v=axkPXCNjOh8)\n"
"- [Juice it or lose it - a talk by Martin Jonasson & Petri Purho](https://www.youtube.com/watch?v=Fy0aCDmgnxg)\n"
"- [The Gamer's Brain - How Neuroscience and UX Can Impact Design](https://www.youtube.com/watch?v=XIpDLa585ao)\n"
"- [Game Feel - Why Your Death Animation Sucks](https://www.youtube.com/watch?v=pmSAG51BybY)\n"
"- [Level Design Workshop blockmesh and lighting](https://www.youtube.com/watch?v=09r1B9cVEQY)\n"
"\n"
"### Tech\n"
"- [CppCon 2014: Chandler Carruth \"Efficiency with Algorithms, Performance with Data Structures\"](https://www.youtube.com/watch?v=fHNmRkzxHWs)\n"
"- [CppCon 2014: Mike Acton \"Data-Oriented Design and C++\"](https://www.youtube.com/watch?v=rX0ItVEVjHc&t=1s)\n"
"- [The Animation Pipeline of Mario + Rabbids Kingdom Battle](https://www.youtube.com/watch?v=qxLR8qbD5JE)\n"
"- [code::dive conference 2014 - Scott Meyers: Cpu Caches and Why You Care](https://www.youtube.com/watch?v=WDIkqP4JbkE)\n"
"- [Bret Victor - The Future of Programming](https://www.youtube.com/watch?v=8pTEmbeENF4)\n"
"- [Bret Victor - Inventing on Principle](https://www.youtube.com/watch?v=PUv66718DII)\n"
"- [Bret Victor - Stop Drawing Dead Fish](https://www.youtube.com/watch?v=ZfytHvgHybA)\n"
"- [Dialogue Systems in Double Fine Games](https://www.youtube.com/watch?v=0hMiPBe_VRc)\n"
"- [A Mental Model of CPU Performance](https://www.youtube.com/watch?v=qin-Eps3U_E)\n"
"- [HandmadeCon 2015 - Mike Acton](https://www.youtube.com/watch?v=qWJpI2adCcs)\n"
"- [How to Open a Black Box (originally from MIT GAMBIT)](https://www.youtube.com/watch?v=SYomOZIfeoU)\n"
"- [Immediate-Mode Graphical User Interfaces - 2005](https://www.youtube.com/watch?v=Z1qyvQsjK5Y)\n"
"- [Jonathan Blow - How to program independent games - CSUA Speech](https://www.youtube.com/watch?v=JjDsP5n2kSM)\n"
"- [Jonathan Blow on Deep Work - The Shape of a Problem Doesn't Start Anywhere](https://www.youtube.com/watch?v=4Ej_3NKA3pk)\n"
"- [Look Ma, No Jutter! Optimizing Performance Across Oculus Mobile - Unite LA](https://www.youtube.com/watch?v=JUiO64KbAHA)\n"
"- [Math for Game Programmers - Fast and Funky 1D Nonlinear Transformations](https://www.youtube.com/watch?v=mr5xkf6zSzk)\n"
"- [Practical Procedural Generation for Everyone](https://www.youtube.com/watch?v=WumyfLEa6bU)\n"
"- [Reboot Develop 2017 - Jonathan Blow, Thekla Inc. _ Making Game Programming Less Terrible](https://www.youtube.com/watch?v=De0Am_QcZiQ)\n"
"- [The Terminator Gene (30 minute version)](https://www.youtube.com/watch?v=biuRt_qdcIg)\n"
"- [The Thirty Million Line Problem](https://www.youtube.com/watch?v=kZRE7HIO3vk)\n"
"- [Crash Course in Online Features for Programmers](https://www.youtube.com/watch?v=HVpcRXVkQl0)\n"
"- [Solving the Right Problems for Engine Programmers - Mike Acton (TGC 2017)](https://www.youtube.com/watch?v=4B00hV3wmMY)\n"
"- [Pitfalls of Object Oriented Programming, Revisited - Tony Albrecht (TGC 2017)](https://www.youtube.com/watch?v=VAT9E-M-PoE)\n"
"- [Holistic Optimisation - Tony Albrecht (TGC 2017)](https://www.youtube.com/watch?v=I9d1JOC_b3c)\n"
"- [Advice for Writing Small Programs in C](https://www.youtube.com/watch?v=eAhWIO1Ra6M)\n"
"- [Stanford Seminar - PCG: A Family of Better Random Number Generators](https://www.youtube.com/watch?v=45Oet5qjlms)\n"
"- [Automated Testing: Using AI Controlled Players to Test 'The Division'](https://www.gdcvault.com/play/1026069/Automated-Testing-Using-AI-Controlled)\n"
"- [How King Uses AI in 'Candy Crush'](https://www.gdcvault.com/play/1023858/How-King-Uses-AI-in)\n"
"- [8 Frames in 16ms: Rollback Networking in Mortal Kombat and Injustice 2](https://www.youtube.com/watch?v=7jb0FOcImdg)\n"
"- [Jason Gregory - Dogged Determination (Naughty Doc Tech Culture)](https://www.youtube.com/watch?v=f8XdvIO8JxE)\n"
"- [Elan Ruskin - AI-driven Dynamic Dialog through Fuzzy Pattern Matching](https://www.youtube.com/watch?v=tAbBID3N64A)\n"
"- [Elan Ruskin - Forensic Debugging: How to Autopsy, Repair, and Reanimate a Release-built Game](https://www.gdcvault.com/play/1014351/Forensic-Debugging-How-to-Autopsy)\n"
"- [Fabian Giesen - Let's Talk About Queues](https://vimeo.com/453080016)\n"
"- [Mark Cerny - Method](https://www.youtube.com/watch?v=QOAW9ioWAvE)\n"
"- [Andreas Fredriksson - Context is Everything](https://guide.handmade-seattle.com/c/2021/context-is-everything/)\n"
"- [Sebastian Aaltonen - REAC 2023 DAY 1 Modern Mobile Rendering @ HypeHype](https://www.youtube.com/watch?v=m3bW8d4Brec)\n"
"- [Christer Ericson: Game Development Memory Optimization](https://www.youtube.com/watch?v=t15T_BkOtm0)";

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

    size_t prefixLength = strlen(readmePrefix);
    size_t postFixLength = strlen(readmePostfix);

    size_t entryLength = 0;

    char* linkFormat = "- [%s](%s) -- %s\n";
    for(int index = 0; index < count; ++index)
    {
        PostEntry* entry = &entries[index];
        size_t currentEntryLength = snprintf(NULL, 0, linkFormat, entry->title, entry->url, entry->domain);
        entry->processedEntryLength = currentEntryLength;

        entryLength += currentEntryLength; 
    }

    size_t totalLength = prefixLength + entryLength + postFixLength;
    char* output = (char*)malloc(totalLength + 1);
    memmove(output, readmePrefix, prefixLength);

    entryLength = 0;
    for(int index = 0; index < count; ++index)
    {
        PostEntry* entry = &entries[index];
        size_t currentEntryLength = snprintf(output + prefixLength + entryLength, entry->processedEntryLength, linkFormat, entry->title, entry->url, entry->domain);
        output[prefixLength + entryLength - 1] = '\n';

        entryLength += currentEntryLength;
    }

    memmove(output + prefixLength + entryLength - 1, readmePostfix, postFixLength);
    output[totalLength] = '\0';
    LOG("%s\n", output);

    FILE* file = fopen("./content/pages/reads.md", "w" );
    fwrite(output, 1, totalLength - 1, file);
    fclose(file);

    struct timeval end;
    gettimeofday(&end, NULL);
    double timeSpent = end.tv_sec + end.tv_usec / 1e6 - start.tv_sec - start.tv_usec / 1e6; // in seconds
    printf("Time taken %f seconds\n", timeSpent);

    return 0;
}
