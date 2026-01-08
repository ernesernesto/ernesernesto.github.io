+++
title = 'How I Spend 1000$ On Reddit Ads'
date = 2025-11-29T10:42:27+07:00
toc = false
+++

This post will document my experience of spending $1000 on Reddit ads.
While I worked in the mobile games industry in the past, I never actually ran a marketing campaign, setting all the budget, ad groups, targeting, etc. All of that was done by our publisher, so my job was solely to make a great game.

Currently while working on my own game, I got $1000 that I could spend from [Gimersia](https://www.instagram.com/gimersia/) (a government initiative from the EKRAF ministry). The caveat is I need to spend it in December, before the 15th. Knowing full well that diving into ads during a holiday season would be pretty difficult and facing big-pocketed campaigns, I delved down into it.

First I'll need to set a goal and a benchmark. I need to know if I could get the biggest bang for my limited buck.

I'm using this post as a benchmark: https://www.reddit.com/r/gamedev/comments/1d22axm/how_i_used_paid_ads_to_reach_steams_popular/ This post ran Reddit ads 2 years ago by spending $4k and got 4k+ wishlists, placing their game in the popular upcoming.
Back of the napkin calculation lands on 1000 wishlists, so ~$1 per wishlist. That means I need to spend $60 daily, which seems pretty big since a lot of people on HTMAG mentioned that you should start small and then adjust the budget accordingly. 

Now that I know what the limit to spend is, I need to set up the targets.
The reason for using Reddit ads was my initial guerilla marketing on social media (X, FB, Twitter, Reddit). Wishlists mostly came from subreddits, more than 60% of traffic coming to Steam that converted to wishlists came from there, largely from r/indiedev.

People also mentioned not to target "generic" subreddits that are too big like r/Games or r/gaming, and don't target developers. So some of the subreddits that I picked are:
- r/TurnBasedLovers
- r/Peglin
- r/SlayTheSpire
- r/DungeonClawler
- r/PuzzleQuest
- r/roguelike
- r/roguelites

Bear in mind that with a small audience your ads would get saturated quickly, where an audience sees ads so often that their effectiveness drops.

I also need to know the details of what a "good" campaign is. After "briefly" spelunking through YouTube videos with the keyword "Reddit Ads", I decided to choose Traffic not Conversions. I don't use conversion campaigns because most conversion campaigns "need time to learn"—they decide which audiences are good based on tracking to learn and give better results. 

Since there are no capabilities on Steam to do callback events whenever an audience gets "converted", there's no way for it to be effective, and logically it would never learn what a good audience is. For better or worse, adding those features on Steam could make the Steam platform itself pay to win.


There are also a bunch of settings on the ad campaign which you can choose.
Bidding strategies:

- Lowest cost - get as many clicks as possible (seems to fit the budget and timeframe of my limit)
- Cost cap - Control cost per click, keep your average cost below the cap. Let's say you set it to $0.2, you'll get beaten by others that put in $0.3. Also given this is high holiday season, doing this would not be as effective for a small budget campaign.

Here's the breakdown of the ads daily.
# Day 1
Ads seem to be spending, but they're spent mostly on T3 countries. Narrow roguelikes has only 4 impressions, while the others got more than thousands of impressions. Also double checking from Steam, traffic mostly comes from mobile—93%!

T3 countries ended up being targeted, and budget was eaten by these low-value countries, which doesn't convert well to wishlists. So it's clearly converting, but not targeting the correct audience. I'm now changing the campaign to desktop only, and making a new campaign so they won't end up eating each other's budget.

Setup is now:

- BroadKeywords (5m audience) - keywords are balatro, indie, indie game, pc gaming, peglin, tokusatsu, slay the spire, roguelike, roguelite, rpg.
- Narrow Roguelike (500k audience) - Targeting subreddits

Both targeting T1 countries (USA, UK, Canada)

# Day 2
I got only 20 clicks! With 1200 impressions, but got 10 wishlists. That puts me at $2 per wishlist. It's expensive, but converting, which is troublesome since the ads are reaching good players but don't have enough traction and people aren't clicking the ads. 

Ok, trying to add more mobile users again, let's see if that works. CTR is also 1%. I also added Australia to the countries.
Setup is now:

- BroadKeywords (17m audience) - Audience suddenly got bigger since I added a lot of new T1 countries
- Narrow Roguelike (4m audience) - Same as above

Both targeting T1 countries (USA, UK, Canada, Australia, Germany, France, New Zealand, Denmark, Netherlands)

Also duh! This is already Saturday so Reddit ads review won't get through before Monday... So I'm going to pause this until Monday.

# Day 3
Turns out it's still Friday in the west so the ads got approved!

They're now spending at $23. Each campaign has a CPC of $0.2-0.3 with CTR of 0.6%, which is quite good. Now I'm going to double the daily spend limit to $40, let's see how the ads are going to spend. Also trying to limit the CPC to $0.2.

# Day 4
I ended up changing the AdGroups like this:

- USA
- T1 Country
- T2 Country

And it gets the biggest bang for the buck. I'm also now trying to create a carousel with 5 images, each one has a different caption as you scroll through the images. Reddit likes humor, so I deliberately made captions like this:

- Made a match-3 game. Grandma is now QA tester (unpaid) (involuntary)
- Grandma-tested. Grandma-disapproved
- She's writing her own patch notes
- Grandma found 12 bugs already
- She hasn't spoken to me in hours
- The elderly have spoken. Now it's your turn.

Also, USA doesn't spend. Setting limit to $0.2 won't work in the USA, probably because this is a holiday season? Previously it could get $0.3-0.5 CPC in the US which is pretty high, so I guess I'm trying to also add images to make sure it could spend.
# Day 6
Previous day isn't spending even if I increase the daily budget to $40. 

For 7 more days, I need $700 more to be spent. Since it's not spending and yesterday I increased the CPC to $0.3 but it's still not spending, then I changed the campaign to lowest cost to drive volume again.

# Day 7
Ads are now getting stable wishlists. I've been poking around and it seems like removing mobile was a mistake, you must always include mobile. Right now 93% of traffic to my Steam page comes from mobile. Seems like this is the most stable setup that I could achieve. I'm going to increase the budget so it can end at the 15th nicely.

# Final Day and Results
Below are the complete result of the campaign. I got mostly 600~ wishlists (tracked, seems like after giving enough traffic to Steam, it might give some algorithm boost so my page also got some wishlists that aren't coming from the ads. I can't confirm this, but given the amount of wishlists that I got before spending on ads, this seems to be the plausible reason). Spending $925 and getting 600 wishlists puts me at ~$1.5 per wishlist.

![Wishlist](/gimersiawishlists.png)
![Campaign](/gimersiacampaigns.png)

## Conclusion:
Doing ads during the holidays as a smaller dev was a really big mistake (duh!), but since it's not my money then I guess it's good for me to experiment and get in the waters to test and validate ideas, especially given I'm solo and doing most social media campaigns is pretty exhausting.
Almost always include mobile in your audience. If I were to do this again I would:

1. Test for a very few days which ad format (image, video, or carousel) has better results
2. Ad Groups would be broken down to US, T1 Countries, and T2 Countries, each with the same audience and bidding strategy but different ad groups so they won't eat each other's budget.
3. Go for Traffic since Conversion won't work for Steam.

Thanks! Hopefully it's as useful for you as it is for me.

Wishlist my game at https://store.steampowered.com/app/4131100/Match_Morphosis/
