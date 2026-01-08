+++
title = 'HowISpend1000$OnRedditAds'
date = 2025-11-29T10:42:27+07:00
toc = false
+++

This post will document my experience of spending 1000$ on reddit ads

While I worked on the mobile games industry in the past, I never actually done a marketing campaign, setting all the budget, ad groups, targeting, etc. All was done by our publisher, so my job was solely to make a great game.

Currently while working on my own game, I got a 1000$ that I could spend from Gimersia (a government initiave from EKRAF ministry), the caveat is I need to spend it on December, before the 15th, knowing full well that diving to ads on a holiday season would be pretty difficult and facing the big pocketed campaign, I delve down into it.

First I'll need to set a goal, and the benchmark. I need to know if I could get the biggest bang for the limited buck. I'm using this post as a benchmark https://www.reddit.com/r/gamedev/comments/1d22axm/how_i_used_paid_ads_to_reach_steams_popular/ this post run reddit ads 2 years ago by spending 4k and got 4k+ wishlist, placing their game in the popular upcoming.

Back of the napkin calculation lands on 1000 Wl ~ 1$ per wl, that mean I need to spend 60$ daily, seems pretty big since a lot of people on HTMAG mentioned that you should start small and then adjust the budget accordingly. Now that I know what is the limit to spend, I need to set up the targets.

The reason for using reddit ads was my initial guerilla marketing on social media (x, fb, twitter, reddits) wishlists mostly comes from subreddits, more than 60% traffic coming to steam that convert to wishlists comes from there, largely from r/indiedev  

People also mentioned not too target a "generic" subreddits that are too big like r/Games or r/gaming, and don't target to developers. So some of the redidts that I picked is
r/TurnBasedLovers
r/Peglin
r/SlayTheSpire
r/DungeonClawler
r/PuzzleQuest
r/roguelike
r/roguelites

Bear in mind that with a small audience your ads would get saturated quickly, in which an audience sees ads so often that it's effectiveness drops.

I need to also know how and the details of a "good" campaign is, after "briefly" spelunking to youtube videos with keyword "Reddit Ads", I decided to choose Traffic not Conversions. I don't use conversions campaign because most conversions campaign "needs time learn" they decide which good audience is based on the tracking to learn and give better results, since there are no capabilities on steam to do callback events whenever an audience get "converted", there's no way for it to be effective and it logically it would never learn what good audience is, and for better or worse adding those features on steam could make the steam platform itself pay to win.

There are also a bunch of settings on the ad campaign which you could choose.
Bidding strategies: 
Lowest cost - get as many clicks as possible (seems to fit the budget and timeframe of my limit)
Cost cap - Control cost per click, keep your average cost below the cap. Let's say you set it to 0.2$, you'll get beaten by others that put it n 0.3$, also given this is high holiday season, doing this would not be as effective for small budget campaign

Here are the breakdown on the ads daily.

Day 1
Ads seems to be spending, but it is spent mostly on T3 countries, Narrow roguelikes has only 4 impressions, while the other got more than thousands impression. Also double checking from steam, traffic mostly come from mobile, 93%!.
T3 country ends up being targeted, budget are eaten by this low country and it doesn't convert well to wishlist..
So it's clearly converting, but not targeting the correct audience. I'm now changing the campaign to desktop only, and making new campaign so it won't end up eating each other budget.

Setup is now
BroadKeywords (5m audience) - keywords are balatro, indie, indie game, pc gaming, peglin, tokusatsu, slay the spire, roguelike, roguelite, rpg.
Narrow Roguelike (500k audience) - Targeting subreddits
Both targeting T1 country (USA, UK, Canada)

Day 2
I got only 20 clicks! With 1200 impressions, but got 10 wishlist. That puts me at 2$ / wishlist. It is expensive, but converting, which is troublesome since the ads got good players, but doesn't have enough traction and people are not clicking the ads. Ok trying to add more mobile users again, let's see if that works. CTR is also 1%. I also added Australia on the countries


Setup is now
BroadKeywords (17m audience) - Audience suddenly got bigger since I add a lot of new T1 countries
Narrow Roguelike (4m audience) - Same as above
Both targeting T1 country (USA, UK, Canada, Australia, Germany, France, New Zealand, Denmark, Netherlands)

Also duh! This is already saturday so reddit ads review won't get throught before monday... So I'm going to pause this until monday..

Day 3
Turns out it's still friday at the west so the ads got approved! It's now spending at 23~$, each campaigns has a cpc 0.2-0.3$ with ctr 0.5~0.6%. Which is quite good, now I'm going to double the daily spend limit to 40$, let's see how the ads going to spend. Also trying to limit the cpc to 0.2

Day 4
I ended up changing the AdGroup like this
USA
T1 Country
T2 Country
and it gets the biggest bang for buck, I also now trying to create carousel with 5 images, each one has different caption as you scroll through the images. Reddit like humor, so it deliberately made a caption like this

"Made a match-3 game. Grandma is now QA tester (unpaid) (involuntary)"
 Grandma-tested. Grandma-disapproved
 She's writing her own patch notes
 Grandma found 12 bugs already
 She hasn't spoken to me in hours
 The elderly have spoken. Now it's your turn.

 Also USA doesn't spend, setting limit to 0.2 won't work at the USA, probably because this is a holiday season? Previously it could get 0.3-0.5 cpc on US which is pretty high, so I guess I'm trying to also add image to make sure it could spend.

 Day 6
 Previous day isn't spending even if I increase the daily budget into 40$. for 7 more days, I need 700$ more to be spend. Since it's not spending and yesterday I increased the cpc into 0.3 not spending, then I changed the campaign into lowest cap to drive volume again.

 Day 7 
 Ads are now getting stable wishlists. I've been poking around and it seems like removing mobile was a mistake, you must always include mobile. Right now 93% of traffic to my steam page comes from mobile. Seems like this is the most stable setup that I could achieve. I'm going to increase the budget so it could end up at the 15th nicely.


 ....

 This is the complete results of the campaign, I got mostly 600~ wishlists (tracked, seems like after giving enough traffic to steam, it might give some algorithm boost so my page also got some wishlist that is not coming from the ads. I can't confirm this but given the amount of wishlist that I got before spending ads, this seems to be the plausible reason. Spending 925$ and getting 600 wishlists puts me at 1.5$~ per wishlists. 

![Wishlist](/gimersiawishlists.png)
![Campaign](/gimersiacampaigns.png)

 Conclusion:
 Doing ads on holiday for smaller dev was a really big mistake (duh!), but since it's not my money then I guess it's good for me to experiment and get in the waters to test and validate ideas, espescially given I'm solo and doing most social media campaign was pretty exhausting. 
 Almost always include mobile to your audience. If I were to do this again I would:
 1. Test for a very few days which ads format (image, video, or carousel) that has better results
 2. Ad Group would be breakdown to US, T1 Countries, and T2 Countries, each with the same audience, bidding strategy but different ad group so it won't eat each other budget.
 3. Go for Traffic since Conversion won't work for steam.

Thanks! Hopefully it's useful for you as it is useful for me. Wishlist my game on https://store.steampowered.com/app/4131100/Match_Morphosis/
