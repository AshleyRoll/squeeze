#include <catch2/catch.hpp>

#include <squeeze/squeeze.h>

using namespace squeeze;
using Catch::Matchers::Equals;

static auto buildTableStrings = [] {
    return std::to_array<std::string_view> ({
        "To be, or not to be--that is the question:\n"
        "Whether 'tis nobler in the mind to suffer\n"
        "The slings and arrows of outrageous fortune\n"
        "Or to take arms against a sea of troubles\n"
        "And by opposing end them. To die, to sleep--\n"
        "No more--and by a sleep to say we end\n"
        "The heartache, and the thousand natural shocks\n"
        "That flesh is heir to. 'Tis a consummation\n"
        "Devoutly to be wished. To die, to sleep--\n"
        "To sleep--perchance to dream: ay, there's the rub,\n"
        "For in that sleep of death what dreams may come\n"
        "When we have shuffled off this mortal coil,\n"
        "Must give us pause. There's the respect\n"
        "That makes calamity of so long life.\n"
        "For who would bear the whips and scorns of time,\n"
        "Th' oppressor's wrong, the proud man's contumely\n"
        "The pangs of despised love, the law's delay,\n"
        "The insolence of office, and the spurns\n"
        "That patient merit of th' unworthy takes,\n"
        "When he himself might his quietus make\n"
        "With a bare bodkin? Who would fardels bear,\n"
        "To grunt and sweat under a weary life,\n"
        "But that the dread of something after death,\n"
        "The undiscovered country, from whose bourn\n"
        "No traveller returns, puzzles the will,\n"
        "And makes us rather bear those ills we have\n"
        "Than fly to others that we know not of?\n"
        "Thus conscience does make cowards of us all,\n"
        "And thus the native hue of resolution\n"
        "Is sicklied o'er with the pale cast of thought,\n"
        "And enterprise of great pitch and moment\n"
        "With this regard their currents turn awry\n"
        "And lose the name of action. -- Soft you now,\n"
        "The fair Ophelia! -- Nymph, in thy orisons\n"
        "Be all my sins remembered.",

        "Think not I love him, though I ask for him;\n"
        "'Tis but a peevish boy; yet he talks well.\n"
        "But what care I for words? Yet words do well\n"
        "when he that speaks them pleases those that hear.\n"
        "It is a pretty youth; not very pretty;\n"
        "But sure he's proud; and yet his pride becomes him.\n"
        "He'll make a proper man. The best thing in him\n"
        "Is his complexion; and faster than his tongue\n"
        "Did make offense, his eye did heal it up.\n"
        "He is not very tall; yet for his year's he's tall.\n"
        "His leg is but so so; and yet 'tis well.\n"
        "There was a pretty redness in his lip,\n"
        "A little riper and more lusty red\n"
        "Than that mixed in his cheek; 'twas just the difference\n"
        "Betwixt the constant red and mingled damask.\n"
        "There be some women, Silvius, had they marked him\n"
        "In parcels as I did, would have gone near\n"
        "To fall in love with him; but, for my part,\n"
        "I love him not nor hate him not; and yet\n"
        "I have more cause to hate him than to love him;\n"
        "For what had he to do to chide at me?\n"
        "He said mine eyes were black and my hair black;\n"
        "And, now I am rememb'red, scorned at me.\n"
        "I marvel why I answered not again.\n"
        "But that's all one; omittance is no quittance.\n"
        "I'll write to him a very taunting letter,\n"
        "And thou shalt bear it. Wilt thou, Silvius?",

        "All the world's a stage,\n"
        "And all the men and women merely players;\n"
        "They have their exits and their entrances,\n"
        "And one man in his time plays many parts,\n"
        "His acts being seven ages. At first, the infant,\n"
        "Mewling and puking in the nurse's arms.\n"
        "Then the whining schoolboy, with his satchel\n"
        "And shining morning face, creeping like a snail\n"
        "Unwillingly to school. And then the lover,\n"
        "Sighing like a furnace, with a woeful ballad\n"
        "Made to his mistress' eyebrow. Then a soldier,\n"
        "Full of strange oaths and bearded like the pard,\n"
        "Jealous in honor, sudden and quick in quarrel,\n"
        "Seeking the bubble reputation\n"
        "Even in the cannon's mouth. And then the justice,\n"
        "In fair round belly with good capon lined,\n"
        "With eyes severe and beard of formal cut,\n"
        "Full of wise saws and modern instances;\n"
        "And so he plays his part. The sixth age shifts\n"
        "Into the lean and slippered pantaloon,\n"
        "With spectacles on nose and pouch on side;\n"
        "His youthful hose, well saved, a world too wide\n"
        "For his shrunk shank, and his big manly voice,\n"
        "Turning again toward childish treble, pipes\n"
        "And whistles in his sound. Last scene of all,\n"
        "That ends this strange eventful history,\n"
        "Is second childishness and mere oblivion,\n"
        "Sans teeth, sans eyes, sans taste, sans everything."
    });
};

SCENARIO("StringTable<HuffmanEncoder> can be compile-time initialised", "[StringTable][HuffmanEncoder]") {
    GIVEN("A compile-time initialised StringTable<HuffmanEncoder>"){
        static constinit auto table = StringTable<HuffmanEncoder>(buildTableStrings);

        THEN("The number of strings should be correct"){
            STATIC_REQUIRE(table.count() == 3);
        }

        WHEN("The first string is retrieved") {
            auto s1 = table[0];

            THEN("The string should match the source data") {
                auto sourceTable = buildTableStrings();
                auto expected = std::string{sourceTable[0]};

                std::string extracted{s1.begin(), s1.end()};

                REQUIRE(s1.size() == expected.size());
                REQUIRE_THAT(extracted, Equals(expected));
            }
        }

        WHEN("The second string is retrieved") {
            auto s2 = table[1];

            THEN("The string should match the source data") {
                auto sourceTable = buildTableStrings();
                auto expected = std::string{sourceTable[1]};

                std::string extracted{s2.begin(), s2.end()};

                REQUIRE(s2.size() == expected.size());
                REQUIRE_THAT(extracted, Equals(expected));
            }
        }

        WHEN("The thrid string is retrieved") {
            auto s3 = table[2];

            THEN("The string should match the source data") {
                auto sourceTable = buildTableStrings();
                auto expected = std::string{sourceTable[2]};

                std::string extracted{s3.begin(), s3.end()};

                REQUIRE(s3.size() == expected.size());
                REQUIRE_THAT(extracted, Equals(expected));
            }
        }

        WHEN("An invalid index is accessed") {
            auto s4 = table[3];
            THEN("An empty IterableString should be returned") {
                REQUIRE(s4.size() == 0);
            }
            AND_THEN("Iterating the empty string works") {
                auto t = std::string{s4.begin(), s4.end()};
                REQUIRE(t.size() == 0);
            }
        }
/*
        WHEN("The first string is retrieved") {
            // convert to strings so Catch can use them
            auto t = std::string{table[0]};

            THEN("The string should match the source data") {
                REQUIRE_THAT( t, Equals("First String") );
            }
        }

        WHEN("The second string is retrieved") {
            // convert to strings so Catch can use them
            auto t = std::string{table[1]};

            THEN("The string should match the source data") {
                REQUIRE_THAT( t, Equals("Second String") );
            }
        }

        WHEN("An invalid index is accessed") {
            THEN("An exception should be thrown") {
                REQUIRE_THROWS(table[3]);
            }
        }
*/
    }
}
