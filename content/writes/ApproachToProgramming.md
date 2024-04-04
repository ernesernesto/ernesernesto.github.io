+++
title = 'Approach To Programming'
date = 2024-04-02T10:25:12+07:00
toc = false
+++

In X, discussion about what programming method that used always have high interaction, probably because the way the current X works is by giving people payout to whomever has post with high engagements, so now it mostly just bait and trigger. Beside all that, discussion about tool, language, frameworks, what particular dogma approach often times usually sparks a big debate. I was asked from my peers/new person joining the industry on how my approach to programming is. This post boils down to what is my thoughts and approach about programming currently. This are not meant to dictate on how you do things daily as a developer nor a magical recipe to make developer behave and collaborate nicely nor is it foolproof, this are merely just my approach. 

# Simplicity with performance mindset
Write your code with simplicity in mind, write the simplest possible things first. A simple code is easier to optimize, maintain and faster to develop. Do not add unecessary cruft, OOP, getter-setter, pub-sub, dependency injections, frameworks or any act that would fit into "shaping" your code, your goal is not to shape a code, your goal is to solve problems. Basically what I'd say is that it irks me that most discussion about clean code or anything along with that it is entirely code-centric. It's like builders talking about which hammer they use, and what sort of grip the hammer should have (s/builders/programmers, s/hammer/code, s/grip/language features). Code is a tool--that's it. Programmers might supply some code for themselves or others, and if that code isn't a useful tool, then it's not well-written. Being an useful tool means approaching the easiest possible thing to use while also approaching the best possible solution to the problem. With that said, again to hammer my point home, write a simplest possible things first.

My point is, I guess, just worry about the problem at hand. Development shouldn't be about worrying about what golden rule you're breaking or whether your code is 'clean' enough, it should be about solving problems. Attempt to write the simplest possible thing to solve a problem, take out the packages of data that need to be replicated during the process, take out operations that are done in multiple spots and throw them into functions, and at the end, the things that are actually done multiple times are very easy to modify, and the data that the system works with is also very easy to modify. In my experience, the above process never really results in accessors/mutators, data-hiding, or member functions, it also produces code that is really easy to reason about and use.

A couple years ago I might just blindly advice people to 'just' write simple and go through with it. But after a recent [Computer Enhance course](https://www.computerenhance.com/) which opened my mind a lot, simplicity is still what you thrive for, but always have a performance mindset in mind. Know how your machine works and how do they do it, learn how they works to do their job. Knowing your performance characteristics of your machine would put you up in a good position in the future any performance problems show up, and usually without knowing what the performance characteristic are, your work could end up in a state in which a complete rewrite needs to be done in order to achieve your goal. Always write a simple code with performance mindset. 

When it comes to large complex programs, the best programmers I ever worked will figure out a way on how to minimize cognitive load (for lack of a better term). Newbies strain to fit all sorts of unnecessary complexities into their heads, unnecessarily memorizing stuff, writing convoluted algorithms, etc. while more advanced programmers write systems in such a way that they don't need to memorize anything because the overhead in understanding a given module is so low because it is so simple. Programmers tend to spend more time doing things like naming variables, designing interfaces, etc but the payoffs are greatly asymmetrical in the long run because you'll be more productive writing code in a simple system vs. a complex one, generally speaking.

Again, not to sound like a broken record, write the simplest things first, you don't want to end up like Homer in his day job making sure which knob is which with your overcomplex solutions.

![Homer Workplace](/homer_workplace.jpeg)

If it helps, think about this when writing your code, "Someone else should be able to understand it easily".
The **you** in the future with the **you** currently that is writing the code is also a different person.
Write for the future **you** that would come back in the future and forgot how things work in the first place
Programming is already hard, make it easier by writing it easier to read for future **you**, even for simple things, this is probably what you look like returning to your work after one month out of your code base

![I have no memory](/gandalf_memory.jpg)

# Your job is not to code but to solve problems
Solving problem is the key here, what exact problem that your code is solving? Is that an actual problem that someone has? 

Keep data in mind when solving problems, again, know your machine performance characteristics. The purpose of all programs, and all parts of that program, is to transform data from one form to another. If you don't understand the data you don't understand the problem. If you understand your problem, you know that it is just simply transforming one data into another form of data, knowing that you don't need to many "objects" or "smart wrappers". 

Focus on the problem you are currently trying to solve and stop solving general things. Don't come up with a general solution that works for your data and the other data that your program will **literally never see**. 

![The general problem](/xkcd_the_general_problem.png)

Take a step away from your problem, [look from another perspective](https://worrydream.com/LadderOfAbstraction/)

# Plan ahead but don't overgeneralize
Before you code, plan ahead, but just enough and do the simplest things to solve today's problem. It can, and will change in the future often in ways you can't predict, even more apparent in game industry when you're working to find the "fun", you'll need to be able to iterate quickly. 

Do not get attached to your code (or anything..). Some code will get thrown away, and some nasty code will live forever. **Your-will-be-rewrite-and-optimized-later** would more likely to be shipped rather than to be fixed. 

On my 13 years experience on the game industry, the only one who cares about the code is us, the rest of the users and peers cross roles don't really care about the code. At the end of the day, only the final product quality that matters to our players. 

Beware on future-proofing your work, it will and always be changing.

![Future proofing](/future_proofing.png)

# Boyscout Rule and Line of Codes
**"Always leave the campground cleaner when you found it"**.  

"Cleaner" is subjective, but be pragmatic and evaluate those objectively. Things like, if I leave the code here, would another person would be prone to make more error? Will this run more faster and simpler?

Show that you care, so that people will care. See [don't live with broken windows](https://www.artima.com/articles/dont-live-with-broken-windows).

Another thing that people would usually suggest is making sure your function is "small" and fit your screen. This is bogus, and merely ritual, doesn't add to anything that the machine actually working on, and at best it would fall into busy work by "shaping" your code, thus you gain nothing. 

Number of lines of code is not your **metrics**. Your **metrics** should be *"Is this easier for the machine to execute?"*, or for designer *"Would it achieve designer specs and easy to iterate, enhance, and play around with so it could achieve the fun that we want?"*.

When working on a larger team ask *"Would another person have a hard time understanding this code if it's cut into smaller pieces? Would it introduce side effects if its called inappropriately with different order?"*, see [John Carmack about inlining functions](http://number-none.com/blow/john_carmack_on_inlined_code.html).

# Avoid the temptation to rewrite
Always question your assumptions, wear the hat of fellow programmers and think differently. Never blame others when trying to fix a problem. Avoid the temptation of a complete rewrite even though the code is ugly. It might be tested, reviewed, and hardened one. If you really need to rewrite in order to achieve certain goals always try to rewrite it on a smaller portion first, making sure the past code remain intact and can be toggled on and off quickly to verify and check. I've done a theseus ship project and the right approach is to just rewrite it byte-sized before eventually slowly replacing all the project components. 

See [Sebastian Aaltonen twitter threads](https://threadreaderapp.com/thread/1637417126255271936.html) where he completely rewrite HypeHype renderer with something new.

I need to mention that rewrite here is different with reinventing the wheel idiom, I think at current era reinventing the wheel is what you need to do because the roads is changed, 10-20 years ago you could go by a single core, but now the only way to gain performance is to harness all your cores.

# Comments
I rarely do comments on my code, comments that are just one-liner and self explanatory on the code, I usually add comments when writing my approach at the point when I'm finished writing the routine. That would get a picture of what problems it actually solve and what restriction that the code is under at the point of writing. It could even explain the concepts behind the code and why certain decision is taken. In general, a missing comment is better than a bad comment, you can waste people time with a comment that is inaccurate, out of date, or poorly phrased. 

Before you write comments, it helps to think on how can you improve the code so that I don't have to explain? Since at the end of the day, the code is the actual work and it won't lie. Code is dynamic and changes often, but comments are slower to change so it could become stale quickly. See [code without comments](https://blog.codinghorror.com/coding-without-comments/)

# Fin
I realize this is merely high level and no concrete action with examples shown, since I only shown a lot of "philosophical" approach, but I hope you could get a glimpse of my approach. For implementation specifics on code that is a post for another day...
  
![Good code](/xkcd_good_code.png)