# Horse Breeding Guide

*A complete guide to breeding horses in Story of Alicia*

---

## Overview

Breeding lets you create new foals by pairing your mare with a registered stallion. The foal inherits traits from both parents and their grandparents — coat, mane, tail, stats, and even special abilities can be passed down!

---

## Getting Started

### Requirements

- A **mare** (your horse)
- A **registered stallion** from the Breeding Market (Grade 4-8 only)
- Enough **carrots** to pay the stallion's breeding fee

### Breeding Fees

The stallion owner sets the fee within these ranges:

```
  Grade 4:   4,000 ~ 12,000 carrots
  Grade 5:   5,000 ~ 15,000 carrots
  Grade 6:   6,000 ~ 18,000 carrots
  Grade 7:   8,000 ~ 24,000 carrots
  Grade 8:  10,000 ~ 40,000 carrots
```

> The fee is charged whether or not breeding succeeds!

---

## Success Rate

Breeding doesn't always succeed. The chance depends on the **stallion's grade** and how many times it has been bred before.

### Base Success Rate

```
  Grade 4:  ♥♥♥♥♥    100%  (best odds!)
  Grade 5:  ♥♥♥♥♡     92%
  Grade 6:  ♥♥♥♥       82%
  Grade 7:  ♥♥♥♡       78%
  Grade 8:  ♥♥♥         64%
```

### How It Decreases

Each time a stallion breeds, the success rate drops by **2%**, down to a minimum of **4%**.

```
  Each breeding attempt reduces success rate by 2%.
  All grades eventually bottom out at 4%.

  Grade 4 — starts at 100%, hits 4% after 48 breedings
  Breed #0  ████████████████████████████████████████████████████  100%
  Breed #10 ████████████████████████████████████████              80%
  Breed #20 ████████████████████████████████                      60%
  Breed #30 ████████████████████████                              40%
  Breed #40 ████████████████                                      20%
  Breed #48 ███                                                    4%

  Grade 5 — starts at 92%, hits 4% after 44 breedings
  Breed #0  ███████████████████████████████████████████████        92%
  Breed #10 ███████████████████████████████████████                72%
  Breed #20 ████████████████████████████                           52%
  Breed #30 ██████████████████                                     32%
  Breed #40 ████████                                               12%
  Breed #44 ███                                                     4%

  Grade 8 — starts at 64%, hits 4% after 30 breedings
  Breed #0  █████████████████████████████████                      64%
  Breed #10 ███████████████████████                                44%
  Breed #20 ████████████                                           24%
  Breed #30 ███                                                     4%
```

> **Tip**: A freshly registered Grade 4 stallion has the highest success rate!

### Lucky Bonuses

Sometimes you get a random bonus during breeding:

- **Grade 4-6 stallions**: 10% chance of a bonus
- **Grade 7-8 stallions**: 15% chance of a bonus

| Bonus Type          | Effect                            |
|---------------------|-----------------------------------|
| Pregnancy Boost     | Bonus to success rate (see below) |
| Fertility Peak      | Improves foal grade (Grade 7-8 only) |

#### Pregnancy Boost by Grade

The boost amount depends on the stallion's grade. Each value has a weighted probability within its tier:

**Grade 4-6 stallions** (10% chance to trigger):

| Boost  | +5%  | +10% | +15% | +20% | +40% | +50% |
|--------|------|------|------|------|------|------|
| Weight |  10  |  30  |  35  |  20  |   3  |   2  |

> Most likely outcome: +10% to +20%. The +40% and +50% boosts are very rare.

**Grade 7-8 stallions** (15% chance to trigger):

| Boost  | +40% | +50% | +60% | +70% |
|--------|------|------|------|------|
| Weight |  10  |  20  |  10  |  10  |

> Grade 7-8 stallions always get a large boost (+40% minimum) when the bonus triggers.

---

## What Happens When Breeding Fails?

```
  ┌──────────────────────────┐
  │     BREEDING FAILED!     │
  │                          │
  │   You receive a card:    │
  │                          │
  │   🟥 Normal Card  (85%)  │
  │   🟨 Chance Card  (15%)  │
  │                          │
  │   Mare's combo resets    │
  └──────────────────────────┘
```

- Your mare's breeding combo resets to 0
- You lose your carrots (no refund)
- But don't give up — the card rewards get better the more you spend!

### Failure Card Rewards

The card you receive contains a reward. The reward **quality** depends on how much total money you've spent on breeding attempts since your last successful breed.

#### Reward Grades

Each card gives a reward from one of three quality tiers:

| Grade   | Normal (Red) Card Rewards     | Chance (Yellow) Card Rewards     |
|---------|-------------------------------|----------------------------------|
| Grade A | 100-350 carrots + basic items | 300-1,000 carrots + basic items  |
| Grade B | 300-1,000 carrots + items     | 1,400-4,000 carrots + items      |
| Grade C | 2,000-13,000 carrots + items  | 7,000-25,000 carrots + items     |

> Chance (Yellow) cards always give significantly better rewards than Normal (Red) cards at the same grade!

#### How Spending Affects Reward Grade

The more carrots you've spent on breeding (cumulatively since your last success), the higher your reward grade:

```
  Total Spent     Grade A    Grade B    Grade C
  ─────────────   ───────    ───────    ───────
  4k-8k            100%        0%         0%     (all common)
  10k               90%       10%         0%
  16k               77%       18%         5%
  25k               50%       39%        11%
  35k               22%       60%        18%
  47k               10%       59%        31%
  59k                7%       45%        48%
  77k               10%       20%        70%
  95k                1%        8%        91%
  100k+              0%        0%       100%     (guaranteed rare)
```

> **Important**: Your cumulative spending resets to 0 after a **successful** breeding. So the pity system builds up across consecutive failures only.

---

## The Foal: What It Inherits

When breeding succeeds, your new foal's traits come from a mix of parents and grandparents.

### Grade

The foal's grade is calculated from the **lower** parent's grade, with a random offset. The further apart the parents' grades are, the wider the range of possible outcomes — but the most likely result always clusters near the lower parent.

#### How to read the table

The **lower** parent's grade is the anchor. The offset column shows how far above or below that anchor the foal's grade lands. The final grade is clamped to 1-8.

**Same grade parents** (distance 0) — tightest distribution:

| Offset from lower parent | -3  | -2   | -1    | +0 (same) |
|--------------------------|-----|------|-------|-----------|
| Chance                   | 6%  | 20%  | 34%   | **40%**   |

> Example: Grade 6 + Grade 6 → 40% Grade 6, 34% Grade 5, 20% Grade 4, 6% Grade 3.

**1 grade apart** (distance 1):

| Offset | -3  | -2   | -1    | +0    | +1    |
|--------|-----|------|-------|-------|-------|
| Chance | 6%  | 20%  | 34%   | 26.2% | 13.8% |

> Example: Grade 5 + Grade 6 → anchored at Grade 5. 13.8% Grade 6, 26.2% Grade 5, 34% Grade 4, etc.

**2 grades apart** (distance 2):

| Offset | -3  | -2   | -1    | +0    | +1    | +2   |
|--------|-----|------|-------|-------|-------|------|
| Chance | 6%  | 20%  | 34%   | 21.2% | 12.9% | 5.9% |

**3 grades apart** (distance 3):

| Offset | -3  | -2   | -1    | +0    | +1    | +2   | +3   |
|--------|-----|------|-------|-------|-------|------|------|
| Chance | 6%  | 20%  | 34%   | 17.7% | 12.5% | 7.3% | 2.6% |

**4 grades apart** (distance 4 — max, e.g. Grade 4 + Grade 8):

| Offset | -3  | -2   | -1    | +0    | +1    | +2   | +3   | +4   |
|--------|-----|------|-------|-------|-------|------|------|------|
| Chance | 6%  | 20%  | 34%   | 13.9% | 11.3% | 7.8% | 4.8% | 2.2% |

> Example: Grade 4 + Grade 8 → anchored at Grade 4. The foal is most likely Grade 3 (34%), but has a small 2.2% chance of reaching Grade 8.

#### Key takeaways

- **Matching grades** give the best odds of maintaining that grade (40% chance of +0).
- **Wider gaps** spread the probability thinner — you *can* get a high-grade foal, but it's unlikely.
- There's always a **6% chance of -3** and a **20% chance of -2** regardless of distance — breeding always carries some downside risk.
- The Fertility Peak bonus (Grade 7-8 stallions only) can improve these odds.

### Stats

Your foal has 5 stats: **Agility, Courage, Rush, Endurance, Ambition**.

Each stat is calculated in three steps:

1. **Base**: Average of both parents' stat `(mare + stallion) / 2`
2. **Variation**: A random adjustment is applied (see below)
3. **Scaling**: All 5 stats are proportionally scaled so the total fits the foal's grade range

#### Normal variation (average stat 3+)

A random offset of **-3 to +3** is added to the base, with each value equally likely.

> Example: Mare Agility 12 + Stallion Agility 8 → base 10 → final 7 to 13 before scaling.

#### Mutation bonus (average stat 0-2)

When both parents have very low stats, the foal gets a weighted bonus instead of the normal ±3 offset:

| Parent Average | +0  | +1   | +2   | +3  |
|----------------|-----|------|------|-----|
| 0              | 30% | 40%  | 20%  | 10% |
| 1              | 20% | 50%  | 25%  | 5%  |
| 2              | 10% | 30%  | 50%  | 10% |

> This prevents weak bloodlines from getting permanently stuck at zero — the lower the average, the more likely a small boost.

### Grade and Total Stats

| Grade | Total Stat Points |
|-------|------------------|
| 1     | 0-9              |
| 2     | 10-19            |
| 3     | 20-29            |
| 4     | 30-39            |
| 5     | 40-49            |
| 6     | 50-59            |
| 7     | 60-69            |
| 8     | 70-79            |

---

## Coat Inheritance

When a foal is born, the game rolls to decide whose coat it inherits. There are 4 possible outcomes:

| Source                     | Base Chance | How it works                                                                                         |
|----------------------------|-------------|------------------------------------------------------------------------------------------------------|
| **Mare** (mother)          | ~10%        | Weighted by the mare's coat inheritance rate                                                         |
| **Stallion** (father)      | ~10%+bonus  | Weighted by the stallion's coat inheritance rate, **multiplied** by the stallion bonus (see below)    |
| **Grandparent** (x4)       | ~0.5% each  | Small flat weight per grandparent (mare's 2 parents + stallion's 2 parents)                          |
| **Random** (grade-appropriate) | ~60%    | A random coat is selected from all coats the foal's grade qualifies for, weighted by inheritance rate |

These weights are normalized to 100%, so the actual percentages shift depending on coat inheritance rates and bonuses. The random portion shrinks as parent influence grows, but never drops below ~30%.

### Stallion Bonus

The stallion gets an inheritance bonus based on:

| Factor              | Bonus                             |
|---------------------|-----------------------------------|
| Breeding Combo      | +1% per consecutive success       |
| Stallion Freshness  | Up to +30% (fewer breedings = more) |
| Stallion Lineage    | +1% per lineage point above 1    |

> **Tip**: Use a fresh stallion with high lineage for the best coat inheritance!

### Coat Rarity Tiers

```
  ★☆☆ COMMON                    ★★☆ UNCOMMON                ★★★ RARE
  ┌──────────────────────┐      ┌──────────────────────┐    ┌──────────────────────┐
  │ Chestnut       (G1+) │      │ Blanket Appal. (G6+) │    │ White Grey     (G6+) │
  │ Bay            (G1+) │      │ Dapple Grey    (G6+) │    │ Chestnut Sabino(G6+) │
  │ Champ. Sabino  (G1+) │      │ Chestnut Pinto (G6+) │    │ Black          (G7+) │
  │ Chest. Stockings(G2+)│      │ Palomino       (G6+) │    │ Mealy Bay      (G7+) │
  │ Buckskin       (G3+) │      │ Black Pinto    (G7+) │    │ Amber Cream    (G8+) │
  │ Champagne      (G3+) │      │ Sooty Bay      (G6+) │    │ Black Sabino   (G8+) │
  │ Leopard Appal. (G5+) │      └──────────────────────┘    │ Dapple Bay     (G8+) │
  └──────────────────────┘                                  └──────────────────────┘

  G = Minimum foal grade required
```

> Higher grade foals unlock access to rarer coats!

---

## Face

The foal's face is inherited directly from one parent — **50% chance** from the mare, **50% chance** from the stallion. No grandparent influence or random selection.

---

## Mane & Tail

Mane and tail are inherited **separately** from the coat, but they follow similar family-tree rules:

```
  For EACH of: Color Group, Mane Shape, Tail Shape

  ┌────────────────────────────────────────────────┐
  │ Mare .............. 10%                         │
  │ Stallion .......... 10%                         │
  │ Grandparent 1 ..... 5%                         │
  │ Grandparent 2 ..... 5%                         │
  │ Grandparent 3 ..... 5%                         │
  │ Grandparent 4 ..... 5%                         │
  │ Random ............ 60%                         │
  └────────────────────────────────────────────────┘
```

**Important**: Mane and tail always share the **same color** but can have **different shapes**.

### Available Mane Styles

When a random mane shape is selected (the 60% case), the inheritance rate determines how likely each shape is to be picked. Shapes that don't meet the foal's grade requirement are excluded, and the remaining rates are used as relative weights.

| Style           | Min Grade | Inheritance Rate | Notes                                      |
|-----------------|-----------|------------------|--------------------------------------------|
| Medium Short    | 1         | 30%              | Most common at low grades                  |
| Shaved          | 1         | 30%              |                                            |
| Short           | 1         | 30%              |                                            |
| Extremely Short | 1         | 30%              |                                            |
| Spiky           | 4         | 30%              | Unlocks at Grade 4                         |
| Medium          | 6         | 20%              | Slightly less common than basic shapes     |
| Long            | 6         | 15%              | Noticeably rarer                           |
| Curly           | 7         | 5%               | Very rare — only 5% weight even when eligible |

> Example: A Grade 7 foal can get any shape. The weights are 30+30+30+30+30+20+15+5 = 190 total, so Curly has a 5/190 = **2.6%** chance, while each basic shape has 30/190 = **15.8%**.

> Example: A Grade 3 foal can only get shapes 0-4. Each has equal 30% weight, so it's a flat **25%** each (20% each if the foal is Grade 4+, since Spiky becomes available).

If a shape is inherited from a parent or grandparent (the 10%/5% rolls) but the foal's grade is too low for that shape, a random valid shape is picked instead.

### Available Tail Styles

Same logic — inheritance rates are used as weights for the random roll, and grade-locked shapes are excluded.

| Style       | Min Grade | Inheritance Rate | Notes                            |
|-------------|-----------|------------------|----------------------------------|
| Medium      | 1         | 30%              |                                  |
| Long Thick  | 1         | 30%              |                                  |
| Cropped     | 1         | 20%              | Slightly less common             |
| Long Thin   | 1         | 30%              |                                  |
| Short Thin  | 1         | 30%              |                                  |
| Long Curly  | 7         | 30%              | Rare due to grade lock, not weight |

### Mane/Tail Colors

The mane and tail always share the **same color**. The color is determined by the foal's coat — each coat restricts which **color group** can be used, and the game picks one color from that group.

The color group is inherited from the family tree using the same probabilities as shapes:

| Source               | Chance |
|----------------------|--------|
| Mare                 | 10%    |
| Stallion             | 10%    |
| Each grandparent (x4)| 5%    |
| Random               | 60%    |

If an inherited color group doesn't match the foal's coat restrictions, a random valid color group is used instead (this effectively increases the random chance).

Once the color group is decided, one of its colors is picked at random with equal probability:

| Color Group | Colors (equal chance each)      | Coats that use this group                                       |
|-------------|---------------------------------|-----------------------------------------------------------------|
| Group 1     | White, Light Brown, Dark Brown  | Chestnut, Bay, Chestnut Stockings, Leopard Appal., Chestnut Pinto, Sooty Bay, Palomino, Chestnut Sabino, Mealy Bay, Amber Cream, Dapple Bay |
| Group 2     | Grey, White, Black              | Champagne Sabino, Buckskin, Blanket Appal., White Grey, Dapple Grey, Black Pinto, Black, Black Sabino |
| Group 3     | Grey, White, Brown              | Champagne only                                                  |

> Example: Your foal gets a Chestnut coat (Group 1). The color group roll lands on "random". The game picks one of White, Light Brown, or Dark Brown — each with a **33%** chance. Both mane and tail will be this same color.

---

## Potential

Foals have a chance to be born with a **potential**!

```
  Base chance:              5%
  With ★★ coat:            10%   (+5%)
  With ★★★ coat:           15%   (+10%)
  One parent has ability:  +10%
  Both parents have ability: +20%

  Maximum chance:           35%
```

The potential type is random (1-15, excluding 12) — you never know what you'll get! Potential level and power start at 0 and must be trained.

---

## Lineage

Lineage measures how "pure" a foal's coat bloodline is. It ranges from **1** (no matches) to **9** (perfect bloodline).

```
  How Lineage is Calculated:

  Base:                           1
  + Each parent with same coat:  +2  (max +4)
  + Each grandparent with same:  +1  (max +4)
                                 ─────
  Maximum:                        9
```

```
  Example: Foal gets Bay coat

  ┌───────┐ ┌───────┐ ┌───────┐ ┌───────┐
  │ GP:Bay│ │GP:Ches│ │GP: Bay│ │GP: Bay│
  │  +1   │ │  +0   │ │  +1   │ │  +1   │
  └───┬───┘ └───┬───┘ └───┬───┘ └───┬───┘
      └────┬────┘         └────┬────┘
     ┌─────┴─────┐      ┌─────┴─────┐
     │ Mare: Bay  │      │Stall: Bay │
     │    +2      │      │    +2     │
     └─────┬──────┘      └─────┬────┘
           └──────┬────────────┘
            ┌─────┴─────┐
            │ Foal: Bay │
            │ Lineage=8 │  (1 + 2 + 2 + 1 + 1 + 1)
            └───────────┘
```

> **Why lineage matters**: High lineage stallions give a coat inheritance bonus, making it easier to pass down rare coats!

---

## Foal Personality (Tendency)

Each foal is born with a random personality:

```
  Tendency 1: ░░░░░░░░░░░░░░░░░░░░░░░░░░░░  35%
  Tendency 2: ░░░░░░░░░░░░░░░░░░░░░░░░      30%
  Tendency 3: ░░░░░░░░░░░░░░░░              20%
  Tendency 4: ░░░░░░░░                       10%
  Tendency 5: ░░░░                            5%
```

---

## Physical Appearance

The foal's physical traits (body size, leg length, etc.) are averaged from both parents with a **±20% random variation**:

```
  Mare value:     6
  Stallion value: 8
                  ─
  Average:        7.0
  × random (0.8~1.2)
  Range:          5.6 ~ 8.4
  Clamped to:     [1, 10]
```

Applies to: **Scale, Leg Length, Leg Volume, Body Length, Body Volume**

